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

#include <ctype.h>
#include <stdio.h>
#include <com/sun/star/beans/PropertyValues.hpp>

#include <svl/ownlist.hxx>

using namespace com::sun::star;


static OUString parseString(const OUString & rCmd, sal_Int32 * pIndex)
{
    OUString result;

    if(rCmd[*pIndex] == sal_Unicode('\"')) {
        (*pIndex) ++;

        sal_Int32 begin = *pIndex;

        while(*pIndex < rCmd.getLength() && rCmd[(*pIndex) ++] != sal_Unicode('\"')) ;

        result = rCmd.copy(begin, *pIndex - begin - 1);
    }

    return result;
}

static OUString parseWord(const OUString & rCmd, sal_Int32 * pIndex)
{
    sal_Int32 begin = *pIndex;

    while(*pIndex < rCmd.getLength()
          && !isspace(sal::static_int_cast<int>(rCmd[*pIndex]))
          && rCmd[*pIndex] != sal_Unicode('='))
        (*pIndex) ++;

    return rCmd.copy(begin, *pIndex - begin);
}

static void eatSpace(const OUString & rCmd, sal_Int32 * pIndex)
{
    while(*pIndex < rCmd.getLength() && isspace(sal::static_int_cast<int>(rCmd[*pIndex])))
        (*pIndex) ++;
}


bool SvCommandList::AppendCommands
(
 const OUString & rCmd,    /* Dieser Text wird in Kommandos umgesetzt */
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
        OUString name = (rCmd[index] == sal_Unicode('\"')) ? parseString(rCmd, &index) : parseWord(rCmd, &index);

        eatSpace(rCmd, &index);
        OUString value;
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

SvCommand & SvCommandList::Append
(
 const OUString & rCommand,    /* das Kommando */
 const OUString & rArg         /* dasArgument des Kommandos */
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
    OUString aCommand, aArg;
    OUString aApiArg;
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
