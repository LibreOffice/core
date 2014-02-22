/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>

#include <oleembobj.hxx>
#include <olecomponent.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/seqstream.hxx>

using namespace ::com::sun::star;
using namespace ::comphelper;

embed::VisualRepresentation OleEmbeddedObject::GetVisualRepresentationInNativeFormat_Impl(
                    const uno::Reference< io::XStream > xCachedVisRepr )
        throw ( uno::Exception )
{
    embed::VisualRepresentation aVisualRepr;

    
    uno::Reference< io::XInputStream > xInStream = xCachedVisRepr->getInputStream();
    uno::Reference< io::XSeekable > xSeekable( xCachedVisRepr, uno::UNO_QUERY );
    if ( !xInStream.is() || !xSeekable.is() )
        throw uno::RuntimeException();

    uno::Sequence< sal_Int8 > aSeq( 2 );
    xInStream->readBytes( aSeq, 2 );
    xSeekable->seek( 0 );
    if ( aSeq.getLength() == 2 && aSeq[0] == 'B' && aSeq[1] == 'M' )
    {
        
        aVisualRepr.Flavor = datatransfer::DataFlavor(
            OUString( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" ),
            OUString( "Bitmap" ),
            ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );
    }
    else
    {
        
        aVisualRepr.Flavor = datatransfer::DataFlavor(
            OUString( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" ),
            OUString( "Windows Metafile" ),
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
    SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::setVisualAreaSize" );

    
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        
        xWrappedObject->setVisualAreaSize( nAspect, aSize );
        return;
    }
    

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.ole", "For iconified objects no graphical replacement is required!\n" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        
        throw embed::WrongStateException( OUString( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object is not loaded!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

#ifdef WNT
    
    
    
    
    sal_Bool bAllowToSetExtent =
      ( ( getStatus( nAspect ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE )
      && !MimeConfigurationHelper::ClassIDsEqual( m_aClassID, MimeConfigurationHelper::GetSequenceClassID( 0x00020906L, 0x0000, 0x0000,
                                                           0xc0,0x00,0x00,0x00,0x00,0x00,0x00,0x46 ) )
      && m_bHasCachedSize );

    if ( m_nObjectState == embed::EmbedStates::LOADED && bAllowToSetExtent )
    {
        aGuard.clear();
        try {
            changeState( embed::EmbedStates::RUNNING );
        }
        catch( const uno::Exception& )
        {
            SAL_WARN( "embeddedobj.ole", "The object should not be resized without activation!\n" );
        }
        aGuard.reset();
    }

    if ( m_pOleComponent && m_nObjectState != embed::EmbedStates::LOADED && bAllowToSetExtent )
    {
        awt::Size aSizeToSet = aSize;
        aGuard.clear();
        try {
            m_pOleComponent->SetExtent( aSizeToSet, nAspect ); 
            m_bHasSizeToSet = sal_False;
        }
        catch( const uno::Exception& )
        {
            
            m_bHasSizeToSet = sal_True;
            m_aSizeToSet = aSizeToSet;
            m_nAspectToSet = nAspect;
        }
        aGuard.reset();
    }
#endif

    
    m_bHasCachedSize = sal_True;
    m_aCachedSize = aSize;
    m_nCachedAspect = nAspect;
}

awt::Size SAL_CALL OleEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::getVisualAreaSize" );

    
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        
        return xWrappedObject->getVisualAreaSize( nAspect );
    }
    

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.ole", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        
        throw embed::WrongStateException( OUString( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object is not loaded!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    awt::Size aResult;

#ifdef WNT
    
    if ( m_pOleComponent && !m_bHasSizeToSet && nAspect == embed::Aspects::MSOLE_CONTENT )
    {
        try
        {
            
            if ( m_bHasCachedSize )
            {
                aResult = m_aCachedSize;
            }
            else
            {
                
                awt::Size aSize;
                aGuard.clear();

                sal_Bool bSuccess = sal_False;
                if ( getCurrentState() == embed::EmbedStates::LOADED )
                {
                    SAL_WARN( "embeddedobj.ole", "Loaded object has no cached size!" );

                    
                    try {
                        changeState( embed::EmbedStates::RUNNING );
                    }
                    catch( const uno::Exception& )
                    {
                        throw embed::NoVisualAreaSizeException(
                                OUString( "No size available!\n" ),
                                uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
                    }
                }

                try
                {
                    
                    aSize = m_pOleComponent->GetExtent( nAspect ); 
                    bSuccess = sal_True;
                }
                catch( const uno::Exception& )
                {
                }

                if ( !bSuccess )
                {
                    try
                    {
                        
                        aSize = m_pOleComponent->GetCachedExtent( nAspect ); 
                        bSuccess = sal_True;
                    }
                    catch( const uno::Exception& )
                    {
                    }
                }

                if ( !bSuccess )
                {
                    try
                    {
                        
                        aSize = m_pOleComponent->GetReccomendedExtent( nAspect ); 
                        bSuccess = sal_True;
                    }
                    catch( const uno::Exception& )
                    {
                    }
                }

                if ( !bSuccess )
                    throw embed::NoVisualAreaSizeException(
                                    OUString( "No size available!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

                aGuard.reset();

                m_aCachedSize = aSize;
                m_nCachedAspect = nAspect;
                m_bHasCachedSize = sal_True;

                aResult = m_aCachedSize;
            }
        }
        catch ( const embed::NoVisualAreaSizeException& )
        {
            throw;
        }
        catch ( const uno::Exception& )
        {
            throw embed::NoVisualAreaSizeException(
                            OUString( "No size available!\n" ),
                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
        }
    }
    else
#endif
    {
        
        if ( m_bHasCachedSize )
        {
            SAL_WARN_IF( nAspect != m_nCachedAspect, "embeddedobj.ole", "Unexpected aspect is requested!" );
            aResult = m_aCachedSize;
        }
        else
        {
            throw embed::NoVisualAreaSizeException(
                            OUString( "No size available!\n" ),
                            uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
        }
    }

    return aResult;
}

embed::VisualRepresentation SAL_CALL OleEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 nAspect )
        throw ( lang::IllegalArgumentException,
                embed::WrongStateException,
                uno::Exception,
                uno::RuntimeException )
{
    SAL_INFO( "embeddedobj.ole", "embeddedobj (mv76033) OleEmbeddedObject::getPreferredVisualRepresentation" );

    
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        
        return xWrappedObject->getPreferredVisualRepresentation( nAspect );
    }
    

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.ole", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        
        throw embed::WrongStateException( OUString( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    
    
    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object is not loaded!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    embed::VisualRepresentation aVisualRepr;

    
    
    if ( !m_xCachedVisualRepresentation.is() && ( !m_bVisReplInitialized || m_bVisReplInStream )
      && m_nObjectState == embed::EmbedStates::LOADED )
    {
        m_xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( m_xObjectStream, sal_True );
        SetVisReplInStream( m_xCachedVisualRepresentation.is() );
    }

    if ( m_xCachedVisualRepresentation.is() )
    {
        return GetVisualRepresentationInNativeFormat_Impl( m_xCachedVisualRepresentation );
    }
#ifdef WNT
    else if ( m_pOleComponent )
    {
        try
        {
            if ( m_nObjectState == embed::EmbedStates::LOADED )
                changeState( embed::EmbedStates::RUNNING );

            datatransfer::DataFlavor aDataFlavor(
                    OUString( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" ),
                    OUString( "Windows Metafile" ),
                    ::getCppuType( (const uno::Sequence< sal_Int8 >*) NULL ) );

            aVisualRepr.Data = m_pOleComponent->getTransferData( aDataFlavor );
            aVisualRepr.Flavor = aDataFlavor;

            uno::Sequence< sal_Int8 > aVisReplSeq;
            aVisualRepr.Data >>= aVisReplSeq;
            if ( aVisReplSeq.getLength() )
            {
                m_xCachedVisualRepresentation = GetNewFilledTempStream_Impl(
                        uno::Reference< io::XInputStream > ( static_cast< io::XInputStream* > (
                            new ::comphelper::SequenceInputStream( aVisReplSeq ) ) ) );
            }

            return aVisualRepr;
        }
        catch( const uno::Exception& )
        {}
    }
#endif

    
    if ( !m_xCachedVisualRepresentation.is() && ( !m_bVisReplInitialized || m_bVisReplInStream ) )
    {
        m_xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( m_xObjectStream );
        SetVisReplInStream( m_xCachedVisualRepresentation.is() );
    }

    if ( !m_xCachedVisualRepresentation.is() )
    {
        
        throw embed::WrongStateException( OUString( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );
    }

    return GetVisualRepresentationInNativeFormat_Impl( m_xCachedVisualRepresentation );
}

sal_Int32 SAL_CALL OleEmbeddedObject::getMapUnit( sal_Int64 nAspect )
        throw ( uno::Exception,
                uno::RuntimeException)
{
    
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        
        return xWrappedObject->getMapUnit( nAspect );
    }
    

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); 

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.ole", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        
        throw embed::WrongStateException( OUString( "Illegal call!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( OUString( "The object is not loaded!\n" ),
                                    uno::Reference< uno::XInterface >( static_cast< ::cppu::OWeakObject* >(this) ) );

    return embed::EmbedMapUnits::ONE_100TH_MM;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
