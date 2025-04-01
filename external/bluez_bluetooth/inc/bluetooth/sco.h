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
 500
 0xFFFF
 (HZ * 40)
 (HZ * 2)
 (HZ * 60)
 0x01
 0x02
#ifdef __cplusplus
#endif
struct sockaddr_sco
{
    sa_family_t sco_family;
    bdaddr_t sco_bdaddr;
    uint16_t sco_pkt_type;
};
#endif
