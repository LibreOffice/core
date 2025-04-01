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
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>
#pragma once
 31
 AF_BLUETOOTH
#endif
#pragma once
 274
#endif
 0
 1
 2
 3
 4
 5
 6
 7
 0
 6
 17
 18
#if __BYTE_ORDER == __LITTLE_ENDIAN
(d) (d)
(d) (d)
(d) (d)
(d) (d)
#elif __BYTE_ORDER == __BIG_ENDIAN
(d) bswap_16(d)
(d) bswap_32(d)
(d) bswap_16(d)
(d) bswap_32(d)
#else
#error "Unknown byte order"
#endif
(ptr)  ({   struct __attribute__((packed)) {   typeof(*(ptr)) __v;   } *__p = (void *) (ptr);   __p->__v;  })
(val, ptr)  do {   struct __attribute__((packed)) {   typeof(*(ptr)) __v;   } *__p = (void *) (ptr);   __p->__v = (val);  } while(0)
 (&(bdaddr_t) {{0, 0, 0, 0, 0, 0}})
 (&(bdaddr_t) {{0xff, 0xff, 0xff, 0xff, 0xff, 0xff}})
 (&(bdaddr_t) {{0, 0, 0, 0xff, 0xff, 0xff}})
#ifdef __cplusplus
#endif
typedef struct
{
  uint8_t b[6];
} __attribute__ ((packed)) bdaddr_t;
static inline void
bacpy (bdaddr_t * dst, const bdaddr_t * src)
{
  memcpy (dst, src, sizeof (bdaddr_t));
}
#endif
