/*************************************************************************
 *
 *  $RCSfile: wldcrd.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:03:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

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
