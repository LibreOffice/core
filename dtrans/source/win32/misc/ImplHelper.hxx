/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ImplHelper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 06:07:41 $
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


#ifndef _IMPLHELPER_HXX_
#define _IMPLHELPER_HXX_

//------------------------------------------------------------------------
// includes
//------------------------------------------------------------------------

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

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
