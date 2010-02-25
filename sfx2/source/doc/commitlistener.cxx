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
#include "precompiled_sfx2.hxx"

#include "commitlistener.hxx"

using namespace ::com::sun::star;

OChildCommitListen_Impl::OChildCommitListen_Impl( SfxBaseModel& aModel )
: m_pModel( &aModel )
{}

OChildCommitListen_Impl::~OChildCommitListen_Impl()
{}

void OChildCommitListen_Impl::OwnerIsDisposed()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_pModel = NULL;
}

void SAL_CALL OChildCommitListen_Impl::preCommit( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    // not interesting
}

void SAL_CALL OChildCommitListen_Impl::commited( const ::com::sun::star::lang::EventObject& /*aEvent*/ )
        throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // StorageIsModified_Impl must not contain any locking!
    if ( m_pModel )
        m_pModel->StorageIsModified_Impl();
}

void SAL_CALL OChildCommitListen_Impl::preRevert( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    // not interesting
}

void SAL_CALL OChildCommitListen_Impl::reverted( const ::com::sun::star::lang::EventObject& )
        throw (::com::sun::star::uno::RuntimeException)
{
    // not interesting
}


void SAL_CALL OChildCommitListen_Impl::disposing( const lang::EventObject& )
        throw ( uno::RuntimeException )
{
    // not interesting
}

