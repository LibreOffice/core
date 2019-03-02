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

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/drawing/BitmapMode.hpp>
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/frame/XTitle.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <o3tl/any.hxx>
#include <svx/svxids.hrc>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/xflclit.hxx>
#include <tools/globname.hxx>
#include <editeng/memberids.h>
#include <swtypes.hxx>
#include <cmdid.h>
#include <unomid.h>
#include <memory>
#include <utility>
#include <hints.hxx>
#include <doc.hxx>
#include <drawdoc.hxx>
#include <IDocumentUndoRedo.hxx>
#include <IDocumentDrawModelAccess.hxx>
#include <IDocumentLayoutAccess.hxx>
#include <IDocumentStylePoolAccess.hxx>
#include <docsh.hxx>
#include <editsh.hxx>
#include <swcli.hxx>
#include <ndindex.hxx>
#include <pam.hxx>
#include <ndnotxt.hxx>
#include <svx/unomid.hxx>
#include <unocrsr.hxx>
#include <unocrsrhelper.hxx>
#include <docstyle.hxx>
#include <dcontact.hxx>
#include <fmtcnct.hxx>
#include <ndole.hxx>
#include <frmfmt.hxx>
#include <frame.hxx>
#include <textboxhelper.hxx>
#include <unotextrange.hxx>
#include <unotextcursor.hxx>
#include <unoparagraph.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unoevent.hxx>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <tools/poly.hxx>
#include <swundo.hxx>
#include <unostyle.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpage.hxx>
#include <editeng/brushitem.hxx>
#include <editeng/protitem.hxx>
#include <fmtornt.hxx>
#include <fmteiro.hxx>
#include <fmturl.hxx>
#include <editeng/lrspitem.hxx>
#include <editeng/ulspitem.hxx>
#include <editeng/boxitem.hxx>
#include <editeng/opaqitem.hxx>
#include <editeng/prntitem.hxx>
#include <editeng/shaditem.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <grfatr.hxx>
#include <unoframe.hxx>
#include <fmtanchr.hxx>
#include <fmtclds.hxx>
#include <fmtcntnt.hxx>
#include <frmatr.hxx>
#include <ndtxt.hxx>
#include <ndgrf.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <vcl/GraphicLoader.hxx>
#include <sfx2/printer.hxx>
#include <SwStyleNameMapper.hxx>
#include <editeng/xmlcnitm.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <editeng/frmdiritem.hxx>
#include <fmtfollowtextflow.hxx>
#include <fmtwrapinfluenceonobjpos.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <calbck.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <sal/log.hxx>

#include <svx/unobrushitemhelper.hxx>
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
#include <svx/unoshape.hxx>
#include <svx/xflhtit.hxx>
#include <svx/xfltrit.hxx>
#include <swunohelper.hxx>
#include <fefly.hxx>
#include <fmtflcnt.hxx>

using namespace ::com::sun::star;

using ::com::sun::star::frame::XModel;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::style::XStyleFamiliesSupplier;

class BaseFrameProperties_Impl
{
    SwUnoCursorHelper::SwAnyMapHelper aAnyMap;

public:
    virtual ~BaseFrameProperties_Impl();

    void            SetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any& rVal);
    bool            GetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any*& pAny );
    bool FillBaseProperties(SfxItemSet& rToSet, const SfxItemSet &rFromSet, bool& rSizeFound);

    virtual bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrameSet, SfxItemSet& rSet, bool& rSizeFound) = 0;
};

BaseFrameProperties_Impl::~BaseFrameProperties_Impl()
{
}

void BaseFrameProperties_Impl::SetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any& rVal)
{
    aAnyMap.SetValue( nWID, nMemberId, rVal );
}

bool BaseFrameProperties_Impl::GetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any*& rpAny)
{
    return aAnyMap.FillValue( nWID, nMemberId, rpAny );
}

bool BaseFrameProperties_Impl::FillBaseProperties(SfxItemSet& rToSet, const SfxItemSet& rFromSet, bool& rSizeFound)
{
    // assert when the target SfxItemSet has no parent. It *should* have the pDfltFrameFormat
    // from SwDoc set as parent (or similar) to have the necessary XFILL_NONE in the ItemSet
    if(!rToSet.GetParent())
    {
        OSL_ENSURE(false, "OOps, target SfxItemSet *should* have a parent which contains XFILL_NONE as XFillStyleItem (!)");
    }

    bool bRet = true;
    // always add an anchor to the set
    SwFormatAnchor aAnchor ( rFromSet.Get ( RES_ANCHOR ) );
    {
        const ::uno::Any* pAnchorPgNo;
        if(GetProperty(RES_ANCHOR, MID_ANCHOR_PAGENUM, pAnchorPgNo))
            bRet &= static_cast<SfxPoolItem&>(aAnchor).PutValue(*pAnchorPgNo, MID_ANCHOR_PAGENUM);
        const ::uno::Any* pAnchorType;
        if(GetProperty(RES_ANCHOR, MID_ANCHOR_ANCHORTYPE, pAnchorType))
            bRet &= static_cast<SfxPoolItem&>(aAnchor).PutValue(*pAnchorType, MID_ANCHOR_ANCHORTYPE);
    }

    rToSet.Put(aAnchor);

    // check for SvxBrushItem (RES_BACKGROUND) properties
    const ::uno::Any* pCol = nullptr; GetProperty(RES_BACKGROUND, MID_BACK_COLOR, pCol );
    const ::uno::Any* pRGBCol = nullptr; GetProperty(RES_BACKGROUND, MID_BACK_COLOR_R_G_B, pRGBCol );
    const ::uno::Any* pColTrans = nullptr; GetProperty(RES_BACKGROUND, MID_BACK_COLOR_TRANSPARENCY, pColTrans);
    const ::uno::Any* pTrans = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_TRANSPARENT, pTrans );
    const ::uno::Any* pGrLoc = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_POSITION, pGrLoc );
    const ::uno::Any* pGraphic = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC, pGraphic     );
    const ::uno::Any* pGrFilter = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_FILTER, pGrFilter     );
    const ::uno::Any* pGraphicURL = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_URL, pGraphicURL );
    const ::uno::Any* pGrTranparency = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_TRANSPARENCY, pGrTranparency     );
    const bool bSvxBrushItemPropertiesUsed(
        pCol ||
        pTrans ||
        pGraphic ||
        pGraphicURL ||
        pGrFilter ||
        pGrLoc ||
        pGrTranparency ||
        pColTrans ||
        pRGBCol);

    // check for FillStyle properties in the range XATTR_FILL_FIRST, XATTR_FILL_LAST
    const uno::Any* pXFillStyleItem = nullptr; GetProperty(XATTR_FILLSTYLE, 0, pXFillStyleItem);
    const uno::Any* pXFillColorItem = nullptr; GetProperty(XATTR_FILLCOLOR, 0, pXFillColorItem);

    // XFillGradientItem: two possible slots supported in UNO API
    const uno::Any* pXFillGradientItem = nullptr; GetProperty(XATTR_FILLGRADIENT, MID_FILLGRADIENT, pXFillGradientItem);
    const uno::Any* pXFillGradientNameItem = nullptr; GetProperty(XATTR_FILLGRADIENT, MID_NAME, pXFillGradientNameItem);

    // XFillHatchItem: two possible slots supported in UNO API
    const uno::Any* pXFillHatchItem = nullptr; GetProperty(XATTR_FILLHATCH, MID_FILLHATCH, pXFillHatchItem);
    const uno::Any* pXFillHatchNameItem = nullptr; GetProperty(XATTR_FILLHATCH, MID_NAME, pXFillHatchNameItem);

    // XFillBitmapItem: three possible slots supported in UNO API
    const uno::Any* pXFillBitmapItem = nullptr; GetProperty(XATTR_FILLBITMAP, MID_BITMAP, pXFillBitmapItem);
    const uno::Any* pXFillBitmapNameItem = nullptr; GetProperty(XATTR_FILLBITMAP, MID_NAME, pXFillBitmapNameItem);

    const uno::Any* pXFillTransparenceItem = nullptr; GetProperty(XATTR_FILLTRANSPARENCE, 0, pXFillTransparenceItem);
    const uno::Any* pXGradientStepCountItem = nullptr; GetProperty(XATTR_GRADIENTSTEPCOUNT, 0, pXGradientStepCountItem);
    const uno::Any* pXFillBmpPosItem = nullptr; GetProperty(XATTR_FILLBMP_POS, 0, pXFillBmpPosItem);
    const uno::Any* pXFillBmpSizeXItem = nullptr; GetProperty(XATTR_FILLBMP_SIZEX, 0, pXFillBmpSizeXItem);
    const uno::Any* pXFillBmpSizeYItem = nullptr; GetProperty(XATTR_FILLBMP_SIZEY, 0, pXFillBmpSizeYItem);

    // XFillFloatTransparenceItem: two possible slots supported in UNO API
    const uno::Any* pXFillFloatTransparenceItem = nullptr; GetProperty(XATTR_FILLFLOATTRANSPARENCE, MID_FILLGRADIENT, pXFillFloatTransparenceItem);
    const uno::Any* pXFillFloatTransparenceNameItem = nullptr; GetProperty(XATTR_FILLFLOATTRANSPARENCE, MID_NAME, pXFillFloatTransparenceNameItem);

    const uno::Any* pXSecondaryFillColorItem = nullptr; GetProperty(XATTR_SECONDARYFILLCOLOR, 0, pXSecondaryFillColorItem);
    const uno::Any* pXFillBmpSizeLogItem = nullptr; GetProperty(XATTR_FILLBMP_SIZELOG, 0, pXFillBmpSizeLogItem);
    const uno::Any* pXFillBmpTileOffsetXItem = nullptr; GetProperty(XATTR_FILLBMP_TILEOFFSETX, 0, pXFillBmpTileOffsetXItem);
    const uno::Any* pXFillBmpTileOffsetYItem = nullptr; GetProperty(XATTR_FILLBMP_TILEOFFSETY, 0, pXFillBmpTileOffsetYItem);
    const uno::Any* pXFillBmpPosOffsetXItem = nullptr; GetProperty(XATTR_FILLBMP_POSOFFSETX, 0, pXFillBmpPosOffsetXItem);
    const uno::Any* pXFillBmpPosOffsetYItem = nullptr; GetProperty(XATTR_FILLBMP_POSOFFSETY, 0, pXFillBmpPosOffsetYItem);
    const uno::Any* pXFillBackgroundItem = nullptr; GetProperty(XATTR_FILLBACKGROUND, 0, pXFillBackgroundItem);
    const uno::Any* pOwnAttrFillBmpItem = nullptr; GetProperty(OWN_ATTR_FILLBMP_MODE, 0, pOwnAttrFillBmpItem);

    // tdf#91140: ignore SOLID fill style for determining if fill style is used
    // but there is a Graphic
    const bool bFillStyleUsed(pXFillStyleItem && pXFillStyleItem->hasValue() &&
        (pXFillStyleItem->get<drawing::FillStyle>() != drawing::FillStyle_SOLID || (!pGraphic || !pGraphicURL) ));
    SAL_INFO_IF(pXFillStyleItem && pXFillStyleItem->hasValue() && !bFillStyleUsed,
            "sw.uno", "FillBaseProperties: ignoring invalid FillStyle");
    const bool bXFillStyleItemUsed(
        bFillStyleUsed ||
        pXFillColorItem ||
        pXFillGradientItem || pXFillGradientNameItem ||
        pXFillHatchItem || pXFillHatchNameItem ||
        pXFillBitmapItem || pXFillBitmapNameItem ||
        pXFillTransparenceItem ||
        pXGradientStepCountItem ||
        pXFillBmpPosItem ||
        pXFillBmpSizeXItem ||
        pXFillBmpSizeYItem ||
        pXFillFloatTransparenceItem || pXFillFloatTransparenceNameItem ||
        pXSecondaryFillColorItem ||
        pXFillBmpSizeLogItem ||
        pXFillBmpTileOffsetXItem ||
        pXFillBmpTileOffsetYItem ||
        pXFillBmpPosOffsetXItem ||
        pXFillBmpPosOffsetYItem ||
        pXFillBackgroundItem ||
        pOwnAttrFillBmpItem);

    // use brush items, but *only* if no FillStyle properties are used; if both are used and when applying both
    // in the obvious order some attributes may be wrong since they are set by the 1st set, but not
    // redefined as needed by the 2nd set when they are default (and thus no tset) in the 2nd set. If
    // it is necessary for any reason to set both (it should not) a in-between step will be needed
    // that resets the items for FillAttributes in rToSet to default.
    // Note: There are other mechanisms in XMLOFF to pre-sort this relationship already, but this version
    // was used initially, is tested and works. Keep it to be able to react when another feed adds attributes
    // from both sets.
    if(bSvxBrushItemPropertiesUsed && !bXFillStyleItemUsed)
    {
        // create a temporary SvxBrushItem, fill the attributes to it and use it to set
        // the corresponding FillAttributes
        SvxBrushItem aBrush(RES_BACKGROUND);

        if(pCol)
        {
            bRet &= static_cast<SfxPoolItem&>(aBrush).PutValue(*pCol,MID_BACK_COLOR    );
        }

        if(pColTrans)
        {
            bRet &= static_cast<SfxPoolItem&>(aBrush).PutValue(*pColTrans, MID_BACK_COLOR_TRANSPARENCY);
        }

        if(pRGBCol)
        {
            bRet &= static_cast<SfxPoolItem&>(aBrush).PutValue(*pRGBCol, MID_BACK_COLOR_R_G_B);
        }

        if(pTrans)
        {
            // don't overwrite transparency with a non-transparence flag
            if(!pColTrans || Any2Bool( *pTrans ))
                bRet &= static_cast<SfxPoolItem&>(aBrush).PutValue(*pTrans, MID_GRAPHIC_TRANSPARENT);
        }

        if (pGraphic)
        {
            bRet &= static_cast<SfxPoolItem&>(aBrush).PutValue(*pGraphic, MID_GRAPHIC);
        }

        if (pGraphicURL)
        {
            bRet &= static_cast<SfxPoolItem&>(aBrush).PutValue(*pGraphicURL, MID_GRAPHIC_URL);
        }

        if(pGrFilter)
        {
            bRet &= static_cast<SfxPoolItem&>(aBrush).PutValue(*pGrFilter, MID_GRAPHIC_FILTER);
        }

        if(pGrLoc)
        {
            bRet &= static_cast<SfxPoolItem&>(aBrush).PutValue(*pGrLoc, MID_GRAPHIC_POSITION);
        }

        if(pGrTranparency)
        {
            bRet &= static_cast<SfxPoolItem&>(aBrush).PutValue(*pGrTranparency, MID_GRAPHIC_TRANSPARENCY);
        }

        setSvxBrushItemAsFillAttributesToTargetSet(aBrush, rToSet);
    }

    if(bXFillStyleItemUsed)
    {
        XFillStyleItem aXFillStyleItem;
        SvxBrushItem aBrush(RES_BACKGROUND);

        if(pXFillStyleItem)
        {
            aXFillStyleItem.PutValue(*pXFillStyleItem, 0);
            rToSet.Put(aXFillStyleItem);
        }

        if(pXFillColorItem)
        {
            const Color aNullCol(COL_DEFAULT_SHAPE_FILLING);
            XFillColorItem aXFillColorItem(OUString(), aNullCol);

            aXFillColorItem.PutValue(*pXFillColorItem, 0);
            rToSet.Put(aXFillColorItem);
            //set old-school brush color if we later encounter the
            //MID_BACK_COLOR_TRANSPARENCY case below
            aBrush = getSvxBrushItemFromSourceSet(rToSet, RES_BACKGROUND, false);
        }
        else if (aXFillStyleItem.GetValue() == drawing::FillStyle_SOLID && (pCol || pRGBCol))
        {
            // Fill style is set to solid, but no fill color is given.
            // On the other hand, we have a BackColor, so use that.
            if (pCol)
                aBrush.PutValue(*pCol, MID_BACK_COLOR);
            else
                aBrush.PutValue(*pRGBCol, MID_BACK_COLOR_R_G_B);
            setSvxBrushItemAsFillAttributesToTargetSet(aBrush, rToSet);
        }

        if(pXFillGradientItem || pXFillGradientNameItem)
        {
            if(pXFillGradientItem)
            {
                const XGradient aNullGrad(COL_BLACK, COL_WHITE);
                XFillGradientItem aXFillGradientItem(aNullGrad);

                aXFillGradientItem.PutValue(*pXFillGradientItem, MID_FILLGRADIENT);
                rToSet.Put(aXFillGradientItem);
            }

            if(pXFillGradientNameItem)
            {
                OUString aTempName;

                if(!(*pXFillGradientNameItem >>= aTempName ))
                {
                    throw lang::IllegalArgumentException();
                }

                bool const bSuccess = SvxShape::SetFillAttribute(
                                        XATTR_FILLGRADIENT, aTempName, rToSet);
                if (aXFillStyleItem.GetValue() == drawing::FillStyle_GRADIENT)
                {   // tdf#90946 ignore invalid gradient-name if SOLID
                    bRet &= bSuccess;
                }
                else
                {
                    SAL_INFO_IF(!bSuccess, "sw.uno",
                       "FillBaseProperties: ignoring invalid FillGradientName");
                }
            }
        }

        if(pXFillHatchItem || pXFillHatchNameItem)
        {
            if(pXFillHatchItem)
            {
                const Color aNullCol(COL_DEFAULT_SHAPE_STROKE);
                const XHatch aNullHatch(aNullCol);
                XFillHatchItem aXFillHatchItem(aNullHatch);

                aXFillHatchItem.PutValue(*pXFillHatchItem, MID_FILLHATCH);
                rToSet.Put(aXFillHatchItem);
            }

            if(pXFillHatchNameItem)
            {
                OUString aTempName;

                if(!(*pXFillHatchNameItem >>= aTempName ))
                {
                    throw lang::IllegalArgumentException();
                }

                bRet &= SvxShape::SetFillAttribute(XATTR_FILLHATCH, aTempName, rToSet);
            }
        }

        if (pXFillBitmapItem || pXFillBitmapNameItem)
        {
            if(pXFillBitmapItem)
            {
                const Graphic aNullGraphic;
                XFillBitmapItem aXFillBitmapItem(aNullGraphic);

                aXFillBitmapItem.PutValue(*pXFillBitmapItem, MID_BITMAP);
                rToSet.Put(aXFillBitmapItem);
            }

            if(pXFillBitmapNameItem)
            {
                OUString aTempName;

                if(!(*pXFillBitmapNameItem >>= aTempName ))
                {
                    throw lang::IllegalArgumentException();
                }

                bRet &= SvxShape::SetFillAttribute(XATTR_FILLBITMAP, aTempName, rToSet);
            }
        }

        if (pXFillTransparenceItem)
        {
            XFillTransparenceItem aXFillTransparenceItem;
            aXFillTransparenceItem.PutValue(*pXFillTransparenceItem, 0);
            rToSet.Put(aXFillTransparenceItem);
        }
        else if (pColTrans &&
            !pXFillFloatTransparenceItem && !pXFillFloatTransparenceNameItem)
        {
            // No fill transparency is given.  On the other hand, we have a
            // BackColorTransparency, so use that.
            // tdf#90640 tdf#90130: this is necessary for LO 4.4.0 - 4.4.2
            // that forgot to write draw:opacity into documents
            // but: the value was *always* wrong for bitmaps! => ignore it
            sal_Int8 nGraphicTransparency(0);
            *pColTrans >>= nGraphicTransparency;
            if (aXFillStyleItem.GetValue() != drawing::FillStyle_BITMAP)
            {
                rToSet.Put(XFillTransparenceItem(nGraphicTransparency));
            }
            if (aXFillStyleItem.GetValue() == drawing::FillStyle_SOLID)
            {
                aBrush.PutValue(*pColTrans, MID_BACK_COLOR_TRANSPARENCY);
                setSvxBrushItemAsFillAttributesToTargetSet(aBrush, rToSet);
            }
        }

        if(pXGradientStepCountItem)
        {
            XGradientStepCountItem aXGradientStepCountItem;

            aXGradientStepCountItem.PutValue(*pXGradientStepCountItem, 0);
            rToSet.Put(aXGradientStepCountItem);
        }

        if(pXFillBmpPosItem)
        {
            XFillBmpPosItem aXFillBmpPosItem;

            aXFillBmpPosItem.PutValue(*pXFillBmpPosItem, 0);
            rToSet.Put(aXFillBmpPosItem);
        }

        if(pXFillBmpSizeXItem)
        {
            XFillBmpSizeXItem aXFillBmpSizeXItem;

            aXFillBmpSizeXItem.PutValue(*pXFillBmpSizeXItem, 0);
            rToSet.Put(aXFillBmpSizeXItem);
        }

        if(pXFillBmpSizeYItem)
        {
            XFillBmpSizeYItem aXFillBmpSizeYItem;

            aXFillBmpSizeYItem.PutValue(*pXFillBmpSizeYItem, 0);
            rToSet.Put(aXFillBmpSizeYItem);
        }

        if(pXFillFloatTransparenceItem || pXFillFloatTransparenceNameItem)
        {
            if(pXFillFloatTransparenceItem)
            {
                const XGradient aNullGrad(COL_BLACK, COL_WHITE);
                XFillFloatTransparenceItem aXFillFloatTransparenceItem(aNullGrad, false);

                aXFillFloatTransparenceItem.PutValue(*pXFillFloatTransparenceItem, MID_FILLGRADIENT);
                rToSet.Put(aXFillFloatTransparenceItem);
            }

            if(pXFillFloatTransparenceNameItem)
            {
                OUString aTempName;

                if(!(*pXFillFloatTransparenceNameItem >>= aTempName ))
                {
                    throw lang::IllegalArgumentException();
                }

                bRet &= SvxShape::SetFillAttribute(XATTR_FILLFLOATTRANSPARENCE, aTempName, rToSet);
            }
        }

        if(pXSecondaryFillColorItem)
        {
            const Color aNullCol(COL_DEFAULT_SHAPE_FILLING);
            XSecondaryFillColorItem aXSecondaryFillColorItem(OUString(), aNullCol);

            aXSecondaryFillColorItem.PutValue(*pXSecondaryFillColorItem, 0);
            rToSet.Put(aXSecondaryFillColorItem);
        }

        if(pXFillBmpSizeLogItem)
        {
            XFillBmpSizeLogItem aXFillBmpSizeLogItem;

            aXFillBmpSizeLogItem.PutValue(*pXFillBmpSizeLogItem, 0);
            rToSet.Put(aXFillBmpSizeLogItem);
        }

        if(pXFillBmpTileOffsetXItem)
        {
            XFillBmpTileOffsetXItem aXFillBmpTileOffsetXItem;

            aXFillBmpTileOffsetXItem.PutValue(*pXFillBmpTileOffsetXItem, 0);
            rToSet.Put(aXFillBmpTileOffsetXItem);
        }

        if(pXFillBmpTileOffsetYItem)
        {
            XFillBmpTileOffsetYItem aXFillBmpTileOffsetYItem;

            aXFillBmpTileOffsetYItem.PutValue(*pXFillBmpTileOffsetYItem, 0);
            rToSet.Put(aXFillBmpTileOffsetYItem);
        }

        if(pXFillBmpPosOffsetXItem)
        {
            XFillBmpPosOffsetXItem aXFillBmpPosOffsetXItem;

            aXFillBmpPosOffsetXItem.PutValue(*pXFillBmpPosOffsetXItem, 0);
            rToSet.Put(aXFillBmpPosOffsetXItem);
        }

        if(pXFillBmpPosOffsetYItem)
        {
            XFillBmpPosOffsetYItem aXFillBmpPosOffsetYItem;

            aXFillBmpPosOffsetYItem.PutValue(*pXFillBmpPosOffsetYItem, 0);
            rToSet.Put(aXFillBmpPosOffsetYItem);
        }

        if(pXFillBackgroundItem)
        {
            XFillBackgroundItem aXFillBackgroundItem;

            aXFillBackgroundItem.PutValue(*pXFillBackgroundItem, 0);
            rToSet.Put(aXFillBackgroundItem);
        }

        if(pOwnAttrFillBmpItem)
        {
            drawing::BitmapMode eMode;

            if(!(*pOwnAttrFillBmpItem >>= eMode))
            {
                sal_Int32 nMode = 0;

                if(!(*pOwnAttrFillBmpItem >>= nMode))
                {
                    throw lang::IllegalArgumentException();
                }

                eMode = static_cast<drawing::BitmapMode>(nMode);
            }

            rToSet.Put(XFillBmpStretchItem(drawing::BitmapMode_STRETCH == eMode));
            rToSet.Put(XFillBmpTileItem(drawing::BitmapMode_REPEAT == eMode));
        }
    }
    {
        const ::uno::Any* pCont = nullptr;
        GetProperty(RES_PROTECT, MID_PROTECT_CONTENT, pCont );
        const ::uno::Any* pPos = nullptr;
        GetProperty(RES_PROTECT,MID_PROTECT_POSITION, pPos );
        const ::uno::Any* pName = nullptr;
        GetProperty(RES_PROTECT, MID_PROTECT_SIZE, pName );
        if(pCont||pPos||pName)
        {
            SvxProtectItem aProt ( rFromSet.Get ( RES_PROTECT ) );
            if(pCont)
                bRet &= static_cast<SfxPoolItem&>(aProt).PutValue(*pCont, MID_PROTECT_CONTENT);
            if(pPos )
                bRet &= static_cast<SfxPoolItem&>(aProt).PutValue(*pPos, MID_PROTECT_POSITION);
            if(pName)
                bRet &= static_cast<SfxPoolItem&>(aProt).PutValue(*pName, MID_PROTECT_SIZE);
            rToSet.Put(aProt);
        }
    }
    {
        const ::uno::Any* pHori  = nullptr;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_ORIENT, pHori );
        const ::uno::Any* pHoriP = nullptr;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_POSITION|CONVERT_TWIPS, pHoriP );
        const ::uno::Any* pHoriR = nullptr;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_RELATION, pHoriR );
        const ::uno::Any* pPageT = nullptr;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_PAGETOGGLE, pPageT);
        if(pHori||pHoriP||pHoriR||pPageT)
        {
            SwFormatHoriOrient aOrient ( rFromSet.Get ( RES_HORI_ORIENT ) );
            if(pHori )
                bRet &= static_cast<SfxPoolItem&>(aOrient).PutValue(*pHori, MID_HORIORIENT_ORIENT);
            if(pHoriP)
                bRet &= static_cast<SfxPoolItem&>(aOrient).PutValue(*pHoriP, MID_HORIORIENT_POSITION|CONVERT_TWIPS);
            if(pHoriR)
                bRet &= static_cast<SfxPoolItem&>(aOrient).PutValue(*pHoriR, MID_HORIORIENT_RELATION);
            if(pPageT)
                bRet &= static_cast<SfxPoolItem&>(aOrient).PutValue(*pPageT, MID_HORIORIENT_PAGETOGGLE);
            rToSet.Put(aOrient);
        }
    }

    {
        const ::uno::Any* pVert  = nullptr;
        GetProperty(RES_VERT_ORIENT, MID_VERTORIENT_ORIENT, pVert);
        const ::uno::Any* pVertP = nullptr;
        GetProperty(RES_VERT_ORIENT, MID_VERTORIENT_POSITION|CONVERT_TWIPS, pVertP );
        const ::uno::Any* pVertR = nullptr;
        GetProperty(RES_VERT_ORIENT, MID_VERTORIENT_RELATION, pVertR );
        if(pVert||pVertP||pVertR)
        {
            SwFormatVertOrient aOrient ( rFromSet.Get ( RES_VERT_ORIENT ) );
            if(pVert )
                bRet &= static_cast<SfxPoolItem&>(aOrient).PutValue(*pVert, MID_VERTORIENT_ORIENT);
            if(pVertP)
                bRet &= static_cast<SfxPoolItem&>(aOrient).PutValue(*pVertP, MID_VERTORIENT_POSITION|CONVERT_TWIPS);
            if(pVertR)
                bRet &= static_cast<SfxPoolItem&>(aOrient).PutValue(*pVertR, MID_VERTORIENT_RELATION);
            rToSet.Put(aOrient);
        }
    }
    {
        const ::uno::Any* pURL = nullptr;
        GetProperty(RES_URL, MID_URL_URL, pURL );
        const ::uno::Any* pTarget = nullptr;
        GetProperty(RES_URL, MID_URL_TARGET, pTarget );
        const ::uno::Any* pHyLNm = nullptr;
        GetProperty(RES_URL, MID_URL_HYPERLINKNAME, pHyLNm );
        const ::uno::Any* pHySMp = nullptr;
        GetProperty(RES_URL, MID_URL_SERVERMAP, pHySMp );
        if(pURL||pTarget||pHyLNm||pHySMp)
        {
            SwFormatURL aURL ( rFromSet.Get ( RES_URL ) );
            if(pURL)
                bRet &= static_cast<SfxPoolItem&>(aURL).PutValue(*pURL, MID_URL_URL);
            if(pTarget)
                bRet &= static_cast<SfxPoolItem&>(aURL).PutValue(*pTarget, MID_URL_TARGET);
            if(pHyLNm)
                bRet &= static_cast<SfxPoolItem&>(aURL).PutValue(*pHyLNm, MID_URL_HYPERLINKNAME  );
            if(pHySMp)
                bRet &= static_cast<SfxPoolItem&>(aURL).PutValue(*pHySMp, MID_URL_SERVERMAP);
            rToSet.Put(aURL);
        }
    }
    const ::uno::Any* pL = nullptr;
    GetProperty(RES_LR_SPACE, MID_L_MARGIN|CONVERT_TWIPS, pL );
    const ::uno::Any* pR = nullptr;
    GetProperty(RES_LR_SPACE, MID_R_MARGIN|CONVERT_TWIPS, pR );
    if(pL||pR)
    {
        SvxLRSpaceItem aLR ( rFromSet.Get ( RES_LR_SPACE ) );
        if(pL)
            bRet &= static_cast<SfxPoolItem&>(aLR).PutValue(*pL, MID_L_MARGIN|CONVERT_TWIPS);
        if(pR)
            bRet &= static_cast<SfxPoolItem&>(aLR).PutValue(*pR, MID_R_MARGIN|CONVERT_TWIPS);
        rToSet.Put(aLR);
    }
    const ::uno::Any* pT = nullptr;
    GetProperty(RES_UL_SPACE, MID_UP_MARGIN|CONVERT_TWIPS, pT );
    const ::uno::Any* pB = nullptr;
    GetProperty(RES_UL_SPACE, MID_LO_MARGIN|CONVERT_TWIPS, pB );
    if(pT||pB)
    {
        SvxULSpaceItem aTB ( rFromSet.Get ( RES_UL_SPACE ) );
        if(pT)
            bRet &= static_cast<SfxPoolItem&>(aTB).PutValue(*pT, MID_UP_MARGIN|CONVERT_TWIPS);
        if(pB)
            bRet &= static_cast<SfxPoolItem&>(aTB).PutValue(*pB, MID_LO_MARGIN|CONVERT_TWIPS);
        rToSet.Put(aTB);
    }
    const ::uno::Any* pOp;
    if(GetProperty(RES_OPAQUE, 0, pOp))
    {
        SvxOpaqueItem aOp ( rFromSet.Get ( RES_OPAQUE ) );
        bRet &= static_cast<SfxPoolItem&>(aOp).PutValue(*pOp, 0);
        rToSet.Put(aOp);
    }
    const ::uno::Any* pPrt;
    if(GetProperty(RES_PRINT, 0, pPrt))
    {
        SvxPrintItem aPrt ( rFromSet.Get ( RES_PRINT ) );
        bRet &= static_cast<SfxPoolItem&>(aPrt).PutValue(*pPrt, 0);
        rToSet.Put(aPrt);
    }
    const ::uno::Any* pSh;
    if(GetProperty(RES_SHADOW, CONVERT_TWIPS, pSh))
    {
        SvxShadowItem aSh ( rFromSet.Get ( RES_SHADOW ) );
        bRet &= static_cast<SfxPoolItem&>(aSh).PutValue(*pSh, CONVERT_TWIPS);
        rToSet.Put(aSh);
    }
    const ::uno::Any* pShTr;
    if(GetProperty(RES_SHADOW, MID_SHADOW_TRANSPARENCE, pShTr) && rToSet.HasItem(RES_SHADOW))
    {
        SvxShadowItem aSh(rToSet.Get(RES_SHADOW));
        bRet &= aSh.PutValue(*pShTr, MID_SHADOW_TRANSPARENCE);
        rToSet.Put(aSh);
    }
    const ::uno::Any* pSur      = nullptr;
    GetProperty(RES_SURROUND, MID_SURROUND_SURROUNDTYPE, pSur);
    const ::uno::Any* pSurAnch = nullptr;
    GetProperty(RES_SURROUND, MID_SURROUND_ANCHORONLY, pSurAnch);
    if(pSur || pSurAnch)
    {
        SwFormatSurround aSrnd ( rFromSet.Get ( RES_SURROUND ) );
        if(pSur)
            bRet &= static_cast<SfxPoolItem&>(aSrnd).PutValue(*pSur, MID_SURROUND_SURROUNDTYPE );
        if(pSurAnch)
            bRet &= static_cast<SfxPoolItem&>(aSrnd).PutValue(*pSurAnch, MID_SURROUND_ANCHORONLY);
        rToSet.Put(aSrnd);
    }
    const ::uno::Any* pLeft        = nullptr;
    GetProperty(RES_BOX, LEFT_BORDER  |CONVERT_TWIPS,    pLeft  );
    const ::uno::Any* pRight       = nullptr;
    GetProperty(RES_BOX, CONVERT_TWIPS|RIGHT_BORDER ,    pRight );
    const ::uno::Any* pTop         = nullptr;
    GetProperty(RES_BOX, CONVERT_TWIPS|TOP_BORDER     , pTop   );
    const ::uno::Any* pBottom  = nullptr;
    GetProperty(RES_BOX, CONVERT_TWIPS|BOTTOM_BORDER,    pBottom);
    const ::uno::Any* pDistance    = nullptr;
    GetProperty(RES_BOX, CONVERT_TWIPS|BORDER_DISTANCE,  pDistance);
    const ::uno::Any* pLeftDistance    = nullptr;
    GetProperty(RES_BOX, CONVERT_TWIPS|LEFT_BORDER_DISTANCE, pLeftDistance);
    const ::uno::Any* pRightDistance   = nullptr;
    GetProperty(RES_BOX, CONVERT_TWIPS|RIGHT_BORDER_DISTANCE,    pRightDistance);
    const ::uno::Any* pTopDistance     = nullptr;
    GetProperty(RES_BOX, CONVERT_TWIPS|TOP_BORDER_DISTANCE,  pTopDistance);
    const ::uno::Any* pBottomDistance  = nullptr;
    GetProperty(RES_BOX, CONVERT_TWIPS|BOTTOM_BORDER_DISTANCE,   pBottomDistance);
    const ::uno::Any* pLineStyle  = nullptr;
    GetProperty(RES_BOX, LINE_STYLE,   pLineStyle);
    const ::uno::Any* pLineWidth  = nullptr;
    GetProperty(RES_BOX, LINE_WIDTH,   pLineWidth);
    if( pLeft || pRight || pTop ||  pBottom || pDistance ||
        pLeftDistance  || pRightDistance || pTopDistance || pBottomDistance ||
        pLineStyle || pLineWidth )
    {
        SvxBoxItem aBox ( rFromSet.Get ( RES_BOX ) );
        if( pLeft )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pLeft, CONVERT_TWIPS|LEFT_BORDER );
        if( pRight )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pRight, CONVERT_TWIPS|RIGHT_BORDER );
        if( pTop )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pTop, CONVERT_TWIPS|TOP_BORDER);
        if( pBottom )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pBottom, CONVERT_TWIPS|BOTTOM_BORDER);
        if( pDistance )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pDistance, CONVERT_TWIPS|BORDER_DISTANCE);
        if( pLeftDistance )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pLeftDistance, CONVERT_TWIPS|LEFT_BORDER_DISTANCE);
        if( pRightDistance )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pRightDistance, CONVERT_TWIPS|RIGHT_BORDER_DISTANCE);
        if( pTopDistance )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pTopDistance, CONVERT_TWIPS|TOP_BORDER_DISTANCE);
        if( pBottomDistance )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pBottomDistance, CONVERT_TWIPS|BOTTOM_BORDER_DISTANCE);
        if( pLineStyle )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pLineStyle, LINE_STYLE);
        if( pLineWidth )
            bRet &= static_cast<SfxPoolItem&>(aBox).PutValue(*pLineWidth, LINE_WIDTH|CONVERT_TWIPS);
        rToSet.Put(aBox);
    }
    {
        const ::uno::Any* pRelH = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_REL_HEIGHT, pRelH);
        const ::uno::Any* pRelHRelation = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_REL_HEIGHT_RELATION, pRelHRelation);
        const ::uno::Any* pRelW = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_REL_WIDTH, pRelW);
        const ::uno::Any* pRelWRelation = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_REL_WIDTH_RELATION, pRelWRelation);
        const ::uno::Any* pSyncWidth = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT, pSyncWidth);
        const ::uno::Any* pSyncHeight = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH, pSyncHeight);
        const ::uno::Any* pWidth = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS, pWidth);
        const ::uno::Any* pHeight = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS, pHeight);
        const ::uno::Any* pSize = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_SIZE|CONVERT_TWIPS, pSize);
        const ::uno::Any* pSizeType = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_SIZE_TYPE, pSizeType);
        const ::uno::Any* pWidthType = nullptr;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_WIDTH_TYPE, pWidthType);
        if( pWidth || pHeight ||pRelH || pRelHRelation || pRelW || pRelWRelation || pSize ||pSizeType ||
            pWidthType ||pSyncWidth || pSyncHeight )
        {
            rSizeFound = true;
            SwFormatFrameSize aFrameSz ( rFromSet.Get ( RES_FRM_SIZE ) );
            if(pWidth)
                bRet &= static_cast<SfxPoolItem&>(aFrameSz).PutValue(*pWidth, MID_FRMSIZE_WIDTH|CONVERT_TWIPS);
            if(pHeight)
                bRet &= static_cast<SfxPoolItem&>(aFrameSz).PutValue(*pHeight, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS);
            if(pRelH )
                bRet &= static_cast<SfxPoolItem&>(aFrameSz).PutValue(*pRelH, MID_FRMSIZE_REL_HEIGHT);
            if (pRelHRelation)
                bRet &= aFrameSz.PutValue(*pRelHRelation, MID_FRMSIZE_REL_HEIGHT_RELATION);
            if(pRelW )
                bRet &= static_cast<SfxPoolItem&>(aFrameSz).PutValue(*pRelW, MID_FRMSIZE_REL_WIDTH);
            if (pRelWRelation)
                bRet &= aFrameSz.PutValue(*pRelWRelation, MID_FRMSIZE_REL_WIDTH_RELATION);
            if(pSyncWidth)
                bRet &= static_cast<SfxPoolItem&>(aFrameSz).PutValue(*pSyncWidth, MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT);
            if(pSyncHeight)
                bRet &= static_cast<SfxPoolItem&>(aFrameSz).PutValue(*pSyncHeight, MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH);
            if(pSize)
                bRet &= static_cast<SfxPoolItem&>(aFrameSz).PutValue(*pSize, MID_FRMSIZE_SIZE|CONVERT_TWIPS);
            if(pSizeType)
                bRet &= static_cast<SfxPoolItem&>(aFrameSz).PutValue(*pSizeType, MID_FRMSIZE_SIZE_TYPE);
            if(pWidthType)
                bRet &= static_cast<SfxPoolItem&>(aFrameSz).PutValue(*pWidthType, MID_FRMSIZE_WIDTH_TYPE);
            if(!aFrameSz.GetWidth())
                aFrameSz.SetWidth(MINFLY);
            if(!aFrameSz.GetHeight())
                aFrameSz.SetHeight(MINFLY);
            rToSet.Put(aFrameSz);
        }
        else
        {
            rSizeFound = false;
            SwFormatFrameSize aFrameSz;
            awt::Size aSize;
            aSize.Width = 2 * MM50;
            aSize.Height = 2 * MM50;
            ::uno::Any aSizeVal;
            aSizeVal <<= aSize;
            static_cast<SfxPoolItem&>(aFrameSz).PutValue(aSizeVal, MID_FRMSIZE_SIZE|CONVERT_TWIPS);
            rToSet.Put(aFrameSz);
        }
    }
    const ::uno::Any* pFrameDirection = nullptr;
    GetProperty(RES_FRAMEDIR, 0, pFrameDirection);
    if(pFrameDirection)
    {
        SvxFrameDirectionItem aAttr(SvxFrameDirection::Horizontal_LR_TB, RES_FRAMEDIR);
        aAttr.PutValue(*pFrameDirection, 0);
        rToSet.Put(aAttr);
    }
    const ::uno::Any* pUnknown = nullptr;
    GetProperty(RES_UNKNOWNATR_CONTAINER, 0, pUnknown);
    if(pUnknown)
    {
        SvXMLAttrContainerItem aAttr(RES_UNKNOWNATR_CONTAINER);
        aAttr.PutValue(*pUnknown, 0);
        rToSet.Put(aAttr);
    }

    // #i18732#
    const ::uno::Any* pFollowTextFlow = nullptr;
    const ::uno::Any* pLayOutinCell = nullptr;
    GetProperty(RES_FOLLOW_TEXT_FLOW, MID_FOLLOW_TEXT_FLOW, pFollowTextFlow);
    GetProperty(RES_FOLLOW_TEXT_FLOW, MID_FTF_LAYOUT_IN_CELL, pLayOutinCell);

    if ( pFollowTextFlow || pLayOutinCell)
    {
        SwFormatFollowTextFlow aFormatFollowTextFlow;
        if( pFollowTextFlow )
        {
            aFormatFollowTextFlow.PutValue(*pFollowTextFlow, MID_FOLLOW_TEXT_FLOW);
        }

        if ( pLayOutinCell )
        {
            aFormatFollowTextFlow.PutValue(*pLayOutinCell, MID_FTF_LAYOUT_IN_CELL);
        }
        rToSet.Put(aFormatFollowTextFlow);
    }

    // #i28701# - RES_WRAP_INFLUENCE_ON_OBJPOS
    const ::uno::Any* pWrapInfluenceOnObjPos = nullptr;
    GetProperty(RES_WRAP_INFLUENCE_ON_OBJPOS, MID_WRAP_INFLUENCE, pWrapInfluenceOnObjPos);
    if ( pWrapInfluenceOnObjPos )
    {
        SwFormatWrapInfluenceOnObjPos aFormatWrapInfluenceOnObjPos;
        aFormatWrapInfluenceOnObjPos.PutValue( *pWrapInfluenceOnObjPos, MID_WRAP_INFLUENCE );
        rToSet.Put(aFormatWrapInfluenceOnObjPos);
    }

    {
        const ::uno::Any* pTextVertAdjust = nullptr;
        GetProperty(RES_TEXT_VERT_ADJUST, 0, pTextVertAdjust);
        if ( pTextVertAdjust )
        {
            SdrTextVertAdjustItem aTextVertAdjust( rFromSet.Get ( RES_TEXT_VERT_ADJUST ) );
            bRet &= static_cast<SfxPoolItem&>(aTextVertAdjust).PutValue(*pTextVertAdjust, 0);
            rToSet.Put(aTextVertAdjust);
        }
    }

    return bRet;
}

class SwFrameProperties_Impl : public BaseFrameProperties_Impl
{
public:
    SwFrameProperties_Impl();

    bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrameSet, SfxItemSet& rSet, bool& rSizeFound) override;
};

SwFrameProperties_Impl::SwFrameProperties_Impl():
    BaseFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME)*/ )
{
}

static void lcl_FillCol ( SfxItemSet &rToSet, const ::SfxItemSet &rFromSet, const ::uno::Any *pAny)
{
    if ( pAny )
    {
        SwFormatCol aCol ( rFromSet.Get ( RES_COL ) );
        static_cast<SfxPoolItem&>(aCol).PutValue( *pAny, MID_COLUMNS);
        rToSet.Put(aCol);
    }
}

bool SwFrameProperties_Impl::AnyToItemSet(SwDoc *pDoc, SfxItemSet& rSet, SfxItemSet&, bool& rSizeFound)
{
    // Properties for all frames
    const ::uno::Any *pStyleName;
    SwDocStyleSheet* pStyle = nullptr;
    bool bRet;

    if ( GetProperty ( FN_UNO_FRAME_STYLE_NAME, 0, pStyleName ) )
    {
        OUString sStyle;
        *pStyleName >>= sStyle;
        SwStyleNameMapper::FillUIName(sStyle, sStyle, SwGetPoolIdFromName::FrmFmt);
        pStyle = static_cast<SwDocStyleSheet*>(pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle,
                                                    SfxStyleFamily::Frame));
    }

    const ::uno::Any* pColumns = nullptr;
    GetProperty (RES_COL, MID_COLUMNS, pColumns);
    if ( pStyle )
    {
        rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *pStyle ) );
        const ::SfxItemSet *pItemSet = &xStyle->GetItemSet();
        bRet = FillBaseProperties( rSet, *pItemSet, rSizeFound );
        lcl_FillCol ( rSet, *pItemSet, pColumns );
    }
    else
    {
        const ::SfxItemSet *pItemSet = &pDoc->getIDocumentStylePoolAccess().GetFrameFormatFromPool( RES_POOLFRM_FRAME )->GetAttrSet();
        bRet = FillBaseProperties( rSet, *pItemSet, rSizeFound );
        lcl_FillCol ( rSet, *pItemSet, pColumns );
    }
    const ::uno::Any* pEdit;
    if(GetProperty(RES_EDIT_IN_READONLY, 0, pEdit))
    {
        SwFormatEditInReadonly item(RES_EDIT_IN_READONLY);
        item.PutValue(*pEdit, 0);
        rSet.Put(item);
    }
    return bRet;
}

class SwGraphicProperties_Impl : public BaseFrameProperties_Impl
{
public:
    SwGraphicProperties_Impl();

    virtual bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrameSet, SfxItemSet& rSet, bool& rSizeFound) override;
};

SwGraphicProperties_Impl::SwGraphicProperties_Impl( ) :
    BaseFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC)*/ )
{
}

static void lcl_FillMirror ( SfxItemSet &rToSet, const ::SfxItemSet &rFromSet, const ::uno::Any *pHEvenMirror, const ::uno::Any *pHOddMirror, const ::uno::Any *pVMirror, bool &rRet )
{
    if(pHEvenMirror || pHOddMirror || pVMirror )
    {
        SwMirrorGrf aMirror ( rFromSet.Get ( RES_GRFATR_MIRRORGRF ) );
        if(pHEvenMirror)
            rRet &= static_cast<SfxPoolItem&>(aMirror).PutValue(*pHEvenMirror, MID_MIRROR_HORZ_EVEN_PAGES);
        if(pHOddMirror)
            rRet &= static_cast<SfxPoolItem&>(aMirror).PutValue(*pHOddMirror, MID_MIRROR_HORZ_ODD_PAGES);
        if(pVMirror)
            rRet &= static_cast<SfxPoolItem&>(aMirror).PutValue(*pVMirror, MID_MIRROR_VERT);
        rToSet.Put(aMirror);
    }
}

bool SwGraphicProperties_Impl::AnyToItemSet(
            SwDoc* pDoc,
            SfxItemSet& rFrameSet,
            SfxItemSet& rGrSet,
            bool& rSizeFound)
{
    // Properties for all frames
    bool bRet;
    const ::uno::Any *pStyleName;
    SwDocStyleSheet* pStyle = nullptr;

    if ( GetProperty ( FN_UNO_FRAME_STYLE_NAME, 0, pStyleName ) )
    {
        OUString sStyle;
        *pStyleName >>= sStyle;
        SwStyleNameMapper::FillUIName(sStyle, sStyle, SwGetPoolIdFromName::FrmFmt);
        pStyle = static_cast<SwDocStyleSheet*>(pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle,
                                                    SfxStyleFamily::Frame));
    }

    const ::uno::Any* pHEvenMirror = nullptr;
    const ::uno::Any* pHOddMirror = nullptr;
    const ::uno::Any* pVMirror = nullptr;
    GetProperty(RES_GRFATR_MIRRORGRF, MID_MIRROR_HORZ_EVEN_PAGES, pHEvenMirror);
    GetProperty(RES_GRFATR_MIRRORGRF, MID_MIRROR_HORZ_ODD_PAGES, pHOddMirror);
    GetProperty(RES_GRFATR_MIRRORGRF, MID_MIRROR_VERT, pVMirror);

    if ( pStyle )
    {
        rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet(*pStyle) );
        const ::SfxItemSet *pItemSet = &xStyle->GetItemSet();
        bRet = FillBaseProperties(rFrameSet, *pItemSet, rSizeFound);
        lcl_FillMirror ( rGrSet, *pItemSet, pHEvenMirror, pHOddMirror, pVMirror, bRet );
    }
    else
    {
        const ::SfxItemSet *pItemSet = &pDoc->getIDocumentStylePoolAccess().GetFrameFormatFromPool( RES_POOLFRM_GRAPHIC )->GetAttrSet();
        bRet = FillBaseProperties(rFrameSet, *pItemSet, rSizeFound);
        lcl_FillMirror ( rGrSet, *pItemSet, pHEvenMirror, pHOddMirror, pVMirror, bRet );
    }

    static const ::sal_uInt16 nIDs[] =
    {
        RES_GRFATR_CROPGRF,
        RES_GRFATR_ROTATION,
        RES_GRFATR_LUMINANCE,
        RES_GRFATR_CONTRAST,
        RES_GRFATR_CHANNELR,
        RES_GRFATR_CHANNELG,
        RES_GRFATR_CHANNELB,
        RES_GRFATR_GAMMA,
        RES_GRFATR_INVERT,
        RES_GRFATR_TRANSPARENCY,
        RES_GRFATR_DRAWMODE,
        0
    };
    const ::uno::Any* pAny;
    for(sal_Int16 nIndex = 0; nIDs[nIndex]; nIndex++)
    {
        sal_uInt8 nMId = RES_GRFATR_CROPGRF == nIDs[nIndex] ? CONVERT_TWIPS : 0;
        if(GetProperty(nIDs[nIndex], nMId, pAny ))
        {
            std::unique_ptr<SfxPoolItem> pItem(::GetDfltAttr( nIDs[nIndex] )->Clone());
            bRet &= pItem->PutValue(*pAny, nMId );
            rGrSet.Put(*pItem);
        }
    }

    return bRet;
}

class SwOLEProperties_Impl : public SwFrameProperties_Impl
{
public:
    SwOLEProperties_Impl() :
        SwFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_EMBEDDED_OBJECT)*/ ){}

    virtual bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrameSet, SfxItemSet& rSet, bool& rSizeFound) override;
};

bool SwOLEProperties_Impl::AnyToItemSet(
        SwDoc* pDoc, SfxItemSet& rFrameSet, SfxItemSet& rSet, bool& rSizeFound)
{
    const ::uno::Any* pTemp;
    if(!GetProperty(FN_UNO_CLSID, 0, pTemp) && !GetProperty(FN_UNO_STREAM_NAME, 0, pTemp)
         && !GetProperty(FN_EMBEDDED_OBJECT, 0, pTemp)
         && !GetProperty(FN_UNO_VISIBLE_AREA_WIDTH, 0, pTemp)
         && !GetProperty(FN_UNO_VISIBLE_AREA_HEIGHT, 0, pTemp) )
        return false;
    SwFrameProperties_Impl::AnyToItemSet( pDoc, rFrameSet, rSet, rSizeFound);

    return true;
}

class SwXFrame::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper2

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::comphelper::OInterfaceContainerHelper2 m_EventListeners;

    Impl() : m_EventListeners(m_Mutex) { }
};

namespace
{
    class theSwXFrameUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXFrameUnoTunnelId > {};
}

const ::uno::Sequence< sal_Int8 > & SwXFrame::getUnoTunnelId()
{
    return theSwXFrameUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXFrame::getSomething( const ::uno::Sequence< sal_Int8 >& rId )
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}


OUString SwXFrame::getImplementationName()
{
    return OUString("SwXFrame");
}

sal_Bool SwXFrame::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXFrame::getSupportedServiceNames()
{
    uno::Sequence< OUString > aRet(3);
    aRet[0] = "com.sun.star.text.BaseFrame";
    aRet[1] = "com.sun.star.text.TextContent";
    aRet[2] = "com.sun.star.document.LinkTarget";
    return aRet;
}

SwXFrame::SwXFrame(FlyCntType eSet, const ::SfxItemPropertySet* pSet, SwDoc *pDoc)
    : m_pImpl(new Impl)
    , m_pPropSet(pSet)
    , m_pDoc(pDoc)
    , eType(eSet)
    , bIsDescriptor(true)
    , m_nDrawAspect(embed::Aspects::MSOLE_CONTENT)
    , m_nVisibleAreaWidth(0)
    , m_nVisibleAreaHeight(0)
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->getIDocumentStylePoolAccess().GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    // get the property set for the default style data
    // First get the model
    uno::Reference < XModel > xModel = pDoc->GetDocShell()->GetBaseModel();
    // Ask the model for its family supplier interface
    uno::Reference < XStyleFamiliesSupplier > xFamilySupplier ( xModel, uno::UNO_QUERY );
    // Get the style families
    uno::Reference < XNameAccess > xFamilies = xFamilySupplier->getStyleFamilies();
    // Get the Frame family (and keep it for later)
    const ::uno::Any aAny = xFamilies->getByName ("FrameStyles");
    aAny >>= mxStyleFamily;
    // In the derived class, we'll ask mxStyleFamily for the relevant default style
    // mxStyleFamily is initialised in the SwXFrame constructor
    switch(eType)
    {
        case FLYCNTTYPE_FRM:
        {
            uno::Any aAny2 = mxStyleFamily->getByName ("Frame");
            aAny2 >>= mxStyleData;
            m_pProps.reset(new SwFrameProperties_Impl);
        }
        break;
        case FLYCNTTYPE_GRF:
        {
            uno::Any aAny2 = mxStyleFamily->getByName ("Graphics");
            aAny2 >>= mxStyleData;
            m_pProps.reset(new SwGraphicProperties_Impl);
        }
        break;
        case FLYCNTTYPE_OLE:
        {
            uno::Any aAny2 = mxStyleFamily->getByName ("OLE");
            aAny2 >>= mxStyleData;
            m_pProps.reset(new SwOLEProperties_Impl);
        }
        break;

        default:
            m_pProps.reset();
        break;
    }
}

SwXFrame::SwXFrame(SwFrameFormat& rFrameFormat, FlyCntType eSet, const ::SfxItemPropertySet* pSet)
    : SwClient(&rFrameFormat)
    , m_pImpl(new Impl)
    , m_pPropSet(pSet)
    , m_pDoc(nullptr)
    , eType(eSet)
    , bIsDescriptor(false)
    , m_nDrawAspect(embed::Aspects::MSOLE_CONTENT)
    , m_nVisibleAreaWidth(0)
    , m_nVisibleAreaHeight(0)
{
}

SwXFrame::~SwXFrame()
{
    SolarMutexGuard aGuard;
    m_pCopySource.reset();
    m_pProps.reset();
    EndListeningAll();
}

template<class Interface, class NameLookupIsHard>
uno::Reference<Interface>
SwXFrame::CreateXFrame(SwDoc & rDoc, SwFrameFormat *const pFrameFormat)
{
    assert(!pFrameFormat || &rDoc == pFrameFormat->GetDoc());
    uno::Reference<Interface> xFrame;
    if (pFrameFormat)
    {
        xFrame.set(pFrameFormat->GetXObject(), uno::UNO_QUERY); // cached?
    }
    if (!xFrame.is())
    {
        NameLookupIsHard *const pNew(pFrameFormat
                ? new NameLookupIsHard(*pFrameFormat)
                : new NameLookupIsHard(&rDoc));
        xFrame.set(pNew);
        if (pFrameFormat)
        {
            pFrameFormat->SetXObject(xFrame);
        }
        // need a permanent Reference to initialize m_wThis
        pNew->SwXFrame::m_pImpl->m_wThis = xFrame;
    }
    return xFrame;
}

OUString SwXFrame::getName()
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
        return pFormat->GetName();
    if(!bIsDescriptor)
        throw uno::RuntimeException();
    return m_sName;
}

void SwXFrame::setName(const OUString& rName)
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        pFormat->GetDoc()->SetFlyName(static_cast<SwFlyFrameFormat&>(*pFormat), rName);
        if(pFormat->GetName() != rName)
        {
            throw uno::RuntimeException();
        }
    }
    else if(bIsDescriptor)
        m_sName = rName;
    else
        throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySetInfo >  SwXFrame::getPropertySetInfo()
{
    uno::Reference< beans::XPropertySetInfo >  xRef;
    static uno::Reference< beans::XPropertySetInfo >  xFrameRef;
    static uno::Reference< beans::XPropertySetInfo >  xGrfRef;
    static uno::Reference< beans::XPropertySetInfo >  xOLERef;
    switch(eType)
    {
    case FLYCNTTYPE_FRM:
        if( !xFrameRef.is() )
            xFrameRef = m_pPropSet->getPropertySetInfo();
        xRef = xFrameRef;
        break;
    case FLYCNTTYPE_GRF:
        if( !xGrfRef.is() )
            xGrfRef = m_pPropSet->getPropertySetInfo();
        xRef = xGrfRef;
        break;
    case FLYCNTTYPE_OLE:
        if( !xOLERef.is() )
            xOLERef = m_pPropSet->getPropertySetInfo();
        xRef = xOLERef;
        break;
    default:
        ;
    }
    return xRef;
}

void SwXFrame::SetSelection(SwPaM& rCopySource)
{
    m_pCopySource.reset(new SwPaM(*rCopySource.Start()));
    m_pCopySource->SetMark();
    *m_pCopySource->GetMark() = *rCopySource.End();
}

SdrObject *SwXFrame::GetOrCreateSdrObject(SwFlyFrameFormat &rFormat)
{
    SdrObject* pObject = rFormat.FindSdrObject();
    if( !pObject )
    {
        SwDoc *pDoc = rFormat.GetDoc();
        // #i52858# - method name changed
        SwFlyDrawContact* pContactObject(rFormat.GetOrCreateContact());
        pObject = pContactObject->GetMaster();

        const ::SwFormatSurround& rSurround = rFormat.GetSurround();
        pObject->SetLayer(
            ( css::text::WrapTextMode_THROUGH == rSurround.GetSurround() &&
              !rFormat.GetOpaque().GetValue() ) ? pDoc->getIDocumentDrawModelAccess().GetHellId()
                                             : pDoc->getIDocumentDrawModelAccess().GetHeavenId() );
        SwDrawModel* pDrawModel = pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
        pDrawModel->GetPage(0)->InsertObject( pObject );
    }

    return pObject;
}

static SwFrameFormat *lcl_GetFrameFormat( const ::uno::Any& rValue, SwDoc *pDoc )
{
    SwFrameFormat *pRet = nullptr;
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if(pDocSh)
    {
        OUString uTemp;
        rValue >>= uTemp;
        OUString sStyle;
        SwStyleNameMapper::FillUIName(uTemp, sStyle,
                SwGetPoolIdFromName::FrmFmt);
        SwDocStyleSheet* pStyle =
                static_cast<SwDocStyleSheet*>(pDocSh->GetStyleSheetPool()->Find(sStyle,
                                                    SfxStyleFamily::Frame));
        if(pStyle)
            pRet = pStyle->GetFrameFormat();
    }

    return pRet;
}

void SwXFrame::setPropertyValue(const OUString& rPropertyName, const ::uno::Any& _rValue)
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    const ::SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);

    if (!pEntry)
        throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const sal_uInt8 nMemberId(pEntry->nMemberId);
    uno::Any aValue(_rValue);

    // check for needed metric translation
    if(pEntry->nMoreFlags & PropertyMoreFlags::METRIC_ITEM)
    {
        bool bDoIt(true);

        if(XATTR_FILLBMP_SIZEX == pEntry->nWID || XATTR_FILLBMP_SIZEY == pEntry->nWID)
        {
            // exception: If these ItemTypes are used, do not convert when these are negative
            // since this means they are intended as percent values
            sal_Int32 nValue = 0;

            if(aValue >>= nValue)
            {
                bDoIt = nValue > 0;
            }
        }

        if(bDoIt)
        {
            const SwDoc* pDoc = (IsDescriptor() ? m_pDoc : GetFrameFormat()->GetDoc());
            const SfxItemPool& rPool = pDoc->GetAttrPool();
            const MapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

            if(eMapUnit != MapUnit::Map100thMM)
            {
                SvxUnoConvertFromMM(eMapUnit, aValue);
            }
        }
    }

    if(pFormat)
    {
        bool bNextFrame = false;
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        SwDoc* pDoc = pFormat->GetDoc();
        if ( ((eType == FLYCNTTYPE_GRF) && isGRFATR(pEntry->nWID)) ||
            (FN_PARAM_CONTOUR_PP         == pEntry->nWID) ||
            (FN_UNO_IS_AUTOMATIC_CONTOUR == pEntry->nWID) ||
            (FN_UNO_IS_PIXEL_CONTOUR     == pEntry->nWID) )
        {
            const ::SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTextNode* pNoText = aIdx.GetNode().GetNoTextNode();
                if(pEntry->nWID == FN_PARAM_CONTOUR_PP)
                {
                    drawing::PointSequenceSequence aParam;
                    if(!aValue.hasValue())
                        pNoText->SetContour(nullptr);
                    else if(aValue >>= aParam)
                    {
                        tools::PolyPolygon aPoly(static_cast<sal_uInt16>(aParam.getLength()));
                        for(sal_Int32 i = 0; i < aParam.getLength(); i++)
                        {
                            const ::drawing::PointSequence* pPointSeq = aParam.getConstArray();
                            sal_Int32 nPoints = pPointSeq[i].getLength();
                            const ::awt::Point* pPoints = pPointSeq[i].getConstArray();
                            tools::Polygon aSet( static_cast<sal_uInt16>(nPoints) );
                            for(sal_Int32 j = 0; j < nPoints; j++)
                            {
                                Point aPoint(pPoints[j].X, pPoints[j].Y);
                                aSet.SetPoint(aPoint, static_cast<sal_uInt16>(j));
                            }
                            // Close polygon if it isn't closed already.
                            aSet.Optimize( PolyOptimizeFlags::CLOSE );
                            aPoly.Insert( aSet );
                        }
                        pNoText->SetContourAPI( &aPoly );
                    }
                    else
                        throw lang::IllegalArgumentException();
                }
                else if(pEntry->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR )
                {
                    pNoText->SetAutomaticContour( *o3tl::doAccess<bool>(aValue) );
                }
                else if(pEntry->nWID == FN_UNO_IS_PIXEL_CONTOUR )
                {
                    // The IsPixelContour property can only be set if there
                    // is no contour, or if the contour has been set by the
                    // API itself (or in other words, if the contour isn't
                    // used already).
                    if( pNoText->HasContour_() && pNoText->IsContourMapModeValid() )
                        throw lang::IllegalArgumentException();

                    pNoText->SetPixelContour( *o3tl::doAccess<bool>(aValue) );

                }
                else
                {
                    SfxItemSet aSet(pNoText->GetSwAttrSet());
                    m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
                    pNoText->SetAttr(aSet);
                }
            }
        }
        // New attribute Title
        else if( FN_UNO_TITLE == pEntry->nWID )
        {
            SwFlyFrameFormat& rFlyFormat = dynamic_cast<SwFlyFrameFormat&>(*pFormat);
            OUString sTitle;
            aValue >>= sTitle;
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject(rFlyFormat);
            rFlyFormat.GetDoc()->SetFlyFrameTitle(rFlyFormat, sTitle);
        }
        // New attribute Description
        else if( FN_UNO_DESCRIPTION == pEntry->nWID )
        {
            SwFlyFrameFormat& rFlyFormat = dynamic_cast<SwFlyFrameFormat&>(*pFormat);
            OUString sDescription;
            aValue >>= sDescription;
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject(rFlyFormat);
            rFlyFormat.GetDoc()->SetFlyFrameDescription(rFlyFormat, sDescription);
        }
        else if(FN_UNO_FRAME_STYLE_NAME == pEntry->nWID)
        {
            SwFrameFormat *pFrameFormat = lcl_GetFrameFormat( aValue, pFormat->GetDoc() );
            if( !pFrameFormat )
                throw lang::IllegalArgumentException();

            UnoActionContext aAction(pFormat->GetDoc());

            std::unique_ptr<SfxItemSet> pSet;
            // #i31771#, #i25798# - No adjustment of
            // anchor ( no call of method <sw_ChkAndSetNewAnchor(..)> ),
            // if document is currently in reading mode.
            if ( !pFormat->GetDoc()->IsInReading() )
            {
                // see SwFEShell::SetFrameFormat( SwFrameFormat *pNewFormat, bool bKeepOrient, Point* pDocPos )
                SwFlyFrame *pFly = nullptr;
                if (auto pFlyFrameFormat = dynamic_cast<const SwFlyFrameFormat*>(pFormat) )
                    pFly = pFlyFrameFormat->GetFrame();
                if ( pFly )
                {
                    const ::SfxPoolItem* pItem;
                    if( SfxItemState::SET == pFrameFormat->GetItemState( RES_ANCHOR, false, &pItem ))
                    {
                        pSet.reset(new SfxItemSet( pDoc->GetAttrPool(), aFrameFormatSetRange ));
                        pSet->Put( *pItem );
                        if ( pFormat->GetDoc()->GetEditShell() != nullptr
                             && !sw_ChkAndSetNewAnchor( *pFly, *pSet ) )
                        {
                            pSet.reset();
                        }
                    }
                }
            }

            pFormat->GetDoc()->SetFrameFormatToFly( *pFormat, *pFrameFormat, pSet.get() );
        }
        else if (FN_UNO_GRAPHIC_FILTER == pEntry->nWID)
        {
            OUString sGrfName;
            OUString sFltName;
            SwDoc::GetGrfNms( *static_cast<SwFlyFrameFormat*>(pFormat), &sGrfName, &sFltName );
            aValue >>= sFltName;
            UnoActionContext aAction(pFormat->GetDoc());
            const ::SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if (pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                {
                    throw uno::RuntimeException();
                }
                SwPaM aGrfPaM(*pGrfNode);
                pFormat->GetDoc()->getIDocumentContentOperations().ReRead(aGrfPaM, sGrfName, sFltName, nullptr);
            }
        }
        else if (FN_UNO_GRAPHIC == pEntry->nWID || FN_UNO_GRAPHIC_URL == pEntry->nWID)
        {
            Graphic aGraphic;
            if (aValue.has<OUString>())
            {
                OUString aURL = aValue.get<OUString>();
                if (!aURL.isEmpty())
                {
                    aGraphic = vcl::graphic::loadFromURL(aURL);
                }
            }
            else if (aValue.has<uno::Reference<graphic::XGraphic>>())
            {
                uno::Reference<graphic::XGraphic> xGraphic;
                xGraphic = aValue.get<uno::Reference<graphic::XGraphic>>();
                if (xGraphic.is())
                {
                    aGraphic = Graphic(xGraphic);
                }
            }

            if (aGraphic)
            {
                const ::SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
                if (pIdx)
                {
                    SwNodeIndex aIdx(*pIdx, 1);
                    SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                    if (!pGrfNode)
                    {
                        throw uno::RuntimeException();
                    }
                    SwPaM aGrfPaM(*pGrfNode);
                    pFormat->GetDoc()->getIDocumentContentOperations().ReRead(aGrfPaM, OUString(), OUString(), &aGraphic);
                }
            }
        }
        else if (FN_UNO_REPLACEMENT_GRAPHIC == pEntry->nWID || FN_UNO_REPLACEMENT_GRAPHIC_URL == pEntry->nWID)
        {
            Graphic aGraphic;
            if (aValue.has<OUString>())
            {
                OUString aURL = aValue.get<OUString>();
                if (!aURL.isEmpty())
                {
                    aGraphic = vcl::graphic::loadFromURL(aURL);
                }
            }
            else if (aValue.has<uno::Reference<graphic::XGraphic>>())
            {
                uno::Reference<graphic::XGraphic> xGraphic;
                xGraphic = aValue.get<uno::Reference<graphic::XGraphic>>();
                if (xGraphic.is())
                {
                    aGraphic = Graphic(xGraphic);
                }
            }

            if (aGraphic)
            {
                const ::SwFormatContent* pCnt = &pFormat->GetContent();
                if ( pCnt->GetContentIdx() && pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ] )
                {
                    SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ]->GetOLENode();

                    if ( pOleNode )
                    {
                        svt::EmbeddedObjectRef &rEmbeddedObject = pOleNode->GetOLEObj().GetObject();
                        rEmbeddedObject.SetGraphic(aGraphic, OUString() );
                    }
                }
            }
        }
        else if((bNextFrame = (rPropertyName == UNO_NAME_CHAIN_NEXT_NAME))
            || rPropertyName == UNO_NAME_CHAIN_PREV_NAME)
        {
            OUString sChainName;
            aValue >>= sChainName;
            if (sChainName.isEmpty())
            {
                if(bNextFrame)
                    pDoc->Unchain(*pFormat);
                else
                {
                    const SwFormatChain& aChain( pFormat->GetChain() );
                    SwFrameFormat *pPrev = aChain.GetPrev();
                    if(pPrev)
                        pDoc->Unchain(*pPrev);
                }
            }
            else
            {
                const size_t nCount = pDoc->GetFlyCount(FLYCNTTYPE_FRM);

                SwFrameFormat* pChain = nullptr;
                for( size_t i = 0; i < nCount; ++i )
                {
                    SwFrameFormat* pFormat2 = pDoc->GetFlyNum(i, FLYCNTTYPE_FRM);
                    if(sChainName == pFormat2->GetName() )
                    {
                        pChain = pFormat2;
                        break;
                    }
                }
                if(pChain)
                {
                    SwFrameFormat* pSource = bNextFrame ? pFormat : pChain;
                    SwFrameFormat* pDest = bNextFrame ? pChain: pFormat;
                    pDoc->Chain(*pSource, *pDest);
                }
            }
        }
        else if(FN_UNO_Z_ORDER == pEntry->nWID)
        {
            sal_Int32 nZOrder = - 1;
            aValue >>= nZOrder;

            // Don't set an explicit ZOrder on TextBoxes.
            if( nZOrder >= 0 && !SwTextBoxHelper::isTextBox(pFormat, RES_FLYFRMFMT) )
            {
                SdrObject* pObject =
                    GetOrCreateSdrObject( static_cast<SwFlyFrameFormat&>(*pFormat) );
                SwDrawModel *pDrawModel = pDoc->getIDocumentDrawModelAccess().GetDrawModel();
                pDrawModel->GetPage(0)->
                            SetObjectOrdNum(pObject->GetOrdNum(), nZOrder);
            }
        }
        else if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORFRAME == nMemberId)
        {
            bool bDone = false;
            uno::Reference<text::XTextFrame> xFrame;
            if(aValue >>= xFrame)
            {
                uno::Reference<lang::XUnoTunnel> xTunnel(xFrame, uno::UNO_QUERY);
                SwXFrame* pFrame = xTunnel.is() ?
                        reinterpret_cast< SwXFrame * >( sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(SwXFrame::getUnoTunnelId()) ))
                        : nullptr;
                if(pFrame && this != pFrame && pFrame->GetFrameFormat() && pFrame->GetFrameFormat()->GetDoc() == pDoc)
                {
                    SfxItemSet aSet( pDoc->GetAttrPool(),
                                svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>{} );
                    aSet.SetParent(&pFormat->GetAttrSet());
                    SwFormatAnchor aAnchor = static_cast<const SwFormatAnchor&>(aSet.Get(pEntry->nWID));

                    SwPosition aPos(*pFrame->GetFrameFormat()->GetContent().GetContentIdx());
                    aAnchor.SetAnchor(&aPos);
                    aAnchor.SetType(RndStdIds::FLY_AT_FLY);
                    aSet.Put(aAnchor);
                    pDoc->SetFlyFrameAttr( *pFormat, aSet );
                    bDone = true;
                }
            }
            if(!bDone)
                throw lang::IllegalArgumentException();
        }
        else
        {
            // standard UNO API write attributes
            // adapt former attr from SvxBrushItem::PutValue to new items XATTR_FILL_FIRST, XATTR_FILL_LAST
            SfxItemSet aSet( pDoc->GetAttrPool(),
                svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
                RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,

                // FillAttribute support
                XATTR_FILL_FIRST, XATTR_FILL_LAST>{});
            bool bDone(false);

            aSet.SetParent(&pFormat->GetAttrSet());

            if(RES_BACKGROUND == pEntry->nWID)
            {
                const SwAttrSet& rSet = pFormat->GetAttrSet();
                const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(rSet, RES_BACKGROUND, true, pDoc->IsInXMLImport()));
                SvxBrushItem aChangedBrushItem(aOriginalBrushItem);

                aChangedBrushItem.PutValue(aValue, nMemberId);

                if(aChangedBrushItem != aOriginalBrushItem)
                {
                    setSvxBrushItemAsFillAttributesToTargetSet(aChangedBrushItem, aSet);
                    pFormat->GetDoc()->SetFlyFrameAttr( *pFormat, aSet );
                }

                bDone = true;
            }
            else if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
            {
                drawing::BitmapMode eMode;

                if(!(aValue >>= eMode))
                {
                    sal_Int32 nMode = 0;

                    if(!(aValue >>= nMode))
                    {
                        throw lang::IllegalArgumentException();
                    }

                    eMode = static_cast<drawing::BitmapMode>(nMode);
                }

                aSet.Put(XFillBmpStretchItem(drawing::BitmapMode_STRETCH == eMode));
                aSet.Put(XFillBmpTileItem(drawing::BitmapMode_REPEAT == eMode));
                pFormat->GetDoc()->SetFlyFrameAttr( *pFormat, aSet );
                bDone = true;
            }

            switch(nMemberId)
            {
                case MID_NAME:
                {
                    // when named items get set, replace these with the NameOrIndex items
                    // which exist already in the pool
                    switch(pEntry->nWID)
                    {
                        case XATTR_FILLGRADIENT:
                        case XATTR_FILLHATCH:
                        case XATTR_FILLBITMAP:
                        case XATTR_FILLFLOATTRANSPARENCE:
                        {
                            OUString aTempName;

                            if(!(aValue >>= aTempName ))
                            {
                                throw lang::IllegalArgumentException();
                            }

                            bDone = SvxShape::SetFillAttribute(pEntry->nWID, aTempName, aSet);
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    break;
                }
                case MID_BITMAP:
                {
                    switch(pEntry->nWID)
                    {
                        case XATTR_FILLBITMAP:
                        {
                            const Graphic aNullGraphic;
                            XFillBitmapItem aXFillBitmapItem(aNullGraphic);

                            aXFillBitmapItem.PutValue(aValue, nMemberId);
                            aSet.Put(aXFillBitmapItem);
                            bDone = true;
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                    break;
                }
                default:
                {
                    break;
                }
            }

            if(!bDone)
            {
                m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
            }

            if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORTYPE == nMemberId)
            {
                SwFormatAnchor aAnchor = static_cast<const SwFormatAnchor&>(aSet.Get(pEntry->nWID));
                if(aAnchor.GetAnchorId() == RndStdIds::FLY_AT_FLY)
                {
                    const ::SwPosition* pPosition = aAnchor.GetContentAnchor();
                    SwFrameFormat* pFlyFormat = pPosition ? pPosition->nNode.GetNode().GetFlyFormat() : nullptr;
                    if(!pFlyFormat || pFlyFormat->Which() == RES_DRAWFRMFMT)
                    {
                        lang::IllegalArgumentException aExcept;
                        aExcept.Message = "Anchor to frame: no frame found";
                        throw aExcept;
                    }
                    else
                    {
                        SwPosition aPos = *pPosition;
                        aPos.nNode = *pFlyFormat->GetContent().GetContentIdx();
                        aAnchor.SetAnchor(&aPos);
                        aSet.Put(aAnchor);
                    }
                }
                else if ((aAnchor.GetAnchorId() != RndStdIds::FLY_AT_PAGE) &&
                         !aAnchor.GetContentAnchor())
                {
                    SwNode& rNode = pDoc->GetNodes().GetEndOfContent();
                    SwPaM aPam(rNode);
                    aPam.Move( fnMoveBackward, GoInDoc );
                    aAnchor.SetAnchor( aPam.Start() );
                    aSet.Put(aAnchor);
                }

                // #i31771#, #i25798# - No adjustment of
                // anchor ( no call of method <sw_ChkAndSetNewAnchor(..)> ),
                // if document is currently in reading mode.
                if ( !pFormat->GetDoc()->IsInReading() )
                {
                    // see SwFEShell::SetFlyFrameAttr( SfxItemSet& rSet )
                    SwFlyFrame *pFly = nullptr;
                    if (auto pFrameFormat = dynamic_cast<SwFlyFrameFormat*>( pFormat) )
                        pFly = pFrameFormat->GetFrame();
                    if (pFly)
                    {
                        const ::SfxPoolItem* pItem;
                        if( SfxItemState::SET == aSet.GetItemState( RES_ANCHOR, false, &pItem ))
                        {
                            aSet.Put( *pItem );
                            if ( pFormat->GetDoc()->GetEditShell() != nullptr )
                            {
                                sw_ChkAndSetNewAnchor( *pFly, aSet );
                            }
                        }
                    }
                }

                pFormat->GetDoc()->SetFlyFrameAttr( *pFormat, aSet );
            }
            else if(FN_UNO_CLSID == pEntry->nWID || FN_UNO_STREAM_NAME == pEntry->nWID || FN_EMBEDDED_OBJECT == pEntry->nWID)
            {
                throw lang::IllegalArgumentException();
            }
            else
            {
                pFormat->SetFormatAttr(aSet);
            }
        }
    }
    else if(IsDescriptor())
    {
        m_pProps->SetProperty(pEntry->nWID, nMemberId, aValue);
        if( FN_UNO_FRAME_STYLE_NAME == pEntry->nWID )
        {
            OUString sStyleName;
            aValue >>= sStyleName;
            try
            {
                uno::Any aAny = mxStyleFamily->getByName ( sStyleName );
                aAny >>= mxStyleData;
            }
            catch ( container::NoSuchElementException const & )
            {
            }
            catch ( lang::WrappedTargetException const  & )
            {
            }
            catch ( uno::RuntimeException const & )
            {
            }
        }
        else if (FN_UNO_DRAW_ASPECT == pEntry->nWID)
        {
            OUString sAspect = "";
            aValue >>= sAspect;

            if (sAspect == "Icon")
                m_nDrawAspect = embed::Aspects::MSOLE_ICON;
            else if (sAspect == "Content")
                m_nDrawAspect = embed::Aspects::MSOLE_CONTENT;
        }
        else if (FN_UNO_VISIBLE_AREA_WIDTH == pEntry->nWID)
        {
            OUString sAspect = "";
            aValue >>= sAspect;
            m_nVisibleAreaWidth = sAspect.toInt64();
        }
        else if (FN_UNO_VISIBLE_AREA_HEIGHT == pEntry->nWID)
        {
            OUString sAspect = "";
            aValue >>= sAspect;
            m_nVisibleAreaHeight = sAspect.toInt64();
        }
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXFrame::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    uno::Any aAny;
    SwFrameFormat* pFormat = GetFrameFormat();
    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
        throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    const sal_uInt8 nMemberId(pEntry->nMemberId);

    if(FN_UNO_ANCHOR_TYPES == pEntry->nWID)
    {
        uno::Sequence<text::TextContentAnchorType> aTypes(5);
        text::TextContentAnchorType* pArray = aTypes.getArray();
        pArray[0] = text::TextContentAnchorType_AT_PARAGRAPH;
        pArray[1] = text::TextContentAnchorType_AS_CHARACTER;
        pArray[2] = text::TextContentAnchorType_AT_PAGE;
        pArray[3] = text::TextContentAnchorType_AT_FRAME;
        pArray[4] = text::TextContentAnchorType_AT_CHARACTER;
        aAny <<= aTypes;
    }
    else if(pFormat)
    {
        if( ((eType == FLYCNTTYPE_GRF) || (eType == FLYCNTTYPE_OLE)) &&
                (isGRFATR(pEntry->nWID) ||
                        pEntry->nWID == FN_PARAM_CONTOUR_PP ||
                        pEntry->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR ||
                        pEntry->nWID == FN_UNO_IS_PIXEL_CONTOUR ))
        {
            const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTextNode* pNoText = aIdx.GetNode().GetNoTextNode();
                if(pEntry->nWID == FN_PARAM_CONTOUR_PP)
                {
                    tools::PolyPolygon aContour;
                    if( pNoText->GetContourAPI( aContour ) )
                    {
                        drawing::PointSequenceSequence aPtSeq(aContour.Count());
                        drawing::PointSequence* pPSeq = aPtSeq.getArray();
                        for(sal_uInt16 i = 0; i < aContour.Count(); i++)
                        {
                            const tools::Polygon& rPoly = aContour.GetObject(i);
                            pPSeq[i].realloc(rPoly.GetSize());
                            awt::Point* pPoints = pPSeq[i].getArray();
                            for(sal_uInt16 j = 0; j < rPoly.GetSize(); j++)
                            {
                                const Point& rPoint = rPoly.GetPoint(j);
                                pPoints[j].X = rPoint.X();
                                pPoints[j].Y = rPoint.Y();
                            }
                        }
                        aAny <<= aPtSeq;
                    }
                }
                else if(pEntry->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR )
                {
                    aAny <<= pNoText->HasAutomaticContour();
                }
                else if(pEntry->nWID == FN_UNO_IS_PIXEL_CONTOUR )
                {
                    aAny <<= pNoText->IsPixelContour();
                }
                else
                {
                    const SfxItemSet& aSet(pNoText->GetSwAttrSet());
                    m_pPropSet->getPropertyValue(*pEntry, aSet, aAny);
                }
            }
        }
        else if (FN_UNO_REPLACEMENT_GRAPHIC == pEntry->nWID)
        {
            const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            uno::Reference<graphic::XGraphic> xGraphic;

            if (pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if (!pGrfNode)
                    throw uno::RuntimeException();

                const GraphicObject* pGraphicObject = pGrfNode->GetReplacementGrfObj();

                if (pGraphicObject)
                {
                    xGraphic = pGraphicObject->GetGraphic().GetXGraphic();
                }
            }
            aAny <<= xGraphic;
        }
        else if( FN_UNO_GRAPHIC_FILTER == pEntry->nWID )
        {
            OUString sFltName;
            SwDoc::GetGrfNms( *static_cast<SwFlyFrameFormat*>(pFormat), nullptr, &sFltName );
            aAny <<= sFltName;
        }
        else if( FN_UNO_GRAPHIC_URL == pEntry->nWID )
        {
            throw uno::RuntimeException("Getting from this property is not supported");
        }
        else if( FN_UNO_GRAPHIC == pEntry->nWID )
        {
            const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                    throw uno::RuntimeException();
                aAny <<= pGrfNode->GetGrf().GetXGraphic();
            }
        }
        else if(FN_UNO_FRAME_STYLE_NAME == pEntry->nWID)
        {
            aAny <<= SwStyleNameMapper::GetProgName(pFormat->DerivedFrom()->GetName(), SwGetPoolIdFromName::FrmFmt );
        }
        // #i73249#
        else if( FN_UNO_TITLE == pEntry->nWID )
        {
            SwFlyFrameFormat& rFlyFormat = dynamic_cast<SwFlyFrameFormat&>(*pFormat);
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject(rFlyFormat);
            aAny <<= rFlyFormat.GetObjTitle();
        }
        // New attribute Description
        else if( FN_UNO_DESCRIPTION == pEntry->nWID )
        {
            SwFlyFrameFormat& rFlyFormat = dynamic_cast<SwFlyFrameFormat&>(*pFormat);
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject(rFlyFormat);
            aAny <<= rFlyFormat.GetObjDescription();
        }
        else if(eType == FLYCNTTYPE_GRF &&
                (rPropertyName == UNO_NAME_ACTUAL_SIZE))
        {
            const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                Size aActSize = aIdx.GetNode().GetNoTextNode()->GetTwipSize();
                awt::Size aTmp;
                aTmp.Width = convertTwipToMm100(aActSize.Width());
                aTmp.Height = convertTwipToMm100(aActSize.Height());
                aAny <<= aTmp;
            }
        }
        else if(FN_PARAM_LINK_DISPLAY_NAME == pEntry->nWID)
        {
            aAny <<= pFormat->GetName();
        }
        else if(FN_UNO_Z_ORDER == pEntry->nWID)
        {
            const SdrObject* pObj = pFormat->FindRealSdrObject();
            if( pObj == nullptr )
                pObj = pFormat->FindSdrObject();
            if( pObj )
            {
                aAny <<= static_cast<sal_Int32>(pObj->GetOrdNum());
            }
        }
        else if(FN_UNO_CLSID == pEntry->nWID || FN_UNO_MODEL == pEntry->nWID||
                FN_UNO_COMPONENT == pEntry->nWID ||FN_UNO_STREAM_NAME == pEntry->nWID||
                FN_EMBEDDED_OBJECT == pEntry->nWID)
        {
            SwDoc* pDoc = pFormat->GetDoc();
            const SwFormatContent* pCnt = &pFormat->GetContent();
            OSL_ENSURE( pCnt->GetContentIdx() &&
                           pDoc->GetNodes()[ pCnt->GetContentIdx()->
                                            GetIndex() + 1 ]->GetOLENode(), "no OLE-Node?");

            SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetContentIdx()
                                            ->GetIndex() + 1 ]->GetOLENode();
            uno::Reference < embed::XEmbeddedObject > xIP = pOleNode->GetOLEObj().GetOleRef();
            OUString aHexCLSID;
            {
                SvGlobalName aClassName( xIP->getClassID() );
                aHexCLSID = aClassName.GetHexName();
                if(FN_UNO_CLSID != pEntry->nWID)
                {
                    if ( svt::EmbeddedObjectRef::TryRunningState( xIP ) )
                    {
                        uno::Reference < lang::XComponent > xComp( xIP->getComponent(), uno::UNO_QUERY );
                        uno::Reference < frame::XModel > xModel( xComp, uno::UNO_QUERY );
                        if ( FN_EMBEDDED_OBJECT == pEntry->nWID )
                        {
                            // when exposing the EmbeddedObject, ensure it has a client site
                            OSL_ENSURE( pDoc->GetDocShell(), "no doc shell => no client site" );
                            if ( pDoc->GetDocShell() )
                                pDoc->GetDocShell()->GetIPClient( svt::EmbeddedObjectRef( xIP, embed::Aspects::MSOLE_CONTENT ) );
                            aAny <<= xIP;
                        }
                        else if ( xModel.is() )
                            aAny <<= xModel;
                        else if ( FN_UNO_COMPONENT == pEntry->nWID )
                            aAny <<= xComp;
                    }
                }
            }

            if(FN_UNO_CLSID == pEntry->nWID)
                aAny <<= aHexCLSID;
            else if(FN_UNO_STREAM_NAME == pEntry->nWID)
            {
                aAny <<= pOleNode->GetOLEObj().GetCurrentPersistName();
            }
            else if(FN_EMBEDDED_OBJECT == pEntry->nWID)
            {
                aAny <<= pOleNode->GetOLEObj().GetOleRef();
            }
        }
        else if(WID_LAYOUT_SIZE == pEntry->nWID)
        {
            // format document completely in order to get correct value
            pFormat->GetDoc()->GetEditShell()->CalcLayout();

            SwFrame* pTmpFrame = SwIterator<SwFrame,SwFormat>( *pFormat ).First();
            if ( pTmpFrame )
            {
                OSL_ENSURE( pTmpFrame->isFrameAreaDefinitionValid(), "frame not valid" );
                const SwRect &rRect = pTmpFrame->getFrameArea();
                Size aMM100Size = OutputDevice::LogicToLogic(
                        Size( rRect.Width(), rRect.Height() ),
                        MapMode( MapUnit::MapTwip ), MapMode( MapUnit::Map100thMM ));
                aAny <<= awt::Size( aMM100Size.Width(), aMM100Size.Height() );
            }
        }
        else
        {
            // standard UNO API read attributes
            // adapt former attr from SvxBrushItem::PutValue to new items XATTR_FILL_FIRST, XATTR_FILL_LAST
            const SwAttrSet& rSet = pFormat->GetAttrSet();
            bool bDone(false);

            if(RES_BACKGROUND == pEntry->nWID)
            {
                const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(rSet, RES_BACKGROUND));

                if(!aOriginalBrushItem.QueryValue(aAny, nMemberId))
                {
                    OSL_ENSURE(false, "Error getting attribute from RES_BACKGROUND (!)");
                }

                bDone = true;
            }
            else if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
            {
                if (rSet.Get(XATTR_FILLBMP_TILE).GetValue())
                {
                    aAny <<= drawing::BitmapMode_REPEAT;
                }
                else if (rSet.Get(XATTR_FILLBMP_STRETCH).GetValue())
                {
                    aAny <<= drawing::BitmapMode_STRETCH;
                }
                else
                {
                    aAny <<= drawing::BitmapMode_NO_REPEAT;
                }

                bDone = true;
            }

            if(!bDone)
            {
                m_pPropSet->getPropertyValue(*pEntry, rSet, aAny);
            }
        }
    }
    else if(IsDescriptor())
    {
        if ( ! m_pDoc )
            throw uno::RuntimeException();
        if(WID_LAYOUT_SIZE != pEntry->nWID)  // there is no LayoutSize in a descriptor
        {
            const uno::Any* pAny = nullptr;
            if (!m_pProps->GetProperty(pEntry->nWID, nMemberId, pAny))
                aAny = mxStyleData->getPropertyValue( rPropertyName );
            else if ( pAny )
                aAny = *pAny;
        }
    }
    else
        throw uno::RuntimeException();

    if (pEntry->aType == ::cppu::UnoType<sal_Int16>::get() && pEntry->aType != aAny.getValueType())
    {
        // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
        sal_Int32 nValue = 0;
        aAny >>= nValue;
        aAny <<= static_cast<sal_Int16>(nValue);
    }

    // check for needed metric translation
    if(pEntry->nMoreFlags & PropertyMoreFlags::METRIC_ITEM)
    {
        bool bDoIt(true);

        if(XATTR_FILLBMP_SIZEX == pEntry->nWID || XATTR_FILLBMP_SIZEY == pEntry->nWID)
        {
            // exception: If these ItemTypes are used, do not convert when these are negative
            // since this means they are intended as percent values
            sal_Int32 nValue = 0;

            if(aAny >>= nValue)
            {
                bDoIt = nValue > 0;
            }
        }

        if(bDoIt)
        {
            const SwDoc* pDoc = (IsDescriptor() ? m_pDoc : GetFrameFormat()->GetDoc());
            const SfxItemPool& rPool = pDoc->GetAttrPool();
            const MapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

            if(eMapUnit != MapUnit::Map100thMM)
            {
                SvxUnoConvertToMM(eMapUnit, aAny);
            }
        }
    }

    return aAny;
}

void SwXFrame::addPropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXFrame::removePropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXFrame::addVetoableChangeListener(const OUString& /*PropertyName*/,
                                const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

void SwXFrame::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
{
    OSL_FAIL("not implemented");
}

beans::PropertyState SwXFrame::getPropertyState( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aPropertyNames { rPropertyName };
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aPropertyNames);
    return aStates.getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXFrame::getPropertyStates(
    const uno::Sequence< OUString >& aPropertyNames )
{
    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyState > aStates(aPropertyNames.getLength());
    beans::PropertyState* pStates = aStates.getArray();
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        const OUString* pNames = aPropertyNames.getConstArray();
        const SwAttrSet& rFormatSet = pFormat->GetAttrSet();
        for(int i = 0; i < aPropertyNames.getLength(); i++)
        {
            const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(pNames[i]);
            if (!pEntry)
                throw beans::UnknownPropertyException("Unknown property: " + pNames[i], static_cast < cppu::OWeakObject * > ( this ) );

            if(pEntry->nWID == FN_UNO_ANCHOR_TYPES||
                pEntry->nWID == FN_PARAM_LINK_DISPLAY_NAME||
                FN_UNO_FRAME_STYLE_NAME == pEntry->nWID||
                FN_UNO_GRAPHIC == pEntry->nWID||
                FN_UNO_GRAPHIC_URL == pEntry->nWID||
                FN_UNO_GRAPHIC_FILTER == pEntry->nWID||
                FN_UNO_ACTUAL_SIZE == pEntry->nWID||
                FN_UNO_ALTERNATIVE_TEXT == pEntry->nWID)
            {
                pStates[i] = beans::PropertyState_DIRECT_VALUE;
            }
            else if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
            {
                if(SfxItemState::SET == rFormatSet.GetItemState(XATTR_FILLBMP_STRETCH, false)
                    || SfxItemState::SET == rFormatSet.GetItemState(XATTR_FILLBMP_TILE, false))
                {
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                }
                else
                {
                    pStates[i] = beans::PropertyState_AMBIGUOUS_VALUE;
                }
            }
            // for FlyFrames we need to mark the used properties from type RES_BACKGROUND
            // as beans::PropertyState_DIRECT_VALUE to let users of this property call
            // getPropertyValue where the member properties will be mapped from the
            // fill attributes to the according SvxBrushItem entries
            else if (RES_BACKGROUND == pEntry->nWID)
            {
                if (SWUnoHelper::needToMapFillItemsToSvxBrushItemTypes(rFormatSet, pEntry->nMemberId))
                    pStates[i] = beans::PropertyState_DIRECT_VALUE;
                else
                    pStates[i] = beans::PropertyState_DEFAULT_VALUE;
            }
            else
            {
                if ((eType == FLYCNTTYPE_GRF) && isGRFATR(pEntry->nWID))
                {
                    const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
                    if(pIdx)
                    {
                        SwNodeIndex aIdx(*pIdx, 1);
                        SwNoTextNode* pNoText = aIdx.GetNode().GetNoTextNode();
                        const SfxItemSet& aSet(pNoText->GetSwAttrSet());
                        aSet.GetItemState(pEntry->nWID);
                        if(SfxItemState::SET == aSet.GetItemState( pEntry->nWID, false ))
                            pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                }
                else
                {
                    if(SfxItemState::SET == rFormatSet.GetItemState( pEntry->nWID, false ))
                        pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    else
                        pStates[i] = beans::PropertyState_DEFAULT_VALUE;
                }
            }
        }
    }
    else if(IsDescriptor())
    {
        for(int i = 0; i < aPropertyNames.getLength(); i++)
            pStates[i] = beans::PropertyState_DIRECT_VALUE;
    }
    else
        throw uno::RuntimeException();
    return aStates;
}

void SwXFrame::setPropertyToDefault( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if (!pEntry)
            throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw uno::RuntimeException("setPropertyToDefault: property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        bool bNextFrame;
        if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
        {
            SwDoc* pDoc = pFormat->GetDoc();
            SfxItemSet aSet(pDoc->GetAttrPool(), svl::Items<XATTR_FILL_FIRST, XATTR_FILL_LAST>{});
            aSet.SetParent(&pFormat->GetAttrSet());

            aSet.ClearItem(XATTR_FILLBMP_STRETCH);
            aSet.ClearItem(XATTR_FILLBMP_TILE);

            pFormat->SetFormatAttr(aSet);
        }
        else if( pEntry->nWID &&
            pEntry->nWID != FN_UNO_ANCHOR_TYPES &&
            pEntry->nWID != FN_PARAM_LINK_DISPLAY_NAME)
        {
            if ( (eType == FLYCNTTYPE_GRF) && isGRFATR(pEntry->nWID) )
            {
                const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
                if(pIdx)
                {
                    SwNodeIndex aIdx(*pIdx, 1);
                    SwNoTextNode* pNoText = aIdx.GetNode().GetNoTextNode();
                    {
                        SfxItemSet aSet(pNoText->GetSwAttrSet());
                        aSet.ClearItem(pEntry->nWID);
                        pNoText->SetAttr(aSet);
                    }
                }
            }
            // #i73249#
            else if( FN_UNO_TITLE == pEntry->nWID )
            {
                SwFlyFrameFormat& rFlyFormat = dynamic_cast<SwFlyFrameFormat&>(*pFormat);
                // assure that <SdrObject> instance exists.
                GetOrCreateSdrObject(rFlyFormat);
                rFlyFormat.GetDoc()->SetFlyFrameTitle(rFlyFormat, OUString());
            }
            // New attribute Description
            else if( FN_UNO_DESCRIPTION == pEntry->nWID )
            {
                SwFlyFrameFormat& rFlyFormat = dynamic_cast<SwFlyFrameFormat&>(*pFormat);
                // assure that <SdrObject> instance exists.
                GetOrCreateSdrObject(rFlyFormat);
                rFlyFormat.GetDoc()->SetFlyFrameDescription(rFlyFormat, OUString());
            }
            else
            {
                SwDoc* pDoc = pFormat->GetDoc();
                SfxItemSet aSet( pDoc->GetAttrPool(),
                    svl::Items<RES_FRMATR_BEGIN, RES_FRMATR_END - 1>{} );
                aSet.SetParent(&pFormat->GetAttrSet());
                aSet.ClearItem(pEntry->nWID);
                if(rPropertyName != UNO_NAME_ANCHOR_TYPE)
                    pFormat->SetFormatAttr(aSet);
            }
        }
        else
        {
            bNextFrame = rPropertyName == UNO_NAME_CHAIN_NEXT_NAME;
            if( bNextFrame || rPropertyName == UNO_NAME_CHAIN_PREV_NAME )
            {
                SwDoc* pDoc = pFormat->GetDoc();
                if(bNextFrame)
                    pDoc->Unchain(*pFormat);
                else
                {
                    const SwFormatChain& aChain( pFormat->GetChain() );
                    SwFrameFormat *pPrev = aChain.GetPrev();
                    if(pPrev)
                        pDoc->Unchain(*pPrev);
                }
            }
        }
    }
    else if(!IsDescriptor())
        throw uno::RuntimeException();

}

uno::Any SwXFrame::getPropertyDefault( const OUString& rPropertyName )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(!pEntry)
            throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        if ( pEntry->nWID < RES_FRMATR_END )
        {
            const SfxPoolItem& rDefItem =
                pFormat->GetDoc()->GetAttrPool().GetDefaultItem(pEntry->nWID);
            rDefItem.QueryValue(aRet, pEntry->nMemberId);
        }

    }
    else if(!IsDescriptor())
        throw uno::RuntimeException();
    return aRet;
}

void SAL_CALL SwXFrame::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXFrame::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

void SwXFrame::Modify(const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if (GetRegisteredIn())
    {
        return; // core object still alive
    }
    mxStyleData.clear();
    mxStyleFamily.clear();
    m_pDoc = nullptr;
    uno::Reference<uno::XInterface> const xThis(m_pImpl->m_wThis);
    if (!xThis.is())
    {   // fdo#72695: if UNO object is already dead, don't revive it with event
        return;
    }
    lang::EventObject const ev(xThis);
    m_pImpl->m_EventListeners.disposeAndClear(ev);
}

void SwXFrame::dispose()
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if ( pFormat )
    {
        SdrObject* pObj = pFormat->FindSdrObject();
        // OD 11.09.2003 #112039# - add condition to perform delete of
        // format/anchor sign, not only if the object is inserted, but also
        // if a contact object is registered, which isn't in the destruction.
        if ( pObj &&
             ( pObj->IsInserted() ||
               ( pObj->GetUserCall() &&
                 !static_cast<SwContact*>(pObj->GetUserCall())->IsInDTOR() ) ) )
        {
            if (pFormat->GetAnchor().GetAnchorId() == RndStdIds::FLY_AS_CHAR)
            {
                const SwPosition &rPos = *(pFormat->GetAnchor().GetContentAnchor());
                SwTextNode *pTextNode = rPos.nNode.GetNode().GetTextNode();
                const sal_Int32 nIdx = rPos.nContent.GetIndex();
                pTextNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
            }
            else
                pFormat->GetDoc()->getIDocumentLayoutAccess().DelLayoutFormat(pFormat);
        }
    }

}

uno::Reference< text::XTextRange >  SwXFrame::getAnchor()
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  aRef;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(!pFormat)
        throw uno::RuntimeException();

    const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
    // return an anchor for non-page bound frames
    // and for page bound frames that have a page no == NULL and a content position
    if ((rAnchor.GetAnchorId() != RndStdIds::FLY_AT_PAGE) ||
        (rAnchor.GetContentAnchor() && !rAnchor.GetPageNum()))
    {
        const SwPosition &rPos = *(rAnchor.GetContentAnchor());
        aRef = SwXTextRange::CreateXTextRange(*pFormat->GetDoc(), rPos, nullptr);
    }

    return aRef;
}

void SwXFrame::ResetDescriptor()
{
    bIsDescriptor = false;
    mxStyleData.clear();
    mxStyleFamily.clear();
    m_pProps.reset();
}

void SwXFrame::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
{
    SolarMutexGuard aGuard;
    if(!IsDescriptor())
        throw uno::RuntimeException();
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = nullptr;
    OTextCursorHelper* pCursor = nullptr;
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
    }

    SwDoc* pDoc = pRange ? &pRange->GetDoc() : pCursor ? pCursor->GetDoc() : nullptr;
    if(!pDoc)
        throw lang::IllegalArgumentException();

    SwUnoInternalPaM aIntPam(*pDoc);
    // this now needs to return TRUE
    ::sw::XTextRangeToSwPaM(aIntPam, xTextRange);

    SwNode& rNode = pDoc->GetNodes().GetEndOfContent();
    SwPaM aPam(rNode);
    aPam.Move( fnMoveBackward, GoInDoc );
    static sal_uInt16 const aFrameAttrRange[] =
    {
        RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
        RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,

        // FillAttribute support
        XATTR_FILL_FIRST, XATTR_FILL_LAST,

        SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
        0
    };
    static sal_uInt16 const aGrAttrRange[] =
    {
        RES_GRFATR_BEGIN,       RES_GRFATR_END-1,
        0
    };
    SfxItemSet aGrSet(pDoc->GetAttrPool(), aGrAttrRange );

    SfxItemSet aFrameSet(pDoc->GetAttrPool(), aFrameAttrRange );

    // set correct parent to get the XFILL_NONE FillStyle as needed
    aFrameSet.SetParent(&pDoc->GetDfltFrameFormat()->GetAttrSet());

    // no the related items need to be added to the set
    bool bSizeFound;
    if (!m_pProps->AnyToItemSet(pDoc, aFrameSet, aGrSet, bSizeFound))
        throw lang::IllegalArgumentException();
    // a TextRange is handled separately
    *aPam.GetPoint() = *aIntPam.GetPoint();
    if(aIntPam.HasMark())
    {
        aPam.SetMark();
        *aPam.GetMark() = *aIntPam.GetMark();
    }

    const SfxPoolItem* pItem;
    RndStdIds eAnchorId = RndStdIds::FLY_AT_PARA;
    if(SfxItemState::SET == aFrameSet.GetItemState(RES_ANCHOR, false, &pItem) )
    {
        eAnchorId = static_cast<const SwFormatAnchor*>(pItem)->GetAnchorId();
        if( RndStdIds::FLY_AT_FLY == eAnchorId &&
            !aPam.GetNode().FindFlyStartNode())
        {
            // framebound only where a frame exists
            SwFormatAnchor aAnchor(RndStdIds::FLY_AT_PARA);
            aFrameSet.Put(aAnchor);
        }
        else if ((RndStdIds::FLY_AT_PAGE == eAnchorId) &&
                 0 == static_cast<const SwFormatAnchor*>(pItem)->GetPageNum() )
        {
            SwFormatAnchor aAnchor( *static_cast<const SwFormatAnchor*>(pItem) );
            aAnchor.SetAnchor( aPam.GetPoint() );
            aFrameSet.Put(aAnchor);
        }
    }

    const ::uno::Any* pStyle;
    SwFrameFormat *pParentFrameFormat = nullptr;
    if (m_pProps->GetProperty(FN_UNO_FRAME_STYLE_NAME, 0, pStyle))
        pParentFrameFormat = lcl_GetFrameFormat( *pStyle, pDoc );

    SwFlyFrameFormat* pFormat = nullptr;
    if( eType == FLYCNTTYPE_FRM)
    {
        UnoActionContext aCont(pDoc);
        if(m_pCopySource)
        {
            std::unique_ptr<SwFormatAnchor> pAnchorItem;
            // the frame is inserted bound to page
            // to prevent conflicts if the to-be-anchored position is part of the to-be-copied text
            if (eAnchorId != RndStdIds::FLY_AT_PAGE)
            {
                pAnchorItem.reset(static_cast<SwFormatAnchor*>(aFrameSet.Get(RES_ANCHOR).Clone()));
                aFrameSet.Put( SwFormatAnchor( RndStdIds::FLY_AT_PAGE, 1 ));
            }

            aPam.DeleteMark(); // mark position node will be deleted!
            aIntPam.DeleteMark(); // mark position node will be deleted!
            pFormat = pDoc->MakeFlyAndMove( *m_pCopySource, aFrameSet,
                           nullptr,
                           pParentFrameFormat );
            if(pAnchorItem && pFormat)
            {
                pFormat->DelFrames();
                pAnchorItem->SetAnchor( m_pCopySource->Start() );
                SfxItemSet aAnchorSet( pDoc->GetAttrPool(), svl::Items<RES_ANCHOR, RES_ANCHOR>{} );
                aAnchorSet.Put( *pAnchorItem );
                pDoc->SetFlyFrameAttr( *pFormat, aAnchorSet );
            }
            m_pCopySource.reset();
        }
        else
        {
            pFormat = pDoc->MakeFlySection( RndStdIds::FLY_AT_PARA, aPam.GetPoint(),
                                     &aFrameSet, pParentFrameFormat );
        }
        if(pFormat)
        {
            pFormat->Add(this);
            if(!m_sName.isEmpty())
                pDoc->SetFlyName(*pFormat, m_sName);
        }
        // wake up the SwXTextFrame
        static_cast<SwXTextFrame*>(this)->SetDoc( bIsDescriptor ? m_pDoc : GetFrameFormat()->GetDoc() );
    }
    else if( eType == FLYCNTTYPE_GRF)
    {
        UnoActionContext aActionContext(pDoc);
        Graphic aGraphic;

        // Read graphic URL from the descriptor, if it has any.
        const ::uno::Any* pGraphicURL;
        if (m_pProps->GetProperty(FN_UNO_GRAPHIC_URL, 0, pGraphicURL))
        {
            OUString sGraphicURL;
            uno::Reference<awt::XBitmap> xBitmap;
            if (((*pGraphicURL) >>= sGraphicURL) && !sGraphicURL.isEmpty())
                aGraphic = vcl::graphic::loadFromURL(sGraphicURL);
            else if ((*pGraphicURL) >>= xBitmap)
            {
                uno::Reference<graphic::XGraphic> xGraphic(xBitmap, uno::UNO_QUERY);
                if (xGraphic.is())
                    aGraphic = xGraphic;
            }
        }

        const ::uno::Any* pGraphicAny;
        const bool bHasGraphic = m_pProps->GetProperty(FN_UNO_GRAPHIC, 0, pGraphicAny);
        if (bHasGraphic)
        {
            uno::Reference<graphic::XGraphic> xGraphic;
            (*pGraphicAny) >>= xGraphic;
            aGraphic = Graphic(xGraphic);
        }

        OUString sFilterName;
        const uno::Any* pFilterAny;
        if (m_pProps->GetProperty(FN_UNO_GRAPHIC_FILTER, 0, pFilterAny))
        {
            (*pFilterAny) >>= sFilterName;
        }

        pFormat = pDoc->getIDocumentContentOperations().InsertGraphic(
                    aPam, OUString(), sFilterName, &aGraphic, &aFrameSet, &aGrSet, pParentFrameFormat);
        if (pFormat)
        {
            SwGrfNode *pGrfNd = pDoc->GetNodes()[ pFormat->GetContent().GetContentIdx()
                                        ->GetIndex()+1 ]->GetGrfNode();
            if (pGrfNd)
                pGrfNd->SetChgTwipSize( !bSizeFound );
            pFormat->Add(this);
            if(!m_sName.isEmpty())
                pDoc->SetFlyName(*pFormat, m_sName);

        }
        const ::uno::Any* pSurroundContour;
        if (m_pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUR, pSurroundContour))
            setPropertyValue(UNO_NAME_SURROUND_CONTOUR, *pSurroundContour);
        const ::uno::Any* pContourOutside;
        if (m_pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUROUTSIDE, pContourOutside))
            setPropertyValue(UNO_NAME_CONTOUR_OUTSIDE, *pContourOutside);
        const ::uno::Any* pContourPoly;
        if (m_pProps->GetProperty(FN_PARAM_CONTOUR_PP, 0, pContourPoly))
            setPropertyValue(UNO_NAME_CONTOUR_POLY_POLYGON, *pContourPoly);
        const ::uno::Any* pPixelContour;
        if (m_pProps->GetProperty(FN_UNO_IS_PIXEL_CONTOUR, 0, pPixelContour))
            setPropertyValue(UNO_NAME_IS_PIXEL_CONTOUR, *pPixelContour);
        const ::uno::Any* pAutoContour;
        if (m_pProps->GetProperty(FN_UNO_IS_AUTOMATIC_CONTOUR, 0, pAutoContour))
            setPropertyValue(UNO_NAME_IS_AUTOMATIC_CONTOUR, *pAutoContour);
    }
    else
    {
        const ::uno::Any* pCLSID = nullptr;
        const ::uno::Any* pStreamName = nullptr;
        const ::uno::Any* pEmbeddedObject = nullptr;
        if (!m_pProps->GetProperty(FN_UNO_CLSID, 0, pCLSID)
            && !m_pProps->GetProperty(FN_UNO_STREAM_NAME, 0, pStreamName)
            && !m_pProps->GetProperty(FN_EMBEDDED_OBJECT, 0, pEmbeddedObject))
        {
            throw uno::RuntimeException();
        }
        if(pCLSID)
        {
            OUString aCLSID;
            SvGlobalName aClassName;
            uno::Reference < embed::XEmbeddedObject > xIPObj;
            std::unique_ptr < comphelper::EmbeddedObjectContainer > pCnt;
            if( (*pCLSID) >>= aCLSID )
            {
                if( !aClassName.MakeId( aCLSID ) )
                {
                    lang::IllegalArgumentException aExcept;
                    aExcept.Message = "CLSID invalid";
                    throw aExcept;
                }

                pCnt.reset( new comphelper::EmbeddedObjectContainer );
                OUString aName;

                OUString sDocumentBaseURL = pDoc->GetPersist()->getDocumentBaseURL();
                xIPObj = pCnt->CreateEmbeddedObject(aClassName.GetByteSequence(), aName,
                                                    &sDocumentBaseURL);
            }
            if ( xIPObj.is() )
            {
                UnoActionContext aAction(pDoc);
                pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT, nullptr);

                // tdf#99631 set imported VisibleArea settings of embedded XLSX OLE objects
                if ( m_nDrawAspect == embed::Aspects::MSOLE_CONTENT
                        && m_nVisibleAreaWidth && m_nVisibleAreaHeight )
                {
                    sal_Int64 nAspect = m_nDrawAspect;
                    MapUnit aUnit = VCLUnoHelper::UnoEmbed2VCLMapUnit( xIPObj->getMapUnit( nAspect ) );
                    Size aSize( OutputDevice::LogicToLogic(Size( m_nVisibleAreaWidth, m_nVisibleAreaHeight),
                        MapMode(MapUnit::MapTwip), MapMode(aUnit)));
                    awt::Size aSz;
                    aSz.Width = aSize.Width();
                    aSz.Height = aSize.Height();
                    xIPObj->setVisualAreaSize(m_nDrawAspect, aSz);
                }

                if(!bSizeFound)
                {
                    //TODO/LATER: how do I transport it to the OLENode?
                    sal_Int64 nAspect = m_nDrawAspect;

                    // TODO/LEAN: VisualArea still needs running state
                    svt::EmbeddedObjectRef::TryRunningState( xIPObj );

                    // set parent to get correct VisArea(in case of object needing parent printer)
                    uno::Reference < container::XChild > xChild( xIPObj, uno::UNO_QUERY );
                    if ( xChild.is() )
                        xChild->setParent( pDoc->GetDocShell()->GetModel() );

                    //The Size should be suggested by the OLE server if not manually set
                    MapUnit aRefMap = VCLUnoHelper::UnoEmbed2VCLMapUnit( xIPObj->getMapUnit( nAspect ) );
                    awt::Size aSize;
                    try
                    {
                        aSize = xIPObj->getVisualAreaSize( nAspect );
                    }
                    catch ( embed::NoVisualAreaSizeException& )
                    {
                        // the default size will be set later
                    }

                    Size aSz( aSize.Width, aSize.Height );
                    if ( !aSz.Width() || !aSz.Height() )
                    {
                        aSz.setWidth(5000);
                        aSz.setHeight(5000);
                        aSz = OutputDevice::LogicToLogic(aSz,
                                MapMode(MapUnit::Map100thMM), MapMode(aRefMap));
                    }
                    MapMode aMyMap( MapUnit::MapTwip );
                    aSz = OutputDevice::LogicToLogic(aSz, MapMode(aRefMap), aMyMap);
                    SwFormatFrameSize aFrameSz;
                    aFrameSz.SetSize(aSz);
                    aFrameSet.Put(aFrameSz);
                }
                SwFlyFrameFormat* pFormat2 = nullptr;

                ::svt::EmbeddedObjectRef xObjRef( xIPObj, m_nDrawAspect);
                pFormat2 = pDoc->getIDocumentContentOperations().InsertEmbObject(
                        aPam, xObjRef, &aFrameSet );

                // store main document name to show in the title bar
                uno::Reference< frame::XTitle > xModelTitle( pDoc->GetDocShell()->GetModel(), css::uno::UNO_QUERY );
                if( xModelTitle.is() )
                    xIPObj->setContainerName( xModelTitle->getTitle() );

                assert(pFormat2 && "Doc->Insert(notxt) failed.");

                pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT, nullptr);
                pFormat2->Add(this);
                if(!m_sName.isEmpty())
                    pDoc->SetFlyName(*pFormat2, m_sName);
            }
        }
        else if( pStreamName )
        {
            OUString sStreamName;
            (*pStreamName) >>= sStreamName;
            pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT, nullptr);

            SwFlyFrameFormat* pFrameFormat = pDoc->getIDocumentContentOperations().InsertOLE(
                aPam, sStreamName, m_nDrawAspect, &aFrameSet, nullptr);

            // store main document name to show in the title bar
            SwOLENode* pNd = nullptr;
            const SwNodeIndex* pIdx = pFrameFormat->GetContent().GetContentIdx();
            if( pIdx )
            {
                SwNodeIndex aIdx( *pIdx, 1 );
                SwNoTextNode* pNoText = aIdx.GetNode().GetNoTextNode();
                pNd = pNoText->GetOLENode();
            }
            if( pNd )
            {
                uno::Reference < embed::XEmbeddedObject > xObj = pNd->GetOLEObj().GetOleRef();
                if( xObj.is() )
                {
                    uno::Reference< frame::XTitle > xModelTitle( pDoc->GetDocShell()->GetModel(), css::uno::UNO_QUERY );
                    if( xModelTitle.is() )
                        xObj->setContainerName( xModelTitle->getTitle() );
                }
            }

            pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT, nullptr);
            pFrameFormat->Add(this);
            if(!m_sName.isEmpty())
                pDoc->SetFlyName(*pFrameFormat, m_sName);
        }
        else if (pEmbeddedObject)
        {
            uno::Reference< embed::XEmbeddedObject > obj;
            (*pEmbeddedObject) >>= obj;
            svt::EmbeddedObjectRef xObj;
            xObj.Assign( obj, embed::Aspects::MSOLE_CONTENT );

            pDoc->GetIDocumentUndoRedo().StartUndo(SwUndoId::INSERT, nullptr);

            // Not sure if these setParent() and InsertEmbeddedObject() calls are really
            // needed, it seems to work without, but logic from code elsewhere suggests
            // they should be done.
            SfxObjectShell& rPers = *pDoc->GetPersist();
            uno::Reference < container::XChild > xChild( obj, uno::UNO_QUERY );
            if ( xChild.is() )
                xChild->setParent( rPers.GetModel() );
            OUString rName;
            rPers.GetEmbeddedObjectContainer().InsertEmbeddedObject( obj, rName );

            SwFlyFrameFormat* pFrameFormat
                = pDoc->getIDocumentContentOperations().InsertEmbObject(aPam, xObj, &aFrameSet);
            pDoc->GetIDocumentUndoRedo().EndUndo(SwUndoId::INSERT, nullptr);
            pFrameFormat->Add(this);
            if(!m_sName.isEmpty())
                pDoc->SetFlyName(*pFrameFormat, m_sName);
        }
    }
    if( pFormat && pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
        GetOrCreateSdrObject(*pFormat);
    const ::uno::Any* pOrder;
    if (m_pProps->GetProperty(FN_UNO_Z_ORDER, 0, pOrder))
        setPropertyValue(UNO_NAME_Z_ORDER, *pOrder);
    const ::uno::Any* pReplacement;
    if (m_pProps->GetProperty(FN_UNO_REPLACEMENT_GRAPHIC, 0, pReplacement))
        setPropertyValue(UNO_NAME_GRAPHIC, *pReplacement);
    // new attribute Title
    const ::uno::Any* pTitle;
    if (m_pProps->GetProperty(FN_UNO_TITLE, 0, pTitle))
    {
        setPropertyValue(UNO_NAME_TITLE, *pTitle);
    }
    // new attribute Description
    const ::uno::Any* pDescription;
    if (m_pProps->GetProperty(FN_UNO_DESCRIPTION, 0, pDescription))
    {
        setPropertyValue(UNO_NAME_DESCRIPTION, *pDescription);
    }

    // For grabbag
    const uno::Any* pFrameIntropgrabbagItem;
    if (m_pProps->GetProperty(RES_FRMATR_GRABBAG, 0, pFrameIntropgrabbagItem))
    {
        setPropertyValue(UNO_NAME_FRAME_INTEROP_GRAB_BAG, *pFrameIntropgrabbagItem);
    }

    // reset the flag and delete Descriptor pointer
    ResetDescriptor();
}

void SwXFrame::attach(const uno::Reference< text::XTextRange > & xTextRange)
{
    SwFrameFormat* pFormat;
    if(IsDescriptor())
        attachToRange(xTextRange);
    else if(nullptr != (pFormat = GetFrameFormat()))
    {
        SwDoc* pDoc = pFormat->GetDoc();
        SwUnoInternalPaM aIntPam(*pDoc);
        if (!::sw::XTextRangeToSwPaM(aIntPam, xTextRange))
            throw lang::IllegalArgumentException();

        SfxItemSet aSet( pDoc->GetAttrPool(), svl::Items<RES_ANCHOR, RES_ANCHOR>{} );
        aSet.SetParent(&pFormat->GetAttrSet());
        SwFormatAnchor aAnchor = aSet.Get(RES_ANCHOR);
        aAnchor.SetAnchor( aIntPam.Start() );
        aSet.Put(aAnchor);
        pDoc->SetFlyFrameAttr( *pFormat, aSet );

    }
}

awt::Point SwXFrame::getPosition()
{
    SolarMutexGuard aGuard;
    uno::RuntimeException aRuntime;
    aRuntime.Message = "position cannot be determined with this method";
    throw aRuntime;
}

void SwXFrame::setPosition(const awt::Point& /*aPosition*/)
{
    SolarMutexGuard aGuard;
    uno::RuntimeException aRuntime;
    aRuntime.Message = "position cannot be changed with this method";
    throw aRuntime;
}

awt::Size SwXFrame::getSize()
{
    const ::uno::Any aVal = getPropertyValue("Size");
    awt::Size const * pRet =  o3tl::doAccess<awt::Size>(aVal);
    return *pRet;
}

void SwXFrame::setSize(const awt::Size& aSize)
{
    const ::uno::Any aVal(&aSize, ::cppu::UnoType<awt::Size>::get());
    setPropertyValue("Size", aVal);
}

OUString SwXFrame::getShapeType()
{
    return OUString("FrameShape");
}

SwXTextFrame::SwXTextFrame( SwDoc *_pDoc ) :
    SwXText(nullptr, CursorType::Frame),
    SwXFrame(FLYCNTTYPE_FRM, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_FRAME), _pDoc )
{
}

SwXTextFrame::SwXTextFrame(SwFrameFormat& rFormat) :
    SwXText(rFormat.GetDoc(), CursorType::Frame),
    SwXFrame(rFormat, FLYCNTTYPE_FRM, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_FRAME))
{

}

SwXTextFrame::~SwXTextFrame()
{
}

uno::Reference<text::XTextFrame>
SwXTextFrame::CreateXTextFrame(SwDoc & rDoc, SwFrameFormat *const pFrameFormat)
{
    return CreateXFrame<text::XTextFrame, SwXTextFrame>(rDoc, pFrameFormat);
}

void SAL_CALL SwXTextFrame::acquire(  )throw()
{
    SwXFrame::acquire();
}

void SAL_CALL SwXTextFrame::release(  )throw()
{
    SwXFrame::release();
}

::uno::Any SAL_CALL SwXTextFrame::queryInterface( const uno::Type& aType )
{
    ::uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == cppu::UnoType<void>::get())
        aRet = SwXText::queryInterface(aType);
    if(aRet.getValueType() == cppu::UnoType<void>::get())
        aRet = SwXTextFrameBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL SwXTextFrame::getTypes(  )
{
    return comphelper::concatSequences(
        SwXTextFrameBaseClass::getTypes(),
        SwXFrame::getTypes(),
        SwXText::getTypes()
    );
}

uno::Sequence< sal_Int8 > SAL_CALL SwXTextFrame::getImplementationId(  )
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Reference< text::XText >  SwXTextFrame::getText()
{
    return this;
}

const SwStartNode *SwXTextFrame::GetStartNode() const
{
    const SwStartNode *pSttNd = nullptr;

    const SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        const SwFormatContent& rFlyContent = pFormat->GetContent();
        if( rFlyContent.GetContentIdx() )
            pSttNd = rFlyContent.GetContentIdx()->GetNode().GetStartNode();
    }

    return pSttNd;
}

uno::Reference< text::XTextCursor >
SwXTextFrame::CreateCursor()
{
    return createTextCursor();
}

uno::Reference< text::XTextCursor >  SwXTextFrame::createTextCursor()
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextCursor >  aRef;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(!pFormat)
        throw uno::RuntimeException();

    //save current start node to be able to check if there is content after the table -
    //otherwise the cursor would be in the body text!
    const SwNode& rNode = pFormat->GetContent().GetContentIdx()->GetNode();
    const SwStartNode* pOwnStartNode = rNode.FindSttNodeByType(SwFlyStartNode);

    SwPaM aPam(rNode);
    aPam.Move(fnMoveForward, GoInNode);
    SwTableNode* pTableNode = aPam.GetNode().FindTableNode();
    SwContentNode* pCont = nullptr;
    while( pTableNode )
    {
        aPam.GetPoint()->nNode = *pTableNode->EndOfSectionNode();
        pCont = GetDoc()->GetNodes().GoNext(&aPam.GetPoint()->nNode);
        pTableNode = pCont->FindTableNode();
    }
    if(pCont)
        aPam.GetPoint()->nContent.Assign(pCont, 0);

    const SwStartNode* pNewStartNode =
        aPam.GetNode().FindSttNodeByType(SwFlyStartNode);
    if(!pNewStartNode || pNewStartNode != pOwnStartNode)
    {
        uno::RuntimeException aExcept;
        aExcept.Message = "no text available";
        throw aExcept;
    }

    SwXTextCursor *const pXCursor = new SwXTextCursor(
             *pFormat->GetDoc(), this, CursorType::Frame, *aPam.GetPoint());
    aRef =  static_cast<text::XWordCursor*>(pXCursor);

    return aRef;
}

uno::Reference< text::XTextCursor >  SwXTextFrame::createTextCursorByRange(const uno::Reference< text::XTextRange > & aTextPosition)
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if (!pFormat)
        throw uno::RuntimeException();
    SwUnoInternalPaM aPam(*GetDoc());
    if (!::sw::XTextRangeToSwPaM(aPam, aTextPosition))
        throw uno::RuntimeException();

    uno::Reference<text::XTextCursor>  aRef;
    SwNode& rNode = pFormat->GetContent().GetContentIdx()->GetNode();
    if(aPam.GetNode().FindFlyStartNode() == rNode.FindFlyStartNode())
    {
        aRef = static_cast<text::XWordCursor*>(
                new SwXTextCursor(*pFormat->GetDoc(), this, CursorType::Frame,
                    *aPam.GetPoint(), aPam.GetMark()));
    }

    return aRef;
}

uno::Reference< container::XEnumeration >  SwXTextFrame::createEnumeration()
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(!pFormat)
        return nullptr;
    SwPosition aPos(pFormat->GetContent().GetContentIdx()->GetNode());
    auto pUnoCursor(GetDoc()->CreateUnoCursor(aPos));
    pUnoCursor->Move(fnMoveForward, GoInNode);
    return SwXParagraphEnumeration::Create(this, pUnoCursor, CursorType::Frame);
}

uno::Type  SwXTextFrame::getElementType()
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SwXTextFrame::hasElements()
{
    return true;
}

void SwXTextFrame::attach(const uno::Reference< text::XTextRange > & xTextRange)
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< text::XTextRange >  SwXTextFrame::getAnchor()
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextFrame::dispose()
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextFrame::addEventListener(const uno::Reference< lang::XEventListener > & aListener)
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextFrame::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
{
    SwXFrame::removeEventListener(aListener);
}

OUString SwXTextFrame::getImplementationName()
{
    return OUString("SwXTextFrame");
}

sal_Bool SwXTextFrame::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextFrame::getSupportedServiceNames()
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 2);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 2] = "com.sun.star.text.TextFrame";
    pArray[aRet.getLength() - 1] = "com.sun.star.text.Text";
    return aRet;
}

uno::Reference<container::XNameReplace > SAL_CALL SwXTextFrame::getEvents()
{
    return new SwFrameEventDescriptor( *this );
}

sal_Int64 SAL_CALL SwXTextFrame::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    sal_Int64 nRet = SwXFrame::getSomething( rId );
    if( !nRet )
        nRet = SwXText::getSomething( rId );

    return nRet;
}

::uno::Any SwXTextFrame::getPropertyValue(const OUString& rPropertyName)
{
    SolarMutexGuard aGuard;
    ::uno::Any aRet;
    if(rPropertyName == UNO_NAME_START_REDLINE||
            rPropertyName == UNO_NAME_END_REDLINE)
    {
        //redline can only be returned if it's a living object
        if(!IsDescriptor())
            aRet = SwXText::getPropertyValue(rPropertyName);
    }
    else
        aRet = SwXFrame::getPropertyValue(rPropertyName);
    return aRet;
}

SwXTextGraphicObject::SwXTextGraphicObject( SwDoc *pDoc )
    : SwXTextGraphicObjectBaseClass(FLYCNTTYPE_GRF,
            aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_GRAPHIC), pDoc)
{
}

SwXTextGraphicObject::SwXTextGraphicObject(SwFrameFormat& rFormat)
    : SwXTextGraphicObjectBaseClass(rFormat, FLYCNTTYPE_GRF,
            aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_GRAPHIC))
{
}

SwXTextGraphicObject::~SwXTextGraphicObject()
{
}

uno::Reference<text::XTextContent>
SwXTextGraphicObject::CreateXTextGraphicObject(SwDoc & rDoc, SwFrameFormat *const pFrameFormat)
{
    return CreateXFrame<text::XTextContent, SwXTextGraphicObject>(rDoc, pFrameFormat);
}

OUString SwXTextGraphicObject::getImplementationName()
{
    return OUString("SwXTextGraphicObject");
}

sal_Bool SwXTextGraphicObject::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextGraphicObject::getSupportedServiceNames()
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 1);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 1] = "com.sun.star.text.TextGraphicObject";
    return aRet;
}

uno::Reference<container::XNameReplace> SAL_CALL
    SwXTextGraphicObject::getEvents()
{
    return new SwFrameEventDescriptor( *this );
}

SwXTextEmbeddedObject::SwXTextEmbeddedObject( SwDoc *pDoc )
    : SwXTextEmbeddedObjectBaseClass(FLYCNTTYPE_OLE,
            aSwMapProvider.GetPropertySet(PROPERTY_MAP_EMBEDDED_OBJECT), pDoc)
{
}

SwXTextEmbeddedObject::SwXTextEmbeddedObject(SwFrameFormat& rFormat)
    : SwXTextEmbeddedObjectBaseClass(rFormat, FLYCNTTYPE_OLE,
            aSwMapProvider.GetPropertySet(PROPERTY_MAP_EMBEDDED_OBJECT))
{
}

SwXTextEmbeddedObject::~SwXTextEmbeddedObject()
{
}

uno::Reference<text::XTextContent>
SwXTextEmbeddedObject::CreateXTextEmbeddedObject(SwDoc & rDoc, SwFrameFormat *const pFrameFormat)
{
    return CreateXFrame<text::XTextContent, SwXTextEmbeddedObject>(rDoc, pFrameFormat);
}

uno::Reference< lang::XComponent >  SwXTextEmbeddedObject::getEmbeddedObject()
{
    uno::Reference<embed::XEmbeddedObject> xObj(getExtendedControlOverEmbeddedObject());
    return xObj.is() ? uno::Reference<lang::XComponent>(xObj->getComponent(), uno::UNO_QUERY) : nullptr;
}

uno::Reference< embed::XEmbeddedObject > SAL_CALL SwXTextEmbeddedObject::getExtendedControlOverEmbeddedObject()
{
    uno::Reference< embed::XEmbeddedObject > xResult;
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(pFormat)
    {
        SwDoc* pDoc = pFormat->GetDoc();
        const SwFormatContent* pCnt = &pFormat->GetContent();
        OSL_ENSURE( pCnt->GetContentIdx() &&
                       pDoc->GetNodes()[ pCnt->GetContentIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "no OLE-Node?");

        SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetContentIdx()
                                        ->GetIndex() + 1 ]->GetOLENode();
        xResult = pOleNode->GetOLEObj().GetOleRef();
        if ( svt::EmbeddedObjectRef::TryRunningState( xResult ) )
        {
            // TODO/LATER: the listener registered after client creation should be able to handle scaling, after that the client is not necessary here
            if ( pDoc->GetDocShell() )
                pDoc->GetDocShell()->GetIPClient( svt::EmbeddedObjectRef( xResult, embed::Aspects::MSOLE_CONTENT ) );

            uno::Reference < lang::XComponent > xComp( xResult->getComponent(), uno::UNO_QUERY );
            uno::Reference< util::XModifyBroadcaster >  xBrdcst( xComp, uno::UNO_QUERY);
            uno::Reference< frame::XModel > xModel( xComp, uno::UNO_QUERY);
            if(xBrdcst.is() && xModel.is() && !m_xOLEListener.is())
            {
                m_xOLEListener = new SwXOLEListener(*pFormat, xModel);
                xBrdcst->addModifyListener( m_xOLEListener );
            }
        }
    }
    return xResult;
}

sal_Int64 SAL_CALL SwXTextEmbeddedObject::getAspect()
{
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(pFormat)
    {
        SwDoc* pDoc = pFormat->GetDoc();
        const SwFormatContent* pCnt = &pFormat->GetContent();
        OSL_ENSURE( pCnt->GetContentIdx() &&
                       pDoc->GetNodes()[ pCnt->GetContentIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "no OLE-Node?");

        return pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ]->GetOLENode()->GetAspect();
    }

    return embed::Aspects::MSOLE_CONTENT; // return the default value
}

void SAL_CALL SwXTextEmbeddedObject::setAspect( sal_Int64 nAspect )
{
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(pFormat)
    {
        SwDoc* pDoc = pFormat->GetDoc();
        const SwFormatContent* pCnt = &pFormat->GetContent();
        OSL_ENSURE( pCnt->GetContentIdx() &&
                       pDoc->GetNodes()[ pCnt->GetContentIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "no OLE-Node?");

        pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ]->GetOLENode()->SetAspect( nAspect );
    }
}

uno::Reference< graphic::XGraphic > SAL_CALL SwXTextEmbeddedObject::getReplacementGraphic()
{
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(pFormat)
    {
        SwDoc* pDoc = pFormat->GetDoc();
        const SwFormatContent* pCnt = &pFormat->GetContent();
        OSL_ENSURE( pCnt->GetContentIdx() &&
                       pDoc->GetNodes()[ pCnt->GetContentIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "no OLE-Node?");

        const Graphic* pGraphic = pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ]->GetOLENode()->GetGraphic();
        if ( pGraphic )
            return pGraphic->GetXGraphic();
    }

    return uno::Reference< graphic::XGraphic >();
}

OUString SwXTextEmbeddedObject::getImplementationName()
{
    return OUString("SwXTextEmbeddedObject");
}

sal_Bool SwXTextEmbeddedObject::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextEmbeddedObject::getSupportedServiceNames()
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 1);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 1] = "com.sun.star.text.TextEmbeddedObject";
    return aRet;
}

uno::Reference<container::XNameReplace> SAL_CALL
    SwXTextEmbeddedObject::getEvents()
{
    return new SwFrameEventDescriptor( *this );
}

namespace
{
    SwOLENode* lcl_GetOLENode(const SwFormat* pFormat)
    {
        if(!pFormat)
            return nullptr;
        const SwNodeIndex* pIdx(pFormat->GetContent().GetContentIdx());
        if(!pIdx)
            return nullptr;
        const SwNodeIndex aIdx(*pIdx, 1);
        return aIdx.GetNode().GetNoTextNode()->GetOLENode();
    }
}

SwXOLEListener::SwXOLEListener( SwFormat& rOLEFormat, uno::Reference< XModel > const & xOLE)
    : m_pOLEFormat(&rOLEFormat)
    , m_xOLEModel(xOLE)
{
    StartListening(m_pOLEFormat->GetNotifier());
}

SwXOLEListener::~SwXOLEListener()
{}

void SwXOLEListener::modified( const lang::EventObject& /*rEvent*/ )
{
    SolarMutexGuard aGuard;
    const auto pNd = lcl_GetOLENode(m_pOLEFormat);
    if(!pNd)
        throw uno::RuntimeException();
    const auto xIP = pNd->GetOLEObj().GetOleRef();
    if(xIP.is())
    {
        sal_Int32 nState = xIP->getCurrentState();
        if(nState == embed::EmbedStates::INPLACE_ACTIVE || nState == embed::EmbedStates::UI_ACTIVE)
            // if the OLE-Node is UI-Active do nothing
            return;
    }
    pNd->SetOLESizeInvalid(true);
    pNd->GetDoc()->SetOLEObjModified();
}

void SwXOLEListener::disposing( const lang::EventObject& rEvent )
{
    SolarMutexGuard aGuard;
    uno::Reference<util::XModifyListener> xListener( this );
    uno::Reference<frame::XModel> xModel(rEvent.Source, uno::UNO_QUERY);
    uno::Reference<util::XModifyBroadcaster> xBrdcst(xModel, uno::UNO_QUERY);
    if(!xBrdcst.is())
        return;
    try
    {
        xBrdcst->removeModifyListener(xListener);
    }
    catch(uno::Exception const &)
    {
        OSL_FAIL("OLE Listener couldn't be removed");
    }
}

void SwXOLEListener::Notify( const SfxHint& rHint )
{
    if(rHint.GetId() == SfxHintId::Dying)
    {
        m_xOLEModel = nullptr;
        m_pOLEFormat = nullptr;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
