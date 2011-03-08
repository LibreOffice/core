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
#include "precompiled_dbaccess.hxx"

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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
