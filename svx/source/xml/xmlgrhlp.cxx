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
#include <sal/log.hxx>

#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/io/NotConnectedException.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/XCancellable.hpp>
#include <com/sun/star/embed/XHierarchicalStorageAccess.hpp>
#include <comphelper/fileformat.h>
#include <comphelper/graphicmimetype.hxx>
#include <comphelper/compbase.hxx>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>

#include <rtl/ref.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <unotools/streamwrap.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/saveopt.hxx>
#include <vcl/filter/SvmWriter.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/metaact.hxx>
#include <tools/zcodec.hxx>
#include <comphelper/diagnose_ex.hxx>

#include <vcl/GraphicObject.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/xmlgrhlp.hxx>
#include <svx/xmleohlp.hxx>

#include <algorithm>
#include <memory>
#include <string_view>
#include <utility>

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::io;

namespace com::sun::star::uno { class XComponentContext; }

constexpr OUStringLiteral XML_GRAPHICSTORAGE_NAME = u"Pictures";
constexpr OUStringLiteral XML_GRAPHICOBJECT_URL_BASE = u"vnd.sun.star.GraphicObject:";

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

class GraphicInputStream : public cppu::WeakImplHelper<XInputStream>
{
private:
    virtual sal_Int32 SAL_CALL readBytes(Sequence<sal_Int8> & aData, sal_Int32 nBytesToRead) override;
    virtual sal_Int32 SAL_CALL readSomeBytes(Sequence<sal_Int8> & aData, sal_Int32 nMaxBytesToRead) override;
    virtual void SAL_CALL skipBytes(sal_Int32 nBytesToSkip) override;
    virtual sal_Int32 SAL_CALL available() override;
    virtual void SAL_CALL closeInput() override;

private:
    utl::TempFileFast maTempFile;
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


GraphicInputStream::GraphicInputStream(GraphicObject const & aGraphicObject, const OUString & rMimeType)
{
    if (aGraphicObject.GetType() == GraphicType::NONE)
        return;

    SvStream* pStream = maTempFile.GetStream(StreamMode::READWRITE);

    if (!pStream)
        return;

    const Graphic& aGraphic(aGraphicObject.GetGraphic());
    const GfxLink aGfxLink(aGraphic.GetGfxLink());
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
            bRet = (rFilter.ExportGraphic(aGraphic, u"", *pStream, rFilter.GetExportFormatNumberForMediaType(rMimeType)) == ERRCODE_NONE);
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

            bRet = (rFilter.ExportGraphic(aGraphic, u"", *pStream, rFilter.GetExportFormatNumberForMediaType(aFormat)) == ERRCODE_NONE);
        }
        else if (rMimeType.isEmpty() && aGraphic.GetType() == GraphicType::GdiMetafile)
        {
            pStream->SetVersion(SOFFICE_FILEFORMAT_8);
            pStream->SetCompressMode(SvStreamCompressFlags::ZBITMAP);
            SvmWriter aWriter(*pStream);
            aWriter.Write(aGraphic.GetGDIMetaFile());
            bRet = (pStream->GetError() == ERRCODE_NONE);
        }
        else if (!rMimeType.isEmpty())
        {
            GraphicFilter & rFilter = GraphicFilter::GetGraphicFilter();
            bRet = ( rFilter.ExportGraphic( aGraphic, u"", *pStream, rFilter.GetExportFormatNumberForMediaType( rMimeType ) ) == ERRCODE_NONE );
        }
    }

    if (bRet)
    {
        pStream->Seek( 0 );
        mxStreamWrapper = new ::utl::OInputStreamWrapper(*pStream);
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

class SvXMLGraphicOutputStream:
    public cppu::WeakImplHelper<XOutputStream>
{
private:

    // XOutputStream
    virtual void SAL_CALL           writeBytes( const Sequence< sal_Int8 >& rData ) override;
    virtual void SAL_CALL           flush() override;
    virtual void SAL_CALL           closeOutput() override;

private:

    std::optional<::utl::TempFileFast> moTmp;
    SvStream*                        mpOStm;
    Reference< XOutputStream >       mxStmWrapper;
    std::optional<GraphicObject>     moGrfObj;
    bool                             mbClosed;

public:

                                    SvXMLGraphicOutputStream();
    virtual                         ~SvXMLGraphicOutputStream() override;
                                    SvXMLGraphicOutputStream(const SvXMLGraphicOutputStream&) = delete;
    SvXMLGraphicOutputStream&       operator=(const SvXMLGraphicOutputStream&) = delete;

    bool                            Exists() const { return mxStmWrapper.is(); }
    const GraphicObject&            GetGraphicObject();
    Graphic GetGraphic();
};

SvXMLGraphicOutputStream::SvXMLGraphicOutputStream()
    : moTmp(std::in_place)
    , moGrfObj(std::in_place)
    , mbClosed(false)
{
    mpOStm = moTmp->GetStream( StreamMode::READWRITE );

    if( mpOStm )
        mxStmWrapper = new ::utl::OOutputStreamWrapper( *mpOStm );
}

SvXMLGraphicOutputStream::~SvXMLGraphicOutputStream()
{
    moTmp.reset();
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

Graphic SvXMLGraphicOutputStream::GetGraphic()
{
    Graphic aGraphic;

    if (mbClosed && moGrfObj->GetType() == GraphicType::NONE && mpOStm)
    {
        mpOStm->Seek( 0 );
        sal_uInt16 nFormat = GRFILTER_FORMAT_DONTKNOW;
        sal_uInt16 nDeterminedFormat = GRFILTER_FORMAT_DONTKNOW;
        GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, u"", *mpOStm ,nFormat,&nDeterminedFormat);

        if (nDeterminedFormat == GRFILTER_FORMAT_DONTKNOW)
        {
            //Read the first two byte to check whether it is a gzipped stream, is so it may be in wmz or emz format
            //unzip them and try again

            sal_uInt8    sFirstBytes[ 2 ];

            sal_uInt64 nStreamLen = mpOStm->TellEnd();
            mpOStm->Seek( 0 );

            if ( nStreamLen == 0 )
            {
                SvLockBytes* pLockBytes = mpOStm->GetLockBytes();
                if ( pLockBytes  )
                    pLockBytes->SetSynchronMode();

                nStreamLen = mpOStm->TellEnd();
                mpOStm->Seek( 0 );
            }
            if( nStreamLen >= 2 )
            {
                //read two byte
                mpOStm->ReadBytes(sFirstBytes, 2);

                if( sFirstBytes[0] == 0x1f && sFirstBytes[1] == 0x8b )
                {
                    SvMemoryStream aDest;
                    ZCodec aZCodec( 0x8000, 0x8000 );
                    aZCodec.BeginCompression(ZCODEC_DEFAULT_COMPRESSION, /*gzLib*/true);
                    mpOStm->Seek( 0 );
                    aZCodec.Decompress( *mpOStm, aDest );

                    if (aZCodec.EndCompression())
                    {
                        sal_uInt64 nStreamLen_ = aDest.TellEnd();
                        if (nStreamLen_ > 0)
                        {
                            aDest.Seek(0);
                            GraphicFilter::GetGraphicFilter().ImportGraphic( aGraphic, u"", aDest ,nFormat,&nDeterminedFormat );
                        }
                    }
                }
            }
        }
    }

    if (aGraphic.GetType() != GraphicType::NONE)
    {
        mpOStm = nullptr;
        moTmp.reset();
    }
    return aGraphic;
}

const GraphicObject& SvXMLGraphicOutputStream::GetGraphicObject()
{
    Graphic aGraphic(GetGraphic());
    if (aGraphic.GetType() != GraphicType::NONE)
    {
        moGrfObj.emplace(std::move(aGraphic));
    }
    return *moGrfObj;
}

}

SvXMLGraphicHelper::SvXMLGraphicHelper(SvXMLGraphicHelperMode eCreateMode)
{
    Init( nullptr, eCreateMode );
}

SvXMLGraphicHelper::SvXMLGraphicHelper()
    : meCreateMode(SvXMLGraphicHelperMode::Read)
{
}

SvXMLGraphicHelper::~SvXMLGraphicHelper()
{
}

bool SvXMLGraphicHelper::ImplGetStreamNames( const OUString& rURLStr,
                                                 OUString& rPictureStorageName,
                                                 OUString& rPictureStreamName )
{
    if (rURLStr.isEmpty())
        return false;

    const OUString aURLStr {rURLStr.copy(rURLStr.lastIndexOf(':')+1)};

    if( !aURLStr.isEmpty() && aURLStr.indexOf('/')<0 ) // just one token?
    {
        rPictureStorageName = OUString();
        rPictureStreamName = aURLStr;
    }
    else
        SvXMLEmbeddedObjectHelper::splitObjectURL(aURLStr, rPictureStorageName, rPictureStreamName);

    SAL_WARN_IF(rPictureStreamName.isEmpty(), "svx", "SvXMLGraphicHelper::ImplInsertGraphicURL: invalid scheme: " << rURLStr);

    return !rPictureStreamName.isEmpty();
}

uno::Reference < embed::XStorage > SvXMLGraphicHelper::ImplGetGraphicStorage( const OUString& rStorageName )
{
    uno::Reference < embed::XStorage > xRetStorage;
    if( mxRootStorage.is() )
    {
        try
        {
            maCurStorageName = rStorageName;
            xRetStorage = mxRootStorage->openStorageElement(
                maCurStorageName,
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
                maCurStorageName = rStorageName;
                xRetStorage = mxRootStorage->openStorageElement( maCurStorageName, embed::ElementModes::READ );
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
    if (!rPictureStorageName.isEmpty())
        aRet.xStorage = ImplGetGraphicStorage(rPictureStorageName);
    else
        aRet.xStorage = mxRootStorage;

    sal_Int32 nMode = embed::ElementModes::READ;
    if (SvXMLGraphicHelperMode::Write == meCreateMode)
    {
        nMode = embed::ElementModes::READWRITE;
    }

    if (aRet.xStorage.is())
    {
        aRet.xStream = aRet.xStorage->openStreamElement( rPictureStreamName, nMode );
    }
    else if (rPictureStorageName.indexOf('/') != -1)
    {
        uno::Reference<embed::XHierarchicalStorageAccess> xHierRootStorage(mxRootStorage,
                                                                           uno::UNO_QUERY);
        if (xHierRootStorage.is())
        {
            try
            {
                aRet.xStream = xHierRootStorage->openStreamElementByHierarchicalName(
                    rPictureStorageName + "/" + rPictureStreamName, nMode);
                aRet.xStorage = mxRootStorage;
            }
            catch (const uno::Exception&)
            {
                TOOLS_WARN_EXCEPTION("svx",
                                     "SvXMLGraphicHelper::ImplGetGraphicStream: failed to open "
                                         << rPictureStreamName);
            }
        }
    }

    if (aRet.xStream.is() && (SvXMLGraphicHelperMode::Write == meCreateMode))
    {
        uno::Reference<beans::XPropertySet> xProps(aRet.xStream, uno::UNO_QUERY);
        xProps->setPropertyValue(u"UseCommonStoragePasswordEncryption"_ustr, uno::Any(true));
    }

    return aRet;
}

OUString SvXMLGraphicHelper::ImplGetGraphicMimeType( std::u16string_view rFileName )
{
    if( ( rFileName.size() >= 4 ) && ( rFileName[ rFileName.size() - 4 ] == '.' ) )
    {
        const OString aExt(OUStringToOString(rFileName.substr(rFileName.size() - 3),
            RTL_TEXTENCODING_ASCII_US));
        return comphelper::GraphicMimeTypeHelper::GetMimeTypeForExtension( aExt );
    }

    return OUString();
}

Graphic SvXMLGraphicHelper::ImplReadGraphic( const OUString& rPictureStorageName,
                                             const OUString& rPictureStreamName )
{
    Graphic aReturnGraphic;
    SvxGraphicHelperStream_Impl aStream( ImplGetGraphicStream( rPictureStorageName, rPictureStreamName ) );
    if (aStream.xStream.is())
    {
        GraphicFilter& rGraphicFilter = GraphicFilter::GetGraphicFilter();
        std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(aStream.xStream));
        Graphic aGraphic = rGraphicFilter.ImportUnloadedGraphic(*pStream);
        if (!aGraphic.IsNone())
            aReturnGraphic = aGraphic;
        else
            rGraphicFilter.ImportGraphic(aReturnGraphic, u"", *pStream);
    }

    return aReturnGraphic;
}

void SvXMLGraphicHelper::Init( const uno::Reference < embed::XStorage >& rXMLStorage,
                               SvXMLGraphicHelperMode eCreateMode,
                               const OUString& rGraphicMimeType )
{
    mxRootStorage = rXMLStorage;
    meCreateMode = eCreateMode;
    maOutputMimeType = rGraphicMimeType;
}

rtl::Reference<SvXMLGraphicHelper> SvXMLGraphicHelper::Create( const uno::Reference < embed::XStorage >& rXMLStorage,
                                                SvXMLGraphicHelperMode eCreateMode )
{
    rtl::Reference<SvXMLGraphicHelper> pThis = new SvXMLGraphicHelper;
    pThis->Init( rXMLStorage, eCreateMode, OUString() );

    return pThis;
}

rtl::Reference<SvXMLGraphicHelper> SvXMLGraphicHelper::Create( SvXMLGraphicHelperMode eCreateMode,
                                              const OUString& rGraphicMimeType )
{
    rtl::Reference<SvXMLGraphicHelper> pThis = new SvXMLGraphicHelper;

    pThis->Init( nullptr, eCreateMode, rGraphicMimeType );

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
OUString SAL_CALL SvXMLGraphicHelper::resolveGraphicObjectURL( const OUString& /*rURL*/ )
{
    throw uno::RuntimeException(u"XGraphicObjectResolver has been removed in LibreOffice 6.1"_ustr);
}

// XGraphicStorageHandler
uno::Reference<graphic::XGraphic> SAL_CALL SvXMLGraphicHelper::loadGraphic(OUString const & rURL)
{
    std::unique_lock aGuard(m_aMutex);

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

uno::Reference<graphic::XGraphic> SAL_CALL SvXMLGraphicHelper::loadGraphicFromOutputStream(uno::Reference<io::XOutputStream> const & rxOutputStream)
{
    std::unique_lock aGuard(m_aMutex);

    uno::Reference<graphic::XGraphic> xGraphic;

    if ((SvXMLGraphicHelperMode::Read == meCreateMode) && rxOutputStream.is())
    {

        SvXMLGraphicOutputStream* pGraphicOutputStream = static_cast<SvXMLGraphicOutputStream*>(rxOutputStream.get());
        if (pGraphicOutputStream)
        {
            xGraphic = pGraphicOutputStream->GetGraphic().GetXGraphic();
        }
    }
    return xGraphic;
}

OUString SAL_CALL SvXMLGraphicHelper::saveGraphicByName(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic,
                                                        OUString & rOutSavedMimeType, OUString const & rRequestName)
{
    return implSaveGraphic(rxGraphic, rOutSavedMimeType, rRequestName);
}

OUString SAL_CALL SvXMLGraphicHelper::saveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic)
{
    OUString aOutMimeType;
    return implSaveGraphic(rxGraphic, aOutMimeType, std::u16string_view());
}

OUString SvXMLGraphicHelper::implSaveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic,
                                             OUString & rOutSavedMimeType, std::u16string_view rRequestName)
{
    Graphic aGraphic(rxGraphic);

    auto aIterator = maExportGraphics.find(aGraphic);
    if (aIterator != maExportGraphics.end())
    {
        auto const & aURLAndMimePair = aIterator->second;
        rOutSavedMimeType = aURLAndMimePair.second;
        return aURLAndMimePair.first;
    }

    GraphicObject aGraphicObject(aGraphic);

    if (aGraphicObject.GetType() != GraphicType::NONE)
    {
        const GfxLink aGfxLink(aGraphic.GetGfxLink());
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
                case GfxLinkType::NativeWmf:
                    if (aGfxLink.IsEMF())
                        aExtension = ".emf";
                    else
                        aExtension = ".wmf";
                    break;
                case GfxLinkType::NativeMet: aExtension = ".met"; break;
                case GfxLinkType::NativePct: aExtension = ".pct"; break;
                case GfxLinkType::NativeSvg:
                {
                    // backward-compat kludge: since no released OOo
                    // version to date can handle svg properly, wrap it up
                    // into an svm. slight catch22 here, since strict ODF
                    // conformance _recommends_ svg - then again, most old
                    // ODF consumers are believed to be OOo
                    auto nSaneVersion = GetODFSaneDefaultVersion();
                    if ( nSaneVersion < SvtSaveOptions::ODFSVER_012
                        || nSaneVersion == SvtSaveOptions::ODFSVER_012_EXT_COMPAT)
                    {
                        bUseGfxLink = false;
                        aExtension = ".svm";
                    }
                    else
                    {
                        aExtension = ".svg";
                    }
                    break;
                }
                case GfxLinkType::NativePdf: aExtension = ".pdf"; break;
                case GfxLinkType::NativeWebp: aExtension = ".webp"; break;

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
                // SJ: first check if this metafile is just an eps file, then we will store the eps instead of svm
                GDIMetaFile& rMetafile(const_cast<GDIMetaFile&>(aGraphic.GetGDIMetaFile()));

                if (ImplCheckForEPS(rMetafile))
                    aExtension = ".eps";
                else
                    aExtension = ".svm";
            }
        }

        OUString rPictureStreamName;
        if (!rRequestName.empty())
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
                xProps->setPropertyValue(u"MediaType"_ustr, uno::Any(aMimeType));
            }

            // picture formats that actually _do_ benefit from zip
            // storage compression
            // .svm pics gets compressed via ZBITMAP old-style stream
            // option below
            static const char* aCompressiblePics[] =
            {
                "image/svg+xml",
                "image/x-emf",
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

            xProps->setPropertyValue(u"Compressed"_ustr, Any(bCompressed));

            std::unique_ptr<SvStream> pStream(utl::UcbStreamHelper::CreateStream(aStream.xStream));
            if (bUseGfxLink && aGfxLink.GetDataSize() && aGfxLink.GetData())
            {
                pStream->WriteBytes(aGfxLink.GetData(), aGfxLink.GetDataSize());
                rOutSavedMimeType = aMimeType;
                bSuccess = (pStream->GetError() == ERRCODE_NONE);
            }
            else
            {
                if (aGraphic.GetType() == GraphicType::Bitmap)
                {
                    GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
                    OUString aFormat;

                    if (aGraphic.IsAnimated())
                    {
                        aFormat = "gif";
                    }
                    else
                    {
                        aFormat = "png";
                    }
                    rOutSavedMimeType = comphelper::GraphicMimeTypeHelper::GetMimeTypeForExtension(aFormat.toUtf8());

                    bSuccess = (rFilter.ExportGraphic(aGraphic, u"", *pStream, rFilter.GetExportFormatNumberForShortName(aFormat)) == ERRCODE_NONE);
                }
                else if (aGraphic.GetType() == GraphicType::GdiMetafile)
                {
                    pStream->SetVersion(SOFFICE_FILEFORMAT_8);
                    pStream->SetCompressMode(SvStreamCompressFlags::ZBITMAP);
                    rOutSavedMimeType = comphelper::GraphicMimeTypeHelper::GetMimeTypeForExtension("svm");

                    // SJ: first check if this metafile is just an eps file, then we will store the eps instead of svm
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
                    {
                        SvmWriter aWriter(*pStream);
                        aWriter.Write(rMtf);
                    }

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

            OUString aStoragePath = "Pictures/" + rPictureStreamName;

            // put into cache
            maExportGraphics[aGraphic] = std::make_pair(aStoragePath, rOutSavedMimeType);

            return aStoragePath;
        }
    }

    return OUString();
}

uno::Reference<io::XInputStream> SAL_CALL SvXMLGraphicHelper::createInputStream(uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    Reference<XInputStream> xInputStream;

    GraphicObject aGraphicObject((Graphic(rxGraphic)));

    if (SvXMLGraphicHelperMode::Write == meCreateMode)
    {
        OUString sMimeType = comphelper::GraphicMimeTypeHelper::GetMimeTypeForExtension(OUStringToOString(maOutputMimeType, RTL_TEXTENCODING_ASCII_US));
        rtl::Reference<GraphicInputStream> pInputStream(new GraphicInputStream(aGraphicObject, sMimeType));

        // We release the pointer from unique_ptr and assign it to the input stream return type.
        // In case the stream doesn't exists, unique_ptr will delete the pointer when we go out of scope.
        if (pInputStream->exists())
            xInputStream = pInputStream.get();
    }

    return xInputStream;
}

// XBinaryStreamResolver
Reference< XInputStream > SAL_CALL SvXMLGraphicHelper::getInputStream( const OUString& /*rURL*/ )
{
    Reference<XInputStream> xRet;
    return xRet;
}

Reference< XOutputStream > SAL_CALL SvXMLGraphicHelper::createOutputStream()
{
    Reference< XOutputStream > xRet;

    if( SvXMLGraphicHelperMode::Read == meCreateMode )
    {
        rtl::Reference<SvXMLGraphicOutputStream> pOutputStream(new SvXMLGraphicOutputStream);

        if( pOutputStream->Exists() )
        {
            xRet = pOutputStream.get();
            maGrfStms.push_back( xRet );
        }
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
                    aRet = XML_GRAPHICOBJECT_URL_BASE + aId;
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
typedef comphelper::WeakComponentImplHelper<lang::XInitialization,
                                        document::XGraphicObjectResolver,
                                        document::XGraphicStorageHandler,
                                        document::XBinaryStreamResolver,
                                        lang::XServiceInfo>
    SvXMLGraphicImportExportHelper_Base;

} // namespace impl

class SvXMLGraphicImportExportHelper :
    public impl::SvXMLGraphicImportExportHelper_Base
{
public:
    explicit SvXMLGraphicImportExportHelper( SvXMLGraphicHelperMode eMode );

protected:
    // is called from WeakComponentImplHelper when XComponent::dispose() was
    // called from outside
    virtual void disposing(std::unique_lock<std::mutex>&) override;

    // ____ XInitialization ____
    // one argument is allowed, which is the XStorage
    virtual void SAL_CALL initialize( const Sequence< Any >& aArguments ) override;

    // ____ XGraphicObjectResolver ____
    virtual OUString SAL_CALL resolveGraphicObjectURL( const OUString& aURL ) override;

    // ____ XGraphicStorageHandler ____
    virtual css::uno::Reference<css::graphic::XGraphic> SAL_CALL
        loadGraphic(const OUString& aURL) override;

    virtual css::uno::Reference<css::graphic::XGraphic> SAL_CALL
        loadGraphicFromOutputStream(css::uno::Reference<css::io::XOutputStream> const & rxOutputStream) override;

    virtual OUString SAL_CALL
        saveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic) override;

    virtual OUString SAL_CALL
        saveGraphicByName(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic, OUString & rOutSavedMimeType, OUString const & rRequestName) override;

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
    rtl::Reference<SvXMLGraphicHelper>  m_xXMLGraphicHelper;
};

SvXMLGraphicImportExportHelper::SvXMLGraphicImportExportHelper( SvXMLGraphicHelperMode eMode ) :
        m_eGraphicHelperMode( eMode )
{}

void SvXMLGraphicImportExportHelper::disposing(std::unique_lock<std::mutex>&)
{
    if (m_xXMLGraphicHelper)
    {
        m_xXMLGraphicHelper->dispose();
        m_xXMLGraphicHelper.clear();
    }
}

// ____ XInitialization ____
void SAL_CALL SvXMLGraphicImportExportHelper::initialize(
    const Sequence< Any >& aArguments )
{
    Reference< embed::XStorage > xStorage;
    if( aArguments.hasElements() )
        aArguments[0] >>= xStorage;

    m_xXMLGraphicHelper = SvXMLGraphicHelper::Create( xStorage, m_eGraphicHelperMode );
}

// ____ XGraphicObjectResolver ____
OUString SAL_CALL SvXMLGraphicImportExportHelper::resolveGraphicObjectURL( const OUString& aURL )
{
    return m_xXMLGraphicHelper->resolveGraphicObjectURL( aURL );
}

// ____ XGraphicStorageHandler ____
uno::Reference<graphic::XGraphic> SAL_CALL SvXMLGraphicImportExportHelper::loadGraphic(OUString const & rURL)
{
    return m_xXMLGraphicHelper->loadGraphic(rURL);
}

uno::Reference<graphic::XGraphic> SAL_CALL SvXMLGraphicImportExportHelper::loadGraphicFromOutputStream(uno::Reference<io::XOutputStream> const & rxOutputStream)
{
    return m_xXMLGraphicHelper->loadGraphicFromOutputStream(rxOutputStream);
}

OUString SAL_CALL SvXMLGraphicImportExportHelper::saveGraphic(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic)
{
    return m_xXMLGraphicHelper->saveGraphic(rxGraphic);
}

OUString SAL_CALL SvXMLGraphicImportExportHelper::saveGraphicByName(css::uno::Reference<css::graphic::XGraphic> const & rxGraphic,
                                                                    OUString & rOutSavedMimeType, OUString const & rRequestName)
{
    return m_xXMLGraphicHelper->saveGraphicByName(rxGraphic, rOutSavedMimeType, rRequestName);
}

uno::Reference<io::XInputStream> SAL_CALL SvXMLGraphicImportExportHelper::createInputStream(uno::Reference<graphic::XGraphic> const & rxGraphic)
{
    return m_xXMLGraphicHelper->createInputStream(rxGraphic);
}

// ____ XBinaryStreamResolver ____
Reference< io::XInputStream > SAL_CALL SvXMLGraphicImportExportHelper::getInputStream( const OUString& aURL )
{
    return m_xXMLGraphicHelper->getInputStream( aURL );
}
Reference< io::XOutputStream > SAL_CALL SvXMLGraphicImportExportHelper::createOutputStream()
{
    return m_xXMLGraphicHelper->createOutputStream();
}
OUString SAL_CALL SvXMLGraphicImportExportHelper::resolveOutputStream( const Reference< io::XOutputStream >& aBinaryStream )
{
    return m_xXMLGraphicHelper->resolveOutputStream( aBinaryStream );
}

// ____ XServiceInfo ____
OUString SAL_CALL SvXMLGraphicImportExportHelper::getImplementationName()
{
    if( m_eGraphicHelperMode == SvXMLGraphicHelperMode::Read )
        return u"com.sun.star.comp.Svx.GraphicImportHelper"_ustr;
    return u"com.sun.star.comp.Svx.GraphicExportHelper"_ustr;
}

sal_Bool SAL_CALL SvXMLGraphicImportExportHelper::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

Sequence< OUString > SAL_CALL SvXMLGraphicImportExportHelper::getSupportedServiceNames()
{
    return { u"com.sun.star.document.GraphicObjectResolver"_ustr,
             u"com.sun.star.document.GraphicStorageHandler"_ustr,
             u"com.sun.star.document.BinaryStreamResolver"_ustr };
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

namespace svx {

    void DropUnusedNamedItems(css::uno::Reference<css::uno::XInterface> const& xModel)
    {
        uno::Reference<lang::XMultiServiceFactory> const xModelFactory(xModel, uno::UNO_QUERY);
        assert(xModelFactory.is());
        try
        {
            uno::Reference<util::XCancellable> const xGradient(
                xModelFactory->createInstance(u"com.sun.star.drawing.GradientTable"_ustr),
                uno::UNO_QUERY );
            if (xGradient.is())
            {
                xGradient->cancel();
            }

            uno::Reference<util::XCancellable> const xHatch(
                xModelFactory->createInstance(u"com.sun.star.drawing.HatchTable"_ustr),
                uno::UNO_QUERY );
            if (xHatch.is())
            {
                xHatch->cancel();
            }

            uno::Reference<util::XCancellable> const xBitmap(
                xModelFactory->createInstance(u"com.sun.star.drawing.BitmapTable"_ustr),
                uno::UNO_QUERY );
            if (xBitmap.is())
            {
                xBitmap->cancel();
            }

            uno::Reference<util::XCancellable> const xTransGradient(
                xModelFactory->createInstance(u"com.sun.star.drawing.TransparencyGradientTable"_ustr),
                uno::UNO_QUERY );
            if (xTransGradient.is())
            {
                xTransGradient->cancel();
            }

            uno::Reference<util::XCancellable> const xMarker(
                xModelFactory->createInstance(u"com.sun.star.drawing.MarkerTable"_ustr),
                uno::UNO_QUERY );
            if (xMarker.is())
            {
                xMarker->cancel();
            }

            uno::Reference<util::XCancellable> const xDashes(
                xModelFactory->createInstance(u"com.sun.star.drawing.DashTable"_ustr),
                uno::UNO_QUERY );
            if (xDashes.is())
            {
                xDashes->cancel();
            }
        }
        catch (const Exception&)
        {
            TOOLS_WARN_EXCEPTION("svx", "dropUnusedNamedItems(): exception during clearing of unused named items");
        }
    }

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
