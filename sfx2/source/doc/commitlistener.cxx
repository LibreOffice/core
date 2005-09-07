/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: commitlistener.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:36:50 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

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

void SAL_CALL OChildCommitListen_Impl::preCommit( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    // not interesting
}

void SAL_CALL OChildCommitListen_Impl::commited( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // StorageIsModified_Impl must not contain any locking!
    if ( m_pModel )
        m_pModel->StorageIsModified_Impl();
}

void SAL_CALL OChildCommitListen_Impl::preRevert( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException)
{
    // not interesting
}

void SAL_CALL OChildCommitListen_Impl::reverted( const ::com::sun::star::lang::EventObject& aEvent )
        throw (::com::sun::star::uno::RuntimeException)
{
    // not interesting
}


void SAL_CALL OChildCommitListen_Impl::disposing( const lang::EventObject& Source )
        throw ( uno::RuntimeException )
{
    // not interesting
}

