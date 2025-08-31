/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
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

#include <tools/color.hxx>

#include <vcl/BitmapColor.hxx>
#include <vcl/BitmapWriteAccess.hxx>

#include <bitmap/BlendFrameCache.hxx>

BlendFrameCache::BlendFrameCache(Size const& rSize, sal_uInt8 nAlpha, Color const& rColorTopLeft,
                                 Color const& rColorTopRight, Color const& rColorBottomRight,
                                 Color const& rColorBottomLeft)
    : m_aLastSize(rSize)
    , m_nLastAlpha(nAlpha)
    , m_aLastColorTopLeft(rColorTopLeft)
    , m_aLastColorTopRight(rColorTopRight)
    , m_aLastColorBottomRight(rColorBottomRight)
    , m_aLastColorBottomLeft(rColorBottomLeft)
{
    if (rSize.Width() <= 1 || rSize.Height() <= 1)
        return;

    Bitmap aContent(rSize, vcl::PixelFormat::N32_BPP);
    aContent.Erase(COL_TRANSPARENT);

    {
        BitmapScopedWriteAccess pContent(aContent);
        assert(pContent);
        if (!pContent)
            return;

        Scanline pScanContent = pContent->GetScanline(0);

        // x == 0, y == 0, top-left corner
        Color aCol(rColorTopLeft);
        aCol.SetAlpha(nAlpha);
        pContent->SetPixelOnData(pScanContent, 0, aCol);

        tools::Long x;
        const tools::Long nW(rSize.Width());

        // y == 0, top line left to right
        for (x = 1; x < nW - 1; x++)
        {
            Color aMix(rColorTopLeft);

            aMix.Merge(rColorTopRight, 255 - sal_uInt8((x * 255) / nW));
            aMix.SetAlpha(nAlpha);
            pContent->SetPixelOnData(pScanContent, x, aMix);
        }

        // x == nW - 1, y == 0, top-right corner
        // #i123690# Caution! When nW is 1, x == nW is possible (!)
        if (x < nW)
        {
            aCol = rColorTopRight;
            aCol.SetAlpha(nAlpha);
            pContent->SetPixelOnData(pScanContent, x, aCol);
        }

        tools::Long y;
        const tools::Long nH(rSize.Height());

        // x == 0 and nW - 1, left and right line top-down
        for (y = 1; y < nH - 1; y++)
        {
            pScanContent = pContent->GetScanline(y);
            Color aMixA(rColorTopLeft);

            aMixA.Merge(rColorBottomLeft, 255 - sal_uInt8((y * 255) / nH));
            aMixA.SetAlpha(nAlpha);
            pContent->SetPixelOnData(pScanContent, 0, aMixA);

            // #i123690# Caution! When nW is 1, x == nW is possible (!)
            if (x < nW)
            {
                Color aMixB(rColorTopRight);

                aMixB.Merge(rColorBottomRight, 255 - sal_uInt8((y * 255) / nH));
                aMixB.SetAlpha(nAlpha);
                pContent->SetPixelOnData(pScanContent, x, aMixB);
            }
        }

        // #i123690# Caution! When nH is 1, y == nH is possible (!)
        if (y < nH)
        {
            // x == 0, y == nH - 1, bottom-left corner
            aCol = rColorBottomLeft;
            aCol.SetAlpha(nAlpha);
            pContent->SetPixelOnData(pScanContent, 0, aCol);

            // y == nH - 1, bottom line left to right
            for (x = 1; x < nW - 1; x++)
            {
                Color aMix(rColorBottomLeft);

                aMix.Merge(rColorBottomRight, 255 - sal_uInt8(((x - 0) * 255) / nW));
                aMix.SetAlpha(nAlpha);
                pContent->SetPixelOnData(pScanContent, x, aMix);
            }

            // x == nW - 1, y == nH - 1, bottom-right corner
            // #i123690# Caution! When nW is 1, x == nW is possible (!)
            if (x < nW)
            {
                aCol = rColorBottomRight;
                aCol.SetAlpha(nAlpha);
                pContent->SetPixelOnData(pScanContent, x, aCol);
            }
        }
    }

    m_aLastResult = std::move(aContent);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
