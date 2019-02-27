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


#include <tools/diagnose_ex.h>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <sal/log.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XComponent.hpp>

#include <tools/urlobj.hxx>

#include <avmedia/mediawindow.hxx>
#include <mediafilemanager.hxx>
#include <soundplayer.hxx>

#include <algorithm>

using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        // TODO(Q3): Move the whole SoundPlayer class to avmedia.

        std::shared_ptr<SoundPlayer> SoundPlayer::create(
            EventMultiplexer & rEventMultiplexer,
            const OUString& rSoundURL,
            const uno::Reference< uno::XComponentContext>&  rComponentContext,
            MediaFileManager& rMediaFileManager)
        {
            std::shared_ptr<SoundPlayer> pPlayer(
                new SoundPlayer( rEventMultiplexer,
                                 rSoundURL,
                                 rComponentContext,
                                 rMediaFileManager) );
            rEventMultiplexer.addPauseHandler( pPlayer );
            pPlayer->mThis = pPlayer;
            return pPlayer;
        }

        bool SoundPlayer::handlePause( bool bPauseShow )
        {
            return bPauseShow ? stopPlayback() : startPlayback();
        }

        void SoundPlayer::dispose()
        {
            if( mThis )
            {
                mrEventMultiplexer.removePauseHandler( mThis );
                mThis.reset();
            }

            if( mxPlayer.is() )
            {
                mxPlayer->stop();
                uno::Reference<lang::XComponent> xComponent(
                    mxPlayer, uno::UNO_QUERY );
                if( xComponent.is() )
                    xComponent->dispose();
                mxPlayer.clear();
            }
        }

        SoundPlayer::SoundPlayer(
            EventMultiplexer & rEventMultiplexer,
            const OUString& rSoundURL,
            const uno::Reference< uno::XComponentContext>&  rComponentContext,
            MediaFileManager& rMediaFileManager)
            : mrEventMultiplexer(rEventMultiplexer),
              mThis(),
              mxPlayer()
        {
            ENSURE_OR_THROW( rComponentContext.is(),
                              "SoundPlayer::SoundPlayer(): Invalid component context" );

            try
            {
                if (rSoundURL.startsWithIgnoreAsciiCase("vnd.sun.star.Package:"))
                {
                    mpMediaTempFile = rMediaFileManager.getMediaTempFile(rSoundURL);
                }
                const INetURLObject aURL( mpMediaTempFile ? mpMediaTempFile->m_TempFileURL : rSoundURL );
                mxPlayer.set( avmedia::MediaWindow::createPlayer(
                                aURL.GetMainURL( INetURLObject::DecodeMechanism::Unambiguous ), ""/*TODO!*/ ),
                                uno::UNO_QUERY);
            }
            catch( uno::RuntimeException& )
            {
                throw;
            }
            catch( uno::Exception& )
            {
            }

            if( !mxPlayer.is() )
                throw lang::NoSupportException(
                    "No sound support for " + rSoundURL );
        }

        SoundPlayer::~SoundPlayer()
        {
            try
            {
                dispose();
            }
            catch (uno::Exception &) {
                SAL_WARN( "slideshow", exceptionToString( cppu::getCaughtException() ) );
            }
        }

        double SoundPlayer::getDuration() const
        {
            if( !mxPlayer.is() )
                return 0.0;

            const double nDuration( mxPlayer->getDuration() );
            if( mxPlayer->isPlaying() )
                return ::std::max( 0.0,
                                   nDuration - mxPlayer->getMediaTime() );
            else
                return nDuration;
        }

        bool SoundPlayer::startPlayback()
        {
            if( !mxPlayer.is() )
                return false;

            if( mxPlayer->isPlaying() )
                mxPlayer->stop();

            mxPlayer->start();
            return true;
        }

        bool SoundPlayer::stopPlayback()
        {
            if( mxPlayer.is() )
                mxPlayer->stop();

            return true;
        }

        void SoundPlayer::setPlaybackLoop( bool bLoop )
        {
            if( mxPlayer.is() )
                mxPlayer->setPlaybackLoop( bLoop );
        }

        bool SoundPlayer::isPlaying() const
        {
            return mxPlayer->isPlaying();
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
