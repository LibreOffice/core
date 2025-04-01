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
#pragma once


#ifdef __cplusplus
#endif
#include <sys/socket.h>
 672
 0xFFFF
 (HZ * 40)
 0x01
 0x02
 0x03
 0x0001
 0x0002
 0x0004
 0x0008
 0x0010
 0x0020
 0x0040
 0x01
 0x02
 0x03
 0x04
 0x05
 0x06
 0x07
 0x08
 0x09
 0x0a
 0x0b
 4
 4
 2
 4
 8
 0x0000
 0x0001
 0x0002
 0x0003
 0x0004
 0x0000
 0x0001
 0x0002
 4
 6
 0x0000
 0x0001
 0x0002
 0x0003
 2
 0x01
 0x02
 0x03
 0x04
 0x04
 22
 0x00
 0x01
 0x02
 4
 4
 2
 4
 0x0001
 0x0002
 0x0000
 0x0001
#ifdef __cplusplus
#endif
struct sockaddr_l2
{
    sa_family_t l2_family;
    unsigned short l2_psm;
    bdaddr_t l2_bdaddr;
};
#endif
