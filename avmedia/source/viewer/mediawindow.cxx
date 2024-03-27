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

#include <avmedia/mediawindow.hxx>
#include "mediawindow_impl.hxx"
#include <mediamisc.hxx>
#include <bitmaps.hlst>
#include <strings.hrc>
#include <tools/urlobj.hxx>
#include <utility>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/frame/XDispatchHelper.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/media/XPlayerNotifier.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePicker3.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <memory>
#include <sal/log.hxx>
#include <o3tl/string_view.hxx>

#define AVMEDIA_FRAMEGRABBER_DEFAULTFRAME_MEDIATIME 3.0

using namespace ::com::sun::star;

namespace avmedia {

MediaWindow::MediaWindow( vcl::Window* parent, bool bInternalMediaControl ) :
    mpImpl( VclPtr<priv::MediaWindowImpl>::Create( parent, this, bInternalMediaControl ) )
{
    mpImpl->Show();
}


MediaWindow::~MediaWindow()
{
    mpImpl.disposeAndClear();
}


void MediaWindow::setURL( const OUString& rURL, const OUString& rReferer )
{
    mpImpl->setURL( rURL, OUString(), rReferer );
}


const OUString& MediaWindow::getURL() const
{
    return mpImpl->getURL();
}


bool MediaWindow::isValid() const
{
    return mpImpl->isValid();
}


void MediaWindow::MouseMove( const MouseEvent& )
{
}


void MediaWindow::MouseButtonDown( const MouseEvent& )
{
}


void MediaWindow::MouseButtonUp( const MouseEvent& )
{
}


void MediaWindow::KeyInput( const KeyEvent& )
{
}


void MediaWindow::KeyUp( const KeyEvent& )
{
}

void MediaWindow::Command( const CommandEvent& )
{
}


sal_Int8 MediaWindow::AcceptDrop( const AcceptDropEvent& )
{
    return 0;
}


sal_Int8 MediaWindow::ExecuteDrop( const ExecuteDropEvent& )
{
    return 0;
}


void MediaWindow::StartDrag( sal_Int8, const Point& )
{
}


Size MediaWindow::getPreferredSize() const
{
    return mpImpl->getPreferredSize();
}


void MediaWindow::setPosSize( const tools::Rectangle& rNewRect )
{
    mpImpl->setPosSize( rNewRect );
}


void MediaWindow::setPointer( PointerStyle nPointer )
{
    mpImpl->setPointer( nPointer );
}


bool MediaWindow::start()
{
    return mpImpl->start();
}

void MediaWindow::updateMediaItem( MediaItem& rItem ) const
{
    mpImpl->updateMediaItem( rItem );
}

void MediaWindow::executeMediaItem( const MediaItem& rItem )
{
    mpImpl->executeMediaItem( rItem );
}

void MediaWindow::show()
{
    mpImpl->Show();
}

void MediaWindow::hide()
{
    mpImpl->Hide();
}

bool MediaWindow::isVisible() const
{
    return mpImpl->IsVisible();
}

vcl::Window* MediaWindow::getWindow() const
{
    return mpImpl.get();
}


FilterNameVector MediaWindow::getMediaFilters()
{
    return {{"Advanced Audio Coding", "aac"},
            {"AIF Audio", "aif;aiff"},
            {"Advanced Systems Format", "asf;wma;wmv"},
            {"AU Audio", "au"},
            {"AC3 Audio", "ac3"},
            {"AVI", "avi"},
            {"CD Audio", "cda"},
            {"Digital Video", "dv"},
            {"FLAC Audio", "flac"},
            {"Flash Video", "flv"},
            {"Matroska Media", "mkv"},
            {"MIDI Audio", "mid;midi"},
            {"MPEG Audio", "mp2;mp3;mpa;m4a"},
            {"MPEG Video", "mpg;mpeg;mpv;mp4;m4v"},
            {"Ogg Audio", "ogg;oga;opus"},
            {"Ogg Video", "ogv;ogx"},
            {"Real Audio", "ra"},
            {"Real Media", "rm"},
            {"RMI MIDI Audio", "rmi"},
            {"SND (SouND) Audio", "snd"},
            {"Quicktime Video", "mov"},
            {"Vivo Video", "viv"},
            {"WAVE Audio", "wav"},
            {"WebM Video", "webm"},
            {"Windows Media Audio", "wma"},
            {"Windows Media Video", "wmv"}};
}


bool MediaWindow::executeMediaURLDialog(weld::Window* pParent, OUString& rURL, bool *const o_pbLink)
{
    ::sfx2::FileDialogHelper        aDlg(o_pbLink != nullptr
            ? ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW
            : ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            FileDialogFlags::NONE, pParent);
    static const char               aWildcard[] = "*.";
    FilterNameVector                aFilters = getMediaFilters();
    static const char               aSeparator[] = ";";
    OUStringBuffer                  aAllTypes;

    aDlg.SetContext(sfx2::FileDialogHelper::InsertMedia);
    aDlg.SetTitle( AvmResId( o_pbLink != nullptr
                ? AVMEDIA_STR_INSERTMEDIA_DLG : AVMEDIA_STR_OPENMEDIA_DLG ) );

    for( const auto &filter : aFilters )
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            if( !aAllTypes.isEmpty() )
                aAllTypes.append(aSeparator);

            aAllTypes.append(OUString::Concat(aWildcard) + o3tl::getToken(filter.second, 0, ';', nIndex ));
        }
    }

    // add filter for all media types
    aDlg.AddFilter( AvmResId( AVMEDIA_STR_ALL_MEDIAFILES ), aAllTypes.makeStringAndClear() );

    for( const auto &filter : aFilters )
    {
        OUStringBuffer aTypes;

        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            if( !aTypes.isEmpty() )
                aTypes.append(aSeparator);

            aTypes.append(OUString::Concat(aWildcard) + o3tl::getToken(filter.second, 0, ';', nIndex ));
        }

        // add single filters
        aDlg.AddFilter( filter.first, aTypes.makeStringAndClear() );
    }

    // add filter for all types
    aDlg.AddFilter( AvmResId( AVMEDIA_STR_ALL_FILES ), "*.*" );

    uno::Reference<ui::dialogs::XFilePicker3> const xFP(aDlg.GetFilePicker());
    uno::Reference<ui::dialogs::XFilePickerControlAccess> const xCtrlAcc(xFP,
            uno::UNO_QUERY_THROW);
    if (o_pbLink != nullptr)
    {
        // for video link should be the default
        xCtrlAcc->setValue(
                ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0,
                uno::Any(true) );
        // disabled for now: TODO: preview?
        xCtrlAcc->enableControl(
                ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW,
                false);
    }

    if( aDlg.Execute() == ERRCODE_NONE )
    {
        const INetURLObject aURL( aDlg.GetPath() );
        rURL = aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous );

        if (o_pbLink != nullptr)
        {
            uno::Any const any = xCtrlAcc->getValue(
                ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0);
            if (!(any >>= *o_pbLink))
            {
                SAL_WARN("avmedia", "invalid link property");
                *o_pbLink = true;
            }
        }
    }
    else if( !rURL.isEmpty() )
        rURL.clear();

    return !rURL.isEmpty();
}

void MediaWindow::executeFormatErrorBox(weld::Window* pParent)
{
    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(pParent,
                                              VclMessageType::Warning, VclButtonsType::Ok, AvmResId(AVMEDIA_STR_ERR_URL)));
    xBox->run();
}

bool MediaWindow::isMediaURL(std::u16string_view rURL, const OUString& rReferer, bool bDeep, const rtl::Reference<PlayerListener>& xPreferredPixelSizeListener)
{
    const INetURLObject aURL( rURL );

    if( aURL.GetProtocol() == INetProtocol::NotValid )
        return false;

    if (bDeep || xPreferredPixelSizeListener)
    {
        try
        {
            uno::Reference< media::XPlayer > xPlayer( priv::MediaWindowImpl::createPlayer(
                                                        aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ),
                                                        rReferer, nullptr ) );

            if( xPlayer.is() )
            {
                if (xPreferredPixelSizeListener)
                {
                    uno::Reference<media::XPlayerNotifier> xPlayerNotifier(xPlayer, css::uno::UNO_QUERY);
                    if (xPlayerNotifier)
                    {
                        // wait until it's possible to query this to get a sensible answer
                        xPreferredPixelSizeListener->startListening(xPlayerNotifier);
                    }
                    else
                    {
                        // assume the size is possible to query immediately
                        xPreferredPixelSizeListener->callPlayerWindowSizeAvailable(xPlayer);
                    }
                }
                return true;
            }
        }
        catch( ... )
        {
        }
    }
    else
    {
        FilterNameVector        aFilters = getMediaFilters();
        const OUString          aExt( aURL.getExtension() );

        for( const auto &filter : aFilters )
        {
            for( sal_Int32 nIndex = 0; nIndex >= 0; )
            {
                if( aExt.equalsIgnoreAsciiCase( o3tl::getToken(filter.second, 0, ';', nIndex ) ) )
                    return true;
            }
        }
    }

    return false;
}

uno::Reference< media::XPlayer > MediaWindow::createPlayer( const OUString& rURL, const OUString& rReferer, const OUString* pMimeType )
{
    return priv::MediaWindowImpl::createPlayer( rURL, rReferer, pMimeType );
}

uno::Reference<graphic::XGraphic>
MediaWindow::grabFrame(const uno::Reference<media::XPlayer>& xPlayer,
                       const uno::Reference<graphic::XGraphic>& rGraphic)
{
    uno::Reference< graphic::XGraphic > xRet;
    std::optional< Graphic > oGraphic;

    if( xPlayer.is() )
    {
        uno::Reference< media::XFrameGrabber > xGrabber( xPlayer->createFrameGrabber() );

        if( xGrabber.is() )
        {
            double fMediaTime = AVMEDIA_FRAMEGRABBER_DEFAULTFRAME_MEDIATIME;

            if( fMediaTime >= xPlayer->getDuration() )
                fMediaTime = ( xPlayer->getDuration() * 0.5 );

            xRet = xGrabber->grabFrame( fMediaTime );
        }

        if( !xRet.is() )
        {
            awt::Size aPrefSize( xPlayer->getPreferredPlayerWindowSize() );

            if( !aPrefSize.Width && !aPrefSize.Height )
            {
                const BitmapEx aBmpEx(AVMEDIA_BMP_AUDIOLOGO);
                oGraphic.emplace( aBmpEx );
            }
        }
    }

    if (!xRet.is() && !oGraphic)
    {
        const BitmapEx aBmpEx(AVMEDIA_BMP_EMPTYLOGO);
        oGraphic.emplace( aBmpEx );
    }

    if (oGraphic)
    {
        if (rGraphic)
            oGraphic.emplace(rGraphic);
        xRet = oGraphic->GetXGraphic();
    }

    return xRet;
}

uno::Reference< graphic::XGraphic > MediaWindow::grabFrame(const OUString& rURL,
                                                           const OUString& rReferer,
                                                           const OUString& sMimeType,
                                                           const rtl::Reference<PlayerListener>& xPreferredPixelSizeListener)
{
    uno::Reference<media::XPlayer> xPlayer(createPlayer(rURL, rReferer, &sMimeType));

    if (xPreferredPixelSizeListener)
    {
        uno::Reference<media::XPlayerNotifier> xPlayerNotifier(xPlayer, css::uno::UNO_QUERY);
        if (xPlayerNotifier)
        {
            // set a callback to call when a more sensible result is available, which
            // might be called immediately if already available
            xPreferredPixelSizeListener->startListening(xPlayerNotifier);
        }
        else
        {
            // assume the size is possible to query immediately
            xPreferredPixelSizeListener->callPlayerWindowSizeAvailable(xPlayer);
        }

        return nullptr;
    }

    return grabFrame(xPlayer);
}

void MediaWindow::dispatchInsertAVMedia(const css::uno::Reference<css::frame::XDispatchProvider>& rDispatchProvider,
                                        const css::awt::Size& rSize, const OUString& rURL, bool bLink)
{
    util::URL aDispatchURL;
    aDispatchURL.Complete = ".uno:InsertAVMedia";

    css::uno::Reference<css::util::XURLTransformer> xTrans(css::util::URLTransformer::create(::comphelper::getProcessComponentContext()));
    xTrans->parseStrict(aDispatchURL);

    css::uno::Reference<css::frame::XDispatch> xDispatch = rDispatchProvider->queryDispatch(aDispatchURL, "", 0);
    css::uno::Sequence<css::beans::PropertyValue> aArgs(comphelper::InitPropertySequence({
        { "URL", css::uno::Any(rURL) },
        { "Size.Width", uno::Any(rSize.Width)},
        { "Size.Height", uno::Any(rSize.Height)},
        { "IsLink", css::uno::Any(bLink) },
    }));
    xDispatch->dispatch(aDispatchURL, aArgs);
}

PlayerListener::PlayerListener(std::function<void(const css::uno::Reference<css::media::XPlayer>&)> fn)
    : m_aFn(std::move(fn))
{
}

void PlayerListener::disposing(std::unique_lock<std::mutex>& rGuard)
{
    stopListening(rGuard);
    WeakComponentImplHelperBase::disposing(rGuard);
}

void PlayerListener::startListening(const css::uno::Reference<media::XPlayerNotifier>& rNotifier)
{
    std::unique_lock aGuard(m_aMutex);

    m_xNotifier = rNotifier;
    m_xNotifier->addPlayerListener(this);
}

void PlayerListener::stopListening()
{
    std::unique_lock aGuard(m_aMutex);
    stopListening(aGuard);
}

void PlayerListener::stopListening(std::unique_lock<std::mutex>&)
{
    if (!m_xNotifier)
        return;
    m_xNotifier->removePlayerListener(this);
    m_xNotifier.clear();
}

void SAL_CALL PlayerListener::preferredPlayerWindowSizeAvailable(const css::lang::EventObject&)
{
    std::unique_lock aGuard(m_aMutex);

    css::uno::Reference<media::XPlayer> xPlayer(m_xNotifier, css::uno::UNO_QUERY_THROW);
    aGuard.unlock();
    callPlayerWindowSizeAvailable(xPlayer);
    aGuard.lock();

    stopListening(aGuard);
}

void SAL_CALL PlayerListener::disposing(const css::lang::EventObject&)
{
}

PlayerListener::~PlayerListener()
{
}

} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
