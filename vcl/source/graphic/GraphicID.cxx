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

#include <graphic/GraphicID.hxx>

#include <impgraph.hxx>
#include <rtl/strbuf.hxx>

GraphicID::GraphicID(ImpGraphic const& rGraphic)
{
    rGraphic.ensureAvailable();

    mnID1 = static_cast<sal_uLong>(rGraphic.ImplGetType()) << 28;
    mnID2 = mnID3 = mnID4 = 0;

    if (rGraphic.ImplGetType() == GraphicType::Bitmap)
    {
        auto const& rVectorGraphicDataPtr = rGraphic.getVectorGraphicData();
        if (rVectorGraphicDataPtr)
        {
            const basegfx::B2DRange& rRange = rVectorGraphicDataPtr->getRange();

            mnID1 |= rVectorGraphicDataPtr->getVectorGraphicDataArrayLength();
            mnID2 = basegfx::fround(rRange.getWidth());
            mnID3 = basegfx::fround(rRange.getHeight());
            mnID4 = vcl_get_checksum(0, rVectorGraphicDataPtr->getBinaryDataContainer().getData(),
                                     rVectorGraphicDataPtr->getBinaryDataContainer().getSize());
        }
        else if (rGraphic.ImplIsAnimated())
        {
            const Animation aAnimation(rGraphic.ImplGetAnimation());

            mnID1 |= (aAnimation.Count() & 0x0fffffff);
            mnID2 = aAnimation.GetDisplaySizePixel().Width();
            mnID3 = aAnimation.GetDisplaySizePixel().Height();
            mnID4 = rGraphic.ImplGetChecksum();
        }
        else
        {
            const BitmapEx aBmpEx(rGraphic.ImplGetBitmapEx(GraphicConversionParameters()));

            mnID1 |= (((static_cast<sal_uLong>(aBmpEx.GetTransparentType()) << 8)
                       | (aBmpEx.IsAlpha() ? 1 : 0))
                      & 0x0fffffff);
            mnID2 = aBmpEx.GetSizePixel().Width();
            mnID3 = aBmpEx.GetSizePixel().Height();
            mnID4 = rGraphic.ImplGetChecksum();
        }
    }
    else if (rGraphic.ImplGetType() == GraphicType::GdiMetafile)
    {
        const GDIMetaFile& rMtf = rGraphic.ImplGetGDIMetaFile();

        mnID1 |= (rMtf.GetActionSize() & 0x0fffffff);
        mnID2 = rMtf.GetPrefSize().Width();
        mnID3 = rMtf.GetPrefSize().Height();
        mnID4 = rGraphic.ImplGetChecksum();
    }
}

OString GraphicID::getIDString() const
{
    static const char aHexData[]
        = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

    sal_Int32 nShift, nIndex = 0;
    sal_Int32 nLen = 24 + (2 * BITMAP_CHECKSUM_SIZE);
    OStringBuffer aHexStr(nLen);
    aHexStr.setLength(nLen);

    for (nShift = 28; nShift >= 0; nShift -= 4)
        aHexStr[nIndex++] = aHexData[(mnID1 >> static_cast<sal_uInt32>(nShift)) & 0xf];

    for (nShift = 28; nShift >= 0; nShift -= 4)
        aHexStr[nIndex++] = aHexData[(mnID2 >> static_cast<sal_uInt32>(nShift)) & 0xf];

    for (nShift = 28; nShift >= 0; nShift -= 4)
        aHexStr[nIndex++] = aHexData[(mnID3 >> static_cast<sal_uInt32>(nShift)) & 0xf];

    for (nShift = (8 * BITMAP_CHECKSUM_SIZE) - 4; nShift >= 0; nShift -= 4)
        aHexStr[nIndex++] = aHexData[(mnID4 >> static_cast<sal_uInt32>(nShift)) & 0xf];

    return aHexStr.makeStringAndClear();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
