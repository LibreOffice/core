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

#include <unobrushitemhelper.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xbtmpit.hxx>
#include <svx/xgrscit.hxx>
#include <svx/xflbmtit.hxx>
#include <svx/xflbmpit.hxx>
#include <svx/xflbmsxy.hxx>
#include <svx/xflbmsxy.hxx>
#include <svx/xflftrit.hxx>
#include <svx/xsflclit.hxx>
#include <svx/xflbmsli.hxx>
#include <svx/xflbtoxy.hxx>
#include <svx/xflbstit.hxx>
#include <svx/xflboxy.hxx>
#include <svx/xflbckit.hxx>
#include <svx/unoshape.hxx>
#include <hintids.hxx>
#include <svx/xflclit.hxx>
#include <svx/xfltrit.hxx>
#include <svx/xflhtit.hxx>

namespace sw {

//UUUU
void setSvxBrushItemAsFillAttributesToTargetSet(const SvxBrushItem& rBrush, SfxItemSet& rToSet)
{
    if(0xff != rBrush.GetColor().GetTransparency())
    {
        const Color aColor(rBrush.GetColor().GetRGBColor());
        const sal_uInt8 nTransparency(rBrush.GetColor().GetTransparency());

        rToSet.Put(XFillStyleItem(XFILL_SOLID));
        rToSet.Put(XFillColorItem(OUString(), aColor));

        if(0xff != nTransparency)
        {
            // nTransparency is in range [0..255]
            rToSet.Put(XFillTransparenceItem((((sal_Int32)nTransparency * 100) + 127) / 255));
        }
    }
    else if(GPOS_NONE != rBrush.GetGraphicPos())
    {
        const Graphic* pGraphic = rBrush.GetGraphic();

        if(pGraphic)
        {
            // set fill style and graphic itself
            rToSet.Put(XFillStyleItem(XFILL_BITMAP));
            rToSet.Put(XFillBitmapItem(OUString(), *pGraphic));

            // set defaults
            // already pool default rToSet.Put(XFillBmpPosItem(RP_MM));
            // already pool default rToSet.Put(XFillBmpTileOffsetXItem(0));
            // already pool default rToSet.Put(XFillBmpTileOffsetYItem(0));
            // already pool default rToSet.Put(XFillBmpPosOffsetXItem(0));
            // already pool default rToSet.Put(XFillBmpPosOffsetYItem(0));
            // already pool default rToSet.Put(XFillBmpSizeLogItem(true));
            // already pool default rToSet.Put(XFillBmpSizeXItem(0));
            // already pool default rToSet.Put(XFillBmpSizeYItem(0));

            if(GPOS_AREA == rBrush.GetGraphicPos())
            {
                // stretch, also means no tile (both items are defaulted to true)
                // rToSet.Put(XFillBmpStretchItem(true));
                rToSet.Put(XFillBmpTileItem(false));

                // default for strech is also top-left, but this will not be visible
                // rToSet.Put(XFillBmpPosItem(RP_LT));
            }
            else if(GPOS_TILED == rBrush.GetGraphicPos())
            {
                // tiled, also means no stretch (both items are defaulted to true)
                rToSet.Put(XFillBmpStretchItem(false));
                //rToSet.Put(XFillBmpTileItem(true));

                // default for tiled is top-left
                rToSet.Put(XFillBmpPosItem(RP_LT));
            }
            else
            {
                // everything else means no tile and no stretch
                rToSet.Put(XFillBmpStretchItem(false));
                rToSet.Put(XFillBmpTileItem(false));

                switch(rBrush.GetGraphicPos())
                {
                    case GPOS_LT: rToSet.Put(XFillBmpPosItem(RP_LT)); break;
                    case GPOS_MT: rToSet.Put(XFillBmpPosItem(RP_MT)); break;
                    case GPOS_RT: rToSet.Put(XFillBmpPosItem(RP_RT)); break;
                    case GPOS_LM: rToSet.Put(XFillBmpPosItem(RP_LM)); break;
                    case GPOS_MM: rToSet.Put(XFillBmpPosItem(RP_MM)); break;
                    case GPOS_RM: rToSet.Put(XFillBmpPosItem(RP_RM)); break;
                    case GPOS_LB: rToSet.Put(XFillBmpPosItem(RP_LB)); break;
                    case GPOS_MB: rToSet.Put(XFillBmpPosItem(RP_MB)); break;
                    case GPOS_RB: rToSet.Put(XFillBmpPosItem(RP_RB)); break;
                    default: break; // already handled GPOS_AREA, GPOS_TILED and GPOS_NONE
                }
            }

            // check for transparency
            const sal_Int8 nTransparency(rBrush.getGraphicTransparency());

            if(0 != nTransparency)
            {
                // nTransparency is in range [0..100]
                rToSet.Put(XFillTransparenceItem(nTransparency));
            }
        }
        else
        {
            OSL_ENSURE(false, "Could not get Graphic from SvxBrushItem (!)");
        }
    }
}

//UUUU
sal_uInt16 getTransparenceForSvxBrushItem(const SfxItemSet& rSourceSet, sal_Bool bSearchInParents)
{
    sal_uInt16 nFillTransparence(static_cast< const XFillTransparenceItem& >(rSourceSet.Get(XATTR_FILLTRANSPARENCE, bSearchInParents)).GetValue());
    const SfxPoolItem* pGradientItem = 0;

    if(SFX_ITEM_SET == rSourceSet.GetItemState(XATTR_FILLFLOATTRANSPARENCE, bSearchInParents, &pGradientItem)
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

//UUUU
SvxBrushItem getSvxBrushItemForSolid(const SfxItemSet& rSourceSet, sal_Bool bSearchInParents)
{
    Color aFillColor(static_cast< const XFillColorItem& >(rSourceSet.Get(XATTR_FILLCOLOR, bSearchInParents)).GetColorValue());

    // get evtl. mixed transparence
    const sal_uInt16 nFillTransparence(getTransparenceForSvxBrushItem(rSourceSet, bSearchInParents));

    if(0 != nFillTransparence)
    {
        // nFillTransparence is in range [0..100] and needs to be in [0..255] unsigned
        aFillColor.SetTransparency(static_cast< sal_uInt8 >((nFillTransparence * 255) / 100));
    }

    return SvxBrushItem(aFillColor, RES_BACKGROUND);
}

//UUUU
SvxBrushItem getSvxBrushItemFromSourceSet(const SfxItemSet& rSourceSet, sal_Bool bSearchInParents)
{
    SvxBrushItem aRetval(RES_BACKGROUND);

    const XFillStyleItem* pXFillStyleItem(static_cast< const XFillStyleItem*  >(rSourceSet.GetItem(XATTR_FILLSTYLE, bSearchInParents)));

    if(!pXFillStyleItem)
    {
        return aRetval;
    }

    switch(pXFillStyleItem->GetValue())
    {
        case XFILL_NONE:
        {
            // done; return default item
            break;
        }
        case XFILL_SOLID:
        {
            // create SvxBrushItem with fill color
            aRetval = getSvxBrushItemForSolid(rSourceSet, bSearchInParents);
            break;
        }
        case XFILL_GRADIENT:
        {
            // cannot be directly supported, but do the best possible
            const XGradient aXGradient(static_cast< const XFillGradientItem& >(rSourceSet.Get(XATTR_FILLGRADIENT)).GetGradientValue());
            const basegfx::BColor aStartColor(aXGradient.GetStartColor().getBColor() * (aXGradient.GetStartIntens() * 0.01));
            const basegfx::BColor aEndColor(aXGradient.GetEndColor().getBColor() * (aXGradient.GetEndIntens() * 0.01));

            // use half/half mixed color from gradient start and end
            Color aMixedColor((aStartColor + aEndColor) * 0.5);

            // get evtl. mixed transparence
            const sal_uInt16 nFillTransparence(getTransparenceForSvxBrushItem(rSourceSet, bSearchInParents));

            if(0 != nFillTransparence)
            {
                // nFillTransparence is in range [0..100] and needs to be in [0..255] unsigned
                aMixedColor.SetTransparency(static_cast< sal_uInt8 >((nFillTransparence * 255) / 100));
            }

            aRetval = SvxBrushItem(aMixedColor, RES_BACKGROUND);
            break;
        }
        case XFILL_HATCH:
        {
            // cannot be directly supported, but do the best possible
            const XHatch& rHatch(static_cast< const XFillHatchItem& >(rSourceSet.Get(XATTR_FILLHATCH)).GetHatchValue());
            const bool bFillBackground(static_cast< const XFillBackgroundItem& >(rSourceSet.Get(XATTR_FILLBACKGROUND)).GetValue());

            if(bFillBackground)
            {
                // hatch is background-filled, use FillColor as if XFILL_SOLID
                aRetval = getSvxBrushItemForSolid(rSourceSet, bSearchInParents);
            }
            else
            {
                // hatch is not background-filled and using hatch color would be too dark; compensate
                // somewhat by making it more transparent
                Color aHatchColor(rHatch.GetColor());

                // get evtl. mixed transparence
                sal_uInt16 nFillTransparence(getTransparenceForSvxBrushItem(rSourceSet, bSearchInParents));

                // take half orig transparence, add half transparent, clamp result
                nFillTransparence = basegfx::clamp((sal_uInt16)((nFillTransparence / 2) + 50), (sal_uInt16)0, (sal_uInt16)255);

                // nFillTransparence is in range [0..100] and needs to be in [0..255] unsigned
                aHatchColor.SetTransparency(static_cast< sal_uInt8 >((nFillTransparence * 255) / 100));

                aRetval = SvxBrushItem(aHatchColor, RES_BACKGROUND);
            }

            break;
        }
        case XFILL_BITMAP:
        {
            // create SvxBrushItem with bitmap info and flags
            const XFillBitmapItem& rBmpItm = static_cast< const XFillBitmapItem& >(rSourceSet.Get(XATTR_FILLBITMAP, bSearchInParents));
            const Graphic aGraphic(rBmpItm.GetGraphicObject().GetGraphic());

            if(GRAPHIC_NONE != aGraphic.GetType())
            {
                // get graphic position
                SvxGraphicPosition aSvxGraphicPosition(GPOS_NONE);
                const XFillBmpStretchItem& rStretchItem = static_cast< const XFillBmpStretchItem& >(rSourceSet.Get(XATTR_FILLBMP_STRETCH, bSearchInParents));
                const XFillBmpTileItem& rTileItem = static_cast< const XFillBmpTileItem& >(rSourceSet.Get(XATTR_FILLBMP_TILE, bSearchInParents));

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
                    const XFillBmpPosItem& rPosItem = static_cast< const XFillBmpPosItem& >(rSourceSet.Get(XATTR_FILLBMP_POS, bSearchInParents));

                    switch(rPosItem.GetValue())
                    {
                        case RP_LT: aSvxGraphicPosition = GPOS_LT; break;
                        case RP_MT: aSvxGraphicPosition = GPOS_MT; break;
                        case RP_RT: aSvxGraphicPosition = GPOS_RT; break;
                        case RP_LM: aSvxGraphicPosition = GPOS_LM; break;
                        case RP_MM: aSvxGraphicPosition = GPOS_MM; break;
                        case RP_RM: aSvxGraphicPosition = GPOS_RM; break;
                        case RP_LB: aSvxGraphicPosition = GPOS_LB; break;
                        case RP_MB: aSvxGraphicPosition = GPOS_MB; break;
                        case RP_RB: aSvxGraphicPosition = GPOS_RB; break;
                    }
                }

                // create with given graphic and position
                aRetval = SvxBrushItem(aGraphic, aSvxGraphicPosition, RES_BACKGROUND);

                // get evtl. mixed transparence
                const sal_uInt16 nFillTransparence(getTransparenceForSvxBrushItem(rSourceSet, bSearchInParents));

                if(0 != nFillTransparence)
                {
                    // nFillTransparence is in range [0..100] and needs to be in [0..100] signed
                    aRetval.setGraphicTransparency(static_cast< sal_Int8 >(nFillTransparence));
                }
            }

            break;
        }
    }

    return aRetval;
}

} // namespace sw

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
