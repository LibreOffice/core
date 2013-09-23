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

#include <svx/xtable.hxx>
#include <svx/xattr.hxx>
#include <svx/xpool.hxx>
#include <svx/svdattr.hxx>
#include <svx/svxids.hrc>
#include <svl/itemset.hxx>

XOutdevItemPool::XOutdevItemPool(
    SfxItemPool* _pMaster,
    sal_uInt16 nAttrStart,
    sal_uInt16 nAttrEnd,
    sal_Bool bLoadRefCounts)
:   SfxItemPool(OUString("XOutdevItemPool"), nAttrStart, nAttrEnd, 0L, 0L, bLoadRefCounts)
{
    // prepare some defaults
    const OUString aNullStr;
    const Graphic aNullGraphic;
    const basegfx::B2DPolyPolygon aNullPol;
    const Color aNullLineCol(COL_DEFAULT_SHAPE_STROKE); // #i121448# Use defined default color
    const Color aNullFillCol(COL_DEFAULT_SHAPE_FILLING); // #i121448# Use defined default color
    const Color aNullShadowCol(RGB_Color(COL_LIGHTGRAY));
    const XDash aNullDash;
    const XGradient aNullGrad(RGB_Color(COL_BLACK), RGB_Color(COL_WHITE));
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
    mppLocalPoolDefaults = new SfxPoolItem*[GetLastWhich() - GetFirstWhich() + 1];

    mppLocalPoolDefaults[XATTR_LINESTYLE          -XATTR_START] = new XLineStyleItem;
    mppLocalPoolDefaults[XATTR_LINEDASH           -XATTR_START] = new XLineDashItem(this,aNullDash);
    mppLocalPoolDefaults[XATTR_LINEWIDTH          -XATTR_START] = new XLineWidthItem;
    mppLocalPoolDefaults[XATTR_LINECOLOR          -XATTR_START] = new XLineColorItem(aNullStr,aNullLineCol);
    mppLocalPoolDefaults[XATTR_LINESTART          -XATTR_START] = new XLineStartItem(this,aNullPol);
    mppLocalPoolDefaults[XATTR_LINEEND            -XATTR_START] = new XLineEndItem  (this,aNullPol);
    mppLocalPoolDefaults[XATTR_LINESTARTWIDTH     -XATTR_START] = new XLineStartWidthItem;
    mppLocalPoolDefaults[XATTR_LINEENDWIDTH       -XATTR_START] = new XLineEndWidthItem;
    mppLocalPoolDefaults[XATTR_LINESTARTCENTER    -XATTR_START] = new XLineStartCenterItem;
    mppLocalPoolDefaults[XATTR_LINEENDCENTER      -XATTR_START] = new XLineEndCenterItem;
    mppLocalPoolDefaults[XATTR_LINETRANSPARENCE   -XATTR_START] = new XLineTransparenceItem;
    mppLocalPoolDefaults[XATTR_LINEJOINT            -XATTR_START] = new XLineJointItem;
    mppLocalPoolDefaults[XATTR_LINECAP            -XATTR_START] = new XLineCapItem;
    mppLocalPoolDefaults[XATTR_FILLSTYLE                -XATTR_START] = new XFillStyleItem;
    mppLocalPoolDefaults[XATTR_FILLCOLOR                -XATTR_START] = new XFillColorItem   (aNullStr,aNullFillCol);
    mppLocalPoolDefaults[XATTR_FILLGRADIENT         -XATTR_START] = new XFillGradientItem(aNullGrad);
    mppLocalPoolDefaults[XATTR_FILLHATCH                -XATTR_START] = new XFillHatchItem   (this,aNullHatch);
    mppLocalPoolDefaults[XATTR_FILLBITMAP               -XATTR_START] = new XFillBitmapItem  (this, aNullGraphic);
    mppLocalPoolDefaults[XATTR_FILLTRANSPARENCE     -XATTR_START] = new XFillTransparenceItem;
    mppLocalPoolDefaults[XATTR_GRADIENTSTEPCOUNT        -XATTR_START] = new XGradientStepCountItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_TILE         -XATTR_START] = new XFillBmpTileItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_POS          -XATTR_START] = new XFillBmpPosItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_SIZEX            -XATTR_START] = new XFillBmpSizeXItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_SIZEY            -XATTR_START] = new XFillBmpSizeYItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_SIZELOG      -XATTR_START] = new XFillBmpSizeLogItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_TILEOFFSETX  -XATTR_START] = new XFillBmpTileOffsetXItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_TILEOFFSETY  -XATTR_START] = new XFillBmpTileOffsetYItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_STRETCH      -XATTR_START] = new XFillBmpStretchItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_POSOFFSETX       -XATTR_START] = new XFillBmpPosOffsetXItem;
    mppLocalPoolDefaults[XATTR_FILLBMP_POSOFFSETY       -XATTR_START] = new XFillBmpPosOffsetYItem;
    mppLocalPoolDefaults[XATTR_FILLFLOATTRANSPARENCE    -XATTR_START] = new XFillFloatTransparenceItem( this, aNullGrad, sal_False );
    mppLocalPoolDefaults[XATTR_SECONDARYFILLCOLOR       -XATTR_START] = new XSecondaryFillColorItem(aNullStr, aNullFillCol);
    mppLocalPoolDefaults[XATTR_FILLBACKGROUND           -XATTR_START] = new XFillBackgroundItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSTYLE       -XATTR_START] = new XFormTextStyleItem;
    mppLocalPoolDefaults[XATTR_FORMTXTADJUST      -XATTR_START] = new XFormTextAdjustItem;
    mppLocalPoolDefaults[XATTR_FORMTXTDISTANCE    -XATTR_START] = new XFormTextDistanceItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSTART       -XATTR_START] = new XFormTextStartItem;
    mppLocalPoolDefaults[XATTR_FORMTXTMIRROR      -XATTR_START] = new XFormTextMirrorItem;
    mppLocalPoolDefaults[XATTR_FORMTXTOUTLINE     -XATTR_START] = new XFormTextOutlineItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSHADOW      -XATTR_START] = new XFormTextShadowItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSHDWCOLOR   -XATTR_START] = new XFormTextShadowColorItem(aNullStr,aNullShadowCol);
    mppLocalPoolDefaults[XATTR_FORMTXTSHDWXVAL    -XATTR_START] = new XFormTextShadowXValItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSHDWYVAL    -XATTR_START] = new XFormTextShadowYValItem;
    mppLocalPoolDefaults[XATTR_FORMTXTHIDEFORM    -XATTR_START] = new XFormTextHideFormItem;
    mppLocalPoolDefaults[XATTR_FORMTXTSHDWTRANSP  -XATTR_START] = new XFormTextShadowTranspItem;

    // create SetItems
    SfxItemSet* pSet=new SfxItemSet(*_pMaster, XATTR_LINE_FIRST, XATTR_LINE_LAST);
    mppLocalPoolDefaults[XATTRSET_LINE - XATTR_START] = new XLineAttrSetItem(pSet);
    pSet=new SfxItemSet(*_pMaster, XATTR_FILL_FIRST, XATTR_FILL_LAST);
    mppLocalPoolDefaults[XATTRSET_FILL - XATTR_START] = new XFillAttrSetItem(pSet);

    // create ItemInfos
    mpLocalItemInfos = new SfxItemInfo[GetLastWhich() - GetFirstWhich() + 1];
    for(sal_uInt16 i(GetFirstWhich()); i <= GetLastWhich(); i++)
    {
        mpLocalItemInfos[i - XATTR_START]._nSID = 0;
        mpLocalItemInfos[i - XATTR_START]._nFlags = SFX_ITEM_POOLABLE;
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
        SetDefaults(mppLocalPoolDefaults);
        SetItemInfos(mpLocalItemInfos);
    }
}

// copy ctor, clones all static defaults
XOutdevItemPool::XOutdevItemPool(const XOutdevItemPool& rPool)
:   SfxItemPool(rPool, sal_True),
    mppLocalPoolDefaults(0L),
    mpLocalItemInfos(0L)
{
}

SfxItemPool* XOutdevItemPool::Clone() const
{
    return new XOutdevItemPool(*this);
}

XOutdevItemPool::~XOutdevItemPool()
{
    Delete();

    // remove own static defaults
    if(mppLocalPoolDefaults)
    {
        SfxPoolItem** ppDefaultItem = mppLocalPoolDefaults;
        for(sal_uInt16 i(GetLastWhich() - GetFirstWhich() + 1); i; --i, ++ppDefaultItem)
        {
            if ( *ppDefaultItem ) // these parts might be already cleaned up from a derived class
            {
                SetRefCount( **ppDefaultItem, 0 );
                delete *ppDefaultItem;
            }
        }

        delete[] mppLocalPoolDefaults;
    }

    if(mpLocalItemInfos)
    {
        delete[] mpLocalItemInfos;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
