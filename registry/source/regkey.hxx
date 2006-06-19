/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: regkey.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 14:28:45 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef INCLUDED_REGISTRY_SOURCE_REGKEY_HXX
#define INCLUDED_REGISTRY_SOURCE_REGKEY_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef _REGISTRY_REGTYPE_H_
#include "registry/regtype.h"
#endif
#ifndef _RTL_USTRING_H_
#include "rtl/ustring.h"
#endif
#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

extern "C" {

void REGISTRY_CALLTYPE acquireKey(RegKeyHandle);
void REGISTRY_CALLTYPE releaseKey(RegKeyHandle);
sal_Bool REGISTRY_CALLTYPE isKeyReadOnly(RegKeyHandle);
RegError REGISTRY_CALLTYPE getKeyName(RegKeyHandle, rtl_uString**);
RegError REGISTRY_CALLTYPE createKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError REGISTRY_CALLTYPE openKey(RegKeyHandle, rtl_uString*, RegKeyHandle*);
RegError REGISTRY_CALLTYPE openSubKeys(
    RegKeyHandle, rtl_uString*, RegKeyHandle**, sal_uInt32*);
RegError REGISTRY_CALLTYPE closeSubKeys(RegKeyHandle*, sal_uInt32);
RegError REGISTRY_CALLTYPE deleteKey(RegKeyHandle, rtl_uString*);
RegError REGISTRY_CALLTYPE closeKey(RegKeyHandle);
RegError REGISTRY_CALLTYPE setValue(
    RegKeyHandle, rtl_uString*, RegValueType, RegValue, sal_uInt32);
RegError REGISTRY_CALLTYPE setLongListValue(
    RegKeyHandle, rtl_uString*, sal_Int32*, sal_uInt32);
RegError REGISTRY_CALLTYPE setStringListValue(
    RegKeyHandle, rtl_uString*, sal_Char**, sal_uInt32);
RegError REGISTRY_CALLTYPE setUnicodeListValue(
    RegKeyHandle, rtl_uString*, sal_Unicode**, sal_uInt32);
RegError REGISTRY_CALLTYPE getValueInfo(
    RegKeyHandle, rtl_uString*, RegValueType*, sal_uInt32*);
RegError REGISTRY_CALLTYPE getValue(RegKeyHandle, rtl_uString*, RegValue);
RegError REGISTRY_CALLTYPE getLongListValue(
    RegKeyHandle, rtl_uString*, sal_Int32**, sal_uInt32*);
RegError REGISTRY_CALLTYPE getStringListValue(
    RegKeyHandle, rtl_uString*, sal_Char***, sal_uInt32*);
RegError REGISTRY_CALLTYPE getUnicodeListValue(
    RegKeyHandle, rtl_uString*, sal_Unicode***, sal_uInt32*);
RegError REGISTRY_CALLTYPE freeValueList(RegValueType, RegValue, sal_uInt32);
RegError REGISTRY_CALLTYPE createLink(RegKeyHandle, rtl_uString*, rtl_uString*);
RegError REGISTRY_CALLTYPE deleteLink(RegKeyHandle, rtl_uString*);
RegError REGISTRY_CALLTYPE getKeyType(RegKeyHandle, rtl_uString*, RegKeyType*);
RegError REGISTRY_CALLTYPE getLinkTarget(
    RegKeyHandle, rtl_uString*, rtl_uString**);
RegError REGISTRY_CALLTYPE getResolvedKeyName(
    RegKeyHandle, rtl_uString*, sal_Bool, rtl_uString**);
RegError REGISTRY_CALLTYPE getKeyNames(
    RegKeyHandle, rtl_uString*, rtl_uString***, sal_uInt32*);
RegError REGISTRY_CALLTYPE freeKeyNames(rtl_uString**, sal_uInt32);

}

#endif
