/*************************************************************************
 *
 *  $RCSfile: visobj.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2003-10-27 12:57:29 $
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

#ifndef _COM_SUN_STAR_EMBED_ASPECTS_HPP_
#include <com/sun/star/embed/Aspects.hpp>
#endif
#ifndef _COM_SUN_STAR_EMBED_EMBEDSTATES_HPP_
#include <com/sun/star/embed/EmbedStates.hpp>
#endif

#ifndef _COM_SUN_STAR_DATATRANSFER_XTRANSFERABLE_HPP_
#include <com/sun/star/datatransfer/XTransferable.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif


#include <commonembobj.hxx>


using namespace ::com::sun::star;

void SAL_CALL OCommonEmbeddedObject::setContainerName( const ::rtl::OUString& sName )
        throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    m_aContainerName = sName;
}

void SAL_CALL OCommonEmbeddedObject::setVisAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
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

    if ( nAspect == embed::Aspects::MSASPECT_CONTENT )
    {
        if ( !m_pDocHolder->SetExtent( aSize ) )
            throw uno::Exception(); // TODO:
    }
    else
    {
        // TODO: do something for other aspects
    }
}

awt::Size SAL_CALL OCommonEmbeddedObject::getVisAreaSize( sal_Int64 nAspect )
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

    if ( nAspect == embed::Aspects::MSASPECT_CONTENT )
    {
        awt::Size aResult;
        if ( !m_pDocHolder->GetExtent( &aResult ) )
            throw uno::Exception(); // TODO:
        return aResult;
    }
    else
    {
        // TODO: do something for other aspects
    }
}

// Probably will be removed!!!
uno::Any SAL_CALL OCommonEmbeddedObject::getVisualCache( sal_Int64 nAspect )
        throw ( uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::EMBED_LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The own object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
#if 0
    OSL_ENSURE( m_xDocument.is(), "Running or Active object has no model!\n" );

    if ( m_xDocument.is() )
    {
        // TODO: return for the aspect of the document
        uno::Reference< datatransfer::XTransferable > xTransferable( m_xDocument, uno::UNO_QUERY );
        if ( xTransferable.is() )
        {
            datatransfer::DataFlavor aDataFlavor(
                    ::rtl::OUString::createFromAscii( "application/x-openoffice;windows_formatname=\"Image EMF\"" ),
                    ::rtl::OUString::createFromAscii( "Image EMF" ),
                    ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );

            return xTransferable->getTransferData( aDataFlavor );
        }
    }
#endif

    return uno::makeAny( uno::Sequence< sal_Int8 >() );
}


