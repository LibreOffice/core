/*************************************************************************
 *
 *  $RCSfile: xmlgrhlp.cxx,v $
 *
 *  $Revision: 1.20 $
 *
 *  last change: $Author: rt $
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

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <tools/debug.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/gfxlink.hxx>

#include "impgrf.hxx"
#include "xmlgrhlp.hxx"

// -----------
// - Defines -
// -----------

using namespace rtl;
using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;

#define XML_GRAPHICSTORAGE_NAME     "Pictures"
#define XML_PACKAGE_URL_BASE        "vnd.sun.star.Package:"
#define XML_GRAPHICOBJECT_URL_BASE  "vnd.sun.star.GraphicObject:"

// ---------------------------
// - SvXMLGraphicInputStream -
// ---------------------------

class SvXMLGraphicInputStream : public::cppu::WeakImplHelper1< XInputStream >
{
private:

    virtual sal_Int32   SAL_CALL    readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual sal_Int32   SAL_CALL    readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual void        SAL_CALL    skipBytes(sal_Int32 nBytesToSkip) throw(NotConnectedException, BufferSizeExceededException, RuntimeException);
    virtual sal_Int32   SAL_CALL    available() throw(NotConnectedException, RuntimeException);
    virtual void        SAL_CALL    closeInput() throw(NotConnectedException, RuntimeException);

private:

    ::utl::TempFile                 maTmp;
    Reference< XInputStream >       mxStmWrapper;

                                    // not available
                                    SvXMLGraphicInputStream();
                                    SvXMLGraphicInputStream( const SvXMLGraphicInputStream& );
    SvXMLGraphicInputStream&        operator==( SvXMLGraphicInputStream& );

public:

                                    SvXMLGraphicInputStream( const OUString& rGraphicId );
    virtual                         ~SvXMLGraphicInputStream();

    sal_Bool                        Exists() const { return mxStmWrapper.is(); }
};

// -----------------------------------------------------------------------------

SvXMLGraphicInputStream::SvXMLGraphicInputStream( const OUString& rGraphicId )
{
    String          aGraphicId( rGraphicId );
    GraphicObject   aGrfObject( ByteString( aGraphicId, RTL_TEXTENCODING_ASCII_US ) );

    maTmp.EnableKillingFile();

    if( aGrfObject.GetType() != GRAPHIC_NONE )
    {
        SvStream* pStm = ::utl::UcbStreamHelper::CreateStream( maTmp.GetURL(), STREAM_WRITE | STREAM_TRUNC );

        if( pStm )
        {
            Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
            const GfxLink   aGfxLink( aGraphic.GetLink() );
            sal_Bool        bRet = sal_False;

            if( aGfxLink.GetDataSize() )
            {
                pStm->Write( aGfxLink.GetData(), aGfxLink.GetDataSize() );
                bRet = ( pStm->GetError() == 0 );
            }
            else
            {
                if( aGraphic.GetType() == GRAPHIC_BITMAP )
                {
                    GraphicFilter*  pFilter = GetGrfFilter();
                    String          aFormat;

                    if( aGraphic.IsAnimated() )
                        aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "gif" ) );
                    else
                        aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "png" ) );

                    bRet = ( pFilter->ExportGraphic( aGraphic, String(), *pStm, pFilter->GetExportFormatNumberForShortName( aFormat ) ) == 0 );
                }
                else if( aGraphic.GetType() == GRAPHIC_GDIMETAFILE )
                {
                    ( (GDIMetaFile&) aGraphic.GetGDIMetaFile() ).Write( *pStm );
                    bRet = ( pStm->GetError() == 0 );
                }
            }

            if( bRet )
            {
                pStm->Seek( 0 );
                mxStmWrapper = new ::utl::OInputStreamWrapper( pStm, sal_True );
            }
            else
                delete pStm;
        }
    }
}

// -----------------------------------------------------------------------------

SvXMLGraphicInputStream::~SvXMLGraphicInputStream()
{
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL SvXMLGraphicInputStream::readBytes( Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead )
    throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException();

    return mxStmWrapper->readBytes( rData, nBytesToRead );
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL SvXMLGraphicInputStream::readSomeBytes( Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead )
    throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    return mxStmWrapper->readSomeBytes( rData, nMaxBytesToRead );
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLGraphicInputStream::skipBytes( sal_Int32 nBytesToSkip )
    throw( NotConnectedException, BufferSizeExceededException, RuntimeException )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->skipBytes( nBytesToSkip );
}

// -----------------------------------------------------------------------------

sal_Int32 SAL_CALL SvXMLGraphicInputStream::available() throw( NotConnectedException, RuntimeException )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    return mxStmWrapper->available();
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLGraphicInputStream::closeInput() throw( NotConnectedException, RuntimeException )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->closeInput();
}

// ----------------------------
// - SvXMLGraphicOutputStream -
// ----------------------------

class SvXMLGraphicOutputStream : public::cppu::WeakImplHelper1< XOutputStream >
{
private:

    // XOutputStream
    virtual void SAL_CALL           writeBytes( const Sequence< sal_Int8 >& rData ) throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException );
    virtual void SAL_CALL           flush() throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException );
    virtual void SAL_CALL           closeOutput() throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException );

private:

    ::utl::TempFile*                mpTmp;
    SvStream*                       mpOStm;
    Reference< XOutputStream >      mxStmWrapper;
    GraphicObject                   maGrfObj;
    sal_Bool                        mbClosed;

                                    // not available
                                    SvXMLGraphicOutputStream( const SvXMLGraphicOutputStream& );
    SvXMLGraphicOutputStream&       operator==( SvXMLGraphicOutputStream& );

public:

                                    SvXMLGraphicOutputStream();
    virtual                         ~SvXMLGraphicOutputStream();

    sal_Bool                        Exists() const { return mxStmWrapper.is(); }
    const GraphicObject&            GetGraphicObject();
};

// -----------------------------------------------------------------------------

SvXMLGraphicOutputStream::SvXMLGraphicOutputStream() :
    mpTmp( new ::utl::TempFile ),
    mbClosed( sal_False )
{
    mpTmp->EnableKillingFile();

    mpOStm = ::utl::UcbStreamHelper::CreateStream( mpTmp->GetURL(), STREAM_WRITE | STREAM_TRUNC );

    if( mpOStm )
        mxStmWrapper = new ::utl::OOutputStreamWrapper( *mpOStm );
}

// -----------------------------------------------------------------------------

SvXMLGraphicOutputStream::~SvXMLGraphicOutputStream()
{
    delete mpTmp;
    delete mpOStm;
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLGraphicOutputStream::writeBytes( const Sequence< sal_Int8 >& rData )
    throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->writeBytes( rData );
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLGraphicOutputStream::flush()
    throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->flush();
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLGraphicOutputStream::closeOutput()
    throw( NotConnectedException, BufferSizeExceededException, IOException, RuntimeException )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->closeOutput();
    mxStmWrapper = Reference< XOutputStream >();

    mbClosed = sal_True;
}

// ------------------------------------------------------------------------------

const GraphicObject& SvXMLGraphicOutputStream::GetGraphicObject()
{
    if( mbClosed && ( maGrfObj.GetType() == GRAPHIC_NONE ) && mpOStm )
    {
        Graphic aGraphic;

        mpOStm->Seek( 0 );
        GetGrfFilter()->ImportGraphic( aGraphic, String(), *mpOStm );

        maGrfObj = aGraphic;

        if( maGrfObj.GetType() != GRAPHIC_NONE )
        {
            delete mpOStm, mpOStm = NULL;
            delete mpTmp, mpTmp = NULL;
        }
    }

    return maGrfObj;
}

// ----------------------
// - SvXMLGraphicHelper -
// ----------------------

SvXMLGraphicHelper::SvXMLGraphicHelper( SvXMLGraphicHelperMode eCreateMode ) :
    ::cppu::WeakComponentImplHelper2< ::com::sun::star::document::XGraphicObjectResolver,
                                      ::com::sun::star::document::XBinaryStreamResolver >( maMutex )
{
    Init( NULL, eCreateMode, sal_False );
}

SvXMLGraphicHelper::SvXMLGraphicHelper() :
    ::cppu::WeakComponentImplHelper2< ::com::sun::star::document::XGraphicObjectResolver,
                                      ::com::sun::star::document::XBinaryStreamResolver >( maMutex )
{
}

// -----------------------------------------------------------------------------

SvXMLGraphicHelper::~SvXMLGraphicHelper()
{
}

// -----------------------------------------------------------------------------

void SAL_CALL SvXMLGraphicHelper::disposing()
{
    Flush();
}

// -----------------------------------------------------------------------------

sal_Bool SvXMLGraphicHelper::ImplGetStreamNames( const ::rtl::OUString& rURLStr,
                                                 ::rtl::OUString& rPictureStorageName,
                                                 ::rtl::OUString& rPictureStreamName )
{
    String      aURLStr( rURLStr );
    sal_Bool    bRet = sal_False;

    if( aURLStr.Len() )
    {
        aURLStr = aURLStr.GetToken( aURLStr.GetTokenCount( ':' ) - 1, ':' );
        const sal_uInt32 nTokenCount = aURLStr.GetTokenCount( '/' );

        if( 1 == nTokenCount )
        {
            rPictureStorageName = String( RTL_CONSTASCII_USTRINGPARAM( XML_GRAPHICSTORAGE_NAME ) );
            rPictureStreamName = aURLStr;
            bRet = sal_True;
        }
        else if( 2 == nTokenCount )
        {
            rPictureStorageName = aURLStr.GetToken( 0, '/' );

            DBG_ASSERT( rPictureStorageName.getLength() &&
                       rPictureStorageName.getStr()[ 0 ] != '#',
                       "invalid relative URL" );

            rPictureStreamName = aURLStr.GetToken( 1, '/' );
            bRet = sal_True;
        }
        else
        {
            DBG_ERROR( "SvXMLGraphicHelper::ImplInsertGraphicURL: invalid scheme" );
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

SotStorageRef SvXMLGraphicHelper::ImplGetGraphicStorage( const ::rtl::OUString& rStorageName )
{
    if( mpRootStorage && ( !mxGraphicStorage.Is() || ( rStorageName != maCurStorageName ) ) )
    {
        if( mxGraphicStorage.Is() && GRAPHICHELPER_MODE_WRITE == meCreateMode )
            mxGraphicStorage->Commit();

        mxGraphicStorage = mpRootStorage->OpenUCBStorage( maCurStorageName = rStorageName, STREAM_READ | STREAM_WRITE );
    }

    return mxGraphicStorage;
}

// -----------------------------------------------------------------------------

SotStorageStreamRef SvXMLGraphicHelper::ImplGetGraphicStream( const ::rtl::OUString& rPictureStorageName,
                                                              const ::rtl::OUString& rPictureStreamName,
                                                              BOOL bTruncate )
{
    SotStorageStreamRef xStm;
    SotStorageRef       xStorage( ImplGetGraphicStorage( rPictureStorageName ) );

    if( xStorage.Is() )
    {
        xStm = xStorage->OpenSotStream( rPictureStreamName,
                                        STREAM_READ | ( ( GRAPHICHELPER_MODE_WRITE == meCreateMode ) ?
                                        ( STREAM_WRITE | ( bTruncate ? STREAM_TRUNC : 0 ) ) : 0 ) );

        if( xStm.Is() && ( GRAPHICHELPER_MODE_WRITE == meCreateMode ) )
        {
            OUString aPropName( RTL_CONSTASCII_USTRINGPARAM("Encrypted") );
            sal_Bool bTrue = sal_True;
            uno::Any aAny;

            aAny.setValue( &bTrue, ::getBooleanCppuType() );
            xStm->SetProperty( aPropName, aAny );

        }
    }

    return xStm;
}

// -----------------------------------------------------------------------------

String SvXMLGraphicHelper::ImplGetGraphicMimeType( const String& rFileName ) const
{
    struct XMLGraphicMimeTypeMapper
    {
        const char* pExt;
        const char* pMimeType;
    };

    static XMLGraphicMimeTypeMapper aMapper[] =
    {
        { "gif", "image/gif" },
        { "png", "image/png" },
        { "jpg", "image/jpeg" },
        { "tif", "image/tiff" }
    };

    String aMimeType;

    if( ( rFileName.Len() >= 4 ) && ( rFileName.GetChar( rFileName.Len() - 4 ) == '.' ) )
    {
        const ByteString aExt( rFileName.Copy( rFileName.Len() - 3 ), RTL_TEXTENCODING_ASCII_US );

        for( long i = 0, nCount = sizeof( aMapper ) / sizeof( aMapper[ 0 ] ); ( i < nCount ) && !aMimeType.Len(); i++ )
            if( aExt == aMapper[ i ].pExt )
                aMimeType = String( aMapper[ i ].pMimeType, RTL_TEXTENCODING_ASCII_US );
    }

    return aMimeType;
}

// -----------------------------------------------------------------------------

Graphic SvXMLGraphicHelper::ImplReadGraphic( const ::rtl::OUString& rPictureStorageName,
                                             const ::rtl::OUString& rPictureStreamName )
{
    Graphic             aGraphic;
    SotStorageStreamRef xStm( ImplGetGraphicStream( rPictureStorageName, rPictureStreamName, FALSE ) );

    if( xStm.Is() )
        GetGrfFilter()->ImportGraphic( aGraphic, String(), *xStm );

    return aGraphic;
}

// -----------------------------------------------------------------------------

sal_Bool SvXMLGraphicHelper::ImplWriteGraphic( const ::rtl::OUString& rPictureStorageName,
                                               const ::rtl::OUString& rPictureStreamName,
                                               const ::rtl::OUString& rGraphicId )
{
    String          aGraphicId( rGraphicId );
    GraphicObject   aGrfObject( ByteString( aGraphicId, RTL_TEXTENCODING_ASCII_US ) );
    sal_Bool        bRet = sal_False;

    if( aGrfObject.GetType() != GRAPHIC_NONE )
    {
        SotStorageStreamRef xStm( ImplGetGraphicStream( rPictureStorageName, rPictureStreamName, TRUE ) );

        if( xStm.Is() )
        {
            Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
            const GfxLink   aGfxLink( aGraphic.GetLink() );
            const OUString  aMimeType( ImplGetGraphicMimeType( rPictureStreamName ) );
            uno::Any        aAny;

            // set stream properties (MediaType/Compression)
            if( aMimeType.getLength() )
            {
                aAny <<= aMimeType;
                xStm->SetProperty( String( RTL_CONSTASCII_USTRINGPARAM( "MediaType" ) ), aAny );
            }

            const sal_Bool bCompressed = ( ( 0 == aMimeType.getLength() ) || ( aMimeType == OUString::createFromAscii( "image/tiff" ) ) );
            aAny <<= bCompressed;
            xStm->SetProperty( String( RTL_CONSTASCII_USTRINGPARAM( "Compressed" ) ), aAny );

            if( aGfxLink.GetDataSize() )
                xStm->Write( aGfxLink.GetData(), aGfxLink.GetDataSize() );
            else
            {
                if( aGraphic.GetType() == GRAPHIC_BITMAP )
                {
                    GraphicFilter*  pFilter = GetGrfFilter();
                    String          aFormat;

                    if( aGraphic.IsAnimated() )
                        aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "gif" ) );
                    else
                        aFormat = String( RTL_CONSTASCII_USTRINGPARAM( "png" ) );

                    bRet = ( pFilter->ExportGraphic( aGraphic, String(), *xStm,
                                                     pFilter->GetExportFormatNumberForShortName( aFormat ) ) == 0 );
                }
                else if( aGraphic.GetType() == GRAPHIC_GDIMETAFILE )
                {
                    ( (GDIMetaFile&) aGraphic.GetGDIMetaFile() ).Write( *xStm );
                    bRet = ( xStm->GetError() == 0 );
                }
            }

            xStm->Commit();
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::ImplInsertGraphicURL( const ::rtl::OUString& rURLStr, sal_uInt32 nInsertPos )
{
    ::rtl::OUString aPictureStorageName, aPictureStreamName;

    if( ( maURLSet.find( rURLStr ) != maURLSet.end() ) )
    {
        URLPairVector::iterator aIter( maGrfURLs.begin() ), aEnd( maGrfURLs.end() );

        while( aIter != aEnd )
        {
            if( rURLStr == (*aIter).first )
            {
                maGrfURLs[ nInsertPos ].second = (*aIter).second;
                aIter = aEnd;
            }
            else
                aIter++;
        }
    }
    else if( ImplGetStreamNames( rURLStr, aPictureStorageName, aPictureStreamName ) )
    {
        URLPair& rURLPair = maGrfURLs[ nInsertPos ];

        if( GRAPHICHELPER_MODE_READ == meCreateMode )
        {
            const GraphicObject aObj( ImplReadGraphic( aPictureStorageName, aPictureStreamName ) );

            if( aObj.GetType() != GRAPHIC_NONE )
            {
                const static ::rtl::OUString aBaseURL( RTL_CONSTASCII_USTRINGPARAM( XML_GRAPHICOBJECT_URL_BASE ) );

                maGrfObjs.push_back( aObj );
                rURLPair.second = aBaseURL;
                rURLPair.second += String( aObj.GetUniqueID().GetBuffer(), RTL_TEXTENCODING_ASCII_US );
            }
            else
                rURLPair.second = String();
        }
        else
        {
            const String        aGraphicObjectId( aPictureStreamName );
            const GraphicObject aGrfObject( ByteString( aGraphicObjectId, RTL_TEXTENCODING_ASCII_US ) );

            if( aGrfObject.GetType() != GRAPHIC_NONE )
            {
                String          aStreamName( aGraphicObjectId );
                Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
                const GfxLink   aGfxLink( aGraphic.GetLink() );

                if( aGfxLink.GetDataSize() )
                {
                    switch( aGfxLink.GetType() )
                    {
                        case( GFX_LINK_TYPE_EPS_BUFFER ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".eps" ) ); break;
                        case( GFX_LINK_TYPE_NATIVE_GIF ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".gif" ) ); break;
                        case( GFX_LINK_TYPE_NATIVE_JPG ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".jpg" ) ); break;
                        case( GFX_LINK_TYPE_NATIVE_PNG ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".png" ) ); break;
                        case( GFX_LINK_TYPE_NATIVE_TIF ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".tif" ) ); break;
                        case( GFX_LINK_TYPE_NATIVE_WMF ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".wmf" ) ); break;
                        case( GFX_LINK_TYPE_NATIVE_MET ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".met" ) ); break;
                        case( GFX_LINK_TYPE_NATIVE_PCT ): aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".pct" ) ); break;

                        default:
                            aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".grf" ) );
                        break;
                    }
                }
                else
                {
                    if( aGrfObject.GetType() == GRAPHIC_BITMAP )
                    {
                        if( aGrfObject.IsAnimated() )
                            aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".gif" ) );
                        else
                            aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".png" ) );
                    }
                    else if( aGrfObject.GetType() == GRAPHIC_GDIMETAFILE )
                        aStreamName += String( RTL_CONSTASCII_USTRINGPARAM( ".svm" ) );
                }

                if( mbDirect && aStreamName.Len() )
                    ImplWriteGraphic( aPictureStorageName, aStreamName, aGraphicObjectId );

                rURLPair.second = String( RTL_CONSTASCII_USTRINGPARAM( "Pictures/" ) );
                rURLPair.second += aStreamName;
            }
        }

        maURLSet.insert( rURLStr );
    }
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::Init( SotStorage* pXMLStorage,
                               SvXMLGraphicHelperMode eCreateMode,
                               BOOL bDirect )
{
    mpRootStorage = pXMLStorage;
    meCreateMode = eCreateMode;
    mbDirect = ( ( GRAPHICHELPER_MODE_READ == meCreateMode ) ? bDirect : sal_True );
}

// -----------------------------------------------------------------------------

SvXMLGraphicHelper* SvXMLGraphicHelper::Create( SotStorage& rXMLStorage,
                                                SvXMLGraphicHelperMode eCreateMode,
                                                BOOL bDirect )
{
    SvXMLGraphicHelper* pThis = new SvXMLGraphicHelper;

    pThis->acquire();
    pThis->Init( &rXMLStorage, eCreateMode, bDirect );

    return pThis;
}

// -----------------------------------------------------------------------------

SvXMLGraphicHelper* SvXMLGraphicHelper::Create( SvXMLGraphicHelperMode eCreateMode )
{
    SvXMLGraphicHelper* pThis = new SvXMLGraphicHelper;

    pThis->acquire();
    pThis->Init( NULL, eCreateMode, sal_False );

    return pThis;
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::Destroy( SvXMLGraphicHelper* pSvXMLGraphicHelper )
{
    if( pSvXMLGraphicHelper )
    {
        pSvXMLGraphicHelper->dispose();
        pSvXMLGraphicHelper->release();
    }
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::Flush()
{
    if( ( GRAPHICHELPER_MODE_WRITE == meCreateMode ) && !mbDirect )
    {
        ::rtl::OUString     aPictureStorageName, aPictureStreamName;
        URLSet::iterator    aSetIter( maURLSet.begin() ), aSetEnd( maURLSet.end() );

        while( aSetIter != aSetEnd )
        {
            URLPairVector::iterator aPairIter( maGrfURLs.begin() ), aPairEnd( maGrfURLs.end() );

            while( aPairIter != aPairEnd )
            {
                if( *aSetIter == (*aPairIter).first )
                {
                    if( ImplGetStreamNames( (*aPairIter).second, aPictureStorageName, aPictureStreamName ) )
                    {
                        DBG_ASSERT( String( aPictureStreamName ).GetTokenCount( '.' ) == 2, "invalid URL" );
                        ImplWriteGraphic( aPictureStorageName, aPictureStreamName, String( aPictureStreamName ).GetToken( 0, '.' ) );
                    }

                    aPairIter = aPairEnd;
                }
                else
                    aPairIter++;
            }

            aSetIter++;
        }

        mbDirect = sal_True;
    }
    if( GRAPHICHELPER_MODE_WRITE == meCreateMode )
    {
        SotStorageRef xStorage = ImplGetGraphicStorage( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(XML_GRAPHICSTORAGE_NAME) ) );

        if( xStorage.Is() )
            xStorage->Commit();
    }

}

// -----------------------------------------------------------------------------

// XGraphicObjectResolver
OUString SAL_CALL SvXMLGraphicHelper::resolveGraphicObjectURL( const OUString& aURL )
    throw(uno::RuntimeException)
{
    ::osl::MutexGuard   aGuard( maMutex );
    const sal_Int32     nIndex = maGrfURLs.size();

    maGrfURLs.push_back( ::std::make_pair( aURL, ::rtl::OUString() ) );
    ImplInsertGraphicURL( aURL, nIndex );

    return maGrfURLs[ nIndex ].second;
}

// -----------------------------------------------------------------------------

// XBinaryStreamResolver
Reference< XInputStream > SAL_CALL SvXMLGraphicHelper::getInputStream( const OUString& rURL )
    throw( RuntimeException )
{
    Reference< XInputStream >   xRet;
    OUString                    aPictureStorageName, aGraphicId;


    if( ( GRAPHICHELPER_MODE_WRITE == meCreateMode ) &&
        ImplGetStreamNames( rURL, aPictureStorageName, aGraphicId ) )
    {
        SvXMLGraphicInputStream* pInputStream = new SvXMLGraphicInputStream( aGraphicId );

        if( pInputStream->Exists() )
            xRet = pInputStream;
        else
            delete pInputStream;
    }

    return xRet;
}

// -----------------------------------------------------------------------------

Reference< XOutputStream > SAL_CALL SvXMLGraphicHelper::createOutputStream()
    throw( RuntimeException )
{
    Reference< XOutputStream > xRet;

    if( GRAPHICHELPER_MODE_READ == meCreateMode )
    {
        SvXMLGraphicOutputStream* pOutputStream = new SvXMLGraphicOutputStream;

        if( pOutputStream->Exists() )
            maGrfStms.push_back( xRet = pOutputStream );
        else
            delete pOutputStream;
    }

    return xRet;
}

// -----------------------------------------------------------------------------

OUString SAL_CALL SvXMLGraphicHelper::resolveOutputStream( const Reference< XOutputStream >& rxBinaryStream )
    throw( RuntimeException )
{
    OUString aRet;

    if( ( GRAPHICHELPER_MODE_READ == meCreateMode ) && rxBinaryStream.is() )
    {
        if( ::std::find( maGrfStms.begin(), maGrfStms.end(), rxBinaryStream ) != maGrfStms.end() )
        {
            SvXMLGraphicOutputStream* pOStm = static_cast< SvXMLGraphicOutputStream* >( rxBinaryStream.get() );

            if( pOStm )
            {
                const GraphicObject&    rGrfObj = pOStm->GetGraphicObject();
                const OUString          aId( OUString::createFromAscii( rGrfObj.GetUniqueID().GetBuffer() ) );

                if( aId.getLength() )
                {
                    aRet = OUString::createFromAscii( XML_GRAPHICOBJECT_URL_BASE );
                    aRet += aId;
                }
            }
        }
    }

    return aRet;
}
