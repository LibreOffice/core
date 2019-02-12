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
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/media/XManager.hpp>
#include <com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp>
#include <com/sun/star/ui/dialogs/TemplateDescription.hpp>
#include <com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp>
#include <memory>
#include <sal/log.hxx>

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


vcl::Window* MediaWindow::getWindow() const
{
    return mpImpl.get();
}


void MediaWindow::getMediaFilters( FilterNameVector& rFilterNameVector )
{
    static const char* pFilters[] = { "Advanced Audio Coding", "aac",
                                      "AIF Audio", "aif;aiff",
                                      "Advanced Systems Format", "asf;wma;wmv",
                                      "AU Audio", "au",
                                      "AC3 Audio", "ac3",
                                      "AVI", "avi",
                                      "CD Audio", "cda",
                                      "Digital Video", "dv",
                                      "FLAC Audio", "flac",
                                      "Flash Video", "flv",
                                      "Matroska Media", "mkv",
                                      "MIDI Audio", "mid;midi",
                                      "MPEG Audio", "mp2;mp3;mpa;m4a",
                                      "MPEG Video", "mpg;mpeg;mpv;mp4;m4v",
                                      "Ogg Audio", "ogg;oga;opus",
                                      "Ogg Video", "ogv;ogx",
                                      "Real Audio", "ra",
                                      "Real Media", "rm",
                                      "RMI MIDI Audio", "rmi",
                                      "SND (SouND) Audio", "snd",
                                      "Quicktime Video", "mov",
                                      "Vivo Video", "viv",
                                      "WAVE Audio", "wav",
                                      "WebM Video", "webm",
                                      "Windows Media Audio", "wma",
                                      "Windows Media Video", "wmv"};

    for( size_t i = 0; i < SAL_N_ELEMENTS(pFilters); i += 2 )
    {
        rFilterNameVector.push_back( std::make_pair< OUString, OUString >(
                                        OUString::createFromAscii(pFilters[i]),
                                        OUString::createFromAscii(pFilters[i+1]) ) );
    }
}


bool MediaWindow::executeMediaURLDialog(weld::Window* pParent, OUString& rURL, bool *const o_pbLink)
{
    ::sfx2::FileDialogHelper        aDlg(o_pbLink != nullptr
            ? ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW
            : ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE,
            FileDialogFlags::NONE, pParent);
    static const char               aWildcard[] = "*.";
    FilterNameVector                aFilters;
    static const char               aSeparator[] = ";";
    OUStringBuffer                  aAllTypes;

    aDlg.SetTitle( AvmResId( o_pbLink != nullptr
                ? AVMEDIA_STR_INSERTMEDIA_DLG : AVMEDIA_STR_OPENMEDIA_DLG ) );

    getMediaFilters( aFilters );

    for( FilterNameVector::size_type i = 0; i < aFilters.size(); ++i )
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            if( !aAllTypes.isEmpty() )
                aAllTypes.append(aSeparator);

            aAllTypes.append(aWildcard).append(aFilters[ i ].second.getToken( 0, ';', nIndex ));
        }
    }

    // add filter for all media types
    aDlg.AddFilter( AvmResId( AVMEDIA_STR_ALL_MEDIAFILES ), aAllTypes.makeStringAndClear() );

    for( FilterNameVector::size_type i = 0; i < aFilters.size(); ++i )
    {
        OUStringBuffer aTypes;

        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            if( !aTypes.isEmpty() )
                aTypes.append(aSeparator);

            aTypes.append(aWildcard).append(aFilters[ i ].second.getToken( 0, ';', nIndex ));
        }

        // add single filters
        aDlg.AddFilter( aFilters[ i ].first, aTypes.makeStringAndClear() );
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

bool MediaWindow::isMediaURL( const OUString& rURL, const OUString& rReferer, bool bDeep, Size* pPreferredSizePixel )
{
    const INetURLObject aURL( rURL );

    if( aURL.GetProtocol() != INetProtocol::NotValid )
    {
        if( bDeep || pPreferredSizePixel )
        {
            try
            {
                uno::Reference< media::XPlayer > xPlayer( priv::MediaWindowImpl::createPlayer(
                                                            aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ),
                                                            rReferer, nullptr ) );

                if( xPlayer.is() )
                {
                    if( pPreferredSizePixel )
                    {
                        const awt::Size aAwtSize( xPlayer->getPreferredPlayerWindowSize() );

                        pPreferredSizePixel->setWidth( aAwtSize.Width );
                        pPreferredSizePixel->setHeight( aAwtSize.Height );
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
            FilterNameVector        aFilters;
            const OUString          aExt( aURL.getExtension() );

            getMediaFilters( aFilters );

            for( FilterNameVector::size_type i = 0; i < aFilters.size(); ++i )
            {
                for( sal_Int32 nIndex = 0; nIndex >= 0; )
                {
                    if( aExt.equalsIgnoreAsciiCase( aFilters[ i ].second.getToken( 0, ';', nIndex ) ) )
                        return true;
                }
            }
        }
    }

    return false;
}


uno::Reference< media::XPlayer > MediaWindow::createPlayer( const OUString& rURL, const OUString& rReferer, const OUString* pMimeType )
{
    return priv::MediaWindowImpl::createPlayer( rURL, rReferer, pMimeType );
}


uno::Reference< graphic::XGraphic > MediaWindow::grabFrame( const OUString& rURL,
                                                            const OUString& rReferer,
                                                            const OUString& sMimeType )
{
    uno::Reference< media::XPlayer >    xPlayer( createPlayer( rURL, rReferer, &sMimeType ) );
    uno::Reference< graphic::XGraphic > xRet;
    std::unique_ptr< Graphic > xGraphic;

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
                xGraphic.reset( new Graphic( aBmpEx ) );
            }
        }
    }

    if (!xRet.is() && !xGraphic)
    {
        const BitmapEx aBmpEx(AVMEDIA_BMP_EMPTYLOGO);
        xGraphic.reset( new Graphic( aBmpEx ) );
    }

    if (xGraphic)
        xRet = xGraphic->GetXGraphic();

    return xRet;
}


} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
