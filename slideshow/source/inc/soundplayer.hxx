/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: soundplayer.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:21:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _SLIDESHOW_SOUNDPLAYER_HXX
#define _SLIDESHOW_SOUNDPLAYER_HXX

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HXX_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XMANAGER_HPP_
#include <com/sun/star/media/XManager.hpp>
#endif
#ifndef _COM_SUN_STAR_MEDIA_XPLAYER_HPP_
#include <com/sun/star/media/XPlayer.hpp>
#endif

#ifndef BOOST_SHARED_PTR_HPP_INCLUDED
#include <boost/shared_ptr.hpp>
#endif

#include "pauseeventhandler.hxx"
#include "eventmultiplexer.hxx"


/* Definition of SoundPlayer class */

namespace presentation
{
    namespace internal
    {
        /** Little class that plays a sound from a URL.
            TODO:
            Must be explicitly disposed (as long as enable_shared_ptr_from_this
            isn't available)!
         */
        class SoundPlayer : public PauseEventHandler
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

            // PauseEventHandler:
            virtual bool handlePause( bool bPauseShow );
            // Disposable:
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

#endif /* _SLIDESHOW_SOUNDPLAYER_HXX */
