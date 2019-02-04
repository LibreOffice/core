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

#include <svx/xtable.hxx>
#include <svx/xattr.hxx>
#include <svx/xpool.hxx>
#include <svx/svdattr.hxx>
#include <svx/svxids.hrc>
#include <svl/itemset.hxx>

XOutdevItemPool::XOutdevItemPool(SfxItemPool* _pMaster)
    : SfxItemPool("XOutdevItemPool", SDRATTR_START, SDRATTR_END, nullptr, nullptr)
    , mpLocalPoolDefaults(new std::vector<SfxPoolItem*>(SDRATTR_END - SDRATTR_START + 1))
    , mpLocalItemInfos(new SfxItemInfo[SDRATTR_END - SDRATTR_START + 1])
{
    // prepare some defaults
    const OUString aNullStr;
    const Graphic aNullGraphic;
    const basegfx::B2DPolyPolygon aNullPol;
    const Color aNullLineCol(COL_DEFAULT_SHAPE_STROKE); // #i121448# Use defined default color
    const Color aNullFillCol(COL_DEFAULT_SHAPE_FILLING); // #i121448# Use defined default color
    const Color aNullShadowCol(COL_LIGHTGRAY);
    const XDash aNullDash;
    const XGradient aNullGrad(COL_BLACK, COL_WHITE);
    const XHatch aNullHatch(aNullLineCol);

    // get master pointer, evtl. add myself to the end of the pools
    if(!_pMaster)
    {
        _pMaster = this;
    }
    else
    {
        SfxItemPool* pParent = _pMaster;

        while(pParent->GetSecondaryPool())
        {
            pParent = pParent->GetSecondaryPool();
        }

        pParent->SetSecondaryPool(this);
    }

    // prepare PoolDefaults
    std::vector<SfxPoolItem*>& rPoolDefaults = *mpLocalPoolDefaults;
    rPoolDefaults[XATTR_LINESTYLE          -XATTR_START] = new XLineStyleItem;
    rPoolDefaults[XATTR_LINEDASH           -XATTR_START] = new XLineDashItem(aNullDash);
    rPoolDefaults[XATTR_LINEWIDTH          -XATTR_START] = new XLineWidthItem;
    rPoolDefaults[XATTR_LINECOLOR          -XATTR_START] = new XLineColorItem(aNullStr,aNullLineCol);
    rPoolDefaults[XATTR_LINESTART          -XATTR_START] = new XLineStartItem(aNullPol);
    rPoolDefaults[XATTR_LINEEND            -XATTR_START] = new XLineEndItem  (aNullPol);
    rPoolDefaults[XATTR_LINESTARTWIDTH     -XATTR_START] = new XLineStartWidthItem;
    rPoolDefaults[XATTR_LINEENDWIDTH       -XATTR_START] = new XLineEndWidthItem;
    rPoolDefaults[XATTR_LINESTARTCENTER    -XATTR_START] = new XLineStartCenterItem;
    rPoolDefaults[XATTR_LINEENDCENTER      -XATTR_START] = new XLineEndCenterItem;
    rPoolDefaults[XATTR_LINETRANSPARENCE   -XATTR_START] = new XLineTransparenceItem;
    rPoolDefaults[XATTR_LINEJOINT            -XATTR_START] = new XLineJointItem;
    rPoolDefaults[XATTR_LINECAP            -XATTR_START] = new XLineCapItem;
    rPoolDefaults[XATTR_FILLSTYLE                -XATTR_START] = new XFillStyleItem;
    rPoolDefaults[XATTR_FILLCOLOR                -XATTR_START] = new XFillColorItem   (aNullStr,aNullFillCol);
    rPoolDefaults[XATTR_FILLGRADIENT         -XATTR_START] = new XFillGradientItem(aNullGrad);
    rPoolDefaults[XATTR_FILLHATCH                -XATTR_START] = new XFillHatchItem   (aNullHatch);
    rPoolDefaults[XATTR_FILLBITMAP               -XATTR_START] = new XFillBitmapItem  (aNullGraphic);
    rPoolDefaults[XATTR_FILLTRANSPARENCE     -XATTR_START] = new XFillTransparenceItem;
    rPoolDefaults[XATTR_GRADIENTSTEPCOUNT        -XATTR_START] = new XGradientStepCountItem;
    rPoolDefaults[XATTR_FILLBMP_TILE         -XATTR_START] = new XFillBmpTileItem;
    rPoolDefaults[XATTR_FILLBMP_POS          -XATTR_START] = new XFillBmpPosItem;
    rPoolDefaults[XATTR_FILLBMP_SIZEX            -XATTR_START] = new XFillBmpSizeXItem;
    rPoolDefaults[XATTR_FILLBMP_SIZEY            -XATTR_START] = new XFillBmpSizeYItem;
    rPoolDefaults[XATTR_FILLBMP_SIZELOG      -XATTR_START] = new XFillBmpSizeLogItem;
    rPoolDefaults[XATTR_FILLBMP_TILEOFFSETX  -XATTR_START] = new XFillBmpTileOffsetXItem;
    rPoolDefaults[XATTR_FILLBMP_TILEOFFSETY  -XATTR_START] = new XFillBmpTileOffsetYItem;
    rPoolDefaults[XATTR_FILLBMP_STRETCH      -XATTR_START] = new XFillBmpStretchItem;
    rPoolDefaults[XATTR_FILLBMP_POSOFFSETX       -XATTR_START] = new XFillBmpPosOffsetXItem;
    rPoolDefaults[XATTR_FILLBMP_POSOFFSETY       -XATTR_START] = new XFillBmpPosOffsetYItem;
    rPoolDefaults[XATTR_FILLFLOATTRANSPARENCE    -XATTR_START] = new XFillFloatTransparenceItem( aNullGrad, false );
    rPoolDefaults[XATTR_SECONDARYFILLCOLOR       -XATTR_START] = new XSecondaryFillColorItem(aNullStr, aNullFillCol);
    rPoolDefaults[XATTR_FILLBACKGROUND           -XATTR_START] = new XFillBackgroundItem;
    rPoolDefaults[XATTR_FORMTXTSTYLE       -XATTR_START] = new XFormTextStyleItem;
    rPoolDefaults[XATTR_FORMTXTADJUST      -XATTR_START] = new XFormTextAdjustItem;
    rPoolDefaults[XATTR_FORMTXTDISTANCE    -XATTR_START] = new XFormTextDistanceItem;
    rPoolDefaults[XATTR_FORMTXTSTART       -XATTR_START] = new XFormTextStartItem;
    rPoolDefaults[XATTR_FORMTXTMIRROR      -XATTR_START] = new XFormTextMirrorItem;
    rPoolDefaults[XATTR_FORMTXTOUTLINE     -XATTR_START] = new XFormTextOutlineItem;
    rPoolDefaults[XATTR_FORMTXTSHADOW      -XATTR_START] = new XFormTextShadowItem;
    rPoolDefaults[XATTR_FORMTXTSHDWCOLOR   -XATTR_START] = new XFormTextShadowColorItem(aNullStr,aNullShadowCol);
    rPoolDefaults[XATTR_FORMTXTSHDWXVAL    -XATTR_START] = new XFormTextShadowXValItem;
    rPoolDefaults[XATTR_FORMTXTSHDWYVAL    -XATTR_START] = new XFormTextShadowYValItem;
    rPoolDefaults[XATTR_FORMTXTHIDEFORM    -XATTR_START] = new XFormTextHideFormItem;
    rPoolDefaults[XATTR_FORMTXTSHDWTRANSP  -XATTR_START] = new XFormTextShadowTranspItem;

    // create SetItems
    rPoolDefaults[XATTRSET_LINE - XATTR_START] = new XLineAttrSetItem(
        std::make_unique<SfxItemSet>(
            *_pMaster, svl::Items<XATTR_LINE_FIRST, XATTR_LINE_LAST>{}));
    rPoolDefaults[XATTRSET_FILL - XATTR_START] = new XFillAttrSetItem(
        std::make_unique<SfxItemSet>(
            *_pMaster, svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{}));

    // create ItemInfos
    for(sal_uInt16 i(GetFirstWhich()); i <= GetLastWhich(); i++)
    {
        mpLocalItemInfos[i - XATTR_START]._nSID = 0;
        mpLocalItemInfos[i - XATTR_START]._bPoolable = true;
    }

    mpLocalItemInfos[XATTR_LINESTYLE        -XATTR_START]._nSID = SID_ATTR_LINE_STYLE;
    mpLocalItemInfos[XATTR_LINEDASH         -XATTR_START]._nSID = SID_ATTR_LINE_DASH;
    mpLocalItemInfos[XATTR_LINEWIDTH        -XATTR_START]._nSID = SID_ATTR_LINE_WIDTH;
    mpLocalItemInfos[XATTR_LINECOLOR        -XATTR_START]._nSID = SID_ATTR_LINE_COLOR;
    mpLocalItemInfos[XATTR_LINESTART        -XATTR_START]._nSID = SID_ATTR_LINE_START;
    mpLocalItemInfos[XATTR_LINEEND          -XATTR_START]._nSID = SID_ATTR_LINE_END;
    mpLocalItemInfos[XATTR_LINESTARTWIDTH   -XATTR_START]._nSID = SID_ATTR_LINE_STARTWIDTH;
    mpLocalItemInfos[XATTR_LINEENDWIDTH     -XATTR_START]._nSID = SID_ATTR_LINE_ENDWIDTH;
    mpLocalItemInfos[XATTR_LINESTARTCENTER  -XATTR_START]._nSID = SID_ATTR_LINE_STARTCENTER;
    mpLocalItemInfos[XATTR_LINEENDCENTER    -XATTR_START]._nSID = SID_ATTR_LINE_ENDCENTER;
    mpLocalItemInfos[XATTR_FILLSTYLE        -XATTR_START]._nSID = SID_ATTR_FILL_STYLE;
    mpLocalItemInfos[XATTR_FILLCOLOR        -XATTR_START]._nSID = SID_ATTR_FILL_COLOR;
    mpLocalItemInfos[XATTR_FILLGRADIENT     -XATTR_START]._nSID = SID_ATTR_FILL_GRADIENT;
    mpLocalItemInfos[XATTR_FILLHATCH        -XATTR_START]._nSID = SID_ATTR_FILL_HATCH;
    mpLocalItemInfos[XATTR_FILLBITMAP       -XATTR_START]._nSID = SID_ATTR_FILL_BITMAP;
    mpLocalItemInfos[XATTR_FORMTXTSTYLE     -XATTR_START]._nSID = SID_FORMTEXT_STYLE;
    mpLocalItemInfos[XATTR_FORMTXTADJUST    -XATTR_START]._nSID = SID_FORMTEXT_ADJUST;
    mpLocalItemInfos[XATTR_FORMTXTDISTANCE  -XATTR_START]._nSID = SID_FORMTEXT_DISTANCE;
    mpLocalItemInfos[XATTR_FORMTXTSTART     -XATTR_START]._nSID = SID_FORMTEXT_START;
    mpLocalItemInfos[XATTR_FORMTXTMIRROR    -XATTR_START]._nSID = SID_FORMTEXT_MIRROR;
    mpLocalItemInfos[XATTR_FORMTXTOUTLINE   -XATTR_START]._nSID = SID_FORMTEXT_OUTLINE;
    mpLocalItemInfos[XATTR_FORMTXTSHADOW    -XATTR_START]._nSID = SID_FORMTEXT_SHADOW;
    mpLocalItemInfos[XATTR_FORMTXTSHDWCOLOR -XATTR_START]._nSID = SID_FORMTEXT_SHDWCOLOR;
    mpLocalItemInfos[XATTR_FORMTXTSHDWXVAL  -XATTR_START]._nSID = SID_FORMTEXT_SHDWXVAL;
    mpLocalItemInfos[XATTR_FORMTXTSHDWYVAL  -XATTR_START]._nSID = SID_FORMTEXT_SHDWYVAL;
    mpLocalItemInfos[XATTR_FORMTXTHIDEFORM  -XATTR_START]._nSID = SID_FORMTEXT_HIDEFORM;

    // associate new slots for panels with known items
    mpLocalItemInfos[XATTR_FILLTRANSPARENCE - XATTR_START]._nSID = SID_ATTR_FILL_TRANSPARENCE;
    mpLocalItemInfos[XATTR_FILLFLOATTRANSPARENCE - XATTR_START]._nSID = SID_ATTR_FILL_FLOATTRANSPARENCE;
    mpLocalItemInfos[XATTR_LINETRANSPARENCE - XATTR_START]._nSID = SID_ATTR_LINE_TRANSPARENCE;
    mpLocalItemInfos[XATTR_LINEJOINT - XATTR_START]._nSID = SID_ATTR_LINE_JOINT;
    mpLocalItemInfos[XATTR_LINECAP - XATTR_START]._nSID = SID_ATTR_LINE_CAP;

    // if it's my own creation level, set Defaults and ItemInfos
    if(XATTR_START == GetFirstWhich() && XATTR_END == GetLastWhich())
    {
        SetDefaults(mpLocalPoolDefaults);
        SetItemInfos(mpLocalItemInfos.get());
    }
}

// copy ctor, clones all static defaults
XOutdevItemPool::XOutdevItemPool(const XOutdevItemPool& rPool)
:   SfxItemPool(rPool, true),
    mpLocalPoolDefaults(nullptr)
{
}

SfxItemPool* XOutdevItemPool::Clone() const
{
    return new XOutdevItemPool(*this);
}

XOutdevItemPool::~XOutdevItemPool()
{
    Delete();
    // release and delete static pool default items
    ReleaseDefaults(true);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
