/*************************************************************************
 *
 *  $RCSfile: ed_ipersiststr.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mav $ $Date: 2003-03-05 15:50:10 $
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

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _OSL_DIAGNOSE_H_
#include <osl/diagnose.h>
#endif

using namespace ::com::sun::star;

extern ::rtl::OUString  getServiceNameFromGUID_Impl( GUID* );
extern ::rtl::OUString  getFilterNameFromGUID_Impl( GUID* );
// extern CLIPFORMAT        getClipFormatFromGUID_Impl( GUID* );

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
{
}

EmbedDocument_Impl::~EmbedDocument_Impl()
{
    if ( m_xDocument.is() )
    {
        uno::Reference< lang::XComponent > xComponent( m_xDocument, uno::UNO_QUERY );

        OSL_ENSURE( xComponent.is(), "Can not dispose created model!\n" );
        if ( xComponent.is() )
            xComponent->dispose();

        m_xDocument = uno::Reference< frame::XModel >();
    }
}

uno::Sequence< beans::PropertyValue > EmbedDocument_Impl::fillArgsForLoading_Impl( uno::Reference< io::XInputStream > xStream, DWORD nStreamMode )
{
    uno::Sequence< beans::PropertyValue > aArgs( 4 );

    aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "URL" ) );
    aArgs[0].Value <<= ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "private:stream" ) );
    aArgs[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "FilterName" ) );
    aArgs[1].Value <<= getFilterNameFromGUID_Impl( &m_guid );
    aArgs[2].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "InputStream" ) );
    aArgs[2].Value <<= xStream;
    aArgs[3].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "ReadOnly" ) );
    aArgs[3].Value <<= ( ( nStreamMode & ( STGM_READWRITE | STGM_WRITE ) ) ? sal_True : sal_False );

    return aArgs;
}

uno::Sequence< beans::PropertyValue > EmbedDocument_Impl::fillArgsForStoring_Impl( uno::Reference< io::XOutputStream > xStream)
{
    uno::Sequence< beans::PropertyValue > aArgs( 2 );

    aArgs[0].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "FilterName" ) );
    aArgs[0].Value <<= getFilterNameFromGUID_Impl( &m_guid );
    aArgs[1].Name = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( "OutputStream" ) );
    aArgs[1].Value <<= xStream;

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

    if ( !m_xDocument.is() )
    {

        STATSTG aStat;
        hr = pStg->Stat( &aStat, STATFLAG_NONAME );
        if ( FAILED( hr ) ) return E_FAIL;

        DWORD nStreamMode = aStat.grfMode;

        hr = E_FAIL;
        if ( m_xFactory.is() && pStg )
        {
               uno::Reference < frame::XComponentLoader > xComponentLoader(
                 m_xFactory->createInstance( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop" ) ) ),
                                             uno::UNO_QUERY );

             rtl::OUString sDocUrl( rtl::OUString::createFromAscii( "file:///d:/test.sxw" ) );

             uno::Reference< lang::XComponent > xComponent = xComponentLoader->loadComponentFromURL(
                 sDocUrl, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM("_blank") ), 0,
                 uno::Sequence < ::com::sun::star::beans::PropertyValue >() );


            m_xDocument = uno::Reference< frame::XModel >( xComponent, uno::UNO_QUERY );
            /*
            m_xDocument = uno::Reference< frame::XModel >(
                            m_xFactory->createInstance( getServiceNameFromGUID_Impl( &m_guid ) ),
                            uno::UNO_QUERY );
            */
            if ( m_xDocument.is() )
            {
                /*
                uno::Reference< frame::XLoadable > xLoadable( m_xDocument, uno::UNO_QUERY );
                if( xLoadable.is() )
                {
                    try
                    {
                        xLoadable->initNew();
                */
                        hr = S_OK;
                /*
                    }
                    catch( uno::Exception& )
                    {
                    }
                }
                */

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
                                                 STGM_CREATE | nStreamMode,
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
                {
                    uno::Reference< lang::XComponent > xComponent( m_xDocument, uno::UNO_QUERY );

                    OSL_ENSURE( xComponent.is(), "Can not dispose created model!\n" );
                    if ( xComponent.is() )
                        xComponent->dispose();

                    m_xDocument = uno::Reference< frame::XModel >();
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP EmbedDocument_Impl::Load( IStorage *pStg )
{

    if ( m_xDocument.is() )
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
                            nStreamMode,
                            0,
                            &m_pOwnStream );
    if ( FAILED( hr ) || !m_pOwnStream ) return E_FAIL;

    hr = E_FAIL;

    uno::Reference < io::XInputStream > xTempIn = createTempXInStreamFromIStream( m_xFactory, m_pOwnStream );
    if ( xTempIn.is() )
    {
        m_xDocument = uno::Reference< frame::XModel >(
                                            m_xFactory->createInstance( getServiceNameFromGUID_Impl( &m_guid ) ),
                                            uno::UNO_QUERY );
        if ( m_xDocument.is() )
        {
            uno::Reference< frame::XLoadable > xLoadable( m_xDocument, uno::UNO_QUERY );
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
            {
                uno::Reference< lang::XComponent > xComponent( m_xDocument, uno::UNO_QUERY );

                OSL_ENSURE( xComponent.is(), "Can not dispose created model!\n" );
                if ( xComponent.is() )
                    xComponent->dispose();

                m_xDocument = uno::Reference< frame::XModel >();
            }
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
    if ( !m_xDocument.is() || !m_xFactory.is() || !pStgSave || !m_pOwnStream )
        return E_FAIL;

    CComPtr< IStream > pTargetStream;

    if ( !fSameAsLoad && pStgSave != m_pMasterStorage )
    {
        OSL_ENSURE( m_pMasterStorage, "How could the document be initialized without storage!??\n" );
        HRESULT hr = m_pMasterStorage->CopyTo( NULL, NULL, NULL, pStgSave );
        if ( FAILED( hr ) ) return E_FAIL;

        hr = pStgSave->OpenStream( aOfficeEmbedStreamName,
                                 0,
                                 STGM_WRITE,
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
        uno::Reference< frame::XStorable > xStorable( m_xDocument, uno::UNO_QUERY );
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
    HRESULT hr = pStgNew->Stat( &aStat, STATFLAG_NONAME );
    if ( FAILED( hr ) ) return E_OUTOFMEMORY;

    DWORD nStreamMode = aStat.grfMode;
    hr = m_pMasterStorage->OpenStream( aOfficeEmbedStreamName,
                                        0,
                                        nStreamMode,
                                        0,
                                        &m_pOwnStream );
    if ( FAILED( hr ) || !m_pOwnStream ) return E_OUTOFMEMORY;

    return S_OK;
}

STDMETHODIMP EmbedDocument_Impl::HandsOffStorage()
{
    m_pMasterStorage = CComPtr< IStorage >();
    m_pOwnStream = CComPtr< IStream >();

    return S_OK;
}


