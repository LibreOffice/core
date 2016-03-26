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

#ifndef INCLUDED_VCL_GFXLINK_HXX
#define INCLUDED_VCL_GFXLINK_HXX

#include <rtl/ustring.hxx>
#include <tools/gen.hxx>
#include <tools/solar.h>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <memory>

class SvStream;

enum class GfxLinkType
{
    NONE         = 0,
    EpsBuffer    = 1,
    NativeGif    = 2,    // Don't forget to update the following defines
    NativeJpeg   = 3,    // Don't forget to update the following defines
    NativePng    = 4,    // Don't forget to update the following defines
    NativeTiff   = 5,    // Don't forget to update the following defines
    NativeWmf    = 6,    // Don't forget to update the following defines
    NativeMet    = 7,    // Don't forget to update the following defines
    NativePict   = 8,    // Don't forget to update the following defines
    NativeSvg    = 9,    // Don't forget to update the following defines
    NativeMov    = 10,   // Don't forget to update the following defines
    NativeBmp    = 11,
    NativePdf    = 12    // Don't forget to update the following defines
};

#define GFX_LINK_FIRST_NATIVE_ID    GfxLinkType::NativeGif
#define GFX_LINK_LAST_NATIVE_ID     GfxLinkType::NativePdf

class Graphic;

class VCL_DLLPUBLIC GfxLink
{
private:

    struct SwapOutData
    {
        SwapOutData(const OUString &aURL);
        ~SwapOutData();

        OUString maURL; // File is removed in the destructor

    };

    GfxLinkType     myType = GfxLinkType::NONE;
    sal_uInt32      mnUserId = 0;

    std::shared_ptr<sal_uInt8> mpSwapInData;
    std::shared_ptr<SwapOutData> mpSwapOutData;

    sal_uInt32      mnSwapInDataSize = 0;
    MapMode         maPrefMapMode;
    Size            maPrefSize;
    bool            mbPrefMapModeValid = false;
    bool            mbPrefSizeValid = false;

    SAL_DLLPRIVATE std::shared_ptr<sal_uInt8> GetSwapInData() const;
public:
                        GfxLink();

                        // pBuff = The Graphic data. This class takes ownership of this
                        GfxLink( std::unique_ptr<sal_uInt8[]> pBuf, sal_uInt32 nBufSize, GfxLinkType nType );

    bool                IsEqual( const GfxLink& ) const;

    GfxLinkType         GetType() const { return myType; }

    void                SetUserId( sal_uInt32 nUserId ) { mnUserId = nUserId; }
    sal_uInt32          GetUserId() const { return mnUserId; }

    sal_uInt32          GetDataSize() const { return mnSwapInDataSize;}
    const sal_uInt8*    GetData() const;

    const Size&         GetPrefSize() const { return maPrefSize;}
    void                SetPrefSize( const Size& rPrefSize );
    bool                IsPrefSizeValid() { return mbPrefSizeValid;}

    const MapMode&      GetPrefMapMode() const { return maPrefMapMode;}
    void                SetPrefMapMode( const MapMode& rPrefMapMode );
    bool                IsPrefMapModeValid() { return mbPrefMapModeValid;}

    bool                IsNative() const;

    bool                LoadNative( Graphic& rGraphic );

    bool                ExportNative( SvStream& rOStream ) const;

    void                SwapOut();
    void                SwapIn();
    bool                IsSwappedOut() const { return( bool(mpSwapOutData) ); }

public:

    friend VCL_DLLPUBLIC SvStream&  WriteGfxLink( SvStream& rOStream, const GfxLink& rGfxLink );
    friend VCL_DLLPUBLIC SvStream&  ReadGfxLink( SvStream& rIStream, GfxLink& rGfxLink );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
