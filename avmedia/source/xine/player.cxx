/*************************************************************************
 *
 *  $RCSfile: player.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date:
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
