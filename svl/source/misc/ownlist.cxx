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
#include <com/sun/star/beans/PropertyValues.hpp>

#include <svl/ownlist.hxx>

using namespace com::sun::star;


static OUString parseString(const OUString & rCmd, sal_Int32 * pIndex)
{
    OUString result;

    if(rCmd[*pIndex] == '\"') {
        (*pIndex) ++;

        sal_Int32 begin = *pIndex;

        while(*pIndex < rCmd.getLength() && rCmd[(*pIndex) ++] != '\"') ;

        result = rCmd.copy(begin, *pIndex - begin - 1);
    }

    return result;
}

static OUString parseWord(const OUString & rCmd, sal_Int32 * pIndex)
{
    sal_Int32 begin = *pIndex;

    while(*pIndex < rCmd.getLength()
          && !isspace(sal::static_int_cast<int>(rCmd[*pIndex]))
          && rCmd[*pIndex] != '=')
        (*pIndex) ++;

    return rCmd.copy(begin, *pIndex - begin);
}

static void eatSpace(const OUString & rCmd, sal_Int32 * pIndex)
{
    while(*pIndex < rCmd.getLength() && isspace(sal::static_int_cast<int>(rCmd[*pIndex])))
        (*pIndex) ++;
}

/**
 * Text is parsed and the single commands are added to the list.
 *
 * @returns bool  true
 *                The text was correctly parsed
                  false
                  The text was not parsed correctly
*/
bool SvCommandList::AppendCommands
(
 const OUString & rCmd,    /* This text is translated to commands */
 sal_Int32 * pEaten        /* Count of chars that have been read */
)
{
    sal_Int32 index = 0;
    while(index < rCmd.getLength())
    {

        eatSpace(rCmd, &index);
        OUString name = (rCmd[index] == '\"') ? parseString(rCmd, &index) : parseWord(rCmd, &index);

        eatSpace(rCmd, &index);
        OUString value;
        if(index < rCmd.getLength() && rCmd[index] == '=')
        {
            index ++;

            eatSpace(rCmd, &index);
            value = (rCmd[index] == '\"') ? parseString(rCmd, &index) : parseWord(rCmd, &index);
        }

        aCommandList.push_back( SvCommand(name, value));
    }

    *pEaten = index;

    return true;
}

/**
 * An object of the type SvCommand is created and the list is
 * attached.
 *
 * @returns SvCommand & The created object
*/
SvCommand & SvCommandList::Append
(
 const OUString & rCommand,    /* The command */
 const OUString & rArg         /* The command's argument */
)
{
    aCommandList.push_back( SvCommand( rCommand, rArg ) );
    return aCommandList.back();
}

bool SvCommandList::FillFromSequence( const css::uno::Sequence < css::beans::PropertyValue >& aCommandSequence )
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

void SvCommandList::FillSequence( css::uno::Sequence < css::beans::PropertyValue >& aCommandSequence )
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
