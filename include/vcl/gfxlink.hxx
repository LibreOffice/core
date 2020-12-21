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

#include <tools/gen.hxx>
#include <vcl/dllapi.h>
#include <vcl/mapmod.hxx>
#include <vcl/BinaryDataContainer.hxx>
#include <memory>

class SvStream;

/** GfxLink graphic types that are supported by GfxLink.
 *
 * It is important that the numbers for native types stay the same, because
 * they are used in serialization to MTF.
 */
enum class GfxLinkType
{
    NONE         = 0,
    EpsBuffer    = 1,
    NativeGif    = 2,
    NativeJpg    = 3,
    NativePng    = 4,
    NativeTif    = 5,
    NativeWmf    = 6,
    NativeMet    = 7,
    NativePct    = 8,
    NativeSvg    = 9,
    NativeMov    = 10,
    NativeBmp    = 11,
    NativePdf    = 12, // If a new type is added, make sure to change NativeLast too

    // Alias for when the first native type starts and last native
    // type ends.
    NativeFirst  = NativeGif,
    NativeLast   = NativePdf,
};

class Graphic;

class VCL_DLLPUBLIC GfxLink
{
private:
    GfxLinkType     meType;
    sal_uInt32      mnUserId;
    BinaryDataContainer maDataContainer;
    mutable size_t  maHash;
    MapMode         maPrefMapMode;
    Size            maPrefSize;
    bool            mbPrefMapModeValid;
    bool            mbPrefSizeValid;

public:
    GfxLink();
    explicit GfxLink(std::unique_ptr<sal_uInt8[]> pBuf, sal_uInt32 nBufSize, GfxLinkType nType);
    explicit GfxLink(BinaryDataContainer const & rDataConainer, GfxLinkType nType);

    bool                operator==( const GfxLink& ) const;

    GfxLinkType         GetType() const { return meType;}

    size_t              GetHash() const;

    void                SetUserId( sal_uInt32 nUserId ) { mnUserId = nUserId; }
    sal_uInt32          GetUserId() const { return mnUserId; }

    sal_uInt32          GetDataSize() const { return maDataContainer.getSize(); }
    const sal_uInt8*    GetData() const;

    const Size&         GetPrefSize() const { return maPrefSize;}
    void                SetPrefSize( const Size& rPrefSize );
    bool                IsPrefSizeValid() const { return mbPrefSizeValid;}

    const MapMode&      GetPrefMapMode() const { return maPrefMapMode;}
    void                SetPrefMapMode( const MapMode& rPrefMapMode );
    bool                IsPrefMapModeValid() const { return mbPrefMapModeValid;}

    bool                IsNative() const;

    bool                LoadNative( Graphic& rGraphic );

    bool                ExportNative( SvStream& rOStream ) const;

    bool                IsEMF() const; // WMF & EMF stored under the same type (NativeWmf)
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
