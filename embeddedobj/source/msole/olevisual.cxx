/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/EmbedMapUnits.hpp>
#include <com/sun/star/embed/EmbedMisc.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/WrongStateException.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>

#include <oleembobj.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/seqstream.hxx>
#include <filter/msfilter/classids.hxx>
#include <sal/log.hxx>

#if defined(_WIN32)
#include "olecomponent.hxx"
#endif

using namespace ::com::sun::star;
using namespace ::comphelper;

embed::VisualRepresentation OleEmbeddedObject::GetVisualRepresentationInNativeFormat_Impl(
                    const uno::Reference< io::XStream >& xCachedVisRepr )
{
    embed::VisualRepresentation aVisualRepr;

    // TODO: detect the format in the future for now use workaround
    uno::Reference< io::XInputStream > xInStream = xCachedVisRepr->getInputStream();
    if ( !xInStream.is() )
        throw uno::RuntimeException();
    uno::Reference< io::XSeekable > xSeekable( xCachedVisRepr, uno::UNO_QUERY_THROW );

    uno::Sequence< sal_Int8 > aSeq( 2 );
    xInStream->readBytes( aSeq, 2 );
    xSeekable->seek( 0 );
    if ( aSeq.getLength() == 2 && aSeq[0] == 'B' && aSeq[1] == 'M' )
    {
        // it's a bitmap
        aVisualRepr.Flavor = datatransfer::DataFlavor(
            "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"",
            "Bitmap",
            cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );
    }
    else
    {
        // it's a metafile
        aVisualRepr.Flavor = datatransfer::DataFlavor(
            "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"",
            "Windows Metafile",
            cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );
    }

    sal_Int32 nStreamLength = static_cast<sal_Int32>(xSeekable->getLength());
    uno::Sequence< sal_Int8 > aRepresent( nStreamLength );
    xInStream->readBytes( aRepresent, nStreamLength );
    aVisualRepr.Data <<= aRepresent;

    return aVisualRepr;
}

void SAL_CALL OleEmbeddedObject::setVisualAreaSize( sal_Int64 nAspect, const awt::Size& aSize )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        xWrappedObject->setVisualAreaSize( nAspect, aSize );
        return;
    }
    // end wrapping related part ====================

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.ole", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object is not loaded!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

#ifdef _WIN32
    // RECOMPOSE_ON_RESIZE misc flag means that the object has to be switched to running state on resize.
    // SetExtent() is called only for objects that require it,
    // it should not be called for MSWord documents to workaround problem i49369
    // If cached size is not set, that means that this is the size initialization, so there is no need to set the real size
    bool bAllowToSetExtent =
      ( ( getStatus( nAspect ) & embed::EmbedMisc::MS_EMBED_RECOMPOSEONRESIZE )
      && !MimeConfigurationHelper::ClassIDsEqual(m_aClassID, MimeConfigurationHelper::GetSequenceClassID(MSO_WW8_CLASSID))
      && m_bHasCachedSize );

    if ( m_nObjectState == embed::EmbedStates::LOADED && bAllowToSetExtent )
    {
        aGuard.clear();
        try {
            changeState( embed::EmbedStates::RUNNING );
        }
        catch( const uno::Exception& )
        {
            SAL_WARN( "embeddedobj.ole", "The object should not be resized without activation!" );
        }
        aGuard.reset();
    }

    if ( m_pOleComponent && m_nObjectState != embed::EmbedStates::LOADED && bAllowToSetExtent )
    {
        awt::Size aSizeToSet = aSize;
        aGuard.clear();
        try {
            m_pOleComponent->SetExtent( aSizeToSet, nAspect ); // will throw an exception in case of failure
            m_bHasSizeToSet = false;
        }
        catch( const uno::Exception& )
        {
            // some objects do not allow to set the size even in running state
            m_bHasSizeToSet = true;
            m_aSizeToSet = aSizeToSet;
            m_nAspectToSet = nAspect;
        }
        aGuard.reset();
    }
#endif

    // cache the values
    m_bHasCachedSize = true;
    m_aCachedSize = aSize;
    m_nCachedAspect = nAspect;
}

awt::Size SAL_CALL OleEmbeddedObject::getVisualAreaSize( sal_Int64 nAspect )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getVisualAreaSize( nAspect );
    }
    // end wrapping related part ====================

    ::osl::ResettableMutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.ole", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object is not loaded!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    awt::Size aResult;

#ifdef _WIN32
    // TODO/LATER: Support different aspects
    if ( m_pOleComponent && !m_bHasSizeToSet && nAspect == embed::Aspects::MSOLE_CONTENT )
    {
        try
        {
            // the cached size updated every time the object is stored
            if ( m_bHasCachedSize )
            {
                aResult = m_aCachedSize;
            }
            else
            {
                // there is no internal cache
                awt::Size aSize;
                aGuard.clear();

                bool bBackToLoaded = false;

                bool bSuccess = false;
                if ( getCurrentState() == embed::EmbedStates::LOADED )
                {
                    SAL_WARN( "embeddedobj.ole", "Loaded object has no cached size!" );

                    // try to switch the object to RUNNING state and request the value again
                    try {
                        changeState( embed::EmbedStates::RUNNING );
                        // the links should be switched back to loaded state to avoid too
                        // many open MathType instances
                        bBackToLoaded = true;
                    }
                    catch( const uno::Exception& )
                    {
                        throw embed::NoVisualAreaSizeException(
                                "No size available!",
                                static_cast< ::cppu::OWeakObject* >(this) );
                    }
                }

                try
                {
                    // first try to get size using replacement image
                    aSize = m_pOleComponent->GetExtent( nAspect ); // will throw an exception in case of failure
                    bSuccess = true;
                }
                catch( const uno::Exception& )
                {
                }

                if (bBackToLoaded)
                {
                    try
                    {
                        changeState(embed::EmbedStates::LOADED);
                    }
                    catch( const uno::Exception& e )
                    {
                        SAL_WARN("embeddedobj.ole", "ignoring " << e);
                    }
                }

                if ( !bSuccess )
                {
                    try
                    {
                        // second try the cached replacement image
                        aSize = m_pOleComponent->GetCachedExtent( nAspect ); // will throw an exception in case of failure
                        bSuccess = true;
                    }
                    catch( const uno::Exception& )
                    {
                    }
                }

                if ( !bSuccess )
                {
                    try
                    {
                        // third try the size reported by the object
                        aSize = m_pOleComponent->GetRecommendedExtent( nAspect ); // will throw an exception in case of failure
                        bSuccess = true;
                    }
                    catch( const uno::Exception& )
                    {
                    }
                }

                if ( !bSuccess )
                    throw embed::NoVisualAreaSizeException(
                                    "No size available!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

                aGuard.reset();

                m_aCachedSize = aSize;
                m_nCachedAspect = nAspect;
                m_bHasCachedSize = true;

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
                            "No size available!",
                            static_cast< ::cppu::OWeakObject* >(this) );
        }
    }
    else
#endif
    {
        // return cached value
        if ( !m_bHasCachedSize )
        {
            throw embed::NoVisualAreaSizeException(
                            "No size available!",
                            static_cast< ::cppu::OWeakObject* >(this) );
        }
        SAL_WARN_IF( nAspect != m_nCachedAspect, "embeddedobj.ole", "Unexpected aspect is requested!" );
        aResult = m_aCachedSize;
    }

    return aResult;
}

embed::VisualRepresentation SAL_CALL OleEmbeddedObject::getPreferredVisualRepresentation( sal_Int64 nAspect )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getPreferredVisualRepresentation( nAspect );
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.ole", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    // TODO: if the object has cached representation then it should be returned
    // TODO: if the object has no cached representation and is in loaded state it should switch itself to the running state
    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object is not loaded!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    embed::VisualRepresentation aVisualRepr;

    // TODO: in case of different aspects they must be applied to the mediatype and XTransferable must be used
    // the cache is used only as a fallback if object is not in loaded state
    if ( !m_xCachedVisualRepresentation.is() && ( !m_bVisReplInitialized || m_bVisReplInStream )
      && m_nObjectState == embed::EmbedStates::LOADED )
    {
        m_xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( m_xObjectStream, true );
        SetVisReplInStream( m_xCachedVisualRepresentation.is() );
    }

#ifdef _WIN32
    if ( !m_xCachedVisualRepresentation.is() && m_pOleComponent )
    {
        try
        {
            if ( m_nObjectState == embed::EmbedStates::LOADED )
                changeState( embed::EmbedStates::RUNNING );

            datatransfer::DataFlavor aDataFlavor(
                    "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"",
                    "Windows Metafile",
                    cppu::UnoType<uno::Sequence< sal_Int8 >>::get() );

            aVisualRepr.Data = m_pOleComponent->getTransferData( aDataFlavor );
            aVisualRepr.Flavor = aDataFlavor;

            uno::Sequence< sal_Int8 > aVisReplSeq;
            aVisualRepr.Data >>= aVisReplSeq;
            if ( aVisReplSeq.getLength() )
            {
                m_xCachedVisualRepresentation = GetNewFilledTempStream_Impl(
                    uno::Reference< io::XInputStream >(
                        new ::comphelper::SequenceInputStream(aVisReplSeq)));
            }

            return aVisualRepr;
        }
        catch( const uno::Exception& )
        {}
    }
#endif

    // the cache is used only as a fallback if object is not in loaded state
    if ( !m_xCachedVisualRepresentation.is() && ( !m_bVisReplInitialized || m_bVisReplInStream ) )
    {
        m_xCachedVisualRepresentation = TryToRetrieveCachedVisualRepresentation_Impl( m_xObjectStream );
        SetVisReplInStream( m_xCachedVisualRepresentation.is() );
    }

    if ( !m_xCachedVisualRepresentation.is() )
    {
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );
    }

    return GetVisualRepresentationInNativeFormat_Impl( m_xCachedVisualRepresentation );
}

sal_Int32 SAL_CALL OleEmbeddedObject::getMapUnit( sal_Int64 nAspect )
{
    // begin wrapping related part ====================
    uno::Reference< embed::XEmbeddedObject > xWrappedObject = m_xWrappedObject;
    if ( xWrappedObject.is() )
    {
        // the object was converted to OOo embedded object, the current implementation is now only a wrapper
        return xWrappedObject->getMapUnit( nAspect );
    }
    // end wrapping related part ====================

    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    SAL_WARN_IF( nAspect == embed::Aspects::MSOLE_ICON, "embeddedobj.ole", "For iconified objects no graphical replacement is required!" );
    if ( nAspect == embed::Aspects::MSOLE_ICON )
        // no representation can be retrieved
        throw embed::WrongStateException( "Illegal call!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    if ( m_nObjectState == -1 )
        throw embed::WrongStateException( "The object is not loaded!",
                                    static_cast< ::cppu::OWeakObject* >(this) );

    return embed::EmbedMapUnits::ONE_100TH_MM;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
