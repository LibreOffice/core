/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: specialobject.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_embeddedobj.hxx"
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedVerbs.hpp>
#include <com/sun/star/embed/EmbedUpdateModes.hpp>
#include <com/sun/star/embed/XEmbeddedClient.hpp>
#include <com/sun/star/embed/XInplaceClient.hpp>
#include <com/sun/star/embed/XWindowSupplier.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/awt/XWindowPeer.hpp>
#include <com/sun/star/util/XCloseBroadcaster.hpp>
#include <com/sun/star/util/XCloseable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/XDispatchProviderInterception.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>

#include <cppuhelper/typeprovider.hxx>

#include "specialobject.hxx"
#include "intercept.hxx"

using namespace ::com::sun::star;


OSpecialEmbeddedObject::OSpecialEmbeddedObject( const uno::Reference< lang::XMultiServiceFactory >& xFactory, const uno::Sequence< beans::NamedValue >& aObjectProps )
: OCommonEmbeddedObject( xFactory, aObjectProps )
{
    maSize.Width = maSize.Height = 10000;
    m_nObjectState = embed::EmbedStates::LOADED;
}

//------------------------------------------------------
uno::Any SAL_CALL OSpecialEmbeddedObject::queryInterface( const uno::Type& rType )
        throw( uno::RuntimeException )
{
    uno::Any aReturn;

    aReturn <<= ::cppu::queryInterface( rType,
                                        static_cast< embed::XEmbeddedObject* >( this ),
                                        static_cast< embed::XInplaceObject* >( this ),
                                        static_cast< embed::XVisualObject* >( this ),
                                        static_cast< embed::XClassifiedObject* >( this ),
                                        static_cast< embed::XComponentSupplier* >( this ),
                                        static_cast< util::XCloseable* >( this ),
                                        static_cast< document::XEventBroadcaster* >( this ) );
    if ( aReturn.hasValue() )
        return aReturn;
    else
        return ::cppu::OWeakObject::queryInterface( rType ) ;

}

//------------------------------------------------------
uno::Sequence< uno::Type > SAL_CALL OSpecialEmbeddedObject::getTypes()
        throw( uno::RuntimeException )
{
    static ::cppu::OTypeCollection* pTypeCollection = NULL;

    if ( !pTypeCollection )
    {
        ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex() );
        if ( !pTypeCollection )
        {
            static ::cppu::OTypeCollection aTypeCollection(
                                        ::getCppuType( (const uno::Reference< lang::XTypeProvider >*)NULL ),
                                        ::getCppuType( (const uno::Reference< embed::XEmbeddedObject >*)NULL ),
                                        ::getCppuType( (const uno::Reference< embed::XInplaceObject >*)NULL ) );

            pTypeCollection = &aTypeCollection ;
        }
    }

    return pTypeCollection->getTypes() ;

}

embed::VisualRepresentation SAL_CALL OSpecialEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 nAspect )
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
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    // TODO: return for the aspect of the document
    embed::VisualRepresentation aVisualRepresentation;
    /*
    OSL_ENSURE( m_pDocHolder->GetComponent().is(), "Running or Active object has no component!\n" );
    uno::Reference< datatransfer::XTransferable > xTransferable( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
    if ( !xTransferable.is() )
        throw uno::RuntimeException();

    datatransfer::DataFlavor aDataFlavor(
            ::rtl::OUString::createFromAscii( "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"" ),
            ::rtl::OUString::createFromAscii( "GDIMetaFile" ),
            ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );

    aVisualRepresentation.Data = xTransferable->getTransferData( aDataFlavor );
    aVisualRepresentation.Flavor = aDataFlavor;*/
    return aVisualRepresentation;
}

void SAL_CALL OSpecialEmbeddedObject::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    maSize = aSize;
}

awt::Size SAL_CALL OSpecialEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The own object has no model!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    awt::Size aResult;
    return maSize;
}

sal_Int32 SAL_CALL OSpecialEmbeddedObject::getMapUnit( sal_Int64 nAspect )
        throw ( uno::Exception,
                uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    OSL_ENSURE( nAspect != embed::Aspects::MSOLE_ICON, "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return embed::EmbedMapUnits::ONE_100TH_MM;
}

void SAL_CALL OSpecialEmbeddedObject::changeState( sal_Int32 nNewState )
        throw ( ::com::sun::star::embed::UnreachableStateException,
                ::com::sun::star::embed::WrongStateException,
                ::com::sun::star::uno::Exception,
                ::com::sun::star::uno::RuntimeException )
{
    if ( nNewState == embed::EmbedStates::UI_ACTIVE )
        nNewState = embed::EmbedStates::INPLACE_ACTIVE;
    OCommonEmbeddedObject::changeState( nNewState );
}

void SAL_CALL OSpecialEmbeddedObject::doVerb( sal_Int32 nVerbID )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                embed::UnreachableStateException,
                uno::Exception,
                uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( ::rtl::OUString::createFromAscii( "The object has no persistence!\n" ),
                                        uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( nVerbID == -7 )
    {

        uno::Reference < ui::dialogs::XExecutableDialog > xDlg( m_pDocHolder->GetComponent(), uno::UNO_QUERY );
        if ( xDlg.is() )
            xDlg->execute();
        else
            throw embed::UnreachableStateException();
    }
    else
        OCommonEmbeddedObject::doVerb( nVerbID );
}

