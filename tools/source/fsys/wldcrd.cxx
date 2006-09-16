/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: wldcrd.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:54:37 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_tools.hxx"

#ifndef _WLDCRD_HXX
#include <wldcrd.hxx>
#endif

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

USHORT WildCard::ImpMatch( const char *pWild, const char *pStr ) const
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

BOOL WildCard::Matches( const String& rString ) const
{
    ByteString aTmpWild = aWildString;
    ByteString aString(rString, osl_getThreadTextEncoding());

    USHORT  nSepPos;

    if ( cSepSymbol != '\0' )
    {
        while ( (nSepPos = aTmpWild.Search( cSepSymbol )) != STRING_NOTFOUND )
        {
            // alle getrennten WildCard's pruefen
            if ( ImpMatch( aTmpWild.Copy( 0, nSepPos ).GetBuffer(), aString.GetBuffer() ) )
                return TRUE;
            aTmpWild.Erase( 0, nSepPos + 1 ); // Trennsymbol entfernen
        }
        // und noch den hinter dem letzen Trennsymbol bzw. den einzigen
    }

    if ( ImpMatch( aTmpWild.GetBuffer(), aString.GetBuffer() ) )
        return TRUE;
    else
        return FALSE;
}
