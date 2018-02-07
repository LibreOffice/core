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
#include <comphelper/fileformat.h>
#include <comphelper/graphicmimetype.hxx>
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
#include <svx/xmlgrhlp.hxx>
#include <svx/xmleohlp.hxx>

#include <algorithm>
#include <memory>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;


#define XML_GRAPHICSTORAGE_NAME     "Pictures"
#define XML_GRAPHICOBJECT_URL_BASE  "vnd.sun.star.GraphicObject:"

namespace {

const MetaCommentAction* ImplCheckForEPS( GDIMetaFile const & rMtf )
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

    explicit                        SvXMLGraphicInputStream( const OUString& rGraphicId, const OUString& rMimeType );
                                    SvXMLGraphicInputStream(const SvXMLGraphicInputStream&) = delete;
    SvXMLGraphicInputStream&        operator=(const SvXMLGraphicInputStream&) = delete;

    bool                            Exists() const { return mxStmWrapper.is(); }
};


SvXMLGraphicInputStream::SvXMLGraphicInputStream( const OUString& rGraphicId, const OUString& rMimeType )
{
    GraphicObject   aGrfObject( OUStringToOString(rGraphicId, RTL_TEXTENCODING_ASCII_US) );

    maTmp.EnableKillingFile();

    if( aGrfObject.GetType() != GraphicType::NONE )
    {
        SvStream* pStm = ::utl::UcbStreamHelper::CreateStream( maTmp.GetURL(), StreamMode::WRITE | StreamMode::TRUNC );

        if( pStm )
        {
            Graphic         aGraphic( aGrfObject.GetGraphic() );
            const GfxLink   aGfxLink( aGraphic.GetLink() );
            bool        bRet = false;

            if( aGfxLink.GetDataSize() && aGfxLink.GetData() )
            {
                if ( rMimeType.isEmpty() )
                {
                    pStm->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                    bRet = ( pStm->GetError() == ERRCODE_NONE );
                }
                else
                {
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    bRet = ( rFilter.ExportGraphic( aGraphic, "", *pStm, rFilter.GetExportFormatNumberForMediaType( rMimeType ) ) == ERRCODE_NONE );
                }
            }
            else
            {
                if( aGraphic.GetType() == GraphicType::Bitmap )
                {
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    OUString          aFormat=rMimeType;

                    if( aGraphic.IsAnimated() )
                        aFormat = "image/gif";
                    else if( aFormat.isEmpty() )
                        aFormat = "image/png";

                    bRet = ( rFilter.ExportGraphic( aGraphic, "", *pStm, rFilter.GetExportFormatNumberForMediaType( aFormat ) ) == ERRCODE_NONE );
                }
                else if( rMimeType.isEmpty() && aGraphic.GetType() == GraphicType::GdiMetafile )
                {
                    pStm->SetVersion( SOFFICE_FILEFORMAT_8 );
                    pStm->SetCompressMode( SvStreamCompressFlags::ZBITMAP );
                    const_cast<GDIMetaFile&>( aGraphic.GetGDIMetaFile() ).Write( *pStm );
                    bRet = ( pStm->GetError() == ERRCODE_NONE );
                }
                else if( !rMimeType.isEmpty() )
                {
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    bRet = ( rFilter.ExportGraphic( aGraphic, "", *pStm, rFilter.GetExportFormatNumberForMediaType( rMimeType ) ) == ERRCODE_NONE );
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

namespace xmloff {

class GraphicInputStream : public cppu::WeakImplHelper<XInputStream>
{
private:
    virtual sal_Int32 SAL_CALL readBytes(Sequence<sal_Int8> & aData, sal_Int32 nBytesToRead) override;
    virtual sal_Int32 SAL_CALL readSomeBytes(Sequence<sal_Int8> & aData, sal_Int32 nMaxBytesToRead) override;
    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip) override;
    virtual sal_Int32 SAL_CALL available() override;
    virtual void SAL_CALL closeInput() override;

private:
    utl::TempFile maTempFile;
    Reference<XInputStream> mxStreamWrapper;

public:

    explicit GraphicInputStream(GraphicObject const & raGraphicObject, const OUString & rMimeType);
    GraphicInputStream(const GraphicInputStream&) = delete;

    GraphicInputStream& operator=(const GraphicInputStream&) = delete;

    bool exists() const
    {
        return mxStreamWrapper.is();
    }
};


GraphicInputStream::GraphicInputStream(GraphicObject const & raGraphicObject, const OUString & rMimeType)
{
    GraphicObject aGraphicObject(raGraphicObject);

    maTempFile.EnableKillingFile();

    if (aGraphicObject.GetType() != GraphicType::NONE)
    {
        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream(maTempFile.GetURL(), StreamMode::WRITE | StreamMode::TRUNC);

        if (pStream)
        {
            Graphic aGraphic(aGraphicObject.GetGraphic());
            const GfxLink aGfxLink(aGraphic.GetLink());
            bool bRet = false;

            if (aGfxLink.GetDataSize() && aGfxLink.GetData())
            {
                if (rMimeType.isEmpty())
                {
                    pStream->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                    bRet = (pStream->GetError() == ERRCODE_NONE);
                }
                else
                {
                    GraphicFilter &rFilter = GraphicFilter::GetGraphicFilter();
                    bRet = (rFilter.ExportGraphic(aGraphic, "", *pStream, rFilter.GetExportFormatNumberForMediaType(rMimeType)) == ERRCODE_NONE);
                }
            }
            else
            {
                if (aGraphic.GetType() == GraphicType::Bitmap)
                {
                    GraphicFilter & rFilter = GraphicFilter::GetGraphicFilter();
                    OUString aFormat = rMimeType;

                    if (aGraphic.IsAnimated())
                        aFormat = "image/gif";
                    else if (aFormat.isEmpty())
                        aFormat = "image/png";

                    bRet = (rFilter.ExportGraphic(aGraphic, "", *pStream, rFilter.GetExportFormatNumberForMediaType(aFormat)) == ERRCODE_NONE);
                }
                else if (rMimeType.isEmpty() && aGraphic.GetType() == GraphicType::GdiMetafile)
                {
                    pStream->SetVersion(SOFFICE_FILEFORMAT_8);
                    pStream->SetCompressMode(SvStreamCompressFlags::ZBITMAP);
                    const_cast<GDIMetaFile&>(aGraphic.GetGDIMetaFile()).Write(*pStream);
                    bRet = (pStream->GetError() == ERRCODE_NONE);
                }
                else if (!rMimeType.isEmpty())
                {
                    GraphicFilter & rFilter = GraphicFilter::GetGraphicFilter();
                    bRet = ( rFilter.ExportGraphic( aGraphic, "", *pStream, rFilter.GetExportFormatNumberForMediaType( rMimeType ) ) == ERRCODE_NONE );
                }
            }

            if (bRet)
            {
                pStream->Seek( 0 );
                mxStreamWrapper = new ::utl::OInputStreamWrapper(pStream, true);
            }
            else
                delete pStream;
        }
    }
}

sal_Int32 SAL_CALL GraphicInputStream::readBytes(Sequence<sal_Int8> & rData, sal_Int32 nBytesToRead)
{
    if (!mxStreamWrapper.is())
        throw NotConnectedException();

    return mxStreamWrapper->readBytes(rData, nBytesToRead);
}

sal_Int32 SAL_CALL GraphicInputStream::readSomeBytes(Sequence<sal_Int8>& rData, sal_Int32 nMaxBytesToRead )
{
    if (!mxStreamWrapper.is())
        throw NotConnectedException() ;

    return mxStreamWrapper->readSomeBytes(rData, nMaxBytesToRead);
}

void SAL_CALL GraphicInputStream::skipBytes(sal_Int32 nBytesToSkip)
{
    if (!mxStreamWrapper.is())
        throw NotConnectedException();

    mxStreamWrapper->skipBytes(nBytesToSkip);
}

sal_Int32 SAL_CALL GraphicInputStream::available()
{
    if (!mxStreamWrapper.is())
        throw NotConnectedException();

    return mxStreamWrapper->available();
}

void SAL_CALL GraphicInputStream::closeInput()
{
    if (!mxStreamWrapper.is())
        throw NotConnectedException();

    mxStreamWrapper->closeInput();
}

} // end xmloff namespace

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
                            pDest->Seek(0);
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

SvXMLGraphicHelper::SvXMLGraphicHelper(SvXMLGraphicHelperMode eCreateMode)
    : cppu::WeakComponentImplHelper<document::XGraphicObjectResolver,
                                    document::XGraphicStorageHandler,
                                    document::XBinaryStreamResolver>(maMutex)
{
    Init( nullptr, eCreateMode, false );
}

SvXMLGraphicHelper::SvXMLGraphicHelper()
    : cppu::WeakComponentImplHelper<document::XGraphicObjectResolver,
                                    document::XGraphicStorageHandler,
                                    document::XBinaryStreamResolver>(maMutex)
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
            uno::Reference < beans::XPropertySet > xProps( aRet.xStream, uno::UNO_QUERY );
            xProps->setPropertyValue( "UseCommonStoragePasswordEncryption", uno::makeAny( true) );
        }
    }

    return aRet;
}

OUString SvXMLGraphicHelper::ImplGetGraphicMimeType( const OUString& rFileName )
{
    if( ( rFileName.getLength() >= 4 ) && ( rFileName[ rFileName.getLength() - 4 ] == '.' ) )
    {
        const OString aExt(OUStringToOString(rFileName.copy(rFileName.getLength() - 3),
            RTL_TEXTENCODING_ASCII_US));
        return comphelper::GraphicMimeTypeHelper::GetMimeTypeForExtension( aExt );
    }

    return OUString();
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
            Graphic         aGraphic( aGrfObject.GetGraphic() );
            const GfxLink   aGfxLink( aGraphic.GetLink() );
            const OUString  aMimeType( ImplGetGraphicMimeType( rPictureStreamName ) );
            uno::Reference < beans::XPropertySet > xProps( aStream.xStream, uno::UNO_QUERY );

            // set stream properties (MediaType/Compression)
            if( !aMimeType.isEmpty() )
            {
                xProps->setPropertyValue( "MediaType", Any(aMimeType) );
            }

            // picture formats that actually _do_ benefit from zip
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
            {
                const uno::Sequence<sal_Int8>& rPdfData = aGraphic.getPdfData();
                if (rPdfData.hasElements())
                {
                    // The graphic has PDF data attached to it, use that.
                    // vcl::ImportPDF() possibly downgraded the PDF data from a
                    // higher PDF version, while aGfxLink still contains the
                    // original data provided by the user.
                    pStream->WriteBytes(rPdfData.getConstArray(), rPdfData.getLength());
                }
                else
                {
                    pStream->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                }
                bRet = (pStream->GetError() == ERRCODE_NONE);
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

                    bRet = ( rFilter.ExportGraphic( aGraphic, "", *pStream,
                                                     rFilter.GetExportFormatNumberForShortName( aFormat ) ) == ERRCODE_NONE );
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

                    bRet = ( pStream->GetError() == ERRCODE_NONE );
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

void SvXMLGraphicHelper::ImplInsertGraphicURL( const OUString& rURLStr, sal_uInt32 nInsertPos, OUString const & rRequestedFileName )
{
    OUString aPictureStorageName, aPictureStreamName;
    if( maURLSet.find( rURLStr ) != maURLSet.end() )
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

                rURLPair.second = XML_GRAPHICOBJECT_URL_BASE;
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
                Graphic         aGraphic( aGrfObject.GetGraphic() );
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
                               bool bDirect,
                               const OUString& rGraphicMimeType )
{
    mxRootStorage = rXMLStorage;
    meCreateMode = eCreateMode;
    maOutputMimeType = rGraphicMimeType;
    mbDirect = meCreateMode != SvXMLGraphicHelperMode::Read || bDirect;
}

rtl::Reference<SvXMLGraphicHelper> SvXMLGraphicHelper::Create( const uno::Reference < embed::XStorage >& rXMLStorage,
                                                SvXMLGraphicHelperMode eCreateMode,
                                                bool bDirect,
                                                const OUString& rGraphicMimeType )
{
    rtl::Reference<SvXMLGraphicHelper> pThis = new SvXMLGraphicHelper;
    pThis->Init( rXMLStorage, eCreateMode, bDirect, rGraphicMimeType );

    return pThis;
}

rtl::Reference<SvXMLGraphicHelper> SvXMLGraphicHelper::Create( SvXMLGraphicHelperMode eCreateMode,
                                              const OUString& rGraphicMimeType )
{
    rtl::Reference<SvXMLGraphicHelper> pThis = new SvXMLGraphicHelper;

    pThis->Init( nullptr, eCreateMode, false, rGraphicMimeType );

    return pThis;
}

namespace
{

void splitUserDataFromURL(OUString const & rWholeURL, OUString & rJustURL, OUString & rUserData)
{
    sal_Int32 nUser = rWholeURL.indexOf('?');
    if (nUser >= 0)
    {
        rJustURL = rWholeURL.copy(0, nUser);
        nUser++;
        rUserData = rWholeURL.copy(nUser);
    }
    else
    {
        rJustURL = rWholeURL;
    }
}

} // end anonymous namespace

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
        aUserData = rURL.copy( nUser );
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
                OUString aValue( aToken.copy( n + 1 ) );

                const OUString sRequestedName( "requestedName" );
                if ( aParam.match( sRequestedName ) )
                    aRequestedFileName = aValue;
            }
        }
        while ( nIndex2 >= 0 );
    }

    maGrfURLs.emplace_back( aURL, OUString() );
    ImplInsertGraphicURL( aURL, nIndex, aRequestedFileName );

    return maGrfURLs[ nIndex ].second;
}

// XGraphicStorageHandler
uno::Reference<graphic::XGraphic> SAL_CALL SvXMLGraphicHelper::loadGraphic(OUString const & rURL)
{
    osl::MutexGuard aGuard(maMutex);

    uno::Reference<graphic::XGraphic> xGraphic;

    OUString aURLOnly;
    OUString aUserData;
    splitUserDataFromURL(rURL, aURLOnly, aUserData);

    auto aIterator = maGraphicObjects.find(aURLOnly);
    if (aIterator != maGraphicObjects.end())
    {
        return aIterator->second;
    }

    OUString aPictureStorageName, aPictureStreamName;

    if (ImplGetStreamNames(aURLOnly, aPictureStorageName, aPictureStreamName))
    {
        const GraphicObject aGraphicObject(ImplReadGraphic(aPictureStorageName, aPictureStreamName));

        if (aGraphicObject.GetType() != GraphicType::NONE)
        {
            xGraphic = aGraphicObject.GetGraphic().GetXGraphic();
            maGraphicObjects[aURLOnly] = xGraphic;
        }
    }

    return xGraphic;
}

OUString SAL_CALL SvXMLGraphicHelper::saveGraphicByName(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic, OUString const & rRequestName)
{
    return implSaveGraphic(rxGraphic, rRequestName);
}

OUString SAL_CALL SvXMLGraphicHelper::saveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic)
{
    OUString aEmpty;
    return implSaveGraphic(rxGraphic, aEmpty);
}

OUString SvXMLGraphicHelper::implSaveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic, OUString const & rRequestName)
{
    Graphic aGraphic(rxGraphic);

    auto aIterator = maExportGraphics.find(aGraphic);
    if (aIterator != maExportGraphics.end())
    {
        return aIterator->second;
    }

    GraphicObject aGraphicObject(aGraphic);

    if (aGraphicObject.GetType() != GraphicType::NONE)
    {
        const GfxLink aGfxLink(aGraphic.GetLink());
        OUString aExtension;
        bool bUseGfxLink = true;

        if (aGfxLink.GetDataSize())
        {
            switch (aGfxLink.GetType())
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
                    if (SvtSaveOptions().GetODFDefaultVersion() <= SvtSaveOptions::ODFVER_012)
                    {
                        bUseGfxLink = false;
                        aExtension = ".svm";
                    }
                    else
                    {
                        aExtension = ".svg";
                    }
                    break;
                case GfxLinkType::NativePdf: aExtension = ".pdf"; break;

                default:
                    aExtension = ".grf";
                break;
            }
        }
        else
        {
            if (aGraphicObject.GetType() == GraphicType::Bitmap)
            {
                if (aGraphicObject.IsAnimated())
                    aExtension = ".gif";
                else
                    aExtension = ".png";
            }
            else if (aGraphicObject.GetType() == GraphicType::GdiMetafile)
            {
                // SJ: first check if this metafile is just a eps file, then we will store the eps instead of svm
                GDIMetaFile& rMetafile(const_cast<GDIMetaFile&>(aGraphic.GetGDIMetaFile()));

                if (ImplCheckForEPS(rMetafile))
                    aExtension = ".eps";
                else
                    aExtension = ".svm";
            }
        }

        OUString rPictureStreamName;
        if (!rRequestName.isEmpty())
        {
            rPictureStreamName = rRequestName + aExtension;
        }
        else
        {
            OUString sId = OStringToOUString(aGraphicObject.GetUniqueID(), RTL_TEXTENCODING_ASCII_US);
            rPictureStreamName = sId + aExtension;
        }

        SvxGraphicHelperStream_Impl aStream(ImplGetGraphicStream(XML_GRAPHICSTORAGE_NAME, rPictureStreamName));

        if (aStream.xStream.is())
        {
            const OUString aMimeType(ImplGetGraphicMimeType(rPictureStreamName));
            uno::Reference<beans::XPropertySet> xProps(aStream.xStream, uno::UNO_QUERY);

            // set stream properties (MediaType/Compression)
            if (!aMimeType.isEmpty())
            {
                xProps->setPropertyValue("MediaType", uno::Any(aMimeType));
            }

            // picture formats that actually _do_ benefit from zip
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

            bool bSuccess = false;

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

            xProps->setPropertyValue("Compressed", Any(bCompressed));

            std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(aStream.xStream));
            if (bUseGfxLink && aGfxLink.GetDataSize() && aGfxLink.GetData())
            {
                const uno::Sequence<sal_Int8>& rPdfData = aGraphic.getPdfData();
                if (rPdfData.hasElements())
                {
                    // The graphic has PDF data attached to it, use that.
                    // vcl::ImportPDF() possibly downgraded the PDF data from a
                    // higher PDF version, while aGfxLink still contains the
                    // original data provided by the user.
                    pStream->WriteBytes(rPdfData.getConstArray(), rPdfData.getLength());
                }
                else
                {
                    pStream->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                }
                bSuccess = (pStream->GetError() == ERRCODE_NONE);
            }
            else
            {
                if (aGraphic.GetType() == GraphicType::Bitmap)
                {
                    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
                    OUString aFormat;

                    if (aGraphic.IsAnimated())
                        aFormat = "gif";
                    else
                        aFormat = "png";

                    bSuccess = (rFilter.ExportGraphic(aGraphic, "", *pStream, rFilter.GetExportFormatNumberForShortName(aFormat)) == ERRCODE_NONE);
                }
                else if (aGraphic.GetType() == GraphicType::GdiMetafile)
                {
                    pStream->SetVersion(SOFFICE_FILEFORMAT_8);
                    pStream->SetCompressMode(SvStreamCompressFlags::ZBITMAP);

                    // SJ: first check if this metafile is just a eps file, then we will store the eps instead of svm
                    GDIMetaFile& rMtf(const_cast<GDIMetaFile&>(aGraphic.GetGDIMetaFile()));
                    const MetaCommentAction* pComment = ImplCheckForEPS(rMtf);
                    if (pComment)
                    {
                        sal_uInt32 nSize = pComment->GetDataSize();
                        const sal_uInt8* pData = pComment->GetData();
                        if (nSize && pData)
                            pStream->WriteBytes(pData, nSize);

                        const MetaEPSAction* pAct = static_cast<const MetaEPSAction*>(rMtf.FirstAction());
                        const GfxLink& rLink = pAct->GetLink();

                        pStream->WriteBytes(rLink.GetData(), rLink.GetDataSize());
                    }
                    else
                        rMtf.Write(*pStream);

                    bSuccess = (pStream->GetError() == ERRCODE_NONE);
                }
            }
            if (!bSuccess)
                return OUString();

            uno::Reference<embed::XTransactedObject> xStorage(aStream.xStorage, uno::UNO_QUERY);
            pStream.reset();
            aStream.xStream->getOutputStream()->closeOutput();
            if (xStorage.is())
                xStorage->commit();

            OUString aStoragePath("Pictures/");
            aStoragePath += rPictureStreamName;

            // put into cache
            maExportGraphics[aGraphic] = aStoragePath;

            return aStoragePath;
        }
    }

    return OUString();
}

uno::Reference<io::XInputStream> SAL_CALL SvXMLGraphicHelper::createInputStream(uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    Reference<XInputStream> xInputStream;

    Graphic aGraphic(rxGraphic);
    GraphicObject aGraphicObject(aGraphic);

    if (SvXMLGraphicHelperMode::Write == meCreateMode)
    {
        OUString sMimeType = comphelper::GraphicMimeTypeHelper::GetMimeTypeForExtension(OUStringToOString(maOutputMimeType, RTL_TEXTENCODING_ASCII_US));
        std::unique_ptr<xmloff::GraphicInputStream> pInputStream(new xmloff::GraphicInputStream(aGraphicObject, sMimeType));

        // We release the pointer from unique_ptr and assign it to the input stream return type.
        // In case the stream doesn't exists, unique_ptr will delete the pointer when we go out of scope.
        if (pInputStream->exists())
            xInputStream = pInputStream.release();
    }

    return xInputStream;
}

// XBinaryStreamResolver
Reference< XInputStream > SAL_CALL SvXMLGraphicHelper::getInputStream( const OUString& rURL )
{
    Reference< XInputStream >   xRet;
    OUString                    aPictureStorageName, aGraphicId;

    if( ( SvXMLGraphicHelperMode::Write == meCreateMode ) &&
        ImplGetStreamNames( rURL, aPictureStorageName, aGraphicId ) )
    {
        OUString sMimeType = comphelper::GraphicMimeTypeHelper::GetMimeTypeForExtension( OUStringToOString( maOutputMimeType, RTL_TEXTENCODING_ASCII_US ) );
        SvXMLGraphicInputStream* pInputStream = new SvXMLGraphicInputStream( aGraphicId, sMimeType );

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
typedef cppu::WeakComponentImplHelper<lang::XInitialization,
                                        document::XGraphicObjectResolver,
                                        document::XGraphicStorageHandler,
                                        document::XBinaryStreamResolver,
                                        lang::XServiceInfo>
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

    // ____ XGraphicStorageHandler ____
    virtual css::uno::Reference<css::graphic::XGraphic> SAL_CALL
        loadGraphic(const OUString& aURL) override;

    virtual OUString SAL_CALL
        saveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic) override;

    virtual OUString SAL_CALL
        saveGraphicByName(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic, OUString const & rRequestName) override;

    virtual css::uno::Reference<css::io::XInputStream> SAL_CALL
        createInputStream(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic) override;

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
    Reference< XGraphicStorageHandler > m_xGraphicStorageHandler;
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
    // m_xBinaryStreamResolver and m_xGraphicStorageHandler are a reference to the same object,
    // don't call dispose() again
}

// ____ XInitialization ____
void SAL_CALL SvXMLGraphicImportExportHelper::initialize(
    const Sequence< Any >& aArguments )
{
    Reference< embed::XStorage > xStorage;
    if( aArguments.getLength() > 0 )
        aArguments[0] >>= xStorage;

    rtl::Reference<SvXMLGraphicHelper> pHelper( SvXMLGraphicHelper::Create( xStorage, m_eGraphicHelperMode ));
    m_xGraphicObjectResolver.set( pHelper.get() );
    m_xGraphicStorageHandler.set( pHelper.get() );
    m_xBinaryStreamResolver.set( pHelper.get() );
}

// ____ XGraphicObjectResolver ____
OUString SAL_CALL SvXMLGraphicImportExportHelper::resolveGraphicObjectURL( const OUString& aURL )
{
    return m_xGraphicObjectResolver->resolveGraphicObjectURL( aURL );
}

// ____ XGraphicStorageHandler ____
uno::Reference<graphic::XGraphic> SAL_CALL SvXMLGraphicImportExportHelper::loadGraphic(OUString const & rURL)
{
    return m_xGraphicStorageHandler->loadGraphic(rURL);
}

OUString SAL_CALL SvXMLGraphicImportExportHelper::saveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic)
{
    return m_xGraphicStorageHandler->saveGraphic(rxGraphic);
}

OUString SAL_CALL SvXMLGraphicImportExportHelper::saveGraphicByName(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic, OUString const & rRequestName)
{
    return m_xGraphicStorageHandler->saveGraphicByName(rxGraphic, rRequestName);
}

uno::Reference<io::XInputStream> SAL_CALL SvXMLGraphicImportExportHelper::createInputStream(uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    return m_xGraphicStorageHandler->createInputStream(rxGraphic);
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
    Sequence< OUString > aSupportedServiceNames(3);
    aSupportedServiceNames[0] = "com.sun.star.document.GraphicObjectResolver";
    aSupportedServiceNames[1] = "com.sun.star.document.GraphicStorageHandler";
    aSupportedServiceNames[2] = "com.sun.star.document.BinaryStreamResolver";
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
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
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
extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface *
com_sun_star_comp_Svx_GraphicExportHelper_get_implementation(
    css::uno::XComponentContext *,
    css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new SvXMLGraphicImportExportHelper(SvXMLGraphicHelperMode::Write));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
