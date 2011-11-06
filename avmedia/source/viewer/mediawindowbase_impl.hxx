/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _AVMEDIA_MEDIAWINDOWBASE_IMPL_HXX
#define _AVMEDIA_MEDIAWINDOWBASE_IMPL_HXX

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

            static ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > createPlayer( const ::rtl::OUString& rURL,
                                                                                                      sal_Bool& rbJavaBased );

        public:

            void    setURL( const ::rtl::OUString& rURL );
            const ::rtl::OUString&  getURL() const;

            bool    isValid() const;

            bool    hasPreferredSize() const;
            Size    getPreferredSize() const;

            bool    setZoom( ::com::sun::star::media::ZoomLevel eLevel );
            ::com::sun::star::media::ZoomLevel getZoom() const;

            bool    start();
            void    stop();

            bool    isPlaying() const;

            double  getDuration() const;

            void    setMediaTime( double fTime );
            double  getMediaTime() const;

            void    setStopTime( double fTime );
            double  getStopTime() const;

            void    setRate( double fRate );
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

            ::rtl::OUString                                                             maFileURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer >        mxPlayer;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow >  mxPlayerWindow;
            MediaWindow*                                                                mpMediaWindow;
            sal_Bool                                                                    mbIsMediaWindowJavaBased;
        };
    }
}

#endif
