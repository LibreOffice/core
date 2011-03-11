 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

#include <com/sun/star/embed/NoVisualAreaSizeException.hpp>
#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/embed/XClassifiedObject.hpp>
#include <com/sun/star/embed/XVisualObject.hpp>
#include <com/sun/star/embed/XComponentSupplier.hpp>
#include <com/sun/star/embed/EmbedStates.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <svx/svxids.hrc>
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
#include <com/sun/star/table/BorderLine.hpp>
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
#include <editeng/brshitem.hxx>
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
#include <vos/mutex.hxx>
#include <vcl/svapp.hxx>
#include <sfx2/printer.hxx>
#include <SwStyleNameMapper.hxx>
#include <xmloff/xmlcnitm.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <tools/urlobj.hxx>
#include <editeng/frmdiritem.hxx>
// DVO, OD 01.10.2003 #i18732#
#include <fmtfollowtextflow.hxx>
// OD 2004-05-05 #i28701#
#include <fmtwrapinfluenceonobjpos.hxx>

#include <toolkit/helper/vclunohelper.hxx>

// from fefly1.cxx
extern sal_Bool lcl_ChkAndSetNewAnchor( const SwFlyFrm& rFly, SfxItemSet& rSet );

using namespace ::com::sun::star;
using ::rtl::OUString;

using ::com::sun::star::frame::XModel;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::style::XStyleFamiliesSupplier;

const sal_Char __FAR_DATA sPackageProtocol[] = "vnd.sun.star.Package:";
const sal_Char __FAR_DATA sGraphicObjectProtocol[] = "vnd.sun.star.GraphicObject:";

/****************************************************************************
    Rahmenbeschreibung
****************************************************************************/
class BaseFrameProperties_Impl
{
    SwUnoCursorHelper::SwAnyMapHelper aAnyMap;

public:
    virtual ~BaseFrameProperties_Impl();

    void            SetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any& rVal);
    sal_Bool        GetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any*& pAny );
//    void          GetProperty(const OUString &rPropertyName, const uno::Reference < beans::XPropertySet > &rxPropertySet, uno::Any& rAny );

//    const SfxItemPropertyMap*       GetMap() const {return _pMap;}
    sal_Bool                        FillBaseProperties(SfxItemSet& rToSet, const SfxItemSet &rFromSet, sal_Bool& rSizeFound);

    virtual sal_Bool                AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound) = 0;

};
/* -----------------------------12.06.01 15:46--------------------------------

 ---------------------------------------------------------------------------*/
BaseFrameProperties_Impl::~BaseFrameProperties_Impl()
{
}
/* -----------------------------12.06.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
void BaseFrameProperties_Impl::SetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any& rVal)
{
    aAnyMap.SetValue( nWID, nMemberId, rVal );
}
/* -----------------------------12.06.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool BaseFrameProperties_Impl::GetProperty(sal_uInt16 nWID, sal_uInt8 nMemberId, const uno::Any*& rpAny)
{
    return aAnyMap.FillValue( nWID, nMemberId, rpAny );
}
//void BaseFrameProperties_Impl::GetProperty( const OUString &rPropertyName, const uno::Reference < beans::XPropertySet > &rxPropertySet, uno::Any & rAny )
//{
//  rAny = rxPropertySet->getPropertyValue( rPropertyName );
//}
/* -----------------29.06.98 09:55-------------------
 *
 * --------------------------------------------------*/
sal_Bool BaseFrameProperties_Impl::FillBaseProperties(SfxItemSet& rToSet, const SfxItemSet& rFromSet, sal_Bool& rSizeFound)
{
    sal_Bool bRet = sal_True;
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
    if( pLeft || pRight || pTop ||  pBottom || pDistance ||
        pLeftDistance  || pRightDistance || pTopDistance || pBottomDistance )
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
            rSizeFound = sal_True;
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
            rSizeFound = sal_False;
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

    // DVO, OD 01.10.2003 #i18732#
    const ::uno::Any* pFollowTextFlow = 0;
    GetProperty(RES_FOLLOW_TEXT_FLOW, 0, pFollowTextFlow);
    if ( pFollowTextFlow )
    {
        SwFmtFollowTextFlow aFmtFollowTextFlow;
        aFmtFollowTextFlow.PutValue(*pFollowTextFlow, 0);
        rToSet.Put(aFmtFollowTextFlow);
    }

    // OD 2004-05-04 #i28701# - RES_WRAP_INFLUENCE_ON_OBJPOS
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
/* -----------------22.06.98 09:17-------------------
 *
 * --------------------------------------------------*/

class SwFrameProperties_Impl : public BaseFrameProperties_Impl
{
protected:
//    SwFrameProperties_Impl(/*const :: SfxItemPropertyMap* pMap*/) :
//        BaseFrameProperties_Impl(/*pMap*/){}
public:
    SwFrameProperties_Impl();
    virtual ~SwFrameProperties_Impl(){}

    virtual sal_Bool        AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound);
};
/* -----------------22.06.98 09:17-------------------
 *
 * --------------------------------------------------*/
SwFrameProperties_Impl::SwFrameProperties_Impl():
    BaseFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME)*/ )
{
}
/* -----------------22.06.98 11:27-------------------
 *
 * --------------------------------------------------*/
inline void lcl_FillCol ( SfxItemSet &rToSet, const :: SfxItemSet &rFromSet, const :: uno::Any *pAny)
{
    if ( pAny )
    {
        SwFmtCol aCol ( static_cast < const :: SwFmtCol & > ( rFromSet.Get ( RES_COL ) ) );
        ((SfxPoolItem&)aCol).PutValue( *pAny, MID_COLUMNS);
        rToSet.Put(aCol);
    }
}
sal_Bool    SwFrameProperties_Impl::AnyToItemSet(SwDoc *pDoc, SfxItemSet& rSet, SfxItemSet&, sal_Bool& rSizeFound)
{
    //Properties fuer alle Frames
    const ::uno::Any *pStyleName;
    SwDocStyleSheet* pStyle = NULL;
    sal_Bool bRet;

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
           bRet = FillBaseProperties( rSet, *pItemSet, rSizeFound );
        lcl_FillCol ( rSet, *pItemSet, pColumns );
    }
    else
    {
        const :: SfxItemSet *pItemSet = &pDoc->GetFrmFmtFromPool( RES_POOLFRM_FRAME )->GetAttrSet();
           bRet = FillBaseProperties( rSet, *pItemSet, rSizeFound );
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

    virtual sal_Bool                AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound);
};
/* -----------------27.06.98 14:53-------------------
 *
 * --------------------------------------------------*/
SwGraphicProperties_Impl::SwGraphicProperties_Impl( ) :
    BaseFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC)*/ )
{
}

/* -----------------27.06.98 14:40-------------------
 *
 * --------------------------------------------------*/
inline void lcl_FillMirror ( SfxItemSet &rToSet, const :: SfxItemSet &rFromSet, const ::uno::Any *pHEvenMirror, const ::uno::Any *pHOddMirror, const ::uno::Any *pVMirror, sal_Bool &rRet )
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

sal_Bool    SwGraphicProperties_Impl::AnyToItemSet(
            SwDoc* pDoc,
            SfxItemSet& rFrmSet,
            SfxItemSet& rGrSet,
            sal_Bool& rSizeFound)
{
    //Properties fuer alle Frames
    sal_Bool bRet;
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
        bRet = FillBaseProperties(rFrmSet, *pItemSet, rSizeFound);
        lcl_FillMirror ( rGrSet, *pItemSet, pHEvenMirror, pHOddMirror, pVMirror, bRet );
    }
    else
    {
        const :: SfxItemSet *pItemSet = &pDoc->GetFrmFmtFromPool( RES_POOLFRM_GRAPHIC )->GetAttrSet();
        bRet = FillBaseProperties(rFrmSet, *pItemSet, rSizeFound);
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
/* -----------------4/1/2003 13:54-------------------

 --------------------------------------------------*/

class SwOLEProperties_Impl : public SwFrameProperties_Impl
{
public:
    SwOLEProperties_Impl() :
        SwFrameProperties_Impl(/*aSwMapProvider.GetPropertyMap(PROPERTY_MAP_EMBEDDED_OBJECT)*/ ){}
    virtual ~SwOLEProperties_Impl(){}

    virtual sal_Bool        AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound);
};
/* -----------------4/1/2003 15:32-------------------

 --------------------------------------------------*/

sal_Bool  SwOLEProperties_Impl::AnyToItemSet(
        SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound)
{
    const ::uno::Any* pTemp;
    if(!GetProperty(FN_UNO_CLSID, 0, pTemp) && !GetProperty(FN_UNO_STREAM_NAME, 0, pTemp) )
        return sal_False;
    SwFrameProperties_Impl::AnyToItemSet( pDoc, rFrmSet, rSet, rSizeFound);
    //
    return sal_True;
}

/******************************************************************
 *  SwXFrame
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const :: uno::Sequence< sal_Int8 > & SwXFrame::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXFrame::getSomething( const :: uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >(this) );
    }
    return 0;
}
/*-----------------24.03.98 14:49-------------------

--------------------------------------------------*/

TYPEINIT1(SwXFrame, SwClient);
/* -----------------------------06.04.00 14:20--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFrame::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXFrame");
}
/* -----------------------------06.04.00 14:20--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXFrame::supportsService(const :: OUString& rServiceName) throw( uno::RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.BaseFrame")||
                !rServiceName.compareToAscii("com.sun.star.text.TextContent") ||
                    !rServiceName.compareToAscii("com.sun.star.document.LinkTarget");
}
/* -----------------------------06.04.00 14:20--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXFrame::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.BaseFrame");
    pArray[1] = C2U("com.sun.star.text.TextContent");
    pArray[2] = C2U("com.sun.star.document.LinkTarget");
    return aRet;
}


/*-- 14.01.99 11:31:52---------------------------------------------------
    Dieser CTor legt den Frame als Descriptor an
  -----------------------------------------------------------------------*/
SwXFrame::SwXFrame(FlyCntType eSet, const :: SfxItemPropertySet* pSet, SwDoc *pDoc) :
    aLstnrCntnr( (container::XNamed*)this),
    m_pPropSet(pSet),
    m_pDoc ( pDoc ),
    eType(eSet),
    bIsDescriptor(sal_True),
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
    const ::uno::Any aAny = xFamilies->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "FrameStyles" ) ) );
    aAny >>= mxStyleFamily;
    // In the derived class, we'll ask mxStyleFamily for the relevant default style
    // mxStyleFamily is initialised in the SwXFrame constructor
    switch(eType)
    {
        case FLYCNTTYPE_FRM:
        {
            uno::Any aAny2 = mxStyleFamily->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Frame" ) ) );
            aAny2 >>= mxStyleData;
            pProps = new SwFrameProperties_Impl( );
        }
        break;
        case FLYCNTTYPE_GRF:
        {
            uno::Any aAny2 = mxStyleFamily->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Graphics" ) ) );
            aAny2 >>= mxStyleData;
            pProps = new SwGraphicProperties_Impl( );
        }
        break;
        case FLYCNTTYPE_OLE:
        {
            uno::Any aAny2 = mxStyleFamily->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "OLE" ) ) );
            aAny2 >>= mxStyleData;
            pProps = new SwOLEProperties_Impl( );
        }
        break;

        default:
            ;
    }
}

/*-- 11.12.98 15:05:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFrame::SwXFrame(SwFrmFmt& rFrmFmt, FlyCntType eSet, const :: SfxItemPropertySet* pSet) :
    SwClient( &rFrmFmt ),
    aLstnrCntnr( (container::XNamed*)this),
    m_pPropSet(pSet),
    m_pDoc( 0 ),
    eType(eSet),
    pProps(0),
    bIsDescriptor(sal_False),
    m_pCopySource(0)
{

}
/*-- 11.12.98 15:05:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFrame::~SwXFrame()
{
    delete m_pCopySource;
    delete pProps;
}
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXFrame::getName(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    String sRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
        sRet = pFmt->GetName();
    else if(bIsDescriptor)
        sRet = sName;
    else
        throw uno::RuntimeException();
    return sRet;
}
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setName(const :: OUString& rName) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    String sTmpName(rName);
    if(pFmt)
    {
        pFmt->GetDoc()->SetFlyName((SwFlyFrmFmt&)*pFmt, sTmpName);
        if(pFmt->GetName() != sTmpName)
        {
            throw uno::RuntimeException();
        }
    }
    else if(bIsDescriptor)
        sName = sTmpName;
    else
        throw uno::RuntimeException();
}
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 15.05.06 12:21:43---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::SetSelection(SwPaM& rCopySource)
{
    if(m_pCopySource)
        delete m_pCopySource;
    m_pCopySource = new SwPaM( *rCopySource.Start() );
    m_pCopySource->SetMark();
    *m_pCopySource->GetMark() = *rCopySource.End();
}
/*-- 11.12.98 15:05:04---------------------------------------------------

  -----------------------------------------------------------------------*/

SdrObject *SwXFrame::GetOrCreateSdrObject( SwFlyFrmFmt *pFmt )
{
    SdrObject* pObject = pFmt->FindSdrObject();
    if( !pObject )
    {
        SwDoc *pDoc = pFmt->GetDoc();
        // --> OD 2005-08-08 #i52858# - method name changed
        SdrModel *pDrawModel = pDoc->GetOrCreateDrawModel();
        // <--
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

SwFrmFmt *lcl_GetFrmFmt( const :: uno::Any& rValue, SwDoc *pDoc )
{
    SwFrmFmt *pRet = 0;
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if(pDocSh)
    {
        OUString uTemp;
        rValue >>= uTemp;
        String sStyle;
        SwStyleNameMapper::FillUIName(String (uTemp), sStyle, nsSwGetPoolIdFromName::GET_POOLID_FRMFMT, sal_True );
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
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    const :: SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap()->getByName(rPropertyName);

    if (!pEntry)
        throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(pFmt)
    {
        sal_Bool bNextFrame = sal_False;
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw beans::PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        SwDoc* pDoc = pFmt->GetDoc();
        if ((eType == FLYCNTTYPE_GRF) && isGRFATR(pEntry->nWID) ||
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
        // --> OD 2009-07-13 #i73249#
        // Attribute AlternativeText was never published.
        // Now it has been replaced by Attribute Title - valid for all <SwXFrame> instances
//        else if( FN_UNO_ALTERNATIVE_TEXT == pEntry->nWID && eType != FLYCNTTYPE_FRM )
//        {
//            const :: SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
//            if(pIdx)
//            {
//                SwNodeIndex aIdx(*pIdx, 1);
//                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
//                OUString uTemp;
//                aValue >>= uTemp;
//                pNoTxt->SetAlternateText(uTemp);
//            }
//        }
        // New attribute Title
        else if( FN_UNO_TITLE == pEntry->nWID )
        {
            SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
            ASSERT( pFmt,
                    "unexpected type of <pFmt> --> crash" );
            OUString uTemp;
            aValue >>= uTemp;
            const String sTitle(uTemp);
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject( pFlyFmt );
            pFlyFmt->GetDoc()->SetFlyFrmTitle( *(pFlyFmt), sTitle );
        }
        // New attribute Description
        else if( FN_UNO_DESCRIPTION == pEntry->nWID )
        {
            SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
            ASSERT( pFmt,
                    "unexpected type of <pFmt> --> crash" );
            OUString uTemp;
            aValue >>= uTemp;
            const String sDescription(uTemp);
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject( pFlyFmt );
            pFlyFmt->GetDoc()->SetFlyFrmDescription( *(pFlyFmt), sDescription );
        }
        // <--
        else if(FN_UNO_FRAME_STYLE_NAME == pEntry->nWID)
        {
            SwFrmFmt *pFrmFmt = lcl_GetFrmFmt( aValue, pFmt->GetDoc() );
            if( pFrmFmt )
            {
                UnoActionContext aAction(pFmt->GetDoc());

                SfxItemSet* pSet = 0;
                // --> OD 2004-08-13 #i31771#, #i25798# - No adjustment of
                // anchor ( no call of method <::lcl_ChkAndSetNewAnchor(..)> ),
                // if document is currently in reading mode.
                if ( !pFmt->GetDoc()->IsInReading() )
                {
                    // see SwFEShell::SetFrmFmt( SwFrmFmt *pNewFmt, sal_Bool bKeepOrient, Point* pDocPos )
                    SwFlyFrm *pFly = 0;
                    {
                        const :: SwFrmFmt* pFmtXX = pFmt;
                        if (PTR_CAST(SwFlyFrmFmt, pFmtXX))
                            pFly = ((SwFlyFrmFmt*)pFmtXX)->GetFrm();
                    }
                    if ( pFly )
                    {
                        const :: SfxPoolItem* pItem;
                        if( SFX_ITEM_SET == pFrmFmt->GetItemState( RES_ANCHOR, sal_False, &pItem ))
                        {
                            pSet = new SfxItemSet( pDoc->GetAttrPool(), aFrmFmtSetRange );
                            pSet->Put( *pItem );
                            if ( !::lcl_ChkAndSetNewAnchor( *pFly, *pSet ) )
                                delete pSet, pSet = 0;
                        }
                    }
                }
                // <--

                pFmt->GetDoc()->SetFrmFmtToFly( *pFmt, *pFrmFmt, pSet, sal_False );
                delete pSet;
            }
            else
                throw lang::IllegalArgumentException();
        }
        else if( FN_UNO_GRAPHIC_U_R_L == pEntry->nWID ||
                FN_UNO_GRAPHIC_FILTER == pEntry->nWID)
        {
            String sGrfName, sFltName;
            GraphicObject *pGrfObj = 0;
            pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, &sGrfName, &sFltName );
            OUString uTemp;
            aValue >>= uTemp;
            String sTmp(uTemp);
            UnoActionContext aAction(pFmt->GetDoc());
            if(FN_UNO_GRAPHIC_U_R_L == pEntry->nWID)
            {
                if( sTmp.EqualsAscii( sPackageProtocol,
                                      0, sizeof( sPackageProtocol )-1 ) )
                {
                    pGrfObj = new GraphicObject;
                    pGrfObj->SetUserData( sTmp );
                    pGrfObj->SetSwapState();
                    sGrfName.Erase();
                }
                else if( sTmp.EqualsAscii( sGraphicObjectProtocol,
                                           0, sizeof(sGraphicObjectProtocol)-1 ) )
                {
                    ByteString sId( sTmp.Copy(sizeof(sGraphicObjectProtocol)-1),
                                    RTL_TEXTENCODING_ASCII_US );
                    pGrfObj = new GraphicObject( sId );
                    sGrfName.Erase();
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
                    pFmt->GetDoc()->ReRead( aGrfPaM, String(), String(), &aGraphic, 0 );
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
                ::rtl::OUString aGraphicProtocol( RTL_CONSTASCII_USTRINGPARAM( sGraphicObjectProtocol ) );
                if( aGrfUrl.compareTo( aGraphicProtocol, aGraphicProtocol.getLength() ) == 0 )
                {
                    ByteString sId( aGrfUrl.copy(sizeof(sGraphicObjectProtocol)-1).getStr(), RTL_TEXTENCODING_ASCII_US );
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
                        svt::EmbeddedObjectRef xObj = pOleNode->GetOLEObj().GetObject();

                        ::rtl::OUString aMediaType;
                        xObj.SetGraphic( aGraphic, aMediaType );
                    }
                }
            }
        }
        else if(0 != (bNextFrame = (rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CHAIN_NEXT_NAME))))
            || rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_CHAIN_PREV_NAME)))
        {
            OUString uTemp;
            aValue >>= uTemp;
            String sChainName(uTemp);
            if(!sChainName.Len())
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
            sal_Bool bDone = sal_False;
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
                    bDone = sal_True;
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
                        aExcept.Message = C2U("Anchor to frame: no frame found");
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

                // --> OD 2004-08-13 #i31771#, #i25798# - No adjustment of
                // anchor ( no call of method <::lcl_ChkAndSetNewAnchor(..)> ),
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
                        if( SFX_ITEM_SET == aSet.GetItemState( RES_ANCHOR, sal_False, &pItem ))
                        {
                            aSet.Put( *pItem );
                            ::lcl_ChkAndSetNewAnchor( *pFly, aSet );
                        }
                    }
                }
                // <--

                pFmt->GetDoc()->SetFlyFrmAttr( *pFmt, aSet );
            }
            else if(FN_UNO_CLSID == pEntry->nWID || FN_UNO_STREAM_NAME == pEntry->nWID)
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
/*-- 11.12.98 15:05:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXFrame::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aAny;
    SwFrmFmt* pFmt = GetFrmFmt();
    const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap()->getByName(rPropertyName);
    if (!pEntry)
        throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

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
            String sGrfName;
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
//              SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                    throw uno::RuntimeException();
                if( pGrfNode->IsGrfLink() )
                {
                    pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, &sGrfName, 0 );
                }
                else
                {
                    String sPrefix( RTL_CONSTASCII_STRINGPARAM(sGraphicObjectProtocol) );
                    String sId( pGrfNode->GetGrfObj().GetUniqueID(),
                                RTL_TEXTENCODING_ASCII_US );
                    (sGrfName = sPrefix) += sId;
                }
            }
            aAny <<= OUString(sGrfName);
        }
        else if( FN_UNO_GRAPHIC_FILTER == pEntry->nWID )
        {
            String sFltName;
            pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, 0, &sFltName );
                aAny <<= OUString(sFltName);
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
        // --> OD 2009-07-13 #i73249#
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
            ASSERT( pFmt,
                    "unexpected type of <pFmt> --> crash" );
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject( pFlyFmt );
            aAny <<= OUString(pFlyFmt->GetObjTitle());
        }
        // New attribute Description
        else if( FN_UNO_DESCRIPTION == pEntry->nWID )
        {
            SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
            ASSERT( pFmt,
                    "unexpected type of <pFmt> --> crash" );
            // assure that <SdrObject> instance exists.
            GetOrCreateSdrObject( pFlyFmt );
            aAny <<= OUString(pFlyFmt->GetObjDescription());
        }
        // <--
        else if(eType == FLYCNTTYPE_GRF &&
                (rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ACTUAL_SIZE))))
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
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
                            pGrfNode->SwapIn( TRUE );
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
            aAny <<= OUString(pFmt->GetName());
        }
        else if(FN_UNO_Z_ORDER == pEntry->nWID)
        {
            const SdrObject* pObj = pFmt->FindRealSdrObject();
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
            DBG_ASSERT( pCnt->GetCntntIdx() &&
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
                            ASSERT( pDoc->GetDocShell(), "no doc shell => no client site" );
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
                aAny <<= ::rtl::OUString(pOleNode->GetOLEObj().GetCurrentPersistName());
            }
        }
        else if(WID_LAYOUT_SIZE == pEntry->nWID)
        {
            // format document completely in order to get correct value
            pFmt->GetDoc()->GetEditShell()->CalcLayout();

            SwClientIter aIter( *pFmt );
            SwClient* pC = aIter.First( TYPE( SwFrm ) );
            if (pC)
            {
                SwFrm *pTmpFrm = static_cast< SwFrm * >(pC);
                DBG_ASSERT( pTmpFrm->IsValid(), "frame not valid" );
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
/*-- 11.12.98 15:05:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::addPropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 11.12.98 15:05:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::removePropertyChangeListener(const OUString& /*PropertyName*/,
    const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 11.12.98 15:05:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::addVetoableChangeListener(const OUString& /*PropertyName*/,
                                const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 11.12.98 15:05:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
        throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    DBG_WARNING("not implemented");
}
/*-- 12.09.00 14:04:53---------------------------------------------------

  -----------------------------------------------------------------------*/
beans::PropertyState SwXFrame::getPropertyState( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< OUString > aPropertyNames(1);
    OUString* pNames = aPropertyNames.getArray();
    pNames[0] = rPropertyName;
    uno::Sequence< beans::PropertyState > aStates = getPropertyStates(aPropertyNames);
    return aStates.getConstArray()[0];
}
/*-- 12.09.00 14:04:54---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< beans::PropertyState > SwXFrame::getPropertyStates(
    const uno::Sequence< OUString >& aPropertyNames )
        throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< beans::PropertyState > aStates(aPropertyNames.getLength());
    beans::PropertyState* pStates = aStates.getArray();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const OUString* pNames = aPropertyNames.getConstArray();
        const SwAttrSet& rFmtSet = pFmt->GetAttrSet();
        for(int i = 0; i < aPropertyNames.getLength(); i++)
        {
            const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap()->getByName(pNames[i]);
            if (!pEntry)
                throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[i], static_cast < cppu::OWeakObject * > ( this ) );

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
/*-- 12.09.00 14:04:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setPropertyToDefault( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap()->getByName(rPropertyName);
        if (!pEntry)
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        if ( pEntry->nFlags & beans::PropertyAttribute::READONLY)
            throw uno::RuntimeException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "setPropertyToDefault: property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        sal_Bool bNextFrame;
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
            // --> OD 2009-07-13 #i73249#
            // Attribute AlternativeText was never published.
            // Now it has been replaced by Attribute Title - valid for all <SwXFrame> instances
//            else if( eType != FLYCNTTYPE_FRM && FN_UNO_ALTERNATIVE_TEXT == pEntry->nWID )
//            {
//                const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
//                if(pIdx)
//                {
//                    SwNodeIndex aIdx(*pIdx, 1);
//                    SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
//                    pNoTxt->SetAlternateText(aEmptyStr);
//                }
//            }
            // New attribute Title
            else if( FN_UNO_TITLE == pEntry->nWID )
            {
                SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
                ASSERT( pFmt,
                        "unexpected type of <pFmt> --> crash" );
                // assure that <SdrObject> instance exists.
                GetOrCreateSdrObject( pFlyFmt );
                pFlyFmt->GetDoc()->SetFlyFrmTitle( *(pFlyFmt), aEmptyStr );
            }
            // New attribute Description
            else if( FN_UNO_DESCRIPTION == pEntry->nWID )
            {
                SwFlyFrmFmt* pFlyFmt = dynamic_cast<SwFlyFrmFmt*>(pFmt);
                ASSERT( pFmt,
                        "unexpected type of <pFmt> --> crash" );
                // assure that <SdrObject> instance exists.
                GetOrCreateSdrObject( pFlyFmt );
                pFlyFmt->GetDoc()->SetFlyFrmDescription( *(pFlyFmt), aEmptyStr );
            }
            // <--
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
/*-- 12.09.00 14:04:55---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXFrame::getPropertyDefault( const OUString& rPropertyName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertySimpleEntry* pEntry = m_pPropSet->getPropertyMap()->getByName(rPropertyName);
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
            throw beans::UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else if(!IsDescriptor())
        throw uno::RuntimeException();
    return aRet;
}
/* -----------------22.04.99 14:59-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn())
        throw uno::RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/* -----------------22.04.99 14:59-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw uno::RuntimeException();
}
/*-- 11.12.98 15:05:06---------------------------------------------------

  -----------------------------------------------------------------------*/
void    SwXFrame::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        mxStyleData.clear();
        mxStyleFamily.clear();
        m_pDoc = 0;
        aLstnrCntnr.Disposing();
    }
}

/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
                const xub_StrLen nIdx = rPos.nContent.GetIndex();
                pTxtNode->DeleteAttributes( RES_TXTATR_FLYCNT, nIdx, nIdx );
            }
            else
                pFmt->GetDoc()->DelLayoutFmt(pFmt);
        }
    }

}
/*-- 11.12.98 16:02:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXFrame::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/* -----------------14.01.99 12:02-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::ResetDescriptor()
{
    bIsDescriptor = sal_False;
    mxStyleData.clear();
    mxStyleFamily.clear();
    DELETEZ(pProps);
}
/* -----------------18.02.99 13:34-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::attachToRange(const uno::Reference< text::XTextRange > & xTextRange)
            throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
        //das muss jetzt sal_True liefern
        ::sw::XTextRangeToSwPaM(aIntPam, xTextRange);

        SwNode& rNode = pDoc->GetNodes().GetEndOfContent();
        SwPaM aPam(rNode);
        aPam.Move( fnMoveBackward, fnGoDoc );
        static sal_uInt16 __READONLY_DATA aFrmAttrRange[] =
        {
            RES_FRMATR_BEGIN,       RES_FRMATR_END-1,
            SID_ATTR_BORDER_INNER,  SID_ATTR_BORDER_INNER,
            RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
            0
        };
        static sal_uInt16 __READONLY_DATA aGrAttrRange[] =
        {
            RES_GRFATR_BEGIN,       RES_GRFATR_END-1,
            0
        };
        SfxItemSet aGrSet(pDoc->GetAttrPool(), aGrAttrRange );

        SfxItemSet aFrmSet(pDoc->GetAttrPool(), aFrmAttrRange );
        //jetzt muessen die passenden Items in den Set
        sal_Bool bSizeFound;
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
        if(SFX_ITEM_SET == aFrmSet.GetItemState(RES_ANCHOR, sal_False, &pItem) )
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
                if(sName.Len())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFmt, sName);
            }
            //den SwXText wecken
            ((SwXTextFrame*)this)->SetDoc( bIsDescriptor ? m_pDoc : GetFrmFmt()->GetDoc() );
        }
        else if( eType == FLYCNTTYPE_GRF)
        {
            UnoActionContext aCont(pDoc);
            const ::uno::Any* pGraphicURL;
            String sGraphicURL;
            GraphicObject *pGrfObj = 0;
            if(pProps->GetProperty(FN_UNO_GRAPHIC_U_R_L, 0, pGraphicURL))
            {
                OUString uTemp;
                (*pGraphicURL) >>= uTemp;
                sGraphicURL = String(uTemp);
                if( sGraphicURL.EqualsAscii( sPackageProtocol,
                                               0, sizeof( sPackageProtocol )-1 ) )
                {
                    pGrfObj = new GraphicObject;
                    pGrfObj->SetUserData( sGraphicURL );
                    pGrfObj->SetSwapState();
                    sGraphicURL.Erase();
                }
                else if( sGraphicURL.EqualsAscii( sGraphicObjectProtocol,
                                       0, sizeof(sGraphicObjectProtocol)-1 ) )
                {
                    ByteString sId( sGraphicURL.Copy( sizeof(sGraphicObjectProtocol)-1 ),
                                    RTL_TEXTENCODING_ASCII_US );
                    pGrfObj = new GraphicObject( sId );
                    sGraphicURL.Erase();
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

            String sFltName;
            const ::uno::Any* pFilter;
            if(pProps->GetProperty(FN_UNO_GRAPHIC_FILTER, 0, pFilter))
            {
                OUString uTemp;
                (*pFilter) >>= uTemp;
                sFltName = String(uTemp);
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
                if(sName.Len())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFmt, sName);

            }
            const ::uno::Any* pSurroundContour;
            if(pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUR, pSurroundContour))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SURROUND_CONTOUR)), *pSurroundContour);
            const ::uno::Any* pContourOutside;
            if(pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUROUTSIDE, pContourOutside))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_CONTOUR_OUTSIDE)), *pContourOutside);
            const ::uno::Any* pContourPoly;
            if(pProps->GetProperty(FN_PARAM_COUNTOUR_PP, 0, pContourPoly))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_CONTOUR_POLY_POLYGON)), *pContourPoly);
            const ::uno::Any* pPixelContour;
            if(pProps->GetProperty(FN_UNO_IS_PIXEL_CONTOUR, 0, pPixelContour))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_IS_PIXEL_CONTOUR)), *pPixelContour);
            const ::uno::Any* pAutoContour;
            if(pProps->GetProperty(FN_UNO_IS_AUTOMATIC_CONTOUR, 0, pAutoContour))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_IS_AUTOMATIC_CONTOUR)), *pAutoContour);
//            const ::uno::Any* pAltText;
//            if(pProps->GetProperty(FN_UNO_ALTERNATIVE_TEXT, 0, pAltText))
//                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_ALTERNATIVE_TEXT)), *pAltText);
        }
        else
        {
            const ::uno::Any* pCLSID = 0;
            const ::uno::Any* pStreamName = 0;
            if(!pProps->GetProperty(FN_UNO_CLSID, 0, pCLSID) && !pProps->GetProperty( FN_UNO_STREAM_NAME, 0, pStreamName ))
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
                        aExcept.Message = OUString::createFromAscii("CLSID invalid");
                        throw aExcept;
                    }

                    pCnt.reset( new comphelper::EmbeddedObjectContainer );
                    ::rtl::OUString aName;
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
                    ASSERT( pFmt2, "Doc->Insert(notxt) failed." );

                    pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
                    pFmt2->Add(this);
                    if(sName.Len())
                        pDoc->SetFlyName((SwFlyFrmFmt&)*pFmt2, sName);
                }
            }
            else if( pStreamName )
            {
                ::rtl::OUString sStreamName;
                (*pStreamName) >>= sStreamName;
                pDoc->GetIDocumentUndoRedo().StartUndo(UNDO_INSERT, NULL);

                SwFlyFrmFmt* pFrmFmt = 0;
                pFrmFmt = pDoc->InsertOLE( aPam, sStreamName, embed::Aspects::MSOLE_CONTENT, &aFrmSet, NULL, NULL );
                pDoc->GetIDocumentUndoRedo().EndUndo(UNDO_INSERT, NULL);
                pFrmFmt->Add(this);
                if(sName.Len())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFrmFmt, sName);
            }
        }
        if( pFmt && pDoc->GetDrawModel() )
            GetOrCreateSdrObject( pFmt );
        const ::uno::Any* pOrder;
        if( pProps->GetProperty(FN_UNO_Z_ORDER, 0, pOrder) )
            setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_Z_ORDER)), *pOrder);
        const ::uno::Any* pReplacement;
        if( pProps->GetProperty(FN_UNO_REPLACEMENT_GRAPHIC, 0, pReplacement) )
            setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_GRAPHIC)), *pReplacement);
        // --> OD 2009-07-13 #i73249#
        // new attribute Title
        const ::uno::Any* pTitle;
        if ( pProps->GetProperty(FN_UNO_TITLE, 0, pTitle) )
        {
            setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_TITLE)), *pTitle);
        }
        // new attribute Description
        const ::uno::Any* pDescription;
        if ( pProps->GetProperty(FN_UNO_DESCRIPTION, 0, pDescription) )
        {
            setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_DESCRIPTION)), *pDescription);
        }
        // <--
    }
    else
        throw lang::IllegalArgumentException();
    //setzt das Flag zurueck und loescht den Descriptor-Pointer
    ResetDescriptor();
}
/* -----------------------------04.04.01 14:27--------------------------------

 ---------------------------------------------------------------------------*/
void SwXFrame::attach(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SwFrmFmt* pFmt;
    if(IsDescriptor())
        attachToRange(xTextRange);
    else if(0 != (pFmt = GetFrmFmt()))
    {
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
        SwDoc* pDoc = pFmt->GetDoc();
        SwUnoInternalPaM aIntPam(*pDoc);
        if (::sw::XTextRangeToSwPaM(aIntPam, xTextRange))
        {
            SfxItemSet aSet( pDoc->GetAttrPool(),
                        RES_ANCHOR, RES_ANCHOR );
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
/*-- 22.04.99 08:03:20---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Point SwXFrame::getPosition(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::RuntimeException aRuntime;
    aRuntime.Message = C2U("position cannot be determined with this method");
    throw aRuntime;
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setPosition(const awt::Point& /*aPosition*/) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::RuntimeException aRuntime;
    aRuntime.Message = C2U("position cannot be changed with this method");
    throw aRuntime;
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Size SwXFrame::getSize(void) throw( uno::RuntimeException )
{
    const ::uno::Any aVal = getPropertyValue(C2U("Size"));
    awt::Size* pRet =  (awt::Size*)aVal.getValue();
    return *pRet;
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setSize(const awt::Size& aSize)
    throw( beans::PropertyVetoException, uno::RuntimeException )
{
    const ::uno::Any aVal(&aSize, ::getCppuType(static_cast<const awt::Size*>(0)));
    setPropertyValue(C2U("Size"), aVal);
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXFrame::getShapeType(void) throw( uno::RuntimeException )
{
    return C2U("FrameShape");
}


/******************************************************************
 *  SwXTextFrame
 ******************************************************************/
/*-- 14.01.99 11:27:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFrame::SwXTextFrame( SwDoc *_pDoc ) :
    SwXText(0, CURSOR_FRAME),
    SwXFrame(FLYCNTTYPE_FRM, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_FRAME), _pDoc )
{
}
/*-- 11.12.98 15:23:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFrame::SwXTextFrame(SwFrmFmt& rFmt) :
    SwXText(rFmt.GetDoc(), CURSOR_FRAME),
    SwXFrame(rFmt, FLYCNTTYPE_FRM, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_FRAME))
{

}
/*-- 11.12.98 15:23:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFrame::~SwXTextFrame()
{
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextFrame::acquire(  )throw()
{
    SwXFrame::acquire();
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextFrame::release(  )throw()
{
    SwXFrame::release();
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXTextFrame::getImplementationId(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XText >  SwXTextFrame::getText(void) throw( uno::RuntimeException )
{
    return this;
}
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXTextFrame::createTextCursor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
            aExcept.Message = S2U("no text available");
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
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXTextFrame::createTextCursorByRange(const uno::Reference< text::XTextRange > & aTextPosition) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXTextFrame::createEnumeration(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< container::XEnumeration >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwPosition aPos(pFmt->GetCntnt().GetCntntIdx()->GetNode());
        ::std::auto_ptr<SwUnoCrsr> pUnoCursor(
                GetDoc()->CreateUnoCrsr(aPos, sal_False));
        pUnoCursor->Move(fnMoveForward, fnGoNode);
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS ))
//          throw  uno::RuntimeException() );
        aRef = new SwXParagraphEnumeration(this, pUnoCursor, CURSOR_FRAME);
    }
    return aRef;
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXTextFrame::getElementType(void) throw( uno::RuntimeException )
{
    return ::getCppuType(static_cast<uno::Reference<text::XTextRange>*>(0));
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextFrame::hasElements(void) throw( uno::RuntimeException )
{
    return sal_True;
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::attach(const uno::Reference< text::XTextRange > & xTextRange)
    throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SwXFrame::attach(xTextRange);
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextFrame::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXFrame::getAnchor();
}
/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXFrame::dispose();
}
/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}
/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextFrame::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextFrame");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextFrame::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.Text")||
            COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextFrame")||
                    SwXFrame::supportsService(rServiceName);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextFrame::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 2);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 2] = C2U("com.sun.star.text.TextFrame");
    pArray[aRet.getLength() - 1] = C2U("com.sun.star.text.Text");
    return aRet;
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void * SAL_CALL SwXTextFrame::operator new( size_t t) throw()
{
    return SwXTextFrameBaseClass::operator new( t);
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextFrame::operator delete( void * p) throw()
{
    SwXTextFrameBaseClass::operator delete(p);
}

uno::Reference<container::XNameReplace > SAL_CALL SwXTextFrame::getEvents()
    throw(uno::RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}
/* -----------------------------10.01.01 13:27--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextFrame::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    sal_Int64 nRet = SwXFrame::getSomething( rId );
    if( !nRet )
        nRet = SwXText::getSomething( rId );

    return nRet;
}
/* -----------------------------19.03.2002 16:43------------------------------

 ---------------------------------------------------------------------------*/
::uno::Any SwXTextFrame::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/*-- 14.01.99 11:27:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextGraphicObject::SwXTextGraphicObject( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_GRF, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_GRAPHIC), pDoc)
{
}
/*-- 11.12.98 16:02:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextGraphicObject::SwXTextGraphicObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_GRF, aSwMapProvider.GetPropertySet(PROPERTY_MAP_TEXT_GRAPHIC))
{

}
/*-- 11.12.98 16:02:26---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextGraphicObject::~SwXTextGraphicObject()
{

}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextGraphicObject::acquire(  )throw()
{
    SwXFrame::acquire();
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextGraphicObject::release(  )throw()
{
    SwXFrame::release();
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
::uno::Any SAL_CALL SwXTextGraphicObject::queryInterface( const uno::Type& aType )
    throw(uno::RuntimeException)
{
    ::uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextGraphicObjectBaseClass::queryInterface(aType);
    return aRet;
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXTextGraphicObject::getImplementationId(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}
/*-- 11.12.98 16:02:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::attach(const uno::Reference< text::XTextRange > & xTextRange) throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SwXFrame::attach(xTextRange);
}
/*-- 11.12.98 16:02:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextGraphicObject::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXFrame::getAnchor();
}
/*-- 11.12.98 16:02:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXFrame::dispose();
}
/*-- 11.12.98 16:02:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::addEventListener(const uno::Reference< lang::XEventListener > & aListener)
                                                    throw( uno::RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}
/*-- 11.12.98 16:02:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::removeEventListener(const uno::Reference< lang::XEventListener > & aListener)
                                                    throw( uno::RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextGraphicObject::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SwXTextGraphicObject");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextGraphicObject::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextGraphicObject") ||
                    SwXFrame::supportsService(rServiceName);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextGraphicObject::getSupportedServiceNames(void)
        throw( uno::RuntimeException )
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 1);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 1] = C2U("com.sun.star.text.TextGraphicObject");
    return aRet;
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void * SAL_CALL SwXTextGraphicObject::operator new( size_t t) throw()
{
    return SwXTextGraphicObjectBaseClass::operator new(t);
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextGraphicObject::operator delete( void * p) throw()
{
    SwXTextGraphicObjectBaseClass::operator delete(p);
}
/* -----------------------------15.12.00 12:45--------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference<container::XNameReplace> SAL_CALL
    SwXTextGraphicObject::getEvents()
        throw(uno::RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}

/******************************************************************
 *
 ******************************************************************/
/*-- 11.12.98 16:16:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextEmbeddedObject::SwXTextEmbeddedObject( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_OLE, aSwMapProvider.GetPropertySet(PROPERTY_MAP_EMBEDDED_OBJECT), pDoc)
{
}
/*-- 11.12.98 16:16:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextEmbeddedObject::SwXTextEmbeddedObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_OLE, aSwMapProvider.GetPropertySet(PROPERTY_MAP_EMBEDDED_OBJECT))
{

}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextEmbeddedObject::~SwXTextEmbeddedObject()
{

}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextEmbeddedObject::acquire()throw()
{
    SwXFrame::acquire();
}
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextEmbeddedObject::release()throw()
{
    SwXFrame::release();
}
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
::uno::Any SAL_CALL SwXTextEmbeddedObject::queryInterface( const uno::Type& aType )
    throw( uno::RuntimeException)
{
    ::uno::Any aRet = SwXFrame::queryInterface(aType);;
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextEmbeddedObjectBaseClass::queryInterface(aType);
    return aRet;
}
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXTextEmbeddedObject::getImplementationId(  ) throw(uno::RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static uno::Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::attach(const uno::Reference< text::XTextRange > & xTextRange) throw( lang::IllegalArgumentException, uno::RuntimeException )
{
    SwXFrame::attach(xTextRange);
}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXTextEmbeddedObject::getAnchor(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXFrame::getAnchor();
}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::dispose(void) throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXFrame::dispose();
}
/*-- 11.12.98 16:16:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::addEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}
/*-- 11.12.98 16:16:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::removeEventListener(const uno::Reference< lang::XEventListener > & aListener) throw( uno::RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}
/*-- 11.12.98 16:16:55---------------------------------------------------

  -----------------------------------------------------------------------*/

uno::Reference< lang::XComponent >  SwXTextEmbeddedObject::getEmbeddedObject(void) throw( uno::RuntimeException )
{
    uno::Reference< lang::XComponent >  xRet;
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        DBG_ASSERT( pCnt->GetCntntIdx() &&
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
                SwClientIter aIter( *pFmt );
                SwXOLEListener* pListener = (SwXOLEListener*)aIter.
                                        First( TYPE( SwXOLEListener ));
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

/* --18.05.2006 16:39---------------------------------------------------

  -----------------------------------------------------------------------*/


uno::Reference< embed::XEmbeddedObject > SAL_CALL SwXTextEmbeddedObject::getExtendedControlOverEmbeddedObject()
        throw( uno::RuntimeException )
{
    uno::Reference< embed::XEmbeddedObject > xResult;
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        DBG_ASSERT( pCnt->GetCntntIdx() &&
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
                SwClientIter aIter( *pFmt );
                SwXOLEListener* pListener = (SwXOLEListener*)aIter.
                                        First( TYPE( SwXOLEListener ));
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
        DBG_ASSERT( pCnt->GetCntntIdx() &&
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
        DBG_ASSERT( pCnt->GetCntntIdx() &&
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
        DBG_ASSERT( pCnt->GetCntntIdx() &&
                       pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        Graphic* pGraphic = pDoc->GetNodes()[ pCnt->GetCntntIdx()->GetIndex() + 1 ]->GetOLENode()->GetGraphic();
        if ( pGraphic )
            return pGraphic->GetXGraphic();
    }

    return uno::Reference< graphic::XGraphic >();
}

/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextEmbeddedObject::getImplementationName(void) throw( uno::RuntimeException )

{
    return C2U("SwXTextEmbeddedObject");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextEmbeddedObject::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return  COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextEmbeddedObject")||
                    SwXFrame::supportsService(rServiceName);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextEmbeddedObject::getSupportedServiceNames(void)
        throw( uno::RuntimeException )
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 1);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 1] = C2U("com.sun.star.text.TextEmbeddedObject");
    return aRet;
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void * SAL_CALL SwXTextEmbeddedObject::operator new( size_t t) throw()
{
    return SwXTextEmbeddedObjectBaseClass::operator new(t);
}
/* -----------------------------20.06.00 10:02--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SwXTextEmbeddedObject::operator delete( void * p) throw()
{
    SwXTextEmbeddedObjectBaseClass::operator delete(p);
}
/* -----------------------------15.12.00 12:45--------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference<container::XNameReplace> SAL_CALL
    SwXTextEmbeddedObject::getEvents()
        throw(uno::RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}


/******************************************************************
 *
 ******************************************************************/
TYPEINIT1(SwXOLEListener, SwClient);
/* -----------------------------18.01.2002 09:59------------------------------

 ---------------------------------------------------------------------------*/
SwXOLEListener::SwXOLEListener( SwFmt& rOLEFmt, uno::Reference< XModel > xOLE) :
    SwClient(&rOLEFmt),
    xOLEModel(xOLE)
{
}
/* -----------------------------18.01.2002 09:59------------------------------

 ---------------------------------------------------------------------------*/
SwXOLEListener::~SwXOLEListener()
{}

void SwXOLEListener::modified( const lang::EventObject& /*rEvent*/ )
                                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

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
    pNd->SetOLESizeInvalid( sal_True );
    pNd->GetDoc()->SetOLEObjModified();
}

/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwXOLEListener::disposing( const lang::EventObject& rEvent )
                        throw( uno::RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());

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
        DBG_ERROR("OLE Listener couldn't be removed");
    }
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwXOLEListener::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
        xOLEModel = 0;
}

