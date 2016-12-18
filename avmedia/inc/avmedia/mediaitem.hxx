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



#ifndef _AVMEDIA_MEDIAITEM_HXX
#define _AVMEDIA_MEDIAITEM_HXX

#include <tools/rtti.hxx>
#ifndef _POOLITEM_HXX
#include <svl/poolitem.hxx>
#endif
#include <com/sun/star/media/ZoomLevel.hpp>
#include <avmedia/avmediadllapi.h>

#define AVMEDIA_SETMASK_NONE        ((sal_uInt32)(0x00000000))
#define AVMEDIA_SETMASK_STATE       ((sal_uInt32)(0x00000001))
#define AVMEDIA_SETMASK_DURATION    ((sal_uInt32)(0x00000002))
#define AVMEDIA_SETMASK_TIME        ((sal_uInt32)(0x00000004))
#define AVMEDIA_SETMASK_LOOP        ((sal_uInt32)(0x00000008))
#define AVMEDIA_SETMASK_MUTE        ((sal_uInt32)(0x00000010))
#define AVMEDIA_SETMASK_VOLUMEDB    ((sal_uInt32)(0x00000020))
#define AVMEDIA_SETMASK_ZOOM        ((sal_uInt32)(0x00000040))
#define AVMEDIA_SETMASK_URL         ((sal_uInt32)(0x00000080))
#define AVMEDIA_SETMASK_ALL         ((sal_uInt32)(0xffffffff))

class SvStream;

namespace avmedia
{

//---------------
// - MediaState -
// --------------

enum MediaState
{
    MEDIASTATE_STOP = 0,
    MEDIASTATE_PLAY = 1,
    MEDIASTATE_PLAYFFW = 2,
    MEDIASTATE_PAUSE = 3
};

// -------------
// - MediaItem -
// -------------

class AVMEDIA_DLLPUBLIC MediaItem : public SfxPoolItem
{
public:
                            TYPEINFO();

                            MediaItem( sal_uInt16 nWhich = 0, sal_uInt32 nMaskSet = AVMEDIA_SETMASK_NONE );
                            MediaItem( const MediaItem& rMediaItem );
    virtual                 ~MediaItem();

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreUnit,
                                                 SfxMapUnit ePresUnit,
                                                 XubString& rText,
                                                 const IntlWrapper *pIntl ) const;
    virtual sal_Bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual sal_Bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

    void                    merge( const MediaItem& rMediaItem );

    sal_uInt32              getMaskSet() const;

    void                    setState( MediaState eState );
    MediaState              getState() const;

    void                    setDuration( double fDuration );
    double                  getDuration() const;

    void                    setTime( double fTime );
    double                  getTime() const;

    void                    setLoop( sal_Bool bLoop );
    sal_Bool                isLoop() const;

    void                    setMute( sal_Bool bMute );
    sal_Bool                isMute() const;

    void                    setVolumeDB( sal_Int16 nDB );
    sal_Int16               getVolumeDB() const;

    void                    setZoom( ::com::sun::star::media::ZoomLevel eZoom );
    ::com::sun::star::media::ZoomLevel  getZoom() const;

    void                    setURL( const ::rtl::OUString& rURL );
    const ::rtl::OUString&  getURL() const;

private:

    ::rtl::OUString         maURL;
    sal_uInt32              mnMaskSet;
    MediaState              meState;
    double                  mfTime;
    double                  mfDuration;
    sal_Int16               mnVolumeDB;
    sal_Bool                mbLoop;
    sal_Bool                mbMute;
    ::com::sun::star::media::ZoomLevel meZoom;
};

typedef ::avmedia::MediaItem avmedia_MediaItem;

}

#endif
