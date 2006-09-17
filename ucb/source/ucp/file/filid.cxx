/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: filid.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 13:46:44 $
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
#include "precompiled_ucb.hxx"
#ifndef _FILID_HXX_
#include "filid.hxx"
#endif
#ifndef _SHELL_HXX_
#include "shell.hxx"
#endif

using namespace fileaccess;
using namespace com::sun::star;
using namespace com::sun::star::ucb;


FileContentIdentifier::FileContentIdentifier(
    shell* pMyShell,
    const rtl::OUString& aUnqPath,
    sal_Bool IsNormalized )
    : m_pMyShell( pMyShell ),
      m_bNormalized( IsNormalized )
{
    if( IsNormalized )
    {
        m_pMyShell->getUrlFromUnq( aUnqPath,m_aContentId );
        m_aNormalizedId = aUnqPath;
        m_pMyShell->getScheme( m_aProviderScheme );
    }
    else
    {
        m_pMyShell->getUnqFromUrl( aUnqPath,m_aNormalizedId );
        m_aContentId = aUnqPath;
        m_pMyShell->getScheme( m_aProviderScheme );
    }
}

FileContentIdentifier::~FileContentIdentifier()
{
}


void SAL_CALL
FileContentIdentifier::acquire(
    void )
    throw()
{
    OWeakObject::acquire();
}


void SAL_CALL
FileContentIdentifier::release(
                void )
  throw()
{
  OWeakObject::release();
}


uno::Any SAL_CALL
FileContentIdentifier::queryInterface(
    const uno::Type& rType )
    throw( uno::RuntimeException )
{
    uno::Any aRet = cppu::queryInterface( rType,
                                          SAL_STATIC_CAST( lang::XTypeProvider*, this),
                                          SAL_STATIC_CAST( XContentIdentifier*, this) );
    return aRet.hasValue() ? aRet : OWeakObject::queryInterface( rType );
}


uno::Sequence< sal_Int8 > SAL_CALL
FileContentIdentifier::getImplementationId()
    throw( uno::RuntimeException )
{
    static cppu::OImplementationId* pId = NULL;
    if ( !pId )
    {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pId )
        {
            static cppu::OImplementationId id( sal_False );
            pId = &id;
        }
    }
    return (*pId).getImplementationId();
}


uno::Sequence< uno::Type > SAL_CALL
FileContentIdentifier::getTypes(
    void )
    throw( uno::RuntimeException )
{
    static cppu::OTypeCollection* pCollection = NULL;
    if ( !pCollection ) {
        osl::Guard< osl::Mutex > aGuard( osl::Mutex::getGlobalMutex() );
        if ( !pCollection )
        {
            static cppu::OTypeCollection collection(
                getCppuType( static_cast< uno::Reference< lang::XTypeProvider >* >( 0 ) ),
                getCppuType( static_cast< uno::Reference< XContentIdentifier >* >( 0 ) ) );
            pCollection = &collection;
        }
    }
    return (*pCollection).getTypes();
}


rtl::OUString
SAL_CALL
FileContentIdentifier::getContentIdentifier(
    void )
    throw( uno::RuntimeException )
{
    return m_aContentId;
}


rtl::OUString SAL_CALL
FileContentIdentifier::getContentProviderScheme(
    void )
    throw( uno::RuntimeException )
{
    return m_aProviderScheme;
}
