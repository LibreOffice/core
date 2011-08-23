/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#include <swtypes.hxx>
#include <cmdid.h>

#include <hints.hxx>

#include <horiornt.hxx>

#include <doc.hxx>
#include <docsh.hxx>

#include <errhdl.hxx>

#include <ndindex.hxx>
#include <pam.hxx>
#include <ndnotxt.hxx>
#include <bf_svx/unomid.hxx>
#include <unocrsr.hxx>
#include <docstyle.hxx>
#include <dcontact.hxx>
#include <fmtcnct.hxx>
#include <ndole.hxx>
#include <unoobj.hxx>
#include <unomap.hxx>
#include <unoprnms.hxx>
#include <unoevent.hxx>
#include <com/sun/star/table/BorderLine.hpp>
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#include <com/sun/star/table/ShadowFormat.hpp>
#include <com/sun/star/style/GraphicLocation.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/text/GraphicCrop.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XTextColumns.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#include <com/sun/star/drawing/PointSequence.hpp>
#include <com/sun/star/drawing/ColorMode.hpp>
#include <tools/poly.hxx>
#include <unomid.h>
#include <unostyle.hxx>
#include <bf_svx/svdmodel.hxx>
#include <bf_svx/svdpage.hxx>
#include <bf_svx/brshitem.hxx>
#include <bf_svx/protitem.hxx>
#include <fmtornt.hxx>
#include <fmturl.hxx>
#include <bf_svx/lrspitem.hxx>
#include <bf_svx/ulspitem.hxx>
#include <bf_svx/boxitem.hxx>
#include <bf_svx/opaqitem.hxx>
#include <bf_svx/prntitem.hxx>
#include <bf_svx/shaditem.hxx>
#include <fmtsrnd.hxx>
#include <fmtfsize.hxx>
#include <grfatr.hxx>
#include <unoframe.hxx>
#include <fmtanchr.hxx>
#include <fmtclds.hxx>
#include <frmatr.hxx>
#include <ndtxt.hxx>
#include <ndgrf.hxx>
#include <osl/mutex.hxx>
#include <vcl/svapp.hxx>
#include <bf_sfx2/printer.hxx>
#include <bf_svx/svdobj.hxx>
#include <rtl/uuid.h>
#include <SwStyleNameMapper.hxx>
#include <bf_xmloff/xmlcnitm.hxx>
#include <poolfmt.hxx>
#include <pagedesc.hxx>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#include <tools/urlobj.hxx>
#include <bf_svx/frmdiritem.hxx>

#include <bf_so3/outplace.hxx>
#include <bf_sfx2/sfxuno.hxx>
namespace binfilter {

// from fefly1.cxx
extern sal_Bool lcl_ChkAndSetNewAnchor( const SwFlyFrm& rFly, SfxItemSet& rSet );

// from fefly1.cxx
extern sal_Bool lcl_ChkAndSetNewAnchor( const SwFlyFrm& rFly, SfxItemSet& rSet );

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::drawing;

using rtl::OUString;

using ::com::sun::star::frame::XModel;
using ::com::sun::star::container::XNameAccess;
using ::com::sun::star::style::XStyleFamiliesSupplier;

const sal_Char __FAR_DATA sPackageProtocol[] = "vnd.sun.star.Package:";
const sal_Char __FAR_DATA sGraphicObjectProtocol[] = "vnd.sun.star.GraphicObject:";

/****************************************************************************
    Rahmenbeschreibung
****************************************************************************/
DECLARE_TABLE(SwFrameAnyTable_Impl, uno::Any*)
class BaseFrameProperties_Impl
{
    const SfxItemPropertyMap*       _pMap;
    SwFrameAnyTable_Impl            aAnyTbl;

public:

    BaseFrameProperties_Impl ( const SfxItemPropertyMap* pMap ) :
        _pMap(pMap) {}
    ~BaseFrameProperties_Impl();

    void            SetProperty(USHORT nWID, BYTE nMemberId, uno::Any aVal);
    sal_Bool        GetProperty(USHORT nWID, BYTE nMemberId, uno::Any*& pAny );
    void        	GetProperty(const OUString &rPropertyName, const Reference < XPropertySet > &rxPropertySet, uno::Any& rAny );

    const SfxItemPropertyMap*       GetMap() const {return _pMap;}
    sal_Bool                        FillBaseProperties(SfxItemSet& rToSet, const SfxItemSet &rFromSet, sal_Bool& rSizeFound);

    virtual sal_Bool                AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound) = 0;

};

BaseFrameProperties_Impl::~BaseFrameProperties_Impl()
{
    uno::Any* pVal = aAnyTbl.First();
    while(pVal)
    {
        delete pVal;
        pVal = aAnyTbl.Next();
    }
}

void BaseFrameProperties_Impl::SetProperty(USHORT nWID, BYTE nMemberId, uno::Any aVal)
{
    ULONG nKey = (nWID << 16) + nMemberId;
    Any* pCurAny = aAnyTbl.Get(nKey);
    if(!pCurAny)
    {
        pCurAny = new uno::Any;
        aAnyTbl.Insert(nKey, pCurAny);
    }
    *pCurAny = aVal;
}

sal_Bool BaseFrameProperties_Impl::GetProperty(USHORT nWID, BYTE nMemberId, uno::Any*& pAny)
{
    ULONG nKey = (nWID << 16) + nMemberId;
    Any *pCurAny = aAnyTbl.Get(nKey);
    pAny = pCurAny;
    return pCurAny != 0;
}
void BaseFrameProperties_Impl::GetProperty( const OUString &rPropertyName, const Reference < XPropertySet > &rxPropertySet, uno::Any & rAny )
{
    rAny = rxPropertySet->getPropertyValue( rPropertyName );
}

sal_Bool BaseFrameProperties_Impl::FillBaseProperties(SfxItemSet& rToSet, const SfxItemSet& rFromSet, sal_Bool& rSizeFound)
{
    sal_Bool bRet = sal_True;
    //Anker kommt auf jeden Fall in den Set
    SwFmtAnchor aAnchor ( static_cast < const SwFmtAnchor & > ( rFromSet.Get ( RES_ANCHOR ) ) );
    {
        uno::Any* pAnchorPgNo;
        if(GetProperty(RES_ANCHOR, MID_ANCHOR_PAGENUM, pAnchorPgNo))
            bRet &= ((SfxPoolItem&)aAnchor).PutValue(*pAnchorPgNo, MID_ANCHOR_PAGENUM);
        uno::Any* pAnchorType;
        if(GetProperty(RES_ANCHOR, MID_ANCHOR_ANCHORTYPE, pAnchorType))
            bRet &= ((SfxPoolItem&)aAnchor).PutValue(*pAnchorType, MID_ANCHOR_ANCHORTYPE);
    }
    rToSet.Put(aAnchor);
    {
        uno::Any* pCol = 0;
        GetProperty(RES_BACKGROUND, MID_BACK_COLOR, pCol );
        uno::Any* pRGBCol = 0;
        GetProperty(RES_BACKGROUND, MID_BACK_COLOR_R_G_B, pRGBCol );
        uno::Any* pColTrans = 0;
        GetProperty(RES_BACKGROUND, MID_BACK_COLOR_TRANSPARENCY, pColTrans);
        uno::Any* pTrans = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_TRANSPARENT, pTrans );
        uno::Any* pGrLoc = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_POSITION, pGrLoc );
        uno::Any* pGrURL = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_URL, pGrURL     );
        uno::Any* pGrFilter = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_FILTER, pGrFilter     );
        uno::Any* pGrTranparency = 0;
        GetProperty(RES_BACKGROUND, MID_GRAPHIC_TRANSPARENCY, pGrTranparency     );

        if(pCol || pTrans || pGrURL || pGrFilter || pGrLoc ||
                            pGrTranparency || pColTrans || pRGBCol)
        {
            SvxBrushItem aBrush ( static_cast < const SvxBrushItem & > ( rFromSet.Get ( RES_BACKGROUND ) ) );
            if(pCol )
                bRet &= ((SfxPoolItem&)aBrush).PutValue(*pCol,MID_BACK_COLOR	);
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
        uno::Any* pCont = 0;
        GetProperty(RES_PROTECT, MID_PROTECT_CONTENT, pCont );
        uno::Any* pPos = 0;
        GetProperty(RES_PROTECT,MID_PROTECT_POSITION, pPos );
        uno::Any* pName = 0;
        GetProperty(RES_PROTECT, MID_PROTECT_SIZE, pName );
        if(pCont||pPos||pName)
        {
            SvxProtectItem aProt ( static_cast < const SvxProtectItem & > ( rFromSet.Get ( RES_PROTECT ) ) );
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
        uno::Any* pHori  = 0;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_ORIENT, pHori );
        uno::Any* pHoriP = 0;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_POSITION|CONVERT_TWIPS, pHoriP );
        uno::Any* pHoriR = 0;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_RELATION, pHoriR );
        uno::Any* pPageT = 0;
        GetProperty(RES_HORI_ORIENT, MID_HORIORIENT_PAGETOGGLE, pPageT);
        if(pHori||pHoriP||pHoriR||pPageT)
        {
            SwFmtHoriOrient aOrient ( static_cast < const SwFmtHoriOrient & > ( rFromSet.Get ( RES_HORI_ORIENT ) ) );
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
        uno::Any* pVert  = 0;
        GetProperty(RES_VERT_ORIENT, MID_VERTORIENT_ORIENT, pVert);
        uno::Any* pVertP = 0;
        GetProperty(RES_VERT_ORIENT, MID_VERTORIENT_POSITION|CONVERT_TWIPS, pVertP );
        uno::Any* pVertR = 0;
        GetProperty(RES_VERT_ORIENT, MID_VERTORIENT_RELATION, pVertR );
        if(pVert||pVertP||pVertR)
        {
            SwFmtVertOrient aOrient ( static_cast < const SwFmtVertOrient & > ( rFromSet.Get ( RES_VERT_ORIENT ) ) );
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
        uno::Any* pURL = 0;
        GetProperty(RES_URL, MID_URL_URL, pURL );
        uno::Any* pTarget = 0;
        GetProperty(RES_URL, MID_URL_TARGET, pTarget );
        uno::Any* pHyLNm = 0;
        GetProperty(RES_URL, MID_URL_HYPERLINKNAME, pHyLNm );
        uno::Any* pHySMp = 0;
        GetProperty(RES_URL, MID_URL_SERVERMAP, pHySMp );
        if(pURL||pTarget||pHyLNm||pHySMp)
        {
            SwFmtURL aURL ( static_cast < const SwFmtURL & > ( rFromSet.Get ( RES_URL ) ) );
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
    uno::Any* pL = 0;
    GetProperty(RES_LR_SPACE, MID_L_MARGIN|CONVERT_TWIPS, pL );
    uno::Any* pR = 0;
    GetProperty(RES_LR_SPACE, MID_R_MARGIN|CONVERT_TWIPS, pR );
    if(pL||pR)
    {
        SvxLRSpaceItem aLR ( static_cast < const SvxLRSpaceItem & > ( rFromSet.Get ( RES_LR_SPACE ) ) );
        if(pL)
            bRet &= ((SfxPoolItem&)aLR).PutValue(*pL, MID_L_MARGIN|CONVERT_TWIPS);
        if(pR)
            bRet &= ((SfxPoolItem&)aLR).PutValue(*pR, MID_R_MARGIN|CONVERT_TWIPS);
        rToSet.Put(aLR);
    }
    uno::Any* pT = 0;
    GetProperty(RES_UL_SPACE, MID_UP_MARGIN|CONVERT_TWIPS, pT );
    uno::Any* pB = 0;
    GetProperty(RES_UL_SPACE, MID_LO_MARGIN|CONVERT_TWIPS, pB );
    if(pT||pB)
    {
        SvxULSpaceItem aTB ( static_cast < const SvxULSpaceItem &> ( rFromSet.Get ( RES_UL_SPACE ) ) );
        if(pT)
            bRet &= ((SfxPoolItem&)aTB).PutValue(*pT, MID_UP_MARGIN|CONVERT_TWIPS);
        if(pB)
            bRet &= ((SfxPoolItem&)aTB).PutValue(*pB, MID_LO_MARGIN|CONVERT_TWIPS);
        rToSet.Put(aTB);
    }
    uno::Any* pOp;
    if(GetProperty(RES_OPAQUE, 0, pOp))
    {
        SvxOpaqueItem aOp ( static_cast < const SvxOpaqueItem& > ( rFromSet.Get ( RES_OPAQUE ) ) );
        bRet &= ((SfxPoolItem&)aOp).PutValue(*pOp, 0);
        rToSet.Put(aOp);
    }
    uno::Any* pPrt;
    if(GetProperty(RES_PRINT, 0, pPrt))
    {
        SvxPrintItem aPrt ( static_cast < const SvxPrintItem & > ( rFromSet.Get ( RES_PRINT ) ) );
        bRet &= ((SfxPoolItem&)aPrt).PutValue(*pPrt, 0);
        rToSet.Put(aPrt);
    }
    uno::Any* pSh;
    if(GetProperty(RES_SHADOW, CONVERT_TWIPS, pSh))
    {
        SvxShadowItem aSh ( static_cast < const SvxShadowItem& > ( rFromSet.Get ( RES_SHADOW ) ) );
        bRet &= ((SfxPoolItem&)aSh).PutValue(*pSh, CONVERT_TWIPS);
        rToSet.Put(aSh);
    }
    uno::Any* pSur 	 = 0;
    GetProperty(RES_SURROUND, MID_SURROUND_SURROUNDTYPE, pSur);
    uno::Any* pSurAnch = 0;
    GetProperty(RES_SURROUND, MID_SURROUND_ANCHORONLY, pSurAnch);
    if(pSur || pSurAnch)
    {
        SwFmtSurround aSrnd ( static_cast < const SwFmtSurround & > ( rFromSet.Get ( RES_SURROUND ) ) );
        if(pSur)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSur, MID_SURROUND_SURROUNDTYPE	);
        if(pSurAnch)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSurAnch, MID_SURROUND_ANCHORONLY);
        rToSet.Put(aSrnd);
    }
    uno::Any* pLeft 		= 0;
    GetProperty(RES_BOX, LEFT_BORDER  |CONVERT_TWIPS,    pLeft  );
    uno::Any* pRight 		= 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|RIGHT_BORDER ,    pRight );
    uno::Any* pTop 		= 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|TOP_BORDER     , pTop   );
    uno::Any* pBottom 	= 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|BOTTOM_BORDER,    pBottom);
    uno::Any* pDistance 	= 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|BORDER_DISTANCE,  pDistance);
    uno::Any* pLeftDistance 	= 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|LEFT_BORDER_DISTANCE, pLeftDistance);
    uno::Any* pRightDistance 	= 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|RIGHT_BORDER_DISTANCE,    pRightDistance);
    uno::Any* pTopDistance 	= 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|TOP_BORDER_DISTANCE,  pTopDistance);
    uno::Any* pBottomDistance 	= 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|BOTTOM_BORDER_DISTANCE,   pBottomDistance);
    if(	pLeft || pRight || pTop ||	pBottom || pDistance ||
        pLeftDistance  || pRightDistance || pTopDistance || pBottomDistance )
    {
        SvxBoxItem aBox ( static_cast < const SvxBoxItem & > ( rFromSet.Get ( RES_BOX ) ) );
        if(	pLeft )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pLeft, CONVERT_TWIPS|LEFT_BORDER );
        if(	pRight )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pRight, CONVERT_TWIPS|RIGHT_BORDER );
        if(	pTop )
            bRet &= ((SfxPoolItem&)aBox).PutValue(*pTop, CONVERT_TWIPS|TOP_BORDER);
        if(	pBottom )
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
        uno::Any* pRelH = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_REL_HEIGHT, pRelH);
        uno::Any* pRelW = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_REL_WIDTH, pRelW);
        uno::Any* pSyncWidth = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_IS_SYNC_WIDTH_TO_HEIGHT, pSyncWidth);
        uno::Any* pSyncHeight = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_IS_SYNC_HEIGHT_TO_WIDTH, pSyncHeight);
        uno::Any* pWidth = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_WIDTH|CONVERT_TWIPS, pWidth);
        uno::Any* pHeight = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_HEIGHT|CONVERT_TWIPS, pHeight);
        uno::Any* pSize = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_SIZE|CONVERT_TWIPS, pSize);
        uno::Any* pSizeType = 0;
        GetProperty(RES_FRM_SIZE, MID_FRMSIZE_SIZE_TYPE, pSizeType);
        if( pWidth || pHeight ||pRelH || pRelW || pSize ||pSizeType ||
            pSyncWidth || pSyncHeight )
        {
            rSizeFound = sal_True;
            SwFmtFrmSize aFrmSz ( static_cast < const SwFmtFrmSize& > ( rFromSet.Get ( RES_FRM_SIZE ) ) );
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
            uno::Any aSizeVal;
            aSizeVal <<= aSize;
            ((SfxPoolItem&)aFrmSz).PutValue(aSizeVal, MID_FRMSIZE_SIZE|CONVERT_TWIPS);
            rToSet.Put(aFrmSz);
        }
    }
    uno::Any* pFrameDirection = 0;
    GetProperty(RES_FRAMEDIR, 0, pFrameDirection);
    if(pFrameDirection)
    {
        SvxFrameDirectionItem aAttr(FRMDIR_HORI_LEFT_TOP, RES_FRAMEDIR);
        aAttr.PutValue(*pFrameDirection, 0);
        rToSet.Put(aAttr);
    }
    uno::Any* pUnknown = 0;
    GetProperty(RES_UNKNOWNATR_CONTAINER, 0, pUnknown);
    if(pUnknown)
    {
        SvXMLAttrContainerItem aAttr(RES_UNKNOWNATR_CONTAINER);
        aAttr.PutValue(*pUnknown, 0);
        rToSet.Put(aAttr);
    }

    return bRet;
}

class SwFrameProperties_Impl : public BaseFrameProperties_Impl
{
protected:
    SwFrameProperties_Impl(const SfxItemPropertyMap* pMap) :
        BaseFrameProperties_Impl(pMap){}
public:
    SwFrameProperties_Impl();
    ~SwFrameProperties_Impl(){}

    virtual sal_Bool        AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound);
};

SwFrameProperties_Impl::SwFrameProperties_Impl():
    BaseFrameProperties_Impl(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME) )
{
}

inline void lcl_FillCol ( SfxItemSet &rToSet, const SfxItemSet &rFromSet, Any *pAny)
{
    if ( pAny )
    {
        SwFmtCol aCol ( static_cast < const SwFmtCol & > ( rFromSet.Get ( RES_COL ) ) );
        ((SfxPoolItem&)aCol).PutValue( *pAny, MID_COLUMNS);
        rToSet.Put(aCol);
    }
}
sal_Bool    SwFrameProperties_Impl::AnyToItemSet(SwDoc *pDoc, SfxItemSet& rSet, SfxItemSet&, sal_Bool& rSizeFound)
{
    //Properties fuer alle Frames
    uno::Any *pStyleName;
    SwDocStyleSheet* pStyle = NULL;
    sal_Bool bRet;

    if ( GetProperty ( FN_UNO_FRAME_STYLE_NAME, 0, pStyleName ) )
    {
        OUString sStyle;
        *pStyleName >>= sStyle;
        pStyle = (SwDocStyleSheet*)pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle,
                                                    SFX_STYLE_FAMILY_FRAME);
    }

    uno::Any* pColumns = NULL;
    GetProperty (RES_COL, MID_COLUMNS, pColumns);
    if ( pStyle )
    {
        SwDocStyleSheet aStyle (*pStyle);
        const SfxItemSet *pItemSet = &aStyle.GetItemSet();
           bRet = FillBaseProperties( rSet, *pItemSet, rSizeFound );
        lcl_FillCol ( rSet, *pItemSet, pColumns );
    }
    else
    {
        const SfxItemSet *pItemSet = &pDoc->GetFrmFmtFromPool( RES_POOLFRM_FRAME )->GetAttrSet();
           bRet = FillBaseProperties( rSet, *pItemSet, rSizeFound );
        lcl_FillCol ( rSet, *pItemSet, pColumns );
    }
    uno::Any* pEdit;
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
    ~SwGraphicProperties_Impl(){}

    virtual sal_Bool                AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound);
};

SwGraphicProperties_Impl::SwGraphicProperties_Impl( ) :
    BaseFrameProperties_Impl(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC) )
{
}

inline void lcl_FillMirror ( SfxItemSet &rToSet, const SfxItemSet &rFromSet, Any *pHEvenMirror, Any *pHOddMirror, Any *pVMirror, sal_Bool &rRet )
{
    if(pHEvenMirror || pHOddMirror || pVMirror )
    {
        SwMirrorGrf aMirror ( static_cast < const SwMirrorGrf& > ( rFromSet.Get ( RES_FRM_SIZE ) ) );
        if(pHEvenMirror)
            rRet &= ((SfxPoolItem&)aMirror).PutValue(*pHEvenMirror, MID_MIRROR_HORZ_EVEN_PAGES);
        if(pHOddMirror)
            rRet &= ((SfxPoolItem&)aMirror).PutValue(*pHOddMirror, MID_MIRROR_HORZ_ODD_PAGES);
        if(pVMirror)
            rRet &= ((SfxPoolItem&)aMirror).PutValue(*pVMirror, MID_MIRROR_VERT);
        rToSet.Put(aMirror);
    }
}

sal_Bool 	SwGraphicProperties_Impl::AnyToItemSet(
            SwDoc* pDoc,
            SfxItemSet& rFrmSet,
            SfxItemSet& rGrSet,
            sal_Bool& rSizeFound)
{
    //Properties fuer alle Frames
    sal_Bool bRet;
    uno::Any *pStyleName;
    SwDocStyleSheet* pStyle = NULL;

    if ( GetProperty ( FN_UNO_FRAME_STYLE_NAME, 0, pStyleName ) )
    {
        OUString sStyle;
        *pStyleName >>= sStyle;
        pStyle = (SwDocStyleSheet*)pDoc->GetDocShell()->GetStyleSheetPool()->Find(sStyle,
                                                    SFX_STYLE_FAMILY_FRAME);
    }

    uno::Any* pHEvenMirror = 0;
    uno::Any* pHOddMirror = 0;
    uno::Any* pVMirror = 0;
    GetProperty(RES_GRFATR_MIRRORGRF, MID_MIRROR_HORZ_EVEN_PAGES, pHEvenMirror);
    GetProperty(RES_GRFATR_MIRRORGRF, MID_MIRROR_HORZ_ODD_PAGES, pHOddMirror);
    GetProperty(RES_GRFATR_MIRRORGRF, MID_MIRROR_VERT, pVMirror);

    if ( pStyle )
    {
        SwDocStyleSheet aStyle (*pStyle);
        const SfxItemSet *pItemSet = &aStyle.GetItemSet();
        bRet = FillBaseProperties(rFrmSet, *pItemSet, rSizeFound);
        lcl_FillMirror ( rGrSet, *pItemSet, pHEvenMirror, pHOddMirror, pVMirror, bRet );
    }
    else
    {
        const SfxItemSet *pItemSet = &pDoc->GetFrmFmtFromPool( RES_POOLFRM_GRAPHIC )->GetAttrSet();
        bRet = FillBaseProperties(rFrmSet, *pItemSet, rSizeFound);
        lcl_FillMirror ( rGrSet, *pItemSet, pHEvenMirror, pHOddMirror, pVMirror, bRet );
    }


    static const USHORT nIDs[] =
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
    uno::Any* pAny;
    for(sal_Int16 nIndex = 0; nIDs[nIndex]; nIndex++)
    {
        BYTE nMId = RES_GRFATR_CROPGRF == nIDs[nIndex] ? CONVERT_TWIPS : 0;
        if(GetProperty(nIDs[nIndex], nMId, pAny ))
        {
            SfxPoolItem* pItem = ::binfilter::GetDfltAttr( nIDs[nIndex] )->Clone();
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
        SwFrameProperties_Impl(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_EMBEDDED_OBJECT) ){}
    ~SwOLEProperties_Impl(){}

    virtual sal_Bool        AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound);
};

sal_Bool  SwOLEProperties_Impl::AnyToItemSet(
        SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound)
{
    uno::Any* pCLSID;
    if(!GetProperty(FN_UNO_CLSID, 0, pCLSID))
        return FALSE;
    SwFrameProperties_Impl::AnyToItemSet( pDoc, rFrmSet, rSet, rSizeFound);
    //
    return TRUE;
}

/******************************************************************
 *	SwXFrame
 ******************************************************************/
const uno::Sequence< sal_Int8 > & SwXFrame::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::binfilter::CreateUnoTunnelId();
    return aSeq;
}

sal_Int64 SAL_CALL SwXFrame::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

TYPEINIT1(SwXFrame, SwClient);

OUString SwXFrame::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFrame");
}

BOOL SwXFrame::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.BaseFrame")||
                !rServiceName.compareToAscii("com.sun.star.text.TextContent") ||
                    !rServiceName.compareToAscii("com.sun.star.document.LinkTarget");
}

Sequence< OUString > SwXFrame::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.BaseFrame");
    pArray[1] = C2U("com.sun.star.text.TextContent");
    pArray[2] = C2U("com.sun.star.document.LinkTarget");
    return aRet;
}
/*-- 14.01.99 11:31:52---------------------------------------------------
    Dieser CTor legt den Frame als Descriptor an
  -----------------------------------------------------------------------*/
SwXFrame::SwXFrame(FlyCntType eSet,	const SfxItemPropertyMap* pMap, SwDoc *pDoc ) :
    aLstnrCntnr( (container::XNamed*)this),
    eType(eSet),
    aPropSet(pMap),
    _pMap(pMap),
    bIsDescriptor(sal_True),
    mpDoc ( pDoc )
{
    // Register ourselves as a listener to the document (via the page descriptor)
    pDoc->GetPageDescFromPool(RES_POOLPAGE_STANDARD)->Add(this);
    // get the property set for the default style data
    // First get the model
    Reference < XModel > xModel = pDoc->GetDocShell()->GetBaseModel();
    // Ask the model for it's family supplier interface
    Reference < XStyleFamiliesSupplier > xFamilySupplier ( xModel, UNO_QUERY );
    // Get the style families
    Reference < XNameAccess > xFamilies = xFamilySupplier->getStyleFamilies();
    // Get the Frame family (and keep it for later)
    Any aAny = xFamilies->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "FrameStyles" ) ) );
    aAny >>= mxStyleFamily;
    // In the derived class, we'll ask mxStyleFamily for the relevant default style
    // mxStyleFamily is initialised in the SwXFrame constructor
    switch(eType)
    {
        case FLYCNTTYPE_FRM:
        {
            Any aAny = mxStyleFamily->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Frame" ) ) );
            aAny >>= mxStyleData;
            pProps = new SwFrameProperties_Impl( );
        }
        break;
        case FLYCNTTYPE_GRF:
        {
            Any aAny = mxStyleFamily->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Graphics" ) ) );
            aAny >>= mxStyleData;
            pProps = new SwGraphicProperties_Impl( );
        }
        break;
        case FLYCNTTYPE_OLE:
        {
            Any aAny = mxStyleFamily->getByName ( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "OLE" ) ) );
            aAny >>= mxStyleData;
            pProps = 0;
            pProps = new SwOLEProperties_Impl( );
        }
        break;
    }
}

SwXFrame::SwXFrame(SwFrmFmt& rFrmFmt, FlyCntType eSet, const SfxItemPropertyMap* pMap) :
    eType(eSet),
    SwClient( &rFrmFmt ),
    aLstnrCntnr( (container::XNamed*)this),
    aPropSet(pMap),
    _pMap(pMap),
    bIsDescriptor(sal_False),
    pProps(0)
{
}

SwXFrame::~SwXFrame()
{
    delete pProps;
}

OUString SwXFrame::getName(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    String sRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
        sRet = pFmt->GetName();
    else if(bIsDescriptor)
        sRet = sName;
    else
        throw RuntimeException();
    return sRet;
}

void SwXFrame::setName(const OUString& rName) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    String sTmpName(rName);
    if(pFmt)
    {
        pFmt->GetDoc()->SetFlyName((SwFlyFrmFmt&)*pFmt, sTmpName);
        if(pFmt->GetName() != sTmpName)
        {
            throw RuntimeException();
        }
    }
    else if(bIsDescriptor)
        sName = sTmpName;
    else
        throw RuntimeException();
}

uno::Reference< XPropertySetInfo >  SwXFrame::getPropertySetInfo(void) throw( RuntimeException )
{
    uno::Reference< XPropertySetInfo >  xRef;
    static uno::Reference< XPropertySetInfo >  xFrmRef;
    static uno::Reference< XPropertySetInfo >  xGrfRef;
    static uno::Reference< XPropertySetInfo >  xOLERef;
    switch(eType)
    {
    case FLYCNTTYPE_FRM:
        if( !xFrmRef.is() )
            xFrmRef = aPropSet.getPropertySetInfo();
        xRef = xFrmRef;
        break;
    case FLYCNTTYPE_GRF:
        if( !xGrfRef.is() )
            xGrfRef = aPropSet.getPropertySetInfo();
        xRef = xGrfRef;
        break;
    case FLYCNTTYPE_OLE:
        if( !xOLERef.is() )
            xOLERef = aPropSet.getPropertySetInfo();
        xRef = xOLERef;
        break;
    }
    return xRef;
}

SdrObject *SwXFrame::GetOrCreateSdrObject( SwFlyFrmFmt *pFmt )
{
    SdrObject* pObject = pFmt->FindSdrObject();
    if( !pObject )
    {
        SwDoc *pDoc = pFmt->GetDoc();
        SdrModel *pDrawModel = pDoc->MakeDrawModel();
        SwFlyDrawContact* pContactObject
                    = new SwFlyDrawContact( pFmt, pDrawModel );
        pObject = pContactObject->GetMaster();

        const SwFmtSurround& rSurround = pFmt->GetSurround();
        pObject->SetLayer(
            ( SURROUND_THROUGHT == rSurround.GetSurround() &&
              !pFmt->GetOpaque().GetValue() ) ? pDoc->GetHellId()
                                             : pDoc->GetHeavenId() );

        pDrawModel->GetPage(0)->InsertObject( pObject );
    }

    return pObject;
}

SwFrmFmt *lcl_GetFrmFmt( const uno::Any& rValue, SwDoc *pDoc )
{
    SwFrmFmt *pRet = 0;
    SwDocShell* pDocSh = pDoc->GetDocShell();
    if(pDocSh)
    {
        OUString uTemp;
        rValue >>= uTemp;
        String sStyle;
        SwStyleNameMapper::FillUIName(String (uTemp), sStyle, GET_POOLID_FRMFMT, sal_True );
        SwDocStyleSheet* pStyle =
                (SwDocStyleSheet*)pDocSh->GetStyleSheetPool()->Find(sStyle,
                                                    SFX_STYLE_FAMILY_FRAME);
        if(pStyle)
            pRet = pStyle->GetFrmFmt();
    }

    return pRet;
}

void SwXFrame::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( UnknownPropertyException, PropertyVetoException, lang::IllegalArgumentException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);

    if (!pCur)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(pFmt)
    {
        sal_Bool bNextFrame = sal_False;
        if ( pCur->nFlags & PropertyAttribute::READONLY)
            throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        SwDoc* pDoc = pFmt->GetDoc();
        if(	eType == FLYCNTTYPE_GRF &&
                    (pCur->nWID >=  RES_GRFATR_BEGIN &&
                        pCur->nWID < RES_GRFATR_END)||
                            pCur->nWID == FN_PARAM_COUNTOUR_PP ||
                            pCur->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR ||
                            pCur->nWID == FN_UNO_IS_PIXEL_CONTOUR ||
                            FN_UNO_ALTERNATIVE_TEXT == pCur->nWID)
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                if(FN_UNO_ALTERNATIVE_TEXT == pCur->nWID )
                {
                    OUString uTemp;
                    aValue >>= uTemp;
                    pNoTxt->SetAlternateText(uTemp);
                }
                else if(pCur->nWID == FN_PARAM_COUNTOUR_PP)
                {
                    PointSequenceSequence aParam;
                    if(!aValue.hasValue())
                        pNoTxt->SetContour(0);
                    else if(aValue >>= aParam)
                    {
                        PolyPolygon aPoly((USHORT)aParam.getLength());
                        for(sal_Int32 i = 0; i < aParam.getLength(); i++)
                        {
                            const PointSequence* pPointSeq = aParam.getConstArray();
                            sal_Int32 nPoints = pPointSeq[i].getLength();
                            const awt::Point* pPoints = pPointSeq[i].getConstArray();
                            Polygon aSet( (USHORT)nPoints );
                            for(sal_Int32 j = 0; j < nPoints; j++)
                            {
                                Point aPoint(pPoints[j].X, pPoints[j].Y);
                                aSet.SetPoint(aPoint, (USHORT)j);
                            }
                            // Close polygon if it isn't closed already.
                            aSet.Optimize( POLY_OPTIMIZE_CLOSE );
                            aPoly.Insert( aSet );
                        }
                        pNoTxt->SetContourAPI( &aPoly );
                    }
                    else
                        throw IllegalArgumentException();
                }
                else if(pCur->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR )
                {
                    pNoTxt->SetAutomaticContour( *(sal_Bool *)aValue.getValue() );
                }
                else if(pCur->nWID == FN_UNO_IS_PIXEL_CONTOUR )
                {
                    // The IsPixelContour property can only be set if there
                    // is no contour, or if the contour has been set by the
                    // API itself (or in other words, if the contour isn't
                    // used already).
                    if( !pNoTxt->_HasContour() ||
                        !pNoTxt->IsContourMapModeValid() )
                        pNoTxt->SetPixelContour( *(sal_Bool *)aValue.getValue() );
                    else
                        throw IllegalArgumentException();
                }
                else
                {
                    SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                    aPropSet.setPropertyValue(*pCur, aValue, aSet);
                    pNoTxt->SetAttr(aSet);
                }
            }
        }
        else if(FN_UNO_FRAME_STYLE_NAME == pCur->nWID)
        {
            SwFrmFmt *pFrmFmt = lcl_GetFrmFmt( aValue, pFmt->GetDoc() );
            if( pFrmFmt )
            {
                UnoActionContext aAction(pFmt->GetDoc());

                // see SwFEShell::SetFrmFmt( SwFrmFmt *pNewFmt, sal_Bool bKeepOrient, Point* pDocPos )
                SwFlyFrm *pFly = 0;
                const SwFrmFmt* pFmtXX = pFmt;
                if (PTR_CAST(SwFlyFrmFmt, pFmtXX))
                    pFly = ((SwFlyFrmFmt*)pFmtXX)->GetFrm();
                SfxItemSet* pSet = 0;
                if (pFly)
                {
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == pFrmFmt->GetItemState( RES_ANCHOR, sal_False, &pItem ))
                    {
                        pSet = new SfxItemSet( pDoc->GetAttrPool(), aFrmFmtSetRange );
                        pSet->Put( *pItem );
                        if( !::binfilter::lcl_ChkAndSetNewAnchor( *pFly, *pSet ))
                            delete pSet, pSet = 0;
                    }
                }

                pFmt->GetDoc()->SetFrmFmtToFly( *pFmt, *pFrmFmt, pSet, FALSE );
                delete pSet;
            }
            else
                throw IllegalArgumentException();
        }
        else if( FN_UNO_GRAPHIC_U_R_L == pCur->nWID ||
                FN_UNO_GRAPHIC_FILTER == pCur->nWID)
        {
            String sGrfName, sFltName;
            BfGraphicObject* pGrfObj = 0;
            pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, &sGrfName, &sFltName );
            OUString uTemp;
            aValue >>= uTemp;
            String sTmp(uTemp);
            UnoActionContext aAction(pFmt->GetDoc());
            if(FN_UNO_GRAPHIC_U_R_L == pCur->nWID)
            {
                if( sTmp.EqualsAscii( sPackageProtocol,
                                      0, sizeof( sPackageProtocol )-1 ) )
                {
                    pGrfObj = new BfGraphicObject;
                    pGrfObj->SetUserData( sTmp );
                    pGrfObj->SetSwapState();
                    sGrfName.Erase();
                }
                else if( sTmp.EqualsAscii( sGraphicObjectProtocol,
                                           0, sizeof(sGraphicObjectProtocol)-1 ) )
                {
                    ByteString sId( sTmp.Copy(sizeof(sGraphicObjectProtocol)-1),
                                    RTL_TEXTENCODING_ASCII_US );
                    pGrfObj = new BfGraphicObject( sId );
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

            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                {
                    delete pGrfObj;
                    throw RuntimeException();
                }
                SwPaM aGrfPaM(*pGrfNode);
                pFmt->GetDoc()->ReRead( aGrfPaM, sGrfName, sFltName, 0,
                                        pGrfObj );
            }
            delete pGrfObj;
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
                    SwFrmFmt* pFmt = pDoc->GetFlyNum(i, FLYCNTTYPE_FRM);
                    if(sChainName == pFmt->GetName() )
                    {
                        pChain = pFmt;
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
        else if(FN_UNO_Z_ORDER == pCur->nWID)
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
        else if(RES_ANCHOR == pCur->nWID && MID_ANCHOR_ANCHORFRAME == pCur->nMemberId)
        {
            sal_Bool bDone = sal_True;
            Reference<XTextFrame> xFrame;
            if(aValue >>= xFrame)
            {
                Reference<XUnoTunnel> xTunnel(xFrame, UNO_QUERY);
                SwXFrame* pFrame = xTunnel.is() ?
                        (SwXFrame*)xTunnel->getSomething(SwXFrame::getUnoTunnelId()) : 0;
                if(pFrame && this != pFrame && pFrame->GetFrmFmt() && pFrame->GetFrmFmt()->GetDoc() == pDoc)
                {
                    SfxItemSet aSet( pDoc->GetAttrPool(),
                                RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                    aSet.SetParent(&pFmt->GetAttrSet());
                    SwFmtAnchor aAnchor = (const SwFmtAnchor&)aSet.Get(pCur->nWID);


                    SwPosition aPos(*pFrame->GetFrmFmt()->GetCntnt().GetCntntIdx());
                    aAnchor.SetAnchor(&aPos);
                    aAnchor.SetType(FLY_AT_FLY);
                    aSet.Put(aAnchor);
                    pDoc->SetFlyFrmAttr( *pFmt, aSet );
                    bDone = sal_True;
                }
            }
            if(!bDone)
                throw IllegalArgumentException();
        }
        else
        {
            SfxItemSet aSet( pDoc->GetAttrPool(),
                RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );

            aSet.SetParent(&pFmt->GetAttrSet());
            aPropSet.setPropertyValue(*pCur, aValue, aSet);
            if(RES_ANCHOR == pCur->nWID && MID_ANCHOR_ANCHORTYPE == pCur->nMemberId)
            {
                SwFmtAnchor aAnchor = (const SwFmtAnchor&)aSet.Get(pCur->nWID);
                if(aAnchor.GetAnchorId() == FLY_AT_FLY)
                {
                    const SwPosition* pPosition = aAnchor.GetCntntAnchor();
                    SwFrmFmt* pFmt = pPosition ? pPosition->nNode.GetNode().GetFlyFmt() : 0;
                    if(!pFmt || pFmt->Which() == RES_DRAWFRMFMT)
                    {
                        lang::IllegalArgumentException aExcept;
                        aExcept.Message = C2U("Anchor to frame: no frame found");
                        throw aExcept;
                    }
                    else
                    {
                        SwPosition aPos = *pPosition;
                        aPos.nNode = *pFmt->GetCntnt().GetCntntIdx();
                        aAnchor.SetAnchor(&aPos);
                        aSet.Put(aAnchor);
                    }
                }
                else if(aAnchor.GetAnchorId() != FLY_PAGE && !aAnchor.GetCntntAnchor())
                {
                    SwNode& rNode = pDoc->GetNodes().GetEndOfContent();
                    SwPaM aPam(rNode);
                    aPam.Move( fnMoveBackward, fnGoDoc );
                    aAnchor.SetAnchor( aPam.Start() );
                    aSet.Put(aAnchor);
                }

                // see SwFEShell::SetFlyFrmAttr( SfxItemSet& rSet )
                SwFlyFrm *pFly = 0;
                if (PTR_CAST(SwFlyFrmFmt, pFmt))
                    pFly = ((SwFlyFrmFmt*)pFmt)->GetFrm();
                if (pFly)
                {
                    const SfxPoolItem* pItem;
                    if( SFX_ITEM_SET == aSet.GetItemState( RES_ANCHOR, sal_False, &pItem ))
                    {
                        aSet.Put( *pItem );
                        ::binfilter::lcl_ChkAndSetNewAnchor( *pFly, aSet );
                    }
                }

                pFmt->GetDoc()->SetFlyFrmAttr( *pFmt, aSet );
            }
            else if(FN_UNO_CLSID == pCur->nWID)
            {
                throw lang::IllegalArgumentException();
            }
            else
                pFmt->SetAttr(aSet);
        }
    }
    else if(IsDescriptor())
    {
        pProps->SetProperty(pCur->nWID, pCur->nMemberId, aValue);
        if( FN_UNO_FRAME_STYLE_NAME == pCur->nWID )
        {
            OUString sStyleName;
            aValue >>= sStyleName;
            try
            {
                Any aAny = mxStyleFamily->getByName ( sStyleName );
                aAny >>= mxStyleData;
            }
            catch ( container::NoSuchElementException& )
            {
            }
            catch ( WrappedTargetException& )
            {
            }
            catch ( RuntimeException& )
            {
            }
        }
    }
    else
        throw RuntimeException();
}

uno::Any SwXFrame::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aAny;
    SwFrmFmt* pFmt = GetFrmFmt();
    const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
    if (!pCur)
        throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

    if(FN_UNO_ANCHOR_TYPES == pCur->nWID)
    {
        uno::Sequence<TextContentAnchorType> aTypes(5);
         TextContentAnchorType* pArray = aTypes.getArray();
        pArray[0] = TextContentAnchorType_AT_PARAGRAPH;
        pArray[1] = TextContentAnchorType_AS_CHARACTER;
        pArray[2] = TextContentAnchorType_AT_PAGE;
        pArray[3] = TextContentAnchorType_AT_FRAME;
        pArray[4] = TextContentAnchorType_AT_CHARACTER;
        aAny.setValue(&aTypes, ::getCppuType((uno::Sequence<TextContentAnchorType>*)0));
    }
    else if(pFmt)
    {
        if( ((eType == FLYCNTTYPE_GRF) || (eType == FLYCNTTYPE_OLE)) &&
                pCur &&
                ((pCur->nWID >=  RES_GRFATR_BEGIN &&
                    pCur->nWID < RES_GRFATR_END )||
                        pCur->nWID == FN_PARAM_COUNTOUR_PP ||
                        pCur->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR ||
                        pCur->nWID == FN_UNO_IS_PIXEL_CONTOUR ))
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                if(pCur->nWID == FN_PARAM_COUNTOUR_PP)
                {
                    PolyPolygon aContour;
                    if( pNoTxt->GetContourAPI( aContour ) )
                    {
                        PointSequenceSequence aPtSeq(aContour.Count());
                        PointSequence* pPSeq = aPtSeq.getArray();
                        for(USHORT i = 0; i < aContour.Count(); i++)
                        {
                            const Polygon& rPoly = aContour.GetObject(i);
                            pPSeq[i].realloc(rPoly.GetSize());
                            awt::Point* pPoints = pPSeq[i].getArray();
                            for(USHORT j = 0; j < rPoly.GetSize(); j++)
                            {
                                const Point& rPoint = rPoly.GetPoint(j);
                                pPoints[j].X = rPoint.X();
                                pPoints[j].Y = rPoint.Y();
                            }
                        }
                        aAny <<= aPtSeq;
                    }
                }
                else if(pCur->nWID == FN_UNO_IS_AUTOMATIC_CONTOUR )
                {
                    BOOL bValue = pNoTxt->HasAutomaticContour();
                    aAny.setValue( &bValue, ::getBooleanCppuType() );
                }
                else if(pCur->nWID == FN_UNO_IS_PIXEL_CONTOUR )
                {
                    BOOL bValue = pNoTxt->IsPixelContour();
                    aAny.setValue( &bValue, ::getBooleanCppuType() );
                }
                else
                {
                    SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                    aAny = aPropSet.getPropertyValue(*pCur, aSet);
                }
            }
        }
        else if( FN_UNO_GRAPHIC_U_R_L == pCur->nWID)
        {
            String sGrfName;
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwGrfNode* pGrfNode = aIdx.GetNode().GetGrfNode();
                if(!pGrfNode)
                    throw RuntimeException();
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
        else if( FN_UNO_GRAPHIC_FILTER == pCur->nWID )
        {
            String sFltName;
            pFmt->GetDoc()->GetGrfNms( *(SwFlyFrmFmt*)pFmt, 0, &sFltName );
                aAny <<= OUString(sFltName);
        }
        else if(FN_UNO_FRAME_STYLE_NAME == pCur->nWID)
        {
            aAny <<= OUString(SwStyleNameMapper::GetProgName(pFmt->DerivedFrom()->GetName(), GET_POOLID_FRMFMT ) );
        }
        else if(eType == FLYCNTTYPE_GRF &&
                (rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ACTUAL_SIZE)) ||
                    FN_UNO_ALTERNATIVE_TEXT == pCur->nWID))
        {
            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
                SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                if(FN_UNO_ALTERNATIVE_TEXT == pCur->nWID)
                    aAny <<= OUString(pNoTxt->GetAlternateText());
                else
                {
                     Size aActSize = ((SwGrfNode*)pNoTxt)->GetTwipSize();
                    awt::Size aTmp;
                    aTmp.Width = TWIP_TO_MM100(aActSize.Width());
                    aTmp.Height = TWIP_TO_MM100(aActSize.Height());
                    aAny.setValue(&aTmp, ::getCppuType((const awt::Size*)0));
                }
            }
        }
        else if(FN_PARAM_LINK_DISPLAY_NAME == pCur->nWID)
        {
            aAny <<= OUString(pFmt->GetName());
        }
        else if(FN_UNO_Z_ORDER == pCur->nWID)
        {
            const SdrObject* pObj = pFmt->FindRealSdrObject();
            if( pObj )
            {
                aAny <<= (sal_Int32)pObj->GetOrdNum();
            }
        }
        else if(FN_UNO_CLSID == pCur->nWID || FN_UNO_MODEL == pCur->nWID||
                FN_UNO_COMPONENT == pCur->nWID)
        {
            SwDoc* pDoc = pFmt->GetDoc();
            const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
            DBG_ASSERT( pCnt->GetCntntIdx() &&
                           pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                            GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

            SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetCntntIdx()
                                            ->GetIndex() + 1 ]->GetOLENode();
            SvInPlaceObjectRef xIP( pOleNode->GetOLEObj().GetOleRef() );
            OUString aHexCLSID;
            if(xIP.Is())
            {
                SvOutPlaceObjectRef xOut(xIP);
                SvGlobalName aClassName = xOut.Is() ? xOut->GetObjectCLSID() : xIP->GetClassName();
                aHexCLSID = aClassName.GetHexName();
                if(FN_UNO_CLSID != pCur->nWID)
                {
                    SfxInPlaceObjectRef xIPO( xIP );
                    SfxObjectShell* pShell = xIPO.Is() ? xIPO->GetObjectShell() : 0;
                    //in both cases the XModel is returned for internal components
                    //external components provide their XComponent for
                    //the "Component" property, only
                    if( pShell )
                        aAny <<= pShell->GetModel();
                    else if(xOut.Is() && FN_UNO_COMPONENT == pCur->nWID)
                         aAny <<= xOut->GetUnoComponent();
                }
            }
            if(FN_UNO_CLSID == pCur->nWID)
                aAny <<= aHexCLSID;
        }
        else
        {
            const SwAttrSet& rSet = pFmt->GetAttrSet();
            aAny = aPropSet.getPropertyValue(*pCur, rSet);
        }
    }
    else if(IsDescriptor())
    {
        if ( ! mpDoc )
            throw RuntimeException();
        uno::Any* pAny = 0;
        if( !pProps->GetProperty( pCur->nWID, pCur->nMemberId, pAny ) )
            pProps->GetProperty( rPropertyName, mxStyleData, aAny );
        else if ( pAny )
            aAny = *pAny;
    }
    else
        throw RuntimeException();
    return aAny;
}

void SwXFrame::addPropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXFrame::removePropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXFrame::addVetoableChangeListener(const OUString& PropertyName,
                                const uno::Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

void SwXFrame::removeVetoableChangeListener(
    const OUString& PropertyName, const uno::Reference< XVetoableChangeListener > & aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented");
}

PropertyState SwXFrame::getPropertyState( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Sequence< OUString > aPropertyNames(1);
    OUString* pNames = aPropertyNames.getArray();
    pNames[0] = rPropertyName;
    Sequence< PropertyState > aStates = getPropertyStates(aPropertyNames);
    return aStates.getConstArray()[0];
}

Sequence< PropertyState > SwXFrame::getPropertyStates(
    const Sequence< OUString >& aPropertyNames )
        throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Sequence< PropertyState > aStates(aPropertyNames.getLength());
    PropertyState* pStates = aStates.getArray();
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const OUString* pNames = aPropertyNames.getConstArray();
        const SwAttrSet& rFmtSet = pFmt->GetAttrSet();
        for(int i = 0; i < aPropertyNames.getLength(); i++)
        {
            const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, pNames[i]);
            if (!pCur)
                throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + pNames[i], static_cast < cppu::OWeakObject * > ( this ) );

            if(pCur->nWID == FN_UNO_ANCHOR_TYPES||
                pCur->nWID == FN_PARAM_LINK_DISPLAY_NAME||
                FN_UNO_FRAME_STYLE_NAME == pCur->nWID||
                FN_UNO_GRAPHIC_U_R_L == pCur->nWID||
                FN_UNO_GRAPHIC_FILTER     == pCur->nWID||
                FN_UNO_ACTUAL_SIZE == pCur->nWID||
                FN_UNO_ALTERNATIVE_TEXT == pCur->nWID)
                pStates[i] = PropertyState_DIRECT_VALUE;
            else
            {
                if(eType == FLYCNTTYPE_GRF &&
                        pCur &&
                        (pCur->nWID >= RES_GRFATR_BEGIN &&
                            pCur->nWID <= RES_GRFATR_END))
                {
                    const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                    if(pIdx)
                    {
                        SwNodeIndex aIdx(*pIdx, 1);
                        SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                        SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                        aSet.GetItemState(pCur->nWID);
                        if(SFX_ITEM_SET == aSet.GetItemState( pCur->nWID, FALSE ))
                            pStates[i] = beans::PropertyState_DIRECT_VALUE;
                    }
                }
                else
                {
                    if(SFX_ITEM_SET == rFmtSet.GetItemState( pCur->nWID, FALSE ))
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
            pStates[i] = PropertyState_DIRECT_VALUE;
    }
    else
        throw RuntimeException();
    return aStates;
}

void SwXFrame::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SwAttrSet& rFmtSet = pFmt->GetAttrSet();
        const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
        if (!pCur)
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
        if ( pCur->nFlags & PropertyAttribute::READONLY)
            throw PropertyVetoException( OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Property is read-only: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );

        BOOL bNextFrame;
        if( pCur->nWID &&
            pCur->nWID != FN_UNO_ANCHOR_TYPES &&
            pCur->nWID != FN_PARAM_LINK_DISPLAY_NAME)
        {
            if(	eType == FLYCNTTYPE_GRF &&
                        (FN_UNO_ALTERNATIVE_TEXT == pCur->nWID||
                        (pCur->nWID >= RES_GRFATR_BEGIN &&
                            pCur->nWID < RES_GRFATR_END)))
            {
                const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
                if(pIdx)
                {
                    SwNodeIndex aIdx(*pIdx, 1);
                    SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
                    if(FN_UNO_ALTERNATIVE_TEXT == pCur->nWID)
                        pNoTxt->SetAlternateText(aEmptyStr);
                    else
                    {
                        SfxItemSet aSet(pNoTxt->GetSwAttrSet());
                        aSet.ClearItem(pCur->nWID);
                        pNoTxt->SetAttr(aSet);
                    }
                }
            }
            else
            {
                SwDoc* pDoc = pFmt->GetDoc();
                SfxItemSet aSet( pDoc->GetAttrPool(),
                    RES_FRMATR_BEGIN, RES_FRMATR_END - 1 );
                aSet.SetParent(&pFmt->GetAttrSet());
                aSet.ClearItem(pCur->nWID);
                if(!rPropertyName.equalsAsciiL( SW_PROP_NAME(UNO_NAME_ANCHOR_TYPE)))
                    pFmt->SetAttr(aSet);
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
        throw RuntimeException();
}

Any SwXFrame::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    SolarMutexGuard aGuard;
    Any aRet;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SfxItemPropertyMap* pCur = SfxItemPropertyMap::GetByName(_pMap, rPropertyName);
        if(pCur)
        {
            if ( pCur->nWID < RES_FRMATR_END )
            {
                const SfxPoolItem& rDefItem =
                    pFmt->GetDoc()->GetAttrPool().GetDefaultItem(pCur->nWID);
                rDefItem.QueryValue(aRet, pCur->nMemberId);
            }
        }
        else
            throw UnknownPropertyException(OUString ( RTL_CONSTASCII_USTRINGPARAM ( "Unknown property: " ) ) + rPropertyName, static_cast < cppu::OWeakObject * > ( this ) );
    }
    else if(!IsDescriptor())
        throw RuntimeException();
    return aRet;
}

void SwXFrame::addEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn())
        throw RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}

void SwXFrame::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw RuntimeException();
}

void 	SwXFrame::Modify( SfxPoolItem *pOld, SfxPoolItem *pNew)
{
    ClientModify(this, pOld, pNew);
    if(!GetRegisteredIn())
    {
        mxStyleData.clear();
        mxStyleFamily.clear();
        mpDoc = 0;
        aLstnrCntnr.Disposing();
    }
}

void SwXFrame::dispose(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        SdrObject* pObj = pFmt->FindSdrObject();
        if( pObj && pObj->IsInserted() )
        {
            if( pFmt->GetAnchor().GetAnchorId() == FLY_IN_CNTNT )
                {
                    const SwPosition &rPos = *(pFmt->GetAnchor().GetCntntAnchor());
                    SwTxtNode *pTxtNode = rPos.nNode.GetNode().GetTxtNode();
                    const xub_StrLen nIdx = rPos.nContent.GetIndex();
                    pTxtNode->Delete( RES_TXTATR_FLYCNT, nIdx, nIdx );
                }
                else
                    pFmt->GetDoc()->DelLayoutFmt(pFmt);
        }
    }
}

uno::Reference< XTextRange >  SwXFrame::getAnchor(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextRange >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    if(pFmt)
    {
        const SwFmtAnchor& rAnchor = pFmt->GetAnchor();
        // return an anchor for non-page bound frames
        // and for page bound frames that have a page no == NULL and a content position
        if( rAnchor.GetAnchorId() != FLY_PAGE ||
            (rAnchor.GetCntntAnchor() && !rAnchor.GetPageNum()))
        {
            const SwPosition &rPos = *(rAnchor.GetCntntAnchor());
            aRef = SwXTextRange::CreateTextRangeFromPosition(pFmt->GetDoc(), rPos, 0);
        }
    }
    else
        throw RuntimeException();
    return aRef;
}

void SwXFrame::ResetDescriptor()
{
    bIsDescriptor = sal_False;
    mxStyleData.clear();
    mxStyleFamily.clear();
    DELETEZ(pProps);
}

void SwXFrame::attachToRange(const uno::Reference< XTextRange > & xTextRange)
            throw( IllegalArgumentException, RuntimeException )
{
    if(!IsDescriptor())
        throw RuntimeException();
    uno::Reference<XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
    SwXTextRange* pRange = 0;
    OTextCursorHelper* pCursor = 0;
    if(xRangeTunnel.is())
    {
        pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                SwXTextRange::getUnoTunnelId());
        pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                OTextCursorHelper::getUnoTunnelId());
    }

    SwDoc* pDoc = pRange ? (SwDoc*)pRange->GetDoc() : pCursor ? (SwDoc*)pCursor->GetDoc() : 0;
    if(pDoc)
    {
        SwUnoInternalPaM aIntPam(*pDoc);
        //das muss jetzt sal_True liefern
        SwXTextRange::XTextRangeToSwPaM(aIntPam, xTextRange);

        SwNode& rNode = pDoc->GetNodes().GetEndOfContent();
        SwPaM aPam(rNode);
        aPam.Move( fnMoveBackward, fnGoDoc );
        static sal_uInt16 __READONLY_DATA aFrmAttrRange[] =
        {
            RES_FRMATR_BEGIN, 		RES_FRMATR_END-1,
            SID_ATTR_BORDER_INNER,	SID_ATTR_BORDER_INNER,
            RES_UNKNOWNATR_CONTAINER, RES_UNKNOWNATR_CONTAINER,
            0
        };
        static sal_uInt16 __READONLY_DATA aGrAttrRange[] =
        {
            RES_GRFATR_BEGIN, 		RES_GRFATR_END-1,
            0
        };
        SfxItemSet aGrSet(pDoc->GetAttrPool(), aGrAttrRange );

        SfxItemSet aFrmSet(pDoc->GetAttrPool(), aFrmAttrRange );
        //jetzt muessen die passenden Items in den Set
        sal_Bool bSizeFound;
        if(!pProps->AnyToItemSet( pDoc, aFrmSet, aGrSet, bSizeFound))
            throw IllegalArgumentException();
        //der TextRange wird einzeln behandelt
        *aPam.GetPoint() = *aIntPam.GetPoint();
        if(aIntPam.HasMark())
        {
            aPam.SetMark();
            *aPam.GetMark() = *aIntPam.GetMark();
        }

        const SfxPoolItem* pItem;
        if(SFX_ITEM_SET == aFrmSet.GetItemState(RES_ANCHOR, sal_False, &pItem) )
        {
            if( FLY_AT_FLY ==((const SwFmtAnchor*)pItem)->GetAnchorId() &&
                !aPam.GetNode()->FindFlyStartNode())
            {
                //rahmengebunden geht nur dort, wo ein Rahmen ist!
                SwFmtAnchor aAnchor(FLY_AT_CNTNT);
                aFrmSet.Put(aAnchor);
            }
            else if( FLY_PAGE ==((const SwFmtAnchor*)pItem)->GetAnchorId() &&
                     0 == ((const SwFmtAnchor*)pItem)->GetPageNum() )
            {
                SwFmtAnchor aAnchor( *((const SwFmtAnchor*)pItem) );
                aAnchor.SetAnchor( aPam.GetPoint() );
                aFrmSet.Put(aAnchor);
            }
        }

        uno::Any* pStyle;
        SwFrmFmt *pParentFrmFmt = 0;
        if(pProps->GetProperty(FN_UNO_FRAME_STYLE_NAME, 0, pStyle))
            pParentFrmFmt = lcl_GetFrmFmt( *pStyle, pDoc );

        SwFlyFrmFmt* pFmt = 0;
        if( eType == FLYCNTTYPE_FRM)
        {
            UnoActionContext aCont(pDoc);
            pFmt = pDoc->MakeFlySection( FLY_AT_CNTNT, aPam.GetPoint(),
                                         &aFrmSet, pParentFrmFmt );
            if(pFmt)
            {
                pFmt->Add(this);
                if(sName.Len())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFmt, sName);
            }
            //den SwXText wecken
            ((SwXTextFrame*)this)->SetDoc( bIsDescriptor ? mpDoc : GetFrmFmt()->GetDoc() );
        }
        else if( eType == FLYCNTTYPE_GRF)
        {
            UnoActionContext aCont(pDoc);
            uno::Any* pGraphicURL;
            String sGraphicURL;
            BfGraphicObject* pGrfObj = 0;
            if(pProps->GetProperty(FN_UNO_GRAPHIC_U_R_L, 0, pGraphicURL))
            {
                OUString uTemp;
                (*pGraphicURL) >>= uTemp;
                sGraphicURL = String(uTemp);
                if( sGraphicURL.EqualsAscii( sPackageProtocol,
                                               0, sizeof( sPackageProtocol )-1 ) )
                {
                    pGrfObj = new BfGraphicObject;
                    pGrfObj->SetUserData( sGraphicURL );
                    pGrfObj->SetSwapState();
                    sGraphicURL.Erase();
                }
                else if( sGraphicURL.EqualsAscii( sGraphicObjectProtocol,
                                       0, sizeof(sGraphicObjectProtocol)-1 ) )
                {
                    ByteString sId( sGraphicURL.Copy( sizeof(sGraphicObjectProtocol)-1 ),
                                    RTL_TEXTENCODING_ASCII_US );
                    pGrfObj = new BfGraphicObject( sId );
                    sGraphicURL.Erase();
                }
            }

            String sFltName;
            uno::Any* pFilter;
            if(pProps->GetProperty(FN_UNO_GRAPHIC_FILTER, 0, pFilter))
            {
                OUString uTemp;
                (*pFilter) >>= uTemp;
                sFltName = String(uTemp);
            }

            pFmt =
                pGrfObj ? pDoc->Insert( aPam, *pGrfObj, &aFrmSet, &aGrSet,
                                        pParentFrmFmt )
                         : pDoc->Insert( aPam, sGraphicURL, sFltName, 0,
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
            uno::Any* pSurroundContour;
            if(pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUR, pSurroundContour))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_SURROUND_CONTOUR)), *pSurroundContour);
            uno::Any* pContourOutside;
            if(pProps->GetProperty(RES_SURROUND, MID_SURROUND_CONTOUROUTSIDE, pContourOutside))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_CONTOUR_OUTSIDE)), *pContourOutside);
            uno::Any* pContourPoly;
            if(pProps->GetProperty(FN_PARAM_COUNTOUR_PP, 0, pContourPoly))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_CONTOUR_POLY_POLYGON)), *pContourPoly);
            uno::Any* pPixelContour;
            if(pProps->GetProperty(FN_UNO_IS_PIXEL_CONTOUR, 0, pPixelContour))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_IS_PIXEL_CONTOUR)), *pPixelContour);
            uno::Any* pAutoContour;
            if(pProps->GetProperty(FN_UNO_IS_AUTOMATIC_CONTOUR, 0, pAutoContour))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_IS_AUTOMATIC_CONTOUR)), *pAutoContour);
            uno::Any* pAltText;
            if(pProps->GetProperty(FN_UNO_ALTERNATIVE_TEXT, 0, pAltText))
                setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_ALTERNATIVE_TEXT)), *pAltText);
        }
        else
        {
            uno::Any* pCLSID;
            if(!pProps->GetProperty(FN_UNO_CLSID, 0, pCLSID))
                throw RuntimeException();
            OUString aCLSID;
            SvGlobalName aClassName;
            SvInPlaceObjectRef xIPObj;
            if( (*pCLSID) >>= aCLSID )
            {
                sal_Bool bInternal = sal_True;
                if( !aClassName.MakeId( aCLSID ) )
                {
                    IllegalArgumentException aExcept;
                    aExcept.Message = OUString::createFromAscii("CLSID invalid");
                    throw aExcept;
                }
                const SotFactory* pFact = SvFactory::Find( aClassName );
                if ( pFact )
                {
                    SvStorageRef aStor = new SvStorage( aEmptyStr );
                    xIPObj = &((SvFactory*)SvInPlaceObject::ClassFactory())->CreateAndInit( aClassName, aStor );
                }
                else
                {
                    SvStorageRef aStor = new SvStorage( FALSE, aEmptyStr );
                    String aFileName;
                    BOOL bOk;
                    xIPObj = SvOutPlaceObject::InsertObject( NULL, &aStor, bOk, aClassName, aFileName );
                }
            }
            if ( xIPObj.Is() )
            {
                if( SVOBJ_MISCSTATUS_RESIZEONPRINTERCHANGE & xIPObj->GetMiscStatus() && pDoc->GetPrt() )
                    xIPObj->OnDocumentPrinterChanged( pDoc->GetPrt() );

                UnoActionContext aAction(pDoc);
                ULONG lDummy;
                String aDummy;
                // determine source CLSID
                xIPObj->SvPseudoObject::FillClass( &aClassName, &lDummy, &aDummy, &aDummy, &aDummy);

                if(!bSizeFound)
                {
                    //The Size should be suggested by the OLE server if not manually set
                    MapMode aRefMap( xIPObj->GetMapUnit() );
                    Size aSz( xIPObj->GetVisArea().GetSize() );
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
                SwFlyFrmFmt* pFmt = 0;

                pFmt = pDoc->Insert(aPam, &xIPObj, &aFrmSet );
                ASSERT( pFmt, "Doc->Insert(notxt) failed." );

                pFmt->Add(this);
                if(sName.Len())
                    pDoc->SetFlyName((SwFlyFrmFmt&)*pFmt, sName);
            }
        }

        if( pFmt && pDoc->GetDrawModel() )
            GetOrCreateSdrObject( pFmt );
        uno::Any* pOrder;
        if( pProps->GetProperty(FN_UNO_Z_ORDER, 0, pOrder) )
            setPropertyValue(C2U(SW_PROP_NAME_STR(UNO_NAME_Z_ORDER)), *pOrder);
    }
    else
        throw IllegalArgumentException();
    //setzt das Flag zurueck und loescht den Descriptor-Pointer
    ResetDescriptor();
}

void SwXFrame::attach(const uno::Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, RuntimeException )
{
    SwFrmFmt* pFmt;
    if(IsDescriptor())
        attachToRange(xTextRange);
    else if(0 != (pFmt = GetFrmFmt()))
    {
        Reference<XUnoTunnel> xRangeTunnel( xTextRange, UNO_QUERY);
        SwXTextRange* pRange = 0;
        OTextCursorHelper* pCursor = 0;
        if(xRangeTunnel.is())
        {
            pRange = (SwXTextRange*)xRangeTunnel->getSomething(
                                    SwXTextRange::getUnoTunnelId());
            pCursor = (OTextCursorHelper*)xRangeTunnel->getSomething(
                                    OTextCursorHelper::getUnoTunnelId());
        }
        SwDoc* pDoc = pFmt->GetDoc();
        SwUnoInternalPaM aIntPam(*pDoc);
        if(SwXTextRange::XTextRangeToSwPaM(aIntPam, xTextRange))
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
            throw IllegalArgumentException();
    }
}

awt::Point SwXFrame::getPosition(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    RuntimeException aRuntime;
    aRuntime.Message = C2U("position cannot be determined with this method");
    throw aRuntime;
    return awt::Point();
}

void SwXFrame::setPosition(const awt::Point& aPosition) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    RuntimeException aRuntime;
    aRuntime.Message = C2U("position cannot be changed with this method");
    throw aRuntime;
}

awt::Size SwXFrame::getSize(void) throw( RuntimeException )
{
    uno::Any aVal = getPropertyValue(C2U("Size"));
    awt::Size* pRet =  (awt::Size*)aVal.getValue();
    return *pRet;
}

void SwXFrame::setSize(const awt::Size& aSize)
    throw( PropertyVetoException, RuntimeException )
{
    uno::Any aVal(&aSize, ::getCppuType((const awt::Size*)0));
    setPropertyValue(C2U("Size"), aVal);
}

OUString SwXFrame::getShapeType(void) throw( RuntimeException )
{
    return C2U("FrameShape");
}

/******************************************************************
 *	SwXTextFrame
 ******************************************************************/
SwXTextFrame::SwXTextFrame( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_FRM, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME), pDoc ),
    SwXText(0, CURSOR_FRAME)
{
}

SwXTextFrame::SwXTextFrame(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_FRM, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME)),
    SwXText(rFmt.GetDoc(), CURSOR_FRAME)
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

uno::Any SAL_CALL SwXTextFrame::queryInterface( const uno::Type& aType )
    throw (RuntimeException)
{
    uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXText::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextFrameBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL SwXTextFrame::getTypes(  ) throw(RuntimeException)
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

uno::Sequence< sal_Int8 > SAL_CALL SwXTextFrame::getImplementationId(  ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

uno::Reference< XText >  SwXTextFrame::getText(void) throw( RuntimeException )
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

uno::Reference< XTextCursor >   SwXTextFrame::createCursor() throw ( ::com::sun::star::uno::RuntimeException)
{
    return createTextCursor();
}

uno::Reference< XTextCursor >  SwXTextFrame::createTextCursor(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextCursor >  aRef;
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

        SwXTextCursor* pXCrsr = new SwXTextCursor(this, *aPam.GetPoint(), CURSOR_FRAME, pFmt->GetDoc());
        aRef =  (XWordCursor*)pXCrsr;
        SwUnoCrsr*	pUnoCrsr = pXCrsr->GetCrsr();
    }
    else
        throw RuntimeException();
    return aRef;
}

uno::Reference< XTextCursor >  SwXTextFrame::createTextCursorByRange(const uno::Reference< XTextRange > & aTextPosition) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< XTextCursor >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
    SwUnoInternalPaM aPam(*GetDoc());
    if(pFmt && SwXTextRange::XTextRangeToSwPaM(aPam, aTextPosition))
    {
        SwNode& rNode = pFmt->GetCntnt().GetCntntIdx()->GetNode();
#if OSL_DEBUG_LEVEL > 1
        const SwStartNode* p1 = aPam.GetNode()->FindFlyStartNode();
        const SwStartNode* p2 = rNode.FindFlyStartNode();
#endif
        if(aPam.GetNode()->FindFlyStartNode() == rNode.FindFlyStartNode())
            aRef =  (XWordCursor*)new SwXTextCursor(this ,
                *aPam.GetPoint(), CURSOR_FRAME, pFmt->GetDoc(), aPam.GetMark());
    }
    else
        throw RuntimeException();
    return aRef;
}

uno::Reference< container::XEnumeration >  SwXTextFrame::createEnumeration(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Reference< container::XEnumeration >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
 uno::XInterface* pRet = 0;
    if(pFmt)
    {
        SwPosition aPos(pFmt->GetCntnt().GetCntntIdx()->GetNode());
        SwUnoCrsr* pUnoCrsr = GetDoc()->CreateUnoCrsr(aPos, sal_False);
        pUnoCrsr->Move( fnMoveForward, fnGoNode );
        aRef = new SwXParagraphEnumeration(this, pUnoCrsr, CURSOR_FRAME);
    }
    return aRef;
}

uno::Type  SwXTextFrame::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}

sal_Bool SwXTextFrame::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}

void SwXTextFrame::attach(const uno::Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, RuntimeException )
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< XTextRange >  SwXTextFrame::getAnchor(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextFrame::dispose(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextFrame::addEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextFrame::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}

OUString SwXTextFrame::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextFrame");
}

sal_Bool SwXTextFrame::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.Text")||
            COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextFrame")||
                    SwXFrame::supportsService(rServiceName);
}

uno::Sequence< OUString > SwXTextFrame::getSupportedServiceNames(void) throw( RuntimeException )
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 2);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 2] = C2U("com.sun.star.text.TextFrame");
    pArray[aRet.getLength() - 1] = C2U("com.sun.star.text.Text");
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
    throw(RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}

sal_Int64 SAL_CALL SwXTextFrame::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    sal_Int64 nRet = SwXFrame::getSomething( rId );
    if( !nRet )
        nRet = SwXText::getSomething( rId );

    return nRet;
}

uno::Any SwXTextFrame::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
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
 *	SwXTextGraphicObject
 ******************************************************************/
SwXTextGraphicObject::SwXTextGraphicObject( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_GRF, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC), pDoc)
{
}

SwXTextGraphicObject::SwXTextGraphicObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_GRF, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC))
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

uno::Any SAL_CALL SwXTextGraphicObject::queryInterface( const uno::Type& aType )
    throw(RuntimeException)
{
    uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextGraphicObjectBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL
    SwXTextGraphicObject::getTypes(  ) throw(RuntimeException)
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

uno::Sequence< sal_Int8 > SAL_CALL SwXTextGraphicObject::getImplementationId(  ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

void SwXTextGraphicObject::attach(const uno::Reference< XTextRange > & xTextRange) throw( IllegalArgumentException, RuntimeException )
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< XTextRange >  SwXTextGraphicObject::getAnchor(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextGraphicObject::dispose(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextGraphicObject::addEventListener(const uno::Reference< XEventListener > & aListener)
                                                    throw( RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextGraphicObject::removeEventListener(const uno::Reference< XEventListener > & aListener)
                                                    throw( RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}

OUString SwXTextGraphicObject::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextGraphicObject");
}

sal_Bool SwXTextGraphicObject::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextGraphicObject") ||
                    SwXFrame::supportsService(rServiceName);
}

uno::Sequence< OUString > SwXTextGraphicObject::getSupportedServiceNames(void)
        throw( RuntimeException )
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 1);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 1] = C2U("com.sun.star.text.TextGraphicObject");
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
        throw(RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}

/******************************************************************
 * SwXTextEmbeddedObject
 ******************************************************************/
SwXTextEmbeddedObject::SwXTextEmbeddedObject( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_OLE, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_EMBEDDED_OBJECT), pDoc)
{
}

SwXTextEmbeddedObject::SwXTextEmbeddedObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_OLE, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_EMBEDDED_OBJECT))
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

uno::Any SAL_CALL SwXTextEmbeddedObject::queryInterface( const uno::Type& aType )
    throw( RuntimeException)
{
    uno::Any aRet = SwXFrame::queryInterface(aType);;
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextEmbeddedObjectBaseClass::queryInterface(aType);
    return aRet;
}

uno::Sequence< uno::Type > SAL_CALL SwXTextEmbeddedObject::getTypes(  ) throw(RuntimeException)
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

uno::Sequence< sal_Int8 > SAL_CALL SwXTextEmbeddedObject::getImplementationId(  ) throw(RuntimeException)
{
    SolarMutexGuard aGuard;
    static Sequence< sal_Int8 > aId( 16 );
    static sal_Bool bInit = sal_False;
    if(!bInit)
    {
        rtl_createUuid( (sal_uInt8 *)(aId.getArray() ), 0, sal_True );
        bInit = sal_True;
    }
    return aId;
}

void SwXTextEmbeddedObject::attach(const uno::Reference< XTextRange > & xTextRange) throw( IllegalArgumentException, RuntimeException )
{
    SwXFrame::attach(xTextRange);
}

uno::Reference< XTextRange >  SwXTextEmbeddedObject::getAnchor(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    return SwXFrame::getAnchor();
}

void SwXTextEmbeddedObject::dispose(void) throw( RuntimeException )
{
    SolarMutexGuard aGuard;
    SwXFrame::dispose();
}

void SwXTextEmbeddedObject::addEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}

void SwXTextEmbeddedObject::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}

static uno::Reference< util::XModifyListener >  xSwXOLEListener;

uno::Reference< XComponent >  SwXTextEmbeddedObject::getEmbeddedObject(void) throw( RuntimeException )
{
    uno::Reference< XComponent >  xRet;
    SwFrmFmt* 	pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        DBG_ASSERT(	pCnt->GetCntntIdx() &&
                       pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?");

        SwOLENode* pOleNode =  pDoc->GetNodes()[ pCnt->GetCntntIdx()
                                        ->GetIndex() + 1 ]->GetOLENode();
        SvInPlaceObjectRef xIP( pOleNode->GetOLEObj().GetOleRef() );
        if (xIP.Is())
        {
            SfxInPlaceObjectRef xSfxObj( xIP );
            if(xSfxObj.Is())
            {
                SfxObjectShell* pObjSh = xSfxObj->GetObjectShell();
                if( pObjSh )
                {
                    uno::Reference< frame::XModel > xModel = pObjSh->GetBaseModel();
                    xRet = uno::Reference< XComponent >(xModel, uno::UNO_QUERY);

                    uno::Reference< util::XModifyBroadcaster >  xBrdcst(xModel, uno::UNO_QUERY);
                    if( xBrdcst.is() )
                    {
                        SwXOLEListener* pSwOLEListener = (SwXOLEListener*)
                                                        xSwXOLEListener.get();
                        if( !pSwOLEListener )
                            xSwXOLEListener = pSwOLEListener = new SwXOLEListener;
                        if( pSwOLEListener->AddOLEFmt( *pFmt ) )
                            xBrdcst->addModifyListener( xSwXOLEListener );
                    }
                }
            }
            else
            {
                SvOutPlaceObjectRef xOut( pOleNode->GetOLEObj().GetOleRef() );
                if ( xOut.Is() )
                    xRet = xOut->GetUnoComponent();
            }
        }
    }
    return xRet;
}

OUString SwXTextEmbeddedObject::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextEmbeddedObject");
}

sal_Bool SwXTextEmbeddedObject::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return  COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextEmbeddedObject")||
                    SwXFrame::supportsService(rServiceName);
}

uno::Sequence< OUString > SwXTextEmbeddedObject::getSupportedServiceNames(void)
        throw( RuntimeException )
{
    uno::Sequence < OUString > aRet = SwXFrame::getSupportedServiceNames();
    aRet.realloc(aRet.getLength() + 1);
    OUString* pArray = aRet.getArray();
    pArray[aRet.getLength() - 1] = C2U("com.sun.star.text.TextEmbeddedObject");
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
        throw(RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}

/******************************************************************
 * SwXOLEListener
 ******************************************************************/
void SwXOLEListener::modified( const EventObject& rEvent )
                                        throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    SwOLENode* pNd;
    sal_uInt16 nFndPos = FindEntry( rEvent, &pNd );
    if( USHRT_MAX != nFndPos && ( !pNd->GetOLEObj().IsOleRef() ||
            !pNd->GetOLEObj().GetOleRef()->GetProtocol().IsInPlaceActive() ))
    {
        // if the OLE-Node is UI-Active do nothing
        pNd->SetOLESizeInvalid( sal_True );
        pNd->GetDoc()->SetOLEObjModified();
    }
}

void SwXOLEListener::disposing( const EventObject& rEvent )
                        throw( RuntimeException )
{
    SolarMutexGuard aGuard;

    uno::Reference< util::XModifyListener >  xListener( this );

    SwOLENode* pNd;
    sal_uInt16 nFndPos = FindEntry( rEvent, &pNd );
    if( USHRT_MAX != nFndPos )
    {
        SwDepend* pDepend = (SwDepend*)aFmts[ nFndPos ];
        aFmts.Remove( nFndPos, 1 );

        uno::Reference< frame::XModel >  xModel( rEvent.Source, uno::UNO_QUERY );
        uno::Reference< util::XModifyBroadcaster >  xBrdcst(xModel, uno::UNO_QUERY);

        if( xBrdcst.is() )
            xBrdcst->removeModifyListener( xListener );

        delete pDepend;
        if( !aFmts.Count() )
        {
            // we are the last?
            // then can we delete us
            xSwXOLEListener = 0;
        }
    }
}

sal_Bool SwXOLEListener::AddOLEFmt( SwFrmFmt& rFmt )
{
    for( sal_uInt16 n = 0, nCnt = aFmts.Count(); n < nCnt; ++n )
        if( &rFmt == ((SwDepend*)aFmts[ n ])->GetRegisteredIn() )
            return sal_False;		// is in the array

    SwDepend* pNew = new SwDepend( this, &rFmt );
    aFmts.Insert( pNew, aFmts.Count() );
    return sal_True;
}

void SwXOLEListener::Modify( SfxPoolItem* pOld, SfxPoolItem* pNew )
{
    const SwClient* pClient = 0;

    switch( pOld ? pOld->Which() : 0 )
    {
    case RES_REMOVE_UNO_OBJECT:
    case RES_OBJECTDYING:
        pClient = (SwClient*)((SwPtrMsgPoolItem *)pOld)->pObject;
        break;

    case RES_FMT_CHG:
        // wurden wir an das neue umgehaengt und wird das alte geloscht?
        if( ((SwFmtChg*)pOld)->pChangedFmt->IsFmtInDTOR() )
        {
            pClient = ((SwFmtChg*)pNew)->pChangedFmt;
        }
        break;
    }

    if( pClient )
    {
        uno::Reference< util::XModifyListener >  xListener( this );

        SwDepend* pDepend;
        for( sal_uInt16 n = 0, nCnt = aFmts.Count(); n < nCnt; ++n )
        {
            if( pClient == (pDepend = (SwDepend*)aFmts[ n ])->GetRegisteredIn() )
            {
                aFmts.Remove( n, 1 );

                 uno::Reference<frame::XModel> xModel = GetModel( *(SwFmt*)pClient );
                if( xModel.is() )
                {
                    uno::Reference< util::XModifyBroadcaster >  xBrdcst(xModel, uno::UNO_QUERY);
                    if( xBrdcst.is() )
                        xBrdcst->removeModifyListener( xListener );
                }

                delete pDepend;
                if( !aFmts.Count() )
                {
                    // we are the last?
                    // then can we delete us
                    xSwXOLEListener = 0;
                }
                break;
            }
        }
    }
}

uno::Reference< frame::XModel > SwXOLEListener::GetModel( const SwFmt& rFmt, SwOLENode** ppNd ) const
{
    SfxObjectShell* pObjSh = GetObjShell( rFmt, ppNd );
    return pObjSh ? pObjSh->GetBaseModel() : (uno::Reference< frame::XModel >)0;
}

SfxObjectShell* SwXOLEListener::GetObjShell( const SwFmt& rFmt,
                                            SwOLENode** ppNd ) const
{
    SfxObjectShell* pShell = 0;
    const SwFmtCntnt& rCnt = rFmt.GetCntnt();
    if( rCnt.GetCntntIdx() )
    {
        SwNodeIndex aIdx( *rCnt.GetCntntIdx(), 1 );
        SwOLENode* pOleNode = aIdx.GetNode().GetOLENode();
        if( pOleNode && pOleNode->GetOLEObj().IsOleRef() )
        {
            SfxInPlaceObjectRef xIP( pOleNode->GetOLEObj().GetOleRef() );
            if( xIP.Is() )
                pShell = xIP->GetObjectShell();
        }
        if( ppNd )
            *ppNd = pOleNode;
    }
    return pShell;
}

sal_uInt16 SwXOLEListener::FindEntry( const EventObject& rEvent,SwOLENode** ppNd)
{
    sal_uInt16 nRet = USHRT_MAX;
    uno::Reference< frame::XModel >  xSrch( rEvent.Source, uno::UNO_QUERY );

    for( sal_uInt16 n = 0, nCnt = aFmts.Count(); n < nCnt; ++n )
    {
        SwDepend* pDepend = (SwDepend*)aFmts[ n ];
        SwFrmFmt* pFmt = (SwFrmFmt*)pDepend->GetRegisteredIn();
        if( !pFmt )
        {
            ASSERT( pFmt, "wo ist das Format geblieben?" );
            aFmts.Remove( n, 1 );
            delete pDepend;
            --n;
            --nCnt;
        }
        else
        {
            uno::Reference< frame::XModel >  xFmt( GetModel( *pFmt, ppNd ), uno::UNO_QUERY);
            if( xFmt == xSrch )
            {
                nRet = n;
                break;
            }
        }
    }
    return nRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
