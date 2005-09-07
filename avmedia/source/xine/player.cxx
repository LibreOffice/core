/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: player.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:48:09 $
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

#include "player.hxx"
#include "window.hxx"

using namespace ::com::sun::star;

namespace avmedia { namespace xine {

// ----------------
// - Player -
// ----------------

Player::Player()
{
}

// ------------------------------------------------------------------------------

Player::~Player()
{
}

// ------------------------------------------------------------------------------

bool Player::create( const ::rtl::OUString& rURL )
{
    bool bRet = false;


    return bRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::start(  )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::stop(  )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaying()
    throw (uno::RuntimeException)
{
    bool bRet = false;

    return bRet;
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getDuration(  )
    throw (uno::RuntimeException)
{
    double fRet = 0.0;

    return fRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMediaTime( double fTime )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getMediaTime(  )
    throw (uno::RuntimeException)
{
    double fRet = 0.0;

    return fRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setStopTime( double fTime )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getStopTime(  )
    throw (uno::RuntimeException)
{
    double fRet = 0.0;

    return fRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setRate( double fRate )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

double SAL_CALL Player::getRate(  )
    throw (uno::RuntimeException)
{
    double fRet = 0.0;

    return fRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setPlaybackLoop( sal_Bool bSet )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isPlaybackLoop(  )
    throw (uno::RuntimeException)
{
    bool bRet = false;

    return bRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setMute( sal_Bool bSet )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::isMute(  )
    throw (uno::RuntimeException)
{
    bool bRet = false;

    return bRet;
}

// ------------------------------------------------------------------------------

void SAL_CALL Player::setVolumeDB( sal_Int16 nVolumeDB )
    throw (uno::RuntimeException)
{
}

// ------------------------------------------------------------------------------

sal_Int16 SAL_CALL Player::getVolumeDB(  )
    throw (uno::RuntimeException)
{
    sal_Int16 nRet = 0;

    return nRet;
}

// ------------------------------------------------------------------------------

awt::Size SAL_CALL Player::getPreferredPlayerWindowSize(  )
    throw (uno::RuntimeException)
{
    awt::Size aSize( 0, 0 );

    return aSize;
}

// ------------------------------------------------------------------------------

uno::Reference< ::media::XPlayerWindow > SAL_CALL Player::createPlayerWindow( const uno::Sequence< uno::Any >& aArguments )
    throw (uno::RuntimeException)
{
    uno::Reference< ::media::XPlayerWindow >    xRet;
    awt::Size                                   aSize( getPreferredPlayerWindowSize() );

    if( aSize.Width > 0 && aSize.Height > 0 )
    {
        ::avmedia::xine::Window* pWindow = new ::avmedia::xine::Window( *this );

        xRet = pWindow;

        if( !pWindow->create( aArguments ) )
            xRet = uno::Reference< ::media::XPlayerWindow >();
    }

    return xRet;
}

// ------------------------------------------------------------------------------

uno::Reference< media::XFrameGrabber > SAL_CALL Player::createFrameGrabber(  )
    throw (::com::sun::star::uno::RuntimeException)
{
    return NULL;
}

// ------------------------------------------------------------------------------

::rtl::OUString SAL_CALL Player::getImplementationName(  )
    throw (uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( AVMEDIA_XINE_PLAYER_IMPLEMENTATIONNAME ) );
}

// ------------------------------------------------------------------------------

sal_Bool SAL_CALL Player::supportsService( const ::rtl::OUString& ServiceName )
    throw (uno::RuntimeException)
{
    return ServiceName.equalsAsciiL( RTL_CONSTASCII_STRINGPARAM ( AVMEDIA_XINE_PLAYER_SERVICENAME ) );
}

// ------------------------------------------------------------------------------

uno::Sequence< ::rtl::OUString > SAL_CALL Player::getSupportedServiceNames(  )
    throw (uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aRet(1);
    aRet[0] = ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM ( AVMEDIA_XINE_PLAYER_SERVICENAME ) );

    return aRet;
}

} // namespace xine
} // namespace avmedia
