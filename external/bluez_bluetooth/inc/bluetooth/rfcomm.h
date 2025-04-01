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
#pragma once


#ifdef __cplusplus
#endif
#include <sys/socket.h>
 127
 3
 (HZ * 30)
 (HZ * 20)
 0x02
 0x03
 0x0001
 0x0002
 0x0004
 0x0008
 0x0010
 0x0020
 256
 _IOW('R', 200, int)
 _IOW('R', 201, int)
 _IOR('R', 210, int)
 _IOR('R', 211, int)
 0
 1
 2
 3
#ifdef __cplusplus
#endif
struct sockaddr_rc
{
    sa_family_t rc_family;
    bdaddr_t rc_bdaddr;
    uint8_t rc_channel;
};
#endif
