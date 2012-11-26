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



#include <avmedia/mediaitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star;

namespace avmedia
{

// -------------
// - MediaItem -
// -------------

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

IMPL_POOLITEM_FACTORY(MediaItem)

MediaItem::MediaItem( sal_uInt16 _nWhich, sal_uInt32 nMaskSet ) :
    SfxPoolItem( _nWhich ),
    mnMaskSet( nMaskSet ),
    meState( MEDIASTATE_STOP ),
    mfTime( 0.0 ),
    mfDuration( 0.0 ),
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
    mfTime( rItem.mfTime ),
    mfDuration( rItem.mfDuration ),
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

SfxPoolItem* MediaItem::Clone( SfxItemPool* ) const
{
    return new MediaItem( *this );
}

//------------------------------------------------------------------------

SfxItemPresentation MediaItem::GetPresentation( SfxItemPresentation,
                                                  SfxMapUnit,
                                                  SfxMapUnit,
                                                  XubString& rText,
                                                  const IntlWrapper * ) const
{
    rText.Erase();
    return SFX_ITEM_PRESENTATION_NONE;
}

//------------------------------------------------------------------------

sal_Bool MediaItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 ) const
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

sal_Bool MediaItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    uno::Sequence< uno::Any >   aSeq;
    sal_Bool                        bRet = false;

    if( ( rVal >>= aSeq ) && ( aSeq.getLength() == 9 ) )
    {
        sal_Int32 nInt32 = 0;

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
