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
#ifndef __HCI_LIB_H
#define __HCI_LIB_H

#ifdef __cplusplus
#endif
#ifdef __cplusplus
#endif
static inline int
hci_test_bit (int nr, void *addr)
{
  return *((uint32_t *) addr + (nr >> 5)) & (1 << (nr & 31));
}
#endif
