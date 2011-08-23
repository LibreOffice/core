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

#include <ctype.h>
#include <stdio.h>
#include <tools/string.hxx>
#include <com/sun/star/beans/PropertyValues.hpp>
#include <bf_svtools/ownlist.hxx>

using namespace com::sun::star;

namespace binfilter
{

//=========================================================================
//============== SvCommandList ============================================
//=========================================================================
PRV_SV_IMPL_OWNER_LIST(SvCommandList,SvCommand)

//=========================================================================
SvCommand & SvCommandList::Append
(
    const String & rCommand,	/* das Kommando	*/
    const String & rArg			/* dasArgument des Kommandos */
)
/*	[Beschreibung]

    Es wird eine Objekt vom Typ SvCommand erzeugt und an die Liste
    angeh"angt.

    [R"uckgabewert]

    SvCommand &		Das erteugte Objekt wird zur"uckgegeben.
*/
{
    SvCommand * pCmd = new SvCommand( rCommand, rArg );
    aTypes.Insert( pCmd, LIST_APPEND );
    return *pCmd;
}

//=========================================================================
SvStream & operator >>
(
    SvStream & rStm,     	/* Stream aus dem gelesen wird */
    SvCommandList & rThis	/* Die zu f"ullende Liste */
)
/*	[Beschreibung]

    Die Liste mit ihren Elementen wird gelesen. Das Format ist:
    1. Anzahl der Elemente
    2. Alle Elemente

    [R"uckgabewert]

    SvStream &		Der "ubergebene Stream.
*/
{
    UINT32 nCount = 0;
    rStm >> nCount;
    if( !rStm.GetError() )
    {
        while( nCount-- )
        {
            SvCommand * pCmd = new SvCommand();
            rStm >> *pCmd;
            rThis.aTypes.Insert( pCmd, LIST_APPEND );
        }
    }
    return rStm;
}

//=========================================================================
SvStream & operator <<
(
    SvStream & rStm,     		/* Stream in den geschrieben wird */
    const SvCommandList & rThis	/* Die zu schreibende Liste */
)
/*	[Beschreibung]

    Die Liste mit ihren Elementen wir geschrieben. Das Format ist:
    1. Anzahl der Elemente
    2. Alle Elemente

    [R"uckgabewert]

    SvStream &		Der "ubergebene Stream.
*/
{
    UINT32 nCount = rThis.aTypes.Count();
    rStm << nCount;

    for( UINT32 i = 0; i < nCount; i++ )
    {
        SvCommand * pCmd = (SvCommand *)rThis.aTypes.GetObject( i );
        rStm << *pCmd;
    }
    return rStm;
}

}
