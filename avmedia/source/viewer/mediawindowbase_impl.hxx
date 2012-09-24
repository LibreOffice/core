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

#ifndef AVMEDIA_MEDIAWINDOWBASE_IMPL_HXX
#define AVMEDIA_MEDIAWINDOWBASE_IMPL_HXX

#include <avmedia/mediawindow.hxx>

#include <com/sun/star/media/XPlayer.hpp>
#include <com/sun/star/media/XPlayerWindow.hpp>


namespace avmedia
{
    namespace priv
    {
        // --------------
        // - UpdateMode -
        // --------------

        enum UpdateMode
        {
            UPDATEMODE_SYNC_STATUSBAR = 0,
            UPDATEMODE_SYNC_PLAYER = 1,
            UPDATEMODE_SYNC_NONE = 2
        };

        // -----------------------
        // - MediaWindowBaseImpl -
        // -----------------------

        class MediaWindowBaseImpl
        {
        public:

                            MediaWindowBaseImpl( MediaWindow* pMediaWindow );
            virtual         ~MediaWindowBaseImpl();

            virtual void    cleanUp();
            virtual void    onURLChanged();

            static ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > createPlayer( const OUString& rURL );

            void    setURL( const OUString& rURL, OUString const& rTempURL );

            const   OUString&  getURL() const;

            bool    isValid() const;

            Size    getPreferredSize() const;

            bool    setZoom( ::com::sun::star::media::ZoomLevel eLevel );
            ::com::sun::star::media::ZoomLevel getZoom() const;

            bool    start();
            void    stop();

            bool    isPlaying() const;

            double  getDuration() const;

            void    setMediaTime( double fTime );
            double  getMediaTime() const;

            double  getRate() const;

            void    setPlaybackLoop( bool bSet );
            bool    isPlaybackLoop() const;

            void    setFixedAspectRatio( bool bSet );
            bool    isFixedAspectRatio() const;

            void    setMute( bool bSet );
            bool    isMute() const;

            void    setVolumeDB( sal_Int16 nVolumeDB );
            sal_Int16 getVolumeDB() const;

            void    updateMediaItem( MediaItem& rItem ) const;
            void    executeMediaItem( const MediaItem& rItem );

        protected:

            void    stopPlayingInternal( bool );

            MediaWindow* getMediaWindow() const;
            inline sal_Bool isMediaWindowJavaBased() const { return( mbIsMediaWindowJavaBased ); }

            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > getPlayer() const;

            void setPlayerWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow >& rxPlayerWindow );
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow > getPlayerWindow() const;

        private:
            OUString                                                                    maFileURL;
            OUString                                                                    mTempFileURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer >        mxPlayer;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow >  mxPlayerWindow;
            MediaWindow*                                                                mpMediaWindow;
            sal_Bool                                                                    mbIsMediaWindowJavaBased;
        };
    }
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
