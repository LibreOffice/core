/*************************************************************************
 *
 *  $RCSfile: visobj.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 19:50:21 $
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

void SAL_CALL OCommonEmbeddedObject::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The own object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    if ( !m_pDocHolder->SetExtent( nAspect, aSize ) )
        throw uno::Exception(); // TODO:
}

awt::Size SAL_CALL OCommonEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The own object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    awt::Size aResult;
    if ( !m_pDocHolder->GetExtent( nAspect, &aResult ) )
        throw uno::Exception(); // TODO:
    return aResult;
}

sal_Int32 SAL_CALL OCommonEmbeddedObject::getMapUnit( sal_Int64 nAspect )
        throw ( uno::Exception,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The own object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    sal_Int32 nResult = m_pDocHolder->GetMapUnit( nAspect );
    if ( nResult < 0  )
        throw uno::Exception(); // TODO:

    return nResult;

}

embed::VisualRepresentation SAL_CALL OCommonEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 nAspect )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    // TODO: if object is in loaded state it should switch itself to the running state
    if ( m_nObjectState == -1 || m_nObjectState == embed::EmbedStates::LOADED )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The own object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    OSL_ENSURE( m_pDocHolder->GetComponent().is(), "Running or Active object has no component!\n" );

    // TODO: return for the aspect of the document
    embed::VisualRepresentation aVisualRepresentation;
    uno::Reference< datatransfer::XTransferable > xTransferable( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
    if ( !xTransferable.is() )
        throw uno::RuntimeException();

    datatransfer::DataFlavor aDataFlavor(
            ::rtl::OUString::createFromAscii( "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"" ),
            ::rtl::OUString::createFromAscii( "GDIMetaFile" ),
            ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );

    aVisualRepresentation.Data = xTransferable->getTransferData( aDataFlavor );
    aVisualRepresentation.Flavor = aDataFlavor;

    return aVisualRepresentation;
}

