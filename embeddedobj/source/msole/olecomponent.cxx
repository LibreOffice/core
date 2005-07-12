/*************************************************************************
 *
 *  $RCSfile: olecomponent.cxx,v $
 *
 *  $Revision: 1.26 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-12 12:18:50 $
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
 *  Source License Version 1.1 ( the "License" ); You may not use this file
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
 *  Contributor( s ): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _COM_SUN_STAR_LANG_ILLEGALARGUMENTEXCEPTION_HPP_
#include <com/sun/star/lang/IllegalArgumentException.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_DISPOSEDEXCEPTION_HPP_
#include <com/sun/star/lang/DisposedException.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_WRONGSTATEEXCEPTION_HPP_
#include <com/sun/star/embed/WrongStateException.hpp>
#endif

#ifndef _COM_SUN_STAR_EMBED_UNREACHABLESTATEEXCEPTION_HPP_
#include <com/sun/star/embed/UnreachableStateException.hpp>
#endif

#ifndef _COM_SUN_STAR_UCB_XSIMPLEFILEACCESS_HPP_
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#endif

#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif

#ifndef _COM_SUN_STAR_IO_XTRUNCATE_HPP_
#include <com/sun/star/io/XTruncate.hpp>
#endif


#include <platform.h>

#include <cppuhelper/interfacecontainer.h>
#include <osl/file.hxx>

#include "olecomponent.hxx"
#include "olewrapclient.hxx"
#include "advisesink.hxx"
#include "oleembobj.hxx"

using namespace ::com::sun::star;

#define     MAX_ENUM_ELE     20
#define     FORMATS_NUM      3

sal_Bool ConvertBufferToFormat( void* pBuf,
                                sal_uInt32 nBufSize,
                                const ::rtl::OUString& aFormatShortName,
                                uno::Any& aResult );

void copyInputToOutput_Impl( const uno::Reference< io::XInputStream >& aIn,
                             const uno::Reference< io::XOutputStream >& aOut );
::rtl::OUString GetNewTempFileURL_Impl( const uno::Reference< lang::XMultiServiceFactory >& xFactory ) throw( io::IOException );
::rtl::OUString GetNewFilledTempFile_Impl( const uno::Reference< io::XInputStream >& xInStream, const uno::Reference< lang::XMultiServiceFactory >& xFactory ) throw( io::IOException );
sal_Bool KillFile_Impl( const ::rtl::OUString& aURL, const uno::Reference< lang::XMultiServiceFactory >& xFactory );

typedef ::std::vector< FORMATETC* > FormatEtcList;

FORMATETC pFormatTemplates[FORMATS_NUM] = {
                    { CF_ENHMETAFILE, NULL, 0, -1, TYMED_ENHMF },
                    { CF_METAFILEPICT, NULL, 0, -1, TYMED_MFPICT },
                    { CF_BITMAP, NULL, 0, -1, TYMED_GDI } };



struct OleComponentNative_Impl {
    CComPtr< IUnknown > m_pObj;
    CComPtr< IOleObject > m_pOleObject;
    CComPtr< IViewObject2 > m_pViewObject2;
    CComPtr< IStorage > m_pIStorage;
    FormatEtcList m_aFormatsList;
    uno::Sequence< datatransfer::DataFlavor > m_aSupportedGraphFormats;
    uno::Sequence< ::rtl::OUString > m_aGraphShortFormats; //short names for formats from previous sequence

    OleComponentNative_Impl()
    {
        // TODO: Extend format list
        m_aSupportedGraphFormats.realloc( 5 );
        m_aGraphShortFormats.realloc( 5 );

        m_aSupportedGraphFormats[0] = datatransfer::DataFlavor(
            ::rtl::OUString::createFromAscii( "application/x-openoffice-emf;windows_formatname=\"Image EMF\"" ),
            ::rtl::OUString::createFromAscii( "Windows Enhanced Metafile" ),
            getCppuType( (const uno::Sequence< sal_Int8 >*) 0 ) );
        m_aGraphShortFormats[0] = ::rtl::OUString::createFromAscii( "EMF" );

        m_aSupportedGraphFormats[1] = datatransfer::DataFlavor(
            ::rtl::OUString::createFromAscii( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"" ),
            ::rtl::OUString::createFromAscii( "Windows Metafile" ),
            getCppuType( (const uno::Sequence< sal_Int8 >*) 0 ) );
        m_aGraphShortFormats[1] = ::rtl::OUString::createFromAscii( "WMF" );

        m_aSupportedGraphFormats[2] = datatransfer::DataFlavor(
            ::rtl::OUString::createFromAscii( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"" ),
            ::rtl::OUString::createFromAscii( "Bitmap" ),
            getCppuType( (const uno::Sequence< sal_Int8 >*) 0 ) );
        m_aGraphShortFormats[2] = ::rtl::OUString::createFromAscii( "BMP" );

        m_aSupportedGraphFormats[3] = datatransfer::DataFlavor(
            ::rtl::OUString::createFromAscii( "image/png" ),
            ::rtl::OUString::createFromAscii( "PNG" ),
            getCppuType( (const uno::Sequence< sal_Int8 >*) 0 ) );
        m_aGraphShortFormats[3] = ::rtl::OUString::createFromAscii( "PNG" );

        m_aSupportedGraphFormats[0] = datatransfer::DataFlavor(
            ::rtl::OUString::createFromAscii( "application/x-openoffice-gdimetafile;windows_formatname=\"GDIMetaFile\"" ),
            ::rtl::OUString::createFromAscii( "GDIMetafile" ),
            getCppuType( (const uno::Sequence< sal_Int8 >*) 0 ) );
        m_aGraphShortFormats[0] = ::rtl::OUString::createFromAscii( "SVM" );
    }

    void AddSupportedFormat( const FORMATETC& aFormatEtc );

    FORMATETC* GetSupportedFormatForAspect( sal_uInt32 nRequestedAspect );

    sal_Bool ConvertDataForFlavor( const STGMEDIUM& aMedium,
                                    const datatransfer::DataFlavor& aFlavor,
                                    uno::Any& aResult );

    sal_Bool GraphicalFlavor( const datatransfer::DataFlavor& aFlavor );

    uno::Sequence< datatransfer::DataFlavor > GetFlavorsForAspects( sal_uInt32 nSupportedAspects );
};

//----------------------------------------------
DWORD GetAspectFromFlavor( const datatransfer::DataFlavor& aFlavor )
{
    if ( aFlavor.MimeType.indexOf( ::rtl::OUString::createFromAscii( ";Aspect=THUMBNAIL" ) ) != -1 )
        return DVASPECT_THUMBNAIL;
    else if ( aFlavor.MimeType.indexOf( ::rtl::OUString::createFromAscii( ";Aspect=ICON" ) ) != -1 )
        return DVASPECT_ICON;
    else if ( aFlavor.MimeType.indexOf( ::rtl::OUString::createFromAscii( ";Aspect=DOCPRINT" ) ) != -1 )
        return DVASPECT_DOCPRINT;
    else
        return DVASPECT_CONTENT;
}

//----------------------------------------------
::rtl::OUString GetFlavorSuffixFromAspect( DWORD nAsp )
{
    ::rtl::OUString aResult;

    if ( nAsp == DVASPECT_THUMBNAIL )
        aResult = ::rtl::OUString::createFromAscii( ";Aspect=THUMBNAIL" );
    else if ( nAsp == DVASPECT_ICON )
        aResult = ::rtl::OUString::createFromAscii( ";Aspect=ICON" );
    else if ( nAsp == DVASPECT_DOCPRINT )
        aResult = ::rtl::OUString::createFromAscii( ";Aspect=DOCPRINT" );

    // no suffix for DVASPECT_CONTENT

    return aResult;
}

//----------------------------------------------
uno::Sequence< sal_Int8 > GetSequenceClassID( sal_uInt32 n1, sal_uInt16 n2, sal_uInt16 n3,
                                                sal_uInt8 b8, sal_uInt8 b9, sal_uInt8 b10, sal_uInt8 b11,
                                                sal_uInt8 b12, sal_uInt8 b13, sal_uInt8 b14, sal_uInt8 b15 )
{
    // TODO: must be removed and used from another library

    uno::Sequence< sal_Int8 > aResult( 16 );
    aResult[0] = (sal_Int8)( n1 >> 24 );
    aResult[1] = (sal_Int8)( ( n1 << 8 ) >> 24 );
    aResult[2] = (sal_Int8)( ( n1 << 16 ) >> 24 );
    aResult[3] = (sal_Int8)( ( n1 << 24 ) >> 24 );
    aResult[4] = (sal_Int8)( n2 >> 8 );
    aResult[5] = (sal_Int8)( ( n2 << 8 ) >> 8 );
    aResult[6] = (sal_Int8)( n3 >> 8 );
    aResult[7] = (sal_Int8)( ( n3 << 8 ) >> 8 );
    aResult[8] = b8;
    aResult[9] = b9;
    aResult[10] = b10;
    aResult[11] = b11;
    aResult[12] = b12;
    aResult[13] = b13;
    aResult[14] = b14;
    aResult[15] = b15;

    return aResult;
}

//----------------------------------------------
sal_Bool OleComponentNative_Impl::ConvertDataForFlavor( const STGMEDIUM& aMedium,
                                                        const datatransfer::DataFlavor& aFlavor,
                                                        uno::Any& aResult )
{
    sal_Bool bAnyIsReady = sal_False;

    // try to convert data from Medium format to specified Flavor format
    if ( aFlavor.DataType == getCppuType( ( const uno::Sequence< sal_Int8 >* ) 0 ) )
    {
        // first the GDI-metafile must be generated

        unsigned char* pBuf = NULL;
        sal_uInt32 nBufSize = 0;

        if ( aMedium.tymed == TYMED_MFPICT ) // Win Metafile
        {
            METAFILEPICT* pMF = ( METAFILEPICT* )GlobalLock( aMedium.hMetaFilePict );
            if ( pMF )
            {
                // TODO: probably in future mapmode must be used in conversion
                nBufSize = GetMetaFileBitsEx( pMF->hMF, 0, NULL );
                pBuf = new unsigned char[nBufSize+22];
                *( (long* )pBuf ) = 0x9ac6cdd7L;
                *( (short* )( pBuf+6 )) = ( SHORT ) 0;
                *( (short* )( pBuf+8 )) = ( SHORT ) 0;
                *( (short* )( pBuf+10 )) = ( SHORT ) pMF->xExt;
                *( (short* )( pBuf+12 )) = ( SHORT ) pMF->yExt;
                *( (short* )( pBuf+14 )) = ( USHORT ) 2540;

                if ( nBufSize && nBufSize == GetMetaFileBitsEx( pMF->hMF, nBufSize, pBuf+22 ) )
                {
                    if ( aFlavor.MimeType.matchAsciiL( "application/x-openoffice-wmf;windows_formatname=\"Image WMF\"", 57 ) )
                    {
                        aResult <<= uno::Sequence< sal_Int8 >( ( sal_Int8* )pBuf, nBufSize + 22 );
                        bAnyIsReady = sal_True;
                    }
                }

                nBufSize += 22;
                GlobalUnlock( aMedium.hMetaFilePict );
            }
        }
        else if ( aMedium.tymed == TYMED_ENHMF ) // Enh Metafile
        {
            nBufSize = GetEnhMetaFileBits( aMedium.hEnhMetaFile, 0, NULL );
            pBuf = new unsigned char[nBufSize];
            if ( nBufSize && nBufSize == GetEnhMetaFileBits( aMedium.hEnhMetaFile, nBufSize, pBuf ) )
            {
                if ( aFlavor.MimeType.matchAsciiL( "application/x-openoffice-emf;windows_formatname=\"Image EMF\"", 57 ) )
                {
                    aResult <<= uno::Sequence< sal_Int8 >( ( sal_Int8* )pBuf, nBufSize );
                    bAnyIsReady = sal_True;
                }
            }
        }
        else if ( aMedium.tymed == TYMED_GDI ) // Bitmap
        {
            nBufSize = GetBitmapBits( aMedium.hBitmap, 0, NULL );
            pBuf = new unsigned char[nBufSize];
            if ( nBufSize && nBufSize == GetBitmapBits( aMedium.hBitmap, nBufSize, pBuf ) )
            {
                if ( aFlavor.MimeType.matchAsciiL( "application/x-openoffice-bitmap;windows_formatname=\"Bitmap\"", 54 ) )
                {
                    aResult <<= uno::Sequence< sal_Int8 >( ( sal_Int8* )pBuf, nBufSize );
                    bAnyIsReady = sal_True;
                }
            }
        }

        if ( pBuf && !bAnyIsReady )
        {
            for ( sal_Int32 nInd = 0; nInd < m_aSupportedGraphFormats.getLength(); nInd++ )
                 if ( aFlavor.MimeType.match( m_aSupportedGraphFormats[nInd].MimeType )
                  && aFlavor.DataType == m_aSupportedGraphFormats[nInd].DataType
                  && aFlavor.DataType == getCppuType( (const uno::Sequence< sal_Int8 >*) 0 ) )
            {
                bAnyIsReady = ConvertBufferToFormat( ( void* )pBuf, nBufSize, m_aGraphShortFormats[nInd], aResult );
                break;
            }
        }

        delete[] pBuf;
    }

    return bAnyIsReady;
}

//----------------------------------------------
sal_Bool OleComponentNative_Impl::GraphicalFlavor( const datatransfer::DataFlavor& aFlavor )
{
    // Actually all the required graphical formats must be supported
    for ( sal_Int32 nInd = 0; nInd < m_aSupportedGraphFormats.getLength(); nInd++ )
         if ( aFlavor.MimeType.match( m_aSupportedGraphFormats[nInd].MimeType )
          && aFlavor.DataType == m_aSupportedGraphFormats[nInd].DataType )
            return sal_True;

    return sal_False;
}

//----------------------------------------------
sal_Bool GetClassIDFromSequence_Impl( uno::Sequence< sal_Int8 > aSeq, CLSID& aResult )
{
    if ( aSeq.getLength() == 16 )
    {
        aResult.Data1 = ( ( ( ( ( ( sal_uInt8 )aSeq[0] << 8 ) + ( sal_uInt8 )aSeq[1] ) << 8 ) + ( sal_uInt8 )aSeq[2] ) << 8 ) + ( sal_uInt8 )aSeq[3];
        aResult.Data2 = ( ( sal_uInt8 )aSeq[4] << 8 ) + ( sal_uInt8 )aSeq[5];
        aResult.Data3 = ( ( sal_uInt8 )aSeq[6] << 8 ) + ( sal_uInt8 )aSeq[7];
        for( int nInd = 0; nInd < 8; nInd++ )
            aResult.Data4[nInd] = ( sal_uInt8 )aSeq[nInd+8];

        return sal_True;
    }

    return sal_False;
}

//----------------------------------------------
::rtl::OUString WinAccToVcl_Impl( const sal_Unicode* pStr )
{
    ::rtl::OUString aResult;

    if( pStr )
    {
        while ( *pStr )
        {
            if ( *pStr == '&' )
            {
                aResult += ::rtl::OUString::createFromAscii( "~" );
                while( *( ++pStr ) == '&' );
            }
            else
            {
                aResult += ::rtl::OUString( pStr, 1 );
                pStr++;
            }
        }
    }

    return aResult;
}

//----------------------------------------------
OleComponent::OleComponent( const uno::Reference< lang::XMultiServiceFactory >& xFactory, OleEmbeddedObject* pUnoOleObject )
: m_pInterfaceContainer( NULL )
, m_bDisposed( sal_False )
, m_bModified( sal_False )
, m_pNativeImpl( new OleComponentNative_Impl() )
, m_xFactory( xFactory )
, m_pOleWrapClientSite( NULL )
, m_pImplAdviseSink( NULL )
, m_pUnoOleObject( pUnoOleObject )
, m_nOLEMiscFlags( 0 )
, m_nAdvConn( 0 )
, m_bOleInitialized( sal_False )
{
    OSL_ENSURE( m_pUnoOleObject, "No owner object is provided!" );

    HRESULT hr = OleInitialize( NULL );
    OSL_ENSURE( hr == S_OK || hr == S_FALSE, "The ole can not be successfuly initialized\n" );
    if ( hr == S_OK || hr == S_FALSE )
        m_bOleInitialized = sal_True;

    m_pOleWrapClientSite = new OleWrapperClientSite( ( OleComponent* )this );
    m_pOleWrapClientSite->AddRef();

    m_pImplAdviseSink = new OleWrapperAdviseSink( ( OleComponent* )this );
    m_pImplAdviseSink->AddRef();

}

//----------------------------------------------
OleComponent::~OleComponent()
{
    OSL_ENSURE( !m_pOleWrapClientSite && !m_pImplAdviseSink && !m_pInterfaceContainer && !m_bOleInitialized,
                "The object was not closed successfully! DISASTER is possible!" );

    if ( m_pOleWrapClientSite || m_pImplAdviseSink || m_pInterfaceContainer || m_bOleInitialized )
    {
        ::osl::MutexGuard aGuard( m_aMutex );
        m_refCount++;
        try {
            Dispose();
        } catch( uno::Exception& ) {}
    }

    for ( FormatEtcList::iterator aIter = m_pNativeImpl->m_aFormatsList.begin();
          aIter != m_pNativeImpl->m_aFormatsList.end();
          aIter++ )
    {
        delete (*aIter);
        (*aIter) = NULL;
    }
    m_pNativeImpl->m_aFormatsList.clear();

    delete m_pNativeImpl;
}

//----------------------------------------------
void OleComponentNative_Impl::AddSupportedFormat( const FORMATETC& aFormatEtc )
{
    FORMATETC* pFormatToInsert = new FORMATETC( aFormatEtc );
    m_aFormatsList.push_back( pFormatToInsert );
}

//----------------------------------------------
FORMATETC* OleComponentNative_Impl::GetSupportedFormatForAspect( sal_uInt32 nRequestedAspect )
{
    for ( FormatEtcList::iterator aIter = m_aFormatsList.begin();
          aIter != m_aFormatsList.end();
          aIter++ )
        if ( (*aIter) && (*aIter)->dwAspect == nRequestedAspect )
            return (*aIter);

    return NULL;
}

//----------------------------------------------
void OleComponent::Dispose()
{
    // the mutex must be locked before this method is called
    if ( m_bDisposed )
        return;

    CloseObject();

    if ( m_pOleWrapClientSite )
    {
        m_pOleWrapClientSite->disconnectOleComponent();
        m_pOleWrapClientSite->Release();
        m_pOleWrapClientSite = NULL;
    }

    if ( m_pImplAdviseSink )
    {
        m_pImplAdviseSink->disconnectOleComponent();
        m_pImplAdviseSink->Release();
        m_pImplAdviseSink = NULL;
    }

    if ( m_pInterfaceContainer )
    {
        lang::EventObject aEvent( static_cast< ::cppu::OWeakObject* >( this ) );
        m_pInterfaceContainer->disposeAndClear( aEvent );

        delete m_pInterfaceContainer;
        m_pInterfaceContainer = NULL;
    }

    if ( m_bOleInitialized )
    {
        // since the disposing can happen not only from main thread but also from a clipboard
        // the deinitialization might lead to a disaster, SO7 does not deinitialize OLE at all
        // so currently the same approach is selected as workaround
        // OleUninitialize();
        m_bOleInitialized = sal_False;
    }

    m_bDisposed = sal_True;
}

//----------------------------------------------
void OleComponent::disconnectEmbeddedObject()
{
    // must not be called from destructor of UNO OLE object!!!
    osl::MutexGuard aGuard( m_aMutex );
    m_pUnoOleObject = NULL;
}

//----------------------------------------------
void OleComponent::CreateIStorageOnXInputStream_Impl( const uno::Reference< io::XInputStream >& xInStream )
{
    // TODO: in future a global memory should be used instead of file.

    OSL_ENSURE( !m_aTempURL.getLength(), "The object already has temporary representation!\n" );

    // write the stream to the temporary file
    m_aTempURL = GetNewFilledTempFile_Impl( xInStream, m_xFactory );
    if ( !m_aTempURL.getLength() )
        throw uno::RuntimeException(); // TODO

    // open an IStorage based on the temporary file
    ::rtl::OUString aTempFilePath;
    if ( ::osl::FileBase::getSystemPathFromFileURL( m_aTempURL, aTempFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happend

    // TODO: remove temporary file when not needed any more
    HRESULT hr = StgOpenStorage( aTempFilePath,
                                 NULL,
                                 STGM_READWRITE | STGM_TRANSACTED, // | STGM_DELETEONRELEASE,
                                 NULL,
                                 0,
                                 &m_pNativeImpl->m_pIStorage );

    if ( FAILED( hr ) || !m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO: transport error code?
}

//----------------------------------------------
void OleComponent::CreateNewIStorage_Impl()
{
    // TODO: in future a global memory should be used instead of file.

    OSL_ENSURE( !m_aTempURL.getLength(), "The object already has temporary representation!\n" );

    // write the stream to the temporary file
    m_aTempURL = GetNewTempFileURL_Impl( m_xFactory );
    if ( !m_aTempURL.getLength() )
        throw uno::RuntimeException(); // TODO

    // open an IStorage based on the temporary file
    ::rtl::OUString aTempFilePath;
    if ( ::osl::FileBase::getSystemPathFromFileURL( m_aTempURL, aTempFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happend

    HRESULT hr = StgCreateDocfile( aTempFilePath, STGM_CREATE | STGM_READWRITE | STGM_TRANSACTED | STGM_DELETEONRELEASE, 0, &m_pNativeImpl->m_pIStorage );
    if ( FAILED( hr ) || !m_pNativeImpl->m_pIStorage )
        throw io::IOException(); // TODO: transport error code?
}

//----------------------------------------------
uno::Sequence< datatransfer::DataFlavor > OleComponentNative_Impl::GetFlavorsForAspects( sal_uInt32 nSupportedAspects )
{
    uno::Sequence< datatransfer::DataFlavor > aResult;
    const sal_uInt32 nAspects[4] = { DVASPECT_CONTENT, DVASPECT_THUMBNAIL, DVASPECT_ICON, DVASPECT_DOCPRINT };
    for ( sal_uInt32 nAsp = 1; nAsp <= 8; nAsp *= 2 )
        if ( ( nSupportedAspects & nAsp ) == nAsp )
        {
            ::rtl::OUString aAspectSuffix = GetFlavorSuffixFromAspect( nAsp );

            sal_Int32 nLength = aResult.getLength();
            aResult.realloc( nLength + m_aSupportedGraphFormats.getLength() );

            for ( sal_Int32 nInd = 0; nInd < m_aSupportedGraphFormats.getLength(); nInd++ )
            {
                aResult[nLength + nInd].MimeType = m_aSupportedGraphFormats[nInd].MimeType + aAspectSuffix;
                aResult[nLength + nInd].HumanPresentableName = m_aSupportedGraphFormats[nInd].HumanPresentableName;
                aResult[nLength + nInd].DataType = m_aSupportedGraphFormats[nInd].DataType;
            }
        }

    return aResult;
}

//----------------------------------------------
void OleComponent::RetrieveObjectDataFlavors_Impl()
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( !m_aDataFlavors.getLength() )
    {
        CComPtr< IDataObject > pDataObject;
        HRESULT hr = m_pNativeImpl->m_pObj->QueryInterface( IID_IDataObject, (void**)&pDataObject );
        if ( SUCCEEDED( hr ) && pDataObject )
        {
            CComPtr< IEnumFORMATETC > pFormatEnum;
            hr = pDataObject->EnumFormatEtc( DATADIR_GET, &pFormatEnum );
            if ( SUCCEEDED( hr ) && pFormatEnum )
            {
                FORMATETC pElem[ MAX_ENUM_ELE ];
                ULONG nNum = 0;

                // if it is possible to retrieve at least one supported graphical format for an aspect
                // this format can be converted to other supported formats
                sal_uInt32 nSupportedAspects = 0;
                do
                {
                    HRESULT hr = pFormatEnum->Next( MAX_ENUM_ELE, pElem, &nNum );
                    if( hr == S_OK || hr == S_FALSE )
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

//----------------------------------------------
sal_Bool OleComponent::InitializeObject_Impl()
// There will be no static objects!
{
    if ( !m_pNativeImpl->m_pObj )
        return sal_False;

    // the linked object will be detected here
    CComPtr< IOleLink > pOleLink;
    HRESULT hr = m_pNativeImpl->m_pObj->QueryInterface( IID_IOleLink, (void**)&pOleLink );
    m_pUnoOleObject->SetObjectIsLink_Impl( pOleLink != NULL );


    hr = m_pNativeImpl->m_pObj->QueryInterface( IID_IViewObject2, (void**)&m_pNativeImpl->m_pViewObject2 );
    if ( FAILED( hr ) || !m_pNativeImpl->m_pViewObject2 )
        return sal_False;

    // not realy needed for now, since object is updated on saving
    // m_pNativeImpl->m_pViewObject2->SetAdvise( DVASPECT_CONTENT, 0, m_pImplAdviseSink );

    // remove all the caches and register own specific one
    IOleCache* pIOleCache = NULL;
    if ( SUCCEEDED( m_pNativeImpl->m_pObj->QueryInterface( IID_IOleCache, (void**)&pIOleCache ) ) && pIOleCache )
    {
        IEnumSTATDATA* pEnumSD = NULL;
        HRESULT hr = pIOleCache->EnumCache( &pEnumSD );

        if ( SUCCEEDED( hr ) && pEnumSD )
        {
            pEnumSD->Reset();
            STATDATA aSD;
            DWORD nNum;
            while( SUCCEEDED( pEnumSD->Next( 1, &aSD, &nNum ) ) && nNum == 1 )
                hr = pIOleCache->Uncache( aSD.dwConnection );
        }

        DWORD nConn;
        FORMATETC aFormat = { 0, 0, DVASPECT_CONTENT, -1, TYMED_MFPICT };
        hr = pIOleCache->Cache( &aFormat, ADVFCACHE_ONSAVE, &nConn );

        pIOleCache->Release();
        pIOleCache = NULL;
    }

    hr = m_pNativeImpl->m_pObj->QueryInterface( IID_IOleObject, (void**)&m_pNativeImpl->m_pOleObject );
    if ( FAILED( hr ) || !m_pNativeImpl->m_pOleObject )
        return sal_False; // Static objects are not supported, they should be inserted as graphics

    m_pNativeImpl->m_pOleObject->GetMiscStatus( DVASPECT_CONTENT, ( DWORD* )&m_nOLEMiscFlags );
    // TODO: use other misc flags also
    // the object should have drawable aspect even in case it supports only iconic representation
    // if ( m_nOLEMiscFlags & OLEMISC_ONLYICONIC )

    m_pNativeImpl->m_pOleObject->SetClientSite( m_pOleWrapClientSite );

    // the only need in this registration is workaround for close notification
    m_pNativeImpl->m_pOleObject->Advise( m_pImplAdviseSink, ( DWORD* )&m_nAdvConn );
    m_pNativeImpl->m_pViewObject2->SetAdvise( DVASPECT_CONTENT, ADVF_PRIMEFIRST, m_pImplAdviseSink );

    OleSetContainedObject( m_pNativeImpl->m_pOleObject, TRUE );

    return sal_True;
}

//----------------------------------------------
void OleComponent::LoadEmbeddedObject( const uno::Reference< io::XInputStream >& xInStream )
{
    if ( !xInStream.is() )
        throw lang::IllegalArgumentException(); // TODO

    if ( m_pNativeImpl->m_pIStorage || m_aTempURL.getLength() )
        throw io::IOException(); // TODO the object is already initialized

    CreateIStorageOnXInputStream_Impl( xInStream );
    if ( !m_pNativeImpl->m_pIStorage )
        throw uno::RuntimeException(); // TODO:

    HRESULT hr = OleLoad( m_pNativeImpl->m_pIStorage, IID_IUnknown, NULL, (void**)&m_pNativeImpl->m_pObj );
    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
    {
        // STATSTG aStat;
        // m_pNativeImpl->m_pIStorage->Stat( &aStat, STATFLAG_NONAME );
        throw uno::RuntimeException();
    }

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO
}

//----------------------------------------------
void OleComponent::CreateObjectFromClipboard()
{
    if ( m_pNativeImpl->m_pIStorage || m_aTempURL.getLength() )
        throw io::IOException(); // TODO:the object is already initialized

    CreateNewIStorage_Impl();
    if ( !m_pNativeImpl->m_pIStorage )
        throw uno::RuntimeException(); // TODO

    IDataObject * pDO = NULL;
    HRESULT hr = OleGetClipboard( &pDO );
    if( SUCCEEDED( hr ) && pDO )
    {
        hr = OleQueryCreateFromData( pDO );
        if( S_OK == GetScode( hr ) )
        {
            hr = OleCreateFromData( pDO,
                                    IID_IUnknown,
                                    OLERENDER_DRAW, // OLERENDER_FORMAT
                                    NULL,           // &aFormat,
                                    NULL,
                                    m_pNativeImpl->m_pIStorage,
                                    (void**)&m_pNativeImpl->m_pObj );
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

//----------------------------------------------
void OleComponent::CreateNewEmbeddedObject( const uno::Sequence< sal_Int8 >& aSeqCLSID )
{
    CLSID aClsID;

    if ( !GetClassIDFromSequence_Impl( aSeqCLSID, aClsID ) )
        throw lang::IllegalArgumentException(); // TODO

    if ( m_pNativeImpl->m_pIStorage || m_aTempURL.getLength() )
        throw io::IOException(); // TODO:the object is already initialized

    CreateNewIStorage_Impl();
    if ( !m_pNativeImpl->m_pIStorage )
        throw uno::RuntimeException(); // TODO

    // FORMATETC aFormat = { CF_METAFILEPICT, NULL, nAspect, -1, TYMED_MFPICT }; // for OLE..._DRAW should be NULL

    HRESULT hr = OleCreate( aClsID,
                            IID_IUnknown,
                            OLERENDER_DRAW, // OLERENDER_FORMAT
                            NULL,           // &aFormat,
                            NULL,
                            m_pNativeImpl->m_pIStorage,
                            (void**)&m_pNativeImpl->m_pObj );

    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
        throw uno::RuntimeException(); // TODO

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO

    // TODO: getExtent???
}

//----------------------------------------------
void OleComponent::CreateObjectFromData( const uno::Reference< datatransfer::XTransferable >& xTransfer )
// Static objects are not supported, they should be inserted as graphics
{
    // TODO: May be this call is useless since there are no static objects
    //       and nonstatic objects will be created based on OLEstorage ( stream ).
    //       ???

    // OleQueryCreateFromData...
}

//----------------------------------------------
void OleComponent::CreateObjectFromFile( const ::rtl::OUString& aFileURL )
{
    if ( m_pNativeImpl->m_pIStorage || m_aTempURL.getLength() )
        throw io::IOException(); // TODO:the object is already initialized

    CreateNewIStorage_Impl();
    if ( !m_pNativeImpl->m_pIStorage )
        throw uno::RuntimeException(); // TODO:

    ::rtl::OUString aFilePath;
    if ( ::osl::FileBase::getSystemPathFromFileURL( aFileURL, aFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happend

    HRESULT hr = OleCreateFromFile( CLSID_NULL,
                                    aFilePath.getStr(),
                                    IID_IUnknown,
                                    OLERENDER_DRAW, // OLERENDER_FORMAT
                                    NULL,
                                    NULL,
                                    m_pNativeImpl->m_pIStorage,
                                    (void**)&m_pNativeImpl->m_pObj );

    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
        throw uno::RuntimeException(); // TODO

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO
}

//----------------------------------------------
void OleComponent::CreateLinkFromFile( const ::rtl::OUString& aFileURL )
{
    if ( m_pNativeImpl->m_pIStorage || m_aTempURL.getLength() )
        throw io::IOException(); // TODO:the object is already initialized

    CreateNewIStorage_Impl();
    if ( !m_pNativeImpl->m_pIStorage )
        throw uno::RuntimeException(); // TODO:

    ::rtl::OUString aFilePath;
    if ( ::osl::FileBase::getSystemPathFromFileURL( aFileURL, aFilePath ) != ::osl::FileBase::E_None )
        throw uno::RuntimeException(); // TODO: something dangerous happend

    HRESULT hr = OleCreateLinkToFile( aFilePath.getStr(),
                                        IID_IUnknown,
                                        OLERENDER_DRAW, // OLERENDER_FORMAT
                                        NULL,
                                        NULL,
                                        m_pNativeImpl->m_pIStorage,
                                        (void**)&m_pNativeImpl->m_pObj );

    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
        throw uno::RuntimeException(); // TODO

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO
}

//----------------------------------------------
void OleComponent::InitEmbeddedCopyOfLink( OleComponent* pOleLinkComponent )
{
    if ( !pOleLinkComponent || !pOleLinkComponent->m_pNativeImpl->m_pObj )
        throw lang::IllegalArgumentException(); // TODO

    if ( m_pNativeImpl->m_pIStorage || m_aTempURL.getLength() )
        throw io::IOException(); // TODO:the object is already initialized

    CComPtr< IDataObject > pDataObject;
    HRESULT hr = pOleLinkComponent->m_pNativeImpl->m_pObj->QueryInterface( IID_IDataObject, (void**)&pDataObject );
    if ( SUCCEEDED( hr ) && pDataObject && SUCCEEDED( OleQueryCreateFromData( pDataObject ) ) )
    {
        CreateNewIStorage_Impl();
        if ( !m_pNativeImpl->m_pIStorage )
            throw uno::RuntimeException(); // TODO:

        hr = OleCreateFromData( pDataObject,
                                IID_IUnknown,
                                OLERENDER_DRAW,
                                NULL,
                                NULL,
                                m_pNativeImpl->m_pIStorage,
                                (void**)&m_pNativeImpl->m_pObj );
    }

    if ( !m_pNativeImpl->m_pObj )
    {
        CComPtr< IOleLink > pOleLink;
        hr = pOleLinkComponent->m_pNativeImpl->m_pObj->QueryInterface( IID_IOleLink, (void**)&pOleLink );
        if ( FAILED( hr ) || !pOleLink )
            throw io::IOException(); // TODO: the object doesn't support IOleLink

        CComPtr< IMoniker > pMoniker;
        hr = pOleLink->GetSourceMoniker( &pMoniker );
        if ( FAILED( hr ) || !pMoniker )
            throw io::IOException(); // TODO: can not retrieve moniker

        // In case of file moniker life is easy : )
        DWORD aMonType = 0;
        hr = pMoniker->IsSystemMoniker( &aMonType );
        if ( SUCCEEDED( hr ) && aMonType == MKSYS_FILEMONIKER )
        {
            CComPtr< IMalloc > pMalloc;
            CoGetMalloc( 1, &pMalloc ); // if fails there will be a memory leak
            OSL_ENSURE( pMalloc, "CoGetMalloc() failed!" );

            LPOLESTR pOleStr = NULL;
            hr = pOleLink->GetSourceDisplayName( &pOleStr );
            if ( SUCCEEDED( hr ) && pOleStr )
            {
                ::rtl::OUString aFilePath( ( sal_Unicode* )pOleStr );
                if ( pMalloc )
                    pMalloc->Free( ( void* )pOleStr );

                hr = OleCreateFromFile( CLSID_NULL,
                                        aFilePath.getStr(),
                                        IID_IUnknown,
                                        OLERENDER_DRAW, // OLERENDER_FORMAT
                                        NULL,
                                        NULL,
                                        m_pNativeImpl->m_pIStorage,
                                        (void**)&m_pNativeImpl->m_pObj );
            }
        }

        // in case of other moniker types the only way is to get storage
        if ( !m_pNativeImpl->m_pObj )
        {
            CComPtr< IBindCtx > pBindCtx;
            hr = CreateBindCtx( 0, ( LPBC FAR* )&pBindCtx );
            if ( SUCCEEDED( hr ) && pBindCtx )
            {
                CComPtr< IStorage > pObjectStorage;
                hr = pMoniker->BindToStorage( pBindCtx, NULL, IID_IStorage, (void**)&pObjectStorage );
                if ( SUCCEEDED( hr ) && pObjectStorage )
                {
                    hr = pObjectStorage->CopyTo( 0, NULL, NULL, m_pNativeImpl->m_pIStorage );
                    if ( SUCCEEDED( hr ) )
                        hr = OleLoad( m_pNativeImpl->m_pIStorage, IID_IUnknown, NULL, (void**)&m_pNativeImpl->m_pObj );
                }
            }
        }
    }

    // If object could not be created the only way is to use graphical representation
    if ( FAILED( hr ) || !m_pNativeImpl->m_pObj )
        throw uno::RuntimeException(); // TODO

    if ( !InitializeObject_Impl() )
        throw uno::RuntimeException(); // TODO
}

//----------------------------------------------
void OleComponent::RunObject()
{
    OSL_ENSURE( m_pNativeImpl->m_pOleObject, "The pointer can not be set to NULL here!\n" );
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if ( !OleIsRunning( m_pNativeImpl->m_pOleObject ) )
    {
        HRESULT hr = OleRun( m_pNativeImpl->m_pObj );
        if ( FAILED( hr ) )
        {
            if ( hr == REGDB_E_CLASSNOTREG )
                throw embed::UnreachableStateException(); // the object server is not installed
            else
                throw io::IOException();
        }
    }
}

//----------------------------------------------
void OleComponent::CloseObject()
{
    if ( m_pNativeImpl->m_pOleObject && OleIsRunning( m_pNativeImpl->m_pOleObject ) )
        m_pNativeImpl->m_pOleObject->Close( OLECLOSE_NOSAVE ); // must be saved before
}

//----------------------------------------------
uno::Sequence< embed::VerbDescriptor > OleComponent::GetVerbList()
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    if( !m_aVerbList.getLength() )
    {
        CComPtr< IEnumOLEVERB > pEnum;
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
                    for( sal_uInt32 nInd = 0; nInd < nNum; nInd++ )
                    {
                        m_aVerbList[nSeqSize-nNum+nInd].VerbID = szEle[ nInd ].lVerb;
                        m_aVerbList[nSeqSize-nNum+nInd].VerbName = WinAccToVcl_Impl( szEle[ nInd ].lpszVerbName );
                        m_aVerbList[nSeqSize-nNum+nInd].VerbFlags = szEle[ nInd ].fuFlags;
                        m_aVerbList[nSeqSize-nNum+nInd].VerbAttributes = szEle[ nInd ].grfAttribs;
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

//----------------------------------------------
void OleComponent::ExecuteVerb( sal_Int32 nVerbID )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO

    HRESULT hr = OleRun( m_pNativeImpl->m_pOleObject );
    if ( FAILED( hr ) )
        throw io::IOException(); // TODO: a specific exception that transport error code can be thrown here

    // TODO: probably extents should be set here and stored in aRect
    // TODO: probably the parent window also should be set
    hr = m_pNativeImpl->m_pOleObject->DoVerb( nVerbID, NULL, m_pOleWrapClientSite, 0, NULL, NULL );

    if ( FAILED( hr ) )
        throw io::IOException(); // TODO

    // TODO/LATER: the real names should be used here
    m_pNativeImpl->m_pOleObject->SetHostNames( L"app name", L"untitled" );
}

//----------------------------------------------
void OleComponent::SetHostName( const ::rtl::OUString& aContName,
                                const ::rtl::OUString& aEmbDocName )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    // TODO: use aContName and aEmbDocName in m_pNativeImpl->m_pOleObject->SetHostNames()
}

//----------------------------------------------
void OleComponent::SetExtent( const awt::Size& aVisAreaSize, sal_Int64 nAspect )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nMSAspect = ( DWORD )nAspect; // first 32 bits are for MS aspects

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

//----------------------------------------------
awt::Size OleComponent::GetExtent( sal_Int64 nAspect )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nMSAspect = ( DWORD )nAspect; // first 32 bits are for MS aspects
    SIZEL aSize;
    HRESULT hr = m_pNativeImpl->m_pViewObject2->GetExtent( nMSAspect, -1, NULL, &aSize );

    if ( FAILED( hr ) )
    {
        // TODO/LATER: is it correct?
        // if there is no appropriate cache for the aspect, OLE_E_BLANK error code is returned
        // if ( hr == OLE_E_BLANK )
        //  throw lang::IllegalArgumentException();
        //else
        //  throw io::IOException(); // TODO

        // actually the following method returns periodically a different value than the method from IViewObject
        // for this reason it is separated in GetReccomendedExtent call
        // hr = m_pNativeImpl->m_pOleObject->GetExtent( nMSAspect, &aSize );
        // if ( FAILED( hr ) )
            throw lang::IllegalArgumentException();
        //  throw io::IOException(); // TODO
    }

    return awt::Size( aSize.cx, aSize.cy );
}

//----------------------------------------------
awt::Size OleComponent::GetReccomendedExtent( sal_Int64 nAspect )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    DWORD nMSAspect = ( DWORD )nAspect; // first 32 bits are for MS aspects
    SIZEL aSize;
    HRESULT hr = m_pNativeImpl->m_pOleObject->GetExtent( nMSAspect, &aSize );
    if ( FAILED( hr ) )
        throw lang::IllegalArgumentException();

    return awt::Size( aSize.cx, aSize.cy );
}

//----------------------------------------------
sal_Int64 OleComponent::GetMiscStatus( sal_Int64 nAspect )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    sal_uInt32 nResult;
    m_pNativeImpl->m_pOleObject->GetMiscStatus( ( DWORD )nAspect, ( DWORD* )&nResult );
    return ( sal_Int64 )nResult; // first 32 bits are for MS flags
}

//----------------------------------------------
uno::Sequence< sal_Int8 > OleComponent::GetCLSID()
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    GUID aCLSID;
    HRESULT hr = m_pNativeImpl->m_pOleObject->GetUserClassID( &aCLSID );
    if ( FAILED( hr ) )
        throw io::IOException(); // TODO:

    return  GetSequenceClassID( aCLSID.Data1, aCLSID.Data2, aCLSID.Data3,
                                aCLSID.Data4[0], aCLSID.Data4[1],
                                aCLSID.Data4[2], aCLSID.Data4[3],
                                aCLSID.Data4[4], aCLSID.Data4[5],
                                aCLSID.Data4[6], aCLSID.Data4[7] );
}

//----------------------------------------------
void OleComponent::StoreObjectToStream( uno::Reference< io::XOutputStream > xOutStream )
{
    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    CComPtr< IPersistStorage > pPersistStorage;
    HRESULT hr = m_pNativeImpl->m_pObj->QueryInterface( IID_IPersistStorage, (void**)&pPersistStorage );
    if ( FAILED( hr ) || !pPersistStorage )
        throw io::IOException(); // TODO

    hr = OleSave( pPersistStorage, m_pNativeImpl->m_pIStorage, TRUE );
    if ( FAILED( hr ) )
        throw io::IOException(); // TODO

    hr = pPersistStorage->SaveCompleted( NULL );
    if ( FAILED( hr ) )
        throw io::IOException(); // TODO

//REMOVE        if ( !bStoreVisReplace )
//REMOVE        {
//REMOVE            // remove all the cache streams from the storage
//REMOVE            for ( sal_uInt8 nInd = 0; nInd < 10; nInd++ )
//REMOVE            {
//REMOVE                ::rtl::OUString aStreamName = ::rtl::OUString::createFromAscii( "\002OlePres00" );
//REMOVE                aStreamName += ::rtl::OUString::valueOf( (sal_Int32)nInd );
//REMOVE                hr = m_pNativeImpl->m_pIStorage->DestroyElement( aStreamName.getStr() );
//REMOVE                if ( FAILED( hr ) )
//REMOVE                    break;
//REMOVE            }
//REMOVE        }

    hr = m_pNativeImpl->m_pIStorage->Commit( STGC_DEFAULT );
    if ( FAILED( hr ) )
        throw io::IOException(); // TODO

    // STATSTG aStat;
    // m_pNativeImpl->m_pIStorage->Stat( &aStat, STATFLAG_NONAME );

    // now all the changes should be in temporary location

    // open temporary file for reading
    uno::Reference < ucb::XSimpleFileAccess > xTempAccess(
                    m_xFactory->createInstance (
                            ::rtl::OUString::createFromAscii( "com.sun.star.ucb.SimpleFileAccess" ) ),
                    uno::UNO_QUERY );

    if ( !xTempAccess.is() )
        throw uno::RuntimeException(); // TODO:

    uno::Reference< io::XInputStream > xTempInStream = xTempAccess->openFileRead( m_aTempURL );
    OSL_ENSURE( xTempInStream.is(), "The object's temporary file can not be reopened for reading!\n" );

    // TODO: use bStoreVisReplace

    if ( xTempInStream.is() )
    {
        // write all the contents to XOutStream
        uno::Reference< io::XTruncate > xTrunc( xOutStream, uno::UNO_QUERY );
        if ( !xTrunc.is() )
            throw uno::RuntimeException(); //TODO:

        xTrunc->truncate();

        copyInputToOutput_Impl( xTempInStream, xOutStream );
    }
    else
        throw io::IOException(); // TODO:

    // TODO: should the view replacement be in the stream ???
    //       probably it must be specified on storing
}

//----------------------------------------------
sal_Bool OleComponent::SaveObject_Impl()
{
    sal_Bool bResult = sal_False;
    OleEmbeddedObject* pLockObject = NULL;

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

//----------------------------------------------
sal_Bool OleComponent::OnShowWindow_Impl( sal_Bool bShow )
{
    sal_Bool bResult = sal_False;
    OleEmbeddedObject* pLockObject = NULL;

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
        bResult = m_pUnoOleObject->OnShowWindow_Impl( bShow );
        pLockObject->release();
    }

    return bResult;
}

//----------------------------------------------
void OleComponent::OnViewChange_Impl( sal_uInt32 dwAspect )
{
    // TODO: make a notification ?
    // TODO: check if it is enough or may be saving notifications are required for Visio2000
    OleEmbeddedObject* pLockObject = NULL;

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
        m_pUnoOleObject->OnViewChanged_Impl();
        pLockObject->release();
    }

}

//----------------------------------------------
sal_Bool OleComponent::GetGraphicalCache_Impl( const datatransfer::DataFlavor& aFlavor, uno::Any& aResult )
{
    sal_Bool bOk = sal_False;
    if ( m_pNativeImpl->m_pIStorage )
    {
        // try to retrieve cached representation
        // TODO: in future it must be converted to requested format
        for ( sal_uInt8 nInd = 0; nInd < 10; nInd++ )
        {
            CComPtr< IStream > pGrStream;
            ::rtl::OUString aStreamName = ::rtl::OUString::createFromAscii( "\002OlePres00" );
            aStreamName += ::rtl::OUString::valueOf( nInd );
            HRESULT hr = m_pNativeImpl->m_pIStorage->OpenStream( aStreamName.getStr(),
                                                    NULL,
                                                    STGM_READ,
                                                    NULL,
                                                    &pGrStream );
            if ( FAILED( hr ) || !pGrStream )
                break;

            // TODO: check that the format is acceptable
            //       if so - break


            // create XInputStream ( for now SvStream ) with graphical representation
            // try to generate a Metafile or Bitmap from it
            // convert the result representation to requested format
            // if ( succeeded ) ( bOk = sal_True ), break;
        }
    }

    return bOk;
}

// XCloseable
//----------------------------------------------
void SAL_CALL OleComponent::close( sal_Bool bDeliverOwnership )
    throw ( util::CloseVetoException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    uno::Reference< uno::XInterface > xSelfHold( static_cast< ::cppu::OWeakObject* >( this ) );
    lang::EventObject aSource( static_cast< ::cppu::OWeakObject* >( this ) );

    if ( m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer =
            m_pInterfaceContainer->getContainer( ::getCppuType( ( const uno::Reference< util::XCloseListener >* ) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
            while ( pIterator.hasMoreElements() )
            {
                try
                {
                    ( (util::XCloseListener* )pIterator.next() )->queryClosing( aSource, bDeliverOwnership );
                }
                catch( uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }

        pContainer = m_pInterfaceContainer->getContainer(
                                    ::getCppuType( ( const uno::Reference< util::XCloseListener >* ) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pCloseIterator( *pContainer );
            while ( pCloseIterator.hasMoreElements() )
            {
                try
                {
                    ( (util::XCloseListener* )pCloseIterator.next() )->notifyClosing( aSource );
                }
                catch( uno::RuntimeException& )
                {
                    pCloseIterator.remove();
                }
            }
        }
    }

    Dispose();
}

//----------------------------------------------
void SAL_CALL OleComponent::addCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( ( const uno::Reference< util::XCloseListener >* )0 ), xListener );
}

//----------------------------------------------
void SAL_CALL OleComponent::removeCloseListener( const uno::Reference< util::XCloseListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( ( const uno::Reference< util::XCloseListener >* )0 ),
                                                xListener );
}

// XTransferable
//----------------------------------------------
uno::Any SAL_CALL OleComponent::getTransferData( const datatransfer::DataFlavor& aFlavor )
    throw ( datatransfer::UnsupportedFlavorException,
            io::IOException,
            uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    uno::Any aResult;
    sal_Bool bSupportedFlavor = sal_False;

    if ( m_pNativeImpl->GraphicalFlavor( aFlavor ) )
    {
        DWORD nRequestedAspect = GetAspectFromFlavor( aFlavor );
        // if own icon is set and icon aspect is requested the own icon can be returned directly

        CComPtr< IDataObject > pDataObject;
        HRESULT hr = m_pNativeImpl->m_pObj->QueryInterface( IID_IDataObject, (void**)&pDataObject );
        if ( FAILED( hr ) || !pDataObject )
            throw io::IOException(); // TODO: transport error code

        // if ( m_nSupportedFormat )
        FORMATETC* pFormatEtc = m_pNativeImpl->GetSupportedFormatForAspect( nRequestedAspect );
        if ( pFormatEtc )
        {
            STGMEDIUM aMedium;
            hr = pDataObject->GetData( pFormatEtc, &aMedium );
            if ( SUCCEEDED( hr ) )
                bSupportedFlavor = m_pNativeImpl->ConvertDataForFlavor( aMedium, aFlavor, aResult );
        }
        else
        {
            // the supported format of the application is still not found, find one
            for ( sal_Int32 nInd = 0; nInd < FORMATS_NUM; nInd++ )
            {
                STGMEDIUM aMedium;
                FORMATETC aFormat = pFormatTemplates[nInd];
                aFormat.dwAspect = nRequestedAspect;

                hr = pDataObject->GetData( &aFormat, &aMedium );
                if ( SUCCEEDED( hr ) )
                {
                    m_pNativeImpl->AddSupportedFormat( aFormat );

                    bSupportedFlavor = m_pNativeImpl->ConvertDataForFlavor( aMedium, aFlavor, aResult );
                    break;
                }
            }
        }

        if ( !bSupportedFlavor && m_pNativeImpl->m_pIStorage && nRequestedAspect == DVASPECT_CONTENT )
        {
            // try to retrieve cached representation
            bSupportedFlavor = GetGraphicalCache_Impl( aFlavor, aResult );
        }

        if ( !bSupportedFlavor )
        {
            // TODO: implement workaround for stampit ( if required )
        }
    }
    // TODO: Investigate if there is already the format name
    //       and whether this format is really required
    else if ( aFlavor.DataType == getCppuType( ( const uno::Reference< io::XInputStream >* ) 0 )
            && aFlavor.MimeType.equalsAscii( "application/x-openoffice-contentstream" ) )
    {
        // allow to retrieve stream-representation of the object persistence
        bSupportedFlavor = sal_True;
        uno::Reference < io::XStream > xTempFileStream(
            m_xFactory->createInstance( ::rtl::OUString::createFromAscii( "com.sun.star.io.TempFile" ) ),
            uno::UNO_QUERY );

        if ( !xTempFileStream.is() )
            throw uno::RuntimeException(); // TODO

        uno::Reference< io::XOutputStream > xTempOutStream = xTempFileStream->getOutputStream();
        uno::Reference< io::XInputStream > xTempInStream = xTempFileStream->getInputStream();
        if ( xTempOutStream.is() && xTempInStream.is() )
        {
            StoreObjectToStream( xTempOutStream );
            xTempOutStream->closeOutput();
            xTempOutStream = uno::Reference< io::XOutputStream >();
        }
        else
            throw io::IOException(); // TODO:

        aResult <<= xTempInStream;
    }

    if ( !bSupportedFlavor )
        throw datatransfer::UnsupportedFlavorException();

    return aResult;
}

//----------------------------------------------
uno::Sequence< datatransfer::DataFlavor > SAL_CALL OleComponent::getTransferDataFlavors()
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pNativeImpl->m_pOleObject )
        throw embed::WrongStateException(); // TODO: the object is in wrong state

    RetrieveObjectDataFlavors_Impl();

    return m_aDataFlavors;
}

//----------------------------------------------
sal_Bool SAL_CALL OleComponent::isDataFlavorSupported( const datatransfer::DataFlavor& aFlavor )
    throw ( uno::RuntimeException )
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

    for ( sal_Int32 nInd = 0; nInd < m_aDataFlavors.getLength(); nInd++ )
        if ( m_aDataFlavors[nInd].MimeType.equals( aFlavor.MimeType ) && m_aDataFlavors[nInd].DataType == aFlavor.DataType )
            return sal_True;

    return sal_False;
}

void SAL_CALL OleComponent::dispose() throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        close( sal_True );
    }
    catch ( uno::Exception& )
    {
    }
}

void SAL_CALL OleComponent::addEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( ( const uno::Reference< lang::XEventListener >* )0 ), xListener );
}

//----------------------------------------------
void SAL_CALL OleComponent::removeEventListener( const uno::Reference< lang::XEventListener >& xListener )
    throw ( uno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( ( const uno::Reference< lang::XEventListener >* )0 ),
                                                xListener );
}

sal_Bool ClassIDsEqual( const uno::Sequence< sal_Int8 >& aClassID1, const uno::Sequence< sal_Int8 >& aClassID2 );

sal_Int64 SAL_CALL OleComponent::getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& aIdentifier ) throw(::com::sun::star::uno::RuntimeException)
{
    try
    {
        uno::Sequence < sal_Int8 > aCLSID = GetCLSID();
        if ( ClassIDsEqual( aIdentifier, aCLSID ) )
            return (sal_Int64) (IUnknown*) m_pNativeImpl->m_pObj;

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
                return (sal_Int64) (IUnknown*) m_pNativeImpl->m_pObj;
        }
    }
    catch ( uno::Exception& )
    {
    }

    return 0;
}

sal_Bool SAL_CALL OleComponent::isModified() throw (::com::sun::star::uno::RuntimeException)
{
    return m_bModified;
}

void SAL_CALL OleComponent::setModified( sal_Bool bModified )
        throw (::com::sun::star::beans::PropertyVetoException, ::com::sun::star::uno::RuntimeException)
{
    m_bModified = bModified;

    if ( bModified && m_pInterfaceContainer )
    {
        ::cppu::OInterfaceContainerHelper* pContainer =
            m_pInterfaceContainer->getContainer( ::getCppuType( ( const uno::Reference< util::XModifyListener >* ) NULL ) );
        if ( pContainer != NULL )
        {
            ::cppu::OInterfaceIteratorHelper pIterator( *pContainer );
            while ( pIterator.hasMoreElements() )
            {
                try
                {
                    lang::EventObject aEvent( (util::XModifiable*) this );
                    ((util::XModifyListener*)pIterator.next())->modified( aEvent );
                }
                catch( uno::RuntimeException& )
                {
                    pIterator.remove();
                }
            }
        }
    }
}

void SAL_CALL OleComponent::addModifyListener( const com::sun::star::uno::Reference < com::sun::star::util::XModifyListener >& xListener ) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( !m_pInterfaceContainer )
        m_pInterfaceContainer = new ::cppu::OMultiTypeInterfaceContainerHelper( m_aMutex );

    m_pInterfaceContainer->addInterface( ::getCppuType( ( const uno::Reference< util::XModifyListener >* )0 ), xListener );
}

void SAL_CALL OleComponent::removeModifyListener( const com::sun::star::uno::Reference < com::sun::star::util::XModifyListener >& xListener) throw(::com::sun::star::uno::RuntimeException)
{
    ::osl::MutexGuard aGuard( m_aMutex );
    if ( m_bDisposed )
        throw lang::DisposedException(); // TODO

    if ( m_pInterfaceContainer )
        m_pInterfaceContainer->removeInterface( ::getCppuType( ( const uno::Reference< util::XModifyListener >* )0 ),
                                                xListener );
}

