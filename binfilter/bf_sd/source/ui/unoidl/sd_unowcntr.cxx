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

#include <tools/list.hxx>

#include <unowcntr.hxx>
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

DECLARE_LIST( WeakRefList, uno::WeakReference< uno::XInterface >* )//STRIP008 ;

SvUnoWeakContainer::SvUnoWeakContainer() throw()
{
    mpList = new WeakRefList;
}

SvUnoWeakContainer::~SvUnoWeakContainer() throw()
{
    uno::WeakReference< uno::XInterface >* pRef = mpList->First();
    while( pRef )
    {
        delete mpList->Remove();
        pRef = mpList->GetCurObject();
    }
    delete mpList;
}

/** inserts the given ref into this container */
void SvUnoWeakContainer::insert( uno::WeakReference< uno::XInterface > xRef ) throw()
{
    uno::WeakReference< uno::XInterface >* pRef = mpList->First();
    while( pRef )
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >  xTestRef( *pRef );
        if(! xTestRef.is() )
        {
            delete mpList->Remove();
            pRef = mpList->GetCurObject();
        }
        else
        {
            if( *pRef == xRef )
                return;

            pRef = mpList->Next();
        }
    }

    mpList->Insert( new uno::WeakReference< uno::XInterface >( xRef ) );
}

/** searches the container for a ref that returns true on the given 
    search function
*/
sal_Bool SvUnoWeakContainer::findRef( uno::WeakReference< uno::XInterface >& rRef, void* pSearchData, weakref_searchfunc pSearchFunc )
{
    uno::WeakReference< uno::XInterface >* pRef = mpList->First();
    while( pRef )
    {
        uno::Reference< ::com::sun::star::uno::XInterface > xTestRef( *pRef );
        if(!xTestRef.is())
        {
            delete mpList->Remove();
            pRef = mpList->GetCurObject();
        }
        else
        {
            if( (*pSearchFunc)( *pRef, pSearchData ) )
            {
                rRef = *pRef;
                return sal_True;
            }

            pRef = mpList->Next();
        }
    }

    return sal_False;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
