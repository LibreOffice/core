/*************************************************************************
 *
 *  $RCSfile: ed_ipersiststr.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: mav $ $Date: 2003-03-25 08:22:13 $
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

#include "embeddoc.hxx"

#ifndef _COM_SUN_STAR_UNO_ANY_H_
#include <com/sun/star/uno/Any.h>
#endif

#ifndef _COM_SUN_STAR_UNO_EXCEPTION_HPP_
#include <com/sun/star/uno/Exception.hpp>
#endif

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XCOMPONENT_HPP_
#include <com/sun/star/lang/XComponent.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XOUTPUTSTREAM_HPP_
#include <com/sun/star/io/XOutputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_IO_XSEEKABLE_HPP_
#include <com/sun/star/io/XSeekable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XLOADABLE_HPP_
#include <com/sun/star/frame/XLoadable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XCOMPONENTLOADER_HPP_
#include <com/sun/star/frame/XComponentLoader.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XURLTRANSFORMER_HPP_
#include <com/sun/star/util/XUrlTransformer.hpp>
#endif


#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

#include <string.h>

using namespace ::com::sun::star;

extern ::rtl::OUString  getServiceNameFromGUID_Impl( GUID* );
extern ::rtl::OUString  getFilterNameFromGUID_Impl( GUID* );
// extern CLIPFORMAT        getClipFormatFromGUID_Impl( GUID* );
::rtl::OUString getTestFileURLFromGUID_Impl( GUID* guid );

const ::rtl::OUString aOfficeEmbedStreamName( RTL_CONSTASCII_USTRINGPARAM ( "package_stream" ) );

uno::Reference< io::XInputStream > createTempXInStreamFromIStream(
                                        uno::Reference< lang::XMultiServiceFactory > xFactory,
                                        IStream *pStream )
{
    uno::Reference< io::XInputStream > xResult;

    if ( !pStream )
        return xResult;

    const ::rtl::OUString aServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.TempFile" ) );
    uno::Reference < io::XOutputStream > xTempOut = uno::Reference < io::XOutputStream > (
                                                            xFactory->createInstance ( aServiceName ),
                                                            uno::UNO_QUERY );
    if ( xTempOut.is() )
    {
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
        catch( uno::Exception& )
        {
        }
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

    ULARGE_INTEGER nNewPos;
    LARGE_INTEGER aZero = { 0L, 0L };
    HRESULT hr = pStream->Seek( aZero, STREAM_SEEK_SET, &nNewPos );
    if ( FAILED( hr ) ) return E_FAIL;

    uno::Sequence< sal_Int8 > aBuffer( nConstBufferSize );
    sal_uInt32 nReadBytes = 0;

    do
    {
        try {
            nReadBytes = xTempIn->readBytes( aBuffer, nConstBufferSize );
        }
        catch( uno::Exception& )
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
//, m_bLoadedFromFile( sal_False )
{
    m_pDocHolder = new DocumentHolder( xFactory );
    m_pDocHolder->acquire();
}

EmbedDocument_Impl::~EmbedDocument_Impl()
{
    m_pDocHolder->CloseDocument();
    m_pDocHolder->FreeOffice();
    m_pDocHolder->release();
}

uno::Sequence< beans::PropertyValue > EmbedDocument_Impl::fillArgsForLoading_Impl( uno::Reference< io::XInputStream > xStream, DWORD nStreamMode, LPCOLESTR pFilePath )
{
    uno::Sequence< beans::PropertyValue > aArgs( 4 );

    aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "FilterName" ) );
    aArgs[0].Value <<= getFilterNameFromGUID_Impl( &m_guid );
    aArgs[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ReadOnly" ) );
    aArgs[1].Value <<= ( ( nStreamMode & ( STGM_READWRITE | STGM_WRITE ) ) ? sal_True : sal_False );

    if ( xStream.is() )
    {
        aArgs[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "InputStream" ) );
        aArgs[2].Value <<= xStream;
        aArgs[3].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "URL" ) );
        aArgs[3].Value <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "private:stream" ) );
    }
    else
    {
        aArgs[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "AsTemplate" ) );
        aArgs[2].Value <<= sal_True;
        aArgs[3].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "URL" ) );

        rtl::OUString sDocUrl;
        if ( pFilePath )
        {

            ::rtl::OUString aServiceName( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.util.URLTransformer" ) );
            uno::Reference< util::XURLTransformer > aTransformer( m_xFactory->createInstance( aServiceName ),
                                                                    uno::UNO_QUERY );
            if ( aTransformer.is() )
            {
                util::URL aURL;

                USES_CONVERSION;
                aURL.Complete = ::rtl::OUString( OLE2CW( pFilePath ) );

                if ( aTransformer->parseSmart( aURL, ::rtl::OUString() ) )
                    sDocUrl = aURL.Complete;
            }
        }
        else // REMOVE
            sDocUrl = getTestFileURLFromGUID_Impl( &m_guid ); // REMOVE

        aArgs[3].Value <<= sDocUrl;
    }

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
    HRESULT hr = Save( pStg, sal_False );
    m_pOwnStream = pOrigOwn;

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
    else if (IsEqualIID(riid, IID_IPersistFile))
    {
        AddRef();
        *ppv = (IPersistFile*) this;
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
    ::osl::MutexGuard aGuard( ::osl::Mutex::getGlobalMutex());
    m_refCount--;
    if ( m_refCount == 0 )
    {
        delete this;
    }

    return m_refCount;
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
    return m_bIsDirty ? S_OK : S_FALSE;
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
                        // xLoadable->initNew();
                        xLoadable->load( fillArgsForLoading_Impl( uno::Reference< io::XInputStream >(), nStreamMode ) );
                        hr = S_OK;
                    }
                    catch( uno::Exception& )
                    {
                    }
                }

                if ( hr == S_OK )
                {
                    ::rtl::OUString aCurType = getServiceNameFromGUID_Impl( &m_guid ); // ???
                    CLIPFORMAT cf = RegisterClipboardFormatA( "Embedded Object" );
                    hr = WriteFmtUserTypeStg( pStg,
                                            cf,                         // ???
                                            ( sal_Unicode* )aCurType.getStr() );

                    if ( hr == S_OK )
                    {
                        hr = pStg->CreateStream( aOfficeEmbedStreamName,
                                                 STGM_CREATE | ( nStreamMode & 0x73 ),
                                                 0,
                                                 0,
                                                 &m_pOwnStream );

                        if ( hr == S_OK && m_pOwnStream )
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
    hr = pStg->OpenStream( aOfficeEmbedStreamName,
                            0,
                            nStreamMode & 0x73,
                            0,
                            &m_pOwnStream );
    if ( FAILED( hr ) || !m_pOwnStream ) return E_FAIL;

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
                    hr = S_OK;
                }
                catch( uno::Exception& )
                {
                }
            }

            if ( FAILED( hr ) )
                m_pDocHolder->CloseDocument();
        }
    }

    if ( FAILED( hr ) )
    {
        m_pOwnStream = CComPtr< IStream >();
        hr = pStg->DestroyElement( aOfficeEmbedStreamName );

        OSL_ENSURE( SUCCEEDED( hr ), "Can not destroy created stream!\n" );
        if ( FAILED( hr ) )
            hr = E_FAIL;
    }

    return hr;
}

STDMETHODIMP EmbedDocument_Impl::Save( IStorage *pStgSave, BOOL fSameAsLoad )
{
    if ( !m_pDocHolder->GetDocument().is() || !m_xFactory.is() || !pStgSave || !m_pOwnStream )
        return E_FAIL;

    CComPtr< IStream > pTargetStream;

    if ( !fSameAsLoad && pStgSave != m_pMasterStorage )
    {
        OSL_ENSURE( m_pMasterStorage, "How could the document be initialized without storage!??\n" );
        HRESULT hr = m_pMasterStorage->CopyTo( NULL, NULL, NULL, pStgSave );
        if ( FAILED( hr ) ) return E_FAIL;

        STATSTG aStat;
        hr = pStgSave->Stat( &aStat, STATFLAG_NONAME );
        if ( FAILED( hr ) ) return E_FAIL;

        DWORD nStreamMode = aStat.grfMode;
        hr = pStgSave->CreateStream( aOfficeEmbedStreamName,
                                 STGM_CREATE | ( nStreamMode & 0x73 ),
                                0,
                                 0,
                                 &pTargetStream );
        if ( FAILED( hr ) || !m_pOwnStream ) return E_FAIL;
    }
    else
        pTargetStream = m_pOwnStream;

    HRESULT hr = E_FAIL;

    const ::rtl::OUString aServiceName ( RTL_CONSTASCII_USTRINGPARAM ( "com.sun.star.io.TempFile" ) );
    uno::Reference < io::XOutputStream > xTempOut = uno::Reference < io::XOutputStream > (
                                                            m_xFactory->createInstance ( aServiceName ),
                                                            uno::UNO_QUERY );

    if ( xTempOut.is() )
    {
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
                    m_pOwnStream = CComPtr< IStream >();
                    if ( fSameAsLoad || pStgSave == m_pMasterStorage )
                        m_bIsDirty = sal_False;

                }
            }
            catch( uno::Exception& )
            {
            }
        }
    }

    return hr;
}

STDMETHODIMP EmbedDocument_Impl::SaveCompleted( IStorage *pStgNew )
{
    // m_pOwnStream == NULL && m_pMasterStorage != NULL means the object is in NoScribble mode
    // m_pOwnStream == NULL && m_pMasterStorage == NULL means the object is in HandsOff mode

    if ( m_pOwnStream )
        return E_UNEXPECTED;

    if ( !m_pMasterStorage && !pStgNew )
        return E_INVALIDARG;

    if ( pStgNew )
        m_pMasterStorage = pStgNew;

    STATSTG aStat;
    HRESULT hr = m_pMasterStorage->Stat( &aStat, STATFLAG_NONAME );
    if ( FAILED( hr ) ) return E_OUTOFMEMORY;

    DWORD nStreamMode = aStat.grfMode;
    hr = m_pMasterStorage->OpenStream( aOfficeEmbedStreamName,
                                        0,
                                        nStreamMode & 0x73,
                                        0,
                                        &m_pOwnStream );
    if ( FAILED( hr ) || !m_pOwnStream ) return E_OUTOFMEMORY;

    for ( AdviseSinkHashMapIterator iAdvise = m_aAdviseHashMap.begin(); iAdvise != m_aAdviseHashMap.end(); iAdvise++ )
    {
        if ( iAdvise->second )
            iAdvise->second->OnSave();
    }

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::HandsOffStorage()
{
    m_pMasterStorage = CComPtr< IStorage >();
    m_pOwnStream = CComPtr< IStream >();

    return S_OK;
}

//-------------------------------------------------------------------------------
// IPersistFile

STDMETHODIMP EmbedDocument_Impl::Load( LPCOLESTR pszFileName, DWORD dwMode )
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
    CLIPFORMAT cf = RegisterClipboardFormatA( "Embedded Object" );
    hr = WriteFmtUserTypeStg( m_pMasterStorage,
                            cf,                         // ???
                            ( sal_Unicode* )aCurType.getStr() );
    if ( FAILED( hr ) ) return E_FAIL;

    hr = m_pMasterStorage->SetClass( m_guid );
    if ( FAILED( hr ) ) return E_FAIL;

    hr = m_pMasterStorage->CreateStream( aOfficeEmbedStreamName,
                            STGM_CREATE | ( nStreamMode & 0x73 ),
                            0,
                            0,
                            &m_pOwnStream );
    if ( FAILED( hr ) || !m_pOwnStream ) return E_FAIL;

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
                xLoadable->load( fillArgsForLoading_Impl( uno::Reference< io::XInputStream >(),
                                                            STGM_READWRITE,
                                                            pszFileName ) );
                hr = S_OK;

                USES_CONVERSION;
                m_aFileName = ::rtl::OUString( OLE2CW( pszFileName ) );
            }
            catch( uno::Exception& )
            {
            }
        }

        if ( hr == S_OK )
        {
            ::rtl::OUString aCurType = getServiceNameFromGUID_Impl( &m_guid ); // ???
            CLIPFORMAT cf = RegisterClipboardFormatA( "Embedded Object" );
            hr = WriteFmtUserTypeStg( m_pMasterStorage,
                                    cf,                         // ???
                                    ( sal_Unicode* )aCurType.getStr() );

            if ( hr == S_OK )
            {
                m_bIsDirty = sal_True;
            }
            else
                hr = E_FAIL;
        }

        if ( hr != S_OK )
        {
            m_pDocHolder->CloseDocument();
            m_pOwnStream = NULL;
            m_pMasterStorage = NULL;
        }
    }

    return hr;
}

STDMETHODIMP EmbedDocument_Impl::Save( LPCOLESTR pszFileName, BOOL fRemember )
{
    if ( !m_pDocHolder->GetDocument().is() || !m_xFactory.is() )
        return E_FAIL;

    USES_CONVERSION;
    ::rtl::OUString aTargetName = pszFileName ? ::rtl::OUString( OLE2CW( pszFileName ) ) : m_aFileName;
    if ( !aTargetName.getLength() )
        return E_FAIL;

    uno::Reference< frame::XStorable > xStorable( m_pDocHolder->GetDocument(), uno::UNO_QUERY );
    if( xStorable.is() )
    {
        try
        {
            xStorable->storeToURL( aTargetName, fillArgsForStoring_Impl( uno::Reference< io::XOutputStream >() ) );

            m_aFileName = ::rtl::OUString();

            if ( !pszFileName || fRemember )
                m_bIsDirty = sal_False;
        }
        catch( uno::Exception& )
        {
        }
    }

    return E_FAIL;
}

STDMETHODIMP EmbedDocument_Impl::SaveCompleted( LPCOLESTR pszFileName )
{
    m_aFileName = ::rtl::OUString( OLE2CW( pszFileName ) );
    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::GetCurFile( LPOLESTR *ppszFileName )
{
    CComPtr<IMalloc> pMalloc;

    HRESULT hr = CoGetMalloc( 1, &pMalloc );
    if ( FAILED( hr ) || !pMalloc ) return E_FAIL;

    *ppszFileName = (LPOLESTR)( pMalloc->Alloc( sizeof( sal_Unicode ) * ( m_aFileName.getLength() + 1 ) ) );
    wcsncpy( *ppszFileName, m_aFileName.getStr(), m_aFileName.getLength() + 1 );

    return m_aFileName.getLength() ? S_OK : S_FALSE;
}

