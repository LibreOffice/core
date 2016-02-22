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
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <svx/svxids.hrc>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <svx/sdtaitm.hxx>
#include <svx/xflclit.hxx>
#include <editeng/memberids.hrc>
#include <swtypes.hxx>
#include <cmdid.h>
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

using namespace ::com::sun::star;

using ::com::sun::star::frame::XModel;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::style::XStyleFamiliesSupplier;

const sal_Char sPackageProtocol[] = "vnd.sun.star.Package:";
const sal_Char sGraphicObjectProtocol[] = "vnd.sun.star.GraphicObject:";

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
    //UUUU assert when the target SfxItemSet has no parent. It *should* have the pDfltFrameFormat
    // from SwDoc set as parent (or similar) to have the necessary XFILL_NONE in the ItemSet
    if(!rToSet.GetParent())
    {
        OSL_ENSURE(false, "OOps, target SfxItemSet *should* have a parent which contains XFILL_NONE as XFillStyleItem (!)");
    }

    bool bRet = true;
    // always add an anchor to the set
    SwFormatAnchor aAnchor ( static_cast < const SwFormatAnchor & > ( rFromSet.Get ( RES_ANCHOR ) ) );
    {
        const ::uno::Any* pAnchorPgNo;
        if(GetProperty(RES_ANCHOR, MID_ANCHOR_PAGENUM, pAnchorPgNo))
            bRet &= ((SfxPoolItem&)aAnchor).PutValue(*pAnchorPgNo, MID_ANCHOR_PAGENUM);
        const ::uno::Any* pAnchorType;
        if(GetProperty(RES_ANCHOR, MID_ANCHOR_ANCHORTYPE, pAnchorType))
            bRet &= ((SfxPoolItem&)aAnchor).PutValue(*pAnchorType, MID_ANCHOR_ANCHORTYPE);
    }

    rToSet.Put(aAnchor);

    //UUUU check for SvxBrushItem (RES_BACKGROUND) properties
    const ::uno::Any* pCol = nullptr; GetProperty(RES_BACKGROUND, MID_BACK_COLOR, pCol );
    const ::uno::Any* pRGBCol = nullptr; GetProperty(RES_BACKGROUND, MID_BACK_COLOR_R_G_B, pRGBCol );
    const ::uno::Any* pColTrans = nullptr; GetProperty(RES_BACKGROUND, MID_BACK_COLOR_TRANSPARENCY, pColTrans);
    const ::uno::Any* pTrans = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_TRANSPARENT, pTrans );
    const ::uno::Any* pGrLoc = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_POSITION, pGrLoc );
    const ::uno::Any* pGrURL = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_URL, pGrURL     );
    const ::uno::Any* pGrFilter = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_FILTER, pGrFilter     );
    const ::uno::Any* pGrTranparency = nullptr; GetProperty(RES_BACKGROUND, MID_GRAPHIC_TRANSPARENCY, pGrTranparency     );
    const bool bSvxBrushItemPropertiesUsed(
        pCol ||
        pTrans ||
        pGrURL ||
        pGrFilter ||
        pGrLoc ||
        pGrTranparency ||
        pColTrans ||
        pRGBCol);

    //UUUU check for FillStyle properties in the range XATTR_FILL_FIRST, XATTR_FILL_LAST
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
    const uno::Any* pXFillBitmapURLItem = nullptr; GetProperty(XATTR_FILLBITMAP, MID_GRAFURL, pXFillBitmapURLItem);

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
    // but there is a GraphicURL
    const bool bFillStyleUsed(pXFillStyleItem && pXFillStyleItem->hasValue() &&
        (pXFillStyleItem->get<drawing::FillStyle>() != drawing::FillStyle_SOLID
         || !pGrURL));
    SAL_INFO_IF(pXFillStyleItem && pXFillStyleItem->hasValue() && !bFillStyleUsed,
            "sw.uno", "FillBaseProperties: ignoring invalid FillStyle");
    const bool bXFillStyleItemUsed(
        bFillStyleUsed ||
        pXFillColorItem ||
        pXFillGradientItem || pXFillGradientNameItem ||
        pXFillHatchItem || pXFillHatchNameItem ||
        pXFillBitmapItem || pXFillBitmapNameItem || pXFillBitmapURLItem ||
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
        //UUUU create a temporary SvxBrushItem, fill the attributes to it and use it to set
        // the corresponding FillAttributes
        SvxBrushItem aBrush(RES_BACKGROUND);

        if(pCol)
        {
            bRet &= ((SfxPoolItem&)aBrush).PutValue(*pCol,MID_BACK_COLOR    );
        }

        if(pColTrans)
        {
            bRet &= ((SfxPoolItem&)aBrush).PutValue(*pColTrans, MID_BACK_COLOR_TRANSPARENCY);
        }

        if(pRGBCol)
        {
            bRet &= ((SfxPoolItem&)aBrush).PutValue(*pRGBCol, MID_BACK_COLOR_R_G_B);
        }

        if(pTrans)
        {
            // don't overwrite transparency with a non-transparence flag
            if(!pColTrans || Any2Bool( *pTrans ))
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pTrans, MID_GRAPHIC_TRANSPARENT);
        }

        if(pGrURL)
        {
            bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrURL, MID_GRAPHIC_URL);
        }

        if(pGrFilter)
        {
            bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrFilter, MID_GRAPHIC_FILTER);
        }

        if(pGrLoc)
        {
            bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrLoc, MID_GRAPHIC_POSITION);
        }

        if(pGrTranparency)
        {
            bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrTranparency, MID_GRAPHIC_TRANSPARENCY);
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
                const XGradient aNullGrad(RGB_Color(COL_BLACK), RGB_Color(COL_WHITE));
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
                XFillHatchItem aXFillHatchItem(rToSet.GetPool(), aNullHatch);

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

        if(pXFillBitmapItem || pXFillBitmapNameItem || pXFillBitmapURLItem)
        {
            if(pXFillBitmapItem)
            {
                const Graphic aNullGraphic;
                XFillBitmapItem aXFillBitmapItem(rToSet.GetPool(), aNullGraphic);

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

            if(pXFillBitmapURLItem)
            {
                const Graphic aNullGraphic;
                XFillBitmapItem aXFillBitmapItem(rToSet.GetPool(), aNullGraphic);

                aXFillBitmapItem.PutValue(*pXFillBitmapURLItem, MID_GRAFURL);
                rToSet.Put(aXFillBitmapItem);
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
                const XGradient aNullGrad(RGB_Color(COL_BLACK), RGB_Color(COL_WHITE));
                XFillFloatTransparenceItem aXFillFloatTransparenceItem(rToSet.GetPool(), aNullGrad, false);

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

                eMode = (drawing::BitmapMode)nMode;
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
            SvxProtectItem aProt ( static_cast < const ::SvxProtectItem & > ( rFromSet.Get ( RES_PROTECT ) ) );
            if(pCont)
                bRet &= ((SfxPoolItem&)aProt).PutValue(*pCont, MID_PROTECT_CONTENT);
            if(pPos )
                bRet &= ((SfxPoolItem&)aProt).PutValue(*pPos, MID_PROTECT_POSITION);
            if(pName)
                bRet &= ((SfxPoolItem&)aProt).PutValue(*pName, MID_PROTECT_SIZE);
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
            SwFormatHoriOrient aOrient ( static_cast < const ::SwFormatHoriOrient & > ( rFromSet.Get ( RES_HORI_ORIENT ) ) );
            if(pHori )
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pHori, MID_HORIORIENT_ORIENT);
            if(pHoriP)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pHoriP, MID_HORIORIENT_POSITION|CONVERT_TWIPS);
            if(pHoriR)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pHoriR, MID_HORIORIENT_RELATION);
            if(pPageT)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pPageT, MID_HORIORIENT_PAGETOGGLE);
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
            SwFormatVertOrient aOrient ( static_cast < const ::SwFormatVertOrient & > ( rFromSet.Get ( RES_VERT_ORIENT ) ) );
            if(pVert )
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pVert, MID_VERTORIENT_ORIENT);
            if(pVertP)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pVertP, MID_VERTORIENT_POSITION|CONVERT_TWIPS);
            if(pVertR)
                bRet &= ((SfxPoolItem&)aOrient).PutValue(*pVertR, MID_VERTORIENT_RELATION);
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
            SwFormatURL aURL ( static_cast < const ::SwFormatURL & > ( rFromSet.Get ( RES_URL ) ) );
            if(pURL)
                bRet &= ((SfxPoolItem&)aURL).PutValue(*pURL, MID_URL_URL);
            if(pTarget)
                bRet &= ((SfxPoolItem&)aURL).PutValue(*pTarget, MID_URL_TARGET);
            if(pHyLNm)
                bRet &= ((SfxPoolItem&)aURL).PutValue(*pHyLNm, MID_URL_HYPERLINKNAME  );
            if(pHySMp)
                bRet &= ((SfxPoolItem&)aURL).PutValue(*pHySMp, MID_URL_SERVERMAP);
            rToSet.Put(aURL);
        }
    }
    const ::uno::Any* pL = nullptr;
    GetProperty(RES_LR_SPACE, MID_L_MARGIN|CONVERT_TWIPS, pL );
    const ::uno::Any* pR = nullptr;
    GetProperty(RES_LR_SPACE, MID_R_MARGIN|CONVERT_TWIPS, pR );
    if(pL||pR)
    {
        SvxLRSpaceItem aLR ( static_cast < const ::SvxLRSpaceItem & > ( rFromSet.Get ( RES_LR_SPACE ) ) );
        if(pL)
            bRet &= ((SfxPoolItem&)aLR).PutValue(*pL, MID_L_MARGIN|CONVERT_TWIPS);
        if(pR)
            bRet &= ((SfxPoolItem&)aLR).PutValue(*pR, MID_R_MARGIN|CONVERT_TWIPS);
        rToSet.Put(aLR);
    }
    const ::uno::Any* pT = nullptr;
    GetProperty(RES_UL_SPACE, MID_UP_MARGIN|CONVERT_TWIPS, pT );
    const ::uno::Any* pB = nullptr;
    GetProperty(RES_UL_SPACE, MID_LO_MARGIN|CONVERT_TWIPS, pB );
    if(pT||pB)
    {
        SvxULSpaceItem aTB ( static_cast < const ::SvxULSpaceItem &> ( rFromSet.Get ( RES_UL_SPACE ) ) );
        if(pT)
            bRet &= ((SfxPoolItem&)aTB).PutValue(*pT, MID_UP_MARGIN|CONVERT_TWIPS);
        if(pB)
            bRet &= ((SfxPoolItem&)aTB).PutValue(*pB, MID_LO_MARGIN|CONVERT_TWIPS);
        rToSet.Put(aTB);
    }
    const ::uno::Any* pOp;
    if(GetProperty(RES_OPAQUE, 0, pOp))
    {
        SvxOpaqueItem aOp ( static_cast < const ::SvxOpaqueItem& > ( rFromSet.Get ( RES_OPAQUE ) ) );
        bRet &= ((SfxPoolItem&)aOp).PutValue(*pOp, 0);
        rToSet.Put(aOp);
    }
    const ::uno::Any* pPrt;
    if(GetProperty(RES_PRINT, 0, pPrt))
    {
        SvxPrintItem aPrt ( static_cast < const ::SvxPrintItem & > ( rFromSet.Get ( RES_PRINT ) ) );
        bRet &= ((SfxPoolItem&)aPrt).PutValue(*pPrt, 0);
        rToSet.Put(aPrt);
    }
    const ::uno::Any* pSh;
    if(GetProperty(RES_SHADOW, CONVERT_TWIPS, pSh))
    {
        SvxShadowItem aSh ( static_cast < const ::SvxShadowItem& > ( rFromSet.Get ( RES_SHADOW ) ) );
        bRet &= ((SfxPoolItem&)aSh).PutValue(*pSh, CONVERT_TWIPS);
        rToSet.Put(aSh);
    }
    const ::uno::Any* pShTr;
    if(GetProperty(RES_SHADOW, MID_SHADOW_TRANSPARENCE, pShTr) && rToSet.HasItem(RES_SHADOW))
    {
        SvxShadowItem aSh(static_cast<const SvxShadowItem&>(rToSet.Get(RES_SHADOW)));
        bRet &= aSh.PutValue(*pShTr, MID_SHADOW_TRANSPARENCE);
        rToSet.Put(aSh);
    }
    const ::uno::Any* pSur      = nullptr;
    GetProperty(RES_SURROUND, MID_SURROUND_SURROUNDTYPE, pSur);
    const ::uno::Any* pSurAnch = nullptr;
    GetProperty(RES_SURROUND, MID_SURROUND_ANCHORONLY, pSurAnch);
    if(pSur || pSurAnch)
    {
        SwFormatSurround aSrnd ( static_cast < const ::SwFormatSurround & > ( rFromSet.Get ( RES_SURROUND ) ) );
        if(pSur)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSur, MID_SURROUND_SURROUNDTYPE );
        if(pSurAnch)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSurAnch, MID_SURROUND_ANCHORONLY);
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
        SvxBoxItem aBox ( static_cast < const ::SvxBoxItem & > ( rFromSet.Get ( RES_BOX ) ) );
        if( pLeft )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pLeft, CONVERT_TWIPS|LEFT_BORDER );
        if( pRight )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pRight, CONVERT_TWIPS|RIGHT_BORDER );
        if( pTop )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pTop, CONVERT_TWIPS|TOP_BORDER);
        if( pBottom )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pBottom, CONVERT_TWIPS|BOTTOM_BORDER);
        if( pDistance )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pDistance, CONVERT_TWIPS|BORDER_DISTANCE);
        if( pLeftDistance )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pLeftDistance, CONVERT_TWIPS|LEFT_BORDER_DISTANCE);
        if( pRightDistance )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pRightDistance, CONVERT_TWIPS|RIGHT_BORDER_DISTANCE);
        if( pTopDistance )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pTopDistance, CONVERT_TWIPS|TOP_BORDER_DISTANCE);
        if( pBottomDistance )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pBottomDistance, CONVERT_TWIPS|BOTTOM_BORDER_DISTANCE);
        if( pLineStyle )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pLineStyle, LINE_STYLE);
        if( pLineWidth )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pLineWidth, LINE_WIDTH|CONVERT_TWIPS);
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
            SwFormatFrameSize aFrameSz ( static_cast < const ::SwFormatFrameSize& > ( rFromSet.Get ( RES_FRM_SIZE ) ) );
            if(pWidth)
                bRet &= ((SfxPoolItem&)aFrameSz).PutValue(*pWidth, MID_FRMSIZE_WIDTH|CONVERT_TWIPS);
            if(pHeight)
                bRet &= ((SfxPoolItem&)aFrameSz).PutValue(*pHeight, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS);
            if(pRelH )
                bRet &= ((SfxPoolItem&)aFrameSz).PutValue(*pRelH, MID_FRMSIZE_REL_HEIGHT);
            if (pRelHRelation)
                bRet &= const_cast<SwFormatFrameSize&>(aFrameSz).PutValue(*pRelHRelation, MID_FRMSIZE_REL_HEIGHT_RELATION);
            if(pRelW )
                bRet &= ((SfxPoolItem&)aFrameSz).PutValue(*pRelW, MID_FRMSIZE_REL_WIDTH);
            if (pRelWRelation)
                bRet &= const_cast<SwFormatFrameSize&>(aFrameSz).PutValue(*pRelWRelation, MID_FRMSIZE_REL_WIDTH_RELATION);
            if(pSyncWidth)
                bRet &= ((SfxPoolItem&)aFrameSz).PutValue(*pSyncWidth, MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT);
            if(pSyncHeight)
                bRet &= ((SfxPoolItem&)aFrameSz).PutValue(*pSyncHeight, MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH);
            if(pSize)
                bRet &= ((SfxPoolItem&)aFrameSz).PutValue(*pSize, MID_FRMSIZE_SIZE|CONVERT_TWIPS);
            if(pSizeType)
                bRet &= ((SfxPoolItem&)aFrameSz).PutValue(*pSizeType, MID_FRMSIZE_SIZE_TYPE);
            if(pWidthType)
                bRet &= ((SfxPoolItem&)aFrameSz).PutValue(*pWidthType, MID_FRMSIZE_WIDTH_TYPE);
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
            ((SfxPoolItem&)aFrameSz).PutValue(aSizeVal, MID_FRMSIZE_SIZE|CONVERT_TWIPS);
            rToSet.Put(aFrameSz);
        }
    }
    const ::uno::Any* pFrameDirection = nullptr;
    GetProperty(RES_FRAMEDIR, 0, pFrameDirection);
    if(pFrameDirection)
    {
        SvxFrameDirectionItem aAttr(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR);
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
    GetProperty(RES_FOLLOW_TEXT_FLOW, 0, pFollowTextFlow);
    if ( pFollowTextFlow )
    {
        SwFormatFollowTextFlow aFormatFollowTextFlow;
        aFormatFollowTextFlow.PutValue(*pFollowTextFlow, 0);
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
            SdrTextVertAdjustItem aTextVertAdjust(static_cast <const ::SdrTextVertAdjustItem & > ( rFromSet.Get ( RES_TEXT_VERT_ADJUST ) ));
            bRet &= ((SfxPoolItem&)aTextVertAdjust).PutValue(*pTextVertAdjust, 0);
            rToSet.Put(aTextVertAdjust);
        }
    }

    return bRet;
}

class SwFrameProperties_Impl : public BaseFrameProperties_Impl
{
public:
    SwFrameProperties_Impl();
    virtual ~SwFrameProperties_Impl(){}

    bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrameSet, SfxItemSet& rSet, bool& rSizeFound) override;
};

SwFrameProperties_Impl::SwFrameProperties_Impl():
    BaseFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME)*/ )
{
}

static inline void lcl_FillCol ( SfxItemSet &rToSet, const ::SfxItemSet &rFromSet, const ::uno::Any *pAny)
{
    if ( pAny )
    {
        SwFormatCol aCol ( static_cast < const ::SwFormatCol & > ( rFromSet.Get ( RES_COL ) ) );
        ((SfxPoolItem&)aCol).PutValue( *pAny, MID_COLUMNS);
        rToSet.Put(aCol);
    }
}

bool SwFrameProperties_Impl::AnyToItemSet(SwDoc *pDoc, SfxItemSet& rSet, SfxItemSet&, bool& rSizeFound)
{
    //Properties fuer alle Frames
    const ::uno::Any *pStyleName;
    SwDocStyleSheet* pStyle = nullptr;
    bool bRet;

    if ( GetProperty ( FN_UNO_FRAME_STYLE_NAME, 0, pStyleName ) )
    {
        OUString sStyle;
        *pStyleName >>= sStyle;
        SwStyleNameMapper::FillUIName(sStyle, sStyle, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT, true);
        pStyle = static_cast<SwDocStyleSheet*>(pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle,
                                                    SFX_STYLE_FAMILY_FRAME));
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
    virtual ~SwGraphicProperties_Impl(){}

    virtual bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrameSet, SfxItemSet& rSet, bool& rSizeFound) override;
};

SwGraphicProperties_Impl::SwGraphicProperties_Impl( ) :
    BaseFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC)*/ )
{
}

static inline void lcl_FillMirror ( SfxItemSet &rToSet, const ::SfxItemSet &rFromSet, const ::uno::Any *pHEvenMirror, const ::uno::Any *pHOddMirror, const ::uno::Any *pVMirror, bool &rRet )
{
    if(pHEvenMirror || pHOddMirror || pVMirror )
    {
        SwMirrorGrf aMirror ( static_cast < const ::SwMirrorGrf& > ( rFromSet.Get ( RES_GRFATR_MIRRORGRF ) ) );
        if(pHEvenMirror)
            rRet &= ((SfxPoolItem&)aMirror).PutValue(*pHEvenMirror, MID_MIRROR_HORZ_EVEN_PAGES);
        if(pHOddMirror)
            rRet &= ((SfxPoolItem&)aMirror).PutValue(*pHOddMirror, MID_MIRROR_HORZ_ODD_PAGES);
        if(pVMirror)
            rRet &= ((SfxPoolItem&)aMirror).PutValue(*pVMirror, MID_MIRROR_VERT);
        rToSet.Put(aMirror);
    }
}

bool SwGraphicProperties_Impl::AnyToItemSet(
            SwDoc* pDoc,
            SfxItemSet& rFrameSet,
            SfxItemSet& rGrSet,
            bool& rSizeFound)
{
    //Properties fuer alle Frames
    bool bRet;
    const ::uno::Any *pStyleName;
    SwDocStyleSheet* pStyle = nullptr;

    if ( GetProperty ( FN_UNO_FRAME_STYLE_NAME, 0, pStyleName ) )
    {
        OUString sStyle;
        *pStyleName >>= sStyle;
        SwStyleNameMapper::FillUIName(sStyle, sStyle, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT, true);
        pStyle = static_cast<SwDocStyleSheet*>(pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle,
                                                    SFX_STYLE_FAMILY_FRAME));
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
            SfxPoolItem* pItem = ::GetDfltAttr( nIDs[nIndex] )->Clone();
            bRet &= pItem->PutValue(*pAny, nMId );
            rGrSet.Put(*pItem);
            delete pItem;
        }
    }

    return bRet;
}

class SwOLEProperties_Impl : public SwFrameProperties_Impl
{
public:
    SwOLEProperties_Impl() :
        SwFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_EMBEDDED_OBJECT)*/ ){}
    virtual ~SwOLEProperties_Impl(){}

    virtual bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrameSet, SfxItemSet& rSet, bool& rSizeFound) override;
};

bool SwOLEProperties_Impl::AnyToItemSet(
        SwDoc* pDoc, SfxItemSet& rFrameSet, SfxItemSet& rSet, bool& rSizeFound)
{
    const ::uno::Any* pTemp;
    if(!GetProperty(FN_UNO_CLSID, 0, pTemp) && !GetProperty(FN_UNO_STREAM_NAME, 0, pTemp)
         && !GetProperty(FN_EMBEDDED_OBJECT, 0, pTemp) )
        return false;
    SwFrameProperties_Impl::AnyToItemSet( pDoc, rFrameSet, rSet, rSizeFound);

    return true;
}

class SwXFrame::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    uno::WeakReference<uno::XInterface> m_wThis;
    ::cppu::OInterfaceContainerHelper m_EventListeners;

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
    throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}


OUString SwXFrame::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXFrame");
}

sal_Bool SwXFrame::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXFrame::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
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
    , m_pCopySource(nullptr)
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
            pProps = new SwFrameProperties_Impl( );
        }
        break;
        case FLYCNTTYPE_GRF:
        {
            uno::Any aAny2 = mxStyleFamily->getByName ("Graphics");
            aAny2 >>= mxStyleData;
            pProps = new SwGraphicProperties_Impl( );
        }
        break;
        case FLYCNTTYPE_OLE:
        {
            uno::Any aAny2 = mxStyleFamily->getByName ("OLE");
            aAny2 >>= mxStyleData;
            pProps = new SwOLEProperties_Impl( );
        }
        break;

        default:
            pProps = nullptr;
            ;
    }
}

SwXFrame::SwXFrame(SwFrameFormat& rFrameFormat, FlyCntType eSet, const ::SfxItemPropertySet* pSet)
    : SwClient( &rFrameFormat )
    , m_pImpl(new Impl)
    ,
    m_pPropSet(pSet),
    m_pDoc( nullptr ),
    eType(eSet),
    pProps(nullptr),
    bIsDescriptor(false),
    m_pCopySource(nullptr)
{

}

SwXFrame::~SwXFrame()
{
    SolarMutexGuard aGuard;
    delete m_pCopySource;
    delete pProps;
    if(GetRegisteredIn())
        GetRegisteredIn()->Remove(this);
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
        NameLookupIsHard *const pNew((pFrameFormat)
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

OUString SwXFrame::getName() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
        return pFormat->GetName();
    if(!bIsDescriptor)
        throw uno::RuntimeException();
    return m_sName;
}

void SwXFrame::setName(const OUString& rName) throw( uno::RuntimeException, std::exception )
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

uno::Reference< beans::XPropertySetInfo >  SwXFrame::getPropertySetInfo() throw( uno::RuntimeException, std::exception )
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
    delete m_pCopySource;
    m_pCopySource = new SwPaM( *rCopySource.Start() );
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
        SwDrawModel* pDrawModel = pDoc->getIDocumentDrawModelAccess().GetOrCreateDrawModel();
        SwFlyDrawContact* pContactObject
                    = new SwFlyDrawContact( &rFormat, pDrawModel );
        pObject = pContactObject->GetMaster();

        const ::SwFormatSurround& rSurround = rFormat.GetSurround();
        pObject->SetLayer(
            ( SURROUND_THROUGHT == rSurround.GetSurround() &&
              !rFormat.GetOpaque().GetValue() ) ? pDoc->getIDocumentDrawModelAccess().GetHellId()
                                             : pDoc->getIDocumentDrawModelAccess().GetHeavenId() );
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
                nsSwGetPoolIdFromName::GET_POOLID_FRMFMT, true);
        SwDocStyleSheet* pStyle =
                static_cast<SwDocStyleSheet*>(pDocSh->GetStyleSheetPool()->Find(sStyle,
                                                    SFX_STYLE_FAMILY_FRAME));
        if(pStyle)
            pRet = pStyle->GetFrameFormat();
    }

    return pRet;
}

void SwXFrame::setPropertyValue(const OUString& rPropertyName, const ::uno::Any& _rValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    const ::SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);

    if (!pEntry)
        throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    //UUUU
    const sal_uInt8 nMemberId(pEntry->nMemberId & (~SFX_METRIC_ITEM));
    uno::Any aValue(_rValue);

    //UUUU check for needed metric translation
    if(pEntry->nMemberId & SFX_METRIC_ITEM)
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
            const SfxMapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

            if(eMapUnit != SFX_MAPUNIT_100TH_MM)
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
            (FN_PARAM_COUNTOUR_PP        == pEntry->nWID) ||
            (FN_UNO_IS_AUTOMATIC_CONTOUR == pEntry->nWID) ||
            (FN_UNO_IS_PIXEL_CONTOUR     == pEntry->nWID) )
        {
            const ::SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTextNode* pNoText = aIdx.GetNode().GetNoTextNode();
                if(pEntry->nWID == FN_PARAM_COUNTOUR_PP)
                {
                    drawing::PointSequenceSequence aParam;
                    if(!aValue.hasValue())
                        pNoText->SetContour(nullptr);
                    else if(aValue >>= aParam)
                    {
                        tools::PolyPolygon aPoly((sal_uInt16)aParam.getLength());
                        for(sal_Int32 i = 0; i < aParam.getLength(); i++)
                        {
                            const ::drawing::PointSequence* pPointSeq = aParam.getConstArray();
                            sal_Int32 nPoints = pPointSeq[i].getLength();
                            const ::awt::Point* pPoints = pPointSeq[i].getConstArray();
                            tools::Polygon aSet( (sal_uInt16)nPoints );
                            for(sal_Int32 j = 0; j < nPoints; j++)
                            {
                                Point aPoint(pPoints[j].X, pPoints[j].Y);
                                aSet.SetPoint(aPoint, (sal_uInt16)j);
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
                    pNoText->SetAutomaticContour( *static_cast<sal_Bool const *>(aValue.getValue()) );
                }
                else if(pEntry->nWID == FN_UNO_IS_PIXEL_CONTOUR )
                {
                    // The IsPixelContour property can only be set if there
                    // is no contour, or if the contour has been set by the
                    // API itself (or in other words, if the contour isn't
                    // used already).
                    if( !pNoText->_HasContour() ||
                        !pNoText->IsContourMapModeValid() )
                        pNoText->SetPixelContour( *static_cast<sal_Bool const *>(aValue.getValue()) );
                    else
                        throw lang::IllegalArgumentException();
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
            if( pFrameFormat )
            {
                UnoActionContext aAction(pFormat->GetDoc());

                SfxItemSet* pSet = nullptr;
                // #i31771#, #i25798# - No adjustment of
                // anchor ( no call of method <sw_ChkAndSetNewAnchor(..)> ),
                // if document is currently in reading mode.
                if ( !pFormat->GetDoc()->IsInReading() )
                {
                    // see SwFEShell::SetFrameFormat( SwFrameFormat *pNewFormat, bool bKeepOrient, Point* pDocPos )
                    SwFlyFrame *pFly = nullptr;
                    {
                        const SwFrameFormat* pFormatXX = pFormat;
                        if (dynamic_cast<const SwFlyFrameFormat*>( pFormatXX) )
                            pFly = static_cast<const SwFlyFrameFormat*>(pFormatXX)->GetFrame();
                    }
                    if ( pFly )
                    {
                        const ::SfxPoolItem* pItem;
                        if( SfxItemState::SET == pFrameFormat->GetItemState( RES_ANCHOR, false, &pItem ))
                        {
                            pSet = new SfxItemSet( pDoc->GetAttrPool(), aFrameFormatSetRange );
                            pSet->Put( *pItem );
                            if ( pFormat->GetDoc()->GetEditShell() != nullptr
                                 && !sw_ChkAndSetNewAnchor( *pFly, *pSet ) )
                            {
                                delete pSet;
                                pSet = nullptr;
                            }
                        }
                    }
                }

                pFormat->GetDoc()->SetFrameFormatToFly( *pFormat, *pFrameFormat, pSet );
                delete pSet;
            }
            else
                throw lang::IllegalArgumentException();
        }
        else if( FN_UNO_GRAPHIC_U_R_L == pEntry->nWID ||
                FN_UNO_GRAPHIC_FILTER == pEntry->nWID)
        {
            OUString sGrfName;
            OUString sFltName;
            GraphicObject *pGrfObj = nullptr;
            SwDoc::GetGrfNms( *static_cast<SwFlyFrameFormat*>(pFormat), &sGrfName, &sFltName );
            OUString sTmp;
            aValue >>= sTmp;
            UnoActionContext aAction(pFormat->GetDoc());
            if(FN_UNO_GRAPHIC_U_R_L == pEntry->nWID)
            {
                if( sTmp.startsWith(sPackageProtocol) )
                {
                    pGrfObj = new GraphicObject;
                    pGrfObj->SetUserData( sTmp );
                    sGrfName.clear();
                }
                else if( sTmp.startsWith(sGraphicObjectProtocol) )
                {
                    const OString sId(OUStringToOString(
                        sTmp.copy(sizeof(sGraphicObjectProtocol)-1),
                        RTL_TEXTENCODING_ASCII_US));
                    pGrfObj = new GraphicObject( sId );
                    sGrfName.clear();
                }
                else
                {
                    sGrfName = sTmp;
                }
            }
            else
            {
                sFltName = sTmp;
            }

            const ::SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                {
                    delete pGrfObj;
                    throw uno::RuntimeException();
                }
                SwPaM aGrfPaM(*pGrfNode);
                pFormat->GetDoc()->getIDocumentContentOperations().ReRead( aGrfPaM, sGrfName, sFltName, nullptr,
                                        pGrfObj );
            }
            delete pGrfObj;
        }
        else if( FN_UNO_GRAPHIC == pEntry->nWID )
        {
            uno::Reference< graphic::XGraphic > xGraphic;
            aValue >>= xGraphic;
            if(xGraphic.is())
            {
                const ::SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
                if(pIdx)
                {
                    SwNodeIndex aIdx(*pIdx, 1);
                    SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                    if(!pGrfNode)
                    {
                        throw uno::RuntimeException();
                    }
                    SwPaM aGrfPaM(*pGrfNode);
                    Graphic aGraphic( xGraphic );
                    pFormat->GetDoc()->getIDocumentContentOperations().ReRead( aGrfPaM, OUString(), OUString(), &aGraphic, nullptr );
                }
            }
        }
        else if( FN_UNO_REPLACEMENT_GRAPHIC_URL == pEntry->nWID || FN_UNO_REPLACEMENT_GRAPHIC == pEntry->nWID )
        {
            bool bURL = FN_UNO_REPLACEMENT_GRAPHIC_URL == pEntry->nWID;
            bool bApply = false;
            Graphic aGraphic;
            if( bURL )
            {
                GraphicObject *pGrfObj = nullptr;
                OUString aGrfUrl;
                aValue >>= aGrfUrl;

                // the package URL based graphics are handled in different way currently
                // TODO/LATER: actually this is the correct place to handle them
                OUString aGraphicProtocol( sGraphicObjectProtocol );
                if( aGrfUrl.startsWith( aGraphicProtocol ) )
                {
                    OString sId(OUStringToOString(
                        aGrfUrl.copy(sizeof(sGraphicObjectProtocol)-1),
                        RTL_TEXTENCODING_ASCII_US));
                    pGrfObj = new GraphicObject( sId );
                    aGraphic = pGrfObj->GetGraphic();
                    bApply = true;
                }
            }
            else
            {
                uno::Reference< graphic::XGraphic > xGraphic;
                aValue >>= xGraphic;
                if( xGraphic.is() )
                {
                    aGraphic = Graphic( xGraphic );
                    bApply = true;
                }
            }

            if ( bApply )
            {
                const ::SwFormatContent* pCnt = &pFormat->GetContent();
                if ( pCnt->GetContentIdx() && pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ] )
                {
                    SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ]->GetOLENode();

                    if ( pOleNode )
                    {
                        svt::EmbeddedObjectRef &rObj = pOleNode->GetOLEObj().GetObject();

                        OUString aMediaType;
                        rObj.SetGraphic( aGraphic, aMediaType );
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
                    SwFormatChain aChain( pFormat->GetChain() );
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
            std::set<const SwFrameFormat*> aTextBoxes = SwTextBoxHelper::findTextBoxes(pDoc);
            if( nZOrder >= 0 && aTextBoxes.find(pFormat) == aTextBoxes.end())
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
                                RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                    aSet.SetParent(&pFormat->GetAttrSet());
                    SwFormatAnchor aAnchor = static_cast<const SwFormatAnchor&>(aSet.Get(pEntry->nWID));

                    SwPosition aPos(*pFrame->GetFrameFormat()->GetContent().GetContentIdx());
                    aAnchor.SetAnchor(&aPos);
                    aAnchor.SetType(FLY_AT_FLY);
                    aSet.Put(aAnchor);
                    pDoc->SetFlyFrameAttr( *pFormat, aSet );
                    bDone = true;
                }
            }
            if(!bDone)
                throw lang::IllegalArgumentException();
        }
        else
        {   //UUUU
            // standard UNO API write attributes
            // adapt former attr from SvxBrushItem::PutValue to new items XATTR_FILL_FIRST, XATTR_FILL_LAST
            SfxItemSet aSet( pDoc->GetAttrPool(),
                RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
                RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,

                //UUUU FillAttribute support
                XATTR_FILL_FIRST, XATTR_FILL_LAST,

                0L);
            bool bDone(false);

            aSet.SetParent(&pFormat->GetAttrSet());

            if(RES_BACKGROUND == pEntry->nWID)
            {
                const SwAttrSet& rSet = pFormat->GetAttrSet();
                const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(rSet, RES_BACKGROUND, true, pDoc->IsInXMLImport()));
                SvxBrushItem aChangedBrushItem(aOriginalBrushItem);

                aChangedBrushItem.PutValue(aValue, nMemberId);

                if(!(aChangedBrushItem == aOriginalBrushItem))
                {
                    setSvxBrushItemAsFillAttributesToTargetSet(aChangedBrushItem, aSet);
                    pFormat->GetDoc()->SetFlyFrameAttr( *pFormat, aSet );
                }

                bDone = true;
            }
            else if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
            {
                //UUUU
                drawing::BitmapMode eMode;

                if(!(aValue >>= eMode))
                {
                    sal_Int32 nMode = 0;

                    if(!(aValue >>= nMode))
                    {
                        throw lang::IllegalArgumentException();
                    }

                    eMode = (drawing::BitmapMode)nMode;
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
                    //UUUU when named items get set, replace these with the NameOrIndex items
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
                case MID_GRAFURL:
                {
                    //UUUU Bitmap also has the MID_GRAFURL mode where a Bitmap URL is used
                    switch(pEntry->nWID)
                    {
                        case XATTR_FILLBITMAP:
                        {
                            const Graphic aNullGraphic;
                            XFillBitmapItem aXFillBitmapItem(aSet.GetPool(), aNullGraphic);

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
                if(aAnchor.GetAnchorId() == FLY_AT_FLY)
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
                else if ((aAnchor.GetAnchorId() != FLY_AT_PAGE) &&
                         !aAnchor.GetContentAnchor())
                {
                    SwNode& rNode = pDoc->GetNodes().GetEndOfContent();
                    SwPaM aPam(rNode);
                    aPam.Move( fnMoveBackward, fnGoDoc );
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
                    if (dynamic_cast<SwFlyFrameFormat*>( pFormat) )
                        pFly = static_cast<SwFlyFrameFormat*>(pFormat)->GetFrame();
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
        pProps->SetProperty(pEntry->nWID, nMemberId, aValue);
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
    }
    else
        throw uno::RuntimeException();
}

uno::Any SwXFrame::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Any aAny;
    SwFrameFormat* pFormat = GetFrameFormat();
    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
        throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    //UUUU
    const sal_uInt8 nMemberId(pEntry->nMemberId & (~SFX_METRIC_ITEM));

    if(FN_UNO_ANCHOR_TYPES == pEntry->nWID)
    {
        uno::Sequence<text::TextContentAnchorType> aTypes(5);
         text::TextContentAnchorType* pArray = aTypes.getArray();
        pArray[0] = text::TextContentAnchorType_AT_PARAGRAPH;
        pArray[1] = text::TextContentAnchorType_AS_CHARACTER;
        pArray[2] = text::TextContentAnchorType_AT_PAGE;
        pArray[3] = text::TextContentAnchorType_AT_FRAME;
        pArray[4] = text::TextContentAnchorType_AT_CHARACTER;
        aAny.setValue(&aTypes, cppu::UnoType<uno::Sequence<text::TextContentAnchorType>>::get());
    }
    else if(pFormat)
    {
        if( ((eType == FLYCNTTYPE_GRF) || (eType == FLYCNTTYPE_OLE)) &&
                (isGRFATR(pEntry->nWID) ||
                        pEntry->nWID == FN_PARAM_COUNTOUR_PP ||
                        pEntry->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR ||
                        pEntry->nWID == FN_UNO_IS_PIXEL_CONTOUR ))
        {
            const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTextNode* pNoText = aIdx.GetNode().GetNoTextNode();
                if(pEntry->nWID == FN_PARAM_COUNTOUR_PP)
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
                    SfxItemSet aSet(pNoText->GetSwAttrSet());
                    m_pPropSet->getPropertyValue(*pEntry, aSet, aAny);
                }
            }
        }
        else if( FN_UNO_GRAPHIC_U_R_L == pEntry->nWID)
        {
            OUString sGrfName;
            const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                    throw uno::RuntimeException();
                if( pGrfNode->IsGrfLink() )
                {
                    SwDoc::GetGrfNms( *static_cast<SwFlyFrameFormat*>(pFormat), &sGrfName, nullptr );
                }
                else
                {
                    OUString sId(OStringToOUString(
                        pGrfNode->GetGrfObj().GetUniqueID(),
                        RTL_TEXTENCODING_ASCII_US));
                    sGrfName = sGraphicObjectProtocol + sId;
                }
            }
            aAny <<= sGrfName;
        }
        else if( FN_UNO_REPLACEMENT_GRAPHIC_U_R_L == pEntry->nWID)
        {
            OUString sGrfName;
            const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();

            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                    throw uno::RuntimeException();

                const GraphicObject* pGraphicObject = pGrfNode->GetReplacementGrfObj();

                if(pGraphicObject)
                {
                    sGrfName = sGraphicObjectProtocol
                             + OStringToOUString( pGraphicObject->GetUniqueID(), RTL_TEXTENCODING_ASCII_US );
                }
            }

            aAny <<= sGrfName;
        }
        else if( FN_UNO_GRAPHIC_FILTER == pEntry->nWID )
        {
            OUString sFltName;
            SwDoc::GetGrfNms( *static_cast<SwFlyFrameFormat*>(pFormat), nullptr, &sFltName );
                aAny <<= sFltName;
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
            aAny <<= OUString(SwStyleNameMapper::GetProgName(pFormat->DerivedFrom()->GetName(), nsSwGetPoolIdFromName::GET_POOLID_FRMFMT ) );
        }
        // #i73249#
        else if( FN_UNO_TITLE == pEntry->nWID )
        {
            SwFlyFrameFormat& rFlyFormat = dynamic_cast<SwFlyFrameFormat&>(*pFormat);
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject(rFlyFormat);
            aAny <<= OUString(rFlyFormat.GetObjTitle());
        }
        // New attribute Description
        else if( FN_UNO_DESCRIPTION == pEntry->nWID )
        {
            SwFlyFrameFormat& rFlyFormat = dynamic_cast<SwFlyFrameFormat&>(*pFormat);
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject(rFlyFormat);
            aAny <<= OUString(rFlyFormat.GetObjDescription());
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
                aAny.setValue(&aTmp, ::cppu::UnoType<awt::Size>::get());
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
                aAny <<= (sal_Int32)pObj->GetOrdNum();
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
                                            GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

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
                aAny <<= OUString(pOleNode->GetOLEObj().GetCurrentPersistName());
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
                OSL_ENSURE( pTmpFrame->IsValid(), "frame not valid" );
                const SwRect &rRect = pTmpFrame->Frame();
                Size aMM100Size = OutputDevice::LogicToLogic(
                        Size( rRect.Width(), rRect.Height() ),
                        MapMode( MAP_TWIP ), MapMode( MAP_100TH_MM ));
                aAny <<= awt::Size( aMM100Size.Width(), aMM100Size.Height() );
            }
        }
        else
        {   //UUUU
            // standard UNO API read attributes
            // adapt former attr from SvxBrushItem::PutValue to new items XATTR_FILL_FIRST, XATTR_FILL_LAST
            const SwAttrSet& rSet = pFormat->GetAttrSet();
            bool bDone(false);

            if(RES_BACKGROUND == pEntry->nWID)
            {
                //UUUU
                const SvxBrushItem aOriginalBrushItem(getSvxBrushItemFromSourceSet(rSet, RES_BACKGROUND));

                if(!aOriginalBrushItem.QueryValue(aAny, nMemberId))
                {
                    OSL_ENSURE(false, "Error getting attribute from RES_BACKGROUND (!)");
                }

                bDone = true;
            }
            else if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
            {
                //UUUU
                const XFillBmpStretchItem* pStretchItem = dynamic_cast< const XFillBmpStretchItem* >(&rSet.Get(XATTR_FILLBMP_STRETCH));
                const XFillBmpTileItem* pTileItem = dynamic_cast< const XFillBmpTileItem* >(&rSet.Get(XATTR_FILLBMP_TILE));

                if( pTileItem && pTileItem->GetValue() )
                {
                    aAny <<= drawing::BitmapMode_REPEAT;
                }
                else if( pStretchItem && pStretchItem->GetValue() )
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
            if( !pProps->GetProperty( pEntry->nWID, nMemberId, pAny ) )
                aAny = mxStyleData->getPropertyValue( rPropertyName );
            else if ( pAny )
                aAny = *pAny;
        }
    }
    else
        throw uno::RuntimeException();

    //UUUU
    if(pEntry && pEntry->aType == ::cppu::UnoType<sal_Int16>::get() && pEntry->aType != aAny.getValueType())
    {
        // since the sfx uint16 item now exports a sal_Int32, we may have to fix this here
        sal_Int32 nValue = 0;
        aAny >>= nValue;
        aAny <<= (sal_Int16)nValue;
    }

    //UUUU check for needed metric translation
    if(pEntry->nMemberId & SFX_METRIC_ITEM)
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
            const SfxMapUnit eMapUnit(rPool.GetMetric(pEntry->nWID));

            if(eMapUnit != SFX_MAPUNIT_100TH_MM)
            {
                SvxUnoConvertToMM(eMapUnit, aAny);
            }
        }
    }

    return aAny;
}

void SwXFrame::addPropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXFrame::removePropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXFrame::addVetoableChangeListener(const OUString& /*PropertyName*/,
                                const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

void SwXFrame::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
{
    OSL_FAIL("not implemented");
}

beans::PropertyState SwXFrame::getPropertyState( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aPropertyNames { rPropertyName };
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aPropertyNames);
    return aStates.getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXFrame::getPropertyStates(
    const uno::Sequence< OUString >& aPropertyNames )
        throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
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
                FN_UNO_GRAPHIC_U_R_L == pEntry->nWID||
                FN_UNO_GRAPHIC_FILTER     == pEntry->nWID||
                FN_UNO_ACTUAL_SIZE == pEntry->nWID||
                FN_UNO_ALTERNATIVE_TEXT == pEntry->nWID)
            {
                pStates[i] = beans::PropertyState_DIRECT_VALUE;
            }
            else if(OWN_ATTR_FILLBMP_MODE == pEntry->nWID)
            {
                //UUUU
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
            //UUUU for FlyFrames we need to mark the used properties from type RES_BACKGROUND
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
                        SfxItemSet aSet(pNoText->GetSwAttrSet());
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
    throw(beans::UnknownPropertyException, uno::RuntimeException, std::exception)
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
            //UUUU
            SwDoc* pDoc = pFormat->GetDoc();
            SfxItemSet aSet(pDoc->GetAttrPool(), XATTR_FILL_FIRST, XATTR_FILL_LAST);
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
                rFlyFormat.GetDoc()->SetFlyFrameTitle(rFlyFormat, aEmptyOUStr);
            }
            // New attribute Description
            else if( FN_UNO_DESCRIPTION == pEntry->nWID )
            {
                SwFlyFrameFormat& rFlyFormat = dynamic_cast<SwFlyFrameFormat&>(*pFormat);
                // assure that <SdrObject> instance exists.
                GetOrCreateSdrObject(rFlyFormat);
                rFlyFormat.GetDoc()->SetFlyFrameDescription(rFlyFormat, aEmptyOUStr);
            }
            else
            {
                SwDoc* pDoc = pFormat->GetDoc();
                SfxItemSet aSet( pDoc->GetAttrPool(),
                    RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                aSet.SetParent(&pFormat->GetAttrSet());
                aSet.ClearItem(pEntry->nWID);
                if(rPropertyName != UNO_NAME_ANCHOR_TYPE)
                    pFormat->SetFormatAttr(aSet);
            }
        }
        else if((bNextFrame = (rPropertyName == UNO_NAME_CHAIN_NEXT_NAME))
                || rPropertyName == UNO_NAME_CHAIN_PREV_NAME)
        {
            SwDoc* pDoc = pFormat->GetDoc();
            if(bNextFrame)
                pDoc->Unchain(*pFormat);
            else
            {
                SwFormatChain aChain( pFormat->GetChain() );
                SwFrameFormat *pPrev = aChain.GetPrev();
                if(pPrev)
                    pDoc->Unchain(*pPrev);
            }
        }
    }
    else if(!IsDescriptor())
        throw uno::RuntimeException();

}

uno::Any SwXFrame::getPropertyDefault( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(pEntry)
        {
            if ( pEntry->nWID < RES_FRMATR_END )
            {
                const SfxPoolItem& rDefItem =
                    pFormat->GetDoc()->GetAttrPool().GetDefaultItem(pEntry->nWID);
                //UUUU
                const sal_uInt8 nMemberId(pEntry->nMemberId & (~SFX_METRIC_ITEM));

                rDefItem.QueryValue(aRet, nMemberId);
            }
        }
        else
            throw beans::UnknownPropertyException( "Unknown property: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else if(!IsDescriptor())
        throw uno::RuntimeException();
    return aRet;
}

void SAL_CALL SwXFrame::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXFrame::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException, std::exception)
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

void SwXFrame::dispose() throw( uno::RuntimeException, std::exception )
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
            if (pFormat->GetAnchor().GetAnchorId() == FLY_AS_CHAR)
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

uno::Reference< text::XTextRange >  SwXFrame::getAnchor() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  aRef;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        const SwFormatAnchor& rAnchor = pFormat->GetAnchor();
        // return an anchor for non-page bound frames
        // and for page bound frames that have a page no == NULL and a content position
        if ((rAnchor.GetAnchorId() != FLY_AT_PAGE) ||
            (rAnchor.GetContentAnchor() && !rAnchor.GetPageNum()))
        {
            const SwPosition &rPos = *(rAnchor.GetContentAnchor());
            aRef = SwXTextRange::CreateXTextRange(*pFormat->GetDoc(), rPos, nullptr);
        }
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

void SwXFrame::ResetDescriptor()
{
    bIsDescriptor = false;
    mxStyleData.clear();
    mxStyleFamily.clear();
    DELETEZ(pProps);
}

void SwXFrame::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
            throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
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
    if(pDoc)
    {
        SwUnoInternalPaM aIntPam(*pDoc);
        // this now needs to return TRUE
        ::sw::XTextRangeToSwPaM(aIntPam, xTextRange);

        SwNode& rNode = pDoc->GetNodes().GetEndOfContent();
        SwPaM aPam(rNode);
        aPam.Move( fnMoveBackward, fnGoDoc );
        static sal_uInt16 const aFrameAttrRange[] =
        {
            RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
            RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,

            //UUUU FillAttribute support
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

        //UUUU set correct parent to get the XFILL_NONE FillStyle as needed
        aFrameSet.SetParent(&pDoc->GetDfltFrameFormat()->GetAttrSet());

        // no the related items need to be added to the set
        bool bSizeFound;
        if(!pProps->AnyToItemSet( pDoc, aFrameSet, aGrSet, bSizeFound))
            throw lang::IllegalArgumentException();
        // a TextRange is handled separately
        *aPam.GetPoint() = *aIntPam.GetPoint();
        if(aIntPam.HasMark())
        {
            aPam.SetMark();
            *aPam.GetMark() = *aIntPam.GetMark();
        }

        const SfxPoolItem* pItem;
        RndStdIds eAnchorId = FLY_AT_PARA;
        if(SfxItemState::SET == aFrameSet.GetItemState(RES_ANCHOR, false, &pItem) )
        {
            eAnchorId = static_cast<const SwFormatAnchor*>(pItem)->GetAnchorId();
            if( FLY_AT_FLY == eAnchorId &&
                !aPam.GetNode().FindFlyStartNode())
            {
                // framebound only where a frame exists
                SwFormatAnchor aAnchor(FLY_AT_PARA);
                aFrameSet.Put(aAnchor);
            }
            else if ((FLY_AT_PAGE == eAnchorId) &&
                     0 == static_cast<const SwFormatAnchor*>(pItem)->GetPageNum() )
            {
                SwFormatAnchor aAnchor( *static_cast<const SwFormatAnchor*>(pItem) );
                aAnchor.SetAnchor( aPam.GetPoint() );
                aFrameSet.Put(aAnchor);
            }
        }

        const ::uno::Any* pStyle;
        SwFrameFormat *pParentFrameFormat = nullptr;
        if(pProps->GetProperty(FN_UNO_FRAME_STYLE_NAME, 0, pStyle))
            pParentFrameFormat = lcl_GetFrameFormat( *pStyle, pDoc );

        SwFlyFrameFormat* pFormat = nullptr;
        if( eType == FLYCNTTYPE_FRM)
        {
            UnoActionContext aCont(pDoc);
            if(m_pCopySource)
            {
                SwFormatAnchor* pAnchorItem = nullptr;
                // the frame is inserted bound to page
                // to prevent conflicts if the to-be-anchored position is part of the to-be-copied text
                if (eAnchorId != FLY_AT_PAGE)
                {
                    pAnchorItem = static_cast<SwFormatAnchor*>(aFrameSet.Get(RES_ANCHOR).Clone());
                    aFrameSet.Put( SwFormatAnchor( FLY_AT_PAGE, 1 ));
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
                    SfxItemSet aAnchorSet( pDoc->GetAttrPool(), RES_ANCHOR, RES_ANCHOR );
                    aAnchorSet.Put( *pAnchorItem );
                    pDoc->SetFlyFrameAttr( *pFormat, aAnchorSet );
                    delete pAnchorItem;
                }
                DELETEZ( m_pCopySource );
            }
            else
            {
                pFormat = pDoc->MakeFlySection( FLY_AT_PARA, aPam.GetPoint(),
                                         &aFrameSet, pParentFrameFormat );
            }
            if(pFormat)
            {
                pFormat->Add(this);
                if(!m_sName.isEmpty())
                    pDoc->SetFlyName((SwFlyFrameFormat&)*pFormat, m_sName);
            }
            // wake up the SwXTextFrame
            static_cast<SwXTextFrame*>(this)->SetDoc( bIsDescriptor ? m_pDoc : GetFrameFormat()->GetDoc() );
        }
        else if( eType == FLYCNTTYPE_GRF)
        {
            UnoActionContext aCont(pDoc);
            const ::uno::Any* pGraphicURL;
            OUString sGraphicURL;
            GraphicObject *pGrfObj = nullptr;
            if(pProps->GetProperty(FN_UNO_GRAPHIC_U_R_L, 0, pGraphicURL))
            {
                (*pGraphicURL) >>= sGraphicURL;
                if( sGraphicURL.startsWith(sPackageProtocol) )
                {
                    pGrfObj = new GraphicObject;
                    pGrfObj->SetUserData( sGraphicURL );
                    sGraphicURL.clear();
                }
                else if( sGraphicURL.startsWith(sGraphicObjectProtocol) )
                {
                    OString sId(OUStringToOString(
                        sGraphicURL.copy( sizeof(sGraphicObjectProtocol)-1 ),
                        RTL_TEXTENCODING_ASCII_US));
                    pGrfObj = new GraphicObject( sId );
                    sGraphicURL.clear();
                }
            }
            Graphic aGraphic;
            const ::uno::Any* pGraphic;
            if( pProps->GetProperty( FN_UNO_GRAPHIC, 0, pGraphic ))
            {
                uno::Reference< graphic::XGraphic > xGraphic;
                (*pGraphic) >>= xGraphic;
                aGraphic = Graphic( xGraphic );
            }

            OUString sFltName;
            const ::uno::Any* pFilter;
            if(pProps->GetProperty(FN_UNO_GRAPHIC_FILTER, 0, pFilter))
            {
                (*pFilter) >>= sFltName;
            }

            pFormat =
                pGrfObj ? pDoc->getIDocumentContentOperations().Insert( aPam, *pGrfObj, &aFrameSet, &aGrSet,
                                        pParentFrameFormat )
                        : pDoc->getIDocumentContentOperations().Insert( aPam, sGraphicURL, sFltName, &aGraphic,
                                        &aFrameSet, &aGrSet, pParentFrameFormat  );
            delete pGrfObj;
            if(pFormat)
            {
                SwGrfNode *pGrfNd = pDoc->GetNodes()[ pFormat->GetContent().GetContentIdx()
                                            ->GetIndex()+1 ]->GetGrfNode();
                if (pGrfNd)
                    pGrfNd->SetChgTwipSize( !bSizeFound );
                pFormat->Add(this);
                if(!m_sName.isEmpty())
                    pDoc->SetFlyName((SwFlyFrameFormat&)*pFormat, m_sName);

            }
            const ::uno::Any* pSurroundContour;
            if(pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUR, pSurroundContour))
                setPropertyValue(UNO_NAME_SURROUND_CONTOUR, *pSurroundContour);
            const ::uno::Any* pContourOutside;
            if(pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUROUTSIDE, pContourOutside))
                setPropertyValue(UNO_NAME_CONTOUR_OUTSIDE, *pContourOutside);
            const ::uno::Any* pContourPoly;
            if(pProps->GetProperty(FN_PARAM_COUNTOUR_PP, 0, pContourPoly))
                setPropertyValue(UNO_NAME_CONTOUR_POLY_POLYGON, *pContourPoly);
            const ::uno::Any* pPixelContour;
            if(pProps->GetProperty(FN_UNO_IS_PIXEL_CONTOUR, 0, pPixelContour))
                setPropertyValue(UNO_NAME_IS_PIXEL_CONTOUR, *pPixelContour);
            const ::uno::Any* pAutoContour;
            if(pProps->GetProperty(FN_UNO_IS_AUTOMATIC_CONTOUR, 0, pAutoContour))
                setPropertyValue(UNO_NAME_IS_AUTOMATIC_CONTOUR, *pAutoContour);
        }
        else
        {
            const ::uno::Any* pCLSID = nullptr;
            const ::uno::Any* pStreamName = nullptr;
            const ::uno::Any* pEmbeddedObject = nullptr;
            if(!pProps->GetProperty(FN_UNO_CLSID, 0, pCLSID)
                && !pProps->GetProperty( FN_UNO_STREAM_NAME, 0, pStreamName )
                && !pProps->GetProperty( FN_EMBEDDED_OBJECT, 0, pEmbeddedObject ))
                throw uno::RuntimeException();
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
                    xIPObj = pCnt->CreateEmbeddedObject( aClassName.GetByteSequence(), aName );
                }
                if ( xIPObj.is() )
                {
                    UnoActionContext aAction(pDoc);
                    pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, nullptr);
                    if(!bSizeFound)
                    {
                        //TODO/LATER: from where do I get a ViewAspect? And how do I transport it to the OLENode?
                        sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;

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
                            aSz.Width() = aSz.Height() = 5000;
                            aSz = OutputDevice::LogicToLogic
                                                    ( aSz, MapMode( MAP_100TH_MM ), aRefMap );
                        }
                        MapMode aMyMap( MAP_TWIP );
                        aSz = OutputDevice::LogicToLogic( aSz, aRefMap, aMyMap );
                        SwFormatFrameSize aFrameSz;
                        aFrameSz.SetSize(aSz);
                        aFrameSet.Put(aFrameSz);
                    }
                    SwFlyFrameFormat* pFormat2 = nullptr;

                    // TODO/LATER: Is it the only possible aspect here?
                    sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
                    ::svt::EmbeddedObjectRef xObjRef( xIPObj, nAspect );
                    pFormat2 = pDoc->getIDocumentContentOperations().Insert(aPam, xObjRef, &aFrameSet, nullptr, nullptr );
                    assert(pFormat2 && "Doc->Insert(notxt) failed.");

                    pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, nullptr);
                    pFormat2->Add(this);
                    if(!m_sName.isEmpty())
                        pDoc->SetFlyName((SwFlyFrameFormat&)*pFormat2, m_sName);
                }
            }
            else if( pStreamName )
            {
                OUString sStreamName;
                (*pStreamName) >>= sStreamName;
                pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, nullptr);

                SwFlyFrameFormat* pFrameFormat = nullptr;
                pFrameFormat = pDoc->getIDocumentContentOperations().InsertOLE( aPam, sStreamName, embed::Aspects::MSOLE_CONTENT, &aFrameSet, nullptr, nullptr );
                pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, nullptr);
                pFrameFormat->Add(this);
                if(!m_sName.isEmpty())
                    pDoc->SetFlyName((SwFlyFrameFormat&)*pFrameFormat, m_sName);
            }
            else if( pEmbeddedObject || pStreamName )
            {
                uno::Reference< embed::XEmbeddedObject > obj;
                (*pEmbeddedObject) >>= obj;
                svt::EmbeddedObjectRef xObj;
                xObj.Assign( obj, embed::Aspects::MSOLE_CONTENT );

                pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, nullptr);

                // Not sure if these setParent() and InsertEmbeddedObject() calls are really
                // needed, it seems to work without, but logic from code elsewhere suggests
                // they should be done.
                SfxObjectShell& mrPers = *pDoc->GetPersist();
                uno::Reference < container::XChild > xChild( obj, uno::UNO_QUERY );
                if ( xChild.is() )
                    xChild->setParent( mrPers.GetModel() );
                OUString rName;
                mrPers.GetEmbeddedObjectContainer().InsertEmbeddedObject( obj, rName );

                SwFlyFrameFormat* pFrameFormat = nullptr;
                pFrameFormat = pDoc->getIDocumentContentOperations().Insert( aPam, xObj, &aFrameSet, nullptr, nullptr );
                pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, nullptr);
                pFrameFormat->Add(this);
                if(!m_sName.isEmpty())
                    pDoc->SetFlyName((SwFlyFrameFormat&)*pFrameFormat, m_sName);
            }
        }
        if( pFormat && pDoc->getIDocumentDrawModelAccess().GetDrawModel() )
            GetOrCreateSdrObject(*pFormat);
        const ::uno::Any* pOrder;
        if( pProps->GetProperty(FN_UNO_Z_ORDER, 0, pOrder) )
            setPropertyValue(UNO_NAME_Z_ORDER, *pOrder);
        const ::uno::Any* pReplacement;
        if( pProps->GetProperty(FN_UNO_REPLACEMENT_GRAPHIC, 0, pReplacement) )
            setPropertyValue(UNO_NAME_GRAPHIC, *pReplacement);
        // new attribute Title
        const ::uno::Any* pTitle;
        if ( pProps->GetProperty(FN_UNO_TITLE, 0, pTitle) )
        {
            setPropertyValue(UNO_NAME_TITLE, *pTitle);
        }
        // new attribute Description
        const ::uno::Any* pDescription;
        if ( pProps->GetProperty(FN_UNO_DESCRIPTION, 0, pDescription) )
        {
            setPropertyValue(UNO_NAME_DESCRIPTION, *pDescription);
        }

        // For grabbag
        const uno::Any* pFrameIntropgrabbagItem;
        if( pProps->GetProperty(RES_FRMATR_GRABBAG, 0, pFrameIntropgrabbagItem) )
        {
            setPropertyValue(UNO_NAME_FRAME_INTEROP_GRAB_BAG, *pFrameIntropgrabbagItem);
        }
    }
    else
        throw lang::IllegalArgumentException();
    // reset the flag and delete Descriptor pointer
    ResetDescriptor();
}

void SwXFrame::attach(const uno::Reference< text::XTextRange > & xTextRange)
    throw (lang::IllegalArgumentException, uno::RuntimeException, std::exception)
{
    SwFrameFormat* pFormat;
    if(IsDescriptor())
        attachToRange(xTextRange);
    else if(nullptr != (pFormat = GetFrameFormat()))
    {
        uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
        SwDoc* pDoc = pFormat->GetDoc();
        SwUnoInternalPaM aIntPam(*pDoc);
        if (::sw::XTextRangeToSwPaM(aIntPam, xTextRange))
        {
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_ANCHOR, RES_ANCHOR );
            aSet.SetParent(&pFormat->GetAttrSet());
            SwFormatAnchor aAnchor = static_cast<const SwFormatAnchor&>(aSet.Get(RES_ANCHOR));
            aAnchor.SetAnchor( aIntPam.Start() );
            aSet.Put(aAnchor);
            pDoc->SetFlyFrameAttr( *pFormat, aSet );
        }
        else
            throw lang::IllegalArgumentException();
    }
}

awt::Point SwXFrame::getPosition() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::RuntimeException aRuntime;
    aRuntime.Message = "position cannot be determined with this method";
    throw aRuntime;
}

void SwXFrame::setPosition(const awt::Point& /*aPosition*/) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::RuntimeException aRuntime;
    aRuntime.Message = "position cannot be changed with this method";
    throw aRuntime;
}

awt::Size SwXFrame::getSize() throw( uno::RuntimeException, std::exception )
{
    const ::uno::Any aVal = getPropertyValue("Size");
    awt::Size const * pRet =  static_cast<awt::Size const *>(aVal.getValue());
    return *pRet;
}

void SwXFrame::setSize(const awt::Size& aSize)
    throw( beans::PropertyVetoException, uno::RuntimeException, std::exception )
{
    const ::uno::Any aVal(&aSize, ::cppu::UnoType<awt::Size>::get());
    setPropertyValue("Size", aVal);
}

OUString SwXFrame::getShapeType() throw( uno::RuntimeException, std::exception )
{
    return OUString("FrameShape");
}

SwXTextFrame::SwXTextFrame( SwDoc *_pDoc ) :
    SwXText(nullptr, CURSOR_FRAME),
    SwXFrame(FLYCNTTYPE_FRM, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_FRAME), _pDoc )
{
}

SwXTextFrame::SwXTextFrame(SwFrameFormat& rFormat) :
    SwXText(rFormat.GetDoc(), CURSOR_FRAME),
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
    throw (uno::RuntimeException, std::exception)
{
    ::uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == cppu::UnoType<void>::get())
        aRet = SwXText::queryInterface(aType);
    if(aRet.getValueType() == cppu::UnoType<void>::get())
        aRet = SwXTextFrameBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL SwXTextFrame::getTypes(  ) throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aTextFrameTypes = SwXTextFrameBaseClass::getTypes();
    uno::Sequence< uno::Type > aFrameTypes = SwXFrame::getTypes();
    uno::Sequence< uno::Type > aTextTypes = SwXText::getTypes();

    long nIndex = aTextFrameTypes.getLength();
    aTextFrameTypes.realloc(
        aTextFrameTypes.getLength() +
        aFrameTypes.getLength() +
        aTextTypes.getLength());

    uno::Type* pTextFrameTypes = aTextFrameTypes.getArray();
    const uno::Type* pFrameTypes = aFrameTypes.getConstArray();
    long nPos;
    for(nPos = 0; nPos <aFrameTypes.getLength(); nPos++)
        pTextFrameTypes[nIndex++] = pFrameTypes[nPos];

    const uno::Type* pTextTypes = aTextTypes.getConstArray();
    for(nPos = 0; nPos <aTextTypes.getLength(); nPos++)
        pTextFrameTypes[nIndex++] = pTextTypes[nPos];

    return aTextFrameTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwXTextFrame::getImplementationId(  ) throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

uno::Reference< text::XText >  SwXTextFrame::getText() throw( uno::RuntimeException, std::exception )
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
SwXTextFrame::CreateCursor() throw (uno::RuntimeException)
{
    return createTextCursor();
}

uno::Reference< text::XTextCursor >  SwXTextFrame::createTextCursor() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextCursor >  aRef;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(pFormat)
    {
        //save current start node to be able to check if there is content after the table -
        //otherwise the cursor would be in the body text!
        const SwNode& rNode = pFormat->GetContent().GetContentIdx()->GetNode();
        const SwStartNode* pOwnStartNode = rNode.FindSttNodeByType(SwFlyStartNode);

        SwPaM aPam(rNode);
        aPam.Move(fnMoveForward, fnGoNode);
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
                 *pFormat->GetDoc(), this, CURSOR_FRAME, *aPam.GetPoint());
        aRef =  static_cast<text::XWordCursor*>(pXCursor);
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

uno::Reference< text::XTextCursor >  SwXTextFrame::createTextCursorByRange(const uno::Reference< text::XTextRange > & aTextPosition) throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextCursor >  aRef;
    SwFrameFormat* pFormat = GetFrameFormat();
    SwUnoInternalPaM aPam(*GetDoc());
    if (pFormat && ::sw::XTextRangeToSwPaM(aPam, aTextPosition))
    {
        SwNode& rNode = pFormat->GetContent().GetContentIdx()->GetNode();
        if(aPam.GetNode().FindFlyStartNode() == rNode.FindFlyStartNode())
        {
            aRef = static_cast<text::XWordCursor*>(
                    new SwXTextCursor(*pFormat->GetDoc(), this, CURSOR_FRAME,
                        *aPam.GetPoint(), aPam.GetMark()));
        }
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

uno::Reference< container::XEnumeration >  SwXTextFrame::createEnumeration() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwFrameFormat* pFormat = GetFrameFormat();
    if(!pFormat)
        return nullptr;
    SwPosition aPos(pFormat->GetContent().GetContentIdx()->GetNode());
    auto pUnoCursor(GetDoc()->CreateUnoCursor(aPos));
    pUnoCursor->Move(fnMoveForward, fnGoNode);
    return SwXParagraphEnumeration::Create(this, pUnoCursor, CURSOR_FRAME);
}

uno::Type  SwXTextFrame::getElementType() throw( uno::RuntimeException, std::exception )
{
    return cppu::UnoType<text::XTextRange>::get();
}

sal_Bool SwXTextFrame::hasElements() throw( uno::RuntimeException, std::exception )
{
    return sal_True;
}

void SwXTextFrame::attach(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException, std::exception )
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< text::XTextRange >  SwXTextFrame::getAnchor() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextFrame::dispose() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextFrame::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException, std::exception )
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextFrame::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException, std::exception )
{
    SwXFrame::removeEventListener(aListener);
}

OUString SwXTextFrame::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextFrame");
}

sal_Bool SwXTextFrame::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextFrame::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 2);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 2] = "com.sun.star.text.TextFrame";
    pArray[aRet.getLength() - 1] = "com.sun.star.text.Text";
    return aRet;
}

void * SAL_CALL SwXTextFrame::operator new( size_t t) throw()
{
    return SwXTextFrameBaseClass::operator new( t);
}

void SAL_CALL SwXTextFrame::operator delete( void * p) throw()
{
    SwXTextFrameBaseClass::operator delete(p);
}

uno::Reference<container::XNameReplace > SAL_CALL SwXTextFrame::getEvents()
    throw(uno::RuntimeException, std::exception)
{
    return new SwFrameEventDescriptor( *this );
}

sal_Int64 SAL_CALL SwXTextFrame::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException, std::exception)
{
    sal_Int64 nRet = SwXFrame::getSomething( rId );
    if( !nRet )
        nRet = SwXText::getSomething( rId );

    return nRet;
}

::uno::Any SwXTextFrame::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException, std::exception )
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

SwXTextGraphicObject::SwXTextGraphicObject( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_GRF, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_GRAPHIC), pDoc)
{
}

SwXTextGraphicObject::SwXTextGraphicObject(SwFrameFormat& rFormat) :
    SwXFrame(rFormat, FLYCNTTYPE_GRF, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_GRAPHIC))
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

void SAL_CALL SwXTextGraphicObject::acquire(  )throw()
{
    SwXFrame::acquire();
}

void SAL_CALL SwXTextGraphicObject::release(  )throw()
{
    SwXFrame::release();
}

::uno::Any SAL_CALL SwXTextGraphicObject::queryInterface( const uno::Type& aType )
    throw(uno::RuntimeException, std::exception)
{
    ::uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == cppu::UnoType<void>::get())
        aRet = SwXTextGraphicObjectBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL
    SwXTextGraphicObject::getTypes(  ) throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aGraphicTypes = SwXTextGraphicObjectBaseClass::getTypes();
    uno::Sequence< uno::Type > aFrameTypes = SwXFrame::getTypes();

    long nIndex = aGraphicTypes.getLength();
    aGraphicTypes.realloc(
        aGraphicTypes.getLength() +
        aFrameTypes.getLength());

    uno::Type* pGraphicTypes = aGraphicTypes.getArray();
    const uno::Type* pFrameTypes = aFrameTypes.getConstArray();
    long nPos;
    for(nPos = 0; nPos <aFrameTypes.getLength(); nPos++)
        pGraphicTypes[nIndex++] = pFrameTypes[nPos];

    return aGraphicTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwXTextGraphicObject::getImplementationId(  ) throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

void SwXTextGraphicObject::attach(const uno::Reference< text::XTextRange > & xTextRange) throw( lang::IllegalArgumentException, uno::RuntimeException, std::exception )
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< text::XTextRange >  SwXTextGraphicObject::getAnchor() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextGraphicObject::dispose() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextGraphicObject::addEventListener(const uno::Reference< lang::XEventListener > & aListener)
                                                    throw( uno::RuntimeException, std::exception )
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextGraphicObject::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
                                                    throw( uno::RuntimeException, std::exception )
{
    SwXFrame::removeEventListener(aListener);
}

OUString SwXTextGraphicObject::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextGraphicObject");
}

sal_Bool SwXTextGraphicObject::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextGraphicObject::getSupportedServiceNames()
        throw( uno::RuntimeException, std::exception )
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 1);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 1] = "com.sun.star.text.TextGraphicObject";
    return aRet;
}

void * SAL_CALL SwXTextGraphicObject::operator new( size_t t) throw()
{
    return SwXTextGraphicObjectBaseClass::operator new(t);
}

void SAL_CALL SwXTextGraphicObject::operator delete( void * p) throw()
{
    SwXTextGraphicObjectBaseClass::operator delete(p);
}

uno::Reference<container::XNameReplace> SAL_CALL
    SwXTextGraphicObject::getEvents()
        throw(uno::RuntimeException, std::exception)
{
    return new SwFrameEventDescriptor( *this );
}

SwXTextEmbeddedObject::SwXTextEmbeddedObject( SwDoc *pDoc )
    : SwXFrame(FLYCNTTYPE_OLE, aSwMapProvider.GetPropertySet(PROPERTY_MAP_EMBEDDED_OBJECT), pDoc)
    , m_xOLEListener(nullptr)
{ }

SwXTextEmbeddedObject::SwXTextEmbeddedObject(SwFrameFormat& rFormat) :
    SwXFrame(rFormat, FLYCNTTYPE_OLE, aSwMapProvider.GetPropertySet(PROPERTY_MAP_EMBEDDED_OBJECT))
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

void SAL_CALL SwXTextEmbeddedObject::acquire()throw()
{
    SwXFrame::acquire();
}

void SAL_CALL SwXTextEmbeddedObject::release()throw()
{
    SwXFrame::release();
}

::uno::Any SAL_CALL SwXTextEmbeddedObject::queryInterface( const uno::Type& aType )
    throw( uno::RuntimeException, std::exception)
{
    ::uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == cppu::UnoType<void>::get())
        aRet = SwXTextEmbeddedObjectBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL SwXTextEmbeddedObject::getTypes(  ) throw(uno::RuntimeException, std::exception)
{
    uno::Sequence< uno::Type > aTextEmbeddedTypes = SwXTextEmbeddedObjectBaseClass::getTypes();
    uno::Sequence< uno::Type > aFrameTypes = SwXFrame::getTypes();

    long nIndex = aTextEmbeddedTypes.getLength();
    aTextEmbeddedTypes.realloc(
        aTextEmbeddedTypes.getLength() +
        aFrameTypes.getLength());

    uno::Type* pTextEmbeddedTypes = aTextEmbeddedTypes.getArray();

    const uno::Type* pFrameTypes = aFrameTypes.getConstArray();
    long nPos;
    for(nPos = 0; nPos <aFrameTypes.getLength(); nPos++)
        pTextEmbeddedTypes[nIndex++] = pFrameTypes[nPos];

    return aTextEmbeddedTypes;
}

uno::Sequence< sal_Int8 > SAL_CALL SwXTextEmbeddedObject::getImplementationId(  ) throw(uno::RuntimeException, std::exception)
{
    return css::uno::Sequence<sal_Int8>();
}

void SwXTextEmbeddedObject::attach(const uno::Reference< text::XTextRange > & xTextRange) throw( lang::IllegalArgumentException, uno::RuntimeException, std::exception )
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< text::XTextRange >  SwXTextEmbeddedObject::getAnchor() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextEmbeddedObject::dispose() throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextEmbeddedObject::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException, std::exception )
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextEmbeddedObject::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException, std::exception )
{
    SwXFrame::removeEventListener(aListener);
}

uno::Reference< lang::XComponent >  SwXTextEmbeddedObject::getEmbeddedObject() throw( uno::RuntimeException, std::exception )
{
    uno::Reference<embed::XEmbeddedObject> xObj(getExtendedControlOverEmbeddedObject());
    return xObj.is() ? uno::Reference<lang::XComponent>(xObj->getComponent(), uno::UNO_QUERY) : nullptr;
}

uno::Reference< embed::XEmbeddedObject > SAL_CALL SwXTextEmbeddedObject::getExtendedControlOverEmbeddedObject()
        throw( uno::RuntimeException, std::exception )
{
    uno::Reference< embed::XEmbeddedObject > xResult;
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(pFormat)
    {
        SwDoc* pDoc = pFormat->GetDoc();
        const SwFormatContent* pCnt = &pFormat->GetContent();
        OSL_ENSURE( pCnt->GetContentIdx() &&
                       pDoc->GetNodes()[ pCnt->GetContentIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

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

sal_Int64 SAL_CALL SwXTextEmbeddedObject::getAspect() throw (uno::RuntimeException, std::exception)
{
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(pFormat)
    {
        SwDoc* pDoc = pFormat->GetDoc();
        const SwFormatContent* pCnt = &pFormat->GetContent();
        OSL_ENSURE( pCnt->GetContentIdx() &&
                       pDoc->GetNodes()[ pCnt->GetContentIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        return pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ]->GetOLENode()->GetAspect();
    }

    return embed::Aspects::MSOLE_CONTENT; // return the default value
}

void SAL_CALL SwXTextEmbeddedObject::setAspect( sal_Int64 nAspect ) throw (uno::RuntimeException, std::exception)
{
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(pFormat)
    {
        SwDoc* pDoc = pFormat->GetDoc();
        const SwFormatContent* pCnt = &pFormat->GetContent();
        OSL_ENSURE( pCnt->GetContentIdx() &&
                       pDoc->GetNodes()[ pCnt->GetContentIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ]->GetOLENode()->SetAspect( nAspect );
    }
}

uno::Reference< graphic::XGraphic > SAL_CALL SwXTextEmbeddedObject::getReplacementGraphic() throw (uno::RuntimeException, std::exception)
{
    SwFrameFormat*   pFormat = GetFrameFormat();
    if(pFormat)
    {
        SwDoc* pDoc = pFormat->GetDoc();
        const SwFormatContent* pCnt = &pFormat->GetContent();
        OSL_ENSURE( pCnt->GetContentIdx() &&
                       pDoc->GetNodes()[ pCnt->GetContentIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        const Graphic* pGraphic = pDoc->GetNodes()[ pCnt->GetContentIdx()->GetIndex() + 1 ]->GetOLENode()->GetGraphic();
        if ( pGraphic )
            return pGraphic->GetXGraphic();
    }

    return uno::Reference< graphic::XGraphic >();
}

OUString SwXTextEmbeddedObject::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("SwXTextEmbeddedObject");
}

sal_Bool SwXTextEmbeddedObject::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SwXTextEmbeddedObject::getSupportedServiceNames()
        throw( uno::RuntimeException, std::exception )
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 1);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 1] = "com.sun.star.text.TextEmbeddedObject";
    return aRet;
}

void * SAL_CALL SwXTextEmbeddedObject::operator new( size_t t) throw()
{
    return SwXTextEmbeddedObjectBaseClass::operator new(t);
}

void SAL_CALL SwXTextEmbeddedObject::operator delete( void * p) throw()
{
    SwXTextEmbeddedObjectBaseClass::operator delete(p);
}

uno::Reference<container::XNameReplace> SAL_CALL
    SwXTextEmbeddedObject::getEvents()
        throw(uno::RuntimeException, std::exception)
{
    return new SwFrameEventDescriptor( *this );
}


SwXOLEListener::SwXOLEListener( SwFormat& rOLEFormat, uno::Reference< XModel > xOLE) :
    SwClient(&rOLEFormat),
    xOLEModel(xOLE)
{
}

SwXOLEListener::~SwXOLEListener()
{}

void SwXOLEListener::modified( const lang::EventObject& /*rEvent*/ )
                                        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    SwOLENode* pNd = nullptr;
    SwFormat* pFormat = GetFormat();
    if(pFormat)
    {const SwNodeIndex* pIdx = pFormat->GetContent().GetContentIdx();
        if(pIdx)
        {
            SwNodeIndex aIdx(*pIdx, 1);
            SwNoTextNode* pNoText = aIdx.GetNode().GetNoTextNode();
            pNd = pNoText->GetOLENode();
        }
    }
    if(!pNd)
        throw uno::RuntimeException();

    uno::Reference < embed::XEmbeddedObject > xIP = pNd->GetOLEObj().GetOleRef();
    if ( xIP.is() )
    {
        sal_Int32 nState = xIP->getCurrentState();
        if ( nState == embed::EmbedStates::INPLACE_ACTIVE || nState == embed::EmbedStates::UI_ACTIVE )
            return;
    }

    // if the OLE-Node is UI-Active do nothing
    pNd->SetOLESizeInvalid(true);
    pNd->GetDoc()->SetOLEObjModified();
}

void SwXOLEListener::disposing( const lang::EventObject& rEvent )
                        throw( uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;

    uno::Reference< util::XModifyListener >  xListener( this );

    uno::Reference< frame::XModel >  xModel( rEvent.Source, uno::UNO_QUERY );
    uno::Reference< util::XModifyBroadcaster >  xBrdcst(xModel, uno::UNO_QUERY);

    try
    {
        if( xBrdcst.is() )
            xBrdcst->removeModifyListener( xListener );
    }
    catch(uno::Exception const &)
    {
        OSL_FAIL("OLE Listener couldn't be removed");
    }
}

void SwXOLEListener::Modify( const SfxPoolItem* pOld, const SfxPoolItem* pNew )
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        xOLEModel = nullptr;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
