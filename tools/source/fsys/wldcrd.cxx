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

#include <tools/wldcrd.hxx>

/** Tests, whether a wildcard in pWild will match for pStr.
 *
 * '*' in pWild means n chars for n > 0.
 * '?' in pWild mean match exactly one character.
 *
 */
bool WildCard::ImpMatch( std::u16string_view aWild, std::u16string_view aStr )
{
    int    pos=0;
    int    flag=0;
    const sal_Unicode* pWild = aWild.data();
    const sal_Unicode* pWildEnd = aWild.data() + aWild.size();
    const sal_Unicode* pStr = aStr.data();
    const sal_Unicode* pStrEnd = aStr.data() + aStr.size();

    while ( pWild != pWildEnd || flag )
    {
        switch (*pWild)
        {
            case '?':
                if ( pStr == pStrEnd )
                    return false;
                break;

            default:
                if ( (*pWild == '\\') && (pWild + 1 != pWildEnd) && ((*(pWild+1)=='?') || (*(pWild+1) == '*')) )
                    pWild++;
                if ( *pWild != *pStr )
                    if ( !pos )
                        return false;
                    else
                        pWild += pos;
                else
                    break;
                // WARNING/TODO: may cause execution of next case in some
                // circumstances!
                [[fallthrough]];
            case '*':
                while ( pWild != pWildEnd && *pWild == '*' )
                    pWild++;
                if ( pWild == pWildEnd )
                    return true;
                flag = 1;
                pos  = 0;
                if ( pStr == pStrEnd )
                    return false;
                while ( pStr != pStrEnd && *pStr != *pWild )
                {
                    if ( *pWild == '?' ) {
                        pWild++;
                        while ( pWild != pWildEnd && *pWild == '*' )
                            pWild++;
                    }
                    pStr++;
                    if ( pStr == pStrEnd )
                        return pWild == pWildEnd;
                }
                break;
        }
        if ( pWild != pWildEnd )
            pWild++;
        if ( pStr != pStrEnd )
            pStr++;
        else
            flag = 0;
        if ( flag )
            pos--;
    }
    return ( pStr == pStrEnd ) && ( pWild == pWildEnd );
}

bool WildCard::Matches( std::u16string_view rString ) const
{
    OUString aTmpWild = aWildString;

    sal_Int32 nSepPos;

    if ( cSepSymbol != '\0' )
    {
        while ( (nSepPos = aTmpWild.indexOf(cSepSymbol)) != -1 )
        {
            // Check all split wildcards
            if ( ImpMatch( aTmpWild.subView( 0, nSepPos ), rString ) )
                return true;
            aTmpWild = aTmpWild.copy(nSepPos + 1); // remove separator
        }
    }

    return ImpMatch( aTmpWild, rString );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
