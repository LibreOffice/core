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

#include <sal/config.h>

#include <algorithm>

#include <svx/unobrushitemhelper.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xgrscit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbmpit.hxx>
#include <svx/xflbmsxy.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xsflclit.hxx>
#include <svx/xflbmsli.hxx>
#include <svx/xflbtoxy.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflboxy.hxx>
#include <svx/xflbckit.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/unoshape.hxx>

using namespace com::sun::star;

void setSvxBrushItemAsFillAttributesToTargetSet(const SvxBrushItem& rBrush, SfxItemSet& rToSet)
{
    // Clear all items from the DrawingLayer FillStyle range (if we have any). All
    // items that need to be set will be set as hard attributes
    for(sal_uInt16 a(XATTR_FILL_FIRST); rToSet.Count() && a < XATTR_FILL_LAST; a++)
    {
        rToSet.ClearItem(a);
    }

    const sal_uInt8 nTransparency(rBrush.GetColor().GetTransparency());

    // tdf#89478 check for image first
    if (GPOS_NONE != rBrush.GetGraphicPos())
    {
        // we have a graphic fill, set fill style
        rToSet.Put(XFillStyleItem(drawing::FillStyle_BITMAP));

        // set graphic (if available)
        const Graphic* pGraphic = rBrush.GetGraphic();

        if(pGraphic)
        {
            rToSet.Put(XFillBitmapItem(OUString(), *pGraphic));
        }
        else
        {
            OSL_ENSURE(false, "Could not get Graphic from SvxBrushItem (!)");
        }

        if(GPOS_AREA == rBrush.GetGraphicPos())
        {
            // stretch, also means no tile (both items are defaulted to true)
            rToSet.Put(XFillBmpStretchItem(true));
            rToSet.Put(XFillBmpTileItem(false));

            // default for stretch is also top-left, but this will not be visible
            rToSet.Put(XFillBmpPosItem(RectPoint::LT));
        }
        else if(GPOS_TILED == rBrush.GetGraphicPos())
        {
            // tiled, also means no stretch (both items are defaulted to true)
            rToSet.Put(XFillBmpStretchItem(false));
            rToSet.Put(XFillBmpTileItem(true));

            // default for tiled is top-left
            rToSet.Put(XFillBmpPosItem(RectPoint::LT));
        }
        else
        {
            // everything else means no tile and no stretch
            rToSet.Put(XFillBmpStretchItem(false));
            rToSet.Put(XFillBmpTileItem(false));

            RectPoint aRectPoint(RectPoint::MM);

            switch(rBrush.GetGraphicPos())
            {
                case GPOS_LT: aRectPoint = RectPoint::LT; break;
                case GPOS_MT: aRectPoint = RectPoint::MT; break;
                case GPOS_RT: aRectPoint = RectPoint::RT; break;
                case GPOS_LM: aRectPoint = RectPoint::LM; break;
                case GPOS_MM: aRectPoint = RectPoint::MM; break;
                case GPOS_RM: aRectPoint = RectPoint::RM; break;
                case GPOS_LB: aRectPoint = RectPoint::LB; break;
                case GPOS_MB: aRectPoint = RectPoint::MB; break;
                case GPOS_RB: aRectPoint = RectPoint::RB; break;
                default: break; // GPOS_NONE, GPOS_AREA and GPOS_TILED already handled
            }

            rToSet.Put(XFillBmpPosItem(aRectPoint));
        }

        // check for graphic's transparency
        const sal_Int8 nGraphicTransparency(rBrush.getGraphicTransparency());

        if(0 != nGraphicTransparency)
        {
            // nGraphicTransparency is in range [0..100]
            rToSet.Put(XFillTransparenceItem(nGraphicTransparency));
        }
    }
    else if (0xff != nTransparency)
    {
        // we have a color fill
        const Color aColor(rBrush.GetColor().GetRGBColor());

        rToSet.Put(XFillStyleItem(drawing::FillStyle_SOLID));
        rToSet.Put(XFillColorItem(OUString(), aColor));

        // #125189# nTransparency is in range [0..254], convert to [0..100] which is used in
        // XFillTransparenceItem (caution with the range which is in an *item-specific* range)
        rToSet.Put(XFillTransparenceItem(((static_cast<sal_Int32>(nTransparency) * 100) + 127) / 254));
    }
    else
    {
        // GPOS_NONE == rBrush.GetGraphicPos() && 0xff == rBrush.GetColor().GetTransparency(),
        // still need to rescue the color used. There are sequences used on the UNO API at
        // import time (OLE. e.g. chart) which first set RGB color (MID_BACK_COLOR_R_G_B,
        // color stays transparent) and then set transparency (MID_BACK_COLOR_TRANSPARENCY)
        // to zero later. When not saving the color, it will be lost.
        // Also need to set the FillStyle to NONE to express the 0xff transparency flag; this
        // is needed when e.g. first transparency is set to 0xff and then a Graphic gets set.
        // When not changing the FillStyle, the next getSvxBrushItemFromSourceSet *will* return
        // to drawing::FillStyle_SOLID with the rescued color.
        const Color aColor(rBrush.GetColor().GetRGBColor());

        rToSet.Put(XFillStyleItem(drawing::FillStyle_NONE));
        rToSet.Put(XFillColorItem(OUString(), aColor));
    }
}

static sal_uInt16 getTransparenceForSvxBrushItem(const SfxItemSet& rSourceSet, bool bSearchInParents)
{
    sal_uInt16 nFillTransparence(rSourceSet.Get(XATTR_FILLTRANSPARENCE, bSearchInParents).GetValue());
    const SfxPoolItem* pGradientItem = nullptr;

    if(SfxItemState::SET == rSourceSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, bSearchInParents, &pGradientItem)
        && static_cast< const XFillFloatTransparenceItem* >(pGradientItem)->IsEnabled())
    {
        const XGradient& rGradient = static_cast< const XFillFloatTransparenceItem* >(pGradientItem)->GetGradientValue();
        const sal_uInt16 nStartLuminance(rGradient.GetStartColor().GetLuminance());
        const sal_uInt16 nEndLuminance(rGradient.GetEndColor().GetLuminance());

        // luminance is [0..255], transparence needs to be in [0..100].Maximum is 51200, thus sal_uInt16 is okay to use
        nFillTransparence = static_cast< sal_uInt16 >(((nStartLuminance + nEndLuminance) * 100) / 512);
    }

    return nFillTransparence;
}

static SvxBrushItem getSvxBrushItemForSolid(const SfxItemSet& rSourceSet, bool bSearchInParents, sal_uInt16 nBackgroundID)
{
    Color aFillColor(rSourceSet.Get(XATTR_FILLCOLOR, bSearchInParents).GetColorValue());

    // get evtl. mixed transparence
    const sal_uInt16 nFillTransparence(getTransparenceForSvxBrushItem(rSourceSet, bSearchInParents));

    if(0 != nFillTransparence)
    {
        // #i125189# nFillTransparence is in range [0..100] and needs to be in [0..254] unsigned
        // It is necessary to use the maximum of 0xfe for transparence for the SvxBrushItem
        // since the oxff value is used for special purposes (like no fill and derive from parent)
        const sal_uInt8 aTargetTrans(std::min(sal_uInt8(0xfe), static_cast< sal_uInt8 >((nFillTransparence * 254) / 100)));

        aFillColor.SetTransparency(aTargetTrans);
    }

    return SvxBrushItem(aFillColor, nBackgroundID);
}

SvxBrushItem getSvxBrushItemFromSourceSet(const SfxItemSet& rSourceSet, sal_uInt16 nBackgroundID, bool bSearchInParents, bool bXMLImportHack)
{
    const XFillStyleItem* pXFillStyleItem(rSourceSet.GetItem<XFillStyleItem>(XATTR_FILLSTYLE, bSearchInParents));

    if(!pXFillStyleItem || drawing::FillStyle_NONE == pXFillStyleItem->GetValue())
    {
        // no fill, still need to rescue the evtl. set RGB color, but use as transparent color (we have drawing::FillStyle_NONE)
        Color aFillColor(rSourceSet.Get(XATTR_FILLCOLOR, bSearchInParents).GetColorValue());

        // for writerfilter: when fill style is none, then don't allow anything other than 0 or auto.
        if (!bXMLImportHack && aFillColor != Color(0))
            aFillColor = COL_AUTO;

        aFillColor.SetTransparency(0xff);

        return SvxBrushItem(aFillColor, nBackgroundID);
    }

    SvxBrushItem aRetval(nBackgroundID);

    switch(pXFillStyleItem->GetValue())
    {
        default:
        case drawing::FillStyle_NONE:
        {
            // already handled above, can not happen again
            break;
        }
        case drawing::FillStyle_SOLID:
        {
            // create SvxBrushItem with fill color
            aRetval = getSvxBrushItemForSolid(rSourceSet, bSearchInParents, nBackgroundID);
            break;
        }
        case drawing::FillStyle_GRADIENT:
        {
            // cannot be directly supported, but do the best possible
            const XGradient aXGradient(rSourceSet.Get(XATTR_FILLGRADIENT).GetGradientValue());
            const basegfx::BColor aStartColor(aXGradient.GetStartColor().getBColor() * (aXGradient.GetStartIntens() * 0.01));
            const basegfx::BColor aEndColor(aXGradient.GetEndColor().getBColor() * (aXGradient.GetEndIntens() * 0.01));

            // use half/half mixed color from gradient start and end
            Color aMixedColor((aStartColor + aEndColor) * 0.5);

            // get evtl. mixed transparence
            const sal_uInt16 nFillTransparence(getTransparenceForSvxBrushItem(rSourceSet, bSearchInParents));

            if(0 != nFillTransparence)
            {
                // #i125189# nFillTransparence is in range [0..100] and needs to be in [0..254] unsigned
                // It is necessary to use the maximum of 0xfe for transparence for the SvxBrushItem
                // since the oxff value is used for special purposes (like no fill and derive from parent)
                const sal_uInt8 aTargetTrans(std::min(sal_uInt8(0xfe), static_cast< sal_uInt8 >((nFillTransparence * 254) / 100)));

                aMixedColor.SetTransparency(aTargetTrans);
            }

            aRetval = SvxBrushItem(aMixedColor, nBackgroundID);
            break;
        }
        case drawing::FillStyle_HATCH:
        {
            // cannot be directly supported, but do the best possible
            const XHatch& rHatch(rSourceSet.Get(XATTR_FILLHATCH).GetHatchValue());
            const bool bFillBackground(rSourceSet.Get(XATTR_FILLBACKGROUND).GetValue());

            if(bFillBackground)
            {
                // hatch is background-filled, use FillColor as if drawing::FillStyle_SOLID
                aRetval = getSvxBrushItemForSolid(rSourceSet, bSearchInParents, nBackgroundID);
            }
            else
            {
                // hatch is not background-filled and using hatch color would be too dark; compensate
                // somewhat by making it more transparent
                Color aHatchColor(rHatch.GetColor());

                // get evtl. mixed transparence
                sal_uInt16 nFillTransparence(getTransparenceForSvxBrushItem(rSourceSet, bSearchInParents));

                // take half orig transparence, add half transparent, clamp result
                nFillTransparence = std::clamp(static_cast<sal_uInt16>((nFillTransparence / 2) + 50), sal_uInt16(0), sal_uInt16(255));

                // #i125189# nFillTransparence is in range [0..100] and needs to be in [0..254] unsigned
                // It is necessary to use the maximum of 0xfe for transparence for the SvxBrushItem
                // since the oxff value is used for special purposes (like no fill and derive from parent)
                const sal_uInt8 aTargetTrans(std::min(sal_uInt8(0xfe), static_cast< sal_uInt8 >((nFillTransparence * 254) / 100)));

                aHatchColor.SetTransparency(aTargetTrans);
                aRetval = SvxBrushItem(aHatchColor, nBackgroundID);
            }

            break;
        }
        case drawing::FillStyle_BITMAP:
        {
            // create SvxBrushItem with bitmap info and flags
            const XFillBitmapItem& rBmpItm = rSourceSet.Get(XATTR_FILLBITMAP, bSearchInParents);
            const Graphic aGraphic(rBmpItm.GetGraphicObject().GetGraphic());

            // continue idependent of evtl. GraphicType::NONE as aGraphic.GetType(), we still need to rescue positions
            SvxGraphicPosition aSvxGraphicPosition(GPOS_NONE);
            const XFillBmpStretchItem& rStretchItem = rSourceSet.Get(XATTR_FILLBMP_STRETCH, bSearchInParents);
            const XFillBmpTileItem& rTileItem = rSourceSet.Get(XATTR_FILLBMP_TILE, bSearchInParents);

            if(rTileItem.GetValue())
            {
                aSvxGraphicPosition = GPOS_TILED;
            }
            else if(rStretchItem.GetValue())
            {
                aSvxGraphicPosition = GPOS_AREA;
            }
            else
            {
                const XFillBmpPosItem& rPosItem = rSourceSet.Get(XATTR_FILLBMP_POS, bSearchInParents);

                switch(rPosItem.GetValue())
                {
                    case RectPoint::LT: aSvxGraphicPosition = GPOS_LT; break;
                    case RectPoint::MT: aSvxGraphicPosition = GPOS_MT; break;
                    case RectPoint::RT: aSvxGraphicPosition = GPOS_RT; break;
                    case RectPoint::LM: aSvxGraphicPosition = GPOS_LM; break;
                    case RectPoint::MM: aSvxGraphicPosition = GPOS_MM; break;
                    case RectPoint::RM: aSvxGraphicPosition = GPOS_RM; break;
                    case RectPoint::LB: aSvxGraphicPosition = GPOS_LB; break;
                    case RectPoint::MB: aSvxGraphicPosition = GPOS_MB; break;
                    case RectPoint::RB: aSvxGraphicPosition = GPOS_RB; break;
                }
            }

            // create with given graphic and position
            aRetval = SvxBrushItem(aGraphic, aSvxGraphicPosition, nBackgroundID);

            // get evtl. mixed transparence
            const sal_uInt16 nFillTransparence(getTransparenceForSvxBrushItem(rSourceSet, bSearchInParents));

            if(0 != nFillTransparence)
            {
                // #i125189# nFillTransparence is in range [0..100] and needs to be in [0..100] signed
                aRetval.setGraphicTransparency(static_cast< sal_Int8 >(nFillTransparence));
            }

            break;
        }
    }

    return aRetval;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
