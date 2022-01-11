/*
 * Copyright (c) 1998-2001
 * University of Southern California/Information Sciences Institute.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Part of this program has been derived from mrouted.
 * The mrouted program is covered by the license in the accompanying file
 * named "LICENSE.mrouted".
 *
 * The mrouted program is COPYRIGHT 1989 by The Board of Trustees of
 * Leland Stanford Junior University.
 *
 */
#ifndef PIMD_DEFS_H_
#define PIMD_DEFS_H_

#include "config.h"

#ifdef BSD		/* Neither Linux or SunOS has this BSDism */
# include <sys/cdefs.h>	/* Defines __BSD_VISIBLE, needed for arc4random() etc. */
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <unistd.h> 
#include <ctype.h>
#include <errno.h>
#include <syslog.h>
#include <signal.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/igmp.h>
#include <netinet/in.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>
#ifdef HAVE_FCNTL_H
# include <fcntl.h>
#endif
#if ((defined(SYSV)) || (defined(__bsdi__)) || ((defined SunOS) && (SunOS < 50)))
#include <sys/sockio.h>
#endif /* SYSV || bsdi || SunOS 4.x */
#include <time.h>
#include <sys/time.h>
#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#include <osreldate.h>
#endif /* __FreeBSD__ */
#if defined(__bsdi__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
#define rtentry kernel_rtentry
#include <net/route.h>
#undef rtentry
#endif /* bsdi or __FreeBSD_version >= 220000 */
#ifdef __linux__
#define _LINUX_IN_H             /* For Linux <= 2.6.25 */
#include <linux/types.h>
#include <linux/mroute.h>
#else
#include <netinet/ip_mroute.h>
#endif /* __linux__ */

#include <strings.h>

#ifndef BYTE_ORDER
#if defined(__linux__) || defined(SYSV)
#include <endian.h>
#elif (BSD >= 199103)
#include <machine/endian.h>
#else
#define LITTLE_ENDIAN	1234	/* least-significant byte first (vax, pc) */
#define BIG_ENDIAN	4321	/* most-significant byte first (IBM, net) */
#define PDP_ENDIAN	3412	/* LSB first in word, MSW first in long (pdp) */

#if defined(vax) || defined(ns32000) || defined(sun386) || defined(i386) || \
    defined(__ia64) || \
    defined(MIPSEL) || defined(_MIPSEL) || defined(BIT_ZERO_ON_RIGHT) || \
    defined(__alpha__) || defined(__alpha)
#define BYTE_ORDER	LITTLE_ENDIAN
#endif

#if defined(sel) || defined(pyr) || defined(mc68000) || defined(sparc) || \
    defined(is68k) || defined(tahoe) || defined(ibm032) || defined(ibm370) || \
    defined(MIPSEB) || defined(_MIPSEB) || defined(_IBMR2) || defined(DGUX) ||\
    defined(apollo) || defined(__convex__) || defined(_CRAY) || \
    defined(__hppa) || defined(__hp9000) || \
    defined(__hp9000s300) || defined(__hp9000s700) || \
    defined(BIT_ZERO_ON_LEFT) || defined(m68k)
#define BYTE_ORDER	BIG_ENDIAN
#endif
#endif /* __linux__ || SYSV or BSD */
#endif /* BYTE_ORDER */

typedef void (*cfunc_t)  (void *);
typedef void (*ihfunc_t) (int);

#include "dvmrp.h"     /* Added for further compatibility and convenience */
#include "pimd.h"
#include "mrt.h"
#include "igmpv2.h"
#include "igmpv3.h"
#include "vif.h"
#include "debug.h"
#include "pathnames.h"
#ifdef RSRR
#include "rsrr.h"
#include "rsrr_var.h"
#endif /* RSRR */

/*
 * Miscellaneous constants and macros
 */

#define ENABLINGSTR(val)        (val) ? "enabling" : "disabling"
#define is_set(flag, flags)     (((flag) & (flags)) == (flag))

/*
 * Various definitions to make it working for different platforms
 */
/* The old style sockaddr definition doesn't have sa_len */
#if defined(_AIX) || (defined(BSD) && (BSD >= 199006)) /* sa_len was added with 4.3-Reno */
#define HAVE_SA_LEN
#endif

/* Versions of Solaris older than 2.6 don't have routing sockets. */
/* XXX TODO: check FreeBSD version and add all other platforms */
#if defined(__linux__)   || (defined(SunOS) && SunOS >=56) || \
    defined(IRIX)        ||  defined(__bsdi__)             || \
    defined(__FreeBSD__) ||  defined(__FreeBSD_kernel__)   || \
    defined(NetBSD)      ||  defined(__DragonFly__)        || \
    defined(OpenBSD)
#define HAVE_ROUTING_SOCKETS	1
#endif

/* Older versions of UNIX don't really give us true raw sockets.
 * Instead, they expect ip_len and ip_off in host byte order, and also
 * provide them to us in that format when receiving raw frames.
 *
 * This list could probably be made longer, e.g., SunOS and __bsdi__
 */
#if defined(__NetBSD__) ||					\
    (defined(__FreeBSD__) && (__FreeBSD_version < 1100030)) ||	\
    (defined(__OpenBSD__) && (OpenBSD < 200311))
#define HAVE_IP_HDRINCL_BSD_ORDER
#endif

/*
 * workaround for SunOS/Solaris/Illumos which defines this in
 * addition to the more logical __sun and __svr4__ macros.
 */
#ifdef sun
#undef sun
#endif

#define TRUE			1
#define FALSE			0

#ifndef MAX
#define MAX(a,b) (((a) >= (b))? (a) : (b))
#define MIN(a,b) (((a) <= (b))? (a) : (b))
#endif /* MAX & MIN */

#define CREATE                  TRUE
#define DONT_CREATE             FALSE

#define MFC_MOVE_FORCE		0x1
#define MFC_UPDATE_FORCE	0x2

#define EQUAL(s1, s2)		(strcmp((s1), (s2)) == 0)
#define NELEMS(a)		(sizeof((a)) / sizeof((a)[0]))

#define JAN_1970                2208988800UL    /* 1970 - 1900 in seconds */

#define MINTTL			1  /* min TTL in the packets send locally */

#define MAX_IP_PACKET_LEN       576
#define MIN_IP_HEADER_LEN       20 /* sizeof(struct ip) */
#define IP_IGMP_HEADER_LEN      24 /* MIN + Router Alert */
#define MAX_IP_HEADER_LEN       60


/*
 * The IGMPv2 <netinet/in.h> defines INADDR_ALLRTRS_GROUP, but earlier
 * ones don't, so we define it conditionally here.
 */
#ifndef INADDR_ALLRTRS_GROUP	/* address for multicast mtrace msg */
#define INADDR_ALLRTRS_GROUP	(in_addr_t)0xe0000002	/* 224.0.0.2 */
#endif

#ifndef INADDR_ALLRPTS_GROUP
#define INADDR_ALLRPTS_GROUP    (in_addr_t)0xe0000016	/* 224.0.0.22, IGMPv3 */
#endif

#ifndef INADDR_MAX_LOCAL_GROUP
#define INADDR_MAX_LOCAL_GROUP	(in_addr_t)0xe00000ff	/* 224.0.0.255 */
#endif

#define INADDR_ANY_N            (in_addr_t)0x00000000	/* INADDR_ANY in network order */
#define CLASSD_PREFIX           (in_addr_t)0xe0000000	/* 224.0.0.0 */
#define STAR_STAR_RP_MSKLEN     4                       /* Masklen for
							 * 224.0.0.0 :
							 * to encode (*,*,RP)
							 */
#define ALL_MCAST_GROUPS_ADDR   (in_addr_t)0xe0000000     /* 224.0.0.0 */
#define ALL_MCAST_GROUPS_LEN    4

/* Used by DVMRP */
#define DEFAULT_METRIC		1	/* default subnet/tunnel metric     */
#define DEFAULT_THRESHOLD	1	/* default subnet/tunnel threshold  */

/* Used if no relaible unicast routing information available */
#define UCAST_DEFAULT_ROUTE_DISTANCE   101
#define UCAST_DEFAULT_ROUTE_METRIC     1024

#define TIMER_INTERVAL		5	/* 5 sec virtual timer granularity  */

/*
 * TODO: recalculate the messages sizes, probably with regard to the MTU
 */
/* We must assume "safe" maximum atleast until we handle MTU correctly. */
#define MAX_JP_MESSAGE_SIZE     (576 \
				- sizeof(struct ip) \
				- sizeof(pim_header_t) \
				- sizeof(pim_jp_header_t))
#define MAX_JP_MESSAGE_POOL_NUMBER 8

#ifdef RSRR
#define BIT_ZERO(X)		((X) = 0)
#define BIT_SET(X,n)		((X) |= 1 << (n))
#define BIT_CLR(X,n)		((X) &= ~(1 << (n)))
#define BIT_TST(X,n)		((X) & 1 << (n))
#endif /* RSRR */

#if defined(SYSV)
#define bcopy(a, b, c)		memcpy((b), (a), (c))
#define bzero(s, n)		memset((s), 0, (n))
#define setlinebuf(s)		setvbuf((s), (NULL), (_IOLBF), 0)
#define RANDOM()		lrand48()
#elif defined(BSD)
#define RANDOM()		arc4random()
#else
#define RANDOM()		(uint32_t)random()
#endif

/* NetBSD 6.1, for instance, does not have IPOPT_RA defined. */
#ifndef IPOPT_RA
#define IPOPT_RA                148
#endif

/*
 * External declarations for global variables and functions.
 */
#define			SEND_BUF_SIZE (128*1024)  /* Maximum buff size to
						   * send a packet */
#define			RECV_BUF_SIZE (128*1024)  /* Maximum buff size to
						   * receive a packet */
#define                 SO_SEND_BUF_SIZE_MAX (256*1024)
#define                 SO_SEND_BUF_SIZE_MIN (48*1024)
#define                 SO_RECV_BUF_SIZE_MAX (256*1024)
#define                 SO_RECV_BUF_SIZE_MIN (48*1024)


/*
 * Global settings, from config.c
 */
extern uint16_t         pim_timer_hello_interval;
extern uint16_t         pim_timer_hello_holdtime;

/* TODO: describe the variables and clean up */
extern char		*igmp_recv_buf;
extern char		*igmp_send_buf;
extern char		*pim_recv_buf;
extern char		*pim_send_buf;
extern int		igmp_socket;
extern int		pim_socket;
extern uint32_t		allhosts_group;
extern uint32_t		allrouters_group;
extern uint32_t		allreports_group;
extern uint32_t		allpimrouters_group;
extern build_jp_message_t *build_jp_message_pool;
extern int		build_jp_message_pool_counter;

extern uint32_t		virtual_time;
extern char            *ident;
extern char	       *config_file;
extern char            *prognm;
extern char             versionstring[];

extern struct cand_rp_adv_message_ {
    uint8_t  *buffer;
    uint8_t  *insert_data_ptr;
    uint8_t  *prefix_cnt_ptr;
    uint16_t  message_size;
} cand_rp_adv_message;

extern int do_vifs;
extern int no_fallback;
extern int retry_forever;
extern int mrt_table_id;

/*
 * Used to contol the switching to the shortest path:
 */
typedef enum {
    SPT_RATE,
    SPT_PACKETS,
    SPT_INF
} spt_mode_t;

typedef struct {
    uint8_t   mode;
    uint32_t  bytes;
    uint32_t  packets;
    uint32_t  interval;
} spt_threshold_t;
extern spt_threshold_t  spt_threshold;

extern cand_rp_t        *cand_rp_list;
extern grp_mask_t       *grp_mask_list;
extern cand_rp_t        *segmented_cand_rp_list;
extern grp_mask_t       *segmented_grp_mask_list;

extern uint16_t          curr_bsr_fragment_tag;
extern uint8_t           curr_bsr_priority;
extern uint32_t          curr_bsr_address;
extern uint32_t          curr_bsr_hash_mask;
extern uint8_t		 cand_bsr_flag;		   /* candidate BSR flag */
extern uint8_t           my_bsr_priority;
extern uint16_t          my_bsr_adv_period;        /* RFC5059: BS_Period */
extern uint16_t          my_bsr_timeout;           /* RFC5059: BS_Timeout */
extern uint16_t          recommended_rp_holdtime;  /* RFC5059: RP_Holdtime */
extern uint32_t          my_bsr_address;
extern uint32_t          my_bsr_hash_mask;
extern uint8_t           cand_rp_flag;              /* Candidate RP flag */
extern uint32_t          my_cand_rp_address;
extern uint8_t           my_cand_rp_priority;
extern uint16_t          my_cand_rp_holdtime;
extern uint16_t          my_cand_rp_adv_period;     /* The locally configured
						    * Cand-RP adv. period.
						    */
extern uint16_t          pim_bootstrap_timer;
extern uint32_t          rp_my_ipv4_hashmask;
extern uint16_t          pim_cand_rp_adv_timer;

/* route.c */
extern uint32_t		default_route_metric;
extern uint32_t		default_route_distance;

/* igmp_proto.c */
extern uint32_t		igmp_query_interval;
extern uint32_t		igmp_querier_timeout;

/* mrt.c */
extern srcentry_t 	*srclist;
extern grpentry_t 	*grplist;

/* vif.c */
extern struct uvif	uvifs[MAXVIFS];
extern vifi_t		numvifs;
extern int              total_interfaces;
extern int              phys_vif;
extern int		udp_socket;

extern int		vifs_down;

#define MAX_INET_BUF_LEN 19
extern char		s1[MAX_INET_BUF_LEN];
extern char		s2[MAX_INET_BUF_LEN];
extern char		s3[MAX_INET_BUF_LEN];
extern char		s4[MAX_INET_BUF_LEN];

#if !((defined(BSD) && (BSD >= 199103)) || (defined(__linux__)))
extern int		errno;
#endif


#ifndef IGMP_MEMBERSHIP_QUERY
#define IGMP_MEMBERSHIP_QUERY		IGMP_HOST_MEMBERSHIP_QUERY
#if !(defined(NetBSD) || defined(OpenBSD) || defined(__FreeBSD__))
#define IGMP_V1_MEMBERSHIP_REPORT	IGMP_HOST_MEMBERSHIP_REPORT
#define IGMP_V2_MEMBERSHIP_REPORT	IGMP_HOST_NEW_MEMBERSHIP_REPORT
#else
#define IGMP_V1_MEMBERSHIP_REPORT	IGMP_v1_HOST_MEMBERSHIP_REPORT
#define IGMP_V2_MEMBERSHIP_REPORT	IGMP_v2_HOST_MEMBERSHIP_REPORT
#endif
#define IGMP_V2_LEAVE_GROUP		IGMP_HOST_LEAVE_MESSAGE
#endif
#if defined(__FreeBSD__)		/* From FreeBSD 8.x */
#define IGMP_V3_MEMBERSHIP_REPORT       IGMP_v3_HOST_MEMBERSHIP_REPORT
#else
#define IGMP_V3_MEMBERSHIP_REPORT	0x22	/* Ver. 3 membership report */
#endif

#if defined(NetBSD) || defined(OpenBSD) || defined(__FreeBSD__)
#define IGMP_MTRACE_RESP		IGMP_MTRACE_REPLY
#define IGMP_MTRACE			IGMP_MTRACE_QUERY
#endif

#ifndef IGMP_TIMER_SCALE
#define IGMP_TIMER_SCALE     10	    /* denotes that the igmp->timer filed */
				    /* specifies time in 10th of seconds  */
#endif

/* For timeout. The timers count down */
#define SET_TIMER(timer, value) (timer) = (value)
#define RESET_TIMER(timer) (timer) = 0
#define COPY_TIMER(timer_1, timer_2) (timer_2) = (timer_1)
#define IF_TIMER_SET(timer) if ((timer) > 0)
#define IF_TIMER_NOT_SET(timer) if ((timer) <= 0)
#define FIRE_TIMER(timer)       (timer) = 0

#define IF_TIMEOUT(timer)		\
	if (!((timer) -= (MIN(timer, TIMER_INTERVAL))))

#define IF_NOT_TIMEOUT(timer)		\
	if ((timer) -= (MIN(timer, TIMER_INTERVAL)))

#define TIMEOUT(timer)			\
	(!((timer) -= (MIN(timer, TIMER_INTERVAL))))

#define NOT_TIMEOUT(timer)		\
	((timer) -= (MIN(timer, TIMER_INTERVAL)))

#define ELSE else           /* To make emacs cc-mode happy */      

#define MASK_TO_VAL(x, i) {		   \
	uint32_t _x = ntohl(x);		   \
	(i) = 1;			   \
	while ((_x) <<= 1)		   \
	    (i)++;			   \
    };

#define VAL_TO_MASK(x, i) {			\
	x = htonl(~((1 << (32 - (i))) - 1));	\
    };

/*
 * External function definitions
 */

/* callout.c */
extern void	timer_init		(void);
extern void	timer_exit		(void);
extern void	timer_age_queue		(int);
extern int	timer_next_delay	(void);
extern int	timer_set		(int, cfunc_t, void *);
extern void	timer_clear		(int);
extern int	timer_get		(int);

/* config.c */
extern void	config_vifs_from_kernel	(void);
extern void	config_vifs_from_file	(void);

/* dvmrp_proto.c */
extern void	dvmrp_accept_probe	(uint32_t src, uint32_t dst, uint8_t *p, int datalen, uint32_t level);
extern void	dvmrp_accept_report	(uint32_t src, uint32_t dst, uint8_t *p, int datalen, uint32_t level);
extern void	dvmrp_accept_info_request (uint32_t src, uint32_t dst, uint8_t *p, int datalen);
extern void	dvmrp_accept_info_reply	(uint32_t src, uint32_t dst, uint8_t *p, int datalen);
extern void	dvmrp_accept_neighbors	(uint32_t src, uint32_t dst, uint8_t *p, int datalen, uint32_t level);
extern void	dvmrp_accept_neighbors2	(uint32_t src, uint32_t dst, uint8_t *p, int datalen, uint32_t level);
extern void	dvmrp_accept_prune	(uint32_t src, uint32_t dst, uint8_t *p, int datalen);
extern void	dvmrp_accept_graft	(uint32_t src, uint32_t dst, uint8_t *p, int datalen);
extern void	dvmrp_accept_g_ack	(uint32_t src, uint32_t dst, uint8_t *p, int datalen);

/* igmp.c */
extern void	init_igmp		(void);
extern void	send_igmp		(char *buf, uint32_t src, uint32_t dst, int type, int code, uint32_t group, int datalen);

/* igmp_proto.c */
extern void	query_groups		(struct uvif *v);
extern void	accept_membership_query	(int ifi, uint32_t src, uint32_t dst, uint32_t group, int tmo, int igmp_version);
extern void	accept_group_report	(int ifi, uint32_t src, uint32_t dst, uint32_t group, int r_type);
extern void	accept_leave_message	(int ifi, uint32_t src, uint32_t dst, uint32_t group);
extern void	accept_membership_report(int ifi, uint32_t src, uint32_t dst, struct igmpv3_report *report, ssize_t reportlen);

/* inet.c */
extern int	inet_cksum		(uint16_t *addr, u_int len);
extern int	inet_valid_host		(uint32_t naddr);
extern int	inet_valid_mask		(uint32_t mask);
extern int	inet_valid_subnet	(uint32_t nsubnet, uint32_t nmask);
extern char	*inet_fmt		(uint32_t addr, char *s, size_t len);
extern char	*netname		(uint32_t addr, uint32_t mask);
extern uint32_t	inet_parse		(char *s, int n);

/* ipc.c */
extern void	ipc_init		(char *sockfile);
extern void	ipc_exit		(void);

/* kern.c */
extern void	k_set_sndbuf		(int socket, int bufsize, int minsize);
extern void	k_set_rcvbuf		(int socket, int bufsize, int minsize);
extern void	k_hdr_include		(int socket, int val);
extern void	k_set_pktinfo		(int socket, int val);
extern void	k_set_ttl		(int socket, int t);
extern void	k_set_loop		(int socket, int l);
extern void	k_set_if		(int socket, uint32_t ifa);
extern void	k_set_router_alert	(int socket);
extern void	k_join			(int socket, uint32_t grp, struct uvif *v);
extern void	k_leave			(int socket, uint32_t grp, struct uvif *v);
extern void	k_init_pim		(int socket);
extern void	k_stop_pim		(int socket);
extern int	k_del_mfc		(int socket, uint32_t source, uint32_t group);
extern int	k_chg_mfc		(int socket, uint32_t source, uint32_t group, vifi_t iif, uint8_t *oifs,
                                         uint32_t rp_addr);
extern void	k_add_vif		(int socket, vifi_t vifi, struct uvif *v);
extern void	k_del_vif		(int socket, vifi_t vifi, struct uvif *v);
extern int	k_get_vif_count		(vifi_t vifi, struct vif_count *retval);
extern int	k_get_sg_cnt		(int socket, uint32_t source, uint32_t group, struct sg_count *retval);

/* main.c */
extern int	register_input_handler	(int fd, ihfunc_t func);
extern int      daemon_restart          (char *buf, size_t len);
extern int      daemon_kill             (char *buf, size_t len);

/* mrt.c */
extern void	init_pim_mrt		(void);
extern mrtentry_t *find_route		(uint32_t source, uint32_t group, uint16_t flags, char create);
extern grpentry_t *find_group		(uint32_t group);
extern srcentry_t *find_source		(uint32_t source);
extern void	delete_mrtentry		(mrtentry_t *mrtentry_ptr);
extern void	delete_srcentry		(srcentry_t *srcentry_ptr);
extern void	delete_grpentry		(grpentry_t *grpentry_ptr);
extern void	delete_mrtentry_all_kernel_cache (mrtentry_t *mrtentry_ptr);
extern void	delete_single_kernel_cache (mrtentry_t *mrtentry_ptr, kernel_cache_t *kernel_cache_ptr);
extern void	delete_single_kernel_cache_addr (mrtentry_t *mrtentry_ptr, uint32_t source, uint32_t group);
extern void	add_kernel_cache	(mrtentry_t *mrtentry_ptr, uint32_t source, uint32_t group, uint16_t flags);
/* pim.c */
extern void	init_pim		(void);
extern void	send_pim		(char *buf, uint32_t src, uint32_t dst, int type, size_t len);
extern void	send_pim_unicast	(char *buf, int mtu, uint32_t src, uint32_t dst, int type, size_t len);

/* pim_proto.c */
extern int	receive_pim_hello	(uint32_t src, uint32_t dst, char *msg, size_t len);
extern int	send_pim_hello		(struct uvif *v, uint16_t holdtime);
extern void	delete_pim_nbr		(pim_nbr_entry_t *nbr_delete);
extern int	receive_pim_register	(uint32_t src, uint32_t dst, char *msg, size_t len);
extern int	send_pim_null_register	(mrtentry_t *r);
extern int	receive_pim_register_stop (uint32_t src, uint32_t dst, char *msg, size_t len);
extern int	send_pim_register	(char *pkt);
extern int	receive_pim_join_prune	(uint32_t src, uint32_t dst, char *msg, size_t len);
extern int	join_or_prune		(mrtentry_t *mrtentry_ptr, pim_nbr_entry_t *upstream_router);
extern int	receive_pim_assert	(uint32_t src, uint32_t dst, char *msg, size_t len);
extern int	send_pim_assert		(uint32_t source, uint32_t group, vifi_t vifi, mrtentry_t *mrtentry_ptr);
extern void     send_pim_join           (pim_nbr_entry_t *pim_nbr, mrtentry_t *mrt, uint16_t flags, uint16_t holdtime);
extern int	send_periodic_pim_join_prune (vifi_t vifi, pim_nbr_entry_t *pim_nbr, uint16_t holdtime);
extern int	add_jp_entry		(pim_nbr_entry_t *pim_nbr, uint16_t holdtime, uint32_t group, uint8_t grp_msklen,
                                         uint32_t source, uint8_t src_msklen,  uint16_t addr_flags, uint8_t join_prune);
extern void	pack_and_send_jp_message (pim_nbr_entry_t *pim_nbr);
extern int	receive_pim_cand_rp_adv	(uint32_t src, uint32_t dst, char *msg, size_t len);
extern int	receive_pim_bootstrap	(uint32_t src, uint32_t dst, char *msg, size_t len);
extern int	send_pim_cand_rp_adv	(void);
extern void	send_pim_bootstrap	(void);

/* route.c */
extern void	init_route		(void);
extern int	set_incoming		(srcentry_t *srcentry_ptr, int srctype);
extern vifi_t	get_iif			(uint32_t source);
extern pim_nbr_entry_t *find_pim_nbr	(uint32_t source);
extern int	add_sg_oif		(mrtentry_t *mrtentry_ptr, vifi_t vifi, uint16_t holdtime, int update_holdtime);
extern void	add_leaf		(vifi_t vifi, uint32_t source, uint32_t group);
extern void	delete_leaf		(vifi_t vifi, uint32_t source, uint32_t group);
extern int	change_interfaces	(mrtentry_t *mrtentry_ptr,  vifi_t new_iif,
                                         uint8_t *new_joined_oifs_, uint8_t *new_pruned_oifs,
                                         uint8_t *new_leaves_, uint8_t *new_asserted_oifs, uint16_t flags);
extern void	calc_oifs		(mrtentry_t *mrtentry_ptr, uint8_t *oifs_ptr);
extern void	process_kernel_call	(void);
extern int	delete_vif_from_mrt	(vifi_t vifi);
extern mrtentry_t *switch_shortest_path	(uint32_t source, uint32_t group);
extern void	age_routes		(void);

/* routesock.c and netlink.c */
extern int	init_routesock		(void);
extern void     routesock_clean         (void);
extern int	k_req_incoming		(uint32_t source, struct rpfctl *rpfp);
extern int	routing_socket;

/* rp.c */
extern void	init_rp_and_bsr		(void);
extern uint16_t	bootstrap_initial_delay (void);
extern rp_grp_entry_t *add_rp_grp_entry (cand_rp_t  **used_cand_rp_list,
                                         grp_mask_t **used_grp_mask_list,
                                         uint32_t rp_addr,
                                         uint8_t  rp_priority,
                                         uint16_t rp_holdtime,
                                         uint32_t group_addr,
                                         uint32_t group_mask,
                                         uint32_t bsr_hash_mask,
                                         uint16_t fragment_tag);
extern void	delete_rp_grp_entry	(cand_rp_t  **used_cand_rp_list, grp_mask_t **used_grp_mask_list,
                                         rp_grp_entry_t *rp_grp_entry_delete);
extern void	delete_grp_mask		(cand_rp_t  **used_cand_rp_list, grp_mask_t **used_grp_mask_list,
                                         uint32_t group_addr, uint32_t group_mask);
extern void	delete_rp		(cand_rp_t  **used_cand_rp_list, grp_mask_t **used_grp_mask_list,
                                         uint32_t rp_addr);
extern void	delete_rp_list		(cand_rp_t  **used_cand_rp_list, grp_mask_t **used_grp_mask_list);
extern rpentry_t *rp_match		(uint32_t group);
extern rp_grp_entry_t *rp_grp_match	(uint32_t group);
extern rpentry_t *rp_find		(uint32_t rp_address);
extern int	remap_grpentry		(grpentry_t *grpentry_ptr);
extern int	create_pim_bootstrap_message (char *send_buff);
extern int	check_mrtentry_rp	(mrtentry_t *mrtentry_ptr, uint32_t rp_addr);
extern int	check_group_rp		(uint32_t group, uint32_t rp_addr);
extern void	age_misc		(void);

#ifdef RSRR
#ifdef PIM
#define gtable				mrtentry
#endif /* PIM */
#define RSRR_NOTIFICATION_OK		TRUE
#define RSRR_NOTIFICATION_FALSE		FALSE

/* rsrr.c */
extern void	rsrr_init		(void);
extern void	rsrr_clean		(void);
extern void	rsrr_cache_send		(struct gtable *, int);
extern void	rsrr_cache_clean	(struct gtable *);
extern void	rsrr_cache_bring_up	(struct gtable *);
#endif /* RSRR */

/* trace.c */
/* u_int is promoted uint8_t */
extern void	accept_mtrace		(uint32_t src, uint32_t dst, uint32_t group, char *data, u_int no, int datalen);
extern void	accept_neighbor_request	(uint32_t src, uint32_t dst);
extern void	accept_neighbor_request2 (uint32_t src, uint32_t dst);

/* vif.c */
extern void	init_vifs		(void);
extern void	zero_vif		(struct uvif *, int);
extern void	stop_all_vifs		(void);
extern void	check_vif_state		(void);
extern vifi_t	local_address		(uint32_t src);
extern vifi_t	find_vif		(int ifi);
extern vifi_t	find_vif_direct		(uint32_t src);
extern vifi_t	find_vif_direct_local	(uint32_t src);
extern uint32_t	max_local_address	(void);
extern void	age_vifs		(void);

struct rp_hold {
	struct rp_hold *next;
	uint32_t	address;
	uint32_t	group;
	uint32_t	mask;
	uint8_t		priority;
};

/* compat declarations */
#ifndef strlcpy
extern size_t	strlcpy			(char *, const char *, size_t);
#endif

#ifndef strlcat
extern size_t	strlcat			(char *, const char *, size_t);
#endif

#ifndef strtonum
extern long long strtonum		(const char *, long long, long long, const char **);
#endif

#ifndef pidfile
extern int	pidfile			(const char *);
#endif

#ifndef tempfile
extern FILE 	*tempfile		(void);
#endif

#endif /* PIMD_DEFS_H_ */

/**
 * Local Variables:
 *  indent-tabs-mode: t
 *  c-file-style: "cc-mode"
 * End:
 */
