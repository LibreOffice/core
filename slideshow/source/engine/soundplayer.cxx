/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: soundplayer.cxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-07 17:02:53 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_slideshow.hxx"

#include <canvas/debug.hxx>
#include <canvas/verbosetrace.hxx>

#include <comphelper/anytostring.hxx>
#include <cppuhelper/exc_hlp.hxx>

#include <com/sun/star/lang/XMultiComponentFactory.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/lang/XComponent.hdl>

#include <tools/urlobj.hxx>

#include <avmedia/mediawindow.hxx>

#include "soundplayer.hxx"

#include <algorithm>

using namespace ::com::sun::star;


namespace slideshow
{
    namespace internal
    {
        // TODO(Q3): Move the whole SoundPlayer class to avmedia.

        boost::shared_ptr<SoundPlayer> SoundPlayer::create(
            EventMultiplexer & rEventMultiplexer,
            const ::rtl::OUString& rSoundURL,
            const uno::Reference< uno::XComponentContext>&  rComponentContext )
        {
            boost::shared_ptr<SoundPlayer> pPlayer(
                new SoundPlayer( rEventMultiplexer,
                                 rSoundURL,
                                 rComponentContext ) );
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
            const ::rtl::OUString& rSoundURL,
            const uno::Reference< uno::XComponentContext>&  rComponentContext )
            : mrEventMultiplexer(rEventMultiplexer),
              mThis(),
              mxPlayer()
        {
            ENSURE_AND_THROW( rComponentContext.is(),
                              "SoundPlayer::SoundPlayer(): Invalid component context" );

            try
            {
                const INetURLObject aURL( rSoundURL );
                mxPlayer.set( avmedia::MediaWindow::createPlayer(
                                aURL.GetMainURL( INetURLObject::DECODE_UNAMBIGUOUS ) ),
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
                    rtl::OUString( RTL_CONSTASCII_USTRINGPARAM(
                                       "No sound support for ") ) + rSoundURL,
                    uno::Reference<uno::XInterface>() );
        }

        SoundPlayer::~SoundPlayer()
        {
            try
            {
                dispose();
            }
            catch (uno::Exception &) {
                OSL_ENSURE( false, rtl::OUStringToOString(
                                comphelper::anyToString(
                                    cppu::getCaughtException() ),
                                RTL_TEXTENCODING_UTF8 ).getStr() );
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
    }
}
