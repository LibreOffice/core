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

#include <com/sun/star/lang/XComponent.hpp>

#include "unowcntr.hxx"

using namespace ::com::sun::star;

SvUnoWeakContainer::SvUnoWeakContainer() noexcept
{
}

SvUnoWeakContainer::~SvUnoWeakContainer() noexcept
{
}

/** inserts the given ref into this container */
void SvUnoWeakContainer::insert( const uno::WeakReference< uno::XInterface >& xRef ) noexcept
{
    for ( auto it = maVector.begin(); it != maVector.end(); )
    {
        uno::WeakReference< uno::XInterface > & rWeakRef = *it;
        uno::Reference< uno::XInterface > xTestRef( rWeakRef );
        if ( !xTestRef.is() )
        {
            it = maVector.erase( it );
        }
        else
        {
            if ( rWeakRef == xRef )
                return;
            ++it;
        }
    }
    maVector.emplace_back( xRef );
}

/** searches the container for a ref that returns true on the given
    search function
*/
bool SvUnoWeakContainer::findRef(
    uno::WeakReference< uno::XInterface >& rRef,
    void const * pSearchData,
    weakref_searchfunc pSearchFunc
)
{
    for ( auto it = maVector.begin(); it != maVector.end(); )
    {
        uno::WeakReference< uno::XInterface > & itRef = *it;
        uno::Reference< uno::XInterface > xTestRef( itRef );
        if ( !xTestRef.is() )
        {
            it = maVector.erase( it );
        }
        else
        {
            if( (*pSearchFunc)( itRef, pSearchData ) )
            {
                rRef = itRef;
                return true;
            }
            ++it;
        }
    }
    return false;
}

void SvUnoWeakContainer::dispose()
{
    for (auto const& elem : maVector)
    {
        uno::Reference< uno::XInterface > xTestRef( elem );
        if ( xTestRef.is() )
        {
            uno::Reference< lang::XComponent > xComp( xTestRef, uno::UNO_QUERY );
            if( xComp.is() )
                xComp->dispose();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
