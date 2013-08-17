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

#include "apitools.hxx"
#include "dbastrings.hrc"
#include <cppuhelper/typeprovider.hxx>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <osl/diagnose.h>
#include <tools/debug.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;
using namespace osl;

// various helper functions
// OSubComponent
DBG_NAME(OSubComponent)
OSubComponent::OSubComponent(Mutex& _rMutex, const Reference< XInterface > & xParent)
              :OComponentHelper(_rMutex)
              ,m_xParent(xParent)
{
    DBG_CTOR(OSubComponent,NULL);

}

OSubComponent::~OSubComponent()
{
    m_xParent = NULL;

    DBG_DTOR(OSubComponent,NULL);
}

// com::sun::star::lang::XTypeProvider
Sequence< Type > OSubComponent::getTypes() throw (RuntimeException)
{
    OTypeCollection aTypes(::getCppuType( (const Reference< XComponent > *)0 ),
                           ::getCppuType( (const Reference< XTypeProvider > *)0 ),
                           ::getCppuType( (const Reference< XWeak > *)0 ));

    return aTypes.getTypes();
}

// XInterface
void OSubComponent::acquire() throw ( )
{
    OComponentHelper::acquire();
}

void OSubComponent::release() throw ( )
{
    Reference< XInterface > x( xDelegator );
    if (! x.is())
    {
        if (osl_atomic_decrement( &m_refCount ) == 0 )
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
        osl_atomic_increment( &m_refCount );
    }

    // as we cover the job of the componenthelper we use the ...
    OWeakAggObject::release();
}

Any OSubComponent::queryInterface( const Type & rType ) throw(RuntimeException)
{
    Any aReturn;
    if (!rType.equals(::getCppuType(static_cast< Reference< XAggregation >* >(NULL))))
        aReturn = OComponentHelper::queryInterface(rType);

    return aReturn;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
