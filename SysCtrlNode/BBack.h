//全局变量定义
struct sockaddr_in ServerAddr,ClientAddr;									//定义地址结构变量.				
LPCSTR ip;																	//本机IP地址
int BBack = 0;	
int BSocket ;															//主总控节点启动时0表示放行,1表示不放行
char SystemNote[20] = {"SystemNote.txt"};								//系统日志文件名
int BSocketInit(int port)
{
	int BClientSocket = 0;
	u_short dport=port;
	char name[255];															//用于存放主机名
    PHOSTENT hostinfo;														//主机信息数据结构变量,用于存放函数返回的主机参数.
	WSADATA wsaData;														//用于WSAStartup函数返回值.
	if ( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) == 0 )
	{
		if( gethostname ( name, sizeof(name)) == 0) 
		{//判断是否成功的将本地主机名存放入由name参数指定的缓冲区中
			if((hostinfo = gethostbyname(name)) != NULL) //获取主机的信息.
			{//如果获得主机名成功的话，调用inet_ntoa()函数取得IP地址
				ip= inet_ntoa (*(struct in_addr *)*hostinfo->h_addr_list);	
			} 
		} 
	}
	ClientAddr.sin_addr.S_un.S_addr= inet_addr(ip);							//设置本机的IP地址.
	ClientAddr.sin_family=AF_INET;											//指定为TCP/IP地址类型
	memset(&ClientAddr.sin_zero,8,0);	
	ServerAddr.sin_addr.S_un.S_addr= inet_addr(ip); 
	ServerAddr.sin_port=htons(dport);          								//端口号,htons()主机格式转换为网络格式
	ServerAddr.sin_family=AF_INET;
	memset(&ServerAddr.sin_zero,8,0);
	if ((BClientSocket=socket(AF_INET,SOCK_STREAM,0))==-1)					//创建插口 socket.
	{
		printf("创建SOCKET失败!!!\n");
	}
	return BClientSocket;
}
DWORD WINAPI ThreadBClientHeart( LPVOID lpParam )
{
	char HR[2];
	int chcnt = 0 ;
	while(1)
	{		
		if ((chcnt = recv(BSocket,HR,2,0))==-1)	//使用accept函数返回的socket号!
		{
			printf("\t\t副总控节点已关闭!!!\n");
			printf("\t\t主总控节点接管工作!!!\n");
			write2Log("副总控节点",SystemNote,0);//0是断开
			BBack = 0;
			break;
		}
		if ( 'S' == HR[0] )
		{	
			printf("\t\t因系统退出，主总控关闭\n");
			Sleep(1000);
			exit(0);
		}
		Sleep(10);
	}
	return 0;
}
//连接转发节点
void Connect2CtrlNode( )
{
	struct sockaddr SA1;
	int i = 3;
	while(i--)
	{
		if (connect( BSocket,(struct sockaddr *)&ServerAddr,sizeof(SA1))==-1)
		{
			printf("\t\t。。");
			BBack = 0;
		}
		else
		{	
			BBack = 1;
			printf("\t\t连接工作总控节点成功,作为备用总控\n");
			CreateThread( NULL, 0, ThreadBClientHeart, &i, 0, 0 );
			break;
		}
	}
	if( 0 == i)
	{
		printf("\n");
	}
}
