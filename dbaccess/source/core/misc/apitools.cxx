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
#include "precompiled_dbaccess.hxx"

#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef DBACCESS_SHARED_DBASTRINGS_HRC
#include "dbastrings.hrc"
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#include <com/sun/star/lang/XServiceInfo.hpp>
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;
using namespace osl;
using namespace dbaccess;

//==================================================================================
//= various helper functions
//==================================================================================
//============================================================
//= OSubComponent
//============================================================
DBG_NAME(OSubComponent)
//--------------------------------------------------------------------------
OSubComponent::OSubComponent(Mutex& _rMutex, const Reference< XInterface > & xParent)
              :OComponentHelper(_rMutex)
              ,m_xParent(xParent)
{
    DBG_CTOR(OSubComponent,NULL);

}
// -----------------------------------------------------------------------------
OSubComponent::~OSubComponent()
{
    m_xParent = NULL;

    DBG_DTOR(OSubComponent,NULL);
}

// com::sun::star::lang::XTypeProvider
//--------------------------------------------------------------------------
Sequence< Type > OSubComponent::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XComponent > *)0 ),
                           ::getCppuType( (const Reference< XTypeProvider > *)0 ),
                           ::getCppuType( (const Reference< XWeak > *)0 ));

    return aTypes.getTypes();
}

// XInterface
//--------------------------------------------------------------------------
void OSubComponent::acquire() throw ( )
{
    OComponentHelper::acquire();
}

//--------------------------------------------------------------------------
void OSubComponent::release() throw ( )
{
    Reference< XInterface > x( xDelegator );
    if (! x.is())
    {
        if (osl_decrementInterlockedCount( &m_refCount ) == 0 )
        {
            if (! rBHelper.bDisposed)
            {
                // *before* again incrementing our ref count, ensure that our weak connection point
                // will not create references to us anymore (via XAdapter::queryAdapted)
                disposeWeakConnectionPoint();

                Reference< XInterface > xHoldAlive( *this );
                // remember the parent
                Reference< XInterface > xParent;
                {
                    MutexGuard aGuard( rBHelper.rMutex );
                    xParent = m_xParent;
                    m_xParent = NULL;
                }

                OSL_ENSURE( m_refCount == 1, "OSubComponent::release: invalid ref count (before dispose)!" );

                // First dispose
                dispose();

                // only the alive ref holds the object
                OSL_ENSURE( m_refCount == 1, "OSubComponent::release: invalid ref count (after dispose)!" );

                // release the parent in the ~
                if (xParent.is())
                {
                    MutexGuard aGuard( rBHelper.rMutex );
                    m_xParent = xParent;
                }

                // destroy the object if xHoldAlive decrement the refcount to 0
                return;
            }
        }
        // restore the reference count
        osl_incrementInterlockedCount( &m_refCount );
    }

    // as we cover the job of the componenthelper we use the ...
    OWeakAggObject::release();
}

//--------------------------------------------------------------------------
Any OSubComponent::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aReturn;
    if (!rType.equals(::getCppuType(static_cast< Reference< XAggregation >* >(NULL))))
        aReturn = OComponentHelper::queryInterface(rType);

    return aReturn;
}


