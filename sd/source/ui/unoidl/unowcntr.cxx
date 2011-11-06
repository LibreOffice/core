/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sd.hxx"
#include <com/sun/star/lang/XComponent.hpp>
#include <tools/list.hxx>

#include <unowcntr.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

DECLARE_LIST( WeakRefList, uno::WeakReference< uno::XInterface >* )

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

void SvUnoWeakContainer::dispose()
{
    uno::WeakReference< uno::XInterface >* pRef = mpList->First();
    while( pRef )
    {
        uno::Reference< uno::XInterface > xTestRef( *pRef );
        if(xTestRef.is())
        {
            uno::Reference< lang::XComponent > xComp( xTestRef, uno::UNO_QUERY );
            if( xComp.is() )
                xComp->dispose();
        }

        pRef = mpList->Next();
    }
}

