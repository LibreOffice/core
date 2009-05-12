/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ImplHelper.hxx,v $
 * $Revision: 1.8 $
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


#ifndef _IMPLHELPER_HXX_
#define _IMPLHELPER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#include <sal/types.h>
#include <rtl/ustring.hxx>

#if defined _MSC_VER
#pragma warning(push,1)
#endif
#include <windows.h>
#if defined _MSC_VER
#pragma warning(pop)
#endif

//------------------------------------------------------------------------
// deklarations
//------------------------------------------------------------------------

// target device and formatetc helper
void      SAL_CALL DeleteTargetDevice(DVTARGETDEVICE* ptd);
sal_Bool  SAL_CALL CopyFormatEtc(LPFORMATETC petcDest, LPFORMATETC petcSrc);
sal_Int32 SAL_CALL CompareFormatEtc( const FORMATETC* pFetcLeft, const FORMATETC* pFetcRight);
sal_Bool  SAL_CALL CompareTargetDevice(DVTARGETDEVICE* ptdLeft, DVTARGETDEVICE* ptdRight);
DVTARGETDEVICE* SAL_CALL CopyTargetDevice(DVTARGETDEVICE* ptdSrc);

// some codepage helper functions

//--------------------------------------------------
// returns a windows codepage appropriate to the
// given mime charset parameter value
//--------------------------------------------------

sal_uInt32 SAL_CALL getWinCPFromMimeCharset(
    const rtl::OUString& charset );

//--------------------------------------------------
// returns a windows codepage appropriate to the
// given locale and locale type
//--------------------------------------------------

rtl::OUString SAL_CALL getWinCPFromLocaleId(
    LCID lcid, LCTYPE lctype );

//--------------------------------------------------
// returns a mime charset parameter value appropriate
// to the given codepage, optional a prefix can be
// given, e.g. "windows-" or "cp"
//--------------------------------------------------

rtl::OUString SAL_CALL getMimeCharsetFromWinCP(
    sal_uInt32 cp, const rtl::OUString& aPrefix );

//--------------------------------------------------
// returns a mime charset parameter value appropriate
// to the given locale id and locale type, optional a
// prefix can be given, e.g. "windows-" or "cp"
//--------------------------------------------------

rtl::OUString SAL_CALL getMimeCharsetFromLocaleId(
    LCID lcid, LCTYPE lctype, const rtl::OUString& aPrefix  );

//-----------------------------------------------------
// returns true, if a given codepage is an oem codepage
//-----------------------------------------------------

sal_Bool SAL_CALL IsOEMCP( sal_uInt32 codepage );

//--------------------------------------------------
// converts a codepage into a string representation
//--------------------------------------------------

rtl::OUString SAL_CALL cptostr( sal_uInt32 codepage );

#endif
