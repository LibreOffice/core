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
#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#ifdef __cplusplus
#endif
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>
#ifndef AF_BLUETOOTH
#define AF_BLUETOOTH 31
#define PF_BLUETOOTH AF_BLUETOOTH
#endif
#ifndef SOL_BLUETOOTH
#define SOL_BLUETOOTH 274
#endif
#define BTPROTO_L2CAP 0
#define BTPROTO_HCI 1
#define BTPROTO_SCO 2
#define BTPROTO_RFCOMM 3
#define BTPROTO_BNEP 4
#define BTPROTO_CMTP 5
#define BTPROTO_HIDP 6
#define BTPROTO_AVDTP 7
#define SOL_HCI 0
#define SOL_L2CAP 6
#define SOL_SCO 17
#define SOL_RFCOMM 18
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define htobs(d) (d)
#define htobl(d) (d)
#define btohs(d) (d)
#define btohl(d) (d)
#elif __BYTE_ORDER == __BIG_ENDIAN
#define htobs(d) bswap_16(d)
#define htobl(d) bswap_32(d)
#define btohs(d) bswap_16(d)
#define btohl(d) bswap_32(d)
#else
#error "Unknown byte order"
#endif
#define bt_get_unaligned(ptr)  ({   struct __attribute__((packed)) {   typeof(*(ptr)) __v;   } *__p = (void *) (ptr);   __p->__v;  })
#define bt_put_unaligned(val, ptr)  do {   struct __attribute__((packed)) {   typeof(*(ptr)) __v;   } *__p = (void *) (ptr);   __p->__v = (val);  } while(0)
#define BDADDR_ANY (&(bdaddr_t) {{0, 0, 0, 0, 0, 0}})
#define BDADDR_ALL (&(bdaddr_t) {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}})
#define BDADDR_LOCAL (&(bdaddr_t) {{0, 0, 0, 0xff, 0xff, 0xff}})
#ifdef __cplusplus
#endif
typedef struct {
    uint8_t b[6];
} __attribute__((packed)) bdaddr_t;
static inline void bacpy(bdaddr_t *dst, const bdaddr_t *src)
{
    memcpy(dst, src, sizeof(bdaddr_t));
}
#endif
