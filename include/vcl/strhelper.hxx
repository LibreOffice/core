/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef _PSPRINT_STRHELPER_HXX_
#define _PSPRINT_STRHELPER_HXX_

#include "vcl/dllapi.h"

#include <tools/string.hxx>
#include <rtl/math.hxx>

#include <cstring>

namespace psp
{
    VCL_DLLPUBLIC OUString GetCommandLineToken( int, const OUString& );
    VCL_DLLPUBLIC OString GetCommandLineToken(int, const OString&);
    // gets one token of a unix command line style string
    // doublequote, singlequote and singleleftquote protect their respective
    // contents

    VCL_DLLPUBLIC int GetCommandLineTokenCount(const OUString&);
    // returns number of tokens (zero if empty or whitespace only)

    VCL_DLLPUBLIC OUString WhitespaceToSpace( const OUString&, bool bProtect = true );
    VCL_DLLPUBLIC OString WhitespaceToSpace(const OString&, bool bProtect = true);
    // returns a string with multiple adjacent occurrences of whitespace
    // converted to a single space. if bProtect is sal_True (nonzero), then
    // doublequote, singlequote and singleleftquote protect their respective
    // contents


    // parses the first double in the string; decimal is '.' only
    VCL_DLLPUBLIC inline double StringToDouble( const OUString& rStr )
    {
        return rtl::math::stringToDouble(rStr, sal_Unicode('.'), sal_Unicode(0));
    }

    VCL_DLLPUBLIC inline double StringToDouble(const OString& rStr)
    {
        return rtl::math::stringToDouble(rStr, '.', static_cast<sal_Char>(0));
    }

    // fills a character buffer with the string representation of a double
    // the buffer has to be long enough (e.g. 128 bytes)
    // returns the string len
    VCL_DLLPUBLIC inline int getValueOfDouble( char* pBuffer, double f, int nPrecision = 0)
    {
        OString aStr( rtl::math::doubleToString( f, rtl_math_StringFormat_G, nPrecision, '.', true ) );
        int nLen = aStr.getLength();
        std::strncpy( pBuffer, aStr.getStr(), nLen+1 ); // copy string including terminating zero
        return nLen;
    }

} // namespace

#endif // _PSPRINT_STRHELPER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
