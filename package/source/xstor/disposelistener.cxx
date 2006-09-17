/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: disposelistener.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 17:23:56 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_package.hxx"

#include "disposelistener.hxx"

#ifndef __XSTORAGE_HXX_
#include "xstorage.hxx"
#endif

using namespace ::com::sun::star;

OChildDispListener_Impl::OChildDispListener_Impl( OStorage& aStorage )
: m_pStorage( &aStorage )
{}

OChildDispListener_Impl::~OChildDispListener_Impl()
{}

void OChildDispListener_Impl::OwnerIsDisposed()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_pStorage = NULL;
}

void SAL_CALL OChildDispListener_Impl::disposing( const lang::EventObject& Source )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    // ObjectIsDisposed must not contain any locking!
    if ( m_pStorage && Source.Source.is() )
        m_pStorage->ChildIsDisposed( Source.Source );
}

