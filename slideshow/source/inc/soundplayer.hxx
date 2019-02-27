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

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_SOUNDPLAYER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_SOUNDPLAYER_HXX

#include <rtl/ustring.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/media/XManager.hpp>
#include <com/sun/star/media/XPlayer.hpp>
#include <avmedia/mediaitem.hxx>

#include <memory>

#include "pauseeventhandler.hxx"
#include "disposable.hxx"
#include "eventmultiplexer.hxx"


/* Definition of SoundPlayer class */

namespace slideshow
{
    namespace internal
    {
        class MediaFileManager;

        /** Little class that plays a sound from a URL.
            TODO:
            Must be explicitly disposed (as long as enable_shared_ptr_from_this
            isn't available)!
         */
        class SoundPlayer : public PauseEventHandler,
                            public Disposable
        {
        public:
            /** Create a sound player object.

                @param rSoundURL
                URL to a sound file.

                @param rComponentContext
                Reference to a component context, used to create the
                needed services

                @throws css::lang::NoSupportException, if
                the sound file is invalid, or not supported by the
                player service.
             */
            static ::std::shared_ptr<SoundPlayer> create(
                EventMultiplexer & rEventMultiplexer,
                const OUString& rSoundURL,
                const css::uno::Reference< css::uno::XComponentContext>& rComponentContext,
                MediaFileManager& rMediaFileManager);

            virtual ~SoundPlayer() override;

            /** Query duration of sound playback.

                If the sound is already playing, this method
                returns the remaining playback time.

                @return the playback duration in seconds.
             */
            double getDuration() const;

            bool startPlayback();
            bool stopPlayback();
            bool isPlaying() const;

            void setPlaybackLoop( bool bLoop );

            // PauseEventHandler:
            virtual bool handlePause( bool bPauseShow ) override;

            // Disposable
            virtual void dispose() override;

        private:
            SoundPlayer(
                EventMultiplexer & rEventMultiplexer,
                const OUString& rSoundURL,
                const css::uno::Reference< css::uno::XComponentContext>& rComponentContext,
                MediaFileManager & rMediaFileManager);

            EventMultiplexer & mrEventMultiplexer;
            // TODO(Q3): obsolete when boost::enable_shared_ptr_from_this
            //           is available
            ::std::shared_ptr<SoundPlayer> mThis;
            // Temp file for pakcage url.
            ::std::shared_ptr<::avmedia::MediaTempFile> mpMediaTempFile;
            css::uno::Reference< css::media::XPlayer > mxPlayer;
        };

        typedef ::std::shared_ptr< SoundPlayer > SoundPlayerSharedPtr;
    }
}

#endif // INCLUDED_SLIDESHOW_SOURCE_INC_SOUNDPLAYER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
