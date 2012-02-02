#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <signal.h>

#include "mypcap.h"
#include "sniff_cookies_lib.h"

/* ---------- Global variables ---------- */
pcap_t *handle = NULL;

int main (int argc, char ** argv)
{
	char *dev, errbuf[PCAP_ERRBUF_SIZE];
	struct bpf_program fp;
	char filter_exp[] = "port 80";
	bpf_u_int32 net, mask;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	printf("###############################\n");
	printf("##  Sniff Cookies by Skyper  ##\n");
	printf("##  http://blog.skyplabs.net ##\n");
	printf("###############################\n");

	if (argc < 2)
	{
		dev = pcap_lookupdev(errbuf);

		if (dev == NULL)
		{
			fprintf(stderr, "[x] Error during looking up device : %s\n", errbuf);
			exit(EXIT_FAILURE);
		}
	}
	else
		dev = argv[1];

	printf("[*] Device :\t%s\n", dev);

	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1)
	{
		fprintf(stderr, "[!] Can't get netmask for device %s : %s\n", dev, errbuf);
		net = mask = 0;
	}

	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);

	if (handle == NULL)
	{
		fprintf(stderr, "[x] Couldn't open device %s : %s\n", dev, errbuf);
		exit(EXIT_FAILURE);
	}

	if ((pcap_compile(handle, &fp, filter_exp, 0, net)) == -1)
	{
		fprintf(stderr, "[x] Couldn't parse filter %s : %s\n", filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	if ((pcap_setfilter(handle, &fp)) == -1)
	{
		fprintf(stderr, "[x] Couldn't install filter %s : %s\n", filter_exp, pcap_geterr(handle));
		exit(EXIT_FAILURE);
	}

	printf("[*] Start sniffing ...\n");

	if ((pcap_loop(handle, -1, got_packet, NULL) == -1))
	{
		fprintf(stderr, "[x] Error during reading packets\n");
		exit(EXIT_FAILURE);
	}

	pcap_close(handle);

	return EXIT_SUCCESS;
}
