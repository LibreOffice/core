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


/**
 * An object of the type SvCommand is created and the list is
 * attached.
*/
void SvCommandList::Append
(
 const OUString & rCommand,    /* The command */
 const OUString & rArg         /* The command's argument */
)
{
    aCommandList.push_back( SvCommand( rCommand, rArg ) );
}

void SvCommandList::FillFromSequence( const css::uno::Sequence < css::beans::PropertyValue >& aCommandSequence )
{
    const sal_Int32 nCount = aCommandSequence.getLength();
    OUString aCommand, aArg;
    OUString aApiArg;
    for( sal_Int32 nIndex=0; nIndex<nCount; nIndex++ )
    {
        aCommand = aCommandSequence[nIndex].Name;
        if( !( aCommandSequence[nIndex].Value >>= aApiArg ) )
            return;
        aArg = aApiArg;
        Append( aCommand, aArg );
    }
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
