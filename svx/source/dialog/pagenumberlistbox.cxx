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

#include <editeng/brushitem.hxx>
#include <editeng/numitem.hxx>
#include <i18nlangtag/mslangid.hxx>
#include <svtools/colorcfg.hxx>
#include <svx/dialmgr.hxx>
#include <svx/numberingpreview.hxx>
#include <svx/pagenumberlistbox.hxx>
#include <vcl/graph.hxx>
#include <vcl/metric.hxx>
#include <vcl/virdev.hxx>
#include <numberingtype.hrc>

#include <com/sun/star/style/NumberingType.hpp>

SvxPageNumberListBox::SvxPageNumberListBox(std::unique_ptr<weld::ComboBox> pControl)
    : m_xControl(std::move(pControl))
{
    m_xControl->set_size_request(150, -1);

    for (size_t i = 0; i < SAL_N_ELEMENTS(RID_SVXSTRARY_NUMBERINGTYPE); ++i)
    {
        sal_uInt16 nData = RID_SVXSTRARY_NUMBERINGTYPE[i].second;
        switch (nData)
        {
            // String list array is also used in Writer and contains strings
            // for Bullet and Graphics, ignore those here.
            case css::style::NumberingType::CHAR_SPECIAL:
            case css::style::NumberingType::BITMAP:
            case css::style::NumberingType::BITMAP | LINK_TOKEN:
                break;
            default:
            {
                OUString aStr = SvxResId(RID_SVXSTRARY_NUMBERINGTYPE[i].first);
                m_xControl->append(OUString::number(nData), aStr);
                break;
            }
        }
    }
}

SvxNumberingPreview::SvxNumberingPreview()
    : pActNum(nullptr)
    , bPosition(false)
    , nActLevel(SAL_MAX_UINT16)
{
}

static tools::Long lcl_DrawGraphic(VirtualDevice& rVDev, const SvxNumberFormat& rFmt,
                                   tools::Long nXStart, tools::Long nYMiddle, tools::Long nDivision)
{
    const SvxBrushItem* pBrushItem = rFmt.GetBrush();
    tools::Long nRet = 0;
    if (pBrushItem)
    {
        const Graphic* pGrf = pBrushItem->GetGraphic();
        if (pGrf)
        {
            Size aGSize(rFmt.GetGraphicSize());
            aGSize.setWidth(aGSize.Width() / nDivision);
            nRet = aGSize.Width();
            aGSize.setHeight(aGSize.Height() / nDivision);
            pGrf->Draw(rVDev, Point(nXStart, nYMiddle - (aGSize.Height() / 2)),
                       rVDev.PixelToLogic(aGSize));
        }
    }
    return nRet;
}

static tools::Long lcl_DrawBullet(VirtualDevice* pVDev, const SvxNumberFormat& rFmt,
                                  tools::Long nXStart, tools::Long nYStart, const Size& rSize)
{
    vcl::Font aTmpFont(pVDev->GetFont());

    // via Uno it's possible that no font has been set!
    vcl::Font aFont(rFmt.GetBulletFont() ? *rFmt.GetBulletFont() : aTmpFont);
    Size aTmpSize(rSize);
    aTmpSize.setWidth(aTmpSize.Width() * (rFmt.GetBulletRelSize()));
    aTmpSize.setWidth(aTmpSize.Width() / 100);
    aTmpSize.setHeight(aTmpSize.Height() * (rFmt.GetBulletRelSize()));
    aTmpSize.setHeight(aTmpSize.Height() / 100);
    // in case of a height of zero it is drawn in original height
    if (!aTmpSize.Height())
        aTmpSize.setHeight(1);
    aFont.SetFontSize(aTmpSize);
    aFont.SetTransparent(true);
    Color aBulletColor = rFmt.GetBulletColor();
    if (aBulletColor == COL_AUTO)
        aBulletColor = pVDev->GetBackgroundColor().IsDark() ? COL_WHITE : COL_BLACK;
    else if (pVDev->GetBackgroundColor().IsDark() == aBulletColor.IsDark())
        aBulletColor = pVDev->GetBackgroundColor().IsDark() ? COL_WHITE : COL_BLACK;
    aFont.SetColor(aBulletColor);
    pVDev->SetFont(aFont);
    sal_UCS4 cChar = rFmt.GetBulletChar();
    OUString aText(&cChar, 1);
    tools::Long nY = nYStart;
    nY -= ((aTmpSize.Height() - rSize.Height()) / 2);
    pVDev->DrawText(Point(nXStart, nY), aText);
    tools::Long nRet = pVDev->GetTextWidth(aText);

    pVDev->SetFont(aTmpFont);
    return nRet;
}

// paint preview of numeration
void SvxNumberingPreview::Paint(vcl::RenderContext& rRenderContext,
                                const ::tools::Rectangle& /*rRect*/)
{
    Size aSize(rRenderContext.PixelToLogic(GetOutputSizePixel()));

    // Use default document and font colors to create preview
    const Color aBackColor = svtools::ColorConfig().GetColorValue(svtools::DOCCOLOR).nColor;
    const Color aTextColor = svtools::ColorConfig().GetColorValue(svtools::FONTCOLOR).nColor;

    ScopedVclPtrInstance<VirtualDevice> pVDev(rRenderContext);
    pVDev->EnableRTL(rRenderContext.IsRTLEnabled());
    pVDev->SetMapMode(rRenderContext.GetMapMode());
    pVDev->SetOutputSize(aSize);

    Color aLineColor(COL_LIGHTGRAY);
    if (aLineColor == aBackColor)
        aLineColor.Invert();
    pVDev->SetLineColor(aLineColor);
    pVDev->SetFillColor(aBackColor);
    pVDev->SetBackground(Wallpaper(aBackColor));
    pVDev->DrawWallpaper(pVDev->GetOutputRectPixel(), pVDev->GetBackground());

    if (pActNum)
    {
        tools::Long nWidthRelation = 30; // chapter dialog

        // height per level
        tools::Long nXStep
            = aSize.Width() / (pActNum->GetLevelCount() > 1 ? 3 * pActNum->GetLevelCount() : 3);
        if (pActNum->GetLevelCount() < 10)
            nXStep /= 2;
        tools::Long nYStart = 4;
        // the whole height mustn't be used for a single level
        tools::Long nYStep
            = (aSize.Height() - 6) / (pActNum->GetLevelCount() > 1 ? pActNum->GetLevelCount() : 5);

        aStdFont = OutputDevice::GetDefaultFont(DefaultFontType::UI_SANS,
                                                MsLangId::getConfiguredSystemLanguage(),
                                                GetDefaultFontFlags::OnlyOne);
        aStdFont.SetColor(aTextColor);
        aStdFont.SetFillColor(aBackColor);

        tools::Long nFontHeight = nYStep * 6 / 10;
        if (bPosition)
            nFontHeight = nYStep * 15 / 10;
        aStdFont.SetFontSize(Size(0, nFontHeight));

        SvxNodeNum aNum;
        sal_uInt16 nPreNum = pActNum->GetLevel(0).GetStart();

        if (bPosition)
        {
            // When bPosition == true, draw the preview used in the Writer's "Position" tab
            // This is not used in Impress/Draw

            tools::Long nLineHeight = nFontHeight * 8 / 7;
            sal_uInt8 nStart = 0;
            while (!(nActLevel & (1 << nStart)))
            {
                nStart++;
            }
            if (nStart)
                nStart--;
            sal_uInt8 nEnd = std::min(sal_uInt8(nStart + 3), sal_uInt8(pActNum->GetLevelCount()));
            for (sal_uInt8 nLevel = nStart; nLevel < nEnd; ++nLevel)
            {
                const SvxNumberFormat& rFmt = pActNum->GetLevel(nLevel);
                aNum.GetLevelVal()[nLevel] = rFmt.GetStart();

                tools::Long nXStart(0);
                short nTextOffset(0);
                tools::Long nNumberXPos(0);
                if (rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
                {
                    nXStart = rFmt.GetAbsLSpace() / nWidthRelation;
                    nTextOffset = rFmt.GetCharTextDistance() / nWidthRelation;
                    nNumberXPos = nXStart;
                    tools::Long nFirstLineOffset = (-rFmt.GetFirstLineOffset()) / nWidthRelation;

                    if (nFirstLineOffset <= nNumberXPos)
                        nNumberXPos = nNumberXPos - nFirstLineOffset;
                    else
                        nNumberXPos = 0;
                    // in draw this is valid
                    if (nTextOffset < 0)
                        nNumberXPos = nNumberXPos + nTextOffset;
                }
                else if (rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
                {
                    const tools::Long nTmpNumberXPos(
                        (rFmt.GetIndentAt() + rFmt.GetFirstLineIndent()) / nWidthRelation);
                    if (nTmpNumberXPos < 0)
                    {
                        nNumberXPos = 0;
                    }
                    else
                    {
                        nNumberXPos = nTmpNumberXPos;
                    }
                }

                tools::Long nBulletWidth = 0;
                if (SVX_NUM_BITMAP == (rFmt.GetNumberingType() & (~LINK_TOKEN)))
                {
                    tools::Long nYMiddle = nYStart + (nFontHeight / 2);
                    nBulletWidth = rFmt.IsShowSymbol() ? lcl_DrawGraphic(*pVDev, rFmt, nNumberXPos,
                                                                         nYMiddle, nWidthRelation)
                                                       : 0;
                }
                else if (SVX_NUM_CHAR_SPECIAL == rFmt.GetNumberingType())
                {
                    nBulletWidth = rFmt.IsShowSymbol()
                                       ? lcl_DrawBullet(pVDev.get(), rFmt, nNumberXPos, nYStart,
                                                        aStdFont.GetFontSize())
                                       : 0;
                }
                else
                {
                    pVDev->SetFont(aStdFont);
                    aNum.SetLevel(nLevel);
                    if (pActNum->IsContinuousNumbering())
                        aNum.GetLevelVal()[nLevel] = nPreNum;
                    OUString aText(pActNum->MakeNumString(aNum));
                    vcl::Font aSaveFont = pVDev->GetFont();
                    vcl::Font aColorFont(aSaveFont);
                    Color aTmpBulletColor = rFmt.GetBulletColor();
                    if (aTmpBulletColor == COL_AUTO)
                        aTmpBulletColor
                            = pVDev->GetBackgroundColor().IsDark() ? COL_WHITE : COL_BLACK;
                    else if (pVDev->GetBackgroundColor().IsDark() == aTmpBulletColor.IsDark())
                        aTmpBulletColor
                            = pVDev->GetBackgroundColor().IsDark() ? COL_WHITE : COL_BLACK;
                    aColorFont.SetColor(aTmpBulletColor);
                    pVDev->SetFont(aColorFont);
                    pVDev->DrawText(Point(nNumberXPos, nYStart), aText);
                    pVDev->SetFont(aSaveFont);
                    nBulletWidth = pVDev->GetTextWidth(aText);
                    nPreNum++;
                }
                if (rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT
                    && rFmt.GetLabelFollowedBy() == SvxNumberFormat::SPACE)
                {
                    pVDev->SetFont(aStdFont);
                    OUString aText(' ');
                    pVDev->DrawText(Point(nNumberXPos, nYStart), aText);
                    nBulletWidth = nBulletWidth + pVDev->GetTextWidth(aText);
                }

                tools::Long nTextXPos(0);
                if (rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
                {
                    nTextXPos = nXStart;
                    if (nTextOffset < 0)
                        nTextXPos = nTextXPos + nTextOffset;
                    if (nNumberXPos + nBulletWidth + nTextOffset > nTextXPos)
                        nTextXPos = nNumberXPos + nBulletWidth + nTextOffset;
                }
                else if (rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
                {
                    switch (rFmt.GetLabelFollowedBy())
                    {
                        case SvxNumberFormat::LISTTAB:
                        {
                            nTextXPos = rFmt.GetListtabPos() / nWidthRelation;
                            if (nTextXPos < nNumberXPos + nBulletWidth)
                            {
                                nTextXPos = nNumberXPos + nBulletWidth;
                            }
                        }
                        break;
                        case SvxNumberFormat::SPACE:
                        case SvxNumberFormat::NOTHING:
                        case SvxNumberFormat::NEWLINE:
                        {
                            nTextXPos = nNumberXPos + nBulletWidth;
                        }
                        break;
                    }

                    nXStart = rFmt.GetIndentAt() / nWidthRelation;
                }

                ::tools::Rectangle aRect1(Point(nTextXPos, nYStart + nFontHeight / 2),
                                          Size(aSize.Width() / 2, 2));
                pVDev->SetFillColor(aBackColor);
                pVDev->DrawRect(aRect1);

                ::tools::Rectangle aRect2(Point(nXStart, nYStart + nLineHeight + nFontHeight / 2),
                                          Size(aSize.Width() / 2, 2));
                pVDev->DrawRect(aRect2);
                nYStart += 2 * nLineHeight;
            }
        }
        else
        {
            // When bPosition == false, draw the preview used in Writer's "Customize" tab
            // and in Impress' "Bullets and Numbering" dialog

            //#i5153# painting gray or black rectangles as 'normal' numbering text
            tools::Long nWidth = pVDev->GetTextWidth(u"Preview"_ustr);
            tools::Long nTextHeight = pVDev->GetTextHeight();
            tools::Long nRectHeight = nTextHeight * 2 / 3;
            tools::Long nTopOffset = nTextHeight - nRectHeight;
            Color aSelRectColor = pVDev->GetBackgroundColor().IsDark() ? COL_WHITE : COL_BLACK;

            for (sal_uInt16 nLevel = 0; nLevel < pActNum->GetLevelCount();
                 ++nLevel, nYStart = nYStart + nYStep)
            {
                const SvxNumberFormat& rFmt = pActNum->GetLevel(nLevel);
                aNum.GetLevelVal()[nLevel] = rFmt.GetStart();
                tools::Long nXStart(0);
                pVDev->SetFillColor(aBackColor);

                if (rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_WIDTH_AND_POSITION)
                {
                    nXStart = rFmt.GetAbsLSpace() / nWidthRelation;
                }
                else if (rFmt.GetPositionAndSpaceMode() == SvxNumberFormat::LABEL_ALIGNMENT)
                {
                    const tools::Long nTmpXStart((rFmt.GetIndentAt() + rFmt.GetFirstLineIndent())
                                                 / nWidthRelation);
                    if (nTmpXStart < 0)
                    {
                        nXStart = 0;
                    }
                    else
                    {
                        nXStart = nTmpXStart;
                    }
                }
                nXStart /= 2;
                nXStart += 2;
                tools::Long nTextOffset = 2 * nXStep;
                if (SVX_NUM_BITMAP == (rFmt.GetNumberingType() & (~LINK_TOKEN)))
                {
                    if (rFmt.IsShowSymbol())
                    {
                        tools::Long nYMiddle = nYStart + (nFontHeight / 2);
                        nTextOffset
                            = lcl_DrawGraphic(*pVDev, rFmt, nXStart, nYMiddle, nWidthRelation);
                        nTextOffset = nTextOffset + nXStep;
                    }
                }
                else if (SVX_NUM_CHAR_SPECIAL == rFmt.GetNumberingType())
                {
                    if (rFmt.IsShowSymbol())
                    {
                        nTextOffset = lcl_DrawBullet(pVDev.get(), rFmt, nXStart, nYStart,
                                                     aStdFont.GetFontSize());
                        nTextOffset = nTextOffset + nXStep;
                    }
                }
                else
                {
                    vcl::Font aFont(aStdFont);
                    Size aTmpSize(aStdFont.GetFontSize());
                    if (pActNum->IsFeatureSupported(SvxNumRuleFlags::BULLET_REL_SIZE))
                    {
                        aTmpSize.setWidth(aTmpSize.Width() * (rFmt.GetBulletRelSize()));
                        aTmpSize.setWidth(aTmpSize.Width() / 100);
                        aTmpSize.setHeight(aTmpSize.Height() * (rFmt.GetBulletRelSize()));
                        aTmpSize.setHeight(aTmpSize.Height() / 100);
                    }
                    if (!aTmpSize.Height())
                        aTmpSize.setHeight(1);
                    aFont.SetFontSize(aTmpSize);
                    Color aTmpBulletColor = rFmt.GetBulletColor();
                    if (aTmpBulletColor == COL_AUTO)
                        aTmpBulletColor
                            = pVDev->GetBackgroundColor().IsDark() ? COL_WHITE : COL_BLACK;
                    else if (pVDev->GetBackgroundColor().IsDark() == aTmpBulletColor.IsDark())
                        aTmpBulletColor
                            = pVDev->GetBackgroundColor().IsDark() ? COL_WHITE : COL_BLACK;
                    aFont.SetColor(aTmpBulletColor);
                    pVDev->SetFont(aFont);
                    aNum.SetLevel(nLevel);
                    if (pActNum->IsContinuousNumbering())
                        aNum.GetLevelVal()[nLevel] = nPreNum;
                    OUString aText(pActNum->MakeNumString(aNum));
                    tools::Long nY = nYStart;
                    nY -= (pVDev->GetTextHeight() - nTextHeight
                           - pVDev->GetFontMetric().GetDescent());
                    pVDev->DrawText(Point(nXStart, nY), aText);
                    nTextOffset = pVDev->GetTextWidth(aText);
                    nTextOffset = nTextOffset + nXStep;
                    nPreNum++;
                    pVDev->SetFont(aStdFont);
                }
                //#i5153# the selected rectangle(s) should be black
                if (0 != (nActLevel & (1 << nLevel)))
                {
                    pVDev->SetFillColor(aSelRectColor);
                    pVDev->SetLineColor(aSelRectColor);
                }
                else
                {
                    //#i5153# unselected levels are gray
                    pVDev->SetFillColor(aLineColor);
                    pVDev->SetLineColor(aLineColor);
                }
                ::tools::Rectangle aRect1(Point(nXStart + nTextOffset, nYStart + nTopOffset),
                                          Size(nWidth, nRectHeight));
                pVDev->DrawRect(aRect1);
            }
        }
    }
    rRenderContext.DrawOutDev(Point(), aSize, Point(), aSize, *pVDev);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
