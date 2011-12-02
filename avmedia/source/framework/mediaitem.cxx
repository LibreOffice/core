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

#include <avmedia/mediaitem.hxx>
#include <com/sun/star/uno/Sequence.hxx>

using namespace ::com::sun::star;

namespace avmedia
{

// -------------
// - MediaItem -
// -------------

TYPEINIT1_AUTOFACTORY( MediaItem, ::SfxPoolItem );

struct MediaItem::Impl
{
    ::rtl::OUString         m_URL;
    sal_uInt32              m_nMaskSet;
    MediaState              m_eState;
    double                  m_fTime;
    double                  m_fDuration;
    sal_Int16               m_nVolumeDB;
    sal_Bool                m_bLoop;
    sal_Bool                m_bMute;
    ::com::sun::star::media::ZoomLevel m_eZoom;

    Impl(sal_uInt32 const nMaskSet)
        : m_nMaskSet( nMaskSet )
        , m_eState( MEDIASTATE_STOP )
        , m_fTime( 0.0 )
        , m_fDuration( 0.0 )
        , m_nVolumeDB( 0 )
        , m_bLoop( false )
        , m_bMute( false )
        , m_eZoom( ::com::sun::star::media::ZoomLevel_NOT_AVAILABLE )
    {
    }
    Impl(Impl const& rOther)
        : m_URL( rOther.m_URL )
        , m_nMaskSet( rOther.m_nMaskSet )
        , m_eState( rOther.m_eState )
        , m_fTime( rOther.m_fTime )
        , m_fDuration( rOther.m_fDuration )
        , m_nVolumeDB( rOther.m_nVolumeDB )
        , m_bLoop( rOther.m_bLoop )
        , m_bMute( rOther.m_bMute )
        , m_eZoom( rOther.m_eZoom )
    {
    }
};

// ------------------------------------------------------------------------------

MediaItem::MediaItem( sal_uInt16 const i_nWhich, sal_uInt32 const nMaskSet )
    : SfxPoolItem( i_nWhich )
    , m_pImpl( new Impl(nMaskSet) )
{
}

// ------------------------------------------------------------------------------

MediaItem::MediaItem( const MediaItem& rItem )
    : SfxPoolItem( rItem )
    , m_pImpl( new Impl(*rItem.m_pImpl) )
{
}

// ------------------------------------------------------------------------------

MediaItem::~MediaItem()
{
}

// ------------------------------------------------------------------------------

int MediaItem::operator==( const SfxPoolItem& rItem ) const
{
    assert( SfxPoolItem::operator==(rItem));
    MediaItem const& rOther(static_cast< const MediaItem& >(rItem));
    return m_pImpl->m_nMaskSet == rOther.m_pImpl->m_nMaskSet
        && m_pImpl->m_URL == rOther.m_pImpl->m_URL
        && m_pImpl->m_eState == rOther.m_pImpl->m_eState
        && m_pImpl->m_fDuration == rOther.m_pImpl->m_fDuration
        && m_pImpl->m_fTime == rOther.m_pImpl->m_fTime
        && m_pImpl->m_nVolumeDB == rOther.m_pImpl->m_nVolumeDB
        && m_pImpl->m_bLoop == rOther.m_pImpl->m_bLoop
        && m_pImpl->m_bMute == rOther.m_pImpl->m_bMute
        && m_pImpl->m_eZoom == rOther.m_pImpl->m_eZoom;
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

bool MediaItem::QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 ) const
{
    uno::Sequence< uno::Any > aSeq( 9 );

    aSeq[ 0 ] <<= m_pImpl->m_URL;
    aSeq[ 1 ] <<= m_pImpl->m_nMaskSet;
    aSeq[ 2 ] <<= static_cast< sal_Int32 >( m_pImpl->m_eState );
    aSeq[ 3 ] <<= m_pImpl->m_fTime;
    aSeq[ 4 ] <<= m_pImpl->m_fDuration;
    aSeq[ 5 ] <<= m_pImpl->m_nVolumeDB;
    aSeq[ 6 ] <<= m_pImpl->m_bLoop;
    aSeq[ 7 ] <<= m_pImpl->m_bMute;
    aSeq[ 8 ] <<= m_pImpl->m_eZoom;

    rVal <<= aSeq;

    return true;
}

//------------------------------------------------------------------------

bool MediaItem::PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 )
{
    uno::Sequence< uno::Any >   aSeq;
    bool                        bRet = false;

    if( ( rVal >>= aSeq ) && ( aSeq.getLength() == 9 ) )
    {
        sal_Int32 nInt32 = 0;

        aSeq[ 0 ] >>= m_pImpl->m_URL;
        aSeq[ 1 ] >>= m_pImpl->m_nMaskSet;
        aSeq[ 2 ] >>= nInt32;
        m_pImpl->m_eState = static_cast< MediaState >( nInt32 );
        aSeq[ 3 ] >>= m_pImpl->m_fTime;
        aSeq[ 4 ] >>= m_pImpl->m_fDuration;
        aSeq[ 5 ] >>= m_pImpl->m_nVolumeDB;
        aSeq[ 6 ] >>= m_pImpl->m_bLoop;
        aSeq[ 7 ] >>= m_pImpl->m_bMute;
        aSeq[ 8 ] >>= m_pImpl->m_eZoom;

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
    return m_pImpl->m_nMaskSet;
}

//------------------------------------------------------------------------

void MediaItem::setURL( const ::rtl::OUString& rURL )
{
    m_pImpl->m_URL = rURL;
    m_pImpl->m_nMaskSet |= AVMEDIA_SETMASK_URL;
}

//------------------------------------------------------------------------

const ::rtl::OUString& MediaItem::getURL() const
{
    return m_pImpl->m_URL;
}

//------------------------------------------------------------------------

void MediaItem::setState( MediaState eState )
{
    m_pImpl->m_eState = eState;
    m_pImpl->m_nMaskSet |= AVMEDIA_SETMASK_STATE;
}

//------------------------------------------------------------------------

MediaState MediaItem::getState() const
{
    return m_pImpl->m_eState;
}

//------------------------------------------------------------------------

void MediaItem::setDuration( double fDuration )
{
    m_pImpl->m_fDuration = fDuration;
    m_pImpl->m_nMaskSet |= AVMEDIA_SETMASK_DURATION;
}

//------------------------------------------------------------------------

double MediaItem::getDuration() const
{
    return m_pImpl->m_fDuration;
}

//------------------------------------------------------------------------

void MediaItem::setTime( double fTime )
{
    m_pImpl->m_fTime = fTime;
    m_pImpl->m_nMaskSet |= AVMEDIA_SETMASK_TIME;
}

//------------------------------------------------------------------------

double MediaItem::getTime() const
{
    return m_pImpl->m_fTime;
}

//------------------------------------------------------------------------

void MediaItem::setLoop( sal_Bool bLoop )
{
    m_pImpl->m_bLoop = bLoop;
    m_pImpl->m_nMaskSet |= AVMEDIA_SETMASK_LOOP;
}

//------------------------------------------------------------------------

sal_Bool MediaItem::isLoop() const
{
    return m_pImpl->m_bLoop;
}

//------------------------------------------------------------------------

void MediaItem::setMute( sal_Bool bMute )
{
    m_pImpl->m_bMute = bMute;
    m_pImpl->m_nMaskSet |= AVMEDIA_SETMASK_MUTE;
}

//------------------------------------------------------------------------

sal_Bool MediaItem::isMute() const
{
    return m_pImpl->m_bMute;
}

//------------------------------------------------------------------------

void MediaItem::setVolumeDB( sal_Int16 nDB )
{
    m_pImpl->m_nVolumeDB = nDB;
    m_pImpl->m_nMaskSet |= AVMEDIA_SETMASK_VOLUMEDB;
}

//------------------------------------------------------------------------

sal_Int16 MediaItem::getVolumeDB() const
{
    return m_pImpl->m_nVolumeDB;
}

//------------------------------------------------------------------------

void MediaItem::setZoom( ::com::sun::star::media::ZoomLevel eZoom )
{
    m_pImpl->m_eZoom = eZoom;
    m_pImpl->m_nMaskSet |= AVMEDIA_SETMASK_ZOOM;
}

//------------------------------------------------------------------------

::com::sun::star::media::ZoomLevel MediaItem::getZoom() const
{
    return m_pImpl->m_eZoom;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
