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
bool WildCard::ImpMatch( const char *pWild, const char *pStr )
{
    int    pos=0;
    int    flag=0;

    while ( *pWild || flag )
    {
        switch (*pWild)
        {
            case '?':
                if ( *pStr == '\0' )
                    return false;
                break;

            default:
                if ( (*pWild == '\\') && ((*(pWild+1)=='?') || (*(pWild+1) == '*')) )
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
                SAL_FALLTHROUGH;
            case '*':
                while ( *pWild == '*' )
                    pWild++;
                if ( *pWild == '\0' )
                    return true;
                flag = 1;
                pos  = 0;
                if ( *pStr == '\0' )
                    return ( *pWild == '\0' );
                while ( *pStr && *pStr != *pWild )
                {
                    if ( *pWild == '?' ) {
                        pWild++;
                        while ( *pWild == '*' )
                            pWild++;
                    }
                    pStr++;
                    if ( *pStr == '\0' )
                        return ( *pWild == '\0' );
                }
                break;
        }
        if ( *pWild != '\0' )
            pWild++;
        if ( *pStr != '\0' )
            pStr++;
        else
            flag = 0;
        if ( flag )
            pos--;
    }
    return ( *pStr == '\0' ) && ( *pWild == '\0' );
}

bool WildCard::Matches( const OUString& rString ) const
{
    OString aTmpWild = aWildString;
    OString aString(OUStringToOString(rString, osl_getThreadTextEncoding()));

    sal_Int32 nSepPos;

    if ( cSepSymbol != '\0' )
    {
        while ( (nSepPos = aTmpWild.indexOf(cSepSymbol)) != -1 )
        {
            // Check all splitted wildcards
            if ( ImpMatch( aTmpWild.copy( 0, nSepPos ).getStr(), aString.getStr() ) )
                return true;
            aTmpWild = aTmpWild.copy(nSepPos + 1); // remove separator
        }
    }

    if ( ImpMatch( aTmpWild.getStr(), aString.getStr() ) )
        return true;
    else
        return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
