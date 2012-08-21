/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Linux kernel header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to libc.  It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef __L2CAP_H
#define __L2CAP_H

#ifdef __cplusplus
#endif
#include <sys/socket.h>
#define L2CAP_DEFAULT_MTU 672
#define L2CAP_DEFAULT_FLUSH_TO 0xFFFF
#define L2CAP_CONN_TIMEOUT (HZ * 40)
#define L2CAP_OPTIONS 0x01
#define L2CAP_CONNINFO 0x02
#define L2CAP_LM 0x03
#define L2CAP_LM_MASTER 0x0001
#define L2CAP_LM_AUTH 0x0002
#define L2CAP_LM_ENCRYPT 0x0004
#define L2CAP_LM_TRUSTED 0x0008
#define L2CAP_LM_RELIABLE 0x0010
#define L2CAP_LM_SECURE 0x0020
#define L2CAP_LM_FLUSHABLE 0x0040
#define L2CAP_COMMAND_REJ 0x01
#define L2CAP_CONN_REQ 0x02
#define L2CAP_CONN_RSP 0x03
#define L2CAP_CONF_REQ 0x04
#define L2CAP_CONF_RSP 0x05
#define L2CAP_DISCONN_REQ 0x06
#define L2CAP_DISCONN_RSP 0x07
#define L2CAP_ECHO_REQ 0x08
#define L2CAP_ECHO_RSP 0x09
#define L2CAP_INFO_REQ 0x0a
#define L2CAP_INFO_RSP 0x0b
#define L2CAP_HDR_SIZE 4
#define L2CAP_CMD_HDR_SIZE 4
#define L2CAP_CMD_REJ_SIZE 2
#define L2CAP_CONN_REQ_SIZE 4
#define L2CAP_CONN_RSP_SIZE 8
#define L2CAP_CR_SUCCESS 0x0000
#define L2CAP_CR_PEND 0x0001
#define L2CAP_CR_BAD_PSM 0x0002
#define L2CAP_CR_SEC_BLOCK 0x0003
#define L2CAP_CR_NO_MEM 0x0004
#define L2CAP_CS_NO_INFO 0x0000
#define L2CAP_CS_AUTHEN_PEND 0x0001
#define L2CAP_CS_AUTHOR_PEND 0x0002
#define L2CAP_CONF_REQ_SIZE 4
#define L2CAP_CONF_RSP_SIZE 6
#define L2CAP_CONF_SUCCESS 0x0000
#define L2CAP_CONF_UNACCEPT 0x0001
#define L2CAP_CONF_REJECT 0x0002
#define L2CAP_CONF_UNKNOWN 0x0003
#define L2CAP_CONF_OPT_SIZE 2
#define L2CAP_CONF_MTU 0x01
#define L2CAP_CONF_FLUSH_TO 0x02
#define L2CAP_CONF_QOS 0x03
#define L2CAP_CONF_RFC 0x04
#define L2CAP_CONF_RFC_MODE 0x04
#define L2CAP_CONF_MAX_SIZE 22
#define L2CAP_MODE_BASIC 0x00
#define L2CAP_MODE_RETRANS 0x01
#define L2CAP_MODE_FLOWCTL 0x02
#define L2CAP_DISCONN_REQ_SIZE 4
#define L2CAP_DISCONN_RSP_SIZE 4
#define L2CAP_INFO_REQ_SIZE 2
#define L2CAP_INFO_RSP_SIZE 4
#define L2CAP_IT_CL_MTU 0x0001
#define L2CAP_IT_FEAT_MASK 0x0002
#define L2CAP_IR_SUCCESS 0x0000
#define L2CAP_IR_NOTSUPP 0x0001
#ifdef __cplusplus
#endif
struct sockaddr_l2
{
  sa_family_t l2_family;
  unsigned short l2_psm;
  bdaddr_t l2_bdaddr;
};
#endif
