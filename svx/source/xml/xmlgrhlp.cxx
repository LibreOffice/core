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

#include <comphelper/string.hxx>
#include <sal/macros.h>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <rtl/ref.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/saveopt.hxx>
#include <vcl/cvtgrf.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/metaact.hxx>
#include <tools/zcodec.hxx>

#include <vcl/graphicfilter.hxx>
#include "svx/xmlgrhlp.hxx"
#include "svx/xmleohlp.hxx"

#include <algorithm>
#include <memory>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;


#define XML_GRAPHICSTORAGE_NAME     "Pictures"
#define XML_GRAPHICOBJECT_URL_BASE  "vnd.sun.star.GraphicObject:"

namespace {

const MetaCommentAction* ImplCheckForEPS( GDIMetaFile& rMtf )
{
    const MetaCommentAction* pComment = nullptr;

    if ( rMtf.GetActionSize() >= 2
            && rMtf.GetAction(0)->GetType() == MetaActionType::EPS
            && rMtf.GetAction(1)->GetType() == MetaActionType::COMMENT
            && ( static_cast<const MetaCommentAction*>(rMtf.GetAction( 1 ))->GetComment() == "EPSReplacementGraphic" ) )
        pComment = static_cast<const MetaCommentAction*>(rMtf.GetAction( 1 ));

    return pComment;
}

class SvXMLGraphicInputStream:
    public cppu::WeakImplHelper<XInputStream>
{
private:

    virtual sal_Int32   SAL_CALL    readBytes( Sequence< sal_Int8 >& aData, sal_Int32 nBytesToRead) override;
    virtual sal_Int32   SAL_CALL    readSomeBytes(Sequence< sal_Int8 >& aData, sal_Int32 nMaxBytesToRead) override;
    virtual void        SAL_CALL    skipBytes(sal_Int32 nBytesToSkip) override;
    virtual sal_Int32   SAL_CALL    available() override;
    virtual void        SAL_CALL    closeInput() override;

private:

    ::utl::TempFile                 maTmp;
    Reference< XInputStream >       mxStmWrapper;

public:

    explicit                        SvXMLGraphicInputStream( const OUString& rGraphicId );
                                    SvXMLGraphicInputStream(const SvXMLGraphicInputStream&) = delete;
    SvXMLGraphicInputStream&        operator=(const SvXMLGraphicInputStream&) = delete;

    bool                            Exists() const { return mxStmWrapper.is(); }
};

SvXMLGraphicInputStream::SvXMLGraphicInputStream( const OUString& rGraphicId )
{
    GraphicObject   aGrfObject( OUStringToOString(rGraphicId, RTL_TEXTENCODING_ASCII_US) );

    maTmp.EnableKillingFile();

    if( aGrfObject.GetType() != GraphicType::NONE )
    {
        SvStream* pStm = ::utl::UcbStreamHelper::CreateStream( maTmp.GetURL(), StreamMode::WRITE | StreamMode::TRUNC );

        if( pStm )
        {
            Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
            const GfxLink   aGfxLink( aGraphic.GetLink() );
            bool        bRet = false;

            if( aGfxLink.GetDataSize() && aGfxLink.GetData() )
            {
                pStm->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                bRet = ( pStm->GetError() == 0 );
            }
            else
            {
                if( aGraphic.GetType() == GraphicType::Bitmap )
                {
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    OUString          aFormat;

                    if( aGraphic.IsAnimated() )
                        aFormat = "gif";
                    else
                        aFormat = "png";

                    bRet = ( rFilter.ExportGraphic( aGraphic, "", *pStm, rFilter.GetExportFormatNumberForShortName( aFormat ) ) == 0 );
                }
                else if( aGraphic.GetType() == GraphicType::GdiMetafile )
                {
                    pStm->SetVersion( SOFFICE_FILEFORMAT_8 );
                    pStm->SetCompressMode( SvStreamCompressFlags::ZBITMAP );
                    const_cast<GDIMetaFile&>( aGraphic.GetGDIMetaFile() ).Write( *pStm );
                    bRet = ( pStm->GetError() == 0 );
                }
            }

            if( bRet )
            {
                pStm->Seek( 0 );
                mxStmWrapper = new ::utl::OInputStreamWrapper( pStm, true );
            }
            else
                delete pStm;
        }
    }
}

sal_Int32 SAL_CALL SvXMLGraphicInputStream::readBytes( Sequence< sal_Int8 >& rData, sal_Int32 nBytesToRead )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException();

    return mxStmWrapper->readBytes( rData, nBytesToRead );
}

sal_Int32 SAL_CALL SvXMLGraphicInputStream::readSomeBytes( Sequence< sal_Int8 >& rData, sal_Int32 nMaxBytesToRead )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    return mxStmWrapper->readSomeBytes( rData, nMaxBytesToRead );
}

void SAL_CALL SvXMLGraphicInputStream::skipBytes( sal_Int32 nBytesToSkip )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->skipBytes( nBytesToSkip );
}

sal_Int32 SAL_CALL SvXMLGraphicInputStream::available()
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    return mxStmWrapper->available();
}

void SAL_CALL SvXMLGraphicInputStream::closeInput()
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->closeInput();
}

class SvXMLGraphicOutputStream:
    public cppu::WeakImplHelper<XOutputStream>
{
private:

    // XOutputStream
    virtual void SAL_CALL           writeBytes( const Sequence< sal_Int8 >& rData ) override;
    virtual void SAL_CALL           flush() override;
    virtual void SAL_CALL           closeOutput() override;

private:

    ::utl::TempFile*                mpTmp;
    SvStream*                       mpOStm;
    Reference< XOutputStream >      mxStmWrapper;
    std::unique_ptr<GraphicObject>  mxGrfObj;
    bool                        mbClosed;

public:

                                    SvXMLGraphicOutputStream();
    virtual                         ~SvXMLGraphicOutputStream() override;
                                    SvXMLGraphicOutputStream(const SvXMLGraphicOutputStream&) = delete;
    SvXMLGraphicOutputStream&       operator=(const SvXMLGraphicOutputStream&) = delete;

    bool                            Exists() const { return mxStmWrapper.is(); }
    const GraphicObject&            GetGraphicObject();
};

SvXMLGraphicOutputStream::SvXMLGraphicOutputStream()
    : mpTmp(new ::utl::TempFile)
    , mxGrfObj(new GraphicObject)
    , mbClosed(false)
{
    mpTmp->EnableKillingFile();

    mpOStm = ::utl::UcbStreamHelper::CreateStream( mpTmp->GetURL(), StreamMode::WRITE | StreamMode::TRUNC );

    if( mpOStm )
        mxStmWrapper = new ::utl::OOutputStreamWrapper( *mpOStm );
}

SvXMLGraphicOutputStream::~SvXMLGraphicOutputStream()
{
    delete mpTmp;
    delete mpOStm;
}

void SAL_CALL SvXMLGraphicOutputStream::writeBytes( const Sequence< sal_Int8 >& rData )
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->writeBytes( rData );
}

void SAL_CALL SvXMLGraphicOutputStream::flush()
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->flush();
}

void SAL_CALL SvXMLGraphicOutputStream::closeOutput()
{
    if( !mxStmWrapper.is() )
        throw NotConnectedException() ;

    mxStmWrapper->closeOutput();
    mxStmWrapper.clear();

    mbClosed = true;
}

const GraphicObject& SvXMLGraphicOutputStream::GetGraphicObject()
{
    if (mbClosed && mxGrfObj->GetType() == GraphicType::NONE && mpOStm)
    {
        Graphic aGraphic;

        mpOStm->Seek( 0 );
        sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW;
        sal_uInt16 nDeterminedFormat = GRFILTER_FORMAT_DONTKNOW;
        GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, "", *mpOStm ,nFormat,&nDeterminedFormat );

        if (nDeterminedFormat == GRFILTER_FORMAT_DONTKNOW)
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
                    pLockBytes->SetSynchronMode();

                mpOStm->Seek( STREAM_SEEK_TO_END );
                nStreamLen = mpOStm->Tell();
                mpOStm->Seek( 0 );
            }
            if( nStreamLen >= 2 )
            {
                //read two byte
                mpOStm->ReadBytes(sFirstBytes, 2);

                if( sFirstBytes[0] == 0x1f && sFirstBytes[1] == 0x8b )
                {
                    std::unique_ptr<SvMemoryStream> pDest(new SvMemoryStream);
                    ZCodec aZCodec( 0x8000, 0x8000 );
                    aZCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, false, true);
                    mpOStm->Seek( 0 );
                    aZCodec.Decompress( *mpOStm, *pDest );

                    if (aZCodec.EndCompression() && pDest )
                    {
                        pDest->Seek( STREAM_SEEK_TO_END );
                        sal_uIntPtr nStreamLen_ = pDest->Tell();
                        if (nStreamLen_)
                        {
                            pDest->Seek(0L);
                            GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, "", *pDest ,nFormat,&nDeterminedFormat );
                        }
                    }
                }
            }
        }

        mxGrfObj.reset(new GraphicObject(aGraphic));
        if (mxGrfObj->GetType() != GraphicType::NONE)
        {
            delete mpOStm;
            mpOStm = nullptr;
            delete mpTmp;
            mpTmp = nullptr;
        }
    }

    return *mxGrfObj;
}

}

SvXMLGraphicHelper::SvXMLGraphicHelper( SvXMLGraphicHelperMode eCreateMode ) :
    cppu::WeakComponentImplHelper< css::document::XGraphicObjectResolver,
                                      css::document::XBinaryStreamResolver >( maMutex )
{
    Init( nullptr, eCreateMode, false );
}

SvXMLGraphicHelper::SvXMLGraphicHelper()
    : cppu::WeakComponentImplHelper< css::document::XGraphicObjectResolver,
                                      css::document::XBinaryStreamResolver >( maMutex )
    , meCreateMode(SvXMLGraphicHelperMode::Read)
    , mbDirect(false)
{
}

SvXMLGraphicHelper::~SvXMLGraphicHelper()
{
}

void SAL_CALL SvXMLGraphicHelper::disposing()
{
}

bool SvXMLGraphicHelper::ImplGetStreamNames( const OUString& rURLStr,
                                                 OUString& rPictureStorageName,
                                                 OUString& rPictureStreamName )
{
    OUString      aURLStr( rURLStr );
    bool    bRet = false;

    if( !aURLStr.isEmpty() )
    {
        aURLStr = aURLStr.getToken( comphelper::string::getTokenCount(aURLStr, ':') - 1, ':' );

        const sal_uInt32 nTokenCount = comphelper::string::getTokenCount(aURLStr, '/');

        if( 1 == nTokenCount )
        {
            rPictureStorageName = XML_GRAPHICSTORAGE_NAME;
            rPictureStreamName = aURLStr;
        }
        else
            SvXMLEmbeddedObjectHelper::splitObjectURL(aURLStr, rPictureStorageName, rPictureStreamName);

        bRet = !rPictureStreamName.isEmpty();
        SAL_WARN_IF(!bRet, "svx", "SvXMLGraphicHelper::ImplInsertGraphicURL: invalid scheme: " << rURLStr);
    }

    return bRet;
}

uno::Reference < embed::XStorage > SvXMLGraphicHelper::ImplGetGraphicStorage( const OUString& rStorageName )
{
    uno::Reference < embed::XStorage > xRetStorage;
    if( mxRootStorage.is() )
    {
        try
        {
            xRetStorage = mxRootStorage->openStorageElement(
                maCurStorageName = rStorageName,
                ( SvXMLGraphicHelperMode::Write == meCreateMode )
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

SvxGraphicHelperStream_Impl SvXMLGraphicHelper::ImplGetGraphicStream( const OUString& rPictureStorageName,
                                                              const OUString& rPictureStreamName )
{
    SvxGraphicHelperStream_Impl aRet;
    aRet.xStorage = ImplGetGraphicStorage( rPictureStorageName );

    if( aRet.xStorage.is() )
    {
        sal_Int32 nMode = embed::ElementModes::READ;
        if ( SvXMLGraphicHelperMode::Write == meCreateMode )
        {
            nMode = embed::ElementModes::READWRITE;
        }

        aRet.xStream = aRet.xStorage->openStreamElement( rPictureStreamName, nMode );
        if( aRet.xStream.is() && ( SvXMLGraphicHelperMode::Write == meCreateMode ) )
        {
            OUString aPropName( "UseCommonStoragePasswordEncryption" );
            uno::Reference < beans::XPropertySet > xProps( aRet.xStream, uno::UNO_QUERY );
            xProps->setPropertyValue( aPropName, uno::makeAny( true) );
        }
    }

    return aRet;
}

OUString SvXMLGraphicHelper::ImplGetGraphicMimeType( const OUString& rFileName )
{
    struct XMLGraphicMimeTypeMapper
    {
        const char* pExt;
        const char* pMimeType;
    };

    static const XMLGraphicMimeTypeMapper aMapper[] =
    {
        { "gif", "image/gif" },
        { "png", "image/png" },
        { "jpg", "image/jpeg" },
        { "tif", "image/tiff" },
        { "svg", "image/svg+xml" },
        { "pdf", "application/pdf" },
        { "wmf", "image/x-wmf" },
        { "eps", "image/x-eps" },
        { "bmp", "image/bmp" },
        { "pct", "image/x-pict" }
    };

    OUString aMimeType;

    if( ( rFileName.getLength() >= 4 ) && ( rFileName[ rFileName.getLength() - 4 ] == '.' ) )
    {
        const OString aExt(OUStringToOString(rFileName.copy(rFileName.getLength() - 3),
            RTL_TEXTENCODING_ASCII_US));

        for( long i = 0, nCount = SAL_N_ELEMENTS(aMapper); ( i < nCount ) && aMimeType.isEmpty(); ++i )
            if( strcmp(aExt.getStr(), aMapper[ i ].pExt) == 0 )
                aMimeType = OUString( aMapper[ i ].pMimeType, strlen( aMapper[ i ].pMimeType ), RTL_TEXTENCODING_ASCII_US );
    }

    return aMimeType;
}

Graphic SvXMLGraphicHelper::ImplReadGraphic( const OUString& rPictureStorageName,
                                             const OUString& rPictureStreamName )
{
    Graphic             aGraphic;
    SvxGraphicHelperStream_Impl aStream( ImplGetGraphicStream( rPictureStorageName, rPictureStreamName ) );
    if( aStream.xStream.is() )
    {
        std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream( aStream.xStream ));
        GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, "", *pStream );
    }

    return aGraphic;
}

bool SvXMLGraphicHelper::ImplWriteGraphic( const OUString& rPictureStorageName,
                                               const OUString& rPictureStreamName,
                                               const OUString& rGraphicId,
                                               bool bUseGfxLink )
{
    GraphicObject   aGrfObject( OUStringToOString(rGraphicId, RTL_TEXTENCODING_ASCII_US) );
    bool        bRet = false;

    if( aGrfObject.GetType() != GraphicType::NONE )
    {
        SvxGraphicHelperStream_Impl aStream( ImplGetGraphicStream( rPictureStorageName, rPictureStreamName ) );
        if( aStream.xStream.is() )
        {
            Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
            const GfxLink   aGfxLink( aGraphic.GetLink() );
            const OUString  aMimeType( ImplGetGraphicMimeType( rPictureStreamName ) );
            uno::Reference < beans::XPropertySet > xProps( aStream.xStream, uno::UNO_QUERY );

            // set stream properties (MediaType/Compression)
            if( !aMimeType.isEmpty() )
            {
                xProps->setPropertyValue( "MediaType", Any(aMimeType) );
            }

            // picture formats that actuall _do_ benefit from zip
            // storage compression
            // .svm pics gets compressed via ZBITMAP old-style stream
            // option below
            static const char* aCompressiblePics[] =
            {
                "image/svg+xml",
                "image/x-wmf",
                "image/tiff",
                "image/x-eps",
                "image/bmp",
                "image/x-pict"
            };

            bool bCompressed = aMimeType.isEmpty();
            if( !bCompressed )
            {
                for(const char* p : aCompressiblePics)
                {
                    if( aMimeType.equalsIgnoreAsciiCaseAscii(p) )
                    {
                        bCompressed = true;
                        break;
                    }
                }
            }

            xProps->setPropertyValue( "Compressed", Any(bCompressed) );

            std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream( aStream.xStream ));
            if( bUseGfxLink && aGfxLink.GetDataSize() && aGfxLink.GetData() )
                pStream->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
            else
            {
                if( aGraphic.GetType() == GraphicType::Bitmap )
                {
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    OUString          aFormat;

                    if( aGraphic.IsAnimated() )
                        aFormat = "gif";
                    else
                        aFormat = "png";

                    bRet = ( rFilter.ExportGraphic( aGraphic, "", *pStream,
                                                     rFilter.GetExportFormatNumberForShortName( aFormat ) ) == 0 );
                }
                else if( aGraphic.GetType() == GraphicType::GdiMetafile )
                {
                    pStream->SetVersion( SOFFICE_FILEFORMAT_8 );
                    pStream->SetCompressMode( SvStreamCompressFlags::ZBITMAP );

                    // SJ: first check if this metafile is just a eps file, then we will store the eps instead of svm
                    GDIMetaFile& rMtf(const_cast<GDIMetaFile&>( aGraphic.GetGDIMetaFile() ));
                    const MetaCommentAction* pComment = ImplCheckForEPS( rMtf );
                    if ( pComment )
                    {
                        sal_uInt32  nSize = pComment->GetDataSize();
                        const sal_uInt8* pData = pComment->GetData();
                        if ( nSize && pData )
                            pStream->WriteBytes(pData, nSize);

                        const MetaEPSAction* pAct = static_cast<const MetaEPSAction*>(rMtf.FirstAction());
                        const GfxLink&       rLink = pAct->GetLink();

                        pStream->WriteBytes(rLink.GetData(), rLink.GetDataSize());
                    }
                    else
                        rMtf.Write( *pStream );

                    bRet = ( pStream->GetError() == 0 );
                }
            }
            uno::Reference < embed::XTransactedObject > xStorage(
                                    aStream.xStorage, uno::UNO_QUERY);
            pStream.reset();
            aStream.xStream->getOutputStream()->closeOutput();
            if( xStorage.is() )
                xStorage->commit();
        }
    }

    return bRet;
}

void SvXMLGraphicHelper::ImplInsertGraphicURL( const OUString& rURLStr, sal_uInt32 nInsertPos, OUString& rRequestedFileName )
{
    OUString aPictureStorageName, aPictureStreamName;
    if( ( maURLSet.find( rURLStr ) != maURLSet.end() ) )
    {
        for (URLPairVector::const_iterator aIter( maGrfURLs.begin() ), aEnd( maGrfURLs.end() ); aIter != aEnd ; ++aIter)
        {
            if( rURLStr == (*aIter).first )
            {
                maGrfURLs[ nInsertPos ].second = (*aIter).second;
                break;
            }
        }
    }
    else if( ImplGetStreamNames( rURLStr, aPictureStorageName, aPictureStreamName ) )
    {
        URLPair& rURLPair = maGrfURLs[ nInsertPos ];

        if( SvXMLGraphicHelperMode::Read == meCreateMode )
        {
            const GraphicObject aObj( ImplReadGraphic( aPictureStorageName, aPictureStreamName ) );

            if( aObj.GetType() != GraphicType::NONE )
            {
                maGrfObjs.push_back( aObj );
                OUString aBaseURL(  XML_GRAPHICOBJECT_URL_BASE  );

                rURLPair.second = aBaseURL;
                rURLPair.second += OStringToOUString(aObj.GetUniqueID(),
                    RTL_TEXTENCODING_ASCII_US);
            }
            else
                rURLPair.second.clear();
        }
        else
        {
            const OString aAsciiObjectID(OUStringToOString(aPictureStreamName, RTL_TEXTENCODING_ASCII_US));
            const GraphicObject aGrfObject( aAsciiObjectID );
            if( aGrfObject.GetType() != GraphicType::NONE )
            {
                OUString        aStreamName( aPictureStreamName );
                Graphic         aGraphic( (Graphic&) aGrfObject.GetGraphic() );
                const GfxLink   aGfxLink( aGraphic.GetLink() );
                OUString        aExtension;
                bool            bUseGfxLink( true );

                if( aGfxLink.GetDataSize() )
                {
                    switch( aGfxLink.GetType() )
                    {
                        case GfxLinkType::EpsBuffer: aExtension = ".eps"; break;
                        case GfxLinkType::NativeGif: aExtension = ".gif"; break;
                        // #i15508# added BMP type for better exports (checked, works)
                        case GfxLinkType::NativeBmp: aExtension = ".bmp"; break;
                        case GfxLinkType::NativeJpg: aExtension = ".jpg"; break;
                        case GfxLinkType::NativePng: aExtension = ".png"; break;
                        case GfxLinkType::NativeTif: aExtension = ".tif"; break;
                        case GfxLinkType::NativeWmf: aExtension = ".wmf"; break;
                        case GfxLinkType::NativeMet: aExtension = ".met"; break;
                        case GfxLinkType::NativePct: aExtension = ".pct"; break;
                        case GfxLinkType::NativeSvg:
                            // backward-compat kludge: since no released OOo
                            // version to date can handle svg properly, wrap it up
                            // into an svm. slight catch22 here, since strict ODF
                            // conformance _recommends_ svg - then again, most old
                            // ODF consumers are believed to be OOo
                            if( SvtSaveOptions().GetODFDefaultVersion() <= SvtSaveOptions::ODFVER_012 )
                            {
                                bUseGfxLink = false;
                                aExtension = ".svm";
                            }
                            else
                                aExtension = ".svg";
                            break;
                        case GfxLinkType::NativePdf: aExtension = ".pdf"; break;

                        default:
                            aExtension = ".grf";
                        break;
                    }
                }
                else
                {
                    if( aGrfObject.GetType() == GraphicType::Bitmap )
                    {
                        if( aGrfObject.IsAnimated() )
                            aExtension = ".gif";
                        else
                            aExtension = ".png";
                    }
                    else if( aGrfObject.GetType() == GraphicType::GdiMetafile )
                    {
                        // SJ: first check if this metafile is just a eps file, then we will store the eps instead of svm
                        GDIMetaFile& rMtf(const_cast<GDIMetaFile&>( aGraphic.GetGDIMetaFile() ));
                        if ( ImplCheckForEPS( rMtf ) )
                            aExtension = ".eps";
                        else
                            aExtension = ".svm";
                    }
                }

                OUString aURLEntry;
                const OUString sPictures(  "Pictures/"  );

                if ( !rRequestedFileName.isEmpty() )
                {
                    aURLEntry = sPictures;
                    aURLEntry += rRequestedFileName;
                    aURLEntry += aExtension;

                    URLPairVector::const_iterator aIter( maGrfURLs.begin() ), aEnd( maGrfURLs.end() );
                    for ( ; aIter != aEnd; ++aIter )
                    {
                        if( aURLEntry == (*aIter).second )
                            break;
                    }
                    if ( aIter == aEnd )
                        aStreamName = rRequestedFileName;
                }

                aStreamName += aExtension;

                if( mbDirect && !aStreamName.isEmpty() )
                    ImplWriteGraphic( aPictureStorageName, aStreamName, aPictureStreamName, bUseGfxLink );

                rURLPair.second = sPictures;
                rURLPair.second += aStreamName;
            }
#if OSL_DEBUG_LEVEL > 0
            else
            {
                OStringBuffer sMessage("graphic object with ID '");
                sMessage.append(aAsciiObjectID).
                    append("' has an unknown type");
                OSL_ENSURE( false, sMessage.getStr() );
            }
#endif
        }

        maURLSet.insert( rURLStr );
    }
}

void SvXMLGraphicHelper::Init( const uno::Reference < embed::XStorage >& rXMLStorage,
                               SvXMLGraphicHelperMode eCreateMode,
                               bool bDirect )
{
    mxRootStorage = rXMLStorage;
    meCreateMode = eCreateMode;
    mbDirect = meCreateMode != SvXMLGraphicHelperMode::Read || bDirect;
}

SvXMLGraphicHelper* SvXMLGraphicHelper::Create( const uno::Reference < embed::XStorage >& rXMLStorage,
                                                SvXMLGraphicHelperMode eCreateMode,
                                                bool bDirect )
{
    SvXMLGraphicHelper* pThis = new SvXMLGraphicHelper;

    pThis->acquire();
    pThis->Init( rXMLStorage, eCreateMode, bDirect );

    return pThis;
}

SvXMLGraphicHelper* SvXMLGraphicHelper::Create( SvXMLGraphicHelperMode eCreateMode )
{
    SvXMLGraphicHelper* pThis = new SvXMLGraphicHelper;

    pThis->acquire();
    pThis->Init( nullptr, eCreateMode, false );

    return pThis;
}

void SvXMLGraphicHelper::Destroy( SvXMLGraphicHelper* pSvXMLGraphicHelper )
{
    if( pSvXMLGraphicHelper )
    {
        pSvXMLGraphicHelper->dispose();
        pSvXMLGraphicHelper->release();
    }
}

// XGraphicObjectResolver
OUString SAL_CALL SvXMLGraphicHelper::resolveGraphicObjectURL( const OUString& rURL )
{
    ::osl::MutexGuard   aGuard( maMutex );
    const sal_Int32     nIndex = maGrfURLs.size();

    OUString aURL( rURL );
    OUString aUserData;
    OUString aRequestedFileName;

    sal_Int32 nUser = rURL.indexOf( '?' );
    if ( nUser >= 0 )
    {
        aURL = rURL.copy( 0, nUser );
        nUser++;
        aUserData = rURL.copy( nUser, rURL.getLength() - nUser );
    }
    if ( !aUserData.isEmpty() )
    {
        sal_Int32 nIndex2 = 0;
        do
        {
            OUString aToken = aUserData.getToken( 0, ';', nIndex2 );
            sal_Int32 n = aToken.indexOf( '=' );
            if ( ( n > 0 ) && ( ( n + 1 ) < aToken.getLength() ) )
            {
                OUString aParam( aToken.copy( 0, n ) );
                OUString aValue( aToken.copy( n + 1, aToken.getLength() - ( n + 1 ) ) );

                const OUString sRequestedName( "requestedName" );
                if ( aParam.match( sRequestedName ) )
                    aRequestedFileName = aValue;
            }
        }
        while ( nIndex2 >= 0 );
    }

    maGrfURLs.push_back( ::std::make_pair( aURL, OUString() ) );
    ImplInsertGraphicURL( aURL, nIndex, aRequestedFileName );

    return maGrfURLs[ nIndex ].second;
}

// XBinaryStreamResolver
Reference< XInputStream > SAL_CALL SvXMLGraphicHelper::getInputStream( const OUString& rURL )
{
    Reference< XInputStream >   xRet;
    OUString                    aPictureStorageName, aGraphicId;


    if( ( SvXMLGraphicHelperMode::Write == meCreateMode ) &&
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

Reference< XOutputStream > SAL_CALL SvXMLGraphicHelper::createOutputStream()
{
    Reference< XOutputStream > xRet;

    if( SvXMLGraphicHelperMode::Read == meCreateMode )
    {
        SvXMLGraphicOutputStream* pOutputStream = new SvXMLGraphicOutputStream;

        if( pOutputStream->Exists() )
            maGrfStms.push_back( xRet = pOutputStream );
        else
            delete pOutputStream;
    }

    return xRet;
}

OUString SAL_CALL SvXMLGraphicHelper::resolveOutputStream( const Reference< XOutputStream >& rxBinaryStream )
{
    OUString aRet;

    if( ( SvXMLGraphicHelperMode::Read == meCreateMode ) && rxBinaryStream.is() )
    {
        if( ::std::find( maGrfStms.begin(), maGrfStms.end(), rxBinaryStream ) != maGrfStms.end() )
        {
            SvXMLGraphicOutputStream* pOStm = static_cast< SvXMLGraphicOutputStream* >( rxBinaryStream.get() );

            if( pOStm )
            {
                const GraphicObject& rGrfObj = pOStm->GetGraphicObject();
                const OUString aId(OStringToOUString(
                    rGrfObj.GetUniqueID(), RTL_TEXTENCODING_ASCII_US));

                if( !aId.isEmpty() )
                {
                    aRet = XML_GRAPHICOBJECT_URL_BASE;
                    aRet += aId;
                }
            }
        }
    }

    return aRet;
}

// for instantiation via service manager
namespace {

namespace impl
{
typedef ::cppu::WeakComponentImplHelper<
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
    explicit SvXMLGraphicImportExportHelper( SvXMLGraphicHelperMode eMode );

protected:
    // is called from WeakComponentImplHelper when XComponent::dispose() was
    // called from outside
    virtual void SAL_CALL disposing() override;

    // ____ XInitialization ____
    // one argument is allowed, which is the XStorage
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // ____ XGraphicObjectResolver ____
    virtual OUString SAL_CALL resolveGraphicObjectURL( const OUString& aURL ) override;

    // ____ XBinaryStreamResolver ____
    virtual Reference< io::XInputStream > SAL_CALL getInputStream( const OUString& aURL ) override;
    virtual Reference< io::XOutputStream > SAL_CALL createOutputStream() override;
    virtual OUString SAL_CALL resolveOutputStream( const Reference< io::XOutputStream >& aBinaryStream ) override;

    // ____ XServiceInfo ____
    virtual OUString SAL_CALL getImplementationName() override;
    virtual sal_Bool SAL_CALL supportsService( const OUString& ServiceName ) override;
    virtual Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

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
OUString SAL_CALL SvXMLGraphicImportExportHelper::resolveGraphicObjectURL( const OUString& aURL )
{
    return m_xGraphicObjectResolver->resolveGraphicObjectURL( aURL );
}


// ____ XBinaryStreamResolver ____
Reference< io::XInputStream > SAL_CALL SvXMLGraphicImportExportHelper::getInputStream( const OUString& aURL )
{
    return m_xBinaryStreamResolver->getInputStream( aURL );
}
Reference< io::XOutputStream > SAL_CALL SvXMLGraphicImportExportHelper::createOutputStream()
{
    return m_xBinaryStreamResolver->createOutputStream();
}
OUString SAL_CALL SvXMLGraphicImportExportHelper::resolveOutputStream( const Reference< io::XOutputStream >& aBinaryStream )
{
    return m_xBinaryStreamResolver->resolveOutputStream( aBinaryStream );
}

// ____ XServiceInfo ____
OUString SAL_CALL SvXMLGraphicImportExportHelper::getImplementationName()
{
    if( m_eGraphicHelperMode == SvXMLGraphicHelperMode::Read )
        return OUString("com.sun.star.comp.Svx.GraphicImportHelper");
    return OUString("com.sun.star.comp.Svx.GraphicExportHelper");
}

sal_Bool SAL_CALL SvXMLGraphicImportExportHelper::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SvXMLGraphicImportExportHelper::getSupportedServiceNames()
{
    // XGraphicObjectResolver and XBinaryStreamResolver are not part of any service
    Sequence< OUString > aSupportedServiceNames( 2 );
    aSupportedServiceNames[0] = "com.sun.star.document.GraphicObjectResolver";
    aSupportedServiceNames[1] = "com.sun.star.document.BinaryStreamResolver";
    return aSupportedServiceNames;
}

}

/** Create this with createInstanceWithArguments. service name
    "com.sun.star.comp.Svx.GraphicImportHelper", one argument which is the
    XStorage.  Without arguments no helper class is created.  With an empty
    argument the helper class is created and initialized like in the CTOR to
    SvXMLGraphicHelper that only gets the create mode.

    You should call dispose after you no longer need this component.

    uses eCreateMode == SvXMLGraphicHelperMode::Read, bDirect == sal_True in
    SvXMLGraphicHelper
 */
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_Svx_GraphicImportHelper_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvXMLGraphicImportExportHelper(SvXMLGraphicHelperMode::Read));
}

/** Create this with createInstanceWithArguments. service name
    "com.sun.star.comp.Svx.GraphicExportHelper", one argument which is the
    XStorage.  Without arguments no helper class is created.  With an empty
    argument the helper class is created and initialized like in the CTOR to
    SvXMLGraphicHelper that only gets the create mode

    To write the Pictures stream, you have to call dispose at this component.
    Make sure you call dispose before you commit the parent storage.

    uses eCreateMode == SvXMLGraphicHelperMode::Write, bDirect == sal_True in
    SvXMLGraphicHelper
 */
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface * SAL_CALL
com_sun_star_comp_Svx_GraphicExportHelper_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvXMLGraphicImportExportHelper(SvXMLGraphicHelperMode::Write));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
