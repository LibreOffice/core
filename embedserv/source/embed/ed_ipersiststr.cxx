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
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4917 4555)
#endif

#include "embeddoc.hxx"
#include <com/sun/star/uno/Any.h>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/io/XOutputStream.hpp>
#include <com/sun/star/io/XSeekable.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>

#include <comphelper/processfactory.hxx>
#include <osl/mutex.hxx>
#include <osl/diagnose.h>

#include <string.h>

#define EXT_STREAM_LENGTH 16

using namespace ::com::sun::star;

extern ::rtl::OUString  getStorageTypeFromGUID_Impl( GUID* guid );
extern ::rtl::OUString  getServiceNameFromGUID_Impl( GUID* );
extern ::rtl::OUString  getFilterNameFromGUID_Impl( GUID* );
// extern CLIPFORMAT        getClipFormatFromGUID_Impl( GUID* );
::rtl::OUString getTestFileURLFromGUID_Impl( GUID* guid );

const ::rtl::OUString aOfficeEmbedStreamName( RTL_CONSTASCII_USTRINGPARAM ( "package_stream" ) );
const ::rtl::OUString aExtentStreamName( RTL_CONSTASCII_USTRINGPARAM ( "properties_stream" ) );

uno::Reference< io::XInputStream > createTempXInStreamFromIStream(
                                        uno::Reference< lang::XMultiServiceFactory > xFactory,
                                        IStream *pStream )
{
    uno::Reference< io::XInputStream > xResult;

    if ( !pStream )
        return xResult;

    uno::Reference < io::XOutputStream > xTempOut( io::TempFile::create(comphelper::getComponentContext(xFactory)),
                                                            uno::UNO_QUERY_THROW );
    ULARGE_INTEGER nNewPos;
    LARGE_INTEGER aZero = { 0L, 0L };
    HRESULT hr = pStream->Seek( aZero, STREAM_SEEK_SET, &nNewPos );
    if ( FAILED( hr ) ) return xResult;

    STATSTG aStat;
    hr = pStream->Stat( &aStat, STATFLAG_NONAME );
    if ( FAILED( hr ) ) return xResult;

    sal_uInt32 nSize = (sal_uInt32)aStat.cbSize.QuadPart;
    sal_uInt32 nCopied = 0;
    uno::Sequence< sal_Int8 > aBuffer( nConstBufferSize );
    try
    {
        sal_uInt32 nRead = 0;
        do
        {
            pStream->Read( (void*)aBuffer.getArray(), nConstBufferSize, &nRead );

            if ( nRead < nConstBufferSize )
                aBuffer.realloc( nRead );

            xTempOut->writeBytes( aBuffer );
            nCopied += nRead;
        } while( nRead == nConstBufferSize );

        if ( nCopied == nSize )
        {
            uno::Reference < io::XSeekable > xTempSeek ( xTempOut, uno::UNO_QUERY );
            if ( xTempSeek.is() )
            {
                xTempSeek->seek ( 0 );
                xResult = uno::Reference< io::XInputStream >( xTempOut, uno::UNO_QUERY );
            }
        }
    }
    catch( const uno::Exception& )
    {
    }

    return xResult;
}

HRESULT copyXTempOutToIStream( uno::Reference< io::XOutputStream > xTempOut, IStream* pStream )
{
    if ( !xTempOut.is() || !pStream )
        return E_FAIL;

    uno::Reference < io::XSeekable > xTempSeek ( xTempOut, uno::UNO_QUERY );
    if ( !xTempSeek.is() )
        return E_FAIL;

    xTempSeek->seek ( 0 );

    uno::Reference< io::XInputStream > xTempIn ( xTempOut, uno::UNO_QUERY );
    if ( !xTempSeek.is() )
        return E_FAIL;

    // Seek to zero and truncate the stream
    ULARGE_INTEGER nNewPos;
    LARGE_INTEGER aZero = { 0L, 0L };
    HRESULT hr = pStream->Seek( aZero, STREAM_SEEK_SET, &nNewPos );
    if ( FAILED( hr ) ) return E_FAIL;
    ULARGE_INTEGER aUZero = { 0L, 0L };
    hr = pStream->SetSize( aUZero );
    if ( FAILED( hr ) ) return E_FAIL;

    uno::Sequence< sal_Int8 > aBuffer( nConstBufferSize );
    sal_uInt32 nReadBytes = 0;

    do
    {
        try {
            nReadBytes = xTempIn->readBytes( aBuffer, nConstBufferSize );
        }
        catch( const uno::Exception& )
        {
            return E_FAIL;
        }

        sal_uInt32 nWritten = 0;
        HRESULT hr = pStream->Write( (void*)aBuffer.getArray(), nReadBytes, &nWritten );
        if ( !SUCCEEDED( hr ) || nWritten != nReadBytes )
            return E_FAIL;

    } while( nReadBytes == nConstBufferSize );

    return S_OK;
}


//===============================================================================
// EmbedDocument_Impl
//===============================================================================

EmbedDocument_Impl::EmbedDocument_Impl( const uno::Reference< lang::XMultiServiceFactory >& xFactory, const GUID* guid )
: m_refCount( 0L )
, m_xFactory( xFactory )
, m_guid( *guid )
, m_bIsDirty( sal_False )
, m_nAdviseNum( 0 )
, m_bIsInVerbHandling( sal_False )
//, m_bLoadedFromFile( sal_False )
{
    m_xOwnAccess = new EmbeddedDocumentInstanceAccess_Impl( this );
    m_pDocHolder = new DocumentHolder( xFactory, m_xOwnAccess );
    m_pDocHolder->acquire();
}

EmbedDocument_Impl::~EmbedDocument_Impl()
{
    m_pDocHolder->FreeOffice();

    if ( m_pDocHolder->HasFrame() && m_pDocHolder->IsLink() )
    {
        // a link with frame should be only disconnected, not closed
        m_pDocHolder->DisconnectFrameDocument( sal_True );
    }
    else
    {
        m_pDocHolder->CloseDocument();
        m_pDocHolder->CloseFrame();
    }

    m_pDocHolder->release();
}

uno::Sequence< beans::PropertyValue > EmbedDocument_Impl::fillArgsForLoading_Impl( uno::Reference< io::XInputStream > xStream, DWORD /*nStreamMode*/, LPCOLESTR pFilePath )
{
    uno::Sequence< beans::PropertyValue > aArgs( 3 );

    sal_Int32 nInd = 0; // must not be bigger than the preset size
    aArgs[nInd].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "FilterName" ) );
    aArgs[nInd++].Value <<= getFilterNameFromGUID_Impl( &m_guid );

    if ( xStream.is() )
    {
        aArgs[nInd].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "InputStream" ) );
        aArgs[nInd++].Value <<= xStream;
        aArgs[nInd].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "URL" ) );
        aArgs[nInd++].Value <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "private:stream" ) );
    }
    else
    {
        aArgs[nInd].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "URL" ) );

        rtl::OUString sDocUrl;
        if ( pFilePath )
        {
            uno::Reference< util::XURLTransformer > aTransformer( util::URLTransformer::create(comphelper::getComponentContext(m_xFactory)) );
            util::URL aURL;

            aURL.Complete = ::rtl::OUString( reinterpret_cast<const sal_Unicode*>(pFilePath) );

            if ( aTransformer->parseSmart( aURL, ::rtl::OUString() ) )
                sDocUrl = aURL.Complete;
        }

        aArgs[nInd++].Value <<= sDocUrl;
    }

    aArgs.realloc( nInd );

    // aArgs[].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ReadOnly" ) );
    // aArgs[].Value <<= sal_False; //( ( nStreamMode & ( STGM_READWRITE | STGM_WRITE ) ) ? sal_True : sal_False );

    return aArgs;
}

uno::Sequence< beans::PropertyValue > EmbedDocument_Impl::fillArgsForStoring_Impl( uno::Reference< io::XOutputStream > xStream)
{
    uno::Sequence< beans::PropertyValue > aArgs( xStream.is() ? 2 : 1 );

    aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "FilterName" ) );
    aArgs[0].Value <<= getFilterNameFromGUID_Impl( &m_guid );

    if ( xStream.is() )
    {
        aArgs[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "OutputStream" ) );
        aArgs[1].Value <<= xStream;
    }

    return aArgs;
}

HRESULT EmbedDocument_Impl::SaveTo_Impl( IStorage* pStg )
{
    if ( !pStg || pStg == m_pMasterStorage )
        return E_FAIL;

    // for saveto operation the master storage
    // should not enter NoScribble mode
    CComPtr< IStream > pOrigOwn = m_pOwnStream;
    CComPtr< IStream > pOrigExt = m_pExtStream;
    HRESULT hr = Save( pStg, sal_False );
    pStg->Commit( STGC_ONLYIFCURRENT );
    m_pOwnStream = pOrigOwn;
    m_pExtStream = pOrigExt;

    return hr;
}

//-------------------------------------------------------------------------------
// IUnknown

STDMETHODIMP EmbedDocument_Impl::QueryInterface( REFIID riid, void FAR* FAR* ppv )
{
    if(IsEqualIID(riid, IID_IUnknown))
    {
        AddRef();
        *ppv = (IUnknown*) (IPersistStorage*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IPersist))
    {
        AddRef();
        *ppv = (IPersist*) (IPersistStorage*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IExternalConnection))
    {
        AddRef();
        *ppv = (IExternalConnection*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IPersistStorage))
    {
        AddRef();
        *ppv = (IPersistStorage*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IDataObject))
    {
        AddRef();
        *ppv = (IDataObject*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IOleObject))
    {
        AddRef();
        *ppv = (IOleObject*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IOleWindow))
    {
        AddRef();
        *ppv = (IOleWindow*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IOleInPlaceObject))
    {
        AddRef();
        *ppv = (IOleInPlaceObject*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IPersistFile))
    {
        AddRef();
        *ppv = (IPersistFile*) this;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IDispatch))
    {
        AddRef();
        *ppv = (IDispatch*) this;
        return S_OK;
    }

    *ppv = NULL;
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG) EmbedDocument_Impl::AddRef()
{
    return osl_incrementInterlockedCount( &m_refCount);
}

STDMETHODIMP_(ULONG) EmbedDocument_Impl::Release()
{
    // if there is a time when the last reference is destructed, that means that only internal pointers are alive
    // after the following call either the refcount is increased or the pointers are empty
    if ( m_refCount == 1 )
        m_xOwnAccess->ClearEmbedDocument();

    sal_Int32 nCount = osl_decrementInterlockedCount( &m_refCount );
    if ( nCount == 0 )
        delete this;
    return nCount;
}

//-------------------------------------------------------------------------------
// IPersist

STDMETHODIMP EmbedDocument_Impl::GetClassID( CLSID* pClassId )
{
    *pClassId = *&m_guid;
    return S_OK;
}

//-------------------------------------------------------------------------------
// IPersistStorage

STDMETHODIMP EmbedDocument_Impl::IsDirty()
{
    // the link modified state is controlled by the document
    if ( m_bIsDirty && !m_aFileName.getLength() )
        return S_OK;

    uno::Reference< util::XModifiable > xMod( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
    if ( xMod.is() )
        return xMod->isModified() ? S_OK : S_FALSE;
    return S_FALSE;
}

STDMETHODIMP EmbedDocument_Impl::InitNew( IStorage *pStg )
{
    HRESULT hr = CO_E_ALREADYINITIALIZED;

    if ( !m_pDocHolder->GetDocument().is() )
    {

        STATSTG aStat;
        hr = pStg->Stat( &aStat, STATFLAG_NONAME );
        if ( FAILED( hr ) ) return E_FAIL;

        DWORD nStreamMode = aStat.grfMode;

        hr = E_FAIL;
        if ( m_xFactory.is() && pStg )
        {
            uno::Reference< frame::XModel > aDocument(
                            m_xFactory->createInstance( getServiceNameFromGUID_Impl( &m_guid ) ),
                            uno::UNO_QUERY );
            if ( aDocument.is() )
            {
                m_pDocHolder->SetDocument( aDocument );

                uno::Reference< frame::XLoadable > xLoadable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
                if( xLoadable.is() )
                {
                    try
                    {
                        xLoadable->initNew();
                        // xLoadable->load( fillArgsForLoading_Impl( uno::Reference< io::XInputStream >(), nStreamMode ) );
                        hr = S_OK;
                    }
                    catch( const uno::Exception& )
                    {
                    }
                }

                if ( hr == S_OK )
                {
                    ::rtl::OUString aCurType = getStorageTypeFromGUID_Impl( &m_guid ); // ???
                    CLIPFORMAT cf = (CLIPFORMAT)RegisterClipboardFormatA( "Embedded Object" );
                    hr = WriteFmtUserTypeStg( pStg,
                                            cf,                         // ???
                                            reinterpret_cast<LPWSTR>(( sal_Unicode* )aCurType.getStr()) );

                    if ( hr == S_OK )
                    {
                        hr = pStg->CreateStream( reinterpret_cast<LPCWSTR>(aOfficeEmbedStreamName.getStr()),
                                                 STGM_CREATE | ( nStreamMode & 0x73 ),
                                                 0,
                                                 0,
                                                 &m_pOwnStream );

                        if ( hr == S_OK && m_pOwnStream )
                        {
                            hr = pStg->CreateStream( reinterpret_cast<LPCWSTR>(aExtentStreamName.getStr()),
                                                     STGM_CREATE | ( nStreamMode & 0x73 ),
                                                     0,
                                                     0,
                                                     &m_pExtStream );

                            if ( hr == S_OK && m_pExtStream )
                            {

                                m_pMasterStorage = pStg;
                                m_bIsDirty = sal_True;
                            }
                            else
                                hr = E_FAIL;
                        }
                        else
                            hr = E_FAIL;
                    }
                    else
                        hr = E_FAIL;
                }

                if ( hr != S_OK )
                    m_pDocHolder->CloseDocument();
            }
        }
    }

    return hr;
}

STDMETHODIMP EmbedDocument_Impl::Load( IStorage *pStg )
{
    if ( m_pDocHolder->GetDocument().is() )
        return CO_E_ALREADYINITIALIZED;

    if ( !m_xFactory.is() || !pStg )
        return E_FAIL;

    HRESULT hr = E_FAIL;

    STATSTG aStat;
    hr = pStg->Stat( &aStat, STATFLAG_NONAME );
    if ( FAILED( hr ) ) return E_FAIL;

    DWORD nStreamMode = aStat.grfMode;
    hr = pStg->OpenStream( reinterpret_cast<LPCWSTR>(aOfficeEmbedStreamName.getStr()),
                            0,
                            nStreamMode & 0x73,
                            0,
                            &m_pOwnStream );
    if ( !m_pOwnStream ) hr = E_FAIL;

    if ( SUCCEEDED( hr ) )
    {
        hr = pStg->OpenStream( reinterpret_cast<LPCWSTR>(aExtentStreamName.getStr()),
                                0,
                                nStreamMode & 0x73,
                                0,
                                &m_pExtStream );
        if ( !m_pExtStream ) hr = E_FAIL;
    }

    // RECTL aRectToSet;
    SIZEL aSizeToSet;
    if ( SUCCEEDED( hr ) )
    {
        ULARGE_INTEGER nNewPos;
        LARGE_INTEGER aZero = { 0L, 0L };
        hr = m_pExtStream->Seek( aZero, STREAM_SEEK_SET, &nNewPos );
        if ( SUCCEEDED( hr ) )
        {
            sal_uInt32 nRead;
            sal_Int8 aInf[EXT_STREAM_LENGTH];
            hr = m_pExtStream->Read( (void*)aInf, EXT_STREAM_LENGTH, &nRead );
            if ( nRead != EXT_STREAM_LENGTH ) hr = E_FAIL;

            if ( SUCCEEDED( hr ) )
            {
                // aRectToSet.left = *((sal_Int32*)aInf);
                // aRectToSet.top = *((sal_Int32*)&aInf[4]);
                // aRectToSet.right = *((sal_Int32*)&aInf[8]);
                // aRectToSet.bottom = *((sal_Int32*)&aInf[12]);
                aSizeToSet.cx = *((sal_Int32*)&aInf[8]) - *((sal_Int32*)aInf);
                aSizeToSet.cy = *((sal_Int32*)&aInf[12]) - *((sal_Int32*)&aInf[4]);
            }
        }
    }

    if ( SUCCEEDED( hr ) )
    {
        hr = E_FAIL;

        uno::Reference < io::XInputStream > xTempIn = createTempXInStreamFromIStream( m_xFactory, m_pOwnStream );
        if ( xTempIn.is() )
        {
            uno::Reference< frame::XModel > aDocument(
                                                m_xFactory->createInstance( getServiceNameFromGUID_Impl( &m_guid ) ),
                                                uno::UNO_QUERY );
            if ( aDocument.is() )
            {
                m_pDocHolder->SetDocument( aDocument );

                uno::Reference< frame::XLoadable > xLoadable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
                if( xLoadable.is() )
                {
                    try
                    {
                        xLoadable->load( fillArgsForLoading_Impl( xTempIn, nStreamMode ) );
                        m_pMasterStorage = pStg;
                        hr = m_pDocHolder->SetExtent( &aSizeToSet );
                        // hr = m_pDocHolder->SetVisArea( &aRectToSet );
                    }
                    catch( const uno::Exception& )
                    {
                    }
                }

                if ( FAILED( hr ) )
                    m_pDocHolder->CloseDocument();
            }
        }
    }

    if ( FAILED( hr ) )
    {
        m_pOwnStream = CComPtr< IStream >();
        m_pExtStream = CComPtr< IStream >();
        hr = pStg->DestroyElement( reinterpret_cast<LPCWSTR>(aOfficeEmbedStreamName.getStr()) );
        hr = pStg->DestroyElement( reinterpret_cast<LPCWSTR>(aExtentStreamName.getStr()) );

        OSL_ENSURE( SUCCEEDED( hr ), "Can not destroy created stream!\n" );
        if ( FAILED( hr ) )
            hr = E_FAIL;
    }

       return hr;
}

STDMETHODIMP EmbedDocument_Impl::Save( IStorage *pStgSave, BOOL fSameAsLoad )
{
    if ( !m_pDocHolder->GetDocument().is() || !m_xFactory.is() || !pStgSave || !m_pOwnStream || !m_pExtStream )
        return E_FAIL;

    CComPtr< IStream > pTargetStream;
    CComPtr< IStream > pNewExtStream;

    if ( !fSameAsLoad && pStgSave != m_pMasterStorage )
    {
        OSL_ENSURE( m_pMasterStorage, "How could the document be initialized without storage!??\n" );
        HRESULT hr = m_pMasterStorage->CopyTo( NULL, NULL, NULL, pStgSave );
        if ( FAILED( hr ) ) return E_FAIL;

        STATSTG aStat;
        hr = pStgSave->Stat( &aStat, STATFLAG_NONAME );
        if ( FAILED( hr ) ) return E_FAIL;

        DWORD nStreamMode = aStat.grfMode;
        hr = pStgSave->CreateStream( reinterpret_cast<LPCWSTR>(aOfficeEmbedStreamName.getStr()),
                                 STGM_CREATE | ( nStreamMode & 0x73 ),
                                0,
                                 0,
                                 &pTargetStream );
        if ( FAILED( hr ) || !pTargetStream ) return E_FAIL;

        hr = pStgSave->CreateStream( reinterpret_cast<LPCWSTR>(aExtentStreamName.getStr()),
                                 STGM_CREATE | ( nStreamMode & 0x73 ),
                                0,
                                 0,
                                 &pNewExtStream );
        if ( FAILED( hr ) || !pNewExtStream ) return E_FAIL;
    }
    else
    {
        pTargetStream = m_pOwnStream;
        pNewExtStream = m_pExtStream;
    }

    HRESULT hr = E_FAIL;

    uno::Reference < io::XOutputStream > xTempOut( io::TempFile::create(comphelper::getComponentContext(m_xFactory)),
                                                            uno::UNO_QUERY_THROW );

    uno::Reference< frame::XStorable > xStorable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
    if( xStorable.is() )
    {
        try
        {
            xStorable->storeToURL( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "private:stream" ) ),
                                        fillArgsForStoring_Impl( xTempOut ) );
            hr = copyXTempOutToIStream( xTempOut, pTargetStream );
            if ( SUCCEEDED( hr ) )
            {
                // no need to truncate the stream, the size of the stream is always the same
                ULARGE_INTEGER nNewPos;
                LARGE_INTEGER aZero = { 0L, 0L };
                hr = pNewExtStream->Seek( aZero, STREAM_SEEK_SET, &nNewPos );
                if ( SUCCEEDED( hr ) )
                {
                    SIZEL aSize;
                    hr = m_pDocHolder->GetExtent( &aSize );

                    if ( SUCCEEDED( hr ) )
                    {
                        sal_uInt32 nWritten;
                        sal_Int8 aInf[EXT_STREAM_LENGTH];
                        *((sal_Int32*)aInf) = 0;
                        *((sal_Int32*)&aInf[4]) = 0;
                        *((sal_Int32*)&aInf[8]) = aSize.cx;
                        *((sal_Int32*)&aInf[12]) = aSize.cy;

                        hr = pNewExtStream->Write( (void*)aInf, EXT_STREAM_LENGTH, &nWritten );
                        if ( nWritten != EXT_STREAM_LENGTH ) hr = E_FAIL;

                        if ( SUCCEEDED( hr ) )
                        {
                            m_pOwnStream = CComPtr< IStream >();
                            m_pExtStream = CComPtr< IStream >();
                            if ( fSameAsLoad || pStgSave == m_pMasterStorage )
                            {
                                uno::Reference< util::XModifiable > xMod( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
                                if ( xMod.is() )
                                    xMod->setModified( sal_False );
                                m_bIsDirty = sal_False;
                            }
                        }
                    }
                }
            }
        }
        catch( const uno::Exception& )
        {
        }
    }

    return hr;
}

STDMETHODIMP EmbedDocument_Impl::SaveCompleted( IStorage *pStgNew )
{
    // m_pOwnStream == NULL && m_pMasterStorage != NULL means the object is in NoScribble mode
    // m_pOwnStream == NULL && m_pMasterStorage == NULL means the object is in HandsOff mode

    if ( m_pOwnStream || m_pExtStream )
        return E_UNEXPECTED;

    if ( !m_pMasterStorage && !pStgNew )
        return E_INVALIDARG;

    if ( pStgNew )
        m_pMasterStorage = pStgNew;

    STATSTG aStat;
    HRESULT hr = m_pMasterStorage->Stat( &aStat, STATFLAG_NONAME );
    if ( FAILED( hr ) ) return E_OUTOFMEMORY;

    DWORD nStreamMode = aStat.grfMode;
    hr = m_pMasterStorage->OpenStream( reinterpret_cast<LPCWSTR>(aOfficeEmbedStreamName.getStr()),
                                        0,
                                        nStreamMode & 0x73,
                                        0,
                                        &m_pOwnStream );
    if ( FAILED( hr ) || !m_pOwnStream ) return E_OUTOFMEMORY;

    hr = m_pMasterStorage->OpenStream( reinterpret_cast<LPCWSTR>(aExtentStreamName.getStr()),
                                        0,
                                        nStreamMode & 0x73,
                                        0,
                                        &m_pExtStream );
    if ( FAILED( hr ) || !m_pExtStream ) return E_OUTOFMEMORY;

    sal_Bool bModified = sal_False;
    uno::Reference< util::XModifiable > xMod( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
    if ( xMod.is() )
        bModified = xMod->isModified();

    for ( AdviseSinkHashMapIterator iAdvise = m_aAdviseHashMap.begin(); iAdvise != m_aAdviseHashMap.end(); iAdvise++ )
    {
        if ( iAdvise->second )
            iAdvise->second->OnSave();
    }

    if ( xMod.is() )
        bModified = xMod->isModified();

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::HandsOffStorage()
{
    m_pMasterStorage = CComPtr< IStorage >();
    m_pOwnStream = CComPtr< IStream >();
    m_pExtStream = CComPtr< IStream >();

    return S_OK;
}

//-------------------------------------------------------------------------------
// IPersistFile

STDMETHODIMP EmbedDocument_Impl::Load( LPCOLESTR pszFileName, DWORD /*dwMode*/ )
{
    if ( m_pDocHolder->GetDocument().is() )
        return CO_E_ALREADYINITIALIZED;

    if ( !m_xFactory.is() )
        return E_FAIL;

    DWORD nStreamMode = STGM_CREATE | STGM_READWRITE | STGM_DELETEONRELEASE | STGM_SHARE_EXCLUSIVE;
    HRESULT hr = StgCreateDocfile( NULL,
                                     nStreamMode ,
                                     0,
                                     &m_pMasterStorage );

    if ( FAILED( hr ) || !m_pMasterStorage ) return E_FAIL;

    ::rtl::OUString aCurType = getServiceNameFromGUID_Impl( &m_guid ); // ???
    CLIPFORMAT cf = (CLIPFORMAT)RegisterClipboardFormatA( "Embedded Object" );
    hr = WriteFmtUserTypeStg( m_pMasterStorage,
                            cf,                         // ???
                            reinterpret_cast<LPWSTR>(( sal_Unicode* )aCurType.getStr()) );
    if ( FAILED( hr ) ) return E_FAIL;

    hr = m_pMasterStorage->SetClass( m_guid );
    if ( FAILED( hr ) ) return E_FAIL;

    hr = m_pMasterStorage->CreateStream( reinterpret_cast<LPCWSTR>(aOfficeEmbedStreamName.getStr()),
                            STGM_CREATE | ( nStreamMode & 0x73 ),
                            0,
                            0,
                            &m_pOwnStream );
    if ( FAILED( hr ) || !m_pOwnStream ) return E_FAIL;

    hr = m_pMasterStorage->CreateStream( reinterpret_cast<LPCWSTR>(aExtentStreamName.getStr()),
                            STGM_CREATE | ( nStreamMode & 0x73 ),
                            0,
                            0,
                            &m_pExtStream );
    if ( FAILED( hr ) || !m_pExtStream ) return E_FAIL;


    uno::Reference< frame::XModel > aDocument(
                    m_xFactory->createInstance( getServiceNameFromGUID_Impl( &m_guid ) ),
                    uno::UNO_QUERY );
    if ( aDocument.is() )
    {
        m_pDocHolder->SetDocument( aDocument, sal_True );

        uno::Reference< frame::XLoadable > xLoadable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
        if( xLoadable.is() )
        {
            try
            {
                xLoadable->load( fillArgsForLoading_Impl( uno::Reference< io::XInputStream >(),
                                                            STGM_READWRITE,
                                                            pszFileName ) );
                hr = S_OK;

                m_aFileName = ::rtl::OUString( reinterpret_cast<const sal_Unicode*>(pszFileName) );
            }
            catch( const uno::Exception& )
            {
            }
        }

        if ( hr == S_OK )
        {
            ::rtl::OUString aCurType = getServiceNameFromGUID_Impl( &m_guid ); // ???
            CLIPFORMAT cf = (CLIPFORMAT)RegisterClipboardFormatA( "Embedded Object" );
            hr = WriteFmtUserTypeStg( m_pMasterStorage,
                                    cf,                         // ???
                                    reinterpret_cast<LPWSTR>(( sal_Unicode* )aCurType.getStr()) );

            if ( SUCCEEDED( hr ) )
            {
                // no need to truncate the stream, the size of the stream is always the same
                ULARGE_INTEGER nNewPos;
                LARGE_INTEGER aZero = { 0L, 0L };
                hr = m_pExtStream->Seek( aZero, STREAM_SEEK_SET, &nNewPos );
                if ( SUCCEEDED( hr ) )
                {
                    SIZEL aSize;
                    hr = m_pDocHolder->GetExtent( &aSize );

                    if ( SUCCEEDED( hr ) )
                    {
                        sal_uInt32 nWritten;
                        sal_Int8 aInf[EXT_STREAM_LENGTH];
                        *((sal_Int32*)aInf) = 0;
                        *((sal_Int32*)&aInf[4]) = 0;
                        *((sal_Int32*)&aInf[8]) = aSize.cx;
                        *((sal_Int32*)&aInf[12]) = aSize.cy;

                        hr = m_pExtStream->Write( (void*)aInf, EXT_STREAM_LENGTH, &nWritten );
                        if ( nWritten != EXT_STREAM_LENGTH ) hr = E_FAIL;
                    }
                }
            }

            if ( SUCCEEDED( hr ) )
                m_bIsDirty = sal_True;
            else
                hr = E_FAIL;
        }

        if ( FAILED( hr ) )
        {
            m_pDocHolder->CloseDocument();
            m_pOwnStream = NULL;
            m_pExtStream = NULL;
            m_pMasterStorage = NULL;
        }
    }

    return hr;
}

STDMETHODIMP EmbedDocument_Impl::Save( LPCOLESTR pszFileName, BOOL fRemember )
{
    if ( !m_pDocHolder->GetDocument().is() || !m_xFactory.is() )
        return E_FAIL;

    HRESULT hr = E_FAIL;

    // TODO/LATER: currently there is no hands off state implemented
    try
    {
        uno::Reference< frame::XStorable > xStorable( m_pDocHolder->GetDocument(), uno::UNO_QUERY_THROW );

        if ( !pszFileName )
            xStorable->store();
        else
        {
            util::URL aURL;
            aURL.Complete = ::rtl::OUString( reinterpret_cast<const sal_Unicode*>( pszFileName ) );

            uno::Reference< util::XURLTransformer > aTransformer( util::URLTransformer::create(comphelper::getComponentContext(m_xFactory)) );

            if ( aTransformer->parseSmart( aURL, ::rtl::OUString() ) && aURL.Complete.getLength() )
            {
                if ( fRemember )
                {
                    xStorable->storeAsURL( aURL.Complete, fillArgsForStoring_Impl( uno::Reference< io::XOutputStream >() ) );
                    m_aFileName = aURL.Complete;
                }
                else
                    xStorable->storeToURL( aURL.Complete, fillArgsForStoring_Impl( uno::Reference< io::XOutputStream >() ) );
            }
        }

        hr = S_OK;
    }
    catch( const uno::Exception& )
    {
    }

    return hr;
}

STDMETHODIMP EmbedDocument_Impl::SaveCompleted( LPCOLESTR pszFileName )
{
    // the different file name would mean error here
    m_aFileName = ::rtl::OUString( reinterpret_cast<const sal_Unicode*>(pszFileName) );
    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::GetCurFile( LPOLESTR *ppszFileName )
{
    CComPtr<IMalloc> pMalloc;

    HRESULT hr = CoGetMalloc( 1, &pMalloc );
    if ( FAILED( hr ) || !pMalloc ) return E_FAIL;

    *ppszFileName = (LPOLESTR)( pMalloc->Alloc( sizeof( sal_Unicode ) * ( m_aFileName.getLength() + 1 ) ) );
    wcsncpy( *ppszFileName, reinterpret_cast<LPCWSTR>(m_aFileName.getStr()), m_aFileName.getLength() + 1 );

    return m_aFileName.getLength() ? S_OK : S_FALSE;
}

// ===============================================

LockedEmbedDocument_Impl EmbeddedDocumentInstanceAccess_Impl::GetEmbedDocument()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    return LockedEmbedDocument_Impl( m_pEmbedDocument );
}

void EmbeddedDocumentInstanceAccess_Impl::ClearEmbedDocument()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_pEmbedDocument = NULL;
}

// ===============================================

LockedEmbedDocument_Impl::LockedEmbedDocument_Impl()
: m_pEmbedDocument( NULL )
{}

LockedEmbedDocument_Impl::LockedEmbedDocument_Impl( EmbedDocument_Impl* pEmbedDocument )
: m_pEmbedDocument( pEmbedDocument )
{
    if ( m_pEmbedDocument )
        m_pEmbedDocument->AddRef();
}

LockedEmbedDocument_Impl::LockedEmbedDocument_Impl( const LockedEmbedDocument_Impl& aDocLock )
: m_pEmbedDocument( aDocLock.m_pEmbedDocument )
{
    if ( m_pEmbedDocument )
        m_pEmbedDocument->AddRef();
}

LockedEmbedDocument_Impl& LockedEmbedDocument_Impl::operator=( const LockedEmbedDocument_Impl& aDocLock )
{
    if ( m_pEmbedDocument )
        m_pEmbedDocument->Release();

    m_pEmbedDocument = aDocLock.m_pEmbedDocument;
    if ( m_pEmbedDocument )
        m_pEmbedDocument->AddRef();

    return *this;
}

LockedEmbedDocument_Impl::~LockedEmbedDocument_Impl()
{
    if ( m_pEmbedDocument )
        m_pEmbedDocument->Release();
}

void LockedEmbedDocument_Impl::ExecuteMethod( sal_Int16 nId )
{
    if ( m_pEmbedDocument )
    {
        if ( nId == OLESERV_SAVEOBJECT )
            m_pEmbedDocument->SaveObject();
        else if ( nId == OLESERV_CLOSE )
            m_pEmbedDocument->Close( 0 );
        else if ( nId == OLESERV_NOTIFY )
            m_pEmbedDocument->notify();
        else if ( nId == OLESERV_NOTIFYCLOSING )
            m_pEmbedDocument->OLENotifyClosing();
        else if ( nId == OLESERV_SHOWOBJECT )
            m_pEmbedDocument->ShowObject();
        else if ( nId == OLESERV_DEACTIVATE )
            m_pEmbedDocument->Deactivate();
    }
}

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
