/*************************************************************************
 *
 *  $RCSfile: olevisual.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2003-11-13 17:01:14 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif

#include <oleembobj.hxx>
#include <olecomponent.hxx>


using namespace ::com::sun::star;

void SAL_CALL OleEmbeddedObject::setContainerName( const ::rtl::OUString& sName )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    m_aContainerName = sName;
}

void SAL_CALL OleEmbeddedObject::setVisAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::EMBED_LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The own object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    m_pOleComponent->SetExtent( aSize, nAspect ); // will throw an exception in case of failure
}

awt::Size SAL_CALL OleEmbeddedObject::getVisAreaSize( sal_Int64 nAspect )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::EMBED_LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The own object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_pOleComponent )
        throw uno::RuntimeException();

    return m_pOleComponent->GetExtent( nAspect ); // will throw an exception in case of failure
}

// Probably will be removed!!!
uno::Any SAL_CALL OleEmbeddedObject::getVisualCache( sal_Int64 nAspect )
        throw ( uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::EMBED_LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The own object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return uno::makeAny( uno::Sequence< sal_Int8 >() );
}


