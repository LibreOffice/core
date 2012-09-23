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

            static ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > createPlayer( const OUString& rURL);

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
