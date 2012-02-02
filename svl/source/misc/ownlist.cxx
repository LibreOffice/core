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

#include <ctype.h>
#include <stdio.h>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <svl/ownlist.hxx>

using namespace com::sun::star;

//=========================================================================
//============== SvCommandList ============================================
//=========================================================================

static ::rtl::OUString parseString(const ::rtl::OUString & rCmd, sal_Int32 * pIndex)
{
    ::rtl::OUString result;

    if(rCmd[*pIndex] == sal_Unicode('\"')) {
        (*pIndex) ++;

        sal_Int32 begin = *pIndex;

        while(*pIndex < rCmd.getLength() && rCmd[(*pIndex) ++] != sal_Unicode('\"')) ;

        result = rCmd.copy(begin, *pIndex - begin - 1);
    }

    return result;
}

static ::rtl::OUString parseWord(const ::rtl::OUString & rCmd, sal_Int32 * pIndex)
{
    sal_Int32 begin = *pIndex;

    while(*pIndex < rCmd.getLength()
          && !isspace(sal::static_int_cast<int>(rCmd[*pIndex]))
          && rCmd[*pIndex] != sal_Unicode('='))
        (*pIndex) ++;

    return rCmd.copy(begin, *pIndex - begin);
}

static void eatSpace(const ::rtl::OUString & rCmd, sal_Int32 * pIndex)
{
    while(*pIndex < rCmd.getLength() && isspace(sal::static_int_cast<int>(rCmd[*pIndex])))
        (*pIndex) ++;
}


//=========================================================================
bool SvCommandList::AppendCommands
(
 const ::rtl::OUString & rCmd,    /* Dieser Text wird in Kommandos umgesetzt */
 sal_Int32 * pEaten         /* Anzahl der Zeichen, die gelesen wurden */
)
/*  [Beschreibung]

    Es wird eine Text geparsed und die einzelnen Kommandos werden an
    die Liste angeh"angt.

    [R"uckgabewert]

    bool        true, der Text wurde korrekt geparsed.
                false, der Text wurde nicht korrekt geparsed.
*/
{
    sal_Int32 index = 0;
    while(index < rCmd.getLength())
    {

        eatSpace(rCmd, &index);
        ::rtl::OUString name = (rCmd[index] == sal_Unicode('\"')) ? parseString(rCmd, &index) : parseWord(rCmd, &index);

        eatSpace(rCmd, &index);
        ::rtl::OUString value;
        if(index < rCmd.getLength() && rCmd[index] == sal_Unicode('='))
        {
            index ++;

            eatSpace(rCmd, &index);
            value = (rCmd[index] == sal_Unicode('\"')) ? parseString(rCmd, &index) : parseWord(rCmd, &index);
        }

        aCommandList.push_back( SvCommand(name, value));
    }

    *pEaten = index;

    return true;
}

//=========================================================================
SvCommand & SvCommandList::Append
(
 const ::rtl::OUString & rCommand,    /* das Kommando */
 const ::rtl::OUString & rArg         /* dasArgument des Kommandos */
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

bool SvCommandList::FillFromSequence( const com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& aCommandSequence )
{
    const sal_Int32 nCount = aCommandSequence.getLength();
    ::rtl::OUString aCommand, aArg;
    ::rtl::OUString aApiArg;
    for( sal_Int32 nIndex=0; nIndex<nCount; nIndex++ )
    {
        aCommand = aCommandSequence[nIndex].Name;
        if( !( aCommandSequence[nIndex].Value >>= aApiArg ) )
            return false;
        aArg = aApiArg;
        Append( aCommand, aArg );
    }

    return true;
}

void SvCommandList::FillSequence( com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue >& aCommandSequence )
{
    const sal_Int32 nCount = aCommandList.size();
    aCommandSequence.realloc( nCount );
    for( sal_Int32 nIndex = 0; nIndex < nCount; nIndex++ )
    {
        aCommandSequence[nIndex].Name = aCommandList[ nIndex ].GetCommand();
        aCommandSequence[nIndex].Handle = -1;
        aCommandSequence[nIndex].Value = uno::makeAny( aCommandList[ nIndex ].GetArgument() );
        aCommandSequence[nIndex].State = beans::PropertyState_DIRECT_VALUE;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
