/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: regkey.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_REGISTRY_SOURCE_REGKEY_HXX
#define INCLUDED_REGISTRY_SOURCE_REGKEY_HXX

#include "sal/config.h"
#include "registry/regtype.h"
#include "rtl/ustring.h"
#include "sal/types.h"

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
