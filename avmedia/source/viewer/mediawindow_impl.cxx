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

#include <iostream>
#include "mediawindow_impl.hxx"
#include "mediaevent_impl.hxx"
#include <mediamisc.hxx>
#include <bitmaps.hlst>
#include <helpids.h>

#include <algorithm>
#include <string_view>

#include <sal/log.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <unotools/securityoptions.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/sysdata.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <vcl/ptrstyle.hxx>

#include <com/sun/star/awt/SystemPointer.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/media/XManager.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
using namespace ::com::sun::star;

namespace avmedia::priv {

MediaWindowControl::MediaWindowControl(vcl::Window* pParent)
    : MediaControl(pParent, MediaControlStyle::MultiLine)
{
}

void MediaWindowControl::update()
{
    MediaItem aItem;

    static_cast< MediaWindowImpl* >( GetParent() )->updateMediaItem( aItem );
    setState(aItem);
}

void MediaWindowControl::execute(const MediaItem& rItem)
{
    static_cast<MediaWindowImpl*>(GetParent())->executeMediaItem(rItem);
}

MediaChildWindow::MediaChildWindow(vcl::Window* pParent)
    : SystemChildWindow(pParent, WB_CLIPCHILDREN)
{
}

void MediaChildWindow::MouseMove( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                          rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseMove( rMEvt );
    GetParent()->MouseMove( aTransformedEvent );
}

void MediaChildWindow::MouseButtonDown( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                          rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseButtonDown( rMEvt );
    GetParent()->MouseButtonDown( aTransformedEvent );
}

void MediaChildWindow::MouseButtonUp( const MouseEvent& rMEvt )
{
    const MouseEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rMEvt.GetPosPixel() ) ),
                                          rMEvt.GetClicks(), rMEvt.GetMode(), rMEvt.GetButtons(), rMEvt.GetModifier() );

    SystemChildWindow::MouseButtonUp( rMEvt );
    GetParent()->MouseButtonUp( aTransformedEvent );
}

void MediaChildWindow::KeyInput( const KeyEvent& rKEvt )
{
    SystemChildWindow::KeyInput( rKEvt );
    GetParent()->KeyInput( rKEvt );
}

void MediaChildWindow::KeyUp( const KeyEvent& rKEvt )
{
    SystemChildWindow::KeyUp( rKEvt );
    GetParent()->KeyUp( rKEvt );
}

void MediaChildWindow::Command( const CommandEvent& rCEvt )
{
    const CommandEvent aTransformedEvent( GetParent()->ScreenToOutputPixel( OutputToScreenPixel( rCEvt.GetMousePosPixel() ) ),
                                          rCEvt.GetCommand(), rCEvt.IsMouseEvent(), rCEvt.GetEventData() );

    SystemChildWindow::Command( rCEvt );
    GetParent()->Command( aTransformedEvent );
}

MediaWindowImpl::MediaWindowImpl(vcl::Window* pParent, MediaWindow* pMediaWindow, bool bInternalMediaControl)
    : Control(pParent)
    , DropTargetHelper(this)
    , DragSourceHelper(this)
    , mpMediaWindow(pMediaWindow)
    , mpMediaWindowControl(bInternalMediaControl ? VclPtr<MediaWindowControl>::Create(this) : nullptr)
{
    if (mpMediaWindowControl)
    {
        mpMediaWindowControl->SetSizePixel(mpMediaWindowControl->GetOptimalSize());
        mpMediaWindowControl->Show();
    }
}

MediaWindowImpl::~MediaWindowImpl()
{
    disposeOnce();
}

void MediaWindowImpl::dispose()
{
    if (mxEvents.is())
        mxEvents->cleanUp();

    if (mxPlayerWindow.is())
    {
        auto pEventsIf = static_cast<cppu::OWeakObject*>(mxEvents.get());
        mxPlayerWindow->removeKeyListener( uno::Reference< awt::XKeyListener >( pEventsIf, uno::UNO_QUERY ) );
        mxPlayerWindow->removeMouseListener( uno::Reference< awt::XMouseListener >( pEventsIf, uno::UNO_QUERY ) );
        mxPlayerWindow->removeMouseMotionListener( uno::Reference< awt::XMouseMotionListener >( pEventsIf, uno::UNO_QUERY ) );
        mxPlayerWindow->dispose();
        mxPlayerWindow.clear();
    }

    uno::Reference< lang::XComponent > xComponent( mxPlayer, uno::UNO_QUERY );
    if (xComponent.is()) // this stops the player
        xComponent->dispose();

    mxPlayer.clear();

    mpMediaWindow = nullptr;

    mpEmptyBmpEx.reset();
    mpAudioBmpEx.reset();
    mpMediaWindowControl.disposeAndClear();
    mpChildWindow.disposeAndClear();

    Control::dispose();
}

uno::Reference<media::XPlayer> MediaWindowImpl::createPlayer(const OUString& rURL, const OUString& rReferer, const OUString* pMimeType)
{
    uno::Reference<media::XPlayer> xPlayer;

    if( rURL.isEmpty() )
        return xPlayer;

    if (SvtSecurityOptions().isUntrustedReferer(rReferer))
    {
        return xPlayer;
    }

    if (!pMimeType || *pMimeType == AVMEDIA_MIMETYPE_COMMON)
    {
        uno::Reference<uno::XComponentContext> xContext(::comphelper::getProcessComponentContext());

        static std::u16string_view aServiceManagers[] =
        {
            u"" AVMEDIA_MANAGER_SERVICE_PREFERRED,
            u"" AVMEDIA_MANAGER_SERVICE_NAME,
        };

        for (const auto& rServiceName : aServiceManagers)
        {
            xPlayer = createPlayer(rURL, rServiceName, xContext);
            if (xPlayer)
                break;
        }
    }

    return xPlayer;
}

uno::Reference< media::XPlayer > MediaWindowImpl::createPlayer(
    const OUString& rURL, const OUString& rManagerServName,
    const uno::Reference< uno::XComponentContext >& xContext)
{
    uno::Reference< media::XPlayer > xPlayer;
    try
    {
        uno::Reference< media::XManager > xManager (
            xContext->getServiceManager()->createInstanceWithContext(rManagerServName, xContext),
            uno::UNO_QUERY );
        if( xManager.is() )
            xPlayer = xManager->createPlayer( rURL );
        else
            SAL_INFO( "avmedia", "failed to create media player service " << rManagerServName );
    } catch ( const uno::Exception & )
    {
        TOOLS_WARN_EXCEPTION( "avmedia", "couldn't create media player " << rManagerServName);
    }
    return xPlayer;
}

void MediaWindowImpl::setURL( const OUString& rURL,
        OUString const& rTempURL, OUString const& rReferer)
{
    maReferer = rReferer;
    if( rURL == getURL() )
        return;

    if( mxPlayer.is() )
        mxPlayer->stop();

    if( mxPlayerWindow.is() )
    {
        mxPlayerWindow->setVisible( false );
        mxPlayerWindow.clear();
    }

    mxPlayer.clear();
    mTempFileURL.clear();

    if (!rTempURL.isEmpty())
    {
        maFileURL = rURL;
        mTempFileURL = rTempURL;
    }
    else
    {
        INetURLObject aURL( rURL );

        if (aURL.GetProtocol() != INetProtocol::NotValid)
            maFileURL = aURL.GetMainURL(INetURLObject::DecodeMechanism::Unambiguous);
        else
            maFileURL = rURL;
    }

    mxPlayer = createPlayer((!mTempFileURL.isEmpty()) ? mTempFileURL : maFileURL, rReferer, &m_sMimeType );
    onURLChanged();
}

const OUString& MediaWindowImpl::getURL() const
{
    return maFileURL;
}

bool MediaWindowImpl::isValid() const
{
    return mxPlayer.is();
}

Size MediaWindowImpl::getPreferredSize() const
{
    Size aRet(480, 360);

    if( mxPlayer.is() )
    {
        awt::Size aPrefSize( mxPlayer->getPreferredPlayerWindowSize() );

        aRet.setWidth( aPrefSize.Width );
        aRet.setHeight( aPrefSize.Height );
    }

    return aRet;
}

bool MediaWindowImpl::start()
{
    return mxPlayer.is() && ( mxPlayer->start(), true );
}

void MediaWindowImpl::updateMediaItem( MediaItem& rItem ) const
{
    if( isPlaying() )
        rItem.setState( MediaState::Play );
    else
        rItem.setState( ( getMediaTime() == 0.0 ) ? MediaState::Stop : MediaState::Pause );

    rItem.setDuration( getDuration() );
    rItem.setTime( getMediaTime() );
    rItem.setLoop( mxPlayer.is() && mxPlayer->isPlaybackLoop() );
    rItem.setMute( mxPlayer.is() && mxPlayer->isMute() );
    rItem.setVolumeDB( mxPlayer.is() ? mxPlayer->getVolumeDB() : 0 );
    rItem.setZoom( mxPlayerWindow.is() ? mxPlayerWindow->getZoomLevel() : media::ZoomLevel_NOT_AVAILABLE );
    rItem.setURL( getURL(), mTempFileURL, maReferer );
}

void MediaWindowImpl::executeMediaItem( const MediaItem& rItem )
{
    const AVMediaSetMask nMaskSet = rItem.getMaskSet();

    // set URL first
    if (nMaskSet & AVMediaSetMask::URL)
    {
        m_sMimeType = rItem.getMimeType();
        setURL(rItem.getURL(), rItem.getTempURL(), rItem.getReferer());
    }

    // set different states next
    if (nMaskSet & AVMediaSetMask::TIME)
        setMediaTime(std::min(rItem.getTime(), getDuration()));

    if (nMaskSet & AVMediaSetMask::LOOP && mxPlayer.is() )
        mxPlayer->setPlaybackLoop( rItem.isLoop() );

    if (nMaskSet & AVMediaSetMask::MUTE && mxPlayer.is() )
        mxPlayer->setMute( rItem.isMute() );

    if (nMaskSet & AVMediaSetMask::VOLUMEDB && mxPlayer.is() )
        mxPlayer->setVolumeDB( rItem.getVolumeDB() );

    if (nMaskSet & AVMediaSetMask::ZOOM && mxPlayerWindow.is() )
        mxPlayerWindow->setZoomLevel( rItem.getZoom() );

    // set play state at last
    if (!(nMaskSet & AVMediaSetMask::STATE))
        return;

    switch (rItem.getState())
    {
        case MediaState::Play:
        {
            if (!isPlaying())
                start();
        }
        break;

        case MediaState::Pause:
        {
            if (isPlaying())
                stop();
        }
        break;

        case MediaState::Stop:
        {
            if (isPlaying())
            {
                setMediaTime( 0.0 );
                stop();
                setMediaTime( 0.0 );
            }
        }
        break;
    }
}

void MediaWindowImpl::stop()
{
    if( mxPlayer.is() )
        mxPlayer->stop();
}

bool MediaWindowImpl::isPlaying() const
{
    return( mxPlayer.is() && mxPlayer->isPlaying() );
}

double MediaWindowImpl::getDuration() const
{
    return( mxPlayer.is() ? mxPlayer->getDuration() : 0.0 );
}

void MediaWindowImpl::setMediaTime( double fTime )
{
    if( mxPlayer.is() )
        mxPlayer->setMediaTime( fTime );
}

double MediaWindowImpl::getMediaTime() const
{
    return( mxPlayer.is() ? mxPlayer->getMediaTime() : 0.0 );
}

void MediaWindowImpl::stopPlayingInternal(bool bStop)
{
    if (isPlaying())
    {
        bStop ? mxPlayer->stop() : mxPlayer->start();
    }
}

void MediaWindowImpl::onURLChanged()
{
    if (m_sMimeType == AVMEDIA_MIMETYPE_COMMON)
    {
        mpChildWindow.disposeAndClear();
        mpChildWindow.reset(VclPtr<MediaChildWindow>::Create(this));
    }
    if (!mpChildWindow)
        return;
    mpChildWindow->SetHelpId(HID_AVMEDIA_PLAYERWINDOW);
    mxEvents = new MediaEventListenersImpl(*mpChildWindow);

    if (mxPlayer.is())
    {
        Resize();
        uno::Sequence<uno::Any> aArgs( 3 );
        uno::Reference<media::XPlayerWindow> xPlayerWindow;
        const Point aPoint;
        const Size aSize(mpChildWindow->GetSizePixel());

        sal_IntPtr nParentWindowHandle(0);
        const SystemEnvData* pEnvData = mpChildWindow->GetSystemData();
        // tdf#139609 gtk doesn't need the handle, and fetching it is undesirable
        if (!pEnvData || pEnvData->toolkit != SystemEnvData::Toolkit::Gtk3)
            nParentWindowHandle = mpChildWindow->GetParentWindowHandle();
        aArgs[0] <<= nParentWindowHandle;
        aArgs[1] <<= awt::Rectangle(aPoint.X(), aPoint.Y(), aSize.Width(), aSize.Height());
        aArgs[2] <<= reinterpret_cast<sal_IntPtr>(mpChildWindow.get());

        try
        {
            xPlayerWindow = mxPlayer->createPlayerWindow( aArgs );
        }
        catch( const uno::RuntimeException& )
        {
            // happens eg, on MacOSX where Java frames cannot be created from X11 window handles
        }

        mxPlayerWindow = xPlayerWindow;

        if( xPlayerWindow.is() )
        {
            auto pEventsIf = static_cast<cppu::OWeakObject*>(mxEvents.get());
            xPlayerWindow->addKeyListener( uno::Reference< awt::XKeyListener >( pEventsIf, uno::UNO_QUERY ) );
            xPlayerWindow->addMouseListener( uno::Reference< awt::XMouseListener >( pEventsIf, uno::UNO_QUERY ) );
            xPlayerWindow->addMouseMotionListener( uno::Reference< awt::XMouseMotionListener >( pEventsIf, uno::UNO_QUERY ) );
            xPlayerWindow->addFocusListener( uno::Reference< awt::XFocusListener >( pEventsIf, uno::UNO_QUERY ) );
        }
    }
    else
        mxPlayerWindow.clear();

    if( mxPlayerWindow.is() )
        mpChildWindow->Show();
    else
        mpChildWindow->Hide();

    if( mpMediaWindowControl )
    {
        MediaItem aItem;

        updateMediaItem( aItem );
        mpMediaWindowControl->setState( aItem );
    }
}

void MediaWindowImpl::setPosSize(const tools::Rectangle& rRect)
{
    SetPosSizePixel(rRect.TopLeft(), rRect.GetSize());
}

void MediaWindowImpl::setPointer(PointerStyle aPointer)
{
    SetPointer(aPointer);

    if (mpChildWindow)
        mpChildWindow->SetPointer(aPointer);

    if (!mxPlayerWindow.is())
        return;

    sal_Int32 nPointer;

    switch (aPointer)
    {
        case PointerStyle::Cross:
            nPointer = awt::SystemPointer::CROSS;
            break;
        case PointerStyle::Hand:
            nPointer = awt::SystemPointer::HAND;
            break;
        case PointerStyle::Move:
            nPointer = awt::SystemPointer::MOVE;
            break;
        case PointerStyle::Wait:
            nPointer = awt::SystemPointer::WAIT;
            break;
        default:
            nPointer = awt::SystemPointer::ARROW;
            break;
    }

    mxPlayerWindow->setPointerType(nPointer);
}

void MediaWindowImpl::Resize()
{
    const Size aCurSize(GetOutputSizePixel());
    const sal_Int32 nOffset(mpMediaWindowControl ? AVMEDIA_CONTROLOFFSET : 0);

    Size aPlayerWindowSize(aCurSize.Width() - (nOffset << 1),
                           aCurSize.Height() - (nOffset << 1));

    if (mpMediaWindowControl)
    {
        const sal_Int32 nControlHeight = mpMediaWindowControl->GetSizePixel().Height();
        const sal_Int32 nControlY = std::max(aCurSize.Height() - nControlHeight - nOffset, tools::Long(0));

        aPlayerWindowSize.setHeight( nControlY - (nOffset << 1) );
        mpMediaWindowControl->SetPosSizePixel(Point(nOffset, nControlY ), Size(aCurSize.Width() - (nOffset << 1), nControlHeight));
    }
    if (mpChildWindow)
        mpChildWindow->SetPosSizePixel(Point(0, 0), aPlayerWindowSize);

    if (mxPlayerWindow.is())
        mxPlayerWindow->setPosSize(0, 0, aPlayerWindowSize.Width(), aPlayerWindowSize.Height(), 0);
}

void MediaWindowImpl::StateChanged(StateChangedType eType)
{
    if (!mxPlayerWindow.is())
        return;

    // stop playing when going disabled or hidden
    switch (eType)
    {
        case StateChangedType::Visible:
        {
            stopPlayingInternal(!IsVisible());
            mxPlayerWindow->setVisible(IsVisible());
        }
        break;

        case StateChangedType::Enable:
        {
            stopPlayingInternal(!IsEnabled());
            mxPlayerWindow->setEnable(IsEnabled());
        }
        break;

        default:
        break;
    }
}

void MediaWindowImpl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    if (mxPlayerWindow.is())
        mxPlayerWindow->update();

    BitmapEx* pLogo = nullptr;

    if (!mxPlayer.is())
    {
        if (!mpEmptyBmpEx)
            mpEmptyBmpEx.reset(new BitmapEx(AVMEDIA_BMP_EMPTYLOGO));

        pLogo = mpEmptyBmpEx.get();
    }
    else if (!mxPlayerWindow.is())
    {
        if (!mpAudioBmpEx)
            mpAudioBmpEx.reset(new BitmapEx(AVMEDIA_BMP_AUDIOLOGO));

        pLogo = mpAudioBmpEx.get();
    }

    if (!mpChildWindow)
        return;

    const Point aBasePos(mpChildWindow->GetPosPixel());
    const tools::Rectangle aVideoRect(aBasePos, mpChildWindow->GetSizePixel());

    if (!pLogo || pLogo->IsEmpty() || aVideoRect.IsEmpty())
        return;

    Size aLogoSize(pLogo->GetSizePixel());
    const Color aBackgroundColor(67, 67, 67);

    rRenderContext.SetLineColor(aBackgroundColor);
    rRenderContext.SetFillColor(aBackgroundColor);
    rRenderContext.DrawRect(aVideoRect);

    if ((aLogoSize.Width() > aVideoRect.GetWidth() || aLogoSize.Height() > aVideoRect.GetHeight() ) &&
        (aLogoSize.Height() > 0))
    {
        const double fLogoWH = double(aLogoSize.Width()) / aLogoSize.Height();

        if (fLogoWH < (double(aVideoRect.GetWidth()) / aVideoRect.GetHeight()))
        {
            aLogoSize.setWidth( tools::Long(aVideoRect.GetHeight() * fLogoWH) );
            aLogoSize.setHeight( aVideoRect.GetHeight() );
        }
        else
        {
            aLogoSize.setWidth( aVideoRect.GetWidth() );
            aLogoSize.setHeight( tools::Long(aVideoRect.GetWidth() / fLogoWH) );
        }
    }

    Point aPoint(aBasePos.X() + ((aVideoRect.GetWidth() - aLogoSize.Width()) >> 1),
                 aBasePos.Y() + ((aVideoRect.GetHeight() - aLogoSize.Height()) >> 1));

    rRenderContext.DrawBitmapEx(aPoint, aLogoSize, *pLogo);
}

void MediaWindowImpl::GetFocus()
{
}

void MediaWindowImpl::MouseMove(const MouseEvent& rMEvt)
{
    if (mpMediaWindow)
        mpMediaWindow->MouseMove(rMEvt);
}

void MediaWindowImpl::MouseButtonDown(const MouseEvent& rMEvt)
{
    if (mpMediaWindow)
        mpMediaWindow->MouseButtonDown(rMEvt);
}

void MediaWindowImpl::MouseButtonUp(const MouseEvent& rMEvt)
{
    if (mpMediaWindow)
        mpMediaWindow->MouseButtonUp(rMEvt);
}

void MediaWindowImpl::KeyInput(const KeyEvent& rKEvt)
{
    if (mpMediaWindow)
        mpMediaWindow->KeyInput(rKEvt);
}

void MediaWindowImpl::KeyUp(const KeyEvent& rKEvt)
{
    if (mpMediaWindow)
        mpMediaWindow->KeyUp(rKEvt);
}

void MediaWindowImpl::Command(const CommandEvent& rCEvt)
{
    if (mpMediaWindow)
        mpMediaWindow->Command(rCEvt);
}

sal_Int8 MediaWindowImpl::AcceptDrop(const AcceptDropEvent& rEvt)
{
    return (mpMediaWindow ? mpMediaWindow->AcceptDrop(rEvt) : 0);
}

sal_Int8 MediaWindowImpl::ExecuteDrop(const ExecuteDropEvent& rEvt)
{
    return (mpMediaWindow ? mpMediaWindow->ExecuteDrop(rEvt) : 0);
}

void MediaWindowImpl::StartDrag(sal_Int8 nAction, const Point& rPosPixel)
{
    if (mpMediaWindow)
        mpMediaWindow->StartDrag(nAction, rPosPixel);
}

} // namespace

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
