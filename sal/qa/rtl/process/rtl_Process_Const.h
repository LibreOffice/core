/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

#ifndef _RTL_PROCESS_CONST_H_
#define _RTL_PROCESS_CONST_H_

//------------------------------------------------------------------------
#include <rtl/ustring.hxx>

using namespace ::rtl;

#ifdef __cplusplus
extern "C"
{
#endif
//------------------------------------------------------------------------
//::rtl::OUString suParam[4];
::rtl::OUString suParam0 = ::rtl::OUString::createFromAscii("-join");
::rtl::OUString suParam1 = OUString::createFromAscii("-with");
::rtl::OUString suParam2 = OUString::createFromAscii("-child");
::rtl::OUString suParam3 = OUString::createFromAscii("-process");

//------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif

#endif /* RTL_PROCESS_CONST_H*/

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
