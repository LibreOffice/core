 /*************************************************************************
 *
 *  $RCSfile: unoframe.cxx,v $
 *
 *  $Revision: 1.83 $
 *
 *  last change: $Author: hr $ $Date: 2003-09-29 15:05:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/


#pragma hdrstop

#include <swtypes.hxx>
#include <cmdid.h>

#ifndef _HINTS_HXX //autogen
#include <hints.hxx>
#endif
#ifndef _DOC_HXX //autogen
#include <doc.hxx>
#endif
#ifndef _DOCSH_HXX //autogen
#include <docsh.hxx>
#endif
#ifndef _NDINDEX_HXX //autogen
#include <ndindex.hxx>
#endif
#ifndef _PAM_HXX //autogen
#include <pam.hxx>
#endif
#ifndef _NDNOTXT_HXX //autogen
#include <ndnotxt.hxx>
#endif
#ifndef _SVX_UNOMID_HXX
#include <svx/unomid.hxx>
#endif
#ifndef _UNOCRSR_HXX //autogen
#include <unocrsr.hxx>
#endif
#ifndef _DOCSTYLE_HXX //autogen
#include <docstyle.hxx>
#endif
#ifndef _DCONTACT_HXX
#include <dcontact.hxx>
#endif
#ifndef _FMTCNCT_HXX //autogen
#include <fmtcnct.hxx>
#endif
#ifndef _NDOLE_HXX //autogen
#include <ndole.hxx>
#endif
#ifndef _UNOOBJ_HXX
#include <unoobj.hxx>
#endif
#ifndef _UNOMAP_HXX
#include <unomap.hxx>
#endif
#ifndef _UNOPRNMS_HXX
#include <unoprnms.hxx>
#endif
#ifndef _UNOEVENT_HXX
#include <unoevent.hxx>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFYBROADCASTER_HPP_
#include <com/sun/star/util/XModifyBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_SHADOWFORMAT_HPP_
#include <com/sun/star/table/ShadowFormat.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_GRAPHICLOCATION_HPP_
#include <com/sun/star/style/GraphicLocation.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_GRAPHICCROP_HPP_
#include <com/sun/star/text/GraphicCrop.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_TEXTCONTENTANCHORTYPE_HPP_
#include <com/sun/star/text/TextContentAnchorType.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_XTEXTCOLUMNS_HPP_
#include <com/sun/star/text/XTextColumns.hpp>
#endif
#ifndef _COM_SUN_STAR_TEXT_WRAPTEXTMODE_HPP_
#include <com/sun/star/text/WrapTextMode.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCESEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequenceSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_POINTSEQUENCE_HPP_
#include <com/sun/star/drawing/PointSequence.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_COLORMODE_HPP_
#include <com/sun/star/drawing/ColorMode.hpp>
#endif
#ifndef _SV_POLY_HXX //autogen
#include <vcl/poly.hxx>
#endif
#ifndef _SWUNDO_HXX //autogen
#include <swundo.hxx>
#endif
#ifndef SW_UNOMID_HXX
#include <unomid.h>
#endif
#ifndef _UNOSTYLE_HXX
#include <unostyle.hxx>
#endif
#ifndef _SVDMODEL_HXX
#include <svx/svdmodel.hxx>
#endif
#ifndef _SVDPAGE_HXX
#include <svx/svdpage.hxx>
#endif
#ifndef _SVX_BRSHITEM_HXX //autogen
#include <svx/brshitem.hxx>
#endif
#ifndef _SVX_PROTITEM_HXX //autogen
#include <svx/protitem.hxx>
#endif
#ifndef _FMTORNT_HXX //autogen
#include <fmtornt.hxx>
#endif
#ifndef _FMTURL_HXX //autogen
#include <fmturl.hxx>
#endif
#ifndef _SVX_LRSPITEM_HXX //autogen
#include <svx/lrspitem.hxx>
#endif
#ifndef _SVX_ULSPITEM_HXX //autogen
#include <svx/ulspitem.hxx>
#endif
#ifndef _SVX_BOXITEM_HXX //autogen
#include <svx/boxitem.hxx>
#endif
#ifndef _SVX_OPAQITEM_HXX //autogen
#include <svx/opaqitem.hxx>
#endif
#ifndef _SVX_PRNTITEM_HXX //autogen
#include <svx/prntitem.hxx>
#endif
#ifndef _SVX_SHADITEM_HXX //autogen
#include <svx/shaditem.hxx>
#endif
#ifndef _FMTSRND_HXX //autogen
#include <fmtsrnd.hxx>
#endif
#ifndef _FMTFSIZE_HXX //autogen
#include <fmtfsize.hxx>
#endif
#ifndef _GRFATR_HXX //autogen
#include <grfatr.hxx>
#endif
#ifndef _UNOFRAME_HXX
#include <unoframe.hxx>
#endif
#ifndef _FMTANCHR_HXX
#include <fmtanchr.hxx>
#endif
#ifndef _FMTCLDS_HXX
#include <fmtclds.hxx>
#endif
#ifndef _FRMATR_HXX
#include <frmatr.hxx>
#endif
#ifndef _NDTXT_HXX
#include <ndtxt.hxx>
#endif
#ifndef _NDGRF_HXX
#include <ndgrf.hxx>
#endif
#ifndef _VOS_MUTEX_HXX_ //autogen
#include <vos/mutex.hxx>
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SVDOBJ_HXX //autogen
#include <svx/svdobj.hxx>
#endif
#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif
#ifndef _SWSTYLENAMEMAPPER_HXX
#include <SwStyleNameMapper.hxx>
#endif
#ifndef _XMLOFF_XMLCNITM_HXX
#include <xmloff/xmlcnitm.hxx>
#endif
#ifndef _POOLFMT_HXX
#include <poolfmt.hxx>
#endif
#ifndef _PAGEDESC_HXX
#include <pagedesc.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef _SVX_FRMDIRITEM_HXX //autogen
#include <svx/frmdiritem.hxx>
#endif

#include <so3/outplace.hxx>

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
using namespace ::rtl;

using com::sun::star::frame::XModel;
using com::sun::star::container::XNameAccess;
using com::sun::star::style::XStyleFamiliesSupplier;

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
    void            GetProperty(const OUString &rPropertyName, const Reference < XPropertySet > &rxPropertySet, uno::Any& rAny );

    const SfxItemPropertyMap*       GetMap() const {return _pMap;}
    sal_Bool                        FillBaseProperties(SfxItemSet& rToSet, const SfxItemSet &rFromSet, sal_Bool& rSizeFound);

    virtual sal_Bool                AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound) = 0;

};
/* -----------------------------12.06.01 15:46--------------------------------

 ---------------------------------------------------------------------------*/
BaseFrameProperties_Impl::~BaseFrameProperties_Impl()
{
    uno::Any* pVal = aAnyTbl.First();
    while(pVal)
    {
        delete pVal;
        pVal = aAnyTbl.Next();
    }
}
/* -----------------------------12.06.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------12.06.01 15:43--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------29.06.98 09:55-------------------
 *
 * --------------------------------------------------*/
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
    uno::Any* pSur   = 0;
    GetProperty(RES_SURROUND, MID_SURROUND_SURROUNDTYPE, pSur);
    uno::Any* pSurAnch = 0;
    GetProperty(RES_SURROUND, MID_SURROUND_ANCHORONLY, pSurAnch);
    if(pSur || pSurAnch)
    {
        SwFmtSurround aSrnd ( static_cast < const SwFmtSurround & > ( rFromSet.Get ( RES_SURROUND ) ) );
        if(pSur)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSur, MID_SURROUND_SURROUNDTYPE );
        if(pSurAnch)
            bRet &= ((SfxPoolItem&)aSrnd).PutValue(*pSurAnch, MID_SURROUND_ANCHORONLY);
        rToSet.Put(aSrnd);
    }
    uno::Any* pLeft         = 0;
    GetProperty(RES_BOX, LEFT_BORDER  |CONVERT_TWIPS,    pLeft  );
    uno::Any* pRight        = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|RIGHT_BORDER ,    pRight );
    uno::Any* pTop      = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|TOP_BORDER     , pTop   );
    uno::Any* pBottom   = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|BOTTOM_BORDER,    pBottom);
    uno::Any* pDistance     = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|BORDER_DISTANCE,  pDistance);
    uno::Any* pLeftDistance     = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|LEFT_BORDER_DISTANCE, pLeftDistance);
    uno::Any* pRightDistance    = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|RIGHT_BORDER_DISTANCE,    pRightDistance);
    uno::Any* pTopDistance  = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|TOP_BORDER_DISTANCE,  pTopDistance);
    uno::Any* pBottomDistance   = 0;
    GetProperty(RES_BOX, CONVERT_TWIPS|BOTTOM_BORDER_DISTANCE,   pBottomDistance);
    if( pLeft || pRight || pTop ||  pBottom || pDistance ||
        pLeftDistance  || pRightDistance || pTopDistance || pBottomDistance )
    {
        SvxBoxItem aBox ( static_cast < const SvxBoxItem & > ( rFromSet.Get ( RES_BOX ) ) );
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
/* -----------------22.06.98 09:17-------------------
 *
 * --------------------------------------------------*/

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
/* -----------------22.06.98 09:17-------------------
 *
 * --------------------------------------------------*/
SwFrameProperties_Impl::SwFrameProperties_Impl():
    BaseFrameProperties_Impl(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME) )
{
}
/* -----------------22.06.98 11:27-------------------
 *
 * --------------------------------------------------*/
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
/* -----------------27.06.98 14:53-------------------
 *
 * --------------------------------------------------*/
SwGraphicProperties_Impl::SwGraphicProperties_Impl( ) :
    BaseFrameProperties_Impl(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC) )
{
}

/* -----------------27.06.98 14:40-------------------
 *
 * --------------------------------------------------*/
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

sal_Bool    SwGraphicProperties_Impl::AnyToItemSet(
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
        SwFrameProperties_Impl(aSwMapProvider.GetPropertyMap(PROPERTY_MAP_EMBEDDED_OBJECT) ){}
    ~SwOLEProperties_Impl(){}

    virtual sal_Bool        AnyToItemSet( SwDoc* pDoc, SfxItemSet& rFrmSet, SfxItemSet& rSet, sal_Bool& rSizeFound);
};
/* -----------------4/1/2003 15:32-------------------

 --------------------------------------------------*/

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
 *  SwXFrame
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXFrame::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
    return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
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
/*-----------------24.03.98 14:49-------------------

--------------------------------------------------*/

TYPEINIT1(SwXFrame, SwClient);
/* -----------------------------06.04.00 14:20--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXFrame::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXFrame");
}
/* -----------------------------06.04.00 14:20--------------------------------

 ---------------------------------------------------------------------------*/
BOOL SwXFrame::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return !rServiceName.compareToAscii("com.sun.star.text.BaseFrame")||
                !rServiceName.compareToAscii("com.sun.star.text.TextContent") ||
                    !rServiceName.compareToAscii("com.sun.star.document.LinkTarget");
}
/* -----------------------------06.04.00 14:20--------------------------------

 ---------------------------------------------------------------------------*/
Sequence< OUString > SwXFrame::getSupportedServiceNames(void) throw( RuntimeException )
{
    Sequence< OUString > aRet(3);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.text.BaseFrame");
    pArray[1] = C2U("com.sun.star.text.TextContent");
    pArray[2] = C2U("com.sun.star.document.LinkTarget");
    return aRet;
}
/*-- 11.12.98 15:05:00---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFrame::SwXFrame(  ) :
    eType(FLYCNTTYPE_FRM),
    aLstnrCntnr( (container::XNamed*)this),
    aPropSet(0),
    _pMap(0),
    bIsDescriptor(sal_False),
    pProps(0)
{
}
/*-- 14.01.99 11:31:52---------------------------------------------------
    Dieser CTor legt den Frame als Descriptor an
  -----------------------------------------------------------------------*/
SwXFrame::SwXFrame(FlyCntType eSet, const SfxItemPropertyMap* pMap, SwDoc *pDoc ) :
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

/*-- 11.12.98 15:05:01---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 11.12.98 15:05:02---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXFrame::~SwXFrame()
{
    delete pProps;
}
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXFrame::getName(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setName(const OUString& rName) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/*-- 11.12.98 15:05:03---------------------------------------------------

  -----------------------------------------------------------------------*/
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
/*-- 11.12.98 15:05:04---------------------------------------------------

  -----------------------------------------------------------------------*/

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
    vos::OGuard aGuard(Application::GetSolarMutex());
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
        if( eType == FLYCNTTYPE_GRF &&
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
                        if( !::lcl_ChkAndSetNewAnchor( *pFly, *pSet ))
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
            GraphicObject *pGrfObj = 0;
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

            const SwNodeIndex* pIdx = pFmt->GetCntnt().GetCntntIdx();
            if(pIdx)
            {
                SwNodeIndex aIdx(*pIdx, 1);
//              SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
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
                        ::lcl_ChkAndSetNewAnchor( *pFly, aSet );
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
/*-- 11.12.98 15:05:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXFrame::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
//              SwNoTxtNode* pNoTxt = aIdx.GetNode().GetNoTxtNode();
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
                                            GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?")

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
/*-- 11.12.98 15:05:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::addPropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 15:05:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::removePropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< XPropertyChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 15:05:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::addVetoableChangeListener(const OUString& PropertyName,
                                const uno::Reference< XVetoableChangeListener > & aListener)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 11.12.98 15:05:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::removeVetoableChangeListener(
    const OUString& PropertyName, const uno::Reference< XVetoableChangeListener > & aListener)
        throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    DBG_WARNING("not implemented")
}
/*-- 12.09.00 14:04:53---------------------------------------------------

  -----------------------------------------------------------------------*/
PropertyState SwXFrame::getPropertyState( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    Sequence< OUString > aPropertyNames(1);
    OUString* pNames = aPropertyNames.getArray();
    pNames[0] = rPropertyName;
    Sequence< PropertyState > aStates = getPropertyStates(aPropertyNames);
    return aStates.getConstArray()[0];
}
/*-- 12.09.00 14:04:54---------------------------------------------------

  -----------------------------------------------------------------------*/
Sequence< PropertyState > SwXFrame::getPropertyStates(
    const Sequence< OUString >& aPropertyNames )
        throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/*-- 12.09.00 14:04:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setPropertyToDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
            if( eType == FLYCNTTYPE_GRF &&
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
/*-- 12.09.00 14:04:55---------------------------------------------------

  -----------------------------------------------------------------------*/
Any SwXFrame::getPropertyDefault( const OUString& rPropertyName )
    throw(UnknownPropertyException, WrappedTargetException, RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/* -----------------22.04.99 14:59-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::addEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn())
        throw RuntimeException();
    aLstnrCntnr.AddListener(aListener);
}
/* -----------------22.04.99 14:59-------------------
 *
 * --------------------------------------------------*/
void SwXFrame::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    if(!GetRegisteredIn() || !aLstnrCntnr.RemoveListener(aListener))
        throw RuntimeException();
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
        mpDoc = 0;
        aLstnrCntnr.Disposing();
    }
}

/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::dispose(void) throw( RuntimeException )
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
/*-- 11.12.98 16:02:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXFrame::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
                pDoc->StartUndo(UNDO_INSERT);
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

                pDoc->EndUndo(UNDO_INSERT);
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
/* -----------------------------04.04.01 14:27--------------------------------

 ---------------------------------------------------------------------------*/
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
/*-- 22.04.99 08:03:20---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Point SwXFrame::getPosition(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    RuntimeException aRuntime;
    aRuntime.Message = C2U("position cannot be determined with this method");
    throw aRuntime;
    return awt::Point();
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setPosition(const awt::Point& aPosition) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    RuntimeException aRuntime;
    aRuntime.Message = C2U("position cannot be changed with this method");
    throw aRuntime;
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
awt::Size SwXFrame::getSize(void) throw( RuntimeException )
{
    uno::Any aVal = getPropertyValue(C2U("Size"));
    awt::Size* pRet =  (awt::Size*)aVal.getValue();
    return *pRet;
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXFrame::setSize(const awt::Size& aSize)
    throw( PropertyVetoException, RuntimeException )
{
    uno::Any aVal(&aSize, ::getCppuType((const awt::Size*)0));
    setPropertyValue(C2U("Size"), aVal);
}
/*-- 22.04.99 08:03:21---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXFrame::getShapeType(void) throw( RuntimeException )
{
    return C2U("FrameShape");
}


/******************************************************************
 *  SwXTextFrame
 ******************************************************************/
/*-- 14.01.99 11:27:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFrame::SwXTextFrame( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_FRM, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME), pDoc ),
    SwXText(0, CURSOR_FRAME)
{
}
/*-- 11.12.98 15:23:01---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextFrame::SwXTextFrame(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_FRM, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_FRAME)),
    SwXText(rFmt.GetDoc(), CURSOR_FRAME)
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
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXTextFrame::getImplementationId(  ) throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
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
uno::Reference< XText >  SwXTextFrame::getText(void) throw( RuntimeException )
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

uno::Reference< XTextCursor >   SwXTextFrame::createCursor() throw ( ::com::sun::star::uno::RuntimeException)
{
    return createTextCursor();
}
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextCursor >  SwXTextFrame::createTextCursor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
        SwUnoCrsr*  pUnoCrsr = pXCrsr->GetCrsr();
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS ))
//          throw  RuntimeException() );
    }
    else
        throw RuntimeException();
    return aRef;
}
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextCursor >  SwXTextFrame::createTextCursorByRange(const uno::Reference< XTextRange > & aTextPosition) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
/*-- 11.12.98 15:23:03---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< container::XEnumeration >  SwXTextFrame::createEnumeration(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Reference< container::XEnumeration >  aRef;
    SwFrmFmt* pFmt = GetFrmFmt();
 uno::XInterface* pRet = 0;
    if(pFmt)
    {
        SwPosition aPos(pFmt->GetCntnt().GetCntntIdx()->GetNode());
        SwUnoCrsr* pUnoCrsr = GetDoc()->CreateUnoCrsr(aPos, sal_False);
        pUnoCrsr->Move( fnMoveForward, fnGoNode );
//      // no Cursor in protected sections
//      SwCrsrSaveState aSave( *pUnoCrsr );
//      if(pUnoCrsr->IsInProtectTable( sal_True ) ||
//          pUnoCrsr->IsSelOvr( SELOVER_TOGGLE | SELOVER_CHANGEPOS ))
//          throw  RuntimeException() );
        aRef = new SwXParagraphEnumeration(this, pUnoCrsr, CURSOR_FRAME);
    }
    return aRef;
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type  SwXTextFrame::getElementType(void) throw( RuntimeException )
{
    return ::getCppuType((uno::Reference<XTextRange>*)0);
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXTextFrame::hasElements(void) throw( RuntimeException )
{
    return sal_True;
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::attach(const uno::Reference< XTextRange > & xTextRange)
    throw( IllegalArgumentException, RuntimeException )
{
    SwXFrame::attach(xTextRange);
}
/*-- 11.12.98 15:23:04---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextFrame::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXFrame::getAnchor();
}
/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::dispose(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXFrame::dispose();
}
/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::addEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}
/*-- 11.12.98 15:23:05---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextFrame::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextFrame::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextFrame");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextFrame::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.Text")||
            COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextFrame")||
                    SwXFrame::supportsService(rServiceName);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextFrame::getSupportedServiceNames(void) throw( RuntimeException )
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
    throw(RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}
/* -----------------------------10.01.01 13:27--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXTextFrame::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(RuntimeException)
{
    sal_Int64 nRet = SwXFrame::getSomething( rId );
    if( !nRet )
        nRet = SwXText::getSomething( rId );

    return nRet;
}
/* -----------------------------19.03.2002 16:43------------------------------

 ---------------------------------------------------------------------------*/
uno::Any SwXTextFrame::getPropertyValue(const OUString& rPropertyName)
    throw( UnknownPropertyException, WrappedTargetException, RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
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
 *  SwXTextGraphicObject
 ******************************************************************/
/*-- 14.01.99 11:27:51---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextGraphicObject::SwXTextGraphicObject( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_GRF, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC), pDoc)
{
}
/*-- 11.12.98 16:02:25---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextGraphicObject::SwXTextGraphicObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_GRF, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_TEXT_GRAPHIC))
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
uno::Any SAL_CALL SwXTextGraphicObject::queryInterface( const uno::Type& aType )
    throw(RuntimeException)
{
    uno::Any aRet = SwXFrame::queryInterface(aType);
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextGraphicObjectBaseClass::queryInterface(aType);
    return aRet;
}
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------15.03.00 16:30--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXTextGraphicObject::getImplementationId(  ) throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
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
void SwXTextGraphicObject::attach(const uno::Reference< XTextRange > & xTextRange) throw( IllegalArgumentException, RuntimeException )
{
    SwXFrame::attach(xTextRange);
}
/*-- 11.12.98 16:02:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextGraphicObject::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXFrame::getAnchor();
}
/*-- 11.12.98 16:02:28---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::dispose(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXFrame::dispose();
}
/*-- 11.12.98 16:02:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::addEventListener(const uno::Reference< XEventListener > & aListener)
                                                    throw( RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}
/*-- 11.12.98 16:02:29---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextGraphicObject::removeEventListener(const uno::Reference< XEventListener > & aListener)
                                                    throw( RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextGraphicObject::getImplementationName(void) throw( RuntimeException )
{
    return C2U("SwXTextGraphicObject");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextGraphicObject::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextGraphicObject") ||
                    SwXFrame::supportsService(rServiceName);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextGraphicObject::getSupportedServiceNames(void)
        throw( RuntimeException )
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
        throw(RuntimeException)
{
    return new SwFrameEventDescriptor( *this );
}

/******************************************************************
 *
 ******************************************************************/
/*-- 11.12.98 16:16:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextEmbeddedObject::SwXTextEmbeddedObject( SwDoc *pDoc ) :
    SwXFrame(FLYCNTTYPE_OLE, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_EMBEDDED_OBJECT), pDoc)
{
}
/*-- 11.12.98 16:16:53---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXTextEmbeddedObject::SwXTextEmbeddedObject(SwFrmFmt& rFmt) :
    SwXFrame(rFmt, FLYCNTTYPE_OLE, aSwMapProvider.GetPropertyMap(PROPERTY_MAP_EMBEDDED_OBJECT))
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
uno::Any SAL_CALL SwXTextEmbeddedObject::queryInterface( const uno::Type& aType )
    throw( RuntimeException)
{
    uno::Any aRet = SwXFrame::queryInterface(aType);;
    if(aRet.getValueType() == ::getCppuVoidType())
        aRet = SwXTextEmbeddedObjectBaseClass::queryInterface(aType);
    return aRet;
}
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
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
/* -----------------------------15.03.00 16:32--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< sal_Int8 > SAL_CALL SwXTextEmbeddedObject::getImplementationId(  ) throw(RuntimeException)
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    static Sequence< sal_Int8 > aId( 16 );
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
void SwXTextEmbeddedObject::attach(const uno::Reference< XTextRange > & xTextRange) throw( IllegalArgumentException, RuntimeException )
{
    SwXFrame::attach(xTextRange);
}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< XTextRange >  SwXTextEmbeddedObject::getAnchor(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    return SwXFrame::getAnchor();
}
/*-- 11.12.98 16:16:54---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::dispose(void) throw( RuntimeException )
{
    vos::OGuard aGuard(Application::GetSolarMutex());
    SwXFrame::dispose();
}
/*-- 11.12.98 16:16:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::addEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::addEventListener(aListener);
}
/*-- 11.12.98 16:16:55---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXTextEmbeddedObject::removeEventListener(const uno::Reference< XEventListener > & aListener) throw( RuntimeException )
{
    SwXFrame::removeEventListener(aListener);
}
/*-- 11.12.98 16:16:55---------------------------------------------------

  -----------------------------------------------------------------------*/

uno::Reference< XComponent >  SwXTextEmbeddedObject::getEmbeddedObject(void) throw( RuntimeException )
{
    uno::Reference< XComponent >  xRet;
    SwFrmFmt*   pFmt = GetFrmFmt();
    if(pFmt)
    {
        SwDoc* pDoc = pFmt->GetDoc();
        const SwFmtCntnt* pCnt = &pFmt->GetCntnt();
        DBG_ASSERT( pCnt->GetCntntIdx() &&
                       pDoc->GetNodes()[ pCnt->GetCntntIdx()->
                                        GetIndex() + 1 ]->GetOLENode(), "kein OLE-Node?")

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
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
OUString SwXTextEmbeddedObject::getImplementationName(void) throw( RuntimeException )

{
    return C2U("SwXTextEmbeddedObject");
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
sal_Bool SwXTextEmbeddedObject::supportsService(const OUString& rServiceName) throw( RuntimeException )
{
    return  COMPARE_EQUAL == rServiceName.compareToAscii("com.sun.star.text.TextEmbeddedObject")||
                    SwXFrame::supportsService(rServiceName);
}
/* -----------------03.05.99 12:28-------------------
 *
 * --------------------------------------------------*/
uno::Sequence< OUString > SwXTextEmbeddedObject::getSupportedServiceNames(void)
        throw( RuntimeException )
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
        throw(RuntimeException)
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

void SwXOLEListener::modified( const EventObject& rEvent )
                                        throw( RuntimeException )
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
        throw RuntimeException();
    if( !pNd->GetOLEObj().IsOleRef() ||
            !pNd->GetOLEObj().GetOleRef()->GetProtocol().IsInPlaceActive() )
    {
        // if the OLE-Node is UI-Active do nothing
        pNd->SetOLESizeInvalid( sal_True );
        pNd->GetDoc()->SetOLEObjModified();
    }
}
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
void SwXOLEListener::disposing( const EventObject& rEvent )
                        throw( RuntimeException )
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
    catch(Exception& rEx)
    {
        DBG_ERROR("OLE Listener couldn't be removed")
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

