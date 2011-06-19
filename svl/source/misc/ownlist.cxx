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
#include "precompiled_svl.hxx"
#include <ctype.h>
#include <stdio.h>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <svl/ownlist.hxx>

using namespace com::sun::star;

//=========================================================================
//============== SvCommandList ============================================
//=========================================================================

static String parseString(const String & rCmd, sal_uInt16 * pIndex)
{
    String result;

    if(rCmd.GetChar( *pIndex ) == '\"') {
        (*pIndex) ++;

        sal_uInt16 begin = *pIndex;

        while(*pIndex < rCmd.Len() && rCmd.GetChar((*pIndex) ++) != '\"') ;

        result = String(rCmd.Copy(begin, *pIndex - begin - 1));
    }

    return result;
}

static String parseWord(const String & rCmd, sal_uInt16 * pIndex)
{
    sal_uInt16 begin = *pIndex;

    while(*pIndex < rCmd.Len() && !isspace(rCmd.GetChar(*pIndex)) && rCmd.GetChar(*pIndex) != '=')
        (*pIndex) ++;

    return String(rCmd.Copy(begin, *pIndex - begin));
}

static void eatSpace(const String & rCmd, sal_uInt16 * pIndex)
{
    while(*pIndex < rCmd.Len() && isspace(rCmd.GetChar(*pIndex)))
        (*pIndex) ++;
}


//=========================================================================
sal_Bool SvCommandList::AppendCommands
(
    const String & rCmd,    /* Dieser Text wird in Kommandos umgesetzt */
    sal_uInt16 * pEaten         /* Anzahl der Zeichen, die gelesen wurden */
)
/*  [Beschreibung]

    Es wird eine Text geparsed und die einzelnen Kommandos werden an
    die Liste angeh"angt.

    [R"uckgabewert]

    sal_Bool        sal_True, der Text wurde korrekt geparsed.
                sal_False, der Text wurde nicht korrekt geparsed.
*/
{
    sal_uInt16 index = 0;
    while(index < rCmd.Len())
    {

        eatSpace(rCmd, &index);
        String name = (rCmd.GetChar(index) == '\"') ? parseString(rCmd, &index) : parseWord(rCmd, &index);

        eatSpace(rCmd, &index);
        String value;
        if(index < rCmd.Len() && rCmd.GetChar(index) == '=')
        {
            index ++;

            eatSpace(rCmd, &index);
            value = (rCmd.GetChar(index) == '\"') ? parseString(rCmd, &index) : parseWord(rCmd, &index);
        }

        aCommandList.push_back( SvCommand(name, value));
    }

    *pEaten = index;

    return sal_True;
}

//=========================================================================
String SvCommandList::GetCommands() const
/*  [Beschreibung]

    Die Kommandos in der Liste werden als Text hintereinander, durch ein
    Leerzeichen getrennt geschrieben. Der Text muss nicht genauso
    aussehen wie der in <SvCommandList::AppendCommands()> "ubergebene.

    [R"uckgabewert]

    String      Die Kommandos werden zur"uckgegeben.
*/
{
    String aRet;
    for( sal_uLong i = 0; i < aCommandList.size(); i++ )
    {
        if( i != 0 )
            aRet += ' ';
        SvCommand aCmd = aCommandList[ i ];
        aRet += aCmd.GetCommand();
        if( aCmd.GetArgument().Len() )
        {
            aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "=\"" ) );
            aRet += aCmd.GetArgument();
            aRet.AppendAscii( RTL_CONSTASCII_STRINGPARAM( "\"" ) );
        }
    }
    return aRet;
}

//=========================================================================
SvCommand & SvCommandList::Append
(
    const String & rCommand,    /* das Kommando */
    const String & rArg         /* dasArgument des Kommandos */
)
/*  [Beschreibung]

    Es wird eine Objekt vom Typ SvCommand erzeugt und an die Liste
    angeh"angt.

    [R"uckgabewert]

    SvCommand &     Das erteugte Objekt wird zur"uckgegeben.
*/
{
    aCommandList.push_back( SvCommand( rCommand, rArg ) );
    return aCommandList.back();
}

//=========================================================================
SvStream & operator >>
(
    SvStream & rStm,        /* Stream aus dem gelesen wird */
    SvCommandList & rThis   /* Die zu f"ullende Liste */
)
/*  [Beschreibung]

    Die Liste mit ihren Elementen wird gelesen. Das Format ist:
    1. Anzahl der Elemente
    2. Alle Elemente

    [R"uckgabewert]

    SvStream &      Der "ubergebene Stream.
*/
{
    sal_uInt32 nCount = 0;
    rStm >> nCount;
    if( !rStm.GetError() )
    {
        while( nCount-- )
        {
            SvCommand aCmd;
            rStm >> aCmd;
            rThis.aCommandList.push_back( aCmd );
        }
    }
    return rStm;
}

//=========================================================================
SvStream & operator <<
(
    SvStream & rStm,            /* Stream in den geschrieben wird */
    const SvCommandList & rThis /* Die zu schreibende Liste */
)
/*  [Beschreibung]

    Die Liste mit ihren Elementen wir geschrieben. Das Format ist:
    1. Anzahl der Elemente
    2. Alle Elemente

    [R"uckgabewert]

    SvStream &      Der "ubergebene Stream.
*/
{
    sal_uInt32 nCount = rThis.aCommandList.size();
    rStm << nCount;

    for( sal_uInt32 i = 0; i < nCount; i++ )
    {
        rStm << rThis.aCommandList[ i ];
    }
    return rStm;
}

sal_Bool SvCommandList::FillFromSequence( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& aCommandSequence )
{
    const sal_Int32 nCount = aCommandSequence.getLength();
    String aCommand, aArg;
    ::rtl::OUString aApiArg;
    for( sal_Int32 nIndex=0; nIndex<nCount; nIndex++ )
    {
        aCommand = aCommandSequence[nIndex].Name;
        if( !( aCommandSequence[nIndex].Value >>= aApiArg ) )
            return sal_False;
        aArg = aApiArg;
        Append( aCommand, aArg );
    }

    return sal_True;
}

void SvCommandList::FillSequence( com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& aCommandSequence )
{
    const sal_Int32 nCount = aCommandList.size();
    aCommandSequence.realloc( nCount );
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        aCommandSequence[nIndex].Name = aCommandList[ nIndex ].GetCommand();
        aCommandSequence[nIndex].Handle = -1;
        aCommandSequence[nIndex].Value = uno::makeAny( ::rtl::OUString( aCommandList[ nIndex ].GetArgument() ) );
        aCommandSequence[nIndex].State = beans::PropertyState_DIRECT_VALUE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
