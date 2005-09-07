/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mediaitem.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:37:50 $
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

#include "mediaitem.hxx"
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star;

namespace avmedia
{

// -------------
// - MediaItem -
// -------------

TYPEINIT1_AUTOFACTORY( MediaItem, ::SfxPoolItem );
    ::rtl::OUString         maURL;
    sal_uInt32              mnMaskSet;
    MediaState              meState;
    double                  mfTime;
    double                  mfDuration;
    sal_Int16               mnVolumeDB;
    sal_Bool                mbLoop;
    sal_Bool                mbMute;
    ::com::sun::star::media::ZoomLevel meZoom;

// ------------------------------------------------------------------------------

MediaItem::MediaItem( USHORT nWhich, sal_uInt32 nMaskSet ) :
    SfxPoolItem( nWhich ),
    mnMaskSet( nMaskSet ),
    meState( MEDIASTATE_STOP ),
    mfDuration( 0.0 ),
    mfTime( 0.0 ),
    mnVolumeDB( 0 ),
    mbLoop( false ),
    mbMute( false ),
    meZoom( ::com::sun::star::media::ZoomLevel_NOT_AVAILABLE )
{
}

// ------------------------------------------------------------------------------

MediaItem::MediaItem( const MediaItem& rItem ) :
    SfxPoolItem( rItem ),
    maURL( rItem.maURL ),
    mnMaskSet( rItem.mnMaskSet ),
    meState( rItem.meState ),
    mfDuration( rItem.mfDuration ),
    mfTime( rItem.mfTime ),
    mnVolumeDB( rItem.mnVolumeDB ),
    mbLoop( rItem.mbLoop ),
    mbMute( rItem.mbMute ),
    meZoom( rItem.meZoom )
{
}

// ------------------------------------------------------------------------------

MediaItem::~MediaItem()
{
}

// ------------------------------------------------------------------------------

int MediaItem::operator==( const SfxPoolItem& rItem ) const
{
    DBG_ASSERT( SfxPoolItem::operator==(rItem), "unequal types" );
    return( mnMaskSet == static_cast< const MediaItem& >( rItem ).mnMaskSet &&
            maURL == static_cast< const MediaItem& >( rItem ).maURL &&
            meState == static_cast< const MediaItem& >( rItem ).meState &&
            mfDuration == static_cast< const MediaItem& >( rItem ).mfDuration &&
            mfTime == static_cast< const MediaItem& >( rItem ).mfTime &&
            mnVolumeDB == static_cast< const MediaItem& >( rItem ).mnVolumeDB &&
            mbLoop == static_cast< const MediaItem& >( rItem ).mbLoop &&
            mbMute == static_cast< const MediaItem& >( rItem ).mbMute &&
            meZoom == static_cast< const MediaItem& >( rItem ).meZoom );
}

// ------------------------------------------------------------------------------

SfxPoolItem* MediaItem::Clone( SfxItemPool* pPool ) const
{
    return new MediaItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation MediaItem::GetPresentation( SfxItemPresentation ePres,
                                                  SfxMapUnit eCoreUnit,
                                                  SfxMapUnit ePresUnit,
                                                  XubString& rText,
                                                  const IntlWrapper *pIntl ) const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

BOOL MediaItem::QueryValue( com::sun::star::uno::Any& rVal, BYTE nMemberId ) const
{
    uno::Sequence< uno::Any > aSeq( 9 );

    aSeq[ 0 ] <<= maURL;
    aSeq[ 1 ] <<= mnMaskSet;
    aSeq[ 2 ] <<= static_cast< sal_Int32 >( meState );
    aSeq[ 3 ] <<= mfTime;
    aSeq[ 4 ] <<= mfDuration;
    aSeq[ 5 ] <<= mnVolumeDB;
    aSeq[ 6 ] <<= mbLoop;
    aSeq[ 7 ] <<= mbMute;
    aSeq[ 8 ] <<= meZoom;

    rVal <<= aSeq;

    return true;
}

//------------------------------------------------------------------------

BOOL MediaItem::PutValue( const com::sun::star::uno::Any& rVal, BYTE nMemberId )
{
    uno::Sequence< uno::Any >   aSeq;
    BOOL                        bRet = false;

    if( ( rVal >>= aSeq ) && ( aSeq.getLength() == 9 ) )
    {
        sal_Int32 nInt32;

        aSeq[ 0 ] >>= maURL;
        aSeq[ 1 ] >>= mnMaskSet;
        aSeq[ 2 ] >>= nInt32; meState = static_cast< MediaState >( nInt32 );
        aSeq[ 3 ] >>= mfTime;
        aSeq[ 4 ] >>= mfDuration;
        aSeq[ 5 ] >>= mnVolumeDB;
        aSeq[ 6 ] >>= mbLoop;
        aSeq[ 7 ] >>= mbMute;
        aSeq[ 8 ] >>= meZoom;

        bRet = true;
    }

    return bRet;
}

//------------------------------------------------------------------------

void MediaItem::merge( const MediaItem& rMediaItem )
{
    const sal_uInt32 nMaskSet = rMediaItem.getMaskSet();

    if( AVMEDIA_SETMASK_URL & nMaskSet )
        setURL( rMediaItem.getURL() );

    if( AVMEDIA_SETMASK_STATE & nMaskSet )
        setState( rMediaItem.getState() );

    if( AVMEDIA_SETMASK_DURATION & nMaskSet )
        setDuration( rMediaItem.getDuration() );

    if( AVMEDIA_SETMASK_TIME & nMaskSet )
        setTime( rMediaItem.getTime() );

    if( AVMEDIA_SETMASK_LOOP & nMaskSet )
        setLoop( rMediaItem.isLoop() );

    if( AVMEDIA_SETMASK_MUTE & nMaskSet )
        setMute( rMediaItem.isMute() );

    if( AVMEDIA_SETMASK_VOLUMEDB & nMaskSet )
        setVolumeDB( rMediaItem.getVolumeDB() );

    if( AVMEDIA_SETMASK_ZOOM & nMaskSet )
        setZoom( rMediaItem.getZoom() );
}

//------------------------------------------------------------------------

sal_uInt32 MediaItem::getMaskSet() const
{
    return mnMaskSet;
}

//------------------------------------------------------------------------

void MediaItem::setURL( const ::rtl::OUString& rURL )
{
    maURL = rURL;
    mnMaskSet |= AVMEDIA_SETMASK_URL;
}

//------------------------------------------------------------------------

const ::rtl::OUString& MediaItem::getURL() const
{
    return maURL;
}

//------------------------------------------------------------------------

void MediaItem::setState( MediaState eState )
{
    meState = eState;
    mnMaskSet |= AVMEDIA_SETMASK_STATE;
}

//------------------------------------------------------------------------

MediaState MediaItem::getState() const
{
    return meState;
}

//------------------------------------------------------------------------

void MediaItem::setDuration( double fDuration )
{
    mfDuration = fDuration;
    mnMaskSet |= AVMEDIA_SETMASK_DURATION;
}

//------------------------------------------------------------------------

double MediaItem::getDuration() const
{
    return mfDuration;
}

//------------------------------------------------------------------------

void MediaItem::setTime( double fTime )
{
    mfTime = fTime;
    mnMaskSet |= AVMEDIA_SETMASK_TIME;
}

//------------------------------------------------------------------------

double MediaItem::getTime() const
{
    return mfTime;
}

//------------------------------------------------------------------------

void MediaItem::setLoop( sal_Bool bLoop )
{
    mbLoop = bLoop;
    mnMaskSet |= AVMEDIA_SETMASK_LOOP;
}

//------------------------------------------------------------------------

sal_Bool MediaItem::isLoop() const
{
    return mbLoop;
}

//------------------------------------------------------------------------

void MediaItem::setMute( sal_Bool bMute )
{
    mbMute = bMute;
    mnMaskSet |= AVMEDIA_SETMASK_MUTE;
}

//------------------------------------------------------------------------

sal_Bool MediaItem::isMute() const
{
    return mbMute;
}

//------------------------------------------------------------------------

void MediaItem::setVolumeDB( sal_Int16 nDB )
{
    mnVolumeDB = nDB;
    mnMaskSet |= AVMEDIA_SETMASK_VOLUMEDB;
}

//------------------------------------------------------------------------

sal_Int16 MediaItem::getVolumeDB() const
{
    return mnVolumeDB;
}

//------------------------------------------------------------------------

void MediaItem::setZoom( ::com::sun::star::media::ZoomLevel eZoom )
{
    meZoom = eZoom;
    mnMaskSet |= AVMEDIA_SETMASK_ZOOM;
}

//------------------------------------------------------------------------

::com::sun::star::media::ZoomLevel MediaItem::getZoom() const
{
    return meZoom;
}

}
