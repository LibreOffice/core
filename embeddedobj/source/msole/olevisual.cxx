/*************************************************************************
 *
 *  $RCSfile: olevisual.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2004-10-04 19:55:50 $
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

#ifndef _COM_SUN_STAR_EMBED_EMBEDMAPUNITS_HPP_
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif

#include <oleembobj.hxx>
#include <olecomponent.hxx>


using namespace ::com::sun::star;

embed::VisualRepresentation OleEmbeddedObject::GetVisualRepresentationInNativeFormat_Impl(
                    const uno::Reference< io::XStream > xCachedVisRepr )
        throw ( uno::Exception )
{
    embed::VisualRepresentation aVisualRepr;

    // TODO: detect the format in the future for now use workaround
    uno::Reference< io::XInputStream > xInStream = xCachedVisRepr->getInputStream();
    uno::Reference< io::XSeekable > xSeekable( xCachedVisRepr, uno::UNO_QUERY );
    if ( !xInStream.is() || !xSeekable.is() )
        throw uno::RuntimeException();

    uno::Sequence< sal_Int8 > aSeq( 2 );
    xInStream->readBytes( aSeq, 2 );
    xSeekable->seek( 0 );
    if ( aSeq.getLength() == 2 && aSeq[0] == 'B' && aSeq[1] == 'M' )
    {
        // it's a bitmap
        aVisualRepr.Flavor = datatransfer::DataFlavor(
            ::rtl::OUString::createFromAscii( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" ),
            ::rtl::OUString::createFromAscii( "Bitmap" ),
            ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );
    }
    else
    {
        // it's a metafile
        aVisualRepr.Flavor = datatransfer::DataFlavor(
            ::rtl::OUString::createFromAscii( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" ),
            ::rtl::OUString::createFromAscii( "Windows Metafile" ),
            ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );
    }

    sal_Int32 nStreamLength = (sal_Int32)xSeekable->getLength();
    uno::Sequence< sal_Int8 > aRepresent( nStreamLength );
    xInStream->readBytes( aRepresent, nStreamLength );
    aVisualRepr.Data <<= aRepresent;

    return aVisualRepr;
}

void SAL_CALL OleEmbeddedObject::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object is not loaded!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

#ifdef WNT
    if ( m_pOleComponent )
    {
        if ( m_nObjectState == embed::EmbedStates::LOADED )
            throw embed::WrongStateException(
                                    ::rtl::OUString::createFromAscii( "The object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

        m_pOleComponent->SetExtent( aSize, nAspect ); // will throw an exception in case of failure
    }
    else
#endif
    {
        // cache the values
        m_aCachedSize = aSize;

        // throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Illegal call!\n" ),
        //                          uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }
}

awt::Size SAL_CALL OleEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object is not loaded!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

#ifdef WNT
    if ( m_pOleComponent )
    {
        // TODO/LATER: the caching should be used also
        if ( m_nObjectState == embed::EmbedStates::LOADED )
            throw embed::WrongStateException(
                                    ::rtl::OUString::createFromAscii( "The object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

        return m_pOleComponent->GetExtent( nAspect ); // will throw an exception in case of failure
    }
    else
#endif
    {
        // return cached value
        return m_aCachedSize;

        // throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Illegal call!\n" ),
        //                          uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );
    }
}

embed::VisualRepresentation SAL_CALL OleEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 nAspect )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    // TODO: if the object has cached representation then it should be returned
    // TODO: if the object has no cached representation and is in loaded state it should switch itself to the running state
    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object is not loaded!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    embed::VisualRepresentation aVisualRepr;

    // TODO: in case of different aspects they must be applied to the mediatype and XTransferable must be used
    if ( !m_xCachedVisualRepresentation.is() && m_bVisReplInStream )
        m_xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( m_xObjectStream );

    if ( m_xCachedVisualRepresentation.is() )
    {
        return GetVisualRepresentationInNativeFormat_Impl( m_xCachedVisualRepresentation );
    }
#ifdef WNT
    else if ( m_pOleComponent )
    {
        if ( m_nObjectState == embed::EmbedStates::LOADED )
            throw embed::WrongStateException(
                                    ::rtl::OUString::createFromAscii( "The object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

        datatransfer::DataFlavor aDataFlavor(
                ::rtl::OUString::createFromAscii( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" ),
                ::rtl::OUString::createFromAscii( "Windows Metafile" ),
                ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );

        aVisualRepr.Data = m_pOleComponent->getTransferData( aDataFlavor );
        aVisualRepr.Flavor = aDataFlavor;
    }
#endif
    else
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    return aVisualRepr;
}

sal_Int32 SAL_CALL OleEmbeddedObject::getMapUnit( sal_Int64 nAspect )
        throw ( uno::Exception,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object is not loaded!\n" ),
                                    uno::Reference< uno::XInterface >( reinterpret_cast< ::cppu::OWeakObject* >(this) ) );

    // TODO/LATER: ??? OLE objects should be always controlled in pixels,
    //             internally a workaround with scaling based on visual representation will be used
    return embed::EmbedMapUnits::ONE_100TH_MM;
}


