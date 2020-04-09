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

#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <vcl/graph.hxx>
#include <vcl/gfxlink.hxx>
#include <vcl/graphicfilter.hxx>
#include <memory>
#include <boost/functional/hash.hpp>

GfxLink::GfxLink()
    : meType(GfxLinkType::NONE)
    , mnUserId(0)
    , mnSwapInDataSize(0)
    , mbPrefMapModeValid(false)
    , mbPrefSizeValid(false)
{
}



GfxLink::GfxLink(std::unique_ptr<sal_uInt8[]> pBuf, sal_uInt32 nSize, GfxLinkType nType)
    : meType(nType)
    , mnUserId(0)
    , mpSwapInData(std::shared_ptr<sal_uInt8>(pBuf.release(), pBuf.get_deleter())) // std::move(pBuf) does not compile on Jenkins MacOSX (24 May 2016)
    , mnSwapInDataSize(nSize)
    , mbPrefMapModeValid(false)
    , mbPrefSizeValid(false)
{
    SAL_WARN_IF(mpSwapInData == nullptr || mnSwapInDataSize <= 0, "vcl",
                "GfxLink::GfxLink(): empty/NULL buffer given");
}

size_t GfxLink::GetHash() const
{
    if (!maHash)
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, mnSwapInDataSize);
        boost::hash_combine(seed, meType);
        const sal_uInt8* pData = GetData();
        if (pData)
            seed += boost::hash_range(pData, pData + GetDataSize());
        maHash = seed;

    }
    return maHash;
}

bool GfxLink::operator==( const GfxLink& rGfxLink ) const
{
    if (GetHash() != rGfxLink.GetHash())
        return false;

    if ( mnSwapInDataSize != rGfxLink.mnSwapInDataSize ||
         meType != rGfxLink.meType )
        return false;

    const sal_uInt8* pSource = GetData();
    const sal_uInt8* pDest = rGfxLink.GetData();
    if ( pSource == pDest )
        return true;
    sal_uInt32 nSourceSize = GetDataSize();
    sal_uInt32 nDestSize = rGfxLink.GetDataSize();
    if ( pSource && pDest && ( nSourceSize == nDestSize ) )
        return (memcmp( pSource, pDest, nSourceSize ) == 0);
    return false;
}

bool GfxLink::IsNative() const
{
    return meType >= GfxLinkType::NativeFirst && meType <= GfxLinkType::NativeLast;
}


const sal_uInt8* GfxLink::GetData() const
{
    return mpSwapInData.get();
}


void GfxLink::SetPrefSize( const Size& rPrefSize )
{
    maPrefSize = rPrefSize;
    mbPrefSizeValid = true;
}


void GfxLink::SetPrefMapMode( const MapMode& rPrefMapMode )
{
    maPrefMapMode = rPrefMapMode;
    mbPrefMapModeValid = true;
}


bool GfxLink::LoadNative( Graphic& rGraphic )
{
    bool bRet = false;

    if( IsNative() && mnSwapInDataSize )
    {
        const sal_uInt8* pData = GetData();
        if (pData)
        {
            SvMemoryStream aMemoryStream(const_cast<sal_uInt8*>(pData), mnSwapInDataSize, StreamMode::READ | StreamMode::WRITE);
            OUString aShortName;

            switch (meType)
            {
                case GfxLinkType::NativeGif: aShortName = GIF_SHORTNAME; break;
                case GfxLinkType::NativeJpg: aShortName = JPG_SHORTNAME; break;
                case GfxLinkType::NativePng: aShortName = PNG_SHORTNAME; break;
                case GfxLinkType::NativeTif: aShortName = TIF_SHORTNAME; break;
                case GfxLinkType::NativeWmf: aShortName = WMF_SHORTNAME; break;
                case GfxLinkType::NativeMet: aShortName = MET_SHORTNAME; break;
                case GfxLinkType::NativePct: aShortName = PCT_SHORTNAME; break;
                case GfxLinkType::NativeSvg: aShortName = SVG_SHORTNAME; break;
                case GfxLinkType::NativeBmp: aShortName = BMP_SHORTNAME; break;
                case GfxLinkType::NativePdf: aShortName = PDF_SHORTNAME; break;
                default: break;
            }
            if (!aShortName.isEmpty())
            {
                GraphicFilter& rFilter = GraphicFilter::GetGraphicFilter();
                sal_uInt16 nFormat = rFilter.GetImportFormatNumberForShortName(aShortName);
                ErrCode nResult = rFilter.ImportGraphic(rGraphic, OUString(), aMemoryStream, nFormat);
                if (nResult == ERRCODE_NONE)
                    bRet = true;
            }
        }
    }

    return bRet;
}

bool GfxLink::ExportNative( SvStream& rOStream ) const
{
    if( GetDataSize() )
    {
        auto pData = GetSwapInData();
        if (pData)
            rOStream.WriteBytes( pData.get(), mnSwapInDataSize );
    }

    return ( rOStream.GetError() == ERRCODE_NONE );
}

std::shared_ptr<sal_uInt8> GfxLink::GetSwapInData() const
{
    return mpSwapInData;
}

bool GfxLink::IsEMF() const
{
    const sal_uInt8* pGraphicAry = GetData();
    if ((GetType() == GfxLinkType::NativeWmf) && pGraphicAry && (GetDataSize() > 0x2c))
    {
        // check the magic number
        if ((pGraphicAry[0x28] == 0x20) && (pGraphicAry[0x29] == 0x45)
            && (pGraphicAry[0x2a] == 0x4d) && (pGraphicAry[0x2b] == 0x46))
        {
            //emf detected
            return true;
        }
    }
    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
