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

#ifndef INCLUDED_SLIDESHOW_SOUNDPLAYER_HXX
#define INCLUDED_SLIDESHOW_SOUNDPLAYER_HXX

#include <rtl/ustring.hxx>

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/media/XManager.hpp>
#include <com/sun/star/media/XPlayer.hpp>

#include <boost/shared_ptr.hpp>

#include "pauseeventhandler.hxx"
#include "disposable.hxx"
#include "eventmultiplexer.hxx"


/* Definition of SoundPlayer class */

namespace slideshow
{
    namespace internal
    {
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

                @throws ::com::sun::star::lang::NoSupportException, if
                the sound file is invalid, or not supported by the
                player service.
             */
            static ::boost::shared_ptr<SoundPlayer> create(
                EventMultiplexer & rEventMultiplexer,
                const ::rtl::OUString& rSoundURL,
                const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext>& rComponentContext );

            virtual ~SoundPlayer();

            /** Query duration of sound playback.

                If the sound is already playing, this method
                returns the remaining playback time.

                @return the playback duration in seconds.
             */
            double getDuration() const;

            bool startPlayback();
            bool stopPlayback();

            void setPlaybackLoop( bool bLoop );

            // PauseEventHandler:
            virtual bool handlePause( bool bPauseShow );

            // Disposable
            virtual void dispose();

        private:
            SoundPlayer(
                EventMultiplexer & rEventMultiplexer,
                const ::rtl::OUString& rSoundURL,
                const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext>& rComponentContext );

            EventMultiplexer & mrEventMultiplexer;
            // TODO(Q3): obsolete when boost::enable_shared_ptr_from_this
            //           is available
            ::boost::shared_ptr<SoundPlayer> mThis;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > mxPlayer;
        };

        typedef ::boost::shared_ptr< SoundPlayer > SoundPlayerSharedPtr;
    }
}

#endif /* INCLUDED_SLIDESHOW_SOUNDPLAYER_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
