/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#include <comphelper/string.hxx>
#include <sal/macros.h>
#include <rtl/strbuf.hxx>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/compbase4.hxx>

#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/saveopt.hxx>
#include <tools/debug.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/metaact.hxx>
#include <tools/zcodec.hxx>

#include "svtools/filter.hxx"
#include "svx/xmlgrhlp.hxx"
#include "svx/xmleohlp.hxx"

#include <algorithm>

// -----------
// - Defines -
// -----------

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;

using ::com::sun::star::lang::XMultiServiceFactory;

#define XML_GRAPHICSTORAGE_NAME     "Pictures"
#define XML_GRAPHICOBJECT_URL_BASE  "vnd.sun.star.GraphicObject:"

// ---------------------------
// - SvXMLGraphicInputStream -
// ---------------------------

const MetaCommentAction* ImplCheckForEPS( GDIMetaFile& rMtf )
{
    const MetaCommentAction* pComment = NULL;

    if ( ( rMtf.GetActionSize() >= 2 )
            && ( rMtf.FirstAction()->GetType() == META_EPS_ACTION )
            && ( ((const MetaAction*)rMtf.GetAction( 1 ))->GetType() == META_COMMENT_ACTION )
            && ( ((const MetaCommentAction*)rMtf.GetAction( 1 ))->GetComment().equalsL(RTL_CONSTASCII_STRINGPARAM("EPSReplacementGraphic")) ) )
        pComment = (const MetaCommentAction*)rMtf.GetAction( 1 );

    return pComment;
}

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

                                    SvXMLGraphicInputStream( const ::rtl::OUString& rGraphicId );
    virtual                         ~SvXMLGraphicInputStream();

    sal_Bool                        Exists() const { return mxStmWrapper.is(); }
};

// -----------------------------------------------------------------------------

SvXMLGraphicInputStream::SvXMLGraphicInputStream( const ::rtl::OUString& rGraphicId )
{
    GraphicObject   aGrfObject( rtl::OUStringToOString(rGraphicId, RTL_TEXTENCODING_ASCII_US) );

    maTmp.EnableKillingFile();

    if( aGrfObject.GetType() != GRAPHIC_NONE )
    {
        SvStream* pStm = ::utl::UcbStreamHelper::CreateStream( maTmp.GetURL(), STREAM_WRITE | STREAM_TRUNC );

        if( pStm )
        {
            Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
            const GfxLink   aGfxLink( aGraphic.GetLink() );
            sal_Bool        bRet = sal_False;

            if( aGfxLink.GetDataSize() && aGfxLink.GetData() )
            {
                pStm->Write( aGfxLink.GetData(), aGfxLink.GetDataSize() );
                bRet = ( pStm->GetError() == 0 );
            }
            else
            {
                if( aGraphic.GetType() == GRAPHIC_BITMAP )
                {
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    String          aFormat;

                    if( aGraphic.IsAnimated() )
                        aFormat = String(  "gif"  );
                    else
                        aFormat = String(  "png"  );

                    bRet = ( rFilter.ExportGraphic( aGraphic, String(), *pStm, rFilter.GetExportFormatNumberForShortName( aFormat ) ) == 0 );
                }
                else if( aGraphic.GetType() == GRAPHIC_GDIMETAFILE )
                {
                    pStm->SetVersion( SOFFICE_FILEFORMAT_8 );
                    pStm->SetCompressMode( COMPRESSMODE_ZBITMAP );
                    ( (GDIMetaFile&) aGraphic.GetGDIMetaFile() ).Write( *pStm, GDIMETAFILE_WRITE_REPLACEMENT_RENDERGRAPHIC );
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
        sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW;
        sal_uInt16 pDeterminedFormat = GRFILTER_FORMAT_DONTKNOW;
        GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, String(), *mpOStm ,nFormat,&pDeterminedFormat );

        if (pDeterminedFormat == GRFILTER_FORMAT_DONTKNOW)
        {
            //Read the first two byte to check whether it is a gzipped stream, is so it may be in wmz or emz format
            //unzip them and try again

            sal_uInt8    sFirstBytes[ 2 ];

            mpOStm->Seek( STREAM_SEEK_TO_END );
            sal_uIntPtr nStreamLen = mpOStm->Tell();
            mpOStm->Seek( 0 );

            if ( !nStreamLen )
            {
                SvLockBytes* pLockBytes = mpOStm->GetLockBytes();
                if ( pLockBytes  )
                    pLockBytes->SetSynchronMode( sal_True );

                mpOStm->Seek( STREAM_SEEK_TO_END );
                nStreamLen = mpOStm->Tell();
                mpOStm->Seek( 0 );
            }
            if( nStreamLen >= 2 )
            {
                //read two byte
                mpOStm->Read( sFirstBytes, 2 );

                if( sFirstBytes[0] == 0x1f && sFirstBytes[1] == 0x8b )
                {
                    SvMemoryStream* pDest = new SvMemoryStream;
                    ZCodec aZCodec( 0x8000, 0x8000 );
                    aZCodec.BeginCompression(ZCODEC_GZ_LIB);
                    mpOStm->Seek( 0 );
                    aZCodec.Decompress( *mpOStm, *pDest );

                    if (aZCodec.EndCompression() && pDest )
                    {
                        pDest->Seek( STREAM_SEEK_TO_END );
                        sal_uIntPtr nStreamLen_ = pDest->Tell();
                        if (nStreamLen_)
                        {
                            pDest->Seek(0L);
                            GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, String(), *pDest ,nFormat,&pDeterminedFormat );
                        }
                    }
                    delete pDest;
                }
            }
        }

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
        aURLStr = aURLStr.GetToken( comphelper::string::getTokenCount(aURLStr, ':') - 1, ':' );

        const sal_uInt32 nTokenCount = comphelper::string::getTokenCount(aURLStr, '/');

        if( 1 == nTokenCount )
        {
            rPictureStorageName = String(  XML_GRAPHICSTORAGE_NAME  );
            rPictureStreamName = aURLStr;
        }
        else
            SvXMLEmbeddedObjectHelper::splitObjectURL(aURLStr, rPictureStorageName, rPictureStreamName);

        bRet = !rPictureStreamName.isEmpty();
        SAL_WARN_IF(!bRet, "svx", "SvXMLGraphicHelper::ImplInsertGraphicURL: invalid scheme: " << rURLStr);
    }

    return bRet;
}

// -----------------------------------------------------------------------------

uno::Reference < embed::XStorage > SvXMLGraphicHelper::ImplGetGraphicStorage( const ::rtl::OUString& rStorageName )
{
    uno::Reference < embed::XStorage > xRetStorage;
    if( mxRootStorage.is() )
    {
        try
        {
            xRetStorage = mxRootStorage->openStorageElement(
                maCurStorageName = rStorageName,
                ( GRAPHICHELPER_MODE_WRITE == meCreateMode )
                    ? embed::ElementModes::READWRITE
                    : embed::ElementModes::READ );
        }
        catch ( uno::Exception& )
        {
        }
        //#i43196# try again to open the storage element - this time readonly
        if(!xRetStorage.is())
        {
            try
            {
                xRetStorage = mxRootStorage->openStorageElement( maCurStorageName = rStorageName, embed::ElementModes::READ );
            }
            catch ( uno::Exception& )
            {
            }
        }
    }

    return xRetStorage;
}

// -----------------------------------------------------------------------------

SvxGraphicHelperStream_Impl SvXMLGraphicHelper::ImplGetGraphicStream( const ::rtl::OUString& rPictureStorageName,
                                                              const ::rtl::OUString& rPictureStreamName,
                                                              sal_Bool bTruncate )
{
    SvxGraphicHelperStream_Impl aRet;
    aRet.xStorage = ImplGetGraphicStorage( rPictureStorageName );

    if( aRet.xStorage.is() )
    {
        sal_Int32 nMode = embed::ElementModes::READ;
        if ( GRAPHICHELPER_MODE_WRITE == meCreateMode )
        {
            nMode = embed::ElementModes::READWRITE;
            if ( bTruncate )
                nMode |= embed::ElementModes::TRUNCATE;
        }

        aRet.xStream = aRet.xStorage->openStreamElement( rPictureStreamName, nMode );
        if( aRet.xStream.is() && ( GRAPHICHELPER_MODE_WRITE == meCreateMode ) )
        {
            ::rtl::OUString aPropName( "UseCommonStoragePasswordEncryption" );
            uno::Reference < beans::XPropertySet > xProps( aRet.xStream, uno::UNO_QUERY );
            xProps->setPropertyValue( aPropName, uno::makeAny( sal_True) );
        }
    }

    return aRet;
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
        { "tif", "image/tiff" },
        { "svg", "image/svg+xml" }
    };

    String aMimeType;

    if( ( rFileName.Len() >= 4 ) && ( rFileName.GetChar( rFileName.Len() - 4 ) == '.' ) )
    {
        const rtl::OString aExt(rtl::OUStringToOString(rFileName.Copy(rFileName.Len() - 3),
            RTL_TEXTENCODING_ASCII_US));

        for( long i = 0, nCount = sizeof (aMapper) / sizeof (aMapper[0]); ( i < nCount ) && !aMimeType.Len(); i++ )
            if( aExt.getStr() == aMapper[ i ].pExt )
                aMimeType = String( aMapper[ i ].pMimeType, RTL_TEXTENCODING_ASCII_US );
    }

    return aMimeType;
}

// -----------------------------------------------------------------------------

Graphic SvXMLGraphicHelper::ImplReadGraphic( const ::rtl::OUString& rPictureStorageName,
                                             const ::rtl::OUString& rPictureStreamName )
{
    Graphic             aGraphic;
    SvxGraphicHelperStream_Impl aStream( ImplGetGraphicStream( rPictureStorageName, rPictureStreamName, sal_False ) );
    if( aStream.xStream.is() )
    {
        SvStream* pStream = utl::UcbStreamHelper::CreateStream( aStream.xStream );
        GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, String(), *pStream );
        delete pStream;
    }

    return aGraphic;
}

// -----------------------------------------------------------------------------

sal_Bool SvXMLGraphicHelper::ImplWriteGraphic( const ::rtl::OUString& rPictureStorageName,
                                               const ::rtl::OUString& rPictureStreamName,
                                               const ::rtl::OUString& rGraphicId,
                                               bool bUseGfxLink )
{
    GraphicObject   aGrfObject( rtl::OUStringToOString(rGraphicId, RTL_TEXTENCODING_ASCII_US) );
    sal_Bool        bRet = sal_False;

    if( aGrfObject.GetType() != GRAPHIC_NONE )
    {
        SvxGraphicHelperStream_Impl aStream( ImplGetGraphicStream( rPictureStorageName, rPictureStreamName, sal_False ) );
        if( aStream.xStream.is() )
        {
            Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
            const GfxLink   aGfxLink( aGraphic.GetLink() );
            const ::rtl::OUString  aMimeType( ImplGetGraphicMimeType( rPictureStreamName ) );
            uno::Any        aAny;
            uno::Reference < beans::XPropertySet > xProps( aStream.xStream, uno::UNO_QUERY );

            // set stream properties (MediaType/Compression)
            if( !aMimeType.isEmpty() )
            {
                aAny <<= aMimeType;
                xProps->setPropertyValue( String(  "MediaType"  ), aAny );
            }

            const sal_Bool bCompressed = aMimeType.isEmpty() || aMimeType == "image/tiff";
            aAny <<= bCompressed;
            xProps->setPropertyValue( String(  "Compressed"  ), aAny );

            SvStream* pStream = utl::UcbStreamHelper::CreateStream( aStream.xStream );
            if( bUseGfxLink && aGfxLink.GetDataSize() && aGfxLink.GetData() )
                pStream->Write( aGfxLink.GetData(), aGfxLink.GetDataSize() );
            else
            {
                if( aGraphic.GetType() == GRAPHIC_BITMAP )
                {
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    String          aFormat;

                    if( aGraphic.IsAnimated() )
                        aFormat = String(  "gif"  );
                    else
                        aFormat = String(  "png"  );

                    bRet = ( rFilter.ExportGraphic( aGraphic, String(), *pStream,
                                                     rFilter.GetExportFormatNumberForShortName( aFormat ) ) == 0 );
                }
                else if( aGraphic.GetType() == GRAPHIC_GDIMETAFILE )
                {
                    pStream->SetVersion( SOFFICE_FILEFORMAT_8 );
                    pStream->SetCompressMode( COMPRESSMODE_ZBITMAP );

                    // SJ: first check if this metafile is just a eps file, then we will store the eps instead of svm
                    GDIMetaFile& rMtf( (GDIMetaFile&)aGraphic.GetGDIMetaFile() );
                    const MetaCommentAction* pComment = ImplCheckForEPS( rMtf );
                    if ( pComment )
                    {
                        sal_uInt32  nSize = pComment->GetDataSize();
                        const sal_uInt8* pData = pComment->GetData();
                        if ( nSize && pData )
                            pStream->Write( pData, nSize );

                        const MetaEPSAction* pAct = ( (const MetaEPSAction*)rMtf.FirstAction() );
                        const GfxLink&       rLink = pAct->GetLink();

                        pStream->Write( rLink.GetData(), rLink.GetDataSize() );
                    }
                    else
                        rMtf.Write( *pStream, GDIMETAFILE_WRITE_REPLACEMENT_RENDERGRAPHIC );

                    bRet = ( pStream->GetError() == 0 );
                }
            }
            uno::Reference < embed::XTransactedObject > xStorage(
                                    aStream.xStorage, uno::UNO_QUERY);
            delete pStream;
            aStream.xStream->getOutputStream()->closeOutput();
            if( xStorage.is() )
                xStorage->commit();
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::ImplInsertGraphicURL( const ::rtl::OUString& rURLStr, sal_uInt32 nInsertPos, rtl::OUString& rRequestedFileName )
{
    rtl::OUString aURLString( rURLStr );
    ::rtl::OUString aPictureStorageName, aPictureStreamName;
    if( ( maURLSet.find( aURLString ) != maURLSet.end() ) )
    {
        URLPairVector::iterator aIter( maGrfURLs.begin() ), aEnd( maGrfURLs.end() );
        while( aIter != aEnd )
        {
            if( aURLString == (*aIter).first )
            {
                maGrfURLs[ nInsertPos ].second = (*aIter).second;
                aIter = aEnd;
            }
            else
                aIter++;
        }
    }
    else if( ImplGetStreamNames( aURLString, aPictureStorageName, aPictureStreamName ) )
    {
        URLPair& rURLPair = maGrfURLs[ nInsertPos ];

        if( GRAPHICHELPER_MODE_READ == meCreateMode )
        {
            const GraphicObject aObj( ImplReadGraphic( aPictureStorageName, aPictureStreamName ) );

            if( aObj.GetType() != GRAPHIC_NONE )
            {
                maGrfObjs.push_back( aObj );
                ::rtl::OUString aBaseURL(  XML_GRAPHICOBJECT_URL_BASE  );

                rURLPair.second = aBaseURL;
                rURLPair.second += rtl::OStringToOUString(aObj.GetUniqueID(),
                    RTL_TEXTENCODING_ASCII_US);
            }
            else
                rURLPair.second = String();
        }
        else
        {
            const String        aGraphicObjectId( aPictureStreamName );
            const rtl::OString aAsciiObjectID(rtl::OUStringToOString(aGraphicObjectId, RTL_TEXTENCODING_ASCII_US));
            const GraphicObject aGrfObject( aAsciiObjectID );
            if( aGrfObject.GetType() != GRAPHIC_NONE )
            {
                String          aStreamName( aGraphicObjectId );
                Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
                const GfxLink   aGfxLink( aGraphic.GetLink() );
                String          aExtension;
                bool            bUseGfxLink( true );

                if( aGfxLink.GetDataSize() )
                {
                    switch( aGfxLink.GetType() )
                    {
                        case( GFX_LINK_TYPE_EPS_BUFFER ): aExtension = String(  ".eps"  ); break;
                        case( GFX_LINK_TYPE_NATIVE_GIF ): aExtension = String(  ".gif"  ); break;
                        case( GFX_LINK_TYPE_NATIVE_JPG ): aExtension = String(  ".jpg"  ); break;
                        case( GFX_LINK_TYPE_NATIVE_PNG ): aExtension = String(  ".png"  ); break;
                        case( GFX_LINK_TYPE_NATIVE_TIF ): aExtension = String(  ".tif"  ); break;
                        case( GFX_LINK_TYPE_NATIVE_WMF ): aExtension = String(  ".wmf"  ); break;
                        case( GFX_LINK_TYPE_NATIVE_MET ): aExtension = String(  ".met"  ); break;
                        case( GFX_LINK_TYPE_NATIVE_PCT ): aExtension = String(  ".pct"  ); break;
                        case( GFX_LINK_TYPE_NATIVE_SVG ): 
                            // backward-compat kludge: since no released OOo
                            // version to date can handle svg properly, wrap it up
                            // into an svm. slight catch22 here, since strict ODF
                            // conformance _recommends_ svg - then again, most old
                            // ODF consumers are believed to be OOo
                            if( SvtSaveOptions().GetODFDefaultVersion() <= SvtSaveOptions::ODFVER_012 )
                            {
                                bUseGfxLink = false;
                                aExtension = String(  ".svm"  );
                            }
                            else
                                aExtension = String(  ".svg"  );
                            break;

                        default:
                            aExtension = String(  ".grf"  );
                        break;
                    }
                }
                else
                {
                    if( aGrfObject.GetType() == GRAPHIC_BITMAP )
                    {
                        if( aGrfObject.IsAnimated() )
                            aExtension = String(  ".gif"  );
                        else
                            aExtension = String(  ".png"  );
                    }
                    else if( aGrfObject.GetType() == GRAPHIC_GDIMETAFILE )
                    {
                        // SJ: first check if this metafile is just a eps file, then we will store the eps instead of svm
                        GDIMetaFile& rMtf( (GDIMetaFile&)aGraphic.GetGDIMetaFile() );
                        if ( ImplCheckForEPS( rMtf ) )
                            aExtension = String(  ".eps"  );
                        else
                            aExtension = String(  ".svm"  );
                    }
                }

                rtl::OUString aURLEntry;
                const String sPictures(  "Pictures/"  );

                if ( !rRequestedFileName.isEmpty() )
                {
                    aURLEntry = sPictures;
                    aURLEntry += rRequestedFileName;
                    aURLEntry += aExtension;

                    URLPairVector::iterator aIter( maGrfURLs.begin() ), aEnd( maGrfURLs.end() );
                    while( aIter != aEnd )
                    {
                        if( aURLEntry == (*aIter).second )
                            break;
                        aIter++;
                    }
                    if ( aIter == aEnd )
                        aStreamName = rRequestedFileName;
                }

                aStreamName += aExtension;

                if( mbDirect && aStreamName.Len() )
                    ImplWriteGraphic( aPictureStorageName, aStreamName, aGraphicObjectId, bUseGfxLink );

                rURLPair.second = sPictures;
                rURLPair.second += aStreamName;
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                rtl::OStringBuffer sMessage(
                    RTL_CONSTASCII_STRINGPARAM("graphic object with ID '"));
                sMessage.append(aAsciiObjectID).
                    append(RTL_CONSTASCII_STRINGPARAM("' has an unknown type"));
                OSL_ENSURE( false, sMessage.getStr() );
            }
#endif
        }

        maURLSet.insert( aURLString );
    }
}

// -----------------------------------------------------------------------------

void SvXMLGraphicHelper::Init( const uno::Reference < embed::XStorage >& rXMLStorage,
                               SvXMLGraphicHelperMode eCreateMode,
                               sal_Bool bDirect )
{
    mxRootStorage = rXMLStorage;
    meCreateMode = eCreateMode;
    mbDirect = ( ( GRAPHICHELPER_MODE_READ == meCreateMode ) ? bDirect : sal_True );
}

// -----------------------------------------------------------------------------

SvXMLGraphicHelper* SvXMLGraphicHelper::Create( const uno::Reference < embed::XStorage >& rXMLStorage,
                                                SvXMLGraphicHelperMode eCreateMode,
                                                sal_Bool bDirect )
{
    SvXMLGraphicHelper* pThis = new SvXMLGraphicHelper;

    pThis->acquire();
    pThis->Init( rXMLStorage, eCreateMode, bDirect );

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

// XGraphicObjectResolver
::rtl::OUString SAL_CALL SvXMLGraphicHelper::resolveGraphicObjectURL( const ::rtl::OUString& rURL )
    throw(uno::RuntimeException)
{
    ::osl::MutexGuard   aGuard( maMutex );
    const sal_Int32     nIndex = maGrfURLs.size();

    rtl::OUString aURL( rURL );
    rtl::OUString aUserData;
    rtl::OUString aRequestedFileName;

    sal_Int32 nUser = rURL.indexOf( '?', 0 );
    if ( nUser >= 0 )
    {
        aURL = rtl::OUString( rURL.copy( 0, nUser ) );
        nUser++;
        aUserData = rURL.copy( nUser, rURL.getLength() - nUser );
    }
    if ( !aUserData.isEmpty() )
    {
        sal_Int32 nIndex2 = 0;
        do
        {
            rtl::OUString aToken = aUserData.getToken( 0, ';', nIndex2 );
            sal_Int32 n = aToken.indexOf( '=' );
            if ( ( n > 0 ) && ( ( n + 1 ) < aToken.getLength() ) )
            {
                rtl::OUString aParam( aToken.copy( 0, n ) );
                rtl::OUString aValue( aToken.copy( n + 1, aToken.getLength() - ( n + 1 ) ) );

                const rtl::OUString sRequestedName( "requestedName" );
                if ( aParam.match( sRequestedName ) )
                    aRequestedFileName = aValue;
            }
        }
        while ( nIndex2 >= 0 );
    }

    maGrfURLs.push_back( ::std::make_pair( aURL, ::rtl::OUString() ) );
    ImplInsertGraphicURL( aURL, nIndex, aRequestedFileName );

    return maGrfURLs[ nIndex ].second;
}

// -----------------------------------------------------------------------------

// XBinaryStreamResolver
Reference< XInputStream > SAL_CALL SvXMLGraphicHelper::getInputStream( const ::rtl::OUString& rURL )
    throw( RuntimeException )
{
    Reference< XInputStream >   xRet;
    ::rtl::OUString                    aPictureStorageName, aGraphicId;


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

::rtl::OUString SAL_CALL SvXMLGraphicHelper::resolveOutputStream( const Reference< XOutputStream >& rxBinaryStream )
    throw( RuntimeException )
{
    ::rtl::OUString aRet;

    if( ( GRAPHICHELPER_MODE_READ == meCreateMode ) && rxBinaryStream.is() )
    {
        if( ::std::find( maGrfStms.begin(), maGrfStms.end(), rxBinaryStream ) != maGrfStms.end() )
        {
            SvXMLGraphicOutputStream* pOStm = static_cast< SvXMLGraphicOutputStream* >( rxBinaryStream.get() );

            if( pOStm )
            {
                const GraphicObject& rGrfObj = pOStm->GetGraphicObject();
                const ::rtl::OUString aId(::rtl::OStringToOUString(
                    rGrfObj.GetUniqueID(), RTL_TEXTENCODING_ASCII_US));

                if( !aId.isEmpty() )
                {
                    aRet = ::rtl::OUString( XML_GRAPHICOBJECT_URL_BASE );
                    aRet += aId;
                }
            }
        }
    }

    return aRet;
}


// --------------------------------------------------------------------------------

// for instantiation via service manager
namespace svx
{

namespace impl
{
typedef ::cppu::WeakComponentImplHelper4<
        lang::XInitialization,
        document::XGraphicObjectResolver,
        document::XBinaryStreamResolver,
        lang::XServiceInfo >
    SvXMLGraphicImportExportHelper_Base;
class MutexContainer
{
public:
    virtual ~MutexContainer();

protected:
    mutable ::osl::Mutex m_aMutex;
};
MutexContainer::~MutexContainer()
{}
} // namespace impl

class SvXMLGraphicImportExportHelper :
    public impl::MutexContainer,
    public impl::SvXMLGraphicImportExportHelper_Base
{
public:
    SvXMLGraphicImportExportHelper( SvXMLGraphicHelperMode eMode );

protected:
    // is called from WeakComponentImplHelper when XComponent::dispose() was
    // called from outside
    virtual void SAL_CALL disposing();

    // ____ XInitialization ____
    // one argument is allowed, which is the XStorage
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments )
        throw (Exception,
               RuntimeException);

    // ____ XGraphicObjectResolver ____
    virtual ::rtl::OUString SAL_CALL resolveGraphicObjectURL( const ::rtl::OUString& aURL )
        throw (RuntimeException);

    // ____ XBinaryStreamResolver ____
    virtual Reference< io::XInputStream > SAL_CALL getInputStream( const ::rtl::OUString& aURL )
        throw (RuntimeException);
    virtual Reference< io::XOutputStream > SAL_CALL createOutputStream()
        throw (RuntimeException);
    virtual ::rtl::OUString SAL_CALL resolveOutputStream( const Reference< io::XOutputStream >& aBinaryStream )
        throw (RuntimeException);

    // ____ XServiceInfo ____
    virtual ::rtl::OUString SAL_CALL getImplementationName()
        throw (RuntimeException);
    virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName )
        throw (RuntimeException);
    virtual Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames()
        throw (RuntimeException);

private:
    SvXMLGraphicHelperMode              m_eGraphicHelperMode;
    Reference< XGraphicObjectResolver > m_xGraphicObjectResolver;
    Reference< XBinaryStreamResolver >  m_xBinaryStreamResolver;
};

SvXMLGraphicImportExportHelper::SvXMLGraphicImportExportHelper( SvXMLGraphicHelperMode eMode ) :
        impl::SvXMLGraphicImportExportHelper_Base( m_aMutex ),
        m_eGraphicHelperMode( eMode )
{}

void SAL_CALL SvXMLGraphicImportExportHelper::disposing()
{
    Reference< XComponent > xComp( m_xGraphicObjectResolver, UNO_QUERY );
    OSL_ASSERT( xComp.is());
    if( xComp.is())
        xComp->dispose();
    // m_xBinaryStreamResolver is a reference to the same object => don't call
    // dispose() again
}

// ____ XInitialization ____
void SAL_CALL SvXMLGraphicImportExportHelper::initialize(
    const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException)
{
    Reference< embed::XStorage > xStorage;
    if( aArguments.getLength() > 0 )
        aArguments[0] >>= xStorage;

    SvXMLGraphicHelper * pHelper( SvXMLGraphicHelper::Create( xStorage, m_eGraphicHelperMode ));
    m_xGraphicObjectResolver.set( pHelper );
    m_xBinaryStreamResolver.set( pHelper );
    // SvXMLGraphicHelper::Create calls acquire.  Since we have two references
    // now it is safe (and necessary) to undo this acquire
    pHelper->release();
}

// ____ XGraphicObjectResolver ____
::rtl::OUString SAL_CALL SvXMLGraphicImportExportHelper::resolveGraphicObjectURL( const ::rtl::OUString& aURL )
    throw (uno::RuntimeException)
{
    return m_xGraphicObjectResolver->resolveGraphicObjectURL( aURL );
}


// ____ XBinaryStreamResolver ____
Reference< io::XInputStream > SAL_CALL SvXMLGraphicImportExportHelper::getInputStream( const ::rtl::OUString& aURL )
    throw (uno::RuntimeException)
{
    return m_xBinaryStreamResolver->getInputStream( aURL );
}
Reference< io::XOutputStream > SAL_CALL SvXMLGraphicImportExportHelper::createOutputStream()
    throw (uno::RuntimeException)
{
    return m_xBinaryStreamResolver->createOutputStream();
}
::rtl::OUString SAL_CALL SvXMLGraphicImportExportHelper::resolveOutputStream( const Reference< io::XOutputStream >& aBinaryStream )
    throw (uno::RuntimeException)
{
    return m_xBinaryStreamResolver->resolveOutputStream( aBinaryStream );
}

// ____ XServiceInfo ____
::rtl::OUString SAL_CALL SvXMLGraphicImportExportHelper::getImplementationName()
    throw (uno::RuntimeException)
{
    if( m_eGraphicHelperMode == GRAPHICHELPER_MODE_READ )
        return SvXMLGraphicImportHelper_getImplementationName();
    return SvXMLGraphicExportHelper_getImplementationName();
}
::sal_Bool SAL_CALL SvXMLGraphicImportExportHelper::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    Sequence< ::rtl::OUString > aServiceNames( getSupportedServiceNames());
    const ::rtl::OUString * pBegin = aServiceNames.getConstArray();
    const ::rtl::OUString * pEnd = pBegin + aServiceNames.getLength();
    return (::std::find( pBegin, pEnd, ServiceName ) != pEnd);
}
Sequence< ::rtl::OUString > SAL_CALL SvXMLGraphicImportExportHelper::getSupportedServiceNames()
    throw (uno::RuntimeException)
{
    if( m_eGraphicHelperMode == GRAPHICHELPER_MODE_READ )
        return SvXMLGraphicImportHelper_getSupportedServiceNames();
    return SvXMLGraphicExportHelper_getSupportedServiceNames();
}

// import
Reference< XInterface > SAL_CALL SvXMLGraphicImportHelper_createInstance(const Reference< XMultiServiceFactory > & /* rSMgr */ )
    throw( Exception )
{
    return static_cast< XWeak* >( new SvXMLGraphicImportExportHelper( GRAPHICHELPER_MODE_READ ));
}
::rtl::OUString SAL_CALL SvXMLGraphicImportHelper_getImplementationName()
    throw()
{
    return ::rtl::OUString(  "com.sun.star.comp.Svx.GraphicImportHelper" );
}
Sequence< ::rtl::OUString > SAL_CALL SvXMLGraphicImportHelper_getSupportedServiceNames()
    throw()
{
    // XGraphicObjectResolver and XBinaryStreamResolver are not part of any service
    Sequence< ::rtl::OUString > aSupportedServiceNames( 2 );
    aSupportedServiceNames[0] = ::rtl::OUString(  "com.sun.star.document.GraphicObjectResolver"  );
    aSupportedServiceNames[1] = ::rtl::OUString(  "com.sun.star.document.BinaryStreamResolver"  );
    return aSupportedServiceNames;
}

// export
Reference< XInterface > SAL_CALL SvXMLGraphicExportHelper_createInstance(const Reference< XMultiServiceFactory > & /* rSMgr */ )
    throw( Exception )
{
    return static_cast< XWeak* >( new SvXMLGraphicImportExportHelper( GRAPHICHELPER_MODE_WRITE ));
}
::rtl::OUString SAL_CALL SvXMLGraphicExportHelper_getImplementationName()
    throw()
{
    return ::rtl::OUString(  "com.sun.star.comp.Svx.GraphicExportHelper" );
}
Sequence< ::rtl::OUString > SAL_CALL SvXMLGraphicExportHelper_getSupportedServiceNames()
    throw()
{
    // XGraphicObjectResolver and XBinaryStreamResolver are not part of any service
    Sequence< ::rtl::OUString > aSupportedServiceNames( 2 );
    aSupportedServiceNames[0] = ::rtl::OUString(  "com.sun.star.document.GraphicObjectResolver"  );
    aSupportedServiceNames[1] = ::rtl::OUString(  "com.sun.star.document.BinaryStreamResolver"  );
    return aSupportedServiceNames;
}

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
