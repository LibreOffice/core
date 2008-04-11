/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: strhelper.hxx,v $
 * $Revision: 1.4 $
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

#include <tools/string.hxx>
#include <rtl/math.hxx>
#include <cstring>

namespace psp {

String GetCommandLineToken( int, const String& );
ByteString GetCommandLineToken( int, const ByteString& );
// gets one token of a unix command line style string
// doublequote, singlequote and singleleftquote protect their respective
// contents

int GetCommandLineTokenCount( const String& );
int GetCommandLineTokenCount( const ByteString& );
// returns number of tokens (zero if empty or whitespace only)

String WhitespaceToSpace( const String&, BOOL bProtect = TRUE );
ByteString WhitespaceToSpace( const ByteString&, BOOL bProtect = TRUE );
// returns a string with multiple adjacent occurences of whitespace
// converted to a single space. if bProtect is TRUE (nonzero), then
// doublequote, singlequote and singleleftquote protect their respective
// contents


// parses the first double in the string; decimal is '.' only
inline double StringToDouble( const String& rStr )
{
    rtl_math_ConversionStatus eStatus;
    return rtl::math::stringToDouble( rStr, sal_Unicode('.'), sal_Unicode(0), &eStatus, NULL);
}

inline double StringToDouble( const ByteString& rStr )
{
    rtl_math_ConversionStatus eStatus;
    return rtl::math::stringToDouble( rtl::OStringToOUString( rStr, osl_getThreadTextEncoding() ), sal_Unicode('.'), sal_Unicode(0), &eStatus, NULL);
}

// fills a character buffer with the string representation of a double
// the buffer has to be long enough (e.g. 128 bytes)
// returns the string len
inline int getValueOfDouble( char* pBuffer, double f, int nPrecision = 0)
{
    rtl::OString aStr( rtl::math::doubleToString( f, rtl_math_StringFormat_G, nPrecision, '.', true ) );
    int nLen = aStr.getLength();
    strncpy( pBuffer, aStr.getStr(), nLen+1 ); // copy string including terminating zero
    return nLen;
}

} // namespace

#endif // _PSPRINT_STRHELPER_HXX_
