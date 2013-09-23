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

#ifndef _SV_GFXLINK_HXX
#define _SV_GFXLINK_HXX

#include <rtl/ustring.hxx>
#include <tools/stream.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>

// -------------
// - ImpBuffer -
// -------------

struct ImpBuffer
{
    sal_uLong       mnRefCount;
    sal_uInt8*      mpBuffer;

                ImpBuffer( sal_uLong nSize )
                {
                    mnRefCount = 1UL;
                    mpBuffer = nSize ? new sal_uInt8[ nSize ] : NULL;
                }

                ImpBuffer( sal_uInt8* pBuf ) { mnRefCount = 1UL; mpBuffer = pBuf; }

                ~ImpBuffer() { delete[] mpBuffer; }
};

// -----------
// - ImpSwap -
// -----------

struct ImpSwap
{
    OUString   maURL;
    sal_uLong           mnDataSize;
    sal_uLong           mnRefCount;

                    ImpSwap( sal_uInt8* pData, sal_uLong nDataSize );
                    ~ImpSwap();

    sal_uInt8*          GetData() const;

    sal_Bool            IsSwapped() const { return maURL.getLength() > 0; }

    void            WriteTo( SvStream& rOStm ) const;
};

// --------------
// - ImpGfxLink -
// --------------

struct ImpGfxLink
{
    MapMode         maPrefMapMode;
    Size            maPrefSize;
    bool            mbPrefMapModeValid;
    bool            mbPrefSizeValid;

    ImpGfxLink() :
        maPrefMapMode(),
        maPrefSize(),
        mbPrefMapModeValid( false ),
        mbPrefSizeValid( false )
    {}
};

//#endif // __PRIVATE

// ---------------
// - GfxLinkType -
// ---------------

enum GfxLinkType
{
    GFX_LINK_TYPE_NONE          = 0,
    GFX_LINK_TYPE_EPS_BUFFER    = 1,
    GFX_LINK_TYPE_NATIVE_GIF    = 2,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_JPG    = 3,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_PNG    = 4,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_TIF    = 5,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_WMF    = 6,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_MET    = 7,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_PCT    = 8,    // Don't forget to update the following defines
    GFX_LINK_TYPE_NATIVE_SVG    = 9,    // Don't forget to update the following defines
    GFX_LINK_TYPE_USER          = 0xffff
};

#define GFX_LINK_FIRST_NATIVE_ID    GFX_LINK_TYPE_NATIVE_GIF
#define GFX_LINK_LAST_NATIVE_ID     GFX_LINK_TYPE_NATIVE_SVG

// -----------
// - GfxLink -
// -----------

struct ImpBuffer;
struct ImpSwap;
struct ImpGfxLink;
class Graphic;

class VCL_DLLPUBLIC GfxLink
{
private:

    GfxLinkType         meType;
    ImpBuffer*          mpBuf;
    ImpSwap*            mpSwap;
    sal_uInt32          mnBufSize;
    sal_uInt32          mnUserId;
    ImpGfxLink*         mpImpData;
    sal_uLong               mnExtra2;

    SAL_DLLPRIVATE void ImplCopy( const GfxLink& rGfxLink );

public:
                        GfxLink();
                        GfxLink( const GfxLink& );
                        GfxLink( const OUString& rPath, GfxLinkType nType );
                        GfxLink( sal_uInt8* pBuf, sal_uInt32 nBufSize, GfxLinkType nType, sal_Bool bOwns );
                        ~GfxLink();

    GfxLink&            operator=( const GfxLink& );
    sal_Bool            IsEqual( const GfxLink& ) const;

    GfxLinkType         GetType() const;

    void                SetUserId( sal_uInt32 nUserId ) { mnUserId = nUserId; }
    sal_uInt32          GetUserId() const { return mnUserId; }

    sal_uInt32          GetDataSize() const;
    void                SetData( sal_uInt8* pBuf, sal_uInt32 nSize, GfxLinkType nType, sal_Bool bOwns );
    const sal_uInt8*            GetData() const;

    const Size&         GetPrefSize() const;
    void                SetPrefSize( const Size& rPrefSize );
    bool                IsPrefSizeValid();

    const MapMode&      GetPrefMapMode() const;
    void                SetPrefMapMode( const MapMode& rPrefMapMode );
    bool                IsPrefMapModeValid();

    sal_Bool                IsNative() const;
    sal_Bool                IsUser() const { return( GFX_LINK_TYPE_USER == meType ); }

    sal_Bool                LoadNative( Graphic& rGraphic );

    sal_Bool                ExportNative( SvStream& rOStream ) const;

    void                SwapOut();
    void                SwapIn();
    sal_Bool                IsSwappedOut() const { return( mpSwap != NULL ); }

public:

    friend VCL_DLLPUBLIC SvStream&  operator<<( SvStream& rOStream, const GfxLink& rGfxLink );
    friend VCL_DLLPUBLIC SvStream&  operator>>( SvStream& rIStream, GfxLink& rGfxLink );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
