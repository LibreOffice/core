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


#include <svx/tbxcolorupdate.hxx>
#include <svx/svxids.hrc>
#include <svx/xdef.hxx>

#include <vcl/toolbox.hxx>
#include <vcl/bitmapaccess.hxx>
#include <vcl/settings.hxx>
#include <tools/debug.hxx>

namespace svx
{
    ToolboxButtonColorUpdater::ToolboxButtonColorUpdater(
        sal_uInt16 nId, sal_uInt16 nTbxBtnId, ToolBox* pToolBox, bool bWideButton)
        : mbWideButton(bWideButton)
        , mnBtnId(nTbxBtnId)
        , mnSlotId(nId)
        , mpTbx(pToolBox)
        , maCurColor(COL_TRANSPARENT)
    {
        DBG_ASSERT(pToolBox, "ToolBox not found :-(");
        mbWasHiContrastMode = pToolBox && pToolBox->GetSettings().GetStyleSettings().GetHighContrastMode();
        switch (mnSlotId)
        {
            case SID_ATTR_CHAR_COLOR:
            case SID_ATTR_CHAR_COLOR2:
                Update(COL_RED_FONTCOLOR);
                break;
            case SID_FRAME_LINECOLOR:
                Update(COL_BLUE);
                break;
            case SID_ATTR_CHAR_COLOR_BACKGROUND:
            case SID_ATTR_CHAR_BACK_COLOR:
            case SID_BACKGROUND_COLOR:
                Update(COL_YELLOW_HIGHLIGHT);
                break;
            case SID_ATTR_LINE_COLOR:
                Update(COL_DEFAULT_SHAPE_STROKE);
                break;
            case SID_ATTR_FILL_COLOR:
                Update(COL_DEFAULT_SHAPE_FILLING);
                break;
            default:
                Update(COL_TRANSPARENT);
        }
    }

    ToolboxButtonColorUpdater::~ToolboxButtonColorUpdater()
    {}

    void ToolboxButtonColorUpdater::Update(const Color& rColor, bool bForceUpdate)
    {
        Image aImage(mpTbx->GetItemImage(mnBtnId));
        Size aItemSize(mbWideButton ? mpTbx->GetItemContentSize(mnBtnId) : aImage.GetSizePixel());

        const bool bSizeChanged = (maBmpSize != aItemSize);
        const bool bDisplayModeChanged = (mbWasHiContrastMode != mpTbx->GetSettings().GetStyleSettings().GetHighContrastMode());
        Color aColor(rColor);

        // !!! #109290# Workaround for SetFillColor with COL_AUTO
        if (aColor.GetColor() == COL_AUTO)
            aColor = Color(COL_TRANSPARENT);

        if ((maCurColor == aColor) && !bSizeChanged && !bDisplayModeChanged && !bForceUpdate)
            return;

        // create an empty bitmap, and copy the original bitmap inside
        // (so that it grows in case the original bitmap was smaller)
        sal_uInt8 nAlpha = 255;
        BitmapEx aBmpEx(Bitmap(aItemSize, 24), AlphaMask(aItemSize, &nAlpha));

        BitmapEx aSource(aImage.GetBitmapEx());
        long nWidth = std::min(aItemSize.Width(), aSource.GetSizePixel().Width());
        long nHeight = std::min(aItemSize.Height(), aSource.GetSizePixel().Height());

        tools::Rectangle aRect(Point(0, 0), Size(nWidth, nHeight));

        aBmpEx.CopyPixel( aRect, aRect, &aSource );

        if (aBmpEx.IsEmpty())
            return;

        maBmpSize = aBmpEx.GetSizePixel();

        mbWasHiContrastMode = mpTbx->GetSettings().GetStyleSettings().GetHighContrastMode();

        Color lineColor;
        if ((COL_TRANSPARENT != aColor.GetColor()) && (maBmpSize.Width() == maBmpSize.Height()))
            lineColor = aColor;
        else if( mpTbx->GetBackground().GetColor().IsDark() )
            lineColor = Color(COL_WHITE);
        else
            lineColor = Color(COL_BLACK);

        // use not only COL_TRANSPARENT for detection of transparence,
        // but the method/way which is designed to do that
        const bool bIsTransparent(0xff == aColor.GetTransparency());
        maCurColor = aColor;

        Color fillColor;
        if (!bIsTransparent)
            fillColor = maCurColor;

        if (maBmpSize.Width() == maBmpSize.Height())
            // tdf#84985 align color bar with icon bottom edge; integer arithmetic e.g. 26 - 26/4 <> 26 * 3/4
            maUpdRect = tools::Rectangle(Point( 0, maBmpSize.Height() - maBmpSize.Height() / 4), Size(maBmpSize.Width(), maBmpSize.Height() / 4));
        else
            maUpdRect = tools::Rectangle(Point( maBmpSize.Height() + 2, 2), Point(maBmpSize.Width() - 3, maBmpSize.Height() - 3));

        Color alphaLineColor;
        Color alphaFillColor;
        if (aBmpEx.IsAlpha())
        {
            if (bIsTransparent)
            {
                alphaLineColor = COL_BLACK;
                alphaFillColor = COL_WHITE;
            }
            else
                alphaFillColor = COL_BLACK;
        }
        aBmpEx.DrawRect(maUpdRect, lineColor, fillColor, alphaLineColor, alphaFillColor);

        mpTbx->SetItemImage(mnBtnId, Image(aBmpEx));
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
