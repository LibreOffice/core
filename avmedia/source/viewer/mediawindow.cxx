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

#include <stdio.h>

#include <avmedia/mediawindow.hxx>
#include "mediawindow_impl.hxx"
#include "mediamisc.hxx"
#include "mediawindow.hrc"
#include <tools/urlobj.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/pathoptions.hxx>
#include <sfx2/filedlghelper.hxx>
#include <comphelper/processfactory.hxx>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/media/XManager.hpp>
#include "com/sun/star/ui/dialogs/ExtendedFilePickerElementIds.hpp"
#include "com/sun/star/ui/dialogs/TemplateDescription.hpp"
#include "com/sun/star/ui/dialogs/XFilePickerControlAccess.hpp"

#define AVMEDIA_FRAMEGRABBER_DEFAULTFRAME_MEDIATIME 3.0

using namespace ::com::sun::star;

namespace avmedia {

// ---------------
// - MediaWindow -
// ---------------

MediaWindow::MediaWindow( Window* parent, bool bInternalMediaControl ) :
    mpImpl( new priv::MediaWindowImpl( parent, this, bInternalMediaControl ) )
{
    mpImpl->Show();
}

// -------------------------------------------------------------------------

MediaWindow::~MediaWindow()
{
    mpImpl->cleanUp();
    delete mpImpl;
    mpImpl = NULL;
}

// -------------------------------------------------------------------------

void MediaWindow::setURL( const OUString& rURL )
{
    if( mpImpl )
        mpImpl->setURL( rURL, OUString() );
}

// -------------------------------------------------------------------------

const OUString& MediaWindow::getURL() const
{
    return mpImpl->getURL();
}

// -------------------------------------------------------------------------

bool MediaWindow::isValid() const
{
    return( mpImpl != NULL && mpImpl->isValid() );
}

// -------------------------------------------------------------------------

void MediaWindow::MouseMove( const MouseEvent& /* rMEvt */ )
{
}

// ---------------------------------------------------------------------

void MediaWindow::MouseButtonDown( const MouseEvent& /* rMEvt */ )
{
}

// ---------------------------------------------------------------------

void MediaWindow::MouseButtonUp( const MouseEvent& /* rMEvt */ )
{
}

// -------------------------------------------------------------------------

void MediaWindow::KeyInput( const KeyEvent& /* rKEvt */ )
{
}

// -------------------------------------------------------------------------

void MediaWindow::KeyUp( const KeyEvent& /* rKEvt */ )
{
}

// -------------------------------------------------------------------------

void MediaWindow::Command( const CommandEvent& /* rCEvt */ )
{
}

// -------------------------------------------------------------------------

sal_Int8 MediaWindow::AcceptDrop( const AcceptDropEvent& /* rEvt */ )
{
    return 0;
}

// -------------------------------------------------------------------------

sal_Int8 MediaWindow::ExecuteDrop( const ExecuteDropEvent& /* rEvt */ )
{
    return 0;
}

// -------------------------------------------------------------------------

void MediaWindow::StartDrag( sal_Int8 /* nAction */, const Point& /* rPosPixel */ )
{
}

// -------------------------------------------------------------------------

Size MediaWindow::getPreferredSize() const
{
    return mpImpl->getPreferredSize();
}

// -------------------------------------------------------------------------

void MediaWindow::setPosSize( const Rectangle& rNewRect )
{
    if( mpImpl )
    {
        mpImpl->setPosSize( rNewRect );
    }
}

// -------------------------------------------------------------------------

void MediaWindow::setPointer( const Pointer& rPointer )
{
    if( mpImpl )
        mpImpl->setPointer( rPointer );
}

// -------------------------------------------------------------------------

bool MediaWindow::start()
{
    return( mpImpl != NULL && mpImpl->start() );
}

// -------------------------------------------------------------------------

void MediaWindow::updateMediaItem( MediaItem& rItem ) const
{
    if( mpImpl )
        mpImpl->updateMediaItem( rItem );
}

// -------------------------------------------------------------------------

void MediaWindow::executeMediaItem( const MediaItem& rItem )
{
    if( mpImpl )
        mpImpl->executeMediaItem( rItem );
}

// -------------------------------------------------------------------------

void MediaWindow::show()
{
    if( mpImpl )
        mpImpl->Show();
}

// -------------------------------------------------------------------------

void MediaWindow::hide()
{
    if( mpImpl )
        mpImpl->Hide();
}

// -------------------------------------------------------------------------

Window* MediaWindow::getWindow() const
{
    return mpImpl;
}

// -------------------------------------------------------------------------

void MediaWindow::getMediaFilters( FilterNameVector& rFilterNameVector )
{
    static const char* pFilters[] = { "AIF Audio", "aif;aiff",
                                      "AU Audio", "au",
                                      "AVI", "avi",
                                      "CD Audio", "cda",
                                      "FLAC Audio", "flac",
                                      "Matroska Media", "mkv",
                                      "MIDI Audio", "mid;midi",
                                      "MPEG Audio", "mp2;mp3;mpa",
                                      "MPEG Video", "mpg;mpeg;mpv;mp4",
                                      "Ogg bitstream", "ogg",
                                      "Quicktime Video", "mov",
                                      "Vivo Video", "viv",
                                      "WAVE Audio", "wav",
                                      "WebM Video", "webm" };

    for( size_t i = 0; i < SAL_N_ELEMENTS(pFilters); i += 2 )
    {
        rFilterNameVector.push_back( ::std::make_pair< OUString, OUString >(
                                        OUString::createFromAscii(pFilters[i]),
                                        OUString::createFromAscii(pFilters[i+1]) ) );
    }
}

// -------------------------------------------------------------------------

bool MediaWindow::executeMediaURLDialog(Window* /* pParent */,
        OUString& rURL, bool *const o_pbLink)
{
    ::sfx2::FileDialogHelper        aDlg( (o_pbLink)
            ? ui::dialogs::TemplateDescription::FILEOPEN_LINK_PREVIEW
            : ui::dialogs::TemplateDescription::FILEOPEN_SIMPLE, 0 );
    static const OUString           aWildcard( "*."  );
    FilterNameVector                aFilters;
    const OUString                  aSeparator( ";"  );
    OUString                        aAllTypes;

    aDlg.SetTitle( AVMEDIA_RESID( (o_pbLink)
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
    aDlg.AddFilter( AVMEDIA_RESID( AVMEDIA_STR_ALL_MEDIAFILES ), aAllTypes );

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
    aDlg.AddFilter( AVMEDIA_RESID( AVMEDIA_STR_ALL_FILES ), String( "*.*"  ) );

    uno::Reference<ui::dialogs::XFilePicker> const xFP(aDlg.GetFilePicker());
    uno::Reference<ui::dialogs::XFilePickerControlAccess> const xCtrlAcc(xFP,
            uno::UNO_QUERY_THROW);
    if (o_pbLink)
    {
        // for video link should be the default
        xCtrlAcc->setValue(
                ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_LINK, 0,
                uno::makeAny(sal_True) );
        // disabled for now: TODO: preview?
        xCtrlAcc->enableControl(
                ui::dialogs::ExtendedFilePickerElementIds::CHECKBOX_PREVIEW,
                sal_False);
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
        rURL = OUString();

    return( !rURL.isEmpty() );
}

// -------------------------------------------------------------------------

void MediaWindow::executeFormatErrorBox( Window* pParent )
{
    ErrorBox aErrBox( pParent, AVMEDIA_RESID( AVMEDIA_ERR_URL ) );

    aErrBox.Execute();
}

// -------------------------------------------------------------------------

bool MediaWindow::isMediaURL( const OUString& rURL, bool bDeep, Size* pPreferredSizePixel )
{
    const INetURLObject aURL( rURL );
    bool                bRet = false;

    if( aURL.GetProtocol() != INET_PROT_NOT_VALID )
    {
        if( bDeep || pPreferredSizePixel )
        {
            try
            {
                uno::Reference< media::XPlayer > xPlayer( priv::MediaWindowImpl::createPlayer(
                                                            aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) ) );

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

// -------------------------------------------------------------------------

uno::Reference< media::XPlayer > MediaWindow::createPlayer( const OUString& rURL )
{
    return priv::MediaWindowImpl::createPlayer( rURL );
}

// -------------------------------------------------------------------------

uno::Reference< graphic::XGraphic > MediaWindow::grabFrame( const OUString& rURL,
                                                            bool bAllowToCreateReplacementGraphic,
                                                            double fMediaTime )
{
    uno::Reference< media::XPlayer >    xPlayer( createPlayer( rURL ) );
    uno::Reference< graphic::XGraphic > xRet;
    ::std::auto_ptr< Graphic >          apGraphic;

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

        if( !xRet.is() && bAllowToCreateReplacementGraphic  )
        {
            awt::Size aPrefSize( xPlayer->getPreferredPlayerWindowSize() );

            if( !aPrefSize.Width && !aPrefSize.Height )
            {
                const BitmapEx aBmpEx( AVMEDIA_RESID( AVMEDIA_BMP_AUDIOLOGO ) );
                apGraphic.reset( new Graphic( aBmpEx ) );
            }
        }
    }

    if( !xRet.is() && !apGraphic.get() && bAllowToCreateReplacementGraphic )
    {
        const BitmapEx aBmpEx( AVMEDIA_RESID( AVMEDIA_BMP_EMPTYLOGO ) );
        apGraphic.reset( new Graphic( aBmpEx ) );
    }

    if( apGraphic.get() )
        xRet = apGraphic->GetXGraphic();

    return xRet;
}

} // namespace avemdia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
