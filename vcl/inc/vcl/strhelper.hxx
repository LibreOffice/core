/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#ifndef _PSPRINT_STRHELPER_HXX_
#define _PSPRINT_STRHELPER_HXX_

#include "vcl/dllapi.h"

#include <tools/string.hxx>
#include <rtl/math.hxx>

#include <cstring>

namespace psp {

String VCL_DLLPUBLIC GetCommandLineToken( int, const String& );
ByteString VCL_DLLPUBLIC GetCommandLineToken( int, const ByteString& );
// gets one token of a unix command line style string
// doublequote, singlequote and singleleftquote protect their respective
// contents

int VCL_DLLPUBLIC GetCommandLineTokenCount( const String& );
int VCL_DLLPUBLIC GetCommandLineTokenCount( const ByteString& );
// returns number of tokens (zero if empty or whitespace only)

String VCL_DLLPUBLIC WhitespaceToSpace( const String&, sal_Bool bProtect = sal_True );
ByteString VCL_DLLPUBLIC WhitespaceToSpace( const ByteString&, sal_Bool bProtect = sal_True );
// returns a string with multiple adjacent occurrences of whitespace
// converted to a single space. if bProtect is sal_True (nonzero), then
// doublequote, singlequote and singleleftquote protect their respective
// contents


// parses the first double in the string; decimal is '.' only
inline double VCL_DLLPUBLIC StringToDouble( const String& rStr )
{
    rtl_math_ConversionStatus eStatus;
    return rtl::math::stringToDouble( rStr, sal_Unicode('.'), sal_Unicode(0), &eStatus, NULL);
}

inline double VCL_DLLPUBLIC StringToDouble( const ByteString& rStr )
{
    rtl_math_ConversionStatus eStatus;
    return rtl::math::stringToDouble( rtl::OStringToOUString( rStr, osl_getThreadTextEncoding() ), sal_Unicode('.'), sal_Unicode(0), &eStatus, NULL);
}

// fills a character buffer with the string representation of a double
// the buffer has to be long enough (e.g. 128 bytes)
// returns the string len
inline int VCL_DLLPUBLIC getValueOfDouble( char* pBuffer, double f, int nPrecision = 0)
{
    rtl::OString aStr( rtl::math::doubleToString( f, rtl_math_StringFormat_G, nPrecision, '.', true ) );
    int nLen = aStr.getLength();
    std::strncpy( pBuffer, aStr.getStr(), nLen+1 ); // copy string including terminating zero
    return nLen;
}

} // namespace

#endif // _PSPRINT_STRHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
