/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef AVMEDIA_MEDIAITEM_HXX
#define AVMEDIA_MEDIAITEM_HXX

#include <boost/scoped_ptr.hpp>

#include <tools/rtti.hxx>
#include <svl/poolitem.hxx>
#include <com/sun/star/media/ZoomLevel.hpp>
#include <com/sun/star/frame/XModel.hpp>
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

                            MediaItem( sal_uInt16 const i_nWhich = 0,
                            sal_uInt32 const nMaskSet = AVMEDIA_SETMASK_NONE );
                            MediaItem( const MediaItem& rMediaItem );
    virtual                 ~MediaItem();

    virtual int             operator==( const SfxPoolItem& ) const;
    virtual SfxPoolItem*    Clone( SfxItemPool* pPool = 0 ) const;
    virtual SfxItemPresentation GetPresentation( SfxItemPresentation ePres,
                                                 SfxMapUnit eCoreUnit,
                                                 SfxMapUnit ePresUnit,
                                                 XubString& rText,
                                                 const IntlWrapper *pIntl ) const;
    virtual bool            QueryValue( com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const;
    virtual bool            PutValue( const com::sun::star::uno::Any& rVal, sal_uInt8 nMemberId = 0 );

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

    void                    setURL( const OUString& rURL,
                                    OUString const*const pTempURL);
    const OUString&  getURL() const;

    const OUString&  getTempURL() const;

private:

    struct Impl;
    ::boost::scoped_ptr<Impl> m_pImpl;
};

typedef ::avmedia::MediaItem avmedia_MediaItem;

bool AVMEDIA_DLLPUBLIC EmbedMedia(
        ::com::sun::star::uno::Reference< ::com::sun::star::frame::XModel>
            const& xModel,
        OUString const& rSourceURL,
        OUString & o_rEmbeddedURL);

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
