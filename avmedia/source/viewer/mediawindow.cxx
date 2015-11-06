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
#include "mediamisc.hxx"
#include "mediawindow.hrc"
#include <tools/urlobj.hxx>
#include <vcl/layout.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/media/XManager.hpp>
#include "com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp"
#include <memory>

#define AVMEDIA_FRAMEGRABBER_DEFAULTFRAME_MEDIATIME 3.0

using namespace ::com::sun::star;

namespace avmedia {


// - MediaWindow -


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



void MediaWindow::MouseMove( const MouseEvent& /* rMEvt */ )
{
}



void MediaWindow::MouseButtonDown( const MouseEvent& /* rMEvt */ )
{
}



void MediaWindow::MouseButtonUp( const MouseEvent& /* rMEvt */ )
{
}



void MediaWindow::KeyInput( const KeyEvent& /* rKEvt */ )
{
}



void MediaWindow::KeyUp( const KeyEvent& /* rKEvt */ )
{
}



void MediaWindow::Command( const CommandEvent& /* rCEvt */ )
{
}



sal_Int8 MediaWindow::AcceptDrop( const AcceptDropEvent& /* rEvt */ )
{
    return 0;
}



sal_Int8 MediaWindow::ExecuteDrop( const ExecuteDropEvent& /* rEvt */ )
{
    return 0;
}



void MediaWindow::StartDrag( sal_Int8 /* nAction */, const Point& /* rPosPixel */ )
{
}



Size MediaWindow::getPreferredSize() const
{
    return mpImpl->getPreferredSize();
}



void MediaWindow::setPosSize( const Rectangle& rNewRect )
{
    mpImpl->setPosSize( rNewRect );
}



void MediaWindow::setPointer( const Pointer& rPointer )
{
    mpImpl->setPointer( rPointer );
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
        rFilterNameVector.push_back( ::std::make_pair< OUString, OUString >(
                                        OUString::createFromAscii(pFilters[i]),
                                        OUString::createFromAscii(pFilters[i+1]) ) );
    }
}



bool MediaWindow::executeMediaURLDialog(vcl::Window* /* pParent */,
        OUString& rURL, bool *const o_pbLink)
{
    ::sfx2::FileDialogHelper        aDlg( (o_pbLink)
            ? ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW
            : ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
    static const char               aWildcard[] = "*.";
    FilterNameVector                aFilters;
    static const char               aSeparator[] = ";";
    OUString                        aAllTypes;

    aDlg.SetTitle( AVMEDIA_RESSTR( (o_pbLink)
                ? AVMEDIA_STR_INSERTMEDIA_DLG : AVMEDIA_STR_OPENMEDIA_DLG ) );

    getMediaFilters( aFilters );

    unsigned int i;
    for( i = 0; i < aFilters.size(); ++i )
    {
        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            if( !aAllTypes.isEmpty() )
                aAllTypes += aSeparator;

            ( aAllTypes += aWildcard ) += aFilters[ i ].second.getToken( 0, ';', nIndex );
        }
    }

    // add filter for all media types
    aDlg.AddFilter( AVMEDIA_RESSTR( AVMEDIA_STR_ALL_MEDIAFILES ), aAllTypes );

    for( i = 0; i < aFilters.size(); ++i )
    {
        OUString aTypes;

        for( sal_Int32 nIndex = 0; nIndex >= 0; )
        {
            if( !aTypes.isEmpty() )
                aTypes += aSeparator;

            ( aTypes += aWildcard ) += aFilters[ i ].second.getToken( 0, ';', nIndex );
        }

        // add single filters
        aDlg.AddFilter( aFilters[ i ].first, aTypes );
    }

    // add filter for all types
    aDlg.AddFilter( AVMEDIA_RESSTR( AVMEDIA_STR_ALL_FILES ), "*.*" );

    uno::Reference<ui::dialogs::XFilePicker2> const xFP(aDlg.GetFilePicker());
    uno::Reference<ui::dialogs::XFilePickerControlAccess> const xCtrlAcc(xFP,
            uno::UNO_QUERY_THROW);
    if (o_pbLink)
    {
        // for video link should be the default
        xCtrlAcc->setValue(
                ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0,
                uno::makeAny(true) );
        // disabled for now: TODO: preview?
        xCtrlAcc->enableControl(
                ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW,
                false);
    }

    if( aDlg.Execute() == ERRCODE_NONE )
    {
        const INetURLObject aURL( aDlg.GetPath() );
        rURL = aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS );

        if (o_pbLink)
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



void MediaWindow::executeFormatErrorBox( vcl::Window* pParent )
{
    ScopedVclPtrInstance< MessageDialog > aErrBox( pParent, AVMEDIA_RESID( AVMEDIA_STR_ERR_URL ) );

    aErrBox->Execute();
    aErrBox.disposeAndClear();
}



bool MediaWindow::isMediaURL( const OUString& rURL, const OUString& rReferer, bool bDeep, Size* pPreferredSizePixel )
{
    const INetURLObject aURL( rURL );
    bool                bRet = false;

    if( aURL.GetProtocol() != INetProtocol::NotValid )
    {
        if( bDeep || pPreferredSizePixel )
        {
            try
            {
                uno::Reference< media::XPlayer > xPlayer( priv::MediaWindowImpl::createPlayer(
                                                            aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ),
                                                            rReferer ) );

                if( xPlayer.is() )
                {
                    bRet = true;

                    if( pPreferredSizePixel )
                    {
                        const awt::Size aAwtSize( xPlayer->getPreferredPlayerWindowSize() );

                        pPreferredSizePixel->Width() = aAwtSize.Width;
                        pPreferredSizePixel->Height() = aAwtSize.Height;
                    }
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

            unsigned int i;
            for( i = 0; ( i < aFilters.size() ) && !bRet; ++i )
            {
                for( sal_Int32 nIndex = 0; nIndex >= 0 && !bRet; )
                {
                    if( aExt.equalsIgnoreAsciiCase( aFilters[ i ].second.getToken( 0, ';', nIndex ) ) )
                        bRet = true;
                }
            }
        }
    }

    return bRet;
}



uno::Reference< media::XPlayer > MediaWindow::createPlayer( const OUString& rURL, const OUString& rReferer, const OUString* pMimeType )
{
    return priv::MediaWindowImpl::createPlayer( rURL, rReferer, pMimeType );
}



uno::Reference< graphic::XGraphic > MediaWindow::grabFrame( const OUString& rURL,
                                                            const OUString& rReferer,
                                                            const OUString& sMimeType,
                                                            double fMediaTime )
{
    uno::Reference< media::XPlayer >    xPlayer( createPlayer( rURL, rReferer, &sMimeType ) );
    uno::Reference< graphic::XGraphic > xRet;
    std::unique_ptr< Graphic > xGraphic;

    if( xPlayer.is() )
    {
        uno::Reference< media::XFrameGrabber > xGrabber( xPlayer->createFrameGrabber() );

        if( xGrabber.is() )
        {
            if( AVMEDIA_FRAMEGRABBER_DEFAULTFRAME == fMediaTime )
                fMediaTime = AVMEDIA_FRAMEGRABBER_DEFAULTFRAME_MEDIATIME;

            if( fMediaTime >= xPlayer->getDuration() )
                fMediaTime = ( xPlayer->getDuration() * 0.5 );

            xRet = xGrabber->grabFrame( fMediaTime );
        }

        if( !xRet.is() )
        {
            awt::Size aPrefSize( xPlayer->getPreferredPlayerWindowSize() );

            if( !aPrefSize.Width && !aPrefSize.Height )
            {
                const BitmapEx aBmpEx( getAudioLogo() );
                xGraphic.reset( new Graphic( aBmpEx ) );
            }
        }
    }

    if( !xRet.is() && !xGraphic.get() )
    {
        const BitmapEx aBmpEx( getEmptyLogo() );
        xGraphic.reset( new Graphic( aBmpEx ) );
    }

    if( xGraphic.get() )
        xRet = xGraphic->GetXGraphic();

    return xRet;
}

BitmapEx MediaWindow::getAudioLogo()
{
    return BitmapEx(AVMEDIA_RESID(AVMEDIA_BMP_AUDIOLOGO));
}

BitmapEx MediaWindow::getEmptyLogo()
{
    return BitmapEx(AVMEDIA_RESID(AVMEDIA_BMP_EMPTYLOGO));
}


} // namespace avemdia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
