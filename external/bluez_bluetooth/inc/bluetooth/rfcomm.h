/****************************************************************************
 ****************************************************************************
 ***
 ***   This header was automatically generated from a Bluez header
 ***   of the same name, to make information necessary for userspace to
 ***   call into the kernel available to Android. It contains only constants,
 ***   structures, and macros generated from the original header, and thus,
 ***   contains no copyrightable information.
 ***
 ****************************************************************************
 ****************************************************************************/
#ifndef __RFCOMM_H
#define __RFCOMM_H

#ifdef __cplusplus
#endif
#include <sys/socket.h>
#define RFCOMM_DEFAULT_MTU 127
#define RFCOMM_PSM 3
#define RFCOMM_CONN_TIMEOUT (HZ * 30)
#define RFCOMM_DISC_TIMEOUT (HZ * 20)
#define RFCOMM_CONNINFO 0x02
#define RFCOMM_LM 0x03
#define RFCOMM_LM_MASTER 0x0001
#define RFCOMM_LM_AUTH 0x0002
#define RFCOMM_LM_ENCRYPT 0x0004
#define RFCOMM_LM_TRUSTED 0x0008
#define RFCOMM_LM_RELIABLE 0x0010
#define RFCOMM_LM_SECURE 0x0020
#define RFCOMM_MAX_DEV 256
#define RFCOMMCREATEDEV _IOW('R', 200, int)
#define RFCOMMRELEASEDEV _IOW('R', 201, int)
#define RFCOMMGETDEVLIST _IOR('R', 210, int)
#define RFCOMMGETDEVINFO _IOR('R', 211, int)
#define RFCOMM_REUSE_DLC 0
#define RFCOMM_RELEASE_ONHUP 1
#define RFCOMM_HANGUP_NOW 2
#define RFCOMM_TTY_ATTACHED 3
#ifdef __cplusplus
#endif
struct sockaddr_rc
{
  sa_family_t rc_family;
  bdaddr_t rc_bdaddr;
  uint8_t rc_channel;
};
#endif
