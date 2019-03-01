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

#include <avmedia/mediaitem.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/uri/UriReferenceFactory.hpp>
#include <com/sun/star/uri/XUriReference.hpp>
#include <com/sun/star/uri/XUriReferenceFactory.hpp>

#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

#include <ucbhelper/content.hxx>

#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <mediamisc.hxx>
#include <osl/file.hxx>
#include <tools/diagnose_ex.h>

using namespace ::com::sun::star;

namespace avmedia
{

SfxPoolItem* MediaItem::CreateDefault() { return new MediaItem; }

struct MediaItem::Impl
{
    OUString                m_URL;
    OUString                m_TempFileURL;
    OUString                m_Referer;
    OUString                m_sMimeType;
    AVMediaSetMask          m_nMaskSet;
    MediaState              m_eState;
    double                  m_fTime;
    double                  m_fDuration;
    sal_Int16               m_nVolumeDB;
    bool                    m_bLoop;
    bool                    m_bMute;
    css::media::ZoomLevel m_eZoom;

    explicit Impl(AVMediaSetMask nMaskSet)
        : m_nMaskSet( nMaskSet )
        , m_eState( MediaState::Stop )
        , m_fTime( 0.0 )
        , m_fDuration( 0.0 )
        , m_nVolumeDB( 0 )
        , m_bLoop( false )
        , m_bMute( false )
        , m_eZoom( css::media::ZoomLevel_NOT_AVAILABLE )
    {
    }
};


MediaItem::MediaItem( sal_uInt16 i_nWhich, AVMediaSetMask nMaskSet )
    : SfxPoolItem( i_nWhich )
    , m_pImpl( new Impl(nMaskSet) )
{
}


MediaItem::MediaItem( const MediaItem& rItem )
    : SfxPoolItem( rItem )
    , m_pImpl( new Impl(*rItem.m_pImpl) )
{
}


MediaItem::~MediaItem()
{
}


bool MediaItem::operator==( const SfxPoolItem& rItem ) const
{
    assert( SfxPoolItem::operator==(rItem));
    MediaItem const& rOther(static_cast< const MediaItem& >(rItem));
    return m_pImpl->m_nMaskSet == rOther.m_pImpl->m_nMaskSet
        && m_pImpl->m_URL == rOther.m_pImpl->m_URL
        && m_pImpl->m_Referer == rOther.m_pImpl->m_Referer
        && m_pImpl->m_sMimeType == rOther.m_pImpl->m_sMimeType
        && m_pImpl->m_eState == rOther.m_pImpl->m_eState
        && m_pImpl->m_fDuration == rOther.m_pImpl->m_fDuration
        && m_pImpl->m_fTime == rOther.m_pImpl->m_fTime
        && m_pImpl->m_nVolumeDB == rOther.m_pImpl->m_nVolumeDB
        && m_pImpl->m_bLoop == rOther.m_pImpl->m_bLoop
        && m_pImpl->m_bMute == rOther.m_pImpl->m_bMute
        && m_pImpl->m_eZoom == rOther.m_pImpl->m_eZoom;
}

SfxPoolItem* MediaItem::Clone( SfxItemPool* ) const
{
    return new MediaItem( *this );
}

bool MediaItem::GetPresentation( SfxItemPresentation,
                                 MapUnit,
                                 MapUnit,
                                 OUString& rText,
                                 const IntlWrapper& ) const
{
    rText.clear();
    return false;
}

bool MediaItem::QueryValue( css::uno::Any& rVal, sal_uInt8 ) const
{
    uno::Sequence< uno::Any > aSeq( 10 );

    aSeq[ 0 ] <<= m_pImpl->m_URL;
    aSeq[ 1 ] <<= static_cast<sal_uInt32>(m_pImpl->m_nMaskSet);
    aSeq[ 2 ] <<= static_cast< sal_Int32 >( m_pImpl->m_eState );
    aSeq[ 3 ] <<= m_pImpl->m_fTime;
    aSeq[ 4 ] <<= m_pImpl->m_fDuration;
    aSeq[ 5 ] <<= m_pImpl->m_nVolumeDB;
    aSeq[ 6 ] <<= m_pImpl->m_bLoop;
    aSeq[ 7 ] <<= m_pImpl->m_bMute;
    aSeq[ 8 ] <<= m_pImpl->m_eZoom;
    aSeq[ 9 ] <<= m_pImpl->m_sMimeType;

    rVal <<= aSeq;

    return true;
}


bool MediaItem::PutValue( const css::uno::Any& rVal, sal_uInt8 )
{
    uno::Sequence< uno::Any >   aSeq;
    bool                        bRet = false;

    if( ( rVal >>= aSeq ) && ( aSeq.getLength() == 10 ) )
    {
        sal_Int32 nInt32 = 0;

        aSeq[ 0 ] >>= m_pImpl->m_URL;
        aSeq[ 1 ] >>= nInt32;
        m_pImpl->m_nMaskSet = static_cast<AVMediaSetMask>(nInt32);
        aSeq[ 2 ] >>= nInt32;
        m_pImpl->m_eState = static_cast< MediaState >( nInt32 );
        aSeq[ 3 ] >>= m_pImpl->m_fTime;
        aSeq[ 4 ] >>= m_pImpl->m_fDuration;
        aSeq[ 5 ] >>= m_pImpl->m_nVolumeDB;
        aSeq[ 6 ] >>= m_pImpl->m_bLoop;
        aSeq[ 7 ] >>= m_pImpl->m_bMute;
        aSeq[ 8 ] >>= m_pImpl->m_eZoom;
        aSeq[ 9 ] >>= m_pImpl->m_sMimeType;

        bRet = true;
    }

    return bRet;
}


void MediaItem::merge( const MediaItem& rMediaItem )
{
    const AVMediaSetMask nMaskSet = rMediaItem.getMaskSet();

    if( AVMediaSetMask::URL & nMaskSet )
        setURL( rMediaItem.getURL(), rMediaItem.getTempURL(), rMediaItem.getReferer() );

    if( AVMediaSetMask::MIME_TYPE & nMaskSet )
        setMimeType( rMediaItem.getMimeType() );

    if( AVMediaSetMask::STATE & nMaskSet )
        setState( rMediaItem.getState() );

    if( AVMediaSetMask::DURATION & nMaskSet )
        setDuration( rMediaItem.getDuration() );

    if( AVMediaSetMask::TIME & nMaskSet )
        setTime( rMediaItem.getTime() );

    if( AVMediaSetMask::LOOP & nMaskSet )
        setLoop( rMediaItem.isLoop() );

    if( AVMediaSetMask::MUTE & nMaskSet )
        setMute( rMediaItem.isMute() );

    if( AVMediaSetMask::VOLUMEDB & nMaskSet )
        setVolumeDB( rMediaItem.getVolumeDB() );

    if( AVMediaSetMask::ZOOM & nMaskSet )
        setZoom( rMediaItem.getZoom() );
}


AVMediaSetMask MediaItem::getMaskSet() const
{
    return m_pImpl->m_nMaskSet;
}


void MediaItem::setURL( const OUString& rURL, const OUString& rTempURL, const OUString& rReferer )
{
    m_pImpl->m_nMaskSet |= AVMediaSetMask::URL;
    m_pImpl->m_URL = rURL;
    m_pImpl->m_TempFileURL = rTempURL;
    m_pImpl->m_Referer = rReferer;
}


const OUString& MediaItem::getURL() const
{
    return m_pImpl->m_URL;
}


const OUString& MediaItem::getTempURL() const
{
    return m_pImpl->m_TempFileURL;
}


const OUString& MediaItem::getReferer() const
{
    return m_pImpl->m_Referer;
}


void MediaItem::setMimeType( const OUString& rMimeType )
{
    m_pImpl->m_nMaskSet |= AVMediaSetMask::MIME_TYPE;
    m_pImpl->m_sMimeType = rMimeType;
}


OUString MediaItem::getMimeType() const
{
    return !m_pImpl->m_sMimeType.isEmpty() ? m_pImpl->m_sMimeType : AVMEDIA_MIMETYPE_COMMON;
}


void MediaItem::setState( MediaState eState )
{
    m_pImpl->m_eState = eState;
    m_pImpl->m_nMaskSet |= AVMediaSetMask::STATE;
}


MediaState MediaItem::getState() const
{
    return m_pImpl->m_eState;
}


void MediaItem::setDuration( double fDuration )
{
    m_pImpl->m_fDuration = fDuration;
    m_pImpl->m_nMaskSet |= AVMediaSetMask::DURATION;
}


double MediaItem::getDuration() const
{
    return m_pImpl->m_fDuration;
}


void MediaItem::setTime( double fTime )
{
    m_pImpl->m_fTime = fTime;
    m_pImpl->m_nMaskSet |= AVMediaSetMask::TIME;
}


double MediaItem::getTime() const
{
    return m_pImpl->m_fTime;
}


void MediaItem::setLoop( bool bLoop )
{
    m_pImpl->m_bLoop = bLoop;
    m_pImpl->m_nMaskSet |= AVMediaSetMask::LOOP;
}


bool MediaItem::isLoop() const
{
    return m_pImpl->m_bLoop;
}


void MediaItem::setMute( bool bMute )
{
    m_pImpl->m_bMute = bMute;
    m_pImpl->m_nMaskSet |= AVMediaSetMask::MUTE;
}


bool MediaItem::isMute() const
{
    return m_pImpl->m_bMute;
}


void MediaItem::setVolumeDB( sal_Int16 nDB )
{
    m_pImpl->m_nVolumeDB = nDB;
    m_pImpl->m_nMaskSet |= AVMediaSetMask::VOLUMEDB;
}


sal_Int16 MediaItem::getVolumeDB() const
{
    return m_pImpl->m_nVolumeDB;
}


void MediaItem::setZoom( css::media::ZoomLevel eZoom )
{
    m_pImpl->m_eZoom = eZoom;
    m_pImpl->m_nMaskSet |= AVMediaSetMask::ZOOM;
}


css::media::ZoomLevel MediaItem::getZoom() const
{
    return m_pImpl->m_eZoom;
}


OUString GetFilename(OUString const& rSourceURL)
{
    uno::Reference<uri::XUriReferenceFactory> const xUriFactory(
        uri::UriReferenceFactory::create(
            comphelper::getProcessComponentContext()));
    uno::Reference<uri::XUriReference> const xSourceURI(
        xUriFactory->parse(rSourceURL), uno::UNO_SET_THROW);

    OUString filename;
    {
        sal_Int32 const nSegments(xSourceURI->getPathSegmentCount());
        if (0 < nSegments)
        {
            filename = xSourceURI->getPathSegment(nSegments - 1);
        }
    }
    if (!::comphelper::OStorageHelper::IsValidZipEntryFileName(
                filename, false) || !filename.getLength())
    {
        filename = "media";
    }
    return filename;
}


uno::Reference<io::XStream>
CreateStream(uno::Reference<embed::XStorage> const& xStorage,
        OUString const& rFilename)
{
    OUString filename(rFilename);

    if (xStorage->hasByName(filename))
    {
        OUString basename;
        OUString suffix;
        sal_Int32 const nIndex(rFilename.lastIndexOf('.'));
        if (0 < nIndex)
        {
            basename = rFilename.copy(0, nIndex);
            suffix = rFilename.copy(nIndex);
        }
        sal_Int32 count(0); // sigh... try to generate non-existent name
        do
        {
            ++count;
            filename = basename + OUString::number(count) + suffix;
        }
        while (xStorage->hasByName(filename));
    }

    uno::Reference<io::XStream> const xStream(
        xStorage->openStreamElement(filename,
            embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE),
        uno::UNO_SET_THROW);
    uno::Reference< beans::XPropertySet > const xStreamProps(xStream,
        uno::UNO_QUERY);
    if (xStreamProps.is()) { // this is NOT supported in FileSystemStorage
        xStreamProps->setPropertyValue("MediaType", uno::Any(OUString(
            //FIXME how to detect real media type?
            //but currently xmloff has this one hardcoded anyway...
            "application/vnd.sun.star.media")));
        xStreamProps->setPropertyValue( // turn off compression
            "Compressed", uno::Any(false));
    }
    return xStream;
}


bool EmbedMedia(uno::Reference<frame::XModel> const& xModel,
        OUString const& rSourceURL, OUString & o_rEmbeddedURL, uno::Reference<io::XInputStream> const& xInputStream)
{
    try
    {
        uno::Reference<document::XStorageBasedDocument> const xSBD(xModel,
                uno::UNO_QUERY_THROW);
        uno::Reference<embed::XStorage> const xStorage(
                xSBD->getDocumentStorage(), uno::UNO_QUERY_THROW);

        OUString const media("Media");
        uno::Reference<embed::XStorage> const xSubStorage(
            xStorage->openStorageElement(media, embed::ElementModes::WRITE));

        OUString filename(GetFilename(rSourceURL));

        uno::Reference<io::XStream> const xStream(
            CreateStream(xSubStorage, filename), uno::UNO_SET_THROW);
        uno::Reference<io::XOutputStream> const xOutStream(
            xStream->getOutputStream(), uno::UNO_SET_THROW);

        if (xInputStream.is())
        {
            // Throw Exception if failed.
            ::comphelper::OStorageHelper::CopyInputToOutput(xInputStream, xOutStream);
        }
        else
        {
            ::ucbhelper::Content sourceContent(rSourceURL,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());

            if (!sourceContent.openStream(xOutStream)) // copy file to storage
            {
                SAL_INFO("avmedia", "openStream to storage failed");
                return false;
            }
        }

        uno::Reference<embed::XTransactedObject> const xSubTransaction(
            xSubStorage, uno::UNO_QUERY);
        if (xSubTransaction.is()) {
            xSubTransaction->commit();
        }
        uno::Reference<embed::XTransactedObject> const xTransaction(
            xStorage, uno::UNO_QUERY);
        if (xTransaction.is()) {
            xTransaction->commit();
        }

        o_rEmbeddedURL = "vnd.sun.star.Package:" + media + "/" + filename;
        return true;
    }
    catch (uno::Exception const&)
    {
        SAL_WARN("avmedia",
                "Exception while trying to embed media");
    }
    return false;
}

bool CreateMediaTempFile(uno::Reference<io::XInputStream> const& xInStream,
        OUString& o_rTempFileURL, const OUString& rDesiredExtension)
{
    OUString tempFileURL;
    ::osl::FileBase::RC const err =
        ::osl::FileBase::createTempFile(nullptr, nullptr, & tempFileURL);
    if (::osl::FileBase::E_None != err)
    {
        SAL_WARN("avmedia", "cannot create temp file");
        return false;
    }

    if (!rDesiredExtension.isEmpty())
    {
        OUString newTempFileURL = tempFileURL + rDesiredExtension;
        if (osl::File::move(tempFileURL, newTempFileURL) != osl::FileBase::E_None)
        {
            SAL_WARN("avmedia", "Could not rename file '" << tempFileURL << "' to '" << newTempFileURL << "'");
            return false;
        }
        tempFileURL = newTempFileURL;
    }

    try
    {
        ::ucbhelper::Content tempContent(tempFileURL,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());
        tempContent.writeStream(xInStream, true); // copy stream to file
    }
    catch (uno::Exception const&)
    {
        css::uno::Any ex( cppu::getCaughtException() );
        SAL_WARN("avmedia", "exception: " << exceptionToString(ex));
        return false;
    }
    o_rTempFileURL = tempFileURL;
    return true;
}

MediaTempFile::~MediaTempFile()
{
    ::osl::File::remove(m_TempFileURL);
}

} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
