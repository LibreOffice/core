/*************************************************************************
 *
 *  $RCSfile: mediawindowbase_impl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: ka $ $Date: 2004-08-23 09:04:42 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _AVMEDIA_MEDIAWINDOWBASE_IMPL_HXX
#define _AVMEDIA_MEDIAWINDOWBASE_IMPL_HXX

#include "mediawindow.hxx"

#ifndef _COM_SUN_STAR_MEDIA_XPLAYER_HPP_
#include <com/sun/star/media/XPlayer.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XPLAYERWINDOW_HPP_
#include <com/sun/star/media/XPlayerWindow.hpp>
#endif

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

            static ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > createPlayer( const ::rtl::OUString& rURL );

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

            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer > getPlayer() const;

            void setPlayerWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow >& rxPlayerWindow );
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow > getPlayerWindow() const;

        private:

            ::rtl::OUString                                                             maFileURL;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayer >        mxPlayer;
            ::com::sun::star::uno::Reference< ::com::sun::star::media::XPlayerWindow >  mxPlayerWindow;
            MediaWindow*                                                                mpMediaWindow;
        };
    }
}

#endif
