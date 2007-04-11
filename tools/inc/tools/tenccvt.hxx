/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tenccvt.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 20:19:17 $
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
#ifndef _TOOLS_TENCCVT_HXX
#define _TOOLS_TENCCVT_HXX

#ifndef _RTL_TEXTENC_H
#include <rtl/textenc.h>
#endif
#ifndef _TOOLS_SOLAR_H
#include <tools/solar.h>
#endif
#ifndef INCLUDED_TOOLSDLLAPI_H
#include "tools/toolsdllapi.h"
#endif

// ----------------------------------------
// - Functions for handling Import/Export -
// ----------------------------------------

// return an encoding which has more defined Characters as the given
// encoding, but have the same definition for the defined characters
// e.g.: windows-1252 for iso-8859-1 or windows-1254 for iso-8859-9
TOOLS_DLLPUBLIC rtl_TextEncoding GetExtendedCompatibilityTextEncoding( rtl_TextEncoding eEncoding );

// return an encoding which has more defined Characters as the given
// encoding. The encodings could be different.
// e.g.: windows-1251 for iso-8859-5
TOOLS_DLLPUBLIC rtl_TextEncoding GetExtendedTextEncoding( rtl_TextEncoding eEncoding );

// if the given encoding is an multi-byte encoding (which allows more than
// one byte per char, e.g. UTF-8 or Shift-JIS), a one-byte encoding
// is returned (normally windows-1252).
TOOLS_DLLPUBLIC rtl_TextEncoding GetOneByteTextEncoding( rtl_TextEncoding eEncoding );

TOOLS_DLLPUBLIC rtl_TextEncoding GetSOLoadTextEncoding( rtl_TextEncoding eEncoding, USHORT nVersion = SOFFICE_FILEFORMAT_50 );
TOOLS_DLLPUBLIC rtl_TextEncoding GetSOStoreTextEncoding( rtl_TextEncoding eEncoding, USHORT nVersion = SOFFICE_FILEFORMAT_50 );

#endif  // _TOOLS_TENCCVT_HXX
