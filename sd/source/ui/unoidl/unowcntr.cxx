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
#include "precompiled_sd.hxx"
#include <com/sun/star/lang/XComponent.hpp>

#include <unowcntr.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

SvUnoWeakContainer::SvUnoWeakContainer() throw()
{
}

SvUnoWeakContainer::~SvUnoWeakContainer() throw()
{
    for ( WeakRefList::iterator it = maList.begin(); it != maList.end(); ++it )
            delete *it;
    maList.clear();
}

/** inserts the given ref into this container */
void SvUnoWeakContainer::insert( uno::WeakReference< uno::XInterface > xRef ) throw()
{
    for ( WeakRefList::iterator it = maList.begin(); it != maList.end(); )
    {
        uno::WeakReference< uno::XInterface >* pRef = *it;
        uno::Reference< uno::XInterface > xTestRef( *pRef );
        if ( !xTestRef.is() )
        {
            delete pRef;
            it = maList.erase( it );
        }
        else
        {
            if ( *pRef == xRef )
                return;
            ++it;
        }
    }
    maList.push_back( new uno::WeakReference< uno::XInterface >( xRef ) );
}

/** searches the container for a ref that returns true on the given
    search function
*/
sal_Bool SvUnoWeakContainer::findRef(
    uno::WeakReference< uno::XInterface >& rRef,
    void* pSearchData,
    weakref_searchfunc pSearchFunc
)
{
    for ( WeakRefList::iterator it = maList.begin(); it != maList.end(); )
    {
        uno::WeakReference< uno::XInterface >* pRef = *it;
        uno::Reference< uno::XInterface > xTestRef( *pRef );
        if ( !xTestRef.is() )
        {
            delete pRef;
            it = maList.erase( it );
        }
        else
        {
            if( (*pSearchFunc)( *pRef, pSearchData ) )
            {
                rRef = *pRef;
                return sal_True;
            }
            ++it;
        }
    }
    return sal_False;
}

void SvUnoWeakContainer::dispose()
{
    for ( WeakRefList::iterator it = maList.begin(); it != maList.end(); ++it )
    {
        uno::WeakReference< uno::XInterface >* pRef = *it;
        uno::Reference< uno::XInterface > xTestRef( *pRef );
        if ( xTestRef.is() )
        {
            uno::Reference< lang::XComponent > xComp( xTestRef, uno::UNO_QUERY );
            if( xComp.is() )
                xComp->dispose();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
