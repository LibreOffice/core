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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"
#include <tools/wldcrd.hxx>

/*************************************************************************
|*
|*    WildCard::Match()
|*
|*    Beschreibung      WLDCRD.SDW
|*    Ersterstellung    MA 19.06.91
|*    Letzte Aenderung  MA 03.07.91
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
|*    Beschreibung      WLDCRD.SDW
|*    Ersterstellung    MA 19.06.91
|*    Letzte Aenderung  TH 02.02.96
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
