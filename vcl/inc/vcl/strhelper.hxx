/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


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
