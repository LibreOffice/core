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
#include <com/sun/star/drawing/FillStyle.hpp>
#include <com/sun/star/awt/Gradient.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <svx/svxids.hrc>
#include <svx/xfillit0.hxx>
#include <svx/xflgrit.hxx>
#include <editeng/memberids.hrc>

#include <swtypes.hxx>
#include <cmdid.h>

#include <memory>
#include <hints.hxx>
#include <doc.hxx>
#include <IDocumentUndoRedo.hxx>
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
#include <switerator.hxx>
#include <comphelper/servicehelper.hxx>

// from fefly1.cxx
extern sal_Bool sw_ChkAndSetNewAnchor( SwEditShell& rEditShell, const SwFlyFrm& rFly, SfxItemSet& rSet );

using namespace ::com::sun::star;

using ::com::sun::star::frame::XModel;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::style::XStyleFamiliesSupplier;

const sal_Char sPackageProtocol[] = "vnd.sun.star.Package:";
const sal_Char sGraphicObjectProtocol[] = "vnd.sun.star.GraphicObject:";

/****************************************************************************
    Rahmenbeschreibung
****************************************************************************/
class BaseFrameProperties_Impl
{
    SwUnoCursorHelper::SwAnyMapHelper aAnyMap;

public:
    virtual ~BaseFrameProperties_Impl();

    void            SetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any& rVal);
    bool            GetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any*& pAny );
    bool FillBaseProperties(SwDoc* pDoc, SfxItemSet& rToSet, const SfxItemSet &rFromSet, bool& rSizeFound);

    virtual bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, bool& rSizeFound) = 0;

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

bool BaseFrameProperties_Impl::FillBaseProperties(SwDoc* pDoc, SfxItemSet& rToSet, const SfxItemSet& rFromSet, bool& rSizeFound)
{
    bool bRet = true;
    //Anker kommt auf jeden Fall in den Set
    SwFmtAnchor aAnchor ( static_cast < const SwFmtAnchor & > ( rFromSet.Get ( RES_ANCHOR ) ) );
    {
        const ::uno::Any* pAnchorPgNo;
        if(GetProperty(RES_ANCHOR, MID_ANCHOR_PAGENUM, pAnchorPgNo))
            bRet &= ((SfxPoolItem&)aAnchor).PutValue(*pAnchorPgNo, MID_ANCHOR_PAGENUM);
        const ::uno::Any* pAnchorType;
        if(GetProperty(RES_ANCHOR, MID_ANCHOR_ANCHORTYPE, pAnchorType))
            bRet &= ((SfxPoolItem&)aAnchor).PutValue(*pAnchorType, MID_ANCHOR_ANCHORTYPE);
    }
    rToSet.Put(aAnchor);
    {
        const ::uno::Any* pCol = 0;
        GetProperty(RES_BACKGROUND, MID_BACK_COLOR, pCol );
        const ::uno::Any* pRGBCol = 0;
        GetProperty(RES_BACKGROUND, MID_BACK_COLOR_R_G_B, pRGBCol );
        const ::uno::Any* pColTrans = 0;
        GetProperty(RES_BACKGROUND, MID_BACK_COLOR_TRANSPARENCY, pColTrans);
        const ::uno::Any* pTrans = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_TRANSPARENT, pTrans );
        const ::uno::Any* pGrLoc = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_POSITION, pGrLoc );
        const ::uno::Any* pGrURL = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_URL, pGrURL     );
        const ::uno::Any* pGrFilter = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_FILTER, pGrFilter     );
        const ::uno::Any* pGrTranparency = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_TRANSPARENCY, pGrTranparency     );

        if(pCol || pTrans || pGrURL || pGrFilter || pGrLoc ||
                            pGrTranparency || pColTrans || pRGBCol)
        {
            SvxBrushItem aBrush ( static_cast < const :: SvxBrushItem & > ( rFromSet.Get ( RES_BACKGROUND ) ) );
            if(pCol )
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pCol,MID_BACK_COLOR    );
            if(pColTrans)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pColTrans, MID_BACK_COLOR_TRANSPARENCY);
            if(pRGBCol)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pRGBCol, MID_BACK_COLOR_R_G_B);
            if(pTrans)
            {
                // don't overwrite transparency with a non-transparence flag
                if(!pColTrans || Any2Bool( *pTrans ))
                    bRet &= ((SfxPoolItem&)aBrush).PutValue(*pTrans, MID_GRAPHIC_TRANSPARENT);
            }
            if(pGrURL)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrURL, MID_GRAPHIC_URL);
            if(pGrFilter)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrFilter, MID_GRAPHIC_FILTER);
            if(pGrLoc)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrLoc, MID_GRAPHIC_POSITION);
            if(pGrTranparency)
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pGrTranparency, MID_GRAPHIC_TRANSPARENCY);

            rToSet.Put(aBrush);
        }
    }
    {
        const ::uno::Any* pFillStyle = 0;
        GetProperty(RES_FILL_STYLE, 0, pFillStyle);
        if (pFillStyle)
        {
            XFillStyleItem aFillStyle( static_cast <const :: XFillStyleItem & > ( rFromSet.Get ( RES_FILL_STYLE ) ) );
            bRet &= ((SfxPoolItem&)aFillStyle).PutValue(*pFillStyle);
            rToSet.Put(aFillStyle);
        }
    }
    {
        const ::uno::Any* pFillGradient = 0;
        GetProperty(RES_FILL_GRADIENT, MID_FILLGRADIENT, pFillGradient);
        const ::uno::Any* pName = 0;
        GetProperty(RES_FILL_GRADIENT, MID_NAME, pName);
        if (pFillGradient || pName)
        {
            XFillGradientItem aFillGradient( static_cast <const :: XFillGradientItem & > ( rFromSet.Get ( RES_FILL_GRADIENT ) ) );
            XFillGradientItem* pItem = &aFillGradient;
            if (pFillGradient)
            {
                bRet &= ((SfxPoolItem*)pItem)->PutValue(*pFillGradient, MID_FILLGRADIENT);
                // If gradient is set directly, we always generate an associated style name for it.
                SdrModel* pModel = pDoc->GetDrawModel();
                pItem = pItem->checkForUniqueItem( pModel );
            }
            if (pName)
            {
                bRet &= ((SfxPoolItem*)pItem)->PutValue(*pName, MID_NAME);
                // Look up the associated style name.
                SfxItemPool& rPool = pDoc->GetDrawModel()->GetItemPool();
                const sal_uInt32 nCount = rPool.GetItemCount2(XATTR_FILLGRADIENT);
                const XFillGradientItem* pStyleItem;
                for (sal_uInt32 i = 0; i < nCount; ++i)
                {
                    pStyleItem = (XFillGradientItem*)rPool.GetItem2(XATTR_FILLGRADIENT, i);
                    if (pStyleItem && pStyleItem->GetName() == pItem->GetName())
                    {
                        pItem->SetGradientValue(pStyleItem->GetGradientValue());
                        break;
                    }
                }
            }
            if (pItem)
            {
                rToSet.Put(*pItem);
                if(pItem != &aFillGradient)
                {
                    // New name was generated? Then insert it to the drawinglayer style table.
                    uno::Reference<frame::XModel> xModel(pDoc->GetDocShell()->GetModel());
                    uno::Reference<lang::XMultiServiceFactory> xServiceFact(xModel, uno::UNO_QUERY);
                    uno::Reference< container::XNameContainer > xGradients(xServiceFact->createInstance("com.sun.star.drawing.GradientTable"), uno::UNO_QUERY);
                    if (!xGradients->hasByName(pItem->GetName()))
                    {
                        xGradients->insertByName(pItem->GetName(), *pFillGradient);
                    }

                    delete pItem;
                }
            }
        }
    }
    {
        const ::uno::Any* pCont = 0;
        GetProperty(RES_PROTECT, MID_PROTECT_CONTENT, pCont );
        const ::uno::Any* pPos = 0;
        GetProperty(RES_PROTECT,MID_PROTECT_POSITION, pPos );
        const ::uno::Any* pName = 0;
        GetProperty(RES_PROTECT, MID_PROTECT_SIZE, pName );
        if(pCont||pPos||pName)
        {
            SvxProtectItem aProt ( static_cast < const :: SvxProtectItem & > ( rFromSet.Get ( RES_PROTECT ) ) );
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
        const ::uno::Any* pHori  = 0;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_ORIENT, pHori );
        const ::uno::Any* pHoriP = 0;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_POSITION|CONVERT_TWIPS, pHoriP );
        const ::uno::Any* pHoriR = 0;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_RELATION, pHoriR );
        const ::uno::Any* pPageT = 0;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_PAGETOGGLE, pPageT);
        if(pHori||pHoriP||pHoriR||pPageT)
        {
            SwFmtHoriOrient aOrient ( static_cast < const :: SwFmtHoriOrient & > ( rFromSet.Get ( RES_HORI_ORIENT ) ) );
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
        const ::uno::Any* pVert  = 0;
        GetProperty(RES_VERT_ORIENT, MID_VERTORIENT_ORIENT, pVert);
        const ::uno::Any* pVertP = 0;
        GetProperty(RES_VERT_ORIENT, MID_VERTORIENT_POSITION|CONVERT_TWIPS, pVertP );
        const ::uno::Any* pVertR = 0;
        GetProperty(RES_VERT_ORIENT, MID_VERTORIENT_RELATION, pVertR );
        if(pVert||pVertP||pVertR)
        {
            SwFmtVertOrient aOrient ( static_cast < const :: SwFmtVertOrient & > ( rFromSet.Get ( RES_VERT_ORIENT ) ) );
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
        const ::uno::Any* pURL = 0;
        GetProperty(RES_URL, MID_URL_URL, pURL );
        const ::uno::Any* pTarget = 0;
        GetProperty(RES_URL, MID_URL_TARGET, pTarget );
        const ::uno::Any* pHyLNm = 0;
        GetProperty(RES_URL, MID_URL_HYPERLINKNAME, pHyLNm );
        const ::uno::Any* pHySMp = 0;
        GetProperty(RES_URL, MID_URL_SERVERMAP, pHySMp );
        if(pURL||pTarget||pHyLNm||pHySMp)
        {
            SwFmtURL aURL ( static_cast < const :: SwFmtURL & > ( rFromSet.Get ( RES_URL ) ) );
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
    const ::uno::Any* pL = 0;
    GetProperty(RES_LR_SPACE, MID_L_MARGIN|CONVERT_TWIPS, pL );
    const ::uno::Any* pR = 0;
    GetProperty(RES_LR_SPACE, MID_R_MARGIN|CONVERT_TWIPS, pR );
    if(pL||pR)
    {
        SvxLRSpaceItem aLR ( static_cast < const :: SvxLRSpaceItem & > ( rFromSet.Get ( RES_LR_SPACE ) ) );
        if(pL)
            bRet &= ((SfxPoolItem&)aLR).PutValue(*pL, MID_L_MARGIN|CONVERT_TWIPS);
        if(pR)
            bRet &= ((SfxPoolItem&)aLR).PutValue(*pR, MID_R_MARGIN|CONVERT_TWIPS);
        rToSet.Put(aLR);
    }
    const ::uno::Any* pT = 0;
    GetProperty(RES_UL_SPACE, MID_UP_MARGIN|CONVERT_TWIPS, pT );
    const ::uno::Any* pB = 0;
    GetProperty(RES_UL_SPACE, MID_LO_MARGIN|CONVERT_TWIPS, pB );
    if(pT||pB)
    {
        SvxULSpaceItem aTB ( static_cast < const :: SvxULSpaceItem &> ( rFromSet.Get ( RES_UL_SPACE ) ) );
        if(pT)
            bRet &= ((SfxPoolItem&)aTB).PutValue(*pT, MID_UP_MARGIN|CONVERT_TWIPS);
        if(pB)
            bRet &= ((SfxPoolItem&)aTB).PutValue(*pB, MID_LO_MARGIN|CONVERT_TWIPS);
        rToSet.Put(aTB);
    }
    const ::uno::Any* pOp;
    if(GetProperty(RES_OPAQUE, 0, pOp))
    {
        SvxOpaqueItem aOp ( static_cast < const :: SvxOpaqueItem& > ( rFromSet.Get ( RES_OPAQUE ) ) );
        bRet &= ((SfxPoolItem&)aOp).PutValue(*pOp, 0);
        rToSet.Put(aOp);
    }
    const ::uno::Any* pPrt;
    if(GetProperty(RES_PRINT, 0, pPrt))
    {
        SvxPrintItem aPrt ( static_cast < const :: SvxPrintItem & > ( rFromSet.Get ( RES_PRINT ) ) );
        bRet &= ((SfxPoolItem&)aPrt).PutValue(*pPrt, 0);
        rToSet.Put(aPrt);
    }
    const ::uno::Any* pSh;
    if(GetProperty(RES_SHADOW, CONVERT_TWIPS, pSh))
    {
        SvxShadowItem aSh ( static_cast < const :: SvxShadowItem& > ( rFromSet.Get ( RES_SHADOW ) ) );
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
    const ::uno::Any* pSur      = 0;
    GetProperty(RES_SURROUND, MID_SURROUND_SURROUNDTYPE, pSur);
    const ::uno::Any* pSurAnch = 0;
    GetProperty(RES_SURROUND, MID_SURROUND_ANCHORONLY, pSurAnch);
    if(pSur || pSurAnch)
    {
        SwFmtSurround aSrnd ( static_cast < const :: SwFmtSurround & > ( rFromSet.Get ( RES_SURROUND ) ) );
        if(pSur)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSur, MID_SURROUND_SURROUNDTYPE );
        if(pSurAnch)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSurAnch, MID_SURROUND_ANCHORONLY);
        rToSet.Put(aSrnd);
    }
    const ::uno::Any* pLeft        = 0;
    GetProperty(RES_BOX, LEFT_BORDER  |CONVERT_TWIPS,    pLeft  );
    const ::uno::Any* pRight       = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|RIGHT_BORDER ,    pRight );
    const ::uno::Any* pTop         = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|TOP_BORDER     , pTop   );
    const ::uno::Any* pBottom  = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|BOTTOM_BORDER,    pBottom);
    const ::uno::Any* pDistance    = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|BORDER_DISTANCE,  pDistance);
    const ::uno::Any* pLeftDistance    = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|LEFT_BORDER_DISTANCE, pLeftDistance);
    const ::uno::Any* pRightDistance   = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|RIGHT_BORDER_DISTANCE,    pRightDistance);
    const ::uno::Any* pTopDistance     = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|TOP_BORDER_DISTANCE,  pTopDistance);
    const ::uno::Any* pBottomDistance  = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|BOTTOM_BORDER_DISTANCE,   pBottomDistance);
    const ::uno::Any* pLineStyle  = 0;
    GetProperty(RES_BOX, LINE_STYLE,   pLineStyle);
    const ::uno::Any* pLineWidth  = 0;
    GetProperty(RES_BOX, LINE_WIDTH,   pLineWidth);
    if( pLeft || pRight || pTop ||  pBottom || pDistance ||
        pLeftDistance  || pRightDistance || pTopDistance || pBottomDistance ||
        pLineStyle || pLineWidth )
    {
        SvxBoxItem aBox ( static_cast < const :: SvxBoxItem & > ( rFromSet.Get ( RES_BOX ) ) );
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
        const ::uno::Any* pRelH = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_REL_HEIGHT, pRelH);
        const ::uno::Any* pRelW = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_REL_WIDTH, pRelW);
        const ::uno::Any* pSyncWidth = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT, pSyncWidth);
        const ::uno::Any* pSyncHeight = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH, pSyncHeight);
        const ::uno::Any* pWidth = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS, pWidth);
        const ::uno::Any* pHeight = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS, pHeight);
        const ::uno::Any* pSize = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_SIZE|CONVERT_TWIPS, pSize);
        const ::uno::Any* pSizeType = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_SIZE_TYPE, pSizeType);
        const ::uno::Any* pWidthType = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_WIDTH_TYPE, pWidthType);
        if( pWidth || pHeight ||pRelH || pRelW || pSize ||pSizeType ||
            pWidthType ||pSyncWidth || pSyncHeight )
        {
            rSizeFound = true;
            SwFmtFrmSize aFrmSz ( static_cast < const :: SwFmtFrmSize& > ( rFromSet.Get ( RES_FRM_SIZE ) ) );
            if(pWidth)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pWidth, MID_FRMSIZE_WIDTH|CONVERT_TWIPS);
            if(pHeight)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pHeight, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS);
            if(pRelH )
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pRelH, MID_FRMSIZE_REL_HEIGHT);
            if(pRelW )
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pRelW, MID_FRMSIZE_REL_WIDTH);
            if(pSyncWidth)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pSyncWidth, MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT);
            if(pSyncHeight)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pSyncHeight, MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH);
            if(pSize)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pSize, MID_FRMSIZE_SIZE|CONVERT_TWIPS);
            if(pSizeType)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pSizeType, MID_FRMSIZE_SIZE_TYPE);
            if(pWidthType)
                bRet &= ((SfxPoolItem&)aFrmSz).PutValue(*pWidthType, MID_FRMSIZE_WIDTH_TYPE);
            if(!aFrmSz.GetWidth())
                aFrmSz.SetWidth(MINFLY);
            if(!aFrmSz.GetHeight())
                aFrmSz.SetHeight(MINFLY);
            rToSet.Put(aFrmSz);
        }
        else
        {
            rSizeFound = false;
            SwFmtFrmSize aFrmSz;
            awt::Size aSize;
            aSize.Width = 2 * MM50;
            aSize.Height = 2 * MM50;
            ::uno::Any aSizeVal;
            aSizeVal <<= aSize;
            ((SfxPoolItem&)aFrmSz).PutValue(aSizeVal, MID_FRMSIZE_SIZE|CONVERT_TWIPS);
            rToSet.Put(aFrmSz);
        }
    }
    const ::uno::Any* pFrameDirection = 0;
    GetProperty(RES_FRAMEDIR, 0, pFrameDirection);
    if(pFrameDirection)
    {
        SvxFrameDirectionItem aAttr(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR);
        aAttr.PutValue(*pFrameDirection, 0);
        rToSet.Put(aAttr);
    }
    const ::uno::Any* pUnknown = 0;
    GetProperty(RES_UNKNOWNATR_CONTAINER, 0, pUnknown);
    if(pUnknown)
    {
        SvXMLAttrContainerItem aAttr(RES_UNKNOWNATR_CONTAINER);
        aAttr.PutValue(*pUnknown, 0);
        rToSet.Put(aAttr);
    }

    // #i18732#
    const ::uno::Any* pFollowTextFlow = 0;
    GetProperty(RES_FOLLOW_TEXT_FLOW, 0, pFollowTextFlow);
    if ( pFollowTextFlow )
    {
        SwFmtFollowTextFlow aFmtFollowTextFlow;
        aFmtFollowTextFlow.PutValue(*pFollowTextFlow, 0);
        rToSet.Put(aFmtFollowTextFlow);
    }

    // #i28701# - RES_WRAP_INFLUENCE_ON_OBJPOS
    const ::uno::Any* pWrapInfluenceOnObjPos = 0;
    GetProperty(RES_WRAP_INFLUENCE_ON_OBJPOS, MID_WRAP_INFLUENCE, pWrapInfluenceOnObjPos);
    if ( pWrapInfluenceOnObjPos )
    {
        SwFmtWrapInfluenceOnObjPos aFmtWrapInfluenceOnObjPos;
        aFmtWrapInfluenceOnObjPos.PutValue( *pWrapInfluenceOnObjPos, MID_WRAP_INFLUENCE );
        rToSet.Put(aFmtWrapInfluenceOnObjPos);
    }

    return bRet;
}

class SwFrameProperties_Impl : public BaseFrameProperties_Impl
{
public:
    SwFrameProperties_Impl();
    virtual ~SwFrameProperties_Impl(){}

    bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, bool& rSizeFound);
};

SwFrameProperties_Impl::SwFrameProperties_Impl():
    BaseFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME)*/ )
{
}

static inline void lcl_FillCol ( SfxItemSet &rToSet, const :: SfxItemSet &rFromSet, const :: uno::Any *pAny)
{
    if ( pAny )
    {
        SwFmtCol aCol ( static_cast < const :: SwFmtCol & > ( rFromSet.Get ( RES_COL ) ) );
        ((SfxPoolItem&)aCol).PutValue( *pAny, MID_COLUMNS);
        rToSet.Put(aCol);
    }
}

bool SwFrameProperties_Impl::AnyToItemSet(SwDoc *pDoc, SfxItemSet& rSet, SfxItemSet&, bool& rSizeFound)
{
    //Properties fuer alle Frames
    const ::uno::Any *pStyleName;
    SwDocStyleSheet* pStyle = NULL;
    bool bRet;

    if ( GetProperty ( FN_UNO_FRAME_STYLE_NAME, 0, pStyleName ) )
    {
        OUString sStyle;
        *pStyleName >>= sStyle;
        pStyle = (SwDocStyleSheet*)pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle,
                                                    SFX_STYLE_FAMILY_FRAME);
    }

    const ::uno::Any* pColumns = NULL;
    GetProperty (RES_COL, MID_COLUMNS, pColumns);
    if ( pStyle )
    {
        rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet( *pStyle ) );
        const :: SfxItemSet *pItemSet = &xStyle->GetItemSet();
           bRet = FillBaseProperties( pDoc, rSet, *pItemSet, rSizeFound );
        lcl_FillCol ( rSet, *pItemSet, pColumns );
    }
    else
    {
        const :: SfxItemSet *pItemSet = &pDoc->GetFrmFmtFromPool( RES_POOLFRM_FRAME )->GetAttrSet();
           bRet = FillBaseProperties( pDoc, rSet, *pItemSet, rSizeFound );
        lcl_FillCol ( rSet, *pItemSet, pColumns );
    }
    const ::uno::Any* pEdit;
    if(GetProperty(RES_EDIT_IN_READONLY, 0, pEdit))
    {
        SfxBoolItem aBool(RES_EDIT_IN_READONLY);
        ((SfxPoolItem&)aBool).PutValue(*pEdit, 0);
        rSet.Put(aBool);
    }
    return bRet;
}

/****************************************************************************
    Grafik-Descriptor
****************************************************************************/
class SwGraphicProperties_Impl : public BaseFrameProperties_Impl
{
public:
    SwGraphicProperties_Impl();
    virtual ~SwGraphicProperties_Impl(){}

    virtual bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, bool& rSizeFound);
};

SwGraphicProperties_Impl::SwGraphicProperties_Impl( ) :
    BaseFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC)*/ )
{
}

static inline void lcl_FillMirror ( SfxItemSet &rToSet, const :: SfxItemSet &rFromSet, const ::uno::Any *pHEvenMirror, const ::uno::Any *pHOddMirror, const ::uno::Any *pVMirror, bool &rRet )
{
    if(pHEvenMirror || pHOddMirror || pVMirror )
    {
        SwMirrorGrf aMirror ( static_cast < const :: SwMirrorGrf& > ( rFromSet.Get ( RES_GRFATR_MIRRORGRF ) ) );
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
            SfxItemSet& rFrmSet,
            SfxItemSet& rGrSet,
            bool& rSizeFound)
{
    //Properties fuer alle Frames
    bool bRet;
    const ::uno::Any *pStyleName;
    SwDocStyleSheet* pStyle = NULL;

    if ( GetProperty ( FN_UNO_FRAME_STYLE_NAME, 0, pStyleName ) )
    {
        OUString sStyle;
        *pStyleName >>= sStyle;
        pStyle = (SwDocStyleSheet*)pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle,
                                                    SFX_STYLE_FAMILY_FRAME);
    }

    const ::uno::Any* pHEvenMirror = 0;
    const ::uno::Any* pHOddMirror = 0;
    const ::uno::Any* pVMirror = 0;
    GetProperty(RES_GRFATR_MIRRORGRF, MID_MIRROR_HORZ_EVEN_PAGES, pHEvenMirror);
    GetProperty(RES_GRFATR_MIRRORGRF, MID_MIRROR_HORZ_ODD_PAGES, pHOddMirror);
    GetProperty(RES_GRFATR_MIRRORGRF, MID_MIRROR_VERT, pVMirror);

    if ( pStyle )
    {
        rtl::Reference< SwDocStyleSheet > xStyle( new SwDocStyleSheet(*pStyle) );
        const :: SfxItemSet *pItemSet = &xStyle->GetItemSet();
        bRet = FillBaseProperties(pDoc, rFrmSet, *pItemSet, rSizeFound);
        lcl_FillMirror ( rGrSet, *pItemSet, pHEvenMirror, pHOddMirror, pVMirror, bRet );
    }
    else
    {
        const :: SfxItemSet *pItemSet = &pDoc->GetFrmFmtFromPool( RES_POOLFRM_GRAPHIC )->GetAttrSet();
        bRet = FillBaseProperties(pDoc, rFrmSet, *pItemSet, rSizeFound);
        lcl_FillMirror ( rGrSet, *pItemSet, pHEvenMirror, pHOddMirror, pVMirror, bRet );
    }


    static const :: sal_uInt16 nIDs[] =
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

    virtual bool AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, bool& rSizeFound);
};

bool SwOLEProperties_Impl::AnyToItemSet(
        SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, bool& rSizeFound)
{
    const ::uno::Any* pTemp;
    if(!GetProperty(FN_UNO_CLSID, 0, pTemp) && !GetProperty(FN_UNO_STREAM_NAME, 0, pTemp)
         && !GetProperty(FN_EMBEDDED_OBJECT, 0, pTemp) )
        return sal_False;
    SwFrameProperties_Impl::AnyToItemSet( pDoc, rFrmSet, rSet, rSizeFound);
    //
    return sal_True;
}


class SwXFrame::Impl
{
private:
    ::osl::Mutex m_Mutex; // just for OInterfaceContainerHelper

public:
    ::cppu::OInterfaceContainerHelper m_EventListeners;

    Impl() : m_EventListeners(m_Mutex) { }
};


namespace
{
    class theSwXFrameUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSwXFrameUnoTunnelId > {};
}

const :: uno::Sequence< sal_Int8 > & SwXFrame::getUnoTunnelId()
{
    return theSwXFrameUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SwXFrame::getSomething( const :: uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}

TYPEINIT1(SwXFrame, SwClient);

OUString SwXFrame::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXFrame");
}

sal_Bool SwXFrame::supportsService(const :: OUString& rServiceName) throw( uno::RuntimeException )
{
    return rServiceName.equalsAscii("com.sun.star.text.BaseFrame") ||
           rServiceName.equalsAscii("com.sun.star.text.TextContent") ||
           rServiceName.equalsAscii("com.sun.star.document.LinkTarget");
}

uno::Sequence< OUString > SwXFrame::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(3);
    aRet[0] = "com.sun.star.text.BaseFrame";
    aRet[1] = "com.sun.star.text.TextContent";
    aRet[2] = "com.sun.star.document.LinkTarget";
    return aRet;
}

SwXFrame::SwXFrame(FlyCntType eSet, const :: SfxItemPropertySet* pSet, SwDoc *pDoc)
    : m_pImpl(new Impl)
    ,
    m_pPropSet(pSet),
    m_pDoc ( pDoc ),
    eType(eSet),
    bIsDescriptor(true),
    m_pCopySource( 0 )
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    // get the property set for the default style data
    // First get the model
    uno::Reference < XModel > xModel = pDoc->GetDocShell()->GetBaseModel();
    // Ask the model for it's family supplier interface
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
            ;
    }
}

SwXFrame::SwXFrame(SwFrmFmt& rFrmFmt, FlyCntType eSet, const :: SfxItemPropertySet* pSet)
    : SwClient( &rFrmFmt )
    , m_pImpl(new Impl)
    ,
    m_pPropSet(pSet),
    m_pDoc( 0 ),
    eType(eSet),
    pProps(0),
    bIsDescriptor(false),
    m_pCopySource(0)
{

}

SwXFrame::~SwXFrame()
{
    delete m_pCopySource;
    delete pProps;
}

OUString SwXFrame::getName(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
        return pFmt->GetName();
    if(!bIsDescriptor)
        throw uno::RuntimeException();
    return m_sName;
}

void SwXFrame::setName(const OUString& rName) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        pFmt->GetDoc()->SetFlyName((SwFlyFrmFmt&)*pFmt, rName);
        if(pFmt->GetName() != rName)
        {
            throw uno::RuntimeException();
        }
    }
    else if(bIsDescriptor)
        m_sName = rName;
    else
        throw uno::RuntimeException();
}

uno::Reference< beans::XPropertySetInfo >  SwXFrame::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    uno::Reference< beans::XPropertySetInfo >  xRef;
    static uno::Reference< beans::XPropertySetInfo >  xFrmRef;
    static uno::Reference< beans::XPropertySetInfo >  xGrfRef;
    static uno::Reference< beans::XPropertySetInfo >  xOLERef;
    switch(eType)
    {
    case FLYCNTTYPE_FRM:
        if( !xFrmRef.is() )
            xFrmRef = m_pPropSet->getPropertySetInfo();
        xRef = xFrmRef;
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

SdrObject *SwXFrame::GetOrCreateSdrObject( SwFlyFrmFmt *pFmt )
{
    SdrObject* pObject = pFmt->FindSdrObject();
    if( !pObject )
    {
        SwDoc *pDoc = pFmt->GetDoc();
        // #i52858# - method name changed
        SdrModel *pDrawModel = pDoc->GetOrCreateDrawModel();
        SwFlyDrawContact* pContactObject
                    = new SwFlyDrawContact( pFmt, pDrawModel );
        pObject = pContactObject->GetMaster();

        const :: SwFmtSurround& rSurround = pFmt->GetSurround();
        pObject->SetLayer(
            ( SURROUND_THROUGHT == rSurround.GetSurround() &&
              !pFmt->GetOpaque().GetValue() ) ? pDoc->GetHellId()
                                             : pDoc->GetHeavenId() );

        pDrawModel->GetPage(0)->InsertObject( pObject );
    }

    return pObject;
}

static SwFrmFmt *lcl_GetFrmFmt( const :: uno::Any& rValue, SwDoc *pDoc )
{
    SwFrmFmt *pRet = 0;
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if(pDocSh)
    {
        OUString uTemp;
        rValue >>= uTemp;
        OUString sStyle;
        SwStyleNameMapper::FillUIName(uTemp, sStyle,
                nsSwGetPoolIdFromName::GET_POOLID_FRMFMT, true);
        SwDocStyleSheet* pStyle =
                (SwDocStyleSheet*)pDocSh->GetStyleSheetPool()->Find(sStyle,
                                                    SFX_STYLE_FAMILY_FRAME);
        if(pStyle)
            pRet = pStyle->GetFrmFmt();
    }

    return pRet;
}

void SwXFrame::setPropertyValue(const :: OUString& rPropertyName, const :: uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    const :: SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);

    if (!pEntry)
        throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(pFmt)
    {
        bool bNextFrame = false;
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException("Property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        SwDoc* pDoc = pFmt->GetDoc();
        if ( ((eType == FLYCNTTYPE_GRF) && isGRFATR(pEntry->nWID)) ||
            (FN_PARAM_COUNTOUR_PP        == pEntry->nWID) ||
            (FN_UNO_IS_AUTOMATIC_CONTOUR == pEntry->nWID) ||
            (FN_UNO_IS_PIXEL_CONTOUR     == pEntry->nWID) )
        {
            const :: SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                if(pEntry->nWID == FN_PARAM_COUNTOUR_PP)
                {
                    drawing::PointSequenceSequence aParam;
                    if(!aValue.hasValue())
                        pNoTxt->SetContour(0);
                    else if(aValue >>= aParam)
                    {
                        PolyPolygon aPoly((sal_uInt16)aParam.getLength());
                        for(sal_Int32 i = 0; i < aParam.getLength(); i++)
                        {
                            const :: drawing::PointSequence* pPointSeq = aParam.getConstArray();
                            sal_Int32 nPoints = pPointSeq[i].getLength();
                            const :: awt::Point* pPoints = pPointSeq[i].getConstArray();
                            Polygon aSet( (sal_uInt16)nPoints );
                            for(sal_Int32 j = 0; j < nPoints; j++)
                            {
                                Point aPoint(pPoints[j].X, pPoints[j].Y);
                                aSet.SetPoint(aPoint, (sal_uInt16)j);
                            }
                            // Close polygon if it isn't closed already.
                            aSet.Optimize( POLY_OPTIMIZE_CLOSE );
                            aPoly.Insert( aSet );
                        }
                        pNoTxt->SetContourAPI( &aPoly );
                    }
                    else
                        throw lang::IllegalArgumentException();
                }
                else if(pEntry->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR )
                {
                    pNoTxt->SetAutomaticContour( *(sal_Bool *)aValue.getValue() );
                }
                else if(pEntry->nWID == FN_UNO_IS_PIXEL_CONTOUR )
                {
                    // The IsPixelContour property can only be set if there
                    // is no contour, or if the contour has been set by the
                    // API itself (or in other words, if the contour isn't
                    // used already).
                    if( !pNoTxt->_HasContour() ||
                        !pNoTxt->IsContourMapModeValid() )
                        pNoTxt->SetPixelContour( *(sal_Bool *)aValue.getValue() );
                    else
                        throw lang::IllegalArgumentException();
                }
                else
                {
                    SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                    m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
                    pNoTxt->SetAttr(aSet);
                }
            }
        }
        // New attribute Title
        else if( FN_UNO_TITLE == pEntry->nWID )
        {
            SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
            OSL_ENSURE( pFmt,
                    "unexpected type of <pFmt> --> crash" );
            OUString sTitle;
            aValue >>= sTitle;
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject( pFlyFmt );
            pFlyFmt->GetDoc()->SetFlyFrmTitle( *(pFlyFmt), sTitle );
        }
        // New attribute Description
        else if( FN_UNO_DESCRIPTION == pEntry->nWID )
        {
            SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
            OSL_ENSURE( pFmt,
                    "unexpected type of <pFmt> --> crash" );
            OUString sDescription;
            aValue >>= sDescription;
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject( pFlyFmt );
            pFlyFmt->GetDoc()->SetFlyFrmDescription( *(pFlyFmt), sDescription );
        }
        else if(FN_UNO_FRAME_STYLE_NAME == pEntry->nWID)
        {
            SwFrmFmt *pFrmFmt = lcl_GetFrmFmt( aValue, pFmt->GetDoc() );
            if( pFrmFmt )
            {
                UnoActionContext aAction(pFmt->GetDoc());

                SfxItemSet* pSet = 0;
                // #i31771#, #i25798# - No adjustment of
                // anchor ( no call of method <sw_ChkAndSetNewAnchor(..)> ),
                // if document is currently in reading mode.
                if ( !pFmt->GetDoc()->IsInReading() )
                {
                    // see SwFEShell::SetFrmFmt( SwFrmFmt *pNewFmt, bool bKeepOrient, Point* pDocPos )
                    SwFlyFrm *pFly = 0;
                    {
                        const :: SwFrmFmt* pFmtXX = pFmt;
                        if (PTR_CAST(SwFlyFrmFmt, pFmtXX))
                            pFly = ((SwFlyFrmFmt*)pFmtXX)->GetFrm();
                    }
                    if ( pFly )
                    {
                        const :: SfxPoolItem* pItem;
                        if( SFX_ITEM_SET == pFrmFmt->GetItemState( RES_ANCHOR, false, &pItem ))
                        {
                            pSet = new SfxItemSet( pDoc->GetAttrPool(), aFrmFmtSetRange );
                            pSet->Put( *pItem );
                            if ( pFmt->GetDoc()->GetEditShell() != NULL
                                 && !sw_ChkAndSetNewAnchor( *(pFmt->GetDoc()->GetEditShell()), *pFly, *pSet ) )
                                delete pSet, pSet = 0;
                        }
                    }
                }

                pFmt->GetDoc()->SetFrmFmtToFly( *pFmt, *pFrmFmt, pSet, false );
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
            GraphicObject *pGrfObj = 0;
            pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, &sGrfName, &sFltName );
            OUString sTmp;
            aValue >>= sTmp;
            UnoActionContext aAction(pFmt->GetDoc());
            if(FN_UNO_GRAPHIC_U_R_L == pEntry->nWID)
            {
                if( sTmp.startsWith(sPackageProtocol) )
                {
                    pGrfObj = new GraphicObject;
                    pGrfObj->SetUserData( sTmp );
                    pGrfObj->SetSwapState();
                    sGrfName = "";
                }
                else if( sTmp.startsWith(sGraphicObjectProtocol) )
                {
                    const OString sId(OUStringToOString(
                        sTmp.copy(sizeof(sGraphicObjectProtocol)-1),
                        RTL_TEXTENCODING_ASCII_US));
                    pGrfObj = new GraphicObject( sId );
                    sGrfName = "";
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

            const :: SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
//              SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                {
                    delete pGrfObj;
                    throw uno::RuntimeException();
                }
                SwPaM aGrfPaM(*pGrfNode);
                pFmt->GetDoc()->ReRead( aGrfPaM, sGrfName, sFltName, 0,
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
                const :: SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
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
                    pFmt->GetDoc()->ReRead( aGrfPaM, OUString(), OUString(), &aGraphic, 0 );
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
                GraphicObject *pGrfObj = 0;
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
                const :: SwFmtCntnt* pCnt = &pFmt->GetCntnt();
                if ( pCnt->GetCntntIdx() && pDoc->GetNodes()[ pCnt->GetCntntIdx()->GetIndex() + 1 ] )
                {
                    SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetCntntIdx()->GetIndex() + 1 ]->GetOLENode();

                    if ( pOleNode )
                    {
                        svt::EmbeddedObjectRef &rObj = pOleNode->GetOLEObj().GetObject();

                        OUString aMediaType;
                        rObj.SetGraphic( aGraphic, aMediaType );
                    }
                }
            }
        }
        else if(0 != (bNextFrame = (rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CHAIN_NEXT_NAME))))
            || rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CHAIN_PREV_NAME)))
        {
            OUString sChainName;
            aValue >>= sChainName;
            if (sChainName.isEmpty())
            {
                if(bNextFrame)
                    pDoc->Unchain(*pFmt);
                else
                {
                    SwFmtChain aChain( pFmt->GetChain() );
                    SwFrmFmt *pPrev = aChain.GetPrev();
                    if(pPrev)
                        pDoc->Unchain(*pPrev);
                }
            }
            else
            {
                sal_uInt16 nCount = pDoc->GetFlyCount(FLYCNTTYPE_FRM);

                SwFrmFmt* pChain = 0;
                for( sal_uInt16 i = 0; i < nCount; i++)
                {
                    SwFrmFmt* pFmt2 = pDoc->GetFlyNum(i, FLYCNTTYPE_FRM);
                    if(sChainName == pFmt2->GetName() )
                    {
                        pChain = pFmt2;
                        break;
                    }
                }
                if(pChain)
                {
                    SwFrmFmt* pSource = bNextFrame ? pFmt : pChain;
                    SwFrmFmt* pDest = bNextFrame ? pChain: pFmt;
                    pDoc->Chain(*pSource, *pDest);
                }
            }
        }
        else if(FN_UNO_Z_ORDER == pEntry->nWID)
        {
            sal_Int32 nZOrder = - 1;
            aValue >>= nZOrder;
            if( nZOrder >= 0)
            {
                SdrObject* pObject =
                    GetOrCreateSdrObject( (SwFlyFrmFmt*)pFmt );
                SdrModel *pDrawModel = pDoc->GetDrawModel();
                pDrawModel->GetPage(0)->
                            SetObjectOrdNum(pObject->GetOrdNum(), nZOrder);
            }
        }
        else if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORFRAME == pEntry->nMemberId)
        {
            bool bDone = false;
            uno::Reference<text::XTextFrame> xFrame;
            if(aValue >>= xFrame)
            {
                uno::Reference<lang::XUnoTunnel> xTunnel(xFrame, uno::UNO_QUERY);
                SwXFrame* pFrame = xTunnel.is() ?
                        reinterpret_cast< SwXFrame * >( sal::static_int_cast< sal_IntPtr >( xTunnel->getSomething(SwXFrame::getUnoTunnelId()) ))
                        : 0;
                if(pFrame && this != pFrame && pFrame->GetFrmFmt() && pFrame->GetFrmFmt()->GetDoc() == pDoc)
                {
                    SfxItemSet aSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                    aSet.SetParent(&pFmt->GetAttrSet());
                    SwFmtAnchor aAnchor = (const :: SwFmtAnchor&)aSet.Get(pEntry->nWID);


                    SwPosition aPos(*pFrame->GetFrmFmt()->GetCntnt().GetCntntIdx());
                    aAnchor.SetAnchor(&aPos);
                    aAnchor.SetType(FLY_AT_FLY);
                    aSet.Put(aAnchor);
                    pDoc->SetFlyFrmAttr( *pFmt, aSet );
                    bDone = true;
                }
            }
            if(!bDone)
                throw lang::IllegalArgumentException();
        }
        else
        {
            SfxItemSet aSet( pDoc->GetAttrPool(),
                RES_FRMATR_BEGIN, RES_FRMATR_END - 1,
                RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
                0L);

            aSet.SetParent(&pFmt->GetAttrSet());
            m_pPropSet->setPropertyValue(*pEntry, aValue, aSet);
            if(RES_ANCHOR == pEntry->nWID && MID_ANCHOR_ANCHORTYPE == pEntry->nMemberId)
            {
                SwFmtAnchor aAnchor = (const :: SwFmtAnchor&)aSet.Get(pEntry->nWID);
                if(aAnchor.GetAnchorId() == FLY_AT_FLY)
                {
                    const :: SwPosition* pPosition = aAnchor.GetCntntAnchor();
                    SwFrmFmt* pFlyFmt = pPosition ? pPosition->nNode.GetNode().GetFlyFmt() : 0;
                    if(!pFlyFmt || pFlyFmt->Which() == RES_DRAWFRMFMT)
                    {
                        lang::IllegalArgumentException aExcept;
                        aExcept.Message = "Anchor to frame: no frame found";
                        throw aExcept;
                    }
                    else
                    {
                        SwPosition aPos = *pPosition;
                        aPos.nNode = *pFlyFmt->GetCntnt().GetCntntIdx();
                        aAnchor.SetAnchor(&aPos);
                        aSet.Put(aAnchor);
                    }
                }
                else if ((aAnchor.GetAnchorId() != FLY_AT_PAGE) &&
                         !aAnchor.GetCntntAnchor())
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
                if ( !pFmt->GetDoc()->IsInReading() )
                {
                    // see SwFEShell::SetFlyFrmAttr( SfxItemSet& rSet )
                    SwFlyFrm *pFly = 0;
                    if (PTR_CAST(SwFlyFrmFmt, pFmt))
                        pFly = ((SwFlyFrmFmt*)pFmt)->GetFrm();
                    if (pFly)
                    {
                        const :: SfxPoolItem* pItem;
                        if( SFX_ITEM_SET == aSet.GetItemState( RES_ANCHOR, false, &pItem ))
                        {
                            aSet.Put( *pItem );
                            if ( pFmt->GetDoc()->GetEditShell() != NULL )
                            {
                                sw_ChkAndSetNewAnchor( *(pFmt->GetDoc()->GetEditShell()), *pFly, aSet );
                            }
                        }
                    }
                }

                pFmt->GetDoc()->SetFlyFrmAttr( *pFmt, aSet );
            }
            else if(FN_UNO_CLSID == pEntry->nWID || FN_UNO_STREAM_NAME == pEntry->nWID || FN_EMBEDDED_OBJECT == pEntry->nWID)
            {
                throw lang::IllegalArgumentException();
            }
            else
                pFmt->SetFmtAttr(aSet);
        }
    }
    else if(IsDescriptor())
    {
        pProps->SetProperty(pEntry->nWID, pEntry->nMemberId, aValue);
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
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aAny;
    SwFrmFmt* pFmt = GetFrmFmt();
    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
    if (!pEntry)
        throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(FN_UNO_ANCHOR_TYPES == pEntry->nWID)
    {
        uno::Sequence<text::TextContentAnchorType> aTypes(5);
         text::TextContentAnchorType* pArray = aTypes.getArray();
        pArray[0] = text::TextContentAnchorType_AT_PARAGRAPH;
        pArray[1] = text::TextContentAnchorType_AS_CHARACTER;
        pArray[2] = text::TextContentAnchorType_AT_PAGE;
        pArray[3] = text::TextContentAnchorType_AT_FRAME;
        pArray[4] = text::TextContentAnchorType_AT_CHARACTER;
        aAny.setValue(&aTypes, ::getCppuType(static_cast<uno::Sequence<text::TextContentAnchorType>*>(0)));
    }
    else if(pFmt)
    {
        if( ((eType == FLYCNTTYPE_GRF) || (eType == FLYCNTTYPE_OLE)) &&
                pEntry &&
                (isGRFATR(pEntry->nWID) ||
                        pEntry->nWID == FN_PARAM_COUNTOUR_PP ||
                        pEntry->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR ||
                        pEntry->nWID == FN_UNO_IS_PIXEL_CONTOUR ))
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                if(pEntry->nWID == FN_PARAM_COUNTOUR_PP)
                {
                    PolyPolygon aContour;
                    if( pNoTxt->GetContourAPI( aContour ) )
                    {
                        drawing::PointSequenceSequence aPtSeq(aContour.Count());
                        drawing::PointSequence* pPSeq = aPtSeq.getArray();
                        for(sal_uInt16 i = 0; i < aContour.Count(); i++)
                        {
                            const Polygon& rPoly = aContour.GetObject(i);
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
                    sal_Bool bValue = pNoTxt->HasAutomaticContour();
                    aAny.setValue( &bValue, ::getBooleanCppuType() );
                }
                else if(pEntry->nWID == FN_UNO_IS_PIXEL_CONTOUR )
                {
                    sal_Bool bValue = pNoTxt->IsPixelContour();
                    aAny.setValue( &bValue, ::getBooleanCppuType() );
                }
                else
                {
                    SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                    m_pPropSet->getPropertyValue(*pEntry, aSet, aAny);
                }
            }
        }
        else if( FN_UNO_GRAPHIC_U_R_L == pEntry->nWID)
        {
            OUString sGrfName;
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                    throw uno::RuntimeException();
                if( pGrfNode->IsGrfLink() )
                {
                    pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, &sGrfName, 0 );
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
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();

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
            pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, 0, &sFltName );
                aAny <<= sFltName;
        }
        else if( FN_UNO_GRAPHIC == pEntry->nWID )
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
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
            aAny <<= OUString(SwStyleNameMapper::GetProgName(pFmt->DerivedFrom()->GetName(), nsSwGetPoolIdFromName::GET_POOLID_FRMFMT ) );
        }
        // #i73249#
        // Attribute AlternativeText was never published.
        // Now it has been replaced by Attribute Title - valid for all <SwXFrame> instances
//        else if(eType != FLYCNTTYPE_FRM &&
//                FN_UNO_ALTERNATIVE_TEXT == pEntry->nWID)
//        {
//            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
//            if(pIdx)
//            {
//                SwNodeIndex aIdx(*pIdx, 1);
//                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
//                aAny <<= OUString(pNoTxt->GetAlternateText());
//            }
//        }
        else if( FN_UNO_TITLE == pEntry->nWID )
        {
            SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
            OSL_ENSURE( pFmt,
                    "unexpected type of <pFmt> --> crash" );
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject( pFlyFmt );
            aAny <<= OUString(pFlyFmt->GetObjTitle());
        }
        // New attribute Description
        else if( FN_UNO_DESCRIPTION == pEntry->nWID )
        {
            SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
            OSL_ENSURE( pFmt,
                    "unexpected type of <pFmt> --> crash" );
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject( pFlyFmt );
            aAny <<= OUString(pFlyFmt->GetObjDescription());
        }
        else if(eType == FLYCNTTYPE_GRF &&
                (rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ACTUAL_SIZE))))
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                // --> OD #i85105#
//                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
//                Size aActSize = ((SwGrfNode*)pNoTxt)->GetTwipSize();
                Size aActSize;
                {
                    SwGrfNode* pGrfNode = dynamic_cast<SwGrfNode*>(aIdx.GetNode().GetNoTxtNode());
                    if ( pGrfNode )
                    {
                        aActSize = pGrfNode->GetTwipSize();
                        if ( aActSize.Width() == 0 && aActSize.Height() == 0 &&
                             pGrfNode->IsLinkedFile() )
                        {
                            pGrfNode->SwapIn( sal_True );
                            aActSize = pGrfNode->GetTwipSize();
                        }
                    }
                }
                // <--
                awt::Size aTmp;
                aTmp.Width = TWIP_TO_MM100(aActSize.Width());
                aTmp.Height = TWIP_TO_MM100(aActSize.Height());
                aAny.setValue(&aTmp, ::getCppuType(static_cast<const awt::Size*>(0)));
            }
        }
        else if(FN_PARAM_LINK_DISPLAY_NAME == pEntry->nWID)
        {
            aAny <<= pFmt->GetName();
        }
        else if(FN_UNO_Z_ORDER == pEntry->nWID)
        {
            const SdrObject* pObj = pFmt->FindRealSdrObject();
            if( pObj == NULL )
                pObj = pFmt->FindSdrObject();
            if( pObj )
            {
                aAny <<= (sal_Int32)pObj->GetOrdNum();
            }
        }
        else if(FN_UNO_CLSID == pEntry->nWID || FN_UNO_MODEL == pEntry->nWID||
                FN_UNO_COMPONENT == pEntry->nWID ||FN_UNO_STREAM_NAME == pEntry->nWID||
                FN_EMBEDDED_OBJECT == pEntry->nWID)
        {
            SwDoc* pDoc = pFmt->GetDoc();
            const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
            OSL_ENSURE( pCnt->GetCntntIdx() &&
                           pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                            GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

            SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetCntntIdx()
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
                            // ensure the
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
            pFmt->GetDoc()->GetEditShell()->CalcLayout();

            SwFrm* pTmpFrm = SwIterator<SwFrm,SwFmt>::FirstElement( *pFmt );
            if ( pTmpFrm )
            {
                OSL_ENSURE( pTmpFrm->IsValid(), "frame not valid" );
                const SwRect &rRect = pTmpFrm->Frm();
                Size aMM100Size = OutputDevice::LogicToLogic(
                        Size( rRect.Width(), rRect.Height() ),
                        MapMode( MAP_TWIP ), MapMode( MAP_100TH_MM ));
                aAny <<= awt::Size( aMM100Size.Width(), aMM100Size.Height() );
            }
        }
        else
        {
            const SwAttrSet& rSet = pFmt->GetAttrSet();
            m_pPropSet->getPropertyValue(*pEntry, rSet, aAny);
        }
    }
    else if(IsDescriptor())
    {
        if ( ! m_pDoc )
            throw uno::RuntimeException();
        if(WID_LAYOUT_SIZE != pEntry->nWID)  // there is no LayoutSize in a descriptor
        {
            const uno::Any* pAny = 0;
            if( !pProps->GetProperty( pEntry->nWID, pEntry->nMemberId, pAny ) )
                aAny = mxStyleData->getPropertyValue( rPropertyName );
            else if ( pAny )
                aAny = *pAny;
        }
    }
    else
        throw uno::RuntimeException();
    return aAny;
}

void SwXFrame::addPropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXFrame::removePropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXFrame::addVetoableChangeListener(const OUString& /*PropertyName*/,
                                const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

void SwXFrame::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    OSL_FAIL("not implemented");
}

beans::PropertyState SwXFrame::getPropertyState( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< OUString > aPropertyNames(1);
    OUString* pNames = aPropertyNames.getArray();
    pNames[0] = rPropertyName;
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aPropertyNames);
    return aStates.getConstArray()[0];
}

uno::Sequence< beans::PropertyState > SwXFrame::getPropertyStates(
    const uno::Sequence< OUString >& aPropertyNames )
        throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Sequence< beans::PropertyState > aStates(aPropertyNames.getLength());
    beans::PropertyState* pStates = aStates.getArray();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const OUString* pNames = aPropertyNames.getConstArray();
        const SwAttrSet& rFmtSet = pFmt->GetAttrSet();
        for(int i = 0; i < aPropertyNames.getLength(); i++)
        {
            const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(pNames[i]);
            if (!pEntry)
                throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + pNames[i], static_cast < cppu::OWeakObject * > ( this ) );

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
            else
            {
                if ((eType == FLYCNTTYPE_GRF) &&
                        pEntry && isGRFATR(pEntry->nWID))
                {
                    const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                    if(pIdx)
                    {
                        SwNodeIndex aIdx(*pIdx, 1);
                        SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                        SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                        aSet.GetItemState(pEntry->nWID);
                        if(SFX_ITEM_SET == aSet.GetItemState( pEntry->nWID, sal_False ))
                            pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                }
                else
                {
                    if(SFX_ITEM_SET == rFmtSet.GetItemState( pEntry->nWID, sal_False ))
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
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if (!pEntry)
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw uno::RuntimeException("setPropertyToDefault: property is read-only: " + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        bool bNextFrame;
        if( pEntry->nWID &&
            pEntry->nWID != FN_UNO_ANCHOR_TYPES &&
            pEntry->nWID != FN_PARAM_LINK_DISPLAY_NAME)
        {
            if ( (eType == FLYCNTTYPE_GRF) && isGRFATR(pEntry->nWID) )
            {
                const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                if(pIdx)
                {
                    SwNodeIndex aIdx(*pIdx, 1);
                    SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                    {
                        SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                        aSet.ClearItem(pEntry->nWID);
                        pNoTxt->SetAttr(aSet);
                    }
                }
            }
            // #i73249#
            // Attribute AlternativeText was never published.
            // Now it has been replaced by Attribute Title - valid for all <SwXFrame> instances
//            else if( eType != FLYCNTTYPE_FRM && FN_UNO_ALTERNATIVE_TEXT == pEntry->nWID )
//            {
//                const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
//                if(pIdx)
//                {
//                    SwNodeIndex aIdx(*pIdx, 1);
//                    SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
//                    pNoTxt->SetAlternateText(aEmptyOUStr);
//                }
//            }
            // New attribute Title
            else if( FN_UNO_TITLE == pEntry->nWID )
            {
                SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
                OSL_ENSURE( pFmt,
                        "unexpected type of <pFmt> --> crash" );
                // assure that <SdrObject> instance exists.
                GetOrCreateSdrObject( pFlyFmt );
                pFlyFmt->GetDoc()->SetFlyFrmTitle( *(pFlyFmt), aEmptyOUStr );
            }
            // New attribute Description
            else if( FN_UNO_DESCRIPTION == pEntry->nWID )
            {
                SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
                OSL_ENSURE( pFmt,
                        "unexpected type of <pFmt> --> crash" );
                // assure that <SdrObject> instance exists.
                GetOrCreateSdrObject( pFlyFmt );
                pFlyFmt->GetDoc()->SetFlyFrmDescription( *(pFlyFmt), aEmptyOUStr );
            }
            else
            {
                SwDoc* pDoc = pFmt->GetDoc();
                SfxItemSet aSet( pDoc->GetAttrPool(),
                    RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                aSet.SetParent(&pFmt->GetAttrSet());
                aSet.ClearItem(pEntry->nWID);
                if(!rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE)))
                    pFmt->SetFmtAttr(aSet);
            }
        }
        else if(0 != (bNextFrame = (rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CHAIN_NEXT_NAME))))
                || rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CHAIN_PREV_NAME)))
        {
            SwDoc* pDoc = pFmt->GetDoc();
            if(bNextFrame)
                pDoc->Unchain(*pFmt);
            else
            {
                SwFmtChain aChain( pFmt->GetChain() );
                SwFrmFmt *pPrev = aChain.GetPrev();
                if(pPrev)
                    pDoc->Unchain(*pPrev);
            }
        }
    }
    else if(!IsDescriptor())
        throw uno::RuntimeException();

}

uno::Any SwXFrame::getPropertyDefault( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap().getByName(rPropertyName);
        if(pEntry)
        {
            if ( pEntry->nWID < RES_FRMATR_END )
            {
                const SfxPoolItem& rDefItem =
                    pFmt->GetDoc()->GetAttrPool().GetDefaultItem(pEntry->nWID);
                rDefItem.QueryValue(aRet, pEntry->nMemberId);
            }
        }
        else
            throw beans::UnknownPropertyException(OUString( "Unknown property: " ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else if(!IsDescriptor())
        throw uno::RuntimeException();
    return aRet;
}

void SAL_CALL SwXFrame::addEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.addInterface(xListener);
}

void SAL_CALL SwXFrame::removeEventListener(
        const uno::Reference<lang::XEventListener> & xListener)
throw (uno::RuntimeException)
{
    // no need to lock here as m_pImpl is const and container threadsafe
    m_pImpl->m_EventListeners.removeInterface(xListener);
}

void    SwXFrame::Modify( const SfxPoolItem* pOld, const SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        mxStyleData.clear();
        mxStyleFamily.clear();
        m_pDoc = 0;
        lang::EventObject const ev(static_cast< ::cppu::OWeakObject&>(*this));
        m_pImpl->m_EventListeners.disposeAndClear(ev);
    }
}

void SwXFrame::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if ( pFmt )
    {
        SdrObject* pObj = pFmt->FindSdrObject();
        // OD 11.09.2003 #112039# - add condition to perform delete of
        // format/anchor sign, not only if the object is inserted, but also
        // if a contact object is registered, which isn't in the destruction.
        if ( pObj &&
             ( pObj->IsInserted() ||
               ( pObj->GetUserCall() &&
                 !static_cast<SwContact*>(pObj->GetUserCall())->IsInDTOR() ) ) )
        {
            if (pFmt->GetAnchor().GetAnchorId() == FLY_AS_CHAR)
            {
                const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
                SwTxtNode *pTxtNode = rPos.nNode.GetNode().GetTxtNode();
                const sal_Int32 nIdx = rPos.nContent.GetIndex();
                pTxtNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
            }
            else
                pFmt->GetDoc()->DelLayoutFmt(pFmt);
        }
    }

}

uno::Reference< text::XTextRange >  SwXFrame::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        // return an anchor for non-page bound frames
        // and for page bound frames that have a page no == NULL and a content position
        if ((rAnchor.GetAnchorId() != FLY_AT_PAGE) ||
            (rAnchor.GetCntntAnchor() && !rAnchor.GetPageNum()))
        {
            const SwPosition &rPos = *(rAnchor.GetCntntAnchor());
            aRef = SwXTextRange::CreateXTextRange(*pFmt->GetDoc(), rPos, 0);
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
            throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if(!IsDescriptor())
        throw uno::RuntimeException();
    uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange  = reinterpret_cast< SwXTextRange * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( SwXTextRange::getUnoTunnelId()) ));
        pCursor = reinterpret_cast< OTextCursorHelper * >(
                sal::static_int_cast< sal_IntPtr >( xRangeTunnel->getSomething( OTextCursorHelper::getUnoTunnelId()) ));
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc)
    {
        SwUnoInternalPaM aIntPam(*pDoc);
        //das muss jetzt true liefern
        ::sw::XTextRangeToSwPaM(aIntPam, xTextRange);

        SwNode& rNode = pDoc->GetNodes().GetEndOfContent();
        SwPaM aPam(rNode);
        aPam.Move( fnMoveBackward, fnGoDoc );
        static sal_uInt16 const aFrmAttrRange[] =
        {
            RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
            SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
            RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
            0
        };
        static sal_uInt16 const aGrAttrRange[] =
        {
            RES_GRFATR_BEGIN,       RES_GRFATR_END-1,
            0
        };
        SfxItemSet aGrSet(pDoc->GetAttrPool(), aGrAttrRange );

        SfxItemSet aFrmSet(pDoc->GetAttrPool(), aFrmAttrRange );
        //jetzt muessen die passenden Items in den Set
        bool bSizeFound;
        if(!pProps->AnyToItemSet( pDoc, aFrmSet, aGrSet, bSizeFound))
            throw lang::IllegalArgumentException();
        //der TextRange wird einzeln behandelt
        *aPam.GetPoint() = *aIntPam.GetPoint();
        if(aIntPam.HasMark())
        {
            aPam.SetMark();
            *aPam.GetMark() = *aIntPam.GetMark();
        }

        const SfxPoolItem* pItem;
        RndStdIds eAnchorId = FLY_AT_PARA;
        if(SFX_ITEM_SET == aFrmSet.GetItemState(RES_ANCHOR, false, &pItem) )
        {
            eAnchorId = ((const SwFmtAnchor*)pItem)->GetAnchorId();
            if( FLY_AT_FLY == eAnchorId &&
                !aPam.GetNode()->FindFlyStartNode())
            {
                //rahmengebunden geht nur dort, wo ein Rahmen ist!
                SwFmtAnchor aAnchor(FLY_AT_PARA);
                aFrmSet.Put(aAnchor);
            }
            else if ((FLY_AT_PAGE == eAnchorId) &&
                     0 == ((const SwFmtAnchor*)pItem)->GetPageNum() )
            {
                SwFmtAnchor aAnchor( *((const SwFmtAnchor*)pItem) );
                aAnchor.SetAnchor( aPam.GetPoint() );
                aFrmSet.Put(aAnchor);
            }
        }

        const ::uno::Any* pStyle;
        SwFrmFmt *pParentFrmFmt = 0;
        if(pProps->GetProperty(FN_UNO_FRAME_STYLE_NAME, 0, pStyle))
            pParentFrmFmt = lcl_GetFrmFmt( *pStyle, pDoc );

        SwFlyFrmFmt* pFmt = 0;
        if( eType == FLYCNTTYPE_FRM)
        {
            UnoActionContext aCont(pDoc);
            if(m_pCopySource)
            {
                SwFmtAnchor* pAnchorItem = 0;
                // the frame is inserted bound to page
                // to prevent conflicts if the to-be-anchored position is part of the to-be-copied text
                if (eAnchorId != FLY_AT_PAGE)
                {
                    pAnchorItem = static_cast<SwFmtAnchor*>(aFrmSet.Get(RES_ANCHOR).Clone());
                    aFrmSet.Put( SwFmtAnchor( FLY_AT_PAGE, 1 ));
                }

                aPam.DeleteMark(); // mark position node will be deleted!
                aIntPam.DeleteMark(); // mark position node will be deleted!
                pFmt = pDoc->MakeFlyAndMove( *m_pCopySource, aFrmSet,
                               0,
                               pParentFrmFmt );
                if(pAnchorItem && pFmt)
                {
                    pFmt->DelFrms();
                    pAnchorItem->SetAnchor( m_pCopySource->Start() );
                    SfxItemSet aAnchorSet( pDoc->GetAttrPool(), RES_ANCHOR, RES_ANCHOR );
                    aAnchorSet.Put( *pAnchorItem );
                    pDoc->SetFlyFrmAttr( *pFmt, aAnchorSet );
                    delete pAnchorItem;
                }
                DELETEZ( m_pCopySource );
            }
            else
            {
                pFmt = pDoc->MakeFlySection( FLY_AT_PARA, aPam.GetPoint(),
                                         &aFrmSet, pParentFrmFmt );
            }
            if(pFmt)
            {
                pFmt->Add(this);
                if(!m_sName.isEmpty())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFmt, m_sName);
            }
            //den SwXText wecken
            ((SwXTextFrame*)this)->SetDoc( bIsDescriptor ? m_pDoc : GetFrmFmt()->GetDoc() );
        }
        else if( eType == FLYCNTTYPE_GRF)
        {
            UnoActionContext aCont(pDoc);
            const ::uno::Any* pGraphicURL;
            OUString sGraphicURL;
            GraphicObject *pGrfObj = 0;
            if(pProps->GetProperty(FN_UNO_GRAPHIC_U_R_L, 0, pGraphicURL))
            {
                (*pGraphicURL) >>= sGraphicURL;
                if( sGraphicURL.startsWith(sPackageProtocol) )
                {
                    pGrfObj = new GraphicObject;
                    pGrfObj->SetUserData( sGraphicURL );
                    pGrfObj->SetSwapState();
                    sGraphicURL = "";
                }
                else if( sGraphicURL.startsWith(sGraphicObjectProtocol) )
                {
                    OString sId(OUStringToOString(
                        sGraphicURL.copy( sizeof(sGraphicObjectProtocol)-1 ),
                        RTL_TEXTENCODING_ASCII_US));
                    pGrfObj = new GraphicObject( sId );
                    sGraphicURL = "";
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

            pFmt =
                pGrfObj ? pDoc->Insert( aPam, *pGrfObj, &aFrmSet, &aGrSet,
                                        pParentFrmFmt )
                        : pDoc->Insert( aPam, sGraphicURL, sFltName, &aGraphic,
                                        &aFrmSet, &aGrSet, pParentFrmFmt  );
            delete pGrfObj;
            if(pFmt)
            {
                SwGrfNode *pGrfNd = pDoc->GetNodes()[ pFmt->GetCntnt().GetCntntIdx()
                                            ->GetIndex()+1 ]->GetGrfNode();
                pGrfNd->SetChgTwipSize( !bSizeFound );
                pFmt->Add(this);
                if(!m_sName.isEmpty())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFmt, m_sName);

            }
            const ::uno::Any* pSurroundContour;
            if(pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUR, pSurroundContour))
                setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_SURROUND_CONTOUR)), *pSurroundContour);
            const ::uno::Any* pContourOutside;
            if(pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUROUTSIDE, pContourOutside))
                setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_CONTOUR_OUTSIDE)), *pContourOutside);
            const ::uno::Any* pContourPoly;
            if(pProps->GetProperty(FN_PARAM_COUNTOUR_PP, 0, pContourPoly))
                setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_CONTOUR_POLY_POLYGON)), *pContourPoly);
            const ::uno::Any* pPixelContour;
            if(pProps->GetProperty(FN_UNO_IS_PIXEL_CONTOUR, 0, pPixelContour))
                setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_IS_PIXEL_CONTOUR)), *pPixelContour);
            const ::uno::Any* pAutoContour;
            if(pProps->GetProperty(FN_UNO_IS_AUTOMATIC_CONTOUR, 0, pAutoContour))
                setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_IS_AUTOMATIC_CONTOUR)), *pAutoContour);
        }
        else
        {
            const ::uno::Any* pCLSID = 0;
            const ::uno::Any* pStreamName = 0;
            const ::uno::Any* pEmbeddedObject = 0;
            if(!pProps->GetProperty(FN_UNO_CLSID, 0, pCLSID)
                && !pProps->GetProperty( FN_UNO_STREAM_NAME, 0, pStreamName )
                && !pProps->GetProperty( FN_EMBEDDED_OBJECT, 0, pEmbeddedObject ))
                throw uno::RuntimeException();
            if(pCLSID)
            {
                OUString aCLSID;
                SvGlobalName aClassName;
                uno::Reference < embed::XEmbeddedObject > xIPObj;
                std::auto_ptr < comphelper::EmbeddedObjectContainer > pCnt;
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
                    //TODO/LATER: MISCSTATUS_RESIZEONPRINTERCHANGE
                    //if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xIPObj->GetMiscStatus() && pDoc->getPrinter( false ) )
                    //    xIPObj->OnDocumentPrinterChanged( pDoc->getPrinter( false ) );

                    UnoActionContext aAction(pDoc);
                    pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);
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
                        SwFmtFrmSize aFrmSz;
                        aFrmSz.SetSize(aSz);
                        aFrmSet.Put(aFrmSz);
                    }
                    SwFlyFrmFmt* pFmt2 = 0;

                    // TODO/LATER: Is it the only possible aspect here?
                    sal_Int64 nAspect = embed::Aspects::MSOLE_CONTENT;
                    ::svt::EmbeddedObjectRef xObjRef( xIPObj, nAspect );
                    pFmt2 = pDoc->Insert(aPam, xObjRef, &aFrmSet, NULL, NULL );
                    OSL_ENSURE( pFmt2, "Doc->Insert(notxt) failed." );

                    pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
                    pFmt2->Add(this);
                    if(!m_sName.isEmpty())
                        pDoc->SetFlyName((SwFlyFrmFmt&)*pFmt2, m_sName);
                }
            }
            else if( pStreamName )
            {
                OUString sStreamName;
                (*pStreamName) >>= sStreamName;
                pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);

                SwFlyFrmFmt* pFrmFmt = 0;
                pFrmFmt = pDoc->InsertOLE( aPam, sStreamName, embed::Aspects::MSOLE_CONTENT, &aFrmSet, NULL, NULL );
                pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
                pFrmFmt->Add(this);
                if(!m_sName.isEmpty())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFrmFmt, m_sName);
            }
            else if( pEmbeddedObject || pStreamName )
            {
                uno::Reference< embed::XEmbeddedObject > obj;
                (*pEmbeddedObject) >>= obj;
                svt::EmbeddedObjectRef xObj;
                xObj.Assign( obj, embed::Aspects::MSOLE_CONTENT );

                pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);

                // Not sure if these setParent() and InsertEmbeddedObject() calls are really
                // needed, it seems to work without, but logic from code elsewhere suggests
                // they should be done.
                SfxObjectShell& mrPers = *pDoc->GetPersist();
                uno::Reference < container::XChild > xChild( obj, uno::UNO_QUERY );
                if ( xChild.is() )
                    xChild->setParent( mrPers.GetModel() );
                OUString rName;
                mrPers.GetEmbeddedObjectContainer().InsertEmbeddedObject( obj, rName );

                SwFlyFrmFmt* pFrmFmt = 0;
                pFrmFmt = pDoc->Insert( aPam, xObj, &aFrmSet, NULL, NULL );
                pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
                pFrmFmt->Add(this);
                if(!m_sName.isEmpty())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFrmFmt, m_sName);
            }
        }
        if( pFmt && pDoc->GetDrawModel() )
            GetOrCreateSdrObject( pFmt );
        const ::uno::Any* pOrder;
        if( pProps->GetProperty(FN_UNO_Z_ORDER, 0, pOrder) )
            setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_Z_ORDER)), *pOrder);
        const ::uno::Any* pReplacement;
        if( pProps->GetProperty(FN_UNO_REPLACEMENT_GRAPHIC, 0, pReplacement) )
            setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_GRAPHIC)), *pReplacement);
        // new attribute Title
        const ::uno::Any* pTitle;
        if ( pProps->GetProperty(FN_UNO_TITLE, 0, pTitle) )
        {
            setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_TITLE)), *pTitle);
        }
        // new attribute Description
        const ::uno::Any* pDescription;
        if ( pProps->GetProperty(FN_UNO_DESCRIPTION, 0, pDescription) )
        {
            setPropertyValue(OUString::createFromAscii(SW_PROP_NAME_STR(UNO_NAME_DESCRIPTION)), *pDescription);
        }
    }
    else
        throw lang::IllegalArgumentException();
    //setzt das Flag zurueck und loescht den Descriptor-Pointer
    ResetDescriptor();
}

void SwXFrame::attach(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SwFrmFmt* pFmt;
    if(IsDescriptor())
        attachToRange(xTextRange);
    else if(0 != (pFmt = GetFrmFmt()))
    {
        uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
        SwDoc* pDoc = pFmt->GetDoc();
        SwUnoInternalPaM aIntPam(*pDoc);
        if (::sw::XTextRangeToSwPaM(aIntPam, xTextRange))
        {
            SfxItemSet aSet( pDoc->GetAttrPool(), RES_ANCHOR, RES_ANCHOR );
            aSet.SetParent(&pFmt->GetAttrSet());
            SwFmtAnchor aAnchor = (const SwFmtAnchor&)aSet.Get(RES_ANCHOR);
            aAnchor.SetAnchor( aIntPam.Start() );
            aSet.Put(aAnchor);
            pDoc->SetFlyFrmAttr( *pFmt, aSet );
        }
        else
            throw lang::IllegalArgumentException();
    }
}

awt::Point SwXFrame::getPosition(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::RuntimeException aRuntime;
    aRuntime.Message = "position cannot be determined with this method";
    throw aRuntime;
}

void SwXFrame::setPosition(const awt::Point& /*aPosition*/) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::RuntimeException aRuntime;
    aRuntime.Message = "position cannot be changed with this method";
    throw aRuntime;
}

awt::Size SwXFrame::getSize(void) throw( uno::RuntimeException )
{
    const ::uno::Any aVal = getPropertyValue("Size");
    awt::Size* pRet =  (awt::Size*)aVal.getValue();
    return *pRet;
}

void SwXFrame::setSize(const awt::Size& aSize)
    throw( beans::PropertyVetoException, uno::RuntimeException )
{
    const ::uno::Any aVal(&aSize, ::getCppuType(static_cast<const awt::Size*>(0)));
    setPropertyValue("Size", aVal);
}

OUString SwXFrame::getShapeType(void) throw( uno::RuntimeException )
{
    return OUString("FrameShape");
}

/******************************************************************
 *  SwXTextFrame
 ******************************************************************/
SwXTextFrame::SwXTextFrame( SwDoc *_pDoc ) :
    SwXText(0, CURSOR_FRAME),
    SwXFrame(FLYCNTTYPE_FRM, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_FRAME), _pDoc )
{
}

SwXTextFrame::SwXTextFrame(SwFrmFmt& rFmt) :
    SwXText(rFmt.GetDoc(), CURSOR_FRAME),
    SwXFrame(rFmt, FLYCNTTYPE_FRM, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_FRAME))
{

}

SwXTextFrame::~SwXTextFrame()
{
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
    throw (uno::RuntimeException)
{
    ::uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXText::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextFrameBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL SwXTextFrame::getTypes(  ) throw(uno::RuntimeException)
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

namespace
{
    class theSwXTextFrameImplementationId : public rtl::Static< UnoTunnelIdInit, theSwXTextFrameImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SwXTextFrame::getImplementationId(  ) throw(uno::RuntimeException)
{
    return theSwXTextFrameImplementationId::get().getSeq();
}

uno::Reference< text::XText >  SwXTextFrame::getText(void) throw( uno::RuntimeException )
{
    return this;
}

const SwStartNode *SwXTextFrame::GetStartNode() const
{
    const SwStartNode *pSttNd = 0;

    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SwFmtCntnt& rFlyCntnt = pFmt->GetCntnt();
        if( rFlyCntnt.GetCntntIdx() )
            pSttNd = rFlyCntnt.GetCntntIdx()->GetNode().GetStartNode();
    }

    return pSttNd;
}

uno::Reference< text::XTextCursor >
SwXTextFrame::CreateCursor() throw (uno::RuntimeException)
{
    return createTextCursor();
}

uno::Reference< text::XTextCursor >  SwXTextFrame::createTextCursor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextCursor >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        //save current start node to be able to check if there is content after the table -
        //otherwise the cursor would be in the body text!
        const SwNode& rNode = pFmt->GetCntnt().GetCntntIdx()->GetNode();
        const SwStartNode* pOwnStartNode = rNode.FindSttNodeByType(SwFlyStartNode);

        SwPaM aPam(rNode);
        aPam.Move(fnMoveForward, fnGoNode);
        SwTableNode* pTblNode = aPam.GetNode()->FindTableNode();
        SwCntntNode* pCont = 0;
        while( pTblNode )
        {
            aPam.GetPoint()->nNode = *pTblNode->EndOfSectionNode();
            pCont = GetDoc()->GetNodes().GoNext(&aPam.GetPoint()->nNode);
            pTblNode = pCont->FindTableNode();
        }
        if(pCont)
            aPam.GetPoint()->nContent.Assign(pCont, 0);

        const SwStartNode* pNewStartNode =
            aPam.GetNode()->FindSttNodeByType(SwFlyStartNode);
        if(!pNewStartNode || pNewStartNode != pOwnStartNode)
        {
            uno::RuntimeException aExcept;
            aExcept.Message = "no text available";
            throw aExcept;
        }

        SwXTextCursor *const pXCursor = new SwXTextCursor(
                 *pFmt->GetDoc(), this, CURSOR_FRAME, *aPam.GetPoint());
        aRef =  static_cast<text::XWordCursor*>(pXCursor);
#if OSL_DEBUG_LEVEL > 1
        SwUnoCrsr *const pUnoCrsr = pXCursor->GetCursor();
        (void) pUnoCrsr;
#endif
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

uno::Reference< text::XTextCursor >  SwXTextFrame::createTextCursorByRange(const uno::Reference< text::XTextRange > & aTextPosition) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< text::XTextCursor >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    SwUnoInternalPaM aPam(*GetDoc());
    if (pFmt && ::sw::XTextRangeToSwPaM(aPam, aTextPosition))
    {
        SwNode& rNode = pFmt->GetCntnt().GetCntntIdx()->GetNode();
#if OSL_DEBUG_LEVEL > 1
        const SwStartNode* p1 = aPam.GetNode()->FindFlyStartNode();
        const SwStartNode* p2 = rNode.FindFlyStartNode();
        (void)p1;
        (void)p2;
#endif
        if(aPam.GetNode()->FindFlyStartNode() == rNode.FindFlyStartNode())
        {
            aRef = static_cast<text::XWordCursor*>(
                    new SwXTextCursor(*pFmt->GetDoc(), this, CURSOR_FRAME,
                        *aPam.GetPoint(), aPam.GetMark()));
        }
    }
    else
        throw uno::RuntimeException();
    return aRef;
}

uno::Reference< container::XEnumeration >  SwXTextFrame::createEnumeration(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XEnumeration >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwPosition aPos(pFmt->GetCntnt().GetCntntIdx()->GetNode());
        ::std::auto_ptr<SwUnoCrsr> pUnoCursor(
                GetDoc()->CreateUnoCrsr(aPos, false));
        pUnoCursor->Move(fnMoveForward, fnGoNode);
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable(true) ||
//          pUnoCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS ))
//          throw  uno::RuntimeException() );
        aRef = new SwXParagraphEnumeration(this, pUnoCursor, CURSOR_FRAME);
    }
    return aRef;
}

uno::Type  SwXTextFrame::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType(static_cast<uno::Reference<text::XTextRange>*>(0));
}

sal_Bool SwXTextFrame::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}

void SwXTextFrame::attach(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< text::XTextRange >  SwXTextFrame::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextFrame::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextFrame::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextFrame::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}

OUString SwXTextFrame::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTextFrame");
}

sal_Bool SwXTextFrame::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return rServiceName == "com.sun.star.text.Text" ||
           rServiceName == "com.sun.star.text.TextFrame" ||
           SwXFrame::supportsService(rServiceName);
}

uno::Sequence< OUString > SwXTextFrame::getSupportedServiceNames(void) throw( uno::RuntimeException )
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
    throw(uno::RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}

sal_Int64 SAL_CALL SwXTextFrame::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    sal_Int64 nRet = SwXFrame::getSomething( rId );
    if( !nRet )
        nRet = SwXText::getSomething( rId );

    return nRet;
}

::uno::Any SwXTextFrame::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    ::uno::Any aRet;
    if(rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_START_REDLINE))||
            rPropertyName.equalsAsciiL(SW_PROP_NAME(UNO_NAME_END_REDLINE)))
    {
        //redline can only be returned if it's a living object
        if(!IsDescriptor())
            aRet = SwXText::getPropertyValue(rPropertyName);
    }
    else
        aRet = SwXFrame::getPropertyValue(rPropertyName);
    return aRet;
}

/******************************************************************
 *  SwXTextGraphicObject
 ******************************************************************/
SwXTextGraphicObject::SwXTextGraphicObject( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_GRF, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_GRAPHIC), pDoc)
{
}

SwXTextGraphicObject::SwXTextGraphicObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_GRF, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_GRAPHIC))
{

}

SwXTextGraphicObject::~SwXTextGraphicObject()
{

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
    throw(uno::RuntimeException)
{
    ::uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextGraphicObjectBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL
    SwXTextGraphicObject::getTypes(  ) throw(uno::RuntimeException)
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

namespace
{
    class theSwXTextGraphicObjectImplementationId : public rtl::Static< UnoTunnelIdInit, theSwXTextGraphicObjectImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SwXTextGraphicObject::getImplementationId(  ) throw(uno::RuntimeException)
{
    return theSwXTextGraphicObjectImplementationId::get().getSeq();
}

void SwXTextGraphicObject::attach(const uno::Reference< text::XTextRange > & xTextRange) throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< text::XTextRange >  SwXTextGraphicObject::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextGraphicObject::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextGraphicObject::addEventListener(const uno::Reference< lang::XEventListener > & aListener)
                                                    throw( uno::RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextGraphicObject::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
                                                    throw( uno::RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}

OUString SwXTextGraphicObject::getImplementationName(void) throw( uno::RuntimeException )
{
    return OUString("SwXTextGraphicObject");
}

sal_Bool SwXTextGraphicObject::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return rServiceName == "com.sun.star.text.TextGraphicObject" ||
           SwXFrame::supportsService(rServiceName);
}

uno::Sequence< OUString > SwXTextGraphicObject::getSupportedServiceNames(void)
        throw( uno::RuntimeException )
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
        throw(uno::RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}

SwXTextEmbeddedObject::SwXTextEmbeddedObject( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_OLE, aSwMapProvider.GetPropertySet(PROPERTY_MAP_EMBEDDED_OBJECT), pDoc)
{
}

SwXTextEmbeddedObject::SwXTextEmbeddedObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_OLE, aSwMapProvider.GetPropertySet(PROPERTY_MAP_EMBEDDED_OBJECT))
{

}

SwXTextEmbeddedObject::~SwXTextEmbeddedObject()
{

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
    throw( uno::RuntimeException)
{
    ::uno::Any aRet = SwXFrame::queryInterface(aType);;
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextEmbeddedObjectBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL SwXTextEmbeddedObject::getTypes(  ) throw(uno::RuntimeException)
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

namespace
{
    class theSwXTextEmbeddedObjectImplementationId : public rtl::Static< UnoTunnelIdInit, theSwXTextEmbeddedObjectImplementationId > {};
}

uno::Sequence< sal_Int8 > SAL_CALL SwXTextEmbeddedObject::getImplementationId(  ) throw(uno::RuntimeException)
{
    return theSwXTextEmbeddedObjectImplementationId::get().getSeq();
}

void SwXTextEmbeddedObject::attach(const uno::Reference< text::XTextRange > & xTextRange) throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< text::XTextRange >  SwXTextEmbeddedObject::getAnchor(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextEmbeddedObject::dispose(void) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextEmbeddedObject::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextEmbeddedObject::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}

uno::Reference< lang::XComponent >  SwXTextEmbeddedObject::getEmbeddedObject(void) throw( uno::RuntimeException )
{
    uno::Reference< lang::XComponent >  xRet;
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        OSL_ENSURE( pCnt->GetCntntIdx() &&
                       pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetCntntIdx()
                                        ->GetIndex() + 1 ]->GetOLENode();
        uno::Reference < embed::XEmbeddedObject > xIP = pOleNode->GetOLEObj().GetOleRef();
        if ( svt::EmbeddedObjectRef::TryRunningState( xIP ) )
        {
            // TODO/LATER: the listener registered after client creation should be able to handle scaling, after that the client is not necessary here
            if ( pDoc->GetDocShell() )
                pDoc->GetDocShell()->GetIPClient( svt::EmbeddedObjectRef( xIP, embed::Aspects::MSOLE_CONTENT ) );

            xRet = uno::Reference < lang::XComponent >( xIP->getComponent(), uno::UNO_QUERY );
            uno::Reference< util::XModifyBroadcaster >  xBrdcst( xRet, uno::UNO_QUERY);
            uno::Reference< frame::XModel > xModel( xRet, uno::UNO_QUERY);
            if( xBrdcst.is() && xModel.is() )
            {
                SwXOLEListener* pListener = SwIterator<SwXOLEListener,SwFmt>::FirstElement( *pFmt );
                //create a new one if the OLE object doesn't have one already
                if( !pListener )
                {
                    uno::Reference< util::XModifyListener > xOLEListener = new SwXOLEListener(*pFmt, xModel);
                    xBrdcst->addModifyListener( xOLEListener );
                }
            }
        }
    }
    return xRet;
}

uno::Reference< embed::XEmbeddedObject > SAL_CALL SwXTextEmbeddedObject::getExtendedControlOverEmbeddedObject()
        throw( uno::RuntimeException )
{
    uno::Reference< embed::XEmbeddedObject > xResult;
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        OSL_ENSURE( pCnt->GetCntntIdx() &&
                       pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetCntntIdx()
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
            if( xBrdcst.is() && xModel.is() )
            {
                SwXOLEListener* pListener = SwIterator<SwXOLEListener,SwFmt>::FirstElement( *pFmt );
                //create a new one if the OLE object doesn't have one already
                if( !pListener )
                {
                    uno::Reference< util::XModifyListener > xOLEListener = new SwXOLEListener(*pFmt, xModel);
                    xBrdcst->addModifyListener( xOLEListener );
                }
            }
        }
    }
    return xResult;
}

sal_Int64 SAL_CALL SwXTextEmbeddedObject::getAspect() throw (uno::RuntimeException)
{
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        OSL_ENSURE( pCnt->GetCntntIdx() &&
                       pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        return pDoc->GetNodes()[ pCnt->GetCntntIdx()->GetIndex() + 1 ]->GetOLENode()->GetAspect();
    }

    return embed::Aspects::MSOLE_CONTENT; // return the default value
}

void SAL_CALL SwXTextEmbeddedObject::setAspect( sal_Int64 nAspect ) throw (uno::RuntimeException)
{
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        OSL_ENSURE( pCnt->GetCntntIdx() &&
                       pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        pDoc->GetNodes()[ pCnt->GetCntntIdx()->GetIndex() + 1 ]->GetOLENode()->SetAspect( nAspect );
    }
}

uno::Reference< graphic::XGraphic > SAL_CALL SwXTextEmbeddedObject::getReplacementGraphic() throw (uno::RuntimeException)
{
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        OSL_ENSURE( pCnt->GetCntntIdx() &&
                       pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        const Graphic* pGraphic = pDoc->GetNodes()[ pCnt->GetCntntIdx()->GetIndex() + 1 ]->GetOLENode()->GetGraphic();
        if ( pGraphic )
            return pGraphic->GetXGraphic();
    }

    return uno::Reference< graphic::XGraphic >();
}


OUString SwXTextEmbeddedObject::getImplementationName(void) throw( uno::RuntimeException )

{
    return OUString("SwXTextEmbeddedObject");
}

sal_Bool SwXTextEmbeddedObject::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return rServiceName == "com.sun.star.text.TextEmbeddedObject" ||
           SwXFrame::supportsService(rServiceName);
}

uno::Sequence< OUString > SwXTextEmbeddedObject::getSupportedServiceNames(void)
        throw( uno::RuntimeException )
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
        throw(uno::RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}

TYPEINIT1(SwXOLEListener, SwClient);

SwXOLEListener::SwXOLEListener( SwFmt& rOLEFmt, uno::Reference< XModel > xOLE) :
    SwClient(&rOLEFmt),
    xOLEModel(xOLE)
{
}

SwXOLEListener::~SwXOLEListener()
{}

void SwXOLEListener::modified( const lang::EventObject& /*rEvent*/ )
                                        throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;

    SwOLENode* pNd = 0;
    SwFmt* pFmt = GetFmt();
    if(pFmt)
    {const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
        if(pIdx)
        {
            SwNodeIndex aIdx(*pIdx, 1);
            SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
            pNd = pNoTxt->GetOLENode();
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
                        throw( uno::RuntimeException )
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
        xOLEModel = 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
