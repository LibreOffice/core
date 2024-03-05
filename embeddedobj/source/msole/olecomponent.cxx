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

#include <sal/config.h>

#include <com/sun/star/datatransfer/UnsupportedFlavorException.hpp>
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#include <com/sun/star/lang/DisposedException.hpp>
#include <com/sun/star/embed/WrongStateException.hpp>
#include <com/sun/star/embed/UnreachableStateException.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/TempFile.hpp>
#include <com/sun/star/io/XTruncate.hpp>
#include <com/sun/star/io/IOException.hpp>
#include <com/sun/star/awt/XRequestCallback.hpp>

#include "platform.h"
#include <comphelper/multicontainer2.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/servicehelper.hxx>
#include <comphelper/windowserrorstring.hxx>
#include <osl/file.hxx>
#include <rtl/ref.hxx>
#include <o3tl/char16_t2wchar_t.hxx>
#include <o3tl/unit_conversion.hxx>
#include <systools/win32/comtools.hxx>
#include <vcl/threadex.hxx>

#include "graphconvert.hxx"
#include "olecomponent.hxx"
#include "olepersist.hxx"
#include "olewrapclient.hxx"
#include "advisesink.hxx"
#include <oleembobj.hxx>
#include "mtnotification.hxx"
#include <memory>
#include <string>

using namespace ::com::sun::star;
using namespace ::comphelper;
#define     MAX_ENUM_ELE     20
#define     FORMATS_NUM      3

typedef std::vector< FORMATETC* > FormatEtcList;

FORMATETC const pFormatTemplates[FORMATS_NUM] = {
                    { CF_ENHMETAFILE, nullptr, 0, -1, TYMED_ENHMF },
                    { CF_METAFILEPICT, nullptr, 0, -1, TYMED_MFPICT },
                    { CF_BITMAP, nullptr, 0, -1, TYMED_GDI } };


struct OleComponentNative_Impl {
    sal::systools::COMReference< IUnknown > m_pObj;
    sal::systools::COMReference< IOleObject > m_pOleObject;
    sal::systools::COMReference< IViewObject2 > m_pViewObject2;
    sal::systools::COMReference< IStorage > m_pIStorage;
    FormatEtcList m_aFormatsList;
    uno::Sequence< datatransfer::DataFlavor > m_aSupportedGraphFormats;

    OleComponentNative_Impl()
    {
        // TODO: Extend format list
        m_aSupportedGraphFormats = {

        datatransfer::DataFlavor(
            "application/x-openoffice-emf;windows_formatname=\"Image EMF\"",
            "Windows Enhanced Metafile",
            cppu::UnoType<uno::Sequence< sal_Int8 >>::get() ),

        datatransfer::DataFlavor(
            "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"",
            "Windows Metafile",
            cppu::UnoType<uno::Sequence< sal_Int8 >>::get() ),

        datatransfer::DataFlavor(
            "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"",
            "Bitmap",
            cppu::UnoType<uno::Sequence< sal_Int8 >>::get() ),

        datatransfer::DataFlavor(
            "image/png",
            "PNG",
            cppu::UnoType<uno::Sequence< sal_Int8 >>::get() ),

        datatransfer::DataFlavor(
            "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"",
            "GDIMetafile",
            cppu::UnoType<uno::Sequence< sal_Int8 >>::get() )
        };
    }

    bool ConvertDataForFlavor( const STGMEDIUM& aMedium,
                                    const datatransfer::DataFlavor& aFlavor,
                                    uno::Any& aResult );

    bool GraphicalFlavor( const datatransfer::DataFlavor& aFlavor );

    uno::Sequence< datatransfer::DataFlavor > GetFlavorsForAspects( sal_uInt32 nSupportedAspects );
};


static DWORD GetAspectFromFlavor( const datatransfer::DataFlavor& aFlavor )
{
    if ( aFlavor.MimeType.indexOf( ";Aspect=THUMBNAIL" ) != -1 )
        return DVASPECT_THUMBNAIL;
    else if ( aFlavor.MimeType.indexOf( ";Aspect=ICON" ) != -1 )
        return DVASPECT_ICON;
    else if ( aFlavor.MimeType.indexOf( ";Aspect=DOCPRINT" ) != -1 )
        return DVASPECT_DOCPRINT;
    else
        return DVASPECT_CONTENT;
}


static OUString GetFlavorSuffixFromAspect( DWORD nAsp )
{
    OUString aResult;

    if ( nAsp == DVASPECT_THUMBNAIL )
        aResult = ";Aspect=THUMBNAIL";
    else if ( nAsp == DVASPECT_ICON )
        aResult = ";Aspect=ICON";
    else if ( nAsp == DVASPECT_DOCPRINT )
        aResult = ";Aspect=DOCPRINT";

    // no suffix for DVASPECT_CONTENT

    return aResult;
}


static HRESULT OpenIStorageFromURL_Impl( const OUString& aURL, IStorage** ppIStorage )
{
    OSL_ENSURE( ppIStorage, "The pointer must not be empty!" );

    OUString aFilePath;
    if ( !ppIStorage || ::osl::FileBase::getSystemPathFromFileURL( aURL, aFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happened

    return StgOpenStorage( o3tl::toW(aFilePath.getStr()),
                             nullptr,
                             STGM_READWRITE | STGM_TRANSACTED, // | STGM_DELETEONRELEASE,
                             nullptr,
                             0,
                             ppIStorage );
}


bool OleComponentNative_Impl::ConvertDataForFlavor( const STGMEDIUM& aMedium,
                                                        const datatransfer::DataFlavor& aFlavor,
                                                        uno::Any& aResult )
{
    bool bAnyIsReady = false;

    // try to convert data from Medium format to specified Flavor format
    if ( aFlavor.DataType == cppu::UnoType<uno::Sequence< sal_Int8 >>::get() )
    {
        // first the GDI-metafile must be generated

        std::unique_ptr<sal_Int8[]> pBuf;
        sal_uInt32 nBufSize = 0;
        OUString aFormat;

        if ( aMedium.tymed == TYMED_MFPICT ) // Win Metafile
        {
            aFormat = "image/x-wmf";
            METAFILEPICT* pMF = static_cast<METAFILEPICT*>(GlobalLock( aMedium.hMetaFilePict ));
            if ( pMF )
            {
                nBufSize = GetMetaFileBitsEx( pMF->hMF, 0, nullptr ) + 22;
                pBuf.reset(new sal_Int8[nBufSize]);


                // TODO/LATER: the unit size must be calculated correctly
                *reinterpret_cast<long*>( pBuf.get() ) = 0x9ac6cdd7L;
                *reinterpret_cast<short*>( pBuf.get()+6 ) = SHORT(0);
                *reinterpret_cast<short*>( pBuf.get()+8 ) = SHORT(0);
                *reinterpret_cast<short*>( pBuf.get()+10 ) = static_cast<SHORT>(pMF->xExt);
                *reinterpret_cast<short*>( pBuf.get()+12 ) = static_cast<SHORT>(pMF->yExt);
                *reinterpret_cast<short*>( pBuf.get()+14 ) = USHORT(2540);


                if ( nBufSize && nBufSize == GetMetaFileBitsEx( pMF->hMF, nBufSize - 22, pBuf.get() + 22 ) )
                {
                    if ( aFlavor.MimeType.matchAsciiL( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"", 57 ) )
                    {
                        aResult <<= uno::Sequence< sal_Int8 >( pBuf.get(), nBufSize );
                        bAnyIsReady = true;
                    }
                }

                GlobalUnlock( aMedium.hMetaFilePict );
            }
        }
        else if ( aMedium.tymed == TYMED_ENHMF ) // Enh Metafile
        {
            aFormat = "image/x-emf";
            nBufSize = GetEnhMetaFileBits( aMedium.hEnhMetaFile, 0, nullptr );
            pBuf.reset(new sal_Int8[nBufSize]);
            if ( nBufSize && nBufSize == GetEnhMetaFileBits( aMedium.hEnhMetaFile, nBufSize, reinterpret_cast<LPBYTE>(pBuf.get()) ) )
            {
                if ( aFlavor.MimeType.matchAsciiL( "application/x-openoffice-emf;windows_formatname=\"Image EMF\"", 57 ) )
                {
                    aResult <<= uno::Sequence< sal_Int8 >( pBuf.get(), nBufSize );
                    bAnyIsReady = true;
                }
            }
        }
        else if ( aMedium.tymed == TYMED_GDI ) // Bitmap
        {
            aFormat = "image/x-MS-bmp";

            // Find out size of buffer: deprecated GetBitmapBits does not have a mode to return
            // required buffer size
            BITMAP aBmp;
            GetObjectW(aMedium.hBitmap, sizeof(aBmp), &aBmp);
            nBufSize = aBmp.bmWidthBytes * aBmp.bmHeight;

            pBuf.reset(new sal_Int8[nBufSize]);
            if ( nBufSize && nBufSize == sal::static_int_cast< ULONG >( GetBitmapBits( aMedium.hBitmap, nBufSize, pBuf.get() ) ) )
            {
                if ( aFlavor.MimeType.matchAsciiL( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"", 54 ) )
                {
                    aResult <<= uno::Sequence< sal_Int8 >( pBuf.get(), nBufSize );
                    bAnyIsReady = true;
                }
            }
        }

        if ( pBuf && !bAnyIsReady )
        {
            for (auto const& supportedFormat : m_aSupportedGraphFormats)
                if ( aFlavor.MimeType.match( supportedFormat.MimeType )
                  && aFlavor.DataType == supportedFormat.DataType
                  && aFlavor.DataType == cppu::UnoType<uno::Sequence< sal_Int8 >>::get() )
                {
                    bAnyIsReady = ConvertBufferToFormat( pBuf.get(), nBufSize, aFormat, aResult );
                    break;
                }
        }
    }

    return bAnyIsReady;
}


bool OleComponentNative_Impl::GraphicalFlavor( const datatransfer::DataFlavor& aFlavor )
{
    // Actually all the required graphical formats must be supported
    for (auto const& supportedFormat : m_aSupportedGraphFormats)
         if ( aFlavor.MimeType.match( supportedFormat.MimeType )
          && aFlavor.DataType == supportedFormat.DataType )
            return true;

    return false;
}


static bool GetClassIDFromSequence_Impl( uno::Sequence< sal_Int8 > const & aSeq, CLSID& aResult )
{
    if ( aSeq.getLength() == 16 )
    {
        aResult.Data1 = ( ( ( ( ( static_cast<sal_uInt8>(aSeq[0]) << 8 ) + static_cast<sal_uInt8>(aSeq[1]) ) << 8 ) + static_cast<sal_uInt8>(aSeq[2]) ) << 8 ) + static_cast<sal_uInt8>(aSeq[3]);
        aResult.Data2 = ( static_cast<sal_uInt8>(aSeq[4]) << 8 ) + static_cast<sal_uInt8>(aSeq[5]);
        aResult.Data3 = ( static_cast<sal_uInt8>(aSeq[6]) << 8 ) + static_cast<sal_uInt8>(aSeq[7]);
        for( int nInd = 0; nInd < 8; nInd++ )
            aResult.Data4[nInd] = static_cast<sal_uInt8>(aSeq[nInd+8]);

        return true;
    }

    return false;
}


static OUString WinAccToVcl_Impl( const sal_Unicode* pStr )
{
    OUString aResult;

    if( pStr )
    {
        while ( *pStr )
        {
            if ( *pStr == '&' )
            {
                aResult += "~";
                while( *( ++pStr ) == '&' );
            }
            else
            {
                aResult += OUStringChar( *pStr );
                pStr++;
            }
        }
    }

    return aResult;
}


OleComponent::OleComponent( const uno::Reference< uno::XComponentContext >& xContext, OleEmbeddedObject* pUnoOleObject )
: m_pInterfaceContainer( nullptr )
, m_bDisposed( false )
, m_bModified( false )
, m_pNativeImpl( new OleComponentNative_Impl() )
, m_pUnoOleObject( pUnoOleObject )
, m_pOleWrapClientSite( nullptr )
, m_pImplAdviseSink( nullptr )
, m_nOLEMiscFlags( 0 )
, m_nAdvConn( 0 )
, m_xContext( xContext )
, m_bOleInitialized( false )
, m_bWorkaroundActive( false )
{
    OSL_ENSURE( m_pUnoOleObject, "No owner object is provided!" );

    HRESULT hr = OleInitialize( nullptr );
    if ( hr == S_OK || hr == S_FALSE )
        m_bOleInitialized = true;
    else
    {
        SAL_WARN("embeddedobj.ole", "OleComponent ctor: OleInitialize() failed with 0x"
                                        << OUString::number(static_cast<sal_uInt32>(hr), 16) << ": "
                                        << WindowsErrorStringFromHRESULT(hr));
    }

    m_pOleWrapClientSite = new OleWrapperClientSite( this );
    m_pOleWrapClientSite->AddRef();

    m_pImplAdviseSink = new OleWrapperAdviseSink( this );
    m_pImplAdviseSink->AddRef();

}


OleComponent::~OleComponent()
{
    OSL_ENSURE( !m_pOleWrapClientSite && !m_pImplAdviseSink && !m_pInterfaceContainer && !m_bOleInitialized,
                "The object was not closed successfully! DISASTER is possible!" );

    if ( m_pOleWrapClientSite || m_pImplAdviseSink || m_pInterfaceContainer || m_bOleInitialized )
    {
        osl_atomic_increment(&m_refCount);
        try {
            Dispose();
        } catch( const uno::Exception& ) {}
    }

    for (auto const& format : m_pNativeImpl->m_aFormatsList)
    {
        delete format;
    }
    m_pNativeImpl->m_aFormatsList.clear();

    delete m_pNativeImpl;
}

void OleComponent::Dispose()
{
    if ( m_bDisposed )
        return;

    // Call CloseObject() without m_aMutex locked, since it will call
    // IOleObject::Close(), which can call event listeners, which can run on a
    // different thread.
    CloseObject();

    osl::MutexGuard aGuard(m_aMutex);
    if ( m_pOleWrapClientSite )
    {
        m_pOleWrapClientSite->disconnectOleComponent();
        m_pOleWrapClientSite->Release();
        m_pOleWrapClientSite = nullptr;
    }

    if ( m_pImplAdviseSink )
    {
        m_pImplAdviseSink->disconnectOleComponent();
        m_pImplAdviseSink->Release();
        m_pImplAdviseSink = nullptr;
    }

    if ( m_pInterfaceContainer )
    {
        lang::EventObject aEvent( static_cast< ::cppu::OWeakObject* >( this ) );
        m_pInterfaceContainer->disposeAndClear( aEvent );

        delete m_pInterfaceContainer;
        m_pInterfaceContainer = nullptr;
    }

    if ( m_bOleInitialized )
    {
        // since the disposing can happen not only from main thread but also from a clipboard
        // the deinitialization might lead to a disaster, SO7 does not deinitialize OLE at all
        // so currently the same approach is selected as workaround
        // OleUninitialize();
        m_bOleInitialized = false;
    }

    m_bDisposed = true;
}


void OleComponent::disconnectEmbeddedObject()
{
    // must not be called from destructor of UNO OLE object!!!
    osl::MutexGuard aGuard( m_aMutex );
    m_pUnoOleObject = nullptr;
}


void OleComponent::CreateNewIStorage_Impl()
{
    // TODO: in future a global memory could be used instead of file.

    // write the stream to the temporary file
    OUString aTempURL;

    OSL_ENSURE( m_pUnoOleObject, "Unexpected object absence!" );
    if ( m_pUnoOleObject )
        aTempURL = m_pUnoOleObject->CreateTempURLEmpty_Impl();
    else
        aTempURL = GetNewTempFileURL_Impl( m_xContext );

    if ( !aTempURL.getLength() )
        throw uno::RuntimeException(); // TODO

    // open an IStorage based on the temporary file
    OUString aTempFilePath;
    if ( ::osl::FileBase::getSystemPathFromFileURL( aTempURL, aTempFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happened

    HRESULT hr = StgCreateDocfile( o3tl::toW(aTempFilePath.getStr()), STGM_CREATE | STGM_READWRITE | STGM_TRANSACTED | STGM_DELETEONRELEASE, 0, &m_pNativeImpl->m_pIStorage );
    if ( FAILED( hr ) || !m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO: transport error code?
}


uno::Sequence< datatransfer::DataFlavor > OleComponentNative_Impl::GetFlavorsForAspects( sal_uInt32 nSupportedAspects )
{
    uno::Sequence< datatransfer::DataFlavor > aResult;
    for ( sal_uInt32 nAsp = 1; nAsp <= 8; nAsp *= 2 )
        if ( ( nSupportedAspects & nAsp ) == nAsp )
        {
            OUString aAspectSuffix = GetFlavorSuffixFromAspect( nAsp );

            sal_Int32 nLength = aResult.getLength();
            aResult.realloc( nLength + m_aSupportedGraphFormats.getLength() );
            auto pResult = aResult.getArray();

            for ( sal_Int32 nInd = 0; nInd < m_aSupportedGraphFormats.getLength(); nInd++ )
            {
                pResult[nLength + nInd].MimeType = m_aSupportedGraphFormats[nInd].MimeType + aAspectSuffix;
                pResult[nLength + nInd].HumanPresentableName = m_aSupportedGraphFormats[nInd].HumanPresentableName;
                pResult[nLength + nInd].DataType = m_aSupportedGraphFormats[nInd].DataType;
            }
        }

    return aResult;
}


void OleComponent::RetrieveObjectDataFlavors_Impl()
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( !m_aDataFlavors.getLength() )
    {
        sal::systools::COMReference< IDataObject > pDataObject(m_pNativeImpl->m_pObj, sal::systools::COM_QUERY);
        if ( pDataObject )
        {
            sal::systools::COMReference< IEnumFORMATETC > pFormatEnum;
            HRESULT hr = pDataObject->EnumFormatEtc( DATADIR_GET, &pFormatEnum );
            if ( SUCCEEDED( hr ) && pFormatEnum )
            {
                FORMATETC pElem[ MAX_ENUM_ELE ];
                ULONG nNum = 0;

                // if it is possible to retrieve at least one supported graphical format for an aspect
                // this format can be converted to other supported formats
                sal_uInt32 nSupportedAspects = 0;
                do
                {
                    HRESULT hr2 = pFormatEnum->Next( MAX_ENUM_ELE, pElem, &nNum );
                    if( hr2 == S_OK || hr2 == S_FALSE )
                    {
                        for( sal_uInt32 nInd = 0; nInd < FORMATS_NUM; nInd++ )
                        {
                            if ( pElem[nInd].cfFormat == pFormatTemplates[nInd].cfFormat
                              && pElem[nInd].tymed == pFormatTemplates[nInd].tymed )
                                nSupportedAspects |= pElem[nInd].dwAspect;
                        }
                    }
                    else
                        break;
                }
                while( nNum == MAX_ENUM_ELE );

                m_aDataFlavors = m_pNativeImpl->GetFlavorsForAspects( nSupportedAspects );
            }
        }

        if ( !m_aDataFlavors.getLength() )
        {
            // TODO:
            // for any reason the object could not provide this information
            // try to get access to the cached representation
        }
    }
}


bool OleComponent::InitializeObject_Impl()
// There will be no static objects!
{
    if ( !m_pNativeImpl->m_pObj )
        return false;

    // the linked object will be detected here
    OSL_ENSURE( m_pUnoOleObject, "Unexpected object absence!" );
    if ( m_pUnoOleObject )
        m_pUnoOleObject->SetObjectIsLink_Impl( m_pNativeImpl->m_pObj.QueryInterface<IOleLink>(sal::systools::COM_QUERY).is() );

    if ( !m_pNativeImpl->m_pViewObject2.set(m_pNativeImpl->m_pObj, sal::systools::COM_QUERY) )
        return false;

    // remove all the caches
    if ( sal::systools::COMReference< IOleCache > pIOleCache{ m_pNativeImpl->m_pObj, sal::systools::COM_QUERY } )
    {
        IEnumSTATDATA* pEnumSD = nullptr;
        HRESULT hr2 = pIOleCache->EnumCache( &pEnumSD );

        if ( SUCCEEDED( hr2 ) && pEnumSD )
        {
            pEnumSD->Reset();
            STATDATA aSD;
            DWORD nNum;
            while( SUCCEEDED( pEnumSD->Next( 1, &aSD, &nNum ) ) && nNum == 1 )
                hr2 = pIOleCache->Uncache( aSD.dwConnection );
        }

        // No IDataObject implementation, caching must be used instead
        DWORD nConn;
        FORMATETC aFormat = { 0, nullptr, DVASPECT_CONTENT, -1, TYMED_MFPICT };
        hr2 = pIOleCache->Cache( &aFormat, ADVFCACHE_ONSAVE, &nConn );
    }

    if ( !m_pNativeImpl->m_pOleObject.set(m_pNativeImpl->m_pObj, sal::systools::COM_QUERY) )
        return false; // Static objects are not supported, they should be inserted as graphics

    m_pNativeImpl->m_pOleObject->GetMiscStatus( DVASPECT_CONTENT, reinterpret_cast<DWORD*>(&m_nOLEMiscFlags) );
    // TODO: use other misc flags also
    // the object should have drawable aspect even in case it supports only iconic representation
    // if ( m_nOLEMiscFlags & OLEMISC_ONLYICONIC )

    m_pNativeImpl->m_pOleObject->SetClientSite( m_pOleWrapClientSite );

    // the only need in this registration is workaround for close notification
    m_pNativeImpl->m_pOleObject->Advise( m_pImplAdviseSink, reinterpret_cast<DWORD*>(&m_nAdvConn) );
    m_pNativeImpl->m_pViewObject2->SetAdvise( DVASPECT_CONTENT, 0, m_pImplAdviseSink );

    OleSetContainedObject( m_pNativeImpl->m_pOleObject, TRUE );

    return true;
}

namespace
{
    HRESULT OleLoadSeh(LPSTORAGE pIStorage, IUnknown** ppObj)
    {
        HRESULT hr = E_FAIL;
        // tdf#119039: there is a nasty bug in OleLoad, that may call an unpaired
        // IUnknown::Release on pIStorage on STG_E_FILENOTFOUND: see
        // https://developercommunity.visualstudio.com/t/10144795
        // Workaround it here to avoid crash in smart COM pointer destructor that
        // would try to release already released object. Since we don't know if
        // the bug appears each time STG_E_FILENOTFOUND is returned, this might
        // potentially leak the storage object.
        if (pIStorage)
            pIStorage->AddRef();

        __try {
            hr = OleLoad(pIStorage, IID_IUnknown, nullptr, IID_PPV_ARGS_Helper(ppObj));
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            hr = E_FAIL;
        }
        if (pIStorage && hr != STG_E_FILENOTFOUND)
            pIStorage->Release();

        return hr;
    }
}

void OleComponent::LoadEmbeddedObject( const OUString& aTempURL )
{
    if ( !aTempURL.getLength() )
        throw lang::IllegalArgumentException(); // TODO

    if ( m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO the object is already initialized or wrong initialization is done

    // open an IStorage based on the temporary file
    HRESULT hr = OpenIStorageFromURL_Impl( aTempURL, &m_pNativeImpl->m_pIStorage );

    if ( FAILED( hr ) || !m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO: transport error code?

    hr = OleLoadSeh(m_pNativeImpl->m_pIStorage, &m_pNativeImpl->m_pObj);
    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
    {
        throw uno::RuntimeException();
    }

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO
}


void OleComponent::CreateObjectFromClipboard()
{
    if ( m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO:the object is already initialized

    CreateNewIStorage_Impl();
    if ( !m_pNativeImpl->m_pIStorage )
        throw uno::RuntimeException(); // TODO

    IDataObject * pDO = nullptr;
    HRESULT hr = OleGetClipboard( &pDO );
    if( SUCCEEDED( hr ) && pDO )
    {
        hr = OleQueryCreateFromData( pDO );
        if( S_OK == GetScode( hr ) )
        {
            hr = OleCreateFromData( pDO,
                                    IID_IUnknown,
                                    OLERENDER_DRAW, // OLERENDER_FORMAT
                                    nullptr,        // &aFormat,
                                    nullptr,
                                    m_pNativeImpl->m_pIStorage,
                                    reinterpret_cast<void**>(&m_pNativeImpl->m_pObj) );
        }
        else
        {
            // Static objects are not supported
            pDO->Release();
        }
    }

    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
        throw uno::RuntimeException();

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO
}


void OleComponent::CreateNewEmbeddedObject( const uno::Sequence< sal_Int8 >& aSeqCLSID )
{
    CLSID aClsID;

    if ( !GetClassIDFromSequence_Impl( aSeqCLSID, aClsID ) )
        throw lang::IllegalArgumentException(); // TODO

    if ( m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO:the object is already initialized

    CreateNewIStorage_Impl();
    if ( !m_pNativeImpl->m_pIStorage )
        throw uno::RuntimeException(); // TODO

    // FORMATETC aFormat = { CF_METAFILEPICT, NULL, nAspect, -1, TYMED_MFPICT }; // for OLE..._DRAW should be NULL

    HRESULT hr = OleCreate( aClsID,
                            IID_IUnknown,
                            OLERENDER_DRAW, // OLERENDER_FORMAT
                            nullptr,        // &aFormat,
                            nullptr,
                            m_pNativeImpl->m_pIStorage,
                            reinterpret_cast<void**>(&m_pNativeImpl->m_pObj) );

    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
        throw uno::RuntimeException(); // TODO

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO

    // TODO: getExtent???
}


void OleComponent::CreateObjectFromData( const uno::Reference< datatransfer::XTransferable >& )
// Static objects are not supported, they should be inserted as graphics
{
    // TODO: May be this call is useless since there are no static objects
    //       and nonstatic objects will be created based on OLEstorage ( stream ).
    //       ???

    // OleQueryCreateFromData...
}


void OleComponent::CreateObjectFromFile( const OUString& aFileURL )
{
    if ( m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO:the object is already initialized

    CreateNewIStorage_Impl();
    if ( !m_pNativeImpl->m_pIStorage )
        throw uno::RuntimeException(); // TODO:

    OUString aFilePath;
    if ( ::osl::FileBase::getSystemPathFromFileURL( aFileURL, aFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happened

    HRESULT hr = OleCreateFromFile( CLSID_NULL,
                                    o3tl::toW(aFilePath.getStr()),
                                    IID_IUnknown,
                                    OLERENDER_DRAW, // OLERENDER_FORMAT
                                    nullptr,
                                    nullptr,
                                    m_pNativeImpl->m_pIStorage,
                                    reinterpret_cast<void**>(&m_pNativeImpl->m_pObj) );

    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
        throw uno::RuntimeException(); // TODO

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO
}


void OleComponent::CreateLinkFromFile( const OUString& aFileURL )
{
    if ( m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO:the object is already initialized

    CreateNewIStorage_Impl();
    if ( !m_pNativeImpl->m_pIStorage )
        throw uno::RuntimeException(); // TODO:

    OUString aFilePath;
    if ( ::osl::FileBase::getSystemPathFromFileURL( aFileURL, aFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happened

    HRESULT hr = OleCreateLinkToFile( o3tl::toW(aFilePath.getStr()),
                                        IID_IUnknown,
                                        OLERENDER_DRAW, // OLERENDER_FORMAT
                                        nullptr,
                                        nullptr,
                                        m_pNativeImpl->m_pIStorage,
                                        reinterpret_cast<void**>(&m_pNativeImpl->m_pObj) );

    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
        throw uno::RuntimeException(); // TODO

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO
}


void OleComponent::InitEmbeddedCopyOfLink( rtl::Reference<OleComponent> const & pOleLinkComponent )
{
    if ( !pOleLinkComponent || !pOleLinkComponent->m_pNativeImpl->m_pObj )
        throw lang::IllegalArgumentException(); // TODO

    if ( m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO:the object is already initialized

    sal::systools::COMReference< IDataObject > pDataObject(pOleLinkComponent->m_pNativeImpl->m_pObj, sal::systools::COM_QUERY);
    if ( pDataObject && SUCCEEDED( OleQueryCreateFromData( pDataObject ) ) )
    {
        // the object must be already disconnected from the temporary URL
        CreateNewIStorage_Impl();
        if ( !m_pNativeImpl->m_pIStorage )
            throw uno::RuntimeException(); // TODO:

        OleCreateFromData( pDataObject,
                                IID_IUnknown,
                                OLERENDER_DRAW,
                                nullptr,
                                nullptr,
                                m_pNativeImpl->m_pIStorage,
                                reinterpret_cast<void**>(&m_pNativeImpl->m_pObj) );
    }

    if ( !m_pNativeImpl->m_pObj )
    {
        sal::systools::COMReference< IOleLink > pOleLink(m_pNativeImpl->m_pObj, sal::systools::COM_QUERY);
        if ( !pOleLink )
            throw io::IOException(); // TODO: the object doesn't support IOleLink

        sal::systools::COMReference< IMoniker > pMoniker;
        HRESULT hr = pOleLink->GetSourceMoniker( &pMoniker );
        if ( FAILED( hr ) || !pMoniker )
            throw io::IOException(); // TODO: can not retrieve moniker

        // In case of file moniker life is easy : )
        DWORD aMonType = 0;
        hr = pMoniker->IsSystemMoniker( &aMonType );
        if ( SUCCEEDED( hr ) && aMonType == MKSYS_FILEMONIKER )
        {
            sal::systools::COMReference< IMalloc > pMalloc;
            hr = CoGetMalloc( 1, &pMalloc ); // if fails there will be a memory leak
            OSL_ENSURE(SUCCEEDED(hr) && pMalloc, "CoGetMalloc() failed!");

            LPOLESTR pOleStr = nullptr;
            hr = pOleLink->GetSourceDisplayName( &pOleStr );
            if ( SUCCEEDED( hr ) && pOleStr )
            {
                std::wstring aFilePath( pOleStr );
                if ( pMalloc )
                    pMalloc->Free( pOleStr );

                hr = OleCreateFromFile( CLSID_NULL,
                                        aFilePath.c_str(),
                                        IID_IUnknown,
                                        OLERENDER_DRAW, // OLERENDER_FORMAT
                                        nullptr,
                                        nullptr,
                                        m_pNativeImpl->m_pIStorage,
                                        reinterpret_cast<void**>(&m_pNativeImpl->m_pObj) );
            }
        }

        // in case of other moniker types the only way is to get storage
        if ( !m_pNativeImpl->m_pObj )
        {
            sal::systools::COMReference< IBindCtx > pBindCtx;
            hr = CreateBindCtx( 0, &pBindCtx );
            if ( SUCCEEDED( hr ) && pBindCtx )
            {
                sal::systools::COMReference< IStorage > pObjectStorage;
                hr = pMoniker->BindToStorage( pBindCtx, nullptr, IID_IStorage, reinterpret_cast<void**>(&pObjectStorage) );
                if ( SUCCEEDED( hr ) && pObjectStorage )
                {
                    hr = pObjectStorage->CopyTo( 0, nullptr, nullptr, m_pNativeImpl->m_pIStorage );
                    if ( SUCCEEDED( hr ) )
                        hr = OleLoadSeh(m_pNativeImpl->m_pIStorage, &m_pNativeImpl->m_pObj);
                }
            }
        }
    }

    // If object could not be created the only way is to use graphical representation
    if ( !m_pNativeImpl->m_pObj )
        throw uno::RuntimeException(); // TODO

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO
}


void OleComponent::RunObject()
{
    OSL_ENSURE( m_pNativeImpl->m_pOleObject, "The pointer can not be set to NULL here!" );
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( !OleIsRunning( m_pNativeImpl->m_pOleObject ) )
    {
        HRESULT hr = OleRun( m_pNativeImpl->m_pObj );

        if ( FAILED( hr ) )
        {
            OUString error = WindowsErrorStringFromHRESULT(hr);
            if ( hr == REGDB_E_CLASSNOTREG )
            {
                if (auto pOleObj
                    = m_pNativeImpl->m_pObj.QueryInterface<IOleObject>(sal::systools::COM_QUERY))
                {
                    LPOLESTR lpUserType = nullptr;
                    if (SUCCEEDED(pOleObj->GetUserType(USERCLASSTYPE_FULL, &lpUserType)))
                    {
                        error += OUString::Concat("\n") + o3tl::toU(lpUserType);
                        sal::systools::COMReference<IMalloc> pMalloc;
                        hr = CoGetMalloc(1, &pMalloc); // if fails there will be a memory leak
                        SAL_WARN_IF(FAILED(hr) || !pMalloc, "embeddedobj.ole", "CoGetMalloc() failed");
                        if (pMalloc)
                            pMalloc->Free(lpUserType);
                    }
                }
                throw embed::UnreachableStateException(
                    error, getXWeak(), -1,
                    css::embed::EmbedStates::RUNNING); // the object server is not installed
            }
            else
                throw io::IOException(error, getXWeak());
        }
    }
}


awt::Size OleComponent::CalculateWithFactor( const awt::Size& aSize,
                                            const awt::Size& aMultiplier,
                                            const awt::Size& aDivisor )
{
    awt::Size aResult;

    sal_Int64 nWidth = static_cast<sal_Int64>(aSize.Width) * static_cast<sal_Int64>(aMultiplier.Width) / static_cast<sal_Int64>(aDivisor.Width);
    sal_Int64 nHeight = static_cast<sal_Int64>(aSize.Height) * static_cast<sal_Int64>(aMultiplier.Height) / static_cast<sal_Int64>(aDivisor.Height);
    OSL_ENSURE( nWidth < SAL_MAX_INT32 && nWidth > SAL_MIN_INT32
             && nHeight < SAL_MAX_INT32 && nHeight > SAL_MIN_INT32,
             "Unacceptable result size!" );

    aResult.Width = static_cast<sal_Int32>(nWidth);
    aResult.Height = static_cast<sal_Int32>(nHeight);

    return aResult;
}


void OleComponent::CloseObject()
{
    if ( m_pNativeImpl->m_pOleObject && OleIsRunning( m_pNativeImpl->m_pOleObject ) )
        m_pNativeImpl->m_pOleObject->Close( OLECLOSE_NOSAVE ); // must be saved before
}


uno::Sequence< embed::VerbDescriptor > OleComponent::GetVerbList()
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if( !m_aVerbList.getLength() )
    {
        sal::systools::COMReference< IEnumOLEVERB > pEnum;
        if( SUCCEEDED( m_pNativeImpl->m_pOleObject->EnumVerbs( &pEnum ) ) )
        {
            OLEVERB     szEle[ MAX_ENUM_ELE ];
            ULONG       nNum = 0;
            sal_Int32   nSeqSize = 0;

            do
            {
                HRESULT hr = pEnum->Next( MAX_ENUM_ELE, szEle, &nNum );
                if( hr == S_OK || hr == S_FALSE )
                {
                    m_aVerbList.realloc( nSeqSize += nNum );
                    auto pVerbList = m_aVerbList.getArray();
                    for( sal_uInt32 nInd = 0; nInd < nNum; nInd++ )
                    {
                        pVerbList[nSeqSize-nNum+nInd].VerbID = szEle[ nInd ].lVerb;
                        pVerbList[nSeqSize-nNum+nInd].VerbName = WinAccToVcl_Impl( o3tl::toU(szEle[ nInd ].lpszVerbName) );
                        pVerbList[nSeqSize-nNum+nInd].VerbFlags = szEle[ nInd ].fuFlags;
                        pVerbList[nSeqSize-nNum+nInd].VerbAttributes = szEle[ nInd ].grfAttribs;
                    }
                }
                else
                    break;
            }
            while( nNum == MAX_ENUM_ELE );
        }
    }

    return m_aVerbList;
}


void OleComponent::ExecuteVerb( sal_Int32 nVerbID )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO

    HRESULT hr = OleRun( m_pNativeImpl->m_pOleObject );
    if ( FAILED( hr ) )
        throw io::IOException(); // TODO: a specific exception that transport error code can be thrown here

    // TODO: probably extents should be set here and stored in aRect
    // TODO: probably the parent window also should be set
    hr = m_pNativeImpl->m_pOleObject->DoVerb( nVerbID, nullptr, m_pOleWrapClientSite, 0, nullptr, nullptr );

    if ( FAILED( hr ) )
        throw io::IOException(); // TODO
}


void OleComponent::SetHostName( const OUString& aEmbDocName )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    m_pNativeImpl->m_pOleObject->SetHostNames( L"app name", o3tl::toW( aEmbDocName.getStr() ) );
}


void OleComponent::SetExtent( const awt::Size& aVisAreaSize, sal_Int64 nAspect )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nMSAspect = static_cast<DWORD>(nAspect); // first 32 bits are for MS aspects

    SIZEL aSize = { aVisAreaSize.Width, aVisAreaSize.Height };
    HRESULT hr = m_pNativeImpl->m_pOleObject->SetExtent( nMSAspect, &aSize );

    if ( FAILED( hr ) )
    {
        // TODO/LATER: is it correct? In future user code probably should be ready for the exception.
        // if the object is running but not activated, RPC_E_SERVER_DIED error code is returned by OLE package
        // in this case just do nothing
        // Also Visio returns E_FAIL on resize if it is in running state
        // if ( hr != RPC_E_SERVER_DIED )
        throw io::IOException(); // TODO
    }
}


awt::Size OleComponent::GetExtent( sal_Int64 nAspect )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nMSAspect = static_cast<DWORD>(nAspect); // first 32 bits are for MS aspects
    awt::Size aSize;
    bool bGotSize = false;

    if ( nMSAspect == DVASPECT_CONTENT )
    {
        // Try to get the size from the replacement image first
        sal::systools::COMReference< IDataObject > pDataObject(m_pNativeImpl->m_pObj, sal::systools::COM_QUERY);
        if ( pDataObject )
        {
            STGMEDIUM aMedium;
            FORMATETC aFormat = pFormatTemplates[1]; // use windows metafile format
            aFormat.dwAspect = nMSAspect;

            HRESULT hr = pDataObject->GetData( &aFormat, &aMedium );

            if (hr == RPC_E_WRONG_THREAD)
            {
                // Assume that the OLE object was loaded on the main thread.
                vcl::solarthread::syncExecute([this, &hr, &pDataObject, &aFormat, &aMedium]() {
                    // Make sure that the current state is embed::EmbedStates::RUNNING.
                    RunObject();
                    // Now try again on the correct thread.
                    hr = pDataObject->GetData(&aFormat, &aMedium);
                });
            }

            if ( SUCCEEDED( hr ) && aMedium.tymed == TYMED_MFPICT ) // Win Metafile
            {
                METAFILEPICT* pMF = static_cast<METAFILEPICT*>(GlobalLock( aMedium.hMetaFilePict ));
                if ( pMF )
                {
                    // the object uses 0.01 mm as unit, so the metafile size should be converted to object unit
                    o3tl::Length eFrom = o3tl::Length::mm100;
                    switch( pMF->mm )
                    {
                        case MM_HIENGLISH:
                            eFrom = o3tl::Length::in1000;
                            break;

                        case MM_LOENGLISH:
                            eFrom = o3tl::Length::in100;
                            break;

                        case MM_LOMETRIC:
                            eFrom = o3tl::Length::mm10;
                            break;

                        case MM_TWIPS:
                            eFrom = o3tl::Length::twip;
                            break;

                        case MM_ISOTROPIC:
                        case MM_ANISOTROPIC:
                        case MM_HIMETRIC:
                            // do nothing
                            break;
                    }

                    sal_Int64 nX = o3tl::convert(abs( pMF->xExt ), eFrom, o3tl::Length::mm100);
                    sal_Int64 nY = o3tl::convert(abs( pMF->yExt ), eFrom, o3tl::Length::mm100);
                    if (  nX < SAL_MAX_INT32 && nY < SAL_MAX_INT32 )
                    {
                        aSize.Width = static_cast<sal_Int32>(nX);
                        aSize.Height = static_cast<sal_Int32>(nY);
                        bGotSize = true;
                    }
                    else
                        OSL_FAIL( "Unexpected size is provided!" );
                }
            }
            else if (!SUCCEEDED(hr))
            {
                SAL_WARN("embeddedobj.ole", " OleComponent::GetExtent: GetData() failed");
            }
            // i113605, to release storage medium
            if ( SUCCEEDED( hr ) )
                ::ReleaseStgMedium(&aMedium);
        }
    }

    if ( !bGotSize )
        throw lang::IllegalArgumentException();

    return aSize;
}


awt::Size OleComponent::GetCachedExtent( sal_Int64 nAspect )
{
    if (!m_pNativeImpl->m_pViewObject2)
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nMSAspect = static_cast<DWORD>(nAspect); // first 32 bits are for MS aspects
    SIZEL aSize;

    HRESULT hr = m_pNativeImpl->m_pViewObject2->GetExtent( nMSAspect, -1, nullptr, &aSize );

    if ( FAILED( hr ) )
    {
        // TODO/LATER: is it correct?
        // if there is no appropriate cache for the aspect, OLE_E_BLANK error code is returned
        // if ( hr == OLE_E_BLANK )
        //  throw lang::IllegalArgumentException();
        //else
        //  throw io::IOException(); // TODO

        SAL_WARN("embeddedobj.ole", " OleComponent::GetCachedExtent: GetExtent() failed");
        throw lang::IllegalArgumentException();
    }

    return awt::Size( aSize.cx, aSize.cy );
}


awt::Size OleComponent::GetRecommendedExtent( sal_Int64 nAspect )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nMSAspect = static_cast<DWORD>(nAspect); // first 32 bits are for MS aspects
    SIZEL aSize;
    HRESULT hr = m_pNativeImpl->m_pOleObject->GetExtent( nMSAspect, &aSize );
    if ( FAILED( hr ) )
    {
        SAL_WARN("embeddedobj.ole", " OleComponent::GetRecommendedExtent: GetExtent() failed");
        throw lang::IllegalArgumentException();
    }

    return awt::Size( aSize.cx, aSize.cy );
}


sal_Int64 OleComponent::GetMiscStatus( sal_Int64 nAspect )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nResult;
    m_pNativeImpl->m_pOleObject->GetMiscStatus( static_cast<DWORD>(nAspect), &nResult );
    return static_cast<sal_Int64>(nResult); // first 32 bits are for MS flags
}


uno::Sequence< sal_Int8 > OleComponent::GetCLSID()
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    GUID aCLSID;
    HRESULT hr = m_pNativeImpl->m_pOleObject->GetUserClassID( &aCLSID );
    if ( FAILED( hr ) )
        throw io::IOException(); // TODO:

    return  MimeConfigurationHelper::GetSequenceClassID( aCLSID.Data1, aCLSID.Data2, aCLSID.Data3,
                                aCLSID.Data4[0], aCLSID.Data4[1],
                                aCLSID.Data4[2], aCLSID.Data4[3],
                                aCLSID.Data4[4], aCLSID.Data4[5],
                                aCLSID.Data4[6], aCLSID.Data4[7] );
}


bool OleComponent::IsDirty()
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( IsWorkaroundActive() )
        return true;

    sal::systools::COMReference< IPersistStorage > pPersistStorage(m_pNativeImpl->m_pObj, sal::systools::COM_QUERY);
    if ( !pPersistStorage )
        throw io::IOException(); // TODO

    HRESULT hr = pPersistStorage->IsDirty();
    return ( hr != S_FALSE );
}


void OleComponent::StoreOwnTmpIfNecessary()
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    sal::systools::COMReference< IPersistStorage > pPersistStorage(m_pNativeImpl->m_pObj, sal::systools::COM_QUERY);
    if ( !pPersistStorage )
        throw io::IOException(); // TODO

    if ( m_bWorkaroundActive || pPersistStorage->IsDirty() != S_FALSE )
    {
        HRESULT hr = OleSave( pPersistStorage, m_pNativeImpl->m_pIStorage, TRUE );
        if ( FAILED( hr ) )
        {
            // Till now was required only for AcrobatReader7.0.8
            GUID aCLSID;
            hr = m_pNativeImpl->m_pOleObject->GetUserClassID( &aCLSID );
            if ( FAILED( hr ) )
            {
                SAL_WARN("embeddedobj.ole", "OleComponent::StoreOwnTmpIfNecessary: GetUserClassID() failed");
                throw io::IOException(); // TODO
            }

            hr = WriteClassStg( m_pNativeImpl->m_pIStorage, aCLSID );
            if ( FAILED( hr ) )
                throw io::IOException(); // TODO

            // the result of the following call is not checked because some objects, for example AcrobatReader7.0.8
            // return error even in case the saving was done correctly
            hr = pPersistStorage->Save( m_pNativeImpl->m_pIStorage, TRUE );

            // another workaround for AcrobatReader7.0.8 object, this object might think that it is not changed
            // when it has been created from file, although it must be saved
            m_bWorkaroundActive = true;
        }

        hr = m_pNativeImpl->m_pIStorage->Commit( STGC_DEFAULT );
        if ( FAILED( hr ) )
            throw io::IOException(); // TODO

        hr = pPersistStorage->SaveCompleted( nullptr );
        if ( FAILED( hr ) && hr != E_UNEXPECTED )
            throw io::IOException(); // TODO

    }
}


bool OleComponent::SaveObject_Impl()
{
    bool bResult = false;
    OleEmbeddedObject* pLockObject = nullptr;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pUnoOleObject )
        {
            pLockObject = m_pUnoOleObject;
            pLockObject->acquire();
        }
    }

    if ( pLockObject )
    {
        bResult = pLockObject->SaveObject_Impl();
        pLockObject->release();
    }

    return bResult;
}


bool OleComponent::OnShowWindow_Impl( bool bShow )
{
    bool bResult = false;
    OleEmbeddedObject* pLockObject = nullptr;

    {
        osl::MutexGuard aGuard( m_aMutex );

        if ( m_pUnoOleObject )
        {
            pLockObject = m_pUnoOleObject;
            pLockObject->acquire();
        }
    }

    if ( pLockObject )
    {
        bResult = pLockObject->OnShowWindow_Impl( bShow );
        pLockObject->release();
    }

    return bResult;
}


void OleComponent::OnViewChange_Impl( sal_uInt32 dwAspect )
{
    // TODO: check if it is enough or may be saving notifications are required for Visio2000
    ::rtl::Reference< OleEmbeddedObject > xLockObject;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pUnoOleObject )
            xLockObject = m_pUnoOleObject;
    }

    if ( xLockObject.is() )
    {
        uno::Reference < awt::XRequestCallback > xRequestCallback(
            m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.awt.AsyncCallback", m_xContext),
             uno::UNO_QUERY );
        xRequestCallback->addCallback( new MainThreadNotificationRequest( xLockObject, OLECOMP_ONVIEWCHANGE, dwAspect ), uno::Any() );
    }
}


void OleComponent::OnClose_Impl()
{
    ::rtl::Reference< OleEmbeddedObject > xLockObject;

    {
        osl::MutexGuard aGuard( m_aMutex );
        if ( m_pUnoOleObject )
            xLockObject = m_pUnoOleObject;
    }

    if ( xLockObject.is() )
    {
        uno::Reference < awt::XRequestCallback > xRequestCallback(
            m_xContext->getServiceManager()->createInstanceWithContext("com.sun.star.awt.AsyncCallback", m_xContext),
             uno::UNO_QUERY );
        xRequestCallback->addCallback( new MainThreadNotificationRequest( xLockObject, OLECOMP_ONCLOSE ), uno::Any() );
    }
}

// XCloseable

void SAL_CALL OleComponent::close( sal_Bool bDeliverOwnership )
{
    uno::Reference< uno::XInterface > xSelfHold;
    {
        osl::MutexGuard aGuard(m_aMutex);
        if (m_bDisposed)
            throw lang::DisposedException(); // TODO

        xSelfHold.set(static_cast<::cppu::OWeakObject*>(this));
        lang::EventObject aSource(static_cast<::cppu::OWeakObject*>(this));

        if (m_pInterfaceContainer)
        {
            comphelper::OInterfaceContainerHelper2* pContainer
                = m_pInterfaceContainer->getContainer(cppu::UnoType<util::XCloseListener>::get());
            if (pContainer != nullptr)
            {
                comphelper::OInterfaceIteratorHelper2 pIterator(*pContainer);
                while (pIterator.hasMoreElements())
                {
                    try
                    {
                        static_cast<util::XCloseListener*>(pIterator.next())
                            ->queryClosing(aSource, bDeliverOwnership);
                    }
                    catch (const uno::RuntimeException&)
                    {
                        pIterator.remove();
                    }
                }
            }

            pContainer
                = m_pInterfaceContainer->getContainer(cppu::UnoType<util::XCloseListener>::get());
            if (pContainer != nullptr)
            {
                comphelper::OInterfaceIteratorHelper2 pCloseIterator(*pContainer);
                while (pCloseIterator.hasMoreElements())
                {
                    try
                    {
                        static_cast<util::XCloseListener*>(pCloseIterator.next())
                            ->notifyClosing(aSource);
                    }
                    catch (const uno::RuntimeException&)
                    {
                        pCloseIterator.remove();
                    }
                }
            }
        }
    }

    Dispose();
}


void SAL_CALL OleComponent::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new comphelper::OMultiTypeInterfaceContainerHelper2( m_aMutex );

    m_pInterfaceContainer->addInterface( cppu::UnoType<util::XCloseListener>::get(), xListener );
}


void SAL_CALL OleComponent::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<util::XCloseListener>::get(),
                                                xListener );
}

// XTransferable

uno::Any SAL_CALL OleComponent::getTransferData( const datatransfer::DataFlavor& aFlavor )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    uno::Any aResult;
    bool bSupportedFlavor = false;

    if ( m_pNativeImpl->GraphicalFlavor( aFlavor ) )
    {
        DWORD nRequestedAspect = GetAspectFromFlavor( aFlavor );
        // if own icon is set and icon aspect is requested the own icon can be returned directly

        sal::systools::COMReference< IDataObject > pDataObject(m_pNativeImpl->m_pObj, sal::systools::COM_QUERY);
        if ( !pDataObject )
            throw io::IOException(); // TODO: transport error code

        // The following optimization does not make much sense currently just because
        // only one aspect is supported, and only three formats for the aspect are supported
        // and moreover it is not guaranteed that the once returned format will be supported further
        // example - i52106
        // TODO/LATER: bring the optimization back when other aspects are supported

        // FORMATETC* pFormatEtc = m_pNativeImpl->GetSupportedFormatForAspect( nRequestedAspect );
        // if ( pFormatEtc )
        // {
        //  STGMEDIUM aMedium;
        //  hr = pDataObject->GetData( pFormatEtc, &aMedium );
        //  if ( SUCCEEDED( hr ) )
        //      bSupportedFlavor = m_pNativeImpl->ConvertDataForFlavor( aMedium, aFlavor, aResult );
        // }
        // else
        {
            // the supported format of the application is still not found, find one
            for ( sal_Int32 nInd = 0; nInd < FORMATS_NUM; nInd++ )
            {
                STGMEDIUM aMedium;
                FORMATETC aFormat = pFormatTemplates[nInd];
                aFormat.dwAspect = nRequestedAspect;

                HRESULT hr = pDataObject->GetData( &aFormat, &aMedium );
                if ( SUCCEEDED( hr ) )
                {
                    bSupportedFlavor = m_pNativeImpl->ConvertDataForFlavor( aMedium, aFlavor, aResult );
                    ::ReleaseStgMedium(&aMedium);     // i113605, to release storage medium
                    if ( bSupportedFlavor )
                    {
                        // TODO/LATER: bring the optimization back when other aspects are supported
                        // m_pNativeImpl->AddSupportedFormat( aFormat );
                        break;
                    }
                }
            }
        }

        // If the replacement could not be retrieved, the cached representation should be used
        // currently it is not necessary to retrieve it here, so it is implemented in the object itself
    }
    // TODO: Investigate if there is already the format name
    //       and whether this format is really required
    else if ( aFlavor.DataType == cppu::UnoType<io::XInputStream>::get()
            && aFlavor.MimeType == "application/x-openoffice-contentstream" )
    {
        // allow to retrieve stream-representation of the object persistence
        bSupportedFlavor = true;
        uno::Reference < io::XStream > xTempFileStream(
            io::TempFile::create(m_xContext),
            uno::UNO_QUERY_THROW );

        uno::Reference< io::XOutputStream > xTempOutStream = xTempFileStream->getOutputStream();
        uno::Reference< io::XInputStream > xTempInStream = xTempFileStream->getInputStream();
        if ( !(xTempOutStream.is() && xTempInStream.is()) )
            throw io::IOException(); // TODO:

        OSL_ENSURE( m_pUnoOleObject, "Unexpected object absence!" );
        if ( !m_pUnoOleObject )
            throw uno::RuntimeException();

        m_pUnoOleObject->StoreObjectToStream( xTempOutStream );

        xTempOutStream->closeOutput();
        xTempOutStream.clear();

        aResult <<= xTempInStream;
    }

    if ( !bSupportedFlavor )
        throw datatransfer::UnsupportedFlavorException();

    return aResult;
}


uno::Sequence< datatransfer::DataFlavor > SAL_CALL OleComponent::getTransferDataFlavors()
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    RetrieveObjectDataFlavors_Impl();

    return m_aDataFlavors;
}


sal_Bool SAL_CALL OleComponent::isDataFlavorSupported( const datatransfer::DataFlavor& aFlavor )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( !m_aDataFlavors.getLength() )
    {
        RetrieveObjectDataFlavors_Impl();
    }

    for (auto const& supportedFormat : m_aDataFlavors)
        if ( supportedFormat.MimeType.equals( aFlavor.MimeType ) && supportedFormat.DataType == aFlavor.DataType )
            return true;

    return false;
}

void SAL_CALL OleComponent::dispose()
{
    try
    {
        close( true );
    }
    catch ( const uno::Exception& )
    {
    }
}

void SAL_CALL OleComponent::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new comphelper::OMultiTypeInterfaceContainerHelper2( m_aMutex );

    m_pInterfaceContainer->addInterface( cppu::UnoType<lang::XEventListener>::get(), xListener );
}


void SAL_CALL OleComponent::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<lang::XEventListener>::get(),
                                                xListener );
}

sal_Int64 SAL_CALL OleComponent::getSomething( const css::uno::Sequence< sal_Int8 >& aIdentifier )
{
    try
    {
        uno::Sequence < sal_Int8 > aCLSID = GetCLSID();
        if ( MimeConfigurationHelper::ClassIDsEqual( aIdentifier, aCLSID ) )
            return comphelper::getSomething_cast(static_cast<IUnknown*>(m_pNativeImpl->m_pObj));

        // compatibility hack for old versions: CLSID was used in wrong order (SvGlobalName order)
        sal_Int32 nLength = aIdentifier.getLength();
        if ( nLength == 16 )
        {
            for ( sal_Int32 n=8; n<16; n++ )
                if ( aIdentifier[n] != aCLSID[n] )
                    return 0;
            if ( aIdentifier[7] == aCLSID[6] &&
                 aIdentifier[6] == aCLSID[7] &&
                 aIdentifier[5] == aCLSID[4] &&
                 aIdentifier[4] == aCLSID[5] &&
                 aIdentifier[3] == aCLSID[0] &&
                 aIdentifier[2] == aCLSID[1] &&
                 aIdentifier[1] == aCLSID[2] &&
                 aIdentifier[0] == aCLSID[3] )
                return reinterpret_cast<sal_Int64>(static_cast<IUnknown*>(m_pNativeImpl->m_pObj));
        }
    }
    catch ( const uno::Exception& )
    {
    }

    return 0;
}

sal_Bool SAL_CALL OleComponent::isModified()
{
    return m_bModified;
}

void SAL_CALL OleComponent::setModified( sal_Bool bModified )
{
    m_bModified = bModified;

    if ( bModified && m_pInterfaceContainer )
    {
        comphelper::OInterfaceContainerHelper2* pContainer =
            m_pInterfaceContainer->getContainer( cppu::UnoType<util::XModifyListener>::get());
        if ( pContainer != nullptr )
        {
            comphelper::OInterfaceIteratorHelper2 pIterator( *pContainer );
            while ( pIterator.hasMoreElements() )
            {
                try
                {
                    lang::EventObject aEvent( static_cast<util::XModifiable*>(this) );
                    static_cast<util::XModifyListener*>(pIterator.next())->modified( aEvent );
                }
                catch( const uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }
    }
}

void SAL_CALL OleComponent::addModifyListener( const css::uno::Reference < css::util::XModifyListener >& xListener )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new comphelper::OMultiTypeInterfaceContainerHelper2( m_aMutex );

    m_pInterfaceContainer->addInterface( cppu::UnoType<util::XModifyListener>::get(), xListener );
}

void SAL_CALL OleComponent::removeModifyListener( const css::uno::Reference < css::util::XModifyListener >& xListener)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( cppu::UnoType<util::XModifyListener>::get(),
                                                xListener );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
