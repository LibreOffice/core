/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: iniadrtk.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 15:16:18 $
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
#include "precompiled_svtools.hxx"

#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

#ifndef SVTOOLS_INIADRTK_HXX
#include <iniadrtk.hxx>
#endif

//============================================================================
//
//  class SfxIniManagerAddressEntry
//
//============================================================================

// static
String SfxIniManagerAddressEntry::get(const String & rAddress,
                                      AddressToken eToken)
{
    String aToken;
    USHORT i = 0, nTxt = 0;

    while ( i < rAddress.Len() )
    {
        while ( i < rAddress.Len() && rAddress.GetChar(i) != '#' )
        {
            if ( rAddress.GetChar(i) == '\\' )
                i++;
            aToken += rAddress.GetChar(i++);
        }

        // rAddress[i] == '#' oder am Ende, also eine Position weiter gehen
        i++;

        if ( eToken == (AddressToken)nTxt )
            break;
        else if ( i >= rAddress.Len() )
        {
            aToken.Erase();
            break;
        }
        else
        {
            aToken.Erase();
            nTxt++;
        }
    }
    return aToken;
}

