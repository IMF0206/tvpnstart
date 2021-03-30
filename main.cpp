#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/types.h>  
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <stdlib.h>

int natstatus = 0;

int dev_get_ip_sys(char *eth, char* strIP)
{
    int sd;
    struct sockaddr_in sin;
    struct ifreq ifr;

    sd = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == sd)
    {
        return -1;
    }

    strcpy(ifr.ifr_name, eth);
    //ifr.ifr_name[IFNAMSIZ - 1] = 0;

    // if error: No such device
    if (ioctl(sd, SIOCGIFADDR, &ifr) < 0)
    {
        close(sd);
        return -1;
    }

    memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
    snprintf(strIP, 16, "%s", inet_ntoa(sin.sin_addr));

    close(sd);
    return 0;
}

int main()
{
    char ip4G[24] = {0};
    while(strlen(ip4G) == 0)
    {
        sleep(5);
        dev_get_ip_sys("eth3", ip4G);
    }
    FILE * ptr;
    char iptun[24] = {0};
    dev_get_ip_sys("tun0", iptun);
    while(1)
    {
		dev_get_ip_sys("tun0", iptun);
        if (strlen(iptun) != 0)
        {
            if (natstatus == 0)
            {
				system("iptables -t nat -A POSTROUTING -o tun0 -j MASQUERADE");
				printf("iptable exec over\n");
				natstatus = 1;
			}
            sleep(5);
            continue;
        }
        ptr = popen("ps -ef| grep tvpn_client | grep -v grep |wc -l", "r");
        if (!ptr)
        {
            printf("ptr is NULL\n");
            sleep(1);
            continue;
        }
        char buf[1024];
        if((fgets(buf, 1024, ptr))!= NULL)
        { 
            int count = atoi(buf);   
            if(count  == 0)
            {
                pclose(ptr);
                printf("进程不存在!\n");
            }
            else 
            {
                printf("进程已找到，有%d个!\n", count);
                pclose(ptr);
                sleep(20);
                continue;
            }
        }
        printf("vpn tun不存在\n");
        char tvpn_cmd[256] = {0};
        strcpy(tvpn_cmd, "/home/sysadm/vpn/nari-peidian-out/tvpn_client /home/sysadm/vpn/nari-peidian-out/client.conf&");
        printf("%s\n", tvpn_cmd);
        pid_t statustun = system(tvpn_cmd);
        if (WIFEXITED(statustun))
        {
            printf("run success\n");
        }
        else
        {
            printf("run failed\n");
        }
    }
}