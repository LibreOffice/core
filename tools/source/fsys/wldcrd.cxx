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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <tools/wldcrd.hxx>

/*************************************************************************
|*
|*    WildCard::Match()
|*
*************************************************************************/

/* Diese Methode ueberprueft, ob die Wilde Karte in pWild mit dem String
 * in pStr matscht.
 * Vertragen sich die beiden, so wird 1 zurueckgegeben, sonst 0.
 *
 * ein '*' in pWild bedeutet n beliebige Zeichen, mit n>=0
 * ein '?' in pWild bedeutet genau ein beliebiges Zeichen
 *
 */

sal_uInt16 WildCard::ImpMatch( const char *pWild, const char *pStr ) const
{
    int    pos=0;
    int    flag=0;

    while ( *pWild || flag )
    {
        switch (*pWild)
        {
            case '?':
                if ( *pStr == '\0' )
                    return 0;
                break;

            default:
                if ( (*pWild == '\\') && ((*(pWild+1)=='?') || (*(pWild+1) == '*')) )
                    pWild++;
                if ( *pWild != *pStr )
                    if ( !pos )
                        return 0;
                    else
                        pWild += pos;
                else
                    break;          // ACHTUNG laeuft unter bestimmten
                                    // Umstaenden in den nachsten case rein!!
            case '*':
                while ( *pWild == '*' )
                    pWild++;
                if ( *pWild == '\0' )
                    return 1;
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

/*************************************************************************
|*
|*    WildCard::Matches()
|*
*************************************************************************/

sal_Bool WildCard::Matches( const String& rString ) const
{
    ByteString aTmpWild = aWildString;
    ByteString aString(rString, osl_getThreadTextEncoding());

    sal_uInt16  nSepPos;

    if ( cSepSymbol != '\0' )
    {
        while ( (nSepPos = aTmpWild.Search( cSepSymbol )) != STRING_NOTFOUND )
        {
            // alle getrennten WildCard's pruefen
            if ( ImpMatch( aTmpWild.Copy( 0, nSepPos ).GetBuffer(), aString.GetBuffer() ) )
                return sal_True;
            aTmpWild.Erase( 0, nSepPos + 1 ); // Trennsymbol entfernen
        }
        // und noch den hinter dem letzen Trennsymbol bzw. den einzigen
    }

    if ( ImpMatch( aTmpWild.GetBuffer(), aString.GetBuffer() ) )
        return sal_True;
    else
        return sal_False;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
