/*************************************************************************
 *
 *  $RCSfile: misc.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:52:28 $
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

#pragma hdrstop

#include "sfxtypes.hxx"

//--------------------------------------------------------------------

#if defined(DBG_UTIL) && ( defined(WNT) || defined(OS2) )
unsigned SfxStack::nLevel = 0;
#endif

//--------------------------------------------------------------------

String SearchAndReplace( const String &rSource,
                         const String &rToReplace,
                         const String &rReplacement )
{
    String aTarget( rSource );
    USHORT nPos = rSource.Search( rToReplace );
    if ( nPos != STRING_NOTFOUND )
    {
        aTarget.Erase( nPos, rToReplace.Len() );
        return aTarget.Insert( rReplacement, nPos );
    }
    return rSource;
}

//--------------------------------------------------------------------

String SfxStringEncode( const String &rSource, const char *pKey  )
{
    String aRet;
    String aCoded(rSource);
/*ASMUSS
    // codieren
    USHORT nCode = 0;
    for ( USHORT n1 = 0; n1 < aCoded.Len(); ++n1 )
    {
        aCoded[n1] ^= pKey[nCode];
        if ( !pKey[++nCode] )
            nCode = 0;
    }

    // als reine Buchstabenfolge darstellen
    for ( USHORT n2 = 0; n2 < aCoded.Len(); ++n2 )
    {
        aRet += (char) ( 'a' + ((aCoded[n2] & 0xF0) >> 4) );
        aRet += (char) ( 'a' +  (aCoded[n2] & 0xF) );
    }
*/
    return aRet;
}

//--------------------------------------------------------------------

String SfxStringDecode( const String &rSource, const char *pKey  )
{
    // Output-Buffer aufbereiten
    String aRet;/*ASMUSS
    aRet.AllocStrBuf( rSource.Len() / 2 );

    // Buchstabenfolge in normale Zeichen wandeln
    USHORT nTo = 0;
    for ( USHORT n1 = 0; n1 < rSource.Len(); n1 += 2)
        aRet[nTo++] = (unsigned char)
                        ( ( ( rSource[n1] - 'a' ) << 4 ) +
                          ( ( rSource[ USHORT(n1+1) ] - 'a' ) ) );

    // decodieren
    USHORT nCode = 0;
    for ( USHORT n2 = 0; n2 < aRet.Len(); ++n2 )
    {
        aRet[n2] ^= pKey[nCode];
        if ( !pKey[++nCode] )
            nCode = 0;
    }
*/
    return aRet;
}


