/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column:100 -*- */
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

#include <config_features.h>

#include <svx/svdomedia.hxx>

#include <com/sun/star/text/GraphicCrop.hpp>

#include <rtl/ustring.hxx>
#include <sal/log.hxx>

#include <ucbhelper/content.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>

#include <vcl/svapp.hxx>

#include <svx/svdmodel.hxx>
#include <svx/dialmgr.hxx>
#include <svx/strings.hrc>
#include <svx/sdr/contact/viewcontactofsdrmediaobj.hxx>
#include <avmedia/mediawindow.hxx>
#include <comphelper/diagnose_ex.hxx>

using namespace ::com::sun::star;


struct SdrMediaObj::Impl
{
    ::avmedia::MediaItem                  m_MediaProperties;
#if HAVE_FEATURE_AVMEDIA
    // Note: the temp file is read only, until it is deleted!
    // It may be shared between multiple documents in case of copy/paste,
    // hence the shared_ptr.
    std::shared_ptr< ::avmedia::MediaTempFile >  m_pTempFile;
#endif
    uno::Reference< graphic::XGraphic >   m_xCachedSnapshot;
    rtl::Reference<avmedia::PlayerListener> m_xPlayerListener;
    OUString m_LastFailedPkgURL;
};

SdrMediaObj::SdrMediaObj(SdrModel& rSdrModel)
:   SdrRectObj(rSdrModel)
    ,m_xImpl( new Impl )
{
}

SdrMediaObj::SdrMediaObj(SdrModel& rSdrModel, SdrMediaObj const & rSource)
:   SdrRectObj(rSdrModel, rSource)
    ,m_xImpl( new Impl )
{
#if HAVE_FEATURE_AVMEDIA
    m_xImpl->m_pTempFile = rSource.m_xImpl->m_pTempFile; // before props
#endif
    setMediaProperties( rSource.getMediaProperties() );
    m_xImpl->m_xCachedSnapshot = rSource.m_xImpl->m_xCachedSnapshot;
}

SdrMediaObj::SdrMediaObj(
    SdrModel& rSdrModel,
    const tools::Rectangle& rRect)
:   SdrRectObj(rSdrModel, rRect)
    ,m_xImpl( new Impl )
{
    osl_atomic_increment(&m_refCount);

    const bool bUndo(rSdrModel.IsUndoEnabled());
    rSdrModel.EnableUndo(false);
    MakeNameUnique();
    rSdrModel.EnableUndo(bUndo);

    osl_atomic_decrement(&m_refCount);
}

SdrMediaObj::~SdrMediaObj()
{
}

bool SdrMediaObj::HasTextEdit() const
{
    return false;
}

std::unique_ptr<sdr::contact::ViewContact> SdrMediaObj::CreateObjectSpecificViewContact()
{
    return std::make_unique<sdr::contact::ViewContactOfSdrMediaObj>( *this );
}

void SdrMediaObj::TakeObjInfo( SdrObjTransformInfoRec& rInfo ) const
{
    rInfo.bMoveAllowed = true;
    rInfo.bResizeFreeAllowed = true;
    rInfo.bResizePropAllowed = true;
    rInfo.bRotateFreeAllowed = false;
    rInfo.bRotate90Allowed = false;
    rInfo.bMirrorFreeAllowed = false;
    rInfo.bMirror45Allowed = false;
    rInfo.bMirror90Allowed = false;
    rInfo.bTransparenceAllowed = false;
    rInfo.bShearAllowed = false;
    rInfo.bEdgeRadiusAllowed = false;
    rInfo.bNoOrthoDesired = false;
    rInfo.bNoContortion = false;
    rInfo.bCanConvToPath = false;
    rInfo.bCanConvToPoly = false;
    rInfo.bCanConvToContour = false;
    rInfo.bCanConvToPathLineToArea = false;
    rInfo.bCanConvToPolyLineToArea = false;
}

SdrObjKind SdrMediaObj::GetObjIdentifier() const
{
    return SdrObjKind::Media;
}

OUString SdrMediaObj::TakeObjNameSingul() const
{
    OUString sName(SvxResId(STR_ObjNameSingulMEDIA));

    OUString aName(GetName());

    if (!aName.isEmpty())
        sName += " '" + aName + "'";

    return sName;
}

OUString SdrMediaObj::TakeObjNamePlural() const
{
    return SvxResId(STR_ObjNamePluralMEDIA);
}

rtl::Reference<SdrObject> SdrMediaObj::CloneSdrObject(SdrModel& rTargetModel) const
{
    return new SdrMediaObj(rTargetModel, *this);
}

uno::Reference< graphic::XGraphic > const & SdrMediaObj::getSnapshot() const
{
#if HAVE_FEATURE_AVMEDIA
    if( !m_xImpl->m_xCachedSnapshot.is() )
    {
        Graphic aGraphic = m_xImpl->m_MediaProperties.getGraphic();
        if (!aGraphic.IsNone())
        {
            Size aPref = aGraphic.GetPrefSize();
            Size aPixel = aGraphic.GetSizePixel();
            const text::GraphicCrop& rCrop = m_xImpl->m_MediaProperties.getCrop();
            if (rCrop.Bottom > 0 || rCrop.Left > 0 || rCrop.Right > 0 || rCrop.Top > 0)
            {
                tools::Long nLeft = aPixel.getWidth() * rCrop.Left / aPref.getWidth();
                tools::Long nTop = aPixel.getHeight() * rCrop.Top / aPref.getHeight();
                tools::Long nRight = aPixel.getWidth() * rCrop.Right / aPref.getWidth();
                tools::Long nBottom = aPixel.getHeight() * rCrop.Bottom / aPref.getHeight();
                BitmapEx aBitmapEx = aGraphic.GetBitmapEx();
                aBitmapEx.Crop({nLeft, nTop, aPixel.getWidth() - nRight, aPixel.getHeight() - nBottom});
                aGraphic = aBitmapEx;
            }

            // We have an explicit graphic for this media object, then go with that instead of
            // generating our own one.
            m_xImpl->m_xCachedSnapshot = aGraphic.GetXGraphic();
            return m_xImpl->m_xCachedSnapshot;
        }

        OUString aRealURL = m_xImpl->m_MediaProperties.getTempURL();
        if( aRealURL.isEmpty() )
            aRealURL = m_xImpl->m_MediaProperties.getURL();
        OUString sReferer = m_xImpl->m_MediaProperties.getReferer();
        OUString sMimeType = m_xImpl->m_MediaProperties.getMimeType();
        uno::Reference<graphic::XGraphic> xCachedSnapshot = m_xImpl->m_xCachedSnapshot;

        m_xImpl->m_xPlayerListener.set(new avmedia::PlayerListener(
            [this, xCachedSnapshot, aRealURL, sReferer, sMimeType](const css::uno::Reference<css::media::XPlayer>& rPlayer){
                SolarMutexGuard g;
                uno::Reference<graphic::XGraphic> xGraphic
                    = m_xImpl->m_MediaProperties.getGraphic().GetXGraphic();
                m_xImpl->m_xCachedSnapshot = avmedia::MediaWindow::grabFrame(rPlayer, xGraphic);
                ActionChanged();
            }));

        avmedia::MediaWindow::grabFrame(aRealURL, sReferer, sMimeType, m_xImpl->m_xPlayerListener);
    }
#endif
    return m_xImpl->m_xCachedSnapshot;
}

void SdrMediaObj::AdjustToMaxRect( const tools::Rectangle& rMaxRect, bool bShrinkOnly /* = false */ )
{
    Size aSize( Application::GetDefaultDevice()->PixelToLogic(
                    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( GetViewContact() ).getPreferredSize(),
                    MapMode(MapUnit::Map100thMM)) );
    Size aMaxSize( rMaxRect.GetSize() );

    if( aSize.IsEmpty() )
        return;

    Point aPos( rMaxRect.TopLeft() );

    // if graphic is too large, fit it to the page
    if ( (!bShrinkOnly                          ||
         ( aSize.Height() > aMaxSize.Height() ) ||
         ( aSize.Width()  > aMaxSize.Width()  ) )&&
         aSize.Height() && aMaxSize.Height() )
    {
        float fGrfWH =  static_cast<float>(aSize.Width()) /
                        static_cast<float>(aSize.Height());
        float fWinWH =  static_cast<float>(aMaxSize.Width()) /
                        static_cast<float>(aMaxSize.Height());

        // scale graphic to page size
        if ( fGrfWH < fWinWH )
        {
            aSize.setWidth( static_cast<tools::Long>(aMaxSize.Height() * fGrfWH) );
            aSize.setHeight( aMaxSize.Height() );
        }
        else if ( fGrfWH > 0.F )
        {
            aSize.setWidth( aMaxSize.Width() );
            aSize.setHeight( static_cast<tools::Long>(aMaxSize.Width() / fGrfWH) );
        }

        aPos = rMaxRect.Center();
    }

    if( bShrinkOnly )
        aPos = getRectangle().TopLeft();

    aPos.AdjustX( -(aSize.Width() / 2) );
    aPos.AdjustY( -(aSize.Height() / 2) );
    SetLogicRect( tools::Rectangle( aPos, aSize ) );
}

void SdrMediaObj::setURL(const OUString& rURL, const OUString& rReferer)
{
    ::avmedia::MediaItem aURLItem;
#if HAVE_FEATURE_AVMEDIA
    aURLItem.setURL( rURL, u""_ustr, rReferer );
#else
    (void) rURL;
    (void) rReferer;
#endif
    setMediaProperties( aURLItem );
}

const OUString& SdrMediaObj::getURL() const
{
#if HAVE_FEATURE_AVMEDIA
    return m_xImpl->m_MediaProperties.getURL();
#else
    static OUString ret;
    return ret;
#endif
}

const OUString& SdrMediaObj::getTempURL() const
{
#if HAVE_FEATURE_AVMEDIA
    return m_xImpl->m_MediaProperties.getTempURL();
#else
static OUString ret;
    return ret;
#endif
}

void SdrMediaObj::setMediaProperties( const ::avmedia::MediaItem& rState )
{
    mediaPropertiesChanged( rState );
    static_cast< sdr::contact::ViewContactOfSdrMediaObj& >( GetViewContact() ).executeMediaItem( getMediaProperties() );
}

const ::avmedia::MediaItem& SdrMediaObj::getMediaProperties() const
{
    return m_xImpl->m_MediaProperties;
}

uno::Reference<io::XInputStream> SdrMediaObj::GetInputStream() const
{
#if HAVE_FEATURE_AVMEDIA
    if (!m_xImpl->m_pTempFile)
    {
        SAL_WARN("svx", "this is only intended for embedded media");
        return nullptr;
    }
    ucbhelper::Content tempFile(m_xImpl->m_pTempFile->m_TempFileURL,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());
    return tempFile.openStream();
#else
    return nullptr;
#endif
}

void SdrMediaObj::SetInputStream(uno::Reference<io::XInputStream> const& xStream)
{
#if !HAVE_FEATURE_AVMEDIA
    (void) xStream;
#else
    if (m_xImpl->m_pTempFile || m_xImpl->m_LastFailedPkgURL.isEmpty())
    {
        SAL_WARN("svx", "this is only intended for embedded media");
        return;
    }

    OUString tempFileURL;
    const bool bSuccess(
        ::avmedia::CreateMediaTempFile(
            xStream,
            tempFileURL,
            u""));

    if (bSuccess)
    {
        m_xImpl->m_pTempFile = std::make_shared<::avmedia::MediaTempFile>(tempFileURL);
        m_xImpl->m_MediaProperties.setURL(
            m_xImpl->m_LastFailedPkgURL, tempFileURL, u""_ustr);
    }
    m_xImpl->m_LastFailedPkgURL.clear(); // once only
#endif
}

/// copy a stream from XStorage to temp file
#if HAVE_FEATURE_AVMEDIA
static bool lcl_HandlePackageURL(
    OUString const & rURL,
    const SdrModel& rModel,
    OUString & o_rTempFileURL)
{
    ::comphelper::LifecycleProxy sourceProxy;
    uno::Reference<io::XInputStream> xInStream;
    try {
        xInStream = rModel.GetDocumentStream(rURL, sourceProxy);
    }
    catch (container::NoSuchElementException const&)
    {
        SAL_INFO("svx", "not found: '" << rURL << "'");
        return false;
    }
    catch (uno::Exception const&)
    {
        TOOLS_WARN_EXCEPTION("svx", "");
        return false;
    }
    if (!xInStream.is())
    {
        SAL_WARN("svx", "no stream?");
        return false;
    }
    // Make sure the temporary copy has the same file name extension as the original media file
    // (like .mp4). That seems to be important for some AVFoundation APIs. For random extension-less
    // file names, they don't seem to even bother looking inside the file.
    sal_Int32 nLastDot = rURL.lastIndexOf('.');
    sal_Int32 nLastSlash = rURL.lastIndexOf('/');
    OUString sDesiredExtension;
    if (nLastDot > nLastSlash && nLastDot+1 < rURL.getLength())
        sDesiredExtension = rURL.copy(nLastDot);
    return ::avmedia::CreateMediaTempFile(xInStream, o_rTempFileURL, sDesiredExtension);
}
#endif

void SdrMediaObj::mediaPropertiesChanged( const ::avmedia::MediaItem& rNewProperties )
{
    bool bBroadcastChanged = false;
#if HAVE_FEATURE_AVMEDIA
    const AVMediaSetMask nMaskSet = rNewProperties.getMaskSet();

    // use only a subset of MediaItem properties for own properties
    if( AVMediaSetMask::MIME_TYPE & nMaskSet )
        m_xImpl->m_MediaProperties.setMimeType( rNewProperties.getMimeType() );

    if (nMaskSet & AVMediaSetMask::GRAPHIC)
    {
        m_xImpl->m_MediaProperties.setGraphic(rNewProperties.getGraphic());
    }

    if (nMaskSet & AVMediaSetMask::CROP)
    {
        m_xImpl->m_MediaProperties.setCrop(rNewProperties.getCrop());
    }

    if( ( AVMediaSetMask::URL & nMaskSet ) &&
        ( rNewProperties.getURL() != getURL() ))
    {
        m_xImpl->m_xCachedSnapshot.clear();
        m_xImpl->m_xPlayerListener.clear();
        OUString const& url(rNewProperties.getURL());
        if (url.startsWithIgnoreAsciiCase("vnd.sun.star.Package:"))
        {
            if (   !m_xImpl->m_pTempFile
                || (m_xImpl->m_pTempFile->m_TempFileURL !=
                                rNewProperties.getTempURL()))
            {
                OUString tempFileURL;
                const bool bSuccess(
                    lcl_HandlePackageURL(
                        url,
                        getSdrModelFromSdrObject(),
                        tempFileURL));

                if (bSuccess)
                {
                    m_xImpl->m_pTempFile =
                            std::make_shared<::avmedia::MediaTempFile>(tempFileURL);
                    m_xImpl->m_MediaProperties.setURL(url, tempFileURL, u""_ustr);
                }
                else // this case is for Clone via operator=
                {
                    m_xImpl->m_pTempFile.reset();
                    m_xImpl->m_MediaProperties.setURL(u""_ustr, u""_ustr, u""_ustr);
                    // UGLY: oox import also gets here, because unlike ODF
                    // getDocumentStorage() is not the imported file...
                    m_xImpl->m_LastFailedPkgURL = url;
                }
            }
            else
            {
                m_xImpl->m_MediaProperties.setURL(url,
                        rNewProperties.getTempURL(), u""_ustr);
            }
        }
        else
        {
            m_xImpl->m_pTempFile.reset();
            m_xImpl->m_MediaProperties.setURL(url, u""_ustr, rNewProperties.getReferer());
        }
        bBroadcastChanged = true;
    }

    if( AVMediaSetMask::LOOP & nMaskSet )
        m_xImpl->m_MediaProperties.setLoop( rNewProperties.isLoop() );

    if( AVMediaSetMask::MUTE & nMaskSet )
        m_xImpl->m_MediaProperties.setMute( rNewProperties.isMute() );

    if( AVMediaSetMask::VOLUMEDB & nMaskSet )
        m_xImpl->m_MediaProperties.setVolumeDB( rNewProperties.getVolumeDB() );

    if( AVMediaSetMask::ZOOM & nMaskSet )
        m_xImpl->m_MediaProperties.setZoom( rNewProperties.getZoom() );
#else
    (void) rNewProperties;
#endif

    if( bBroadcastChanged )
    {
        SetChanged();
        BroadcastObjectChange();
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
