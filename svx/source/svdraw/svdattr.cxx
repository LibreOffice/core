/*************************************************************************
 *
 *  $RCSfile: svdattr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-10-09 16:32:39 $
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

#define ITEMID_GRF_CROP 0

#ifndef _COM_SUN_STAR_DRAWING_TEXTFITTOSIZETYPE_HPP_
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTADJUST_HPP_
#include <com/sun/star/drawing/TextAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTANIMATIONKIND_HPP_
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_ANY_HXX_
#include <com/sun/star/uno/Any.hxx>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTANIMATIONDIRECTION_HPP_
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CONNECTORTYPE_HPP_
#include <com/sun/star/drawing/ConnectorType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_MEASUREKIND_HPP_
#include <com/sun/star/drawing/MeasureKind.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_MEASURETEXTHORZPOS_HPP_
#include <com/sun/star/drawing/MeasureTextHorzPos.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_MEASURETEXTVERTPOS_HPP_
#include <com/sun/star/drawing/MeasureTextVertPos.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_CIRCLEKIND_HPP_
#include <com/sun/star/drawing/CircleKind.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#include "svdattr.hxx"
#include "svdattrx.hxx"
#include "svdpool.hxx"

#ifndef _SOLAR_HRC
#include <svtools/solar.hrc>
#endif

#include "svxids.hrc"
#include "xtable.hxx"    // fuer RGB_Color()
#include "svditext.hxx"
#include "svdmodel.hxx"  // fuer DEGREE_CHAR
#include "svdtrans.hxx"
#include "svdglob.hxx"  // Stringcache
#include "svdstr.hrc"
#include "sdgcpitm.hxx"
#include "adjitem.hxx"

#ifndef _BIGINT_HXX //autogen
#include <tools/bigint.hxx>
#endif

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif

#include <xtxasit.hxx>
#include <xfillit0.hxx>
#include <xflclit.hxx>
#include <xlineit0.hxx>

using namespace ::rtl;
using namespace ::com::sun::star;

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

SdrItemPool::SdrItemPool(USHORT nAttrStart, USHORT nAttrEnd, FASTBOOL bLoadRefCounts):
    XOutdevItemPool(nAttrStart,nAttrEnd,bLoadRefCounts)
{
    Ctor(NULL,nAttrStart,nAttrEnd);
}

SdrItemPool::SdrItemPool(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd, FASTBOOL bLoadRefCounts):
    XOutdevItemPool(pMaster,nAttrStart,nAttrEnd,bLoadRefCounts)
{
    Ctor(pMaster,nAttrStart,nAttrEnd);
}

void SdrItemPool::Ctor(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd)
{
#ifdef SDRDEFITEMCACHE
    pDefLineAttr=NULL;
#endif SDRDEFITEMCACHE

    if (pMaster==NULL) {
        pMaster=this;
    }

    Color aNullCol(RGB_Color(COL_BLACK));
    XubString aEmptyStr;

    USHORT i;
    for (i=SDRATTR_NOTPERSIST_FIRST; i<=SDRATTR_NOTPERSIST_LAST; i++) {
        pItemInfos[i-SDRATTR_START]._nFlags=0;
    }
    // Schatten
    ppPoolDefaults[SDRATTR_SHADOW            -SDRATTR_START]=new SdrShadowItem;
    ppPoolDefaults[SDRATTR_SHADOWCOLOR       -SDRATTR_START]=new SdrShadowColorItem(aEmptyStr,aNullCol);
    ppPoolDefaults[SDRATTR_SHADOWXDIST       -SDRATTR_START]=new SdrShadowXDistItem;
    ppPoolDefaults[SDRATTR_SHADOWYDIST       -SDRATTR_START]=new SdrShadowYDistItem;
    ppPoolDefaults[SDRATTR_SHADOWTRANSPARENCE-SDRATTR_START]=new SdrShadowTransparenceItem;
    ppPoolDefaults[SDRATTR_SHADOW3D          -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOW3D    );
    ppPoolDefaults[SDRATTR_SHADOWPERSP       -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOWPERSP );
    for (i=SDRATTR_SHADOWRESERVE1; i<=SDRATTR_SHADOWRESERVE5; i++) {
        ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
    }

    ppPoolDefaults[SDRATTRSET_SHADOW-SDRATTR_START]=new SdrShadowSetItem(pMaster);

    // SID_ATTR_FILL_SHADOW = SID_SVX_START+299 = SID_LIB_START+299 = 10299
    pItemInfos[SDRATTR_SHADOW-SDRATTR_START]._nSID=SID_ATTR_FILL_SHADOW;

    // Legendenobjekt
    ppPoolDefaults[SDRATTR_CAPTIONTYPE      -SDRATTR_START]=new SdrCaptionTypeItem      ;
    ppPoolDefaults[SDRATTR_CAPTIONFIXEDANGLE-SDRATTR_START]=new SdrCaptionFixedAngleItem;
    ppPoolDefaults[SDRATTR_CAPTIONANGLE     -SDRATTR_START]=new SdrCaptionAngleItem     ;
    ppPoolDefaults[SDRATTR_CAPTIONGAP       -SDRATTR_START]=new SdrCaptionGapItem       ;
    ppPoolDefaults[SDRATTR_CAPTIONESCDIR    -SDRATTR_START]=new SdrCaptionEscDirItem    ;
    ppPoolDefaults[SDRATTR_CAPTIONESCISREL  -SDRATTR_START]=new SdrCaptionEscIsRelItem  ;
    ppPoolDefaults[SDRATTR_CAPTIONESCREL    -SDRATTR_START]=new SdrCaptionEscRelItem    ;
    ppPoolDefaults[SDRATTR_CAPTIONESCABS    -SDRATTR_START]=new SdrCaptionEscAbsItem    ;
    ppPoolDefaults[SDRATTR_CAPTIONLINELEN   -SDRATTR_START]=new SdrCaptionLineLenItem   ;
    ppPoolDefaults[SDRATTR_CAPTIONFITLINELEN-SDRATTR_START]=new SdrCaptionFitLineLenItem;
    for (i=SDRATTR_CAPTIONRESERVE1; i<=SDRATTR_CAPTIONRESERVE5; i++) {
        ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
    }
    ppPoolDefaults[SDRATTRSET_CAPTION-SDRATTR_START]=new SdrCaptionSetItem(pMaster);

    // Outliner-Attribute
    ppPoolDefaults[SDRATTRSET_OUTLINER-SDRATTR_START]=new SdrOutlinerSetItem(pMaster);

    // Misc-Attribute
    ppPoolDefaults[SDRATTR_ECKENRADIUS          -SDRATTR_START]=new SdrEckenradiusItem;
    ppPoolDefaults[SDRATTR_TEXT_MINFRAMEHEIGHT  -SDRATTR_START]=new SdrTextMinFrameHeightItem;
    ppPoolDefaults[SDRATTR_TEXT_AUTOGROWHEIGHT  -SDRATTR_START]=new SdrTextAutoGrowHeightItem;
    ppPoolDefaults[SDRATTR_TEXT_FITTOSIZE       -SDRATTR_START]=new SdrTextFitToSizeTypeItem;
    ppPoolDefaults[SDRATTR_TEXT_LEFTDIST        -SDRATTR_START]=new SdrTextLeftDistItem;
    ppPoolDefaults[SDRATTR_TEXT_RIGHTDIST       -SDRATTR_START]=new SdrTextRightDistItem;
    ppPoolDefaults[SDRATTR_TEXT_UPPERDIST       -SDRATTR_START]=new SdrTextUpperDistItem;
    ppPoolDefaults[SDRATTR_TEXT_LOWERDIST       -SDRATTR_START]=new SdrTextLowerDistItem;
    ppPoolDefaults[SDRATTR_TEXT_VERTADJUST      -SDRATTR_START]=new SdrTextVertAdjustItem;
    ppPoolDefaults[SDRATTR_TEXT_MAXFRAMEHEIGHT  -SDRATTR_START]=new SdrTextMaxFrameHeightItem;
    ppPoolDefaults[SDRATTR_TEXT_MINFRAMEWIDTH   -SDRATTR_START]=new SdrTextMinFrameWidthItem;
    ppPoolDefaults[SDRATTR_TEXT_MAXFRAMEWIDTH   -SDRATTR_START]=new SdrTextMaxFrameWidthItem;
    ppPoolDefaults[SDRATTR_TEXT_AUTOGROWWIDTH   -SDRATTR_START]=new SdrTextAutoGrowWidthItem;
    ppPoolDefaults[SDRATTR_TEXT_HORZADJUST      -SDRATTR_START]=new SdrTextHorzAdjustItem;
    ppPoolDefaults[SDRATTR_TEXT_ANIKIND         -SDRATTR_START]=new SdrTextAniKindItem;
    ppPoolDefaults[SDRATTR_TEXT_ANIDIRECTION    -SDRATTR_START]=new SdrTextAniDirectionItem;
    ppPoolDefaults[SDRATTR_TEXT_ANISTARTINSIDE  -SDRATTR_START]=new SdrTextAniStartInsideItem;
    ppPoolDefaults[SDRATTR_TEXT_ANISTOPINSIDE   -SDRATTR_START]=new SdrTextAniStopInsideItem;
    ppPoolDefaults[SDRATTR_TEXT_ANICOUNT        -SDRATTR_START]=new SdrTextAniCountItem;
    ppPoolDefaults[SDRATTR_TEXT_ANIDELAY        -SDRATTR_START]=new SdrTextAniDelayItem;
    ppPoolDefaults[SDRATTR_TEXT_ANIAMOUNT       -SDRATTR_START]=new SdrTextAniAmountItem;
    ppPoolDefaults[SDRATTR_TEXT_CONTOURFRAME    -SDRATTR_START]=new SdrTextContourFrameItem;
    ppPoolDefaults[SDRATTR_AUTOSHAPE_ADJUSTMENT -SDRATTR_START]=new SdrAutoShapeAdjustmentItem;
    for (i=SDRATTR_RESERVE14; i<=SDRATTR_RESERVE19; i++) {
        ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
    }
    ppPoolDefaults[SDRATTRSET_MISC-SDRATTR_START]=new SdrMiscSetItem(pMaster);

    pItemInfos[SDRATTR_TEXT_FITTOSIZE-SDRATTR_START]._nSID=SID_ATTR_TEXT_FITTOSIZE;

    // Objektverbinder
    ppPoolDefaults[SDRATTR_EDGEKIND         -SDRATTR_START]=new SdrEdgeKindItem;
    long nDefEdgeDist=500; // Erstmal hart defaulted fuer Draw (100TH_MM). hier muss noch der MapMode beruecksichtigt werden.
    ppPoolDefaults[SDRATTR_EDGENODE1HORZDIST-SDRATTR_START]=new SdrEdgeNode1HorzDistItem(nDefEdgeDist);
    ppPoolDefaults[SDRATTR_EDGENODE1VERTDIST-SDRATTR_START]=new SdrEdgeNode1VertDistItem(nDefEdgeDist);
    ppPoolDefaults[SDRATTR_EDGENODE2HORZDIST-SDRATTR_START]=new SdrEdgeNode2HorzDistItem(nDefEdgeDist);
    ppPoolDefaults[SDRATTR_EDGENODE2VERTDIST-SDRATTR_START]=new SdrEdgeNode2VertDistItem(nDefEdgeDist);
    ppPoolDefaults[SDRATTR_EDGENODE1GLUEDIST-SDRATTR_START]=new SdrEdgeNode1GlueDistItem;
    ppPoolDefaults[SDRATTR_EDGENODE2GLUEDIST-SDRATTR_START]=new SdrEdgeNode2GlueDistItem;
    ppPoolDefaults[SDRATTR_EDGELINEDELTAANZ -SDRATTR_START]=new SdrEdgeLineDeltaAnzItem;
    ppPoolDefaults[SDRATTR_EDGELINE1DELTA   -SDRATTR_START]=new SdrEdgeLine1DeltaItem;
    ppPoolDefaults[SDRATTR_EDGELINE2DELTA   -SDRATTR_START]=new SdrEdgeLine2DeltaItem;
    ppPoolDefaults[SDRATTR_EDGELINE3DELTA   -SDRATTR_START]=new SdrEdgeLine3DeltaItem;
    for (i=SDRATTR_EDGERESERVE02; i<=SDRATTR_EDGERESERVE09; i++) {
        ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
    }
    ppPoolDefaults[SDRATTRSET_EDGE-SDRATTR_START]=new SdrEdgeSetItem(pMaster);

     // Bemassungsobjekt
    ppPoolDefaults[SDRATTR_MEASUREKIND             -SDRATTR_START]=new SdrMeasureKindItem;
    ppPoolDefaults[SDRATTR_MEASURETEXTHPOS         -SDRATTR_START]=new SdrMeasureTextHPosItem;
    ppPoolDefaults[SDRATTR_MEASURETEXTVPOS         -SDRATTR_START]=new SdrMeasureTextVPosItem;
    ppPoolDefaults[SDRATTR_MEASURELINEDIST         -SDRATTR_START]=new SdrMeasureLineDistItem(800);
    ppPoolDefaults[SDRATTR_MEASUREHELPLINEOVERHANG -SDRATTR_START]=new SdrMeasureHelplineOverhangItem(200);
    ppPoolDefaults[SDRATTR_MEASUREHELPLINEDIST     -SDRATTR_START]=new SdrMeasureHelplineDistItem(100);
    ppPoolDefaults[SDRATTR_MEASUREHELPLINE1LEN     -SDRATTR_START]=new SdrMeasureHelpline1LenItem;
    ppPoolDefaults[SDRATTR_MEASUREHELPLINE2LEN     -SDRATTR_START]=new SdrMeasureHelpline2LenItem;
    ppPoolDefaults[SDRATTR_MEASUREBELOWREFEDGE     -SDRATTR_START]=new SdrMeasureBelowRefEdgeItem;
    ppPoolDefaults[SDRATTR_MEASURETEXTROTA90       -SDRATTR_START]=new SdrMeasureTextRota90Item;
    ppPoolDefaults[SDRATTR_MEASURETEXTUPSIDEDOWN   -SDRATTR_START]=new SdrMeasureTextUpsideDownItem;
    ppPoolDefaults[SDRATTR_MEASUREOVERHANG         -SDRATTR_START]=new SdrMeasureOverhangItem(600);
    ppPoolDefaults[SDRATTR_MEASUREUNIT             -SDRATTR_START]=new SdrMeasureUnitItem;
    ppPoolDefaults[SDRATTR_MEASURESCALE            -SDRATTR_START]=new SdrMeasureScaleItem;
    ppPoolDefaults[SDRATTR_MEASURESHOWUNIT         -SDRATTR_START]=new SdrMeasureShowUnitItem;
    ppPoolDefaults[SDRATTR_MEASUREFORMATSTRING     -SDRATTR_START]=new SdrMeasureFormatStringItem();
    ppPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLE    -SDRATTR_START]=new SdrMeasureTextAutoAngleItem();
    ppPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLEVIEW-SDRATTR_START]=new SdrMeasureTextAutoAngleViewItem();
    ppPoolDefaults[SDRATTR_MEASURETEXTISFIXEDANGLE -SDRATTR_START]=new SdrMeasureTextIsFixedAngleItem();
    ppPoolDefaults[SDRATTR_MEASURETEXTFIXEDANGLE   -SDRATTR_START]=new SdrMeasureTextFixedAngleItem();
    for (i=SDRATTR_MEASURERESERVE04; i<=SDRATTR_MEASURERESERVE07; i++) {
        ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
    }
    ppPoolDefaults[SDRATTRSET_MEASURE-SDRATTR_START]=new SdrMeasureSetItem(pMaster);

     // Kreis
    ppPoolDefaults[SDRATTR_CIRCKIND      -SDRATTR_START]=new SdrCircKindItem;
    ppPoolDefaults[SDRATTR_CIRCSTARTANGLE-SDRATTR_START]=new SdrCircStartAngleItem;
    ppPoolDefaults[SDRATTR_CIRCENDANGLE  -SDRATTR_START]=new SdrCircEndAngleItem;
    for (i=SDRATTR_CIRCRESERVE0; i<=SDRATTR_CIRCRESERVE3; i++) {
        ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
    }
    ppPoolDefaults[SDRATTRSET_CIRC-SDRATTR_START]=new SdrCircSetItem(pMaster);

     // Nichtpersistente-Items
    ppPoolDefaults[SDRATTR_OBJMOVEPROTECT -SDRATTR_START]=new SdrObjMoveProtectItem;
    ppPoolDefaults[SDRATTR_OBJSIZEPROTECT -SDRATTR_START]=new SdrObjSizeProtectItem;
    ppPoolDefaults[SDRATTR_OBJPRINTABLE   -SDRATTR_START]=new SdrObjPrintableItem;
    ppPoolDefaults[SDRATTR_LAYERID        -SDRATTR_START]=new SdrLayerIdItem;
    ppPoolDefaults[SDRATTR_LAYERNAME      -SDRATTR_START]=new SdrLayerNameItem;
    ppPoolDefaults[SDRATTR_OBJECTNAME     -SDRATTR_START]=new SdrObjectNameItem;
    ppPoolDefaults[SDRATTR_ALLPOSITIONX   -SDRATTR_START]=new SdrAllPositionXItem;
    ppPoolDefaults[SDRATTR_ALLPOSITIONY   -SDRATTR_START]=new SdrAllPositionYItem;
    ppPoolDefaults[SDRATTR_ALLSIZEWIDTH   -SDRATTR_START]=new SdrAllSizeWidthItem;
    ppPoolDefaults[SDRATTR_ALLSIZEHEIGHT  -SDRATTR_START]=new SdrAllSizeHeightItem;
    ppPoolDefaults[SDRATTR_ONEPOSITIONX   -SDRATTR_START]=new SdrOnePositionXItem;
    ppPoolDefaults[SDRATTR_ONEPOSITIONY   -SDRATTR_START]=new SdrOnePositionYItem;
    ppPoolDefaults[SDRATTR_ONESIZEWIDTH   -SDRATTR_START]=new SdrOneSizeWidthItem;
    ppPoolDefaults[SDRATTR_ONESIZEHEIGHT  -SDRATTR_START]=new SdrOneSizeHeightItem;
    ppPoolDefaults[SDRATTR_LOGICSIZEWIDTH -SDRATTR_START]=new SdrLogicSizeWidthItem;
    ppPoolDefaults[SDRATTR_LOGICSIZEHEIGHT-SDRATTR_START]=new SdrLogicSizeHeightItem;
    ppPoolDefaults[SDRATTR_ROTATEANGLE    -SDRATTR_START]=new SdrRotateAngleItem;
    ppPoolDefaults[SDRATTR_SHEARANGLE     -SDRATTR_START]=new SdrShearAngleItem;
    ppPoolDefaults[SDRATTR_MOVEX          -SDRATTR_START]=new SdrMoveXItem;
    ppPoolDefaults[SDRATTR_MOVEY          -SDRATTR_START]=new SdrMoveYItem;
    ppPoolDefaults[SDRATTR_RESIZEXONE     -SDRATTR_START]=new SdrResizeXOneItem;
    ppPoolDefaults[SDRATTR_RESIZEYONE     -SDRATTR_START]=new SdrResizeYOneItem;
    ppPoolDefaults[SDRATTR_ROTATEONE      -SDRATTR_START]=new SdrRotateOneItem;
    ppPoolDefaults[SDRATTR_HORZSHEARONE   -SDRATTR_START]=new SdrHorzShearOneItem;
    ppPoolDefaults[SDRATTR_VERTSHEARONE   -SDRATTR_START]=new SdrVertShearOneItem;
    ppPoolDefaults[SDRATTR_RESIZEXALL     -SDRATTR_START]=new SdrResizeXAllItem;
    ppPoolDefaults[SDRATTR_RESIZEYALL     -SDRATTR_START]=new SdrResizeYAllItem;
    ppPoolDefaults[SDRATTR_ROTATEALL      -SDRATTR_START]=new SdrRotateAllItem;
    ppPoolDefaults[SDRATTR_HORZSHEARALL   -SDRATTR_START]=new SdrHorzShearAllItem;
    ppPoolDefaults[SDRATTR_VERTSHEARALL   -SDRATTR_START]=new SdrVertShearAllItem;
    ppPoolDefaults[SDRATTR_TRANSFORMREF1X -SDRATTR_START]=new SdrTransformRef1XItem;
    ppPoolDefaults[SDRATTR_TRANSFORMREF1Y -SDRATTR_START]=new SdrTransformRef1YItem;
    ppPoolDefaults[SDRATTR_TRANSFORMREF2X -SDRATTR_START]=new SdrTransformRef2XItem;
    ppPoolDefaults[SDRATTR_TRANSFORMREF2Y -SDRATTR_START]=new SdrTransformRef2YItem;
    for (i=SDRATTR_NOTPERSISTRESERVE1; i<=SDRATTR_NOTPERSISTRESERVE15; i++) {
        ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
    }

    // Grafik
    ppPoolDefaults[ SDRATTR_GRAFRED             - SDRATTR_START] = new SdrGrafRedItem;
    ppPoolDefaults[ SDRATTR_GRAFGREEN           - SDRATTR_START] = new SdrGrafGreenItem;
    ppPoolDefaults[ SDRATTR_GRAFBLUE            - SDRATTR_START] = new SdrGrafBlueItem;
    ppPoolDefaults[ SDRATTR_GRAFLUMINANCE       - SDRATTR_START] = new SdrGrafLuminanceItem;
    ppPoolDefaults[ SDRATTR_GRAFCONTRAST        - SDRATTR_START] = new SdrGrafContrastItem;
    ppPoolDefaults[ SDRATTR_GRAFGAMMA           - SDRATTR_START] = new SdrGrafGamma100Item;
    ppPoolDefaults[ SDRATTR_GRAFTRANSPARENCE    - SDRATTR_START] = new SdrGrafTransparenceItem;
    ppPoolDefaults[ SDRATTR_GRAFINVERT          - SDRATTR_START] = new SdrGrafInvertItem;
    ppPoolDefaults[ SDRATTR_GRAFMODE            - SDRATTR_START] = new SdrGrafModeItem;
    ppPoolDefaults[ SDRATTR_GRAFCROP            - SDRATTR_START] = new SdrGrafCropItem;
    for( i = SDRATTR_GRAFRESERVE3; i <= SDRATTR_GRAFRESERVE6; i++ )
        ppPoolDefaults[ i - SDRATTR_START ] = new SfxVoidItem( i );
    ppPoolDefaults[ SDRATTRSET_GRAF - SDRATTR_START ] = new SdrGrafSetItem( pMaster );

    ////////////////////////////////
#ifdef DBG_UTIL
    UINT16 nAnz(SDRATTR_END-SDRATTR_START + 1);

    for(UINT16 nNum = 0; nNum < nAnz; nNum++)
    {
        const SfxPoolItem* pItem = ppPoolDefaults[nNum];

        if(!pItem)
        {
            ByteString aStr("PoolDefaultItem not set: ");

            aStr += "Num=";
            aStr += nNum;
            aStr += "Which=";
            aStr += nNum + 1000;

            DBG_ERROR(aStr.GetBuffer());
        }
        else if(pItem->Which() != nNum + 1000)
        {
            ByteString aStr("PoolDefaultItem has wrong WhichId: ");

            aStr += "Num=";
            aStr += ByteString::CreateFromInt32( nNum );
            aStr += " Which=";
            aStr += ByteString::CreateFromInt32( pItem->Which() );

            DBG_ERROR(aStr.GetBuffer());
        }
    }
#endif

    if (nAttrStart==SDRATTR_START && nAttrEnd==SDRATTR_END) {
        SetDefaults(ppPoolDefaults);
        SetItemInfos(pItemInfos);
    }
}

void SdrItemPool::ImpMakeDefItems()
{
#ifdef SDRDEFITEMCACHE
    if (pDefLineAttr==NULL) {
        XLineAttrSetItem aLineSetItem(this);
        pDefLineAttr=(XLineAttrSetItem*)&Put(aLineSetItem);
        aLineSetItem.GetItemSet().Put(XLineStyleItem(XLINE_NONE));
        pDefTextFrameLineAttr=(XLineAttrSetItem*)&Put(aLineSetItem);

        XFillAttrSetItem aFillSetItem(this);
        pDefFillAttr=(XFillAttrSetItem*)&Put(aFillSetItem);
        aFillSetItem.GetItemSet().Put(XFillColorItem(String(),Color(COL_WHITE))); // Falls einer auf Solid umschaltet
        aFillSetItem.GetItemSet().Put(XFillStyleItem(XFILL_NONE));
        pDefTextFrameFillAttr=(XFillAttrSetItem*)&Put(aFillSetItem);

        XTextAttrSetItem aTextSetItem(this);
        pDefTextAttr=(XTextAttrSetItem*)&Put(aTextSetItem);
        pDefTextFrameTextAttr=pDefTextAttr;

        SdrShadowSetItem aShadSetItem(this);
        pDefShadAttr=(SdrShadowSetItem*)&Put(aShadSetItem);
        pDefTextFrameShadAttr=pDefShadAttr;

        SdrOutlinerSetItem aOutlSetItem(this);
        pDefTextFrameOutlAttr=(SdrOutlinerSetItem*)&Put(aOutlSetItem);
        aOutlSetItem.GetItemSet().Put(SvxAdjustItem(SVX_ADJUST_CENTER));
        pDefOutlAttr=(SdrOutlinerSetItem*)&Put(aOutlSetItem);

        SdrMiscSetItem aMiscSetItem(this);
        pDefTextFrameMiscAttr=(SdrMiscSetItem*)&Put(aMiscSetItem);
        aMiscSetItem.GetItemSet().Put(SdrTextHorzAdjustItem(SDRTEXTHORZADJUST_CENTER));
        aMiscSetItem.GetItemSet().Put(SdrTextVertAdjustItem(SDRTEXTVERTADJUST_CENTER));
        pDefMiscAttr=(SdrMiscSetItem*)&Put(aMiscSetItem);
    }
#endif SDRDEFITEMCACHE
}

/*************************************************************************
|*
|* copy ctor, sorgt dafuer, dass die static defaults gecloned werden
|*            (Parameter 2 = TRUE)
|*
\************************************************************************/

SdrItemPool::SdrItemPool(const SdrItemPool& rPool):
    XOutdevItemPool(rPool)
{
}

/*************************************************************************
|*
|* Clone()
|*
\************************************************************************/

SfxItemPool* __EXPORT SdrItemPool::Clone() const
{
    return new SdrItemPool(*this);
}

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

SdrItemPool::~SdrItemPool()
{
    Delete(); // erstmal den 'dtor' des SfxItemPools rufen
    // und nun meine eigenen statischen Defaults abraeumen
    if (ppPoolDefaults!=NULL) {
        unsigned nBeg=SDRATTR_SHADOW-SDRATTR_START;
        unsigned nEnd=SDRATTR_END-SDRATTR_START;
        for (unsigned i=nBeg; i<=nEnd; i++) {
            SetRef(*ppPoolDefaults[i],0);
            delete ppPoolDefaults[i];
            ppPoolDefaults[i]=NULL;
        }
    }
    // Vor dem zerstoeren die Pools ggf. voneinander trennen
    SetSecondaryPool(NULL);
}

SfxItemPresentation __EXPORT SdrItemPool::GetPresentation(
              const SfxPoolItem& rItem, SfxItemPresentation ePresentation,
              SfxMapUnit ePresentationMetric, XubString& rText,
              const International * pInternational) const
{
    if (!IsInvalidItem(&rItem)) {
        USHORT nWhich=rItem.Which();
        if (nWhich>=SDRATTR_SHADOW_FIRST && nWhich<=SDRATTR_END) {
            rItem.GetPresentation(SFX_ITEM_PRESENTATION_NAMELESS,
                        GetMetric(nWhich),ePresentationMetric,rText,
                        pInternational);
            String aStr;

            TakeItemName(nWhich, aStr);
            aStr += sal_Unicode(' ');
            rText.Insert(aStr, 0);

            return ePresentation;
        }
    }
    return XOutdevItemPool::GetPresentation(rItem,ePresentation,ePresentationMetric,rText,pInternational);
}

FASTBOOL SdrItemPool::TakeItemName(USHORT nWhich, String& rItemName)
{
    ResMgr* pResMgr = ImpGetResMgr();
    USHORT  nResId = SIP_UNKNOWN_ATTR;

    switch (nWhich)
    {
        case XATTR_LINESTYLE        : nResId = SIP_XA_LINESTYLE;break;
        case XATTR_LINEDASH         : nResId = SIP_XA_LINEDASH;break;
        case XATTR_LINEWIDTH        : nResId = SIP_XA_LINEWIDTH;break;
        case XATTR_LINECOLOR        : nResId = SIP_XA_LINECOLOR;break;
        case XATTR_LINESTART        : nResId = SIP_XA_LINESTART;break;
        case XATTR_LINEEND          : nResId = SIP_XA_LINEEND;break;
        case XATTR_LINESTARTWIDTH   : nResId = SIP_XA_LINESTARTWIDTH;break;
        case XATTR_LINEENDWIDTH     : nResId = SIP_XA_LINEENDWIDTH;break;
        case XATTR_LINESTARTCENTER  : nResId = SIP_XA_LINESTARTCENTER;break;
        case XATTR_LINEENDCENTER    : nResId = SIP_XA_LINEENDCENTER;break;
        case XATTR_LINETRANSPARENCE : nResId = SIP_XA_LINETRANSPARENCE;break;
        case XATTR_LINEJOINT        : nResId = SIP_XA_LINEJOINT;break;
        case XATTR_LINERESERVED2    : nResId = SIP_XA_LINERESERVED2;break;
        case XATTR_LINERESERVED3    : nResId = SIP_XA_LINERESERVED3;break;
        case XATTR_LINERESERVED4    : nResId = SIP_XA_LINERESERVED4;break;
        case XATTR_LINERESERVED5    : nResId = SIP_XA_LINERESERVED5;break;
        case XATTR_LINERESERVED_LAST: nResId = SIP_XA_LINERESERVED_LAST;break;
        case XATTRSET_LINE          : nResId = SIP_XATTRSET_LINE;break;

        case XATTR_FILLSTYLE            : nResId = SIP_XA_FILLSTYLE;break;
        case XATTR_FILLCOLOR            : nResId = SIP_XA_FILLCOLOR;break;
        case XATTR_FILLGRADIENT         : nResId = SIP_XA_FILLGRADIENT;break;
        case XATTR_FILLHATCH            : nResId = SIP_XA_FILLHATCH;break;
        case XATTR_FILLBITMAP           : nResId = SIP_XA_FILLBITMAP;break;
        case XATTR_FILLTRANSPARENCE     : nResId = SIP_XA_FILLTRANSPARENCE;break;
        case XATTR_GRADIENTSTEPCOUNT    : nResId = SIP_XA_GRADIENTSTEPCOUNT;break;
        case XATTR_FILLBMP_TILE         : nResId = SIP_XA_FILLBMP_TILE;break;
        case XATTR_FILLBMP_POS          : nResId = SIP_XA_FILLBMP_POS;break;
        case XATTR_FILLBMP_SIZEX        : nResId = SIP_XA_FILLBMP_SIZEX;break;
        case XATTR_FILLBMP_SIZEY        : nResId = SIP_XA_FILLBMP_SIZEY;break;
        case XATTR_FILLFLOATTRANSPARENCE: nResId = SIP_XA_FILLFLOATTRANSPARENCE;break;
        case XATTR_FILLRESERVED2        : nResId = SIP_XA_FILLRESERVED2;break;
        case XATTR_FILLBMP_SIZELOG      : nResId = SIP_XA_FILLBMP_SIZELOG;break;
        case XATTR_FILLBMP_TILEOFFSETX  : nResId = SIP_XA_FILLBMP_TILEOFFSETX;break;
        case XATTR_FILLBMP_TILEOFFSETY  : nResId = SIP_XA_FILLBMP_TILEOFFSETY;break;
        case XATTR_FILLBMP_STRETCH      : nResId = SIP_XA_FILLBMP_STRETCH;break;
        case XATTR_FILLRESERVED3        : nResId = SIP_XA_FILLRESERVED3;break;
        case XATTR_FILLRESERVED4        : nResId = SIP_XA_FILLRESERVED4;break;
        case XATTR_FILLRESERVED5        : nResId = SIP_XA_FILLRESERVED5;break;
        case XATTR_FILLRESERVED6        : nResId = SIP_XA_FILLRESERVED6;break;
        case XATTR_FILLRESERVED7        : nResId = SIP_XA_FILLRESERVED7;break;
        case XATTR_FILLRESERVED8        : nResId = SIP_XA_FILLRESERVED8;break;
        case XATTR_FILLBMP_POSOFFSETX   : nResId = SIP_XA_FILLBMP_POSOFFSETX;break;
        case XATTR_FILLBMP_POSOFFSETY   : nResId = SIP_XA_FILLBMP_POSOFFSETY;break;
        case XATTR_FILLBACKGROUND       : nResId = SIP_XA_FILLBACKGROUND;break;
        case XATTR_FILLRESERVED10       : nResId = SIP_XA_FILLRESERVED10;break;
        case XATTR_FILLRESERVED11       : nResId = SIP_XA_FILLRESERVED11;break;
        case XATTR_FILLRESERVED_LAST    : nResId = SIP_XA_FILLRESERVED_LAST;break;

        case XATTRSET_FILL             : nResId = SIP_XATTRSET_FILL;break;

        case XATTR_FORMTXTSTYLE     : nResId = SIP_XA_FORMTXTSTYLE;break;
        case XATTR_FORMTXTADJUST    : nResId = SIP_XA_FORMTXTADJUST;break;
        case XATTR_FORMTXTDISTANCE  : nResId = SIP_XA_FORMTXTDISTANCE;break;
        case XATTR_FORMTXTSTART     : nResId = SIP_XA_FORMTXTSTART;break;
        case XATTR_FORMTXTMIRROR    : nResId = SIP_XA_FORMTXTMIRROR;break;
        case XATTR_FORMTXTOUTLINE   : nResId = SIP_XA_FORMTXTOUTLINE;break;
        case XATTR_FORMTXTSHADOW    : nResId = SIP_XA_FORMTXTSHADOW;break;
        case XATTR_FORMTXTSHDWCOLOR : nResId = SIP_XA_FORMTXTSHDWCOLOR;break;
        case XATTR_FORMTXTSHDWXVAL  : nResId = SIP_XA_FORMTXTSHDWXVAL;break;
        case XATTR_FORMTXTSHDWYVAL  : nResId = SIP_XA_FORMTXTSHDWYVAL;break;
        case XATTR_FORMTXTSTDFORM   : nResId = SIP_XA_FORMTXTSTDFORM;break;
        case XATTR_FORMTXTHIDEFORM  : nResId = SIP_XA_FORMTXTHIDEFORM;break;
        case XATTR_FORMTXTSHDWTRANSP: nResId = SIP_XA_FORMTXTSHDWTRANSP;break;
        case XATTR_FTRESERVED2      : nResId = SIP_XA_FTRESERVED2;break;
        case XATTR_FTRESERVED3      : nResId = SIP_XA_FTRESERVED3;break;
        case XATTR_FTRESERVED4      : nResId = SIP_XA_FTRESERVED4;break;
        case XATTR_FTRESERVED5      : nResId = SIP_XA_FTRESERVED5;break;
        case XATTR_FTRESERVED_LAST  : nResId = SIP_XA_FTRESERVED_LAST;break;
        case XATTRSET_TEXT          : nResId = SIP_XATTRSET_TEXT;break;

        case SDRATTR_SHADOW            : nResId = SIP_SA_SHADOW;break;
        case SDRATTR_SHADOWCOLOR       : nResId = SIP_SA_SHADOWCOLOR;break;
        case SDRATTR_SHADOWXDIST       : nResId = SIP_SA_SHADOWXDIST;break;
        case SDRATTR_SHADOWYDIST       : nResId = SIP_SA_SHADOWYDIST;break;
        case SDRATTR_SHADOWTRANSPARENCE: nResId = SIP_SA_SHADOWTRANSPARENCE;break;
        case SDRATTR_SHADOW3D          : nResId = SIP_SA_SHADOW3D;break;
        case SDRATTR_SHADOWPERSP       : nResId = SIP_SA_SHADOWPERSP;break;
        case SDRATTR_SHADOWRESERVE1    : nResId = SIP_SA_SHADOWRESERVE1;break;
        case SDRATTR_SHADOWRESERVE2    : nResId = SIP_SA_SHADOWRESERVE2;break;
        case SDRATTR_SHADOWRESERVE3    : nResId = SIP_SA_SHADOWRESERVE3;break;
        case SDRATTR_SHADOWRESERVE4    : nResId = SIP_SA_SHADOWRESERVE4;break;
        case SDRATTR_SHADOWRESERVE5    : nResId = SIP_SA_SHADOWRESERVE5;break;
        case SDRATTRSET_SHADOW         : nResId = SIP_SDRATTRSET_SHADOW;break;

        case SDRATTR_CAPTIONTYPE      : nResId = SIP_SA_CAPTIONTYPE;break;
        case SDRATTR_CAPTIONFIXEDANGLE: nResId = SIP_SA_CAPTIONFIXEDANGLE;break;
        case SDRATTR_CAPTIONANGLE     : nResId = SIP_SA_CAPTIONANGLE;break;
        case SDRATTR_CAPTIONGAP       : nResId = SIP_SA_CAPTIONGAP;break;
        case SDRATTR_CAPTIONESCDIR    : nResId = SIP_SA_CAPTIONESCDIR;break;
        case SDRATTR_CAPTIONESCISREL  : nResId = SIP_SA_CAPTIONESCISREL;break;
        case SDRATTR_CAPTIONESCREL    : nResId = SIP_SA_CAPTIONESCREL;break;
        case SDRATTR_CAPTIONESCABS    : nResId = SIP_SA_CAPTIONESCABS;break;
        case SDRATTR_CAPTIONLINELEN   : nResId = SIP_SA_CAPTIONLINELEN;break;
        case SDRATTR_CAPTIONFITLINELEN: nResId = SIP_SA_CAPTIONFITLINELEN;break;
        case SDRATTR_CAPTIONRESERVE1  : nResId = SIP_SA_CAPTIONRESERVE1;break;
        case SDRATTR_CAPTIONRESERVE2  : nResId = SIP_SA_CAPTIONRESERVE2;break;
        case SDRATTR_CAPTIONRESERVE3  : nResId = SIP_SA_CAPTIONRESERVE3;break;
        case SDRATTR_CAPTIONRESERVE4  : nResId = SIP_SA_CAPTIONRESERVE4;break;
        case SDRATTR_CAPTIONRESERVE5  : nResId = SIP_SA_CAPTIONRESERVE5;break;
        case SDRATTRSET_CAPTION       : nResId = SIP_SDRATTRSET_CAPTION;break;

        case SDRATTRSET_OUTLINER: nResId = SIP_SDRATTRSET_OUTLINER;break;

        case SDRATTR_ECKENRADIUS            : nResId = SIP_SA_ECKENRADIUS;break;
        case SDRATTR_TEXT_MINFRAMEHEIGHT    : nResId = SIP_SA_TEXT_MINFRAMEHEIGHT;break;
        case SDRATTR_TEXT_AUTOGROWHEIGHT    : nResId = SIP_SA_TEXT_AUTOGROWHEIGHT;break;
        case SDRATTR_TEXT_FITTOSIZE         : nResId = SIP_SA_TEXT_FITTOSIZE;break;
        case SDRATTR_TEXT_LEFTDIST          : nResId = SIP_SA_TEXT_LEFTDIST;break;
        case SDRATTR_TEXT_RIGHTDIST         : nResId = SIP_SA_TEXT_RIGHTDIST;break;
        case SDRATTR_TEXT_UPPERDIST         : nResId = SIP_SA_TEXT_UPPERDIST;break;
        case SDRATTR_TEXT_LOWERDIST         : nResId = SIP_SA_TEXT_LOWERDIST;break;
        case SDRATTR_TEXT_VERTADJUST        : nResId = SIP_SA_TEXT_VERTADJUST;break;
        case SDRATTR_TEXT_MAXFRAMEHEIGHT    : nResId = SIP_SA_TEXT_MAXFRAMEHEIGHT;break;
        case SDRATTR_TEXT_MINFRAMEWIDTH     : nResId = SIP_SA_TEXT_MINFRAMEWIDTH;break;
        case SDRATTR_TEXT_MAXFRAMEWIDTH     : nResId = SIP_SA_TEXT_MAXFRAMEWIDTH;break;
        case SDRATTR_TEXT_AUTOGROWWIDTH     : nResId = SIP_SA_TEXT_AUTOGROWWIDTH;break;
        case SDRATTR_TEXT_HORZADJUST        : nResId = SIP_SA_TEXT_HORZADJUST;break;
        case SDRATTR_TEXT_ANIKIND           : nResId = SIP_SA_TEXT_ANIKIND;break;
        case SDRATTR_TEXT_ANIDIRECTION      : nResId = SIP_SA_TEXT_ANIDIRECTION;break;
        case SDRATTR_TEXT_ANISTARTINSIDE    : nResId = SIP_SA_TEXT_ANISTARTINSIDE;break;
        case SDRATTR_TEXT_ANISTOPINSIDE     : nResId = SIP_SA_TEXT_ANISTOPINSIDE;break;
        case SDRATTR_TEXT_ANICOUNT          : nResId = SIP_SA_TEXT_ANICOUNT;break;
        case SDRATTR_TEXT_ANIDELAY          : nResId = SIP_SA_TEXT_ANIDELAY;break;
        case SDRATTR_TEXT_ANIAMOUNT         : nResId = SIP_SA_TEXT_ANIAMOUNT;break;
        case SDRATTR_TEXT_CONTOURFRAME      : nResId = SIP_SA_TEXT_CONTOURFRAME;break;
        case SDRATTR_AUTOSHAPE_ADJUSTMENT   : nResId = SIP_SA_AUTOSHAPE_ADJUSTMENT;break;
        case SDRATTR_RESERVE14              : nResId = SIP_SA_RESERVE14;break;
        case SDRATTR_RESERVE15              : nResId = SIP_SA_RESERVE15;break;
        case SDRATTR_RESERVE16              : nResId = SIP_SA_RESERVE16;break;
        case SDRATTR_RESERVE17              : nResId = SIP_SA_RESERVE17;break;
        case SDRATTR_RESERVE18              : nResId = SIP_SA_RESERVE18;break;
        case SDRATTR_RESERVE19              : nResId = SIP_SA_RESERVE19;break;
        case SDRATTRSET_MISC                : nResId = SIP_SDRATTRSET_MISC;break;

        case SDRATTR_EDGEKIND           : nResId = SIP_SA_EDGEKIND;break;
        case SDRATTR_EDGENODE1HORZDIST  : nResId = SIP_SA_EDGENODE1HORZDIST;break;
        case SDRATTR_EDGENODE1VERTDIST  : nResId = SIP_SA_EDGENODE1VERTDIST;break;
        case SDRATTR_EDGENODE2HORZDIST  : nResId = SIP_SA_EDGENODE2HORZDIST;break;
        case SDRATTR_EDGENODE2VERTDIST  : nResId = SIP_SA_EDGENODE2VERTDIST;break;
        case SDRATTR_EDGENODE1GLUEDIST  : nResId = SIP_SA_EDGENODE1GLUEDIST;break;
        case SDRATTR_EDGENODE2GLUEDIST  : nResId = SIP_SA_EDGENODE2GLUEDIST;break;
        case SDRATTR_EDGELINEDELTAANZ   : nResId = SIP_SA_EDGELINEDELTAANZ;break;
        case SDRATTR_EDGELINE1DELTA     : nResId = SIP_SA_EDGELINE1DELTA;break;
        case SDRATTR_EDGELINE2DELTA     : nResId = SIP_SA_EDGELINE2DELTA;break;
        case SDRATTR_EDGELINE3DELTA     : nResId = SIP_SA_EDGELINE3DELTA;break;
        case SDRATTR_EDGERESERVE02      : nResId = SIP_SA_EDGERESERVE02;break;
        case SDRATTR_EDGERESERVE03      : nResId = SIP_SA_EDGERESERVE03;break;
        case SDRATTR_EDGERESERVE04      : nResId = SIP_SA_EDGERESERVE04;break;
        case SDRATTR_EDGERESERVE05      : nResId = SIP_SA_EDGERESERVE05;break;
        case SDRATTR_EDGERESERVE06      : nResId = SIP_SA_EDGERESERVE06;break;
        case SDRATTR_EDGERESERVE07      : nResId = SIP_SA_EDGERESERVE07;break;
        case SDRATTR_EDGERESERVE08      : nResId = SIP_SA_EDGERESERVE08;break;
        case SDRATTR_EDGERESERVE09      : nResId = SIP_SA_EDGERESERVE09;break;
        case SDRATTRSET_EDGE            : nResId = SIP_SDRATTRSET_EDGE;break;

        case SDRATTR_MEASUREKIND             : nResId = SIP_SA_MEASUREKIND;break;
        case SDRATTR_MEASURETEXTHPOS         : nResId = SIP_SA_MEASURETEXTHPOS;break;
        case SDRATTR_MEASURETEXTVPOS         : nResId = SIP_SA_MEASURETEXTVPOS;break;
        case SDRATTR_MEASURELINEDIST         : nResId = SIP_SA_MEASURELINEDIST;break;
        case SDRATTR_MEASUREHELPLINEOVERHANG : nResId = SIP_SA_MEASUREHELPLINEOVERHANG;break;
        case SDRATTR_MEASUREHELPLINEDIST     : nResId = SIP_SA_MEASUREHELPLINEDIST;break;
        case SDRATTR_MEASUREHELPLINE1LEN     : nResId = SIP_SA_MEASUREHELPLINE1LEN;break;
        case SDRATTR_MEASUREHELPLINE2LEN     : nResId = SIP_SA_MEASUREHELPLINE2LEN;break;
        case SDRATTR_MEASUREBELOWREFEDGE     : nResId = SIP_SA_MEASUREBELOWREFEDGE;break;
        case SDRATTR_MEASURETEXTROTA90       : nResId = SIP_SA_MEASURETEXTROTA90;break;
        case SDRATTR_MEASURETEXTUPSIDEDOWN   : nResId = SIP_SA_MEASURETEXTUPSIDEDOWN;break;
        case SDRATTR_MEASUREOVERHANG         : nResId = SIP_SA_MEASUREOVERHANG;break;
        case SDRATTR_MEASUREUNIT             : nResId = SIP_SA_MEASUREUNIT;break;
        case SDRATTR_MEASURESCALE            : nResId = SIP_SA_MEASURESCALE;break;
        case SDRATTR_MEASURESHOWUNIT         : nResId = SIP_SA_MEASURESHOWUNIT;break;
        case SDRATTR_MEASUREFORMATSTRING     : nResId = SIP_SA_MEASUREFORMATSTRING;break;
        case SDRATTR_MEASURETEXTAUTOANGLE    : nResId = SIP_SA_MEASURETEXTAUTOANGLE;break;
        case SDRATTR_MEASURETEXTAUTOANGLEVIEW: nResId = SIP_SA_MEASURETEXTAUTOANGLEVIEW;break;
        case SDRATTR_MEASURETEXTISFIXEDANGLE : nResId = SIP_SA_MEASURETEXTISFIXEDANGLE;break;
        case SDRATTR_MEASURETEXTFIXEDANGLE   : nResId = SIP_SA_MEASURETEXTFIXEDANGLE;break;
        case SDRATTR_MEASURERESERVE04        : nResId = SIP_SA_MEASURERESERVE04;break;
        case SDRATTR_MEASURERESERVE05        : nResId = SIP_SA_MEASURERESERVE05;break;
        case SDRATTR_MEASURERESERVE06        : nResId = SIP_SA_MEASURERESERVE06;break;
        case SDRATTR_MEASURERESERVE07        : nResId = SIP_SA_MEASURERESERVE07;break;
        case SDRATTRSET_MEASURE              : nResId = SIP_SDRATTRSET_MEASURE;break;

        case SDRATTR_CIRCKIND      : nResId = SIP_SA_CIRCKIND;break;
        case SDRATTR_CIRCSTARTANGLE: nResId = SIP_SA_CIRCSTARTANGLE;break;
        case SDRATTR_CIRCENDANGLE  : nResId = SIP_SA_CIRCENDANGLE;break;
        case SDRATTR_CIRCRESERVE0  : nResId = SIP_SA_CIRCRESERVE0;break;
        case SDRATTR_CIRCRESERVE1  : nResId = SIP_SA_CIRCRESERVE1;break;
        case SDRATTR_CIRCRESERVE2  : nResId = SIP_SA_CIRCRESERVE2;break;
        case SDRATTR_CIRCRESERVE3  : nResId = SIP_SA_CIRCRESERVE3;break;
        case SDRATTRSET_CIRC       : nResId = SIP_SDRATTRSET_CIRC;break;

        case SDRATTR_OBJMOVEPROTECT : nResId = SIP_SA_OBJMOVEPROTECT;break;
        case SDRATTR_OBJSIZEPROTECT : nResId = SIP_SA_OBJSIZEPROTECT;break;
        case SDRATTR_OBJPRINTABLE   : nResId = SIP_SA_OBJPRINTABLE;break;
        case SDRATTR_LAYERID        : nResId = SIP_SA_LAYERID;break;
        case SDRATTR_LAYERNAME      : nResId = SIP_SA_LAYERNAME;break;
        case SDRATTR_OBJECTNAME     : nResId = SIP_SA_OBJECTNAME;break;
        case SDRATTR_ALLPOSITIONX   : nResId = SIP_SA_ALLPOSITIONX;break;
        case SDRATTR_ALLPOSITIONY   : nResId = SIP_SA_ALLPOSITIONY;break;
        case SDRATTR_ALLSIZEWIDTH   : nResId = SIP_SA_ALLSIZEWIDTH;break;
        case SDRATTR_ALLSIZEHEIGHT  : nResId = SIP_SA_ALLSIZEHEIGHT;break;
        case SDRATTR_ONEPOSITIONX   : nResId = SIP_SA_ONEPOSITIONX;break;
        case SDRATTR_ONEPOSITIONY   : nResId = SIP_SA_ONEPOSITIONY;break;
        case SDRATTR_ONESIZEWIDTH   : nResId = SIP_SA_ONESIZEWIDTH;break;
        case SDRATTR_ONESIZEHEIGHT  : nResId = SIP_SA_ONESIZEHEIGHT;break;
        case SDRATTR_LOGICSIZEWIDTH : nResId = SIP_SA_LOGICSIZEWIDTH;break;
        case SDRATTR_LOGICSIZEHEIGHT: nResId = SIP_SA_LOGICSIZEHEIGHT;break;
        case SDRATTR_ROTATEANGLE    : nResId = SIP_SA_ROTATEANGLE;break;
        case SDRATTR_SHEARANGLE     : nResId = SIP_SA_SHEARANGLE;break;
        case SDRATTR_MOVEX          : nResId = SIP_SA_MOVEX;break;
        case SDRATTR_MOVEY          : nResId = SIP_SA_MOVEY;break;
        case SDRATTR_RESIZEXONE     : nResId = SIP_SA_RESIZEXONE;break;
        case SDRATTR_RESIZEYONE     : nResId = SIP_SA_RESIZEYONE;break;
        case SDRATTR_ROTATEONE      : nResId = SIP_SA_ROTATEONE;break;
        case SDRATTR_HORZSHEARONE   : nResId = SIP_SA_HORZSHEARONE;break;
        case SDRATTR_VERTSHEARONE   : nResId = SIP_SA_VERTSHEARONE;break;
        case SDRATTR_RESIZEXALL     : nResId = SIP_SA_RESIZEXALL;break;
        case SDRATTR_RESIZEYALL     : nResId = SIP_SA_RESIZEYALL;break;
        case SDRATTR_ROTATEALL      : nResId = SIP_SA_ROTATEALL;break;
        case SDRATTR_HORZSHEARALL   : nResId = SIP_SA_HORZSHEARALL;break;
        case SDRATTR_VERTSHEARALL   : nResId = SIP_SA_VERTSHEARALL;break;
        case SDRATTR_TRANSFORMREF1X : nResId = SIP_SA_TRANSFORMREF1X;break;
        case SDRATTR_TRANSFORMREF1Y : nResId = SIP_SA_TRANSFORMREF1Y;break;
        case SDRATTR_TRANSFORMREF2X : nResId = SIP_SA_TRANSFORMREF2X;break;
        case SDRATTR_TRANSFORMREF2Y : nResId = SIP_SA_TRANSFORMREF2Y;break;

        case SDRATTR_GRAFRED            : nResId = SIP_SA_GRAFRED;break;
        case SDRATTR_GRAFGREEN          : nResId = SIP_SA_GRAFGREEN;break;
        case SDRATTR_GRAFBLUE           : nResId = SIP_SA_GRAFBLUE;break;
        case SDRATTR_GRAFLUMINANCE      : nResId = SIP_SA_GRAFLUMINANCE;break;
        case SDRATTR_GRAFCONTRAST       : nResId = SIP_SA_GRAFCONTRAST;break;
        case SDRATTR_GRAFGAMMA          : nResId = SIP_SA_GRAFGAMMA;break;
        case SDRATTR_GRAFTRANSPARENCE   : nResId = SIP_SA_GRAFTRANSPARENCE;break;
        case SDRATTR_GRAFINVERT         : nResId = SIP_SA_GRAFINVERT;break;
        case SDRATTR_GRAFMODE           : nResId = SIP_SA_GRAFMODE;break;
        case SDRATTR_GRAFCROP           : nResId = SIP_SA_GRAFCROP;break;
        case SDRATTR_GRAFRESERVE3       : nResId = SIP_SA_GRAFRESERVE3;break;
        case SDRATTR_GRAFRESERVE4       : nResId = SIP_SA_GRAFRESERVE4;break;
        case SDRATTR_GRAFRESERVE5       : nResId = SIP_SA_GRAFRESERVE5;break;
        case SDRATTR_GRAFRESERVE6       : nResId = SIP_SA_GRAFRESERVE6;break;
        case SDRATTRSET_GRAF            : nResId = SIP_SDRATTRSET_GRAF;break;

        case EE_PARA_HYPHENATE  : nResId = SIP_EE_PARA_HYPHENATE;break;
        case EE_PARA_BULLETSTATE: nResId = SIP_EE_PARA_BULLETSTATE;break;
        case EE_PARA_OUTLLRSPACE: nResId = SIP_EE_PARA_OUTLLRSPACE;break;
        case EE_PARA_OUTLLEVEL  : nResId = SIP_EE_PARA_OUTLLEVEL;break;
        case EE_PARA_BULLET     : nResId = SIP_EE_PARA_BULLET;break;
        case EE_PARA_LRSPACE    : nResId = SIP_EE_PARA_LRSPACE;break;
        case EE_PARA_ULSPACE    : nResId = SIP_EE_PARA_ULSPACE;break;
        case EE_PARA_SBL        : nResId = SIP_EE_PARA_SBL;break;
        case EE_PARA_JUST       : nResId = SIP_EE_PARA_JUST;break;
        case EE_PARA_TABS       : nResId = SIP_EE_PARA_TABS;break;

        case EE_CHAR_COLOR      : nResId = SIP_EE_CHAR_COLOR;break;
        case EE_CHAR_FONTINFO   : nResId = SIP_EE_CHAR_FONTINFO;break;
        case EE_CHAR_FONTHEIGHT : nResId = SIP_EE_CHAR_FONTHEIGHT;break;
        case EE_CHAR_FONTWIDTH  : nResId = SIP_EE_CHAR_FONTWIDTH;break;
        case EE_CHAR_WEIGHT     : nResId = SIP_EE_CHAR_WEIGHT;break;
        case EE_CHAR_UNDERLINE  : nResId = SIP_EE_CHAR_UNDERLINE;break;
        case EE_CHAR_STRIKEOUT  : nResId = SIP_EE_CHAR_STRIKEOUT;break;
        case EE_CHAR_ITALIC     : nResId = SIP_EE_CHAR_ITALIC;break;
        case EE_CHAR_OUTLINE    : nResId = SIP_EE_CHAR_OUTLINE;break;
        case EE_CHAR_SHADOW     : nResId = SIP_EE_CHAR_SHADOW;break;
        case EE_CHAR_ESCAPEMENT : nResId = SIP_EE_CHAR_ESCAPEMENT;break;
        case EE_CHAR_PAIRKERNING: nResId = SIP_EE_CHAR_PAIRKERNING;break;
        case EE_CHAR_KERNING    : nResId = SIP_EE_CHAR_KERNING;break;
        case EE_CHAR_WLM        : nResId = SIP_EE_CHAR_WLM;break;
        case EE_FEATURE_TAB     : nResId = SIP_EE_FEATURE_TAB;break;
        case EE_FEATURE_LINEBR  : nResId = SIP_EE_FEATURE_LINEBR;break;
        case EE_FEATURE_NOTCONV : nResId = SIP_EE_FEATURE_NOTCONV;break;
        case EE_FEATURE_FIELD   : nResId = SIP_EE_FEATURE_FIELD;break;
    } // switch

    rItemName = String( ResId( nResId, pResMgr ) );

    return (BOOL)rItemName.Len();
}

BOOL SdrItemPool::TakeWhichName(USHORT nWhich, ByteString& rWhichName)
{
    ByteString aStr;

#ifdef DBG_UTIL
    switch(nWhich)
    {
        case XATTR_LINESTYLE                 : aStr="XATTR_LINESTYLE                 "; break;
        case XATTR_LINEDASH                  : aStr="XATTR_LINEDASH                  "; break;
        case XATTR_LINEWIDTH                 : aStr="XATTR_LINEWIDTH                 "; break;
        case XATTR_LINECOLOR                 : aStr="XATTR_LINECOLOR                 "; break;
        case XATTR_LINESTART                 : aStr="XATTR_LINESTART                 "; break;
        case XATTR_LINEEND                   : aStr="XATTR_LINEEND                   "; break;
        case XATTR_LINESTARTWIDTH            : aStr="XATTR_LINESTARTWIDTH            "; break;
        case XATTR_LINEENDWIDTH              : aStr="XATTR_LINEENDWIDTH              "; break;
        case XATTR_LINESTARTCENTER           : aStr="XATTR_LINESTARTCENTER           "; break;
        case XATTR_LINEENDCENTER             : aStr="XATTR_LINEENDCENTER             "; break;
        case XATTR_LINETRANSPARENCE          : aStr="XATTR_LINETRANSPARENCE          "; break;
        case XATTR_LINEJOINT                 : aStr="XATTR_LINEJOINT                 "; break;
        case XATTR_LINERESERVED2             : aStr="XATTR_LINERESERVED2             "; break;
        case XATTR_LINERESERVED3             : aStr="XATTR_LINERESERVED3             "; break;
        case XATTR_LINERESERVED4             : aStr="XATTR_LINERESERVED4             "; break;
        case XATTR_LINERESERVED5             : aStr="XATTR_LINERESERVED5             "; break;
        case XATTR_LINERESERVED_LAST         : aStr="XATTR_LINERESERVED_LAST         "; break;
        case XATTRSET_LINE                   : aStr="XATTRSET_LINE                   "; break;

        case XATTR_FILLSTYLE                 : aStr="XATTR_FILLSTYLE                 "; break;
        case XATTR_FILLCOLOR                 : aStr="XATTR_FILLCOLOR                 "; break;
        case XATTR_FILLGRADIENT              : aStr="XATTR_FILLGRADIENT              "; break;
        case XATTR_FILLHATCH                 : aStr="XATTR_FILLHATCH                 "; break;
        case XATTR_FILLBITMAP                : aStr="XATTR_FILLBITMAP                "; break;
        case XATTR_FILLTRANSPARENCE          : aStr="XATTR_FILLTRANSPARENCE          "; break;
        case XATTR_GRADIENTSTEPCOUNT         : aStr="XATTR_GRADIENTSTEPCOUNT         "; break;

        case XATTR_FILLBMP_TILE              : aStr="XATTR_FILLBMP_TILE              "; break;
        case XATTR_FILLBMP_POS               : aStr="XATTR_FILLBMP_POS               "; break;
        case XATTR_FILLBMP_SIZEX             : aStr="XATTR_FILLBMP_SIZEX             "; break;
        case XATTR_FILLBMP_SIZEY             : aStr="XATTR_FILLBMP_SIZEY             "; break;
        case XATTR_FILLFLOATTRANSPARENCE     : aStr="XATTR_FILLFLOATTRANSPARENCE     "; break;
        case XATTR_FILLRESERVED2             : aStr="XATTR_FILLRESERVED2             "; break;
        case XATTR_FILLBMP_SIZELOG           : aStr="XATTR_FILLBMP_SIZELOG           "; break;
        case XATTR_FILLBMP_TILEOFFSETX       : aStr="XATTR_FILLBMP_TILEOFFSETX       "; break;
        case XATTR_FILLBMP_TILEOFFSETY       : aStr="XATTR_FILLBMP_TILEOFFSETY       "; break;

        case XATTR_FILLBMP_STRETCH           : aStr="XATTR_FILLBMP_STRETCH           "; break;
        case XATTR_FILLRESERVED3             : aStr="XATTR_FILLRESERVED3             "; break;
        case XATTR_FILLRESERVED4             : aStr="XATTR_FILLRESERVED4             "; break;
        case XATTR_FILLRESERVED5             : aStr="XATTR_FILLRESERVED5             "; break;
        case XATTR_FILLRESERVED6             : aStr="XATTR_FILLRESERVED6             "; break;
        case XATTR_FILLRESERVED7             : aStr="XATTR_FILLRESERVED7             "; break;
        case XATTR_FILLRESERVED8             : aStr="XATTR_FILLRESERVED8             "; break;
        case XATTR_FILLBMP_POSOFFSETX        : aStr="XATTR_FILLBMP_POSOFFSETX        "; break;
        case XATTR_FILLBMP_POSOFFSETY        : aStr="XATTR_FILLBMP_POSOFFSETY        "; break;
        case XATTR_FILLBACKGROUND            : aStr="XATTR_FILLBACKGROUND            "; break;
        case XATTR_FILLRESERVED10            : aStr="XATTR_FILLRESERVED10            "; break;
        case XATTR_FILLRESERVED11            : aStr="XATTR_FILLRESERVED11            "; break;
        case XATTR_FILLRESERVED_LAST         : aStr="XATTR_FILLRESERVED_LAST         "; break;

        case XATTRSET_FILL                   : aStr="XATTRSET_FILL                   "; break;

        case XATTR_FORMTXTSTYLE              : aStr="XATTR_FORMTXTSTYLE              "; break;
        case XATTR_FORMTXTADJUST             : aStr="XATTR_FORMTXTADJUST             "; break;
        case XATTR_FORMTXTDISTANCE           : aStr="XATTR_FORMTXTDISTANCE           "; break;
        case XATTR_FORMTXTSTART              : aStr="XATTR_FORMTXTSTART              "; break;
        case XATTR_FORMTXTMIRROR             : aStr="XATTR_FORMTXTMIRROR             "; break;
        case XATTR_FORMTXTOUTLINE            : aStr="XATTR_FORMTXTOUTLINE            "; break;
        case XATTR_FORMTXTSHADOW             : aStr="XATTR_FORMTXTSHADOW             "; break;
        case XATTR_FORMTXTSHDWCOLOR          : aStr="XATTR_FORMTXTSHDWCOLOR          "; break;
        case XATTR_FORMTXTSHDWXVAL           : aStr="XATTR_FORMTXTSHDWXVAL           "; break;
        case XATTR_FORMTXTSHDWYVAL           : aStr="XATTR_FORMTXTSHDWYVAL           "; break;
        case XATTR_FORMTXTSTDFORM            : aStr="XATTR_FORMTXTSTDFORM            "; break;
        case XATTR_FORMTXTHIDEFORM           : aStr="XATTR_FORMTXTHIDEFORM           "; break;
        case XATTR_FORMTXTSHDWTRANSP         : aStr="XATTR_FORMTXTSHDWTRANSP         "; break;
        case XATTR_FTRESERVED2               : aStr="XATTR_FTRESERVED2               "; break;
        case XATTR_FTRESERVED3               : aStr="XATTR_FTRESERVED3               "; break;
        case XATTR_FTRESERVED4               : aStr="XATTR_FTRESERVED4               "; break;
        case XATTR_FTRESERVED5               : aStr="XATTR_FTRESERVED5               "; break;
        case XATTR_FTRESERVED_LAST           : aStr="XATTR_FTRESERVED_LAST           "; break;
        case XATTRSET_TEXT                   : aStr="XATTRSET_TEXT                   "; break;

        case SDRATTR_SHADOW                  : aStr="SDRATTR_SHADOW                  "; break;
        case SDRATTR_SHADOWCOLOR             : aStr="SDRATTR_SHADOWCOLOR             "; break;
        case SDRATTR_SHADOWXDIST             : aStr="SDRATTR_SHADOWXDIST             "; break;
        case SDRATTR_SHADOWYDIST             : aStr="SDRATTR_SHADOWYDIST             "; break;
        case SDRATTR_SHADOWTRANSPARENCE      : aStr="SDRATTR_SHADOWTRANSPARENCE      "; break;
        case SDRATTR_SHADOW3D                : aStr="SDRATTR_SHADOW3D                "; break;
        case SDRATTR_SHADOWPERSP             : aStr="SDRATTR_SHADOWPERSP             "; break;
        case SDRATTR_SHADOWRESERVE1          : aStr="SDRATTR_SHADOWRESERVE1          "; break;
        case SDRATTR_SHADOWRESERVE2          : aStr="SDRATTR_SHADOWRESERVE2          "; break;
        case SDRATTR_SHADOWRESERVE3          : aStr="SDRATTR_SHADOWRESERVE3          "; break;
        case SDRATTR_SHADOWRESERVE4          : aStr="SDRATTR_SHADOWRESERVE4          "; break;
        case SDRATTR_SHADOWRESERVE5          : aStr="SDRATTR_SHADOWRESERVE5          "; break;
        case SDRATTRSET_SHADOW               : aStr="SDRATTRSET_SHADOW               "; break;

        case SDRATTR_CAPTIONTYPE             : aStr="SDRATTR_CAPTIONTYPE             "; break;
        case SDRATTR_CAPTIONFIXEDANGLE       : aStr="SDRATTR_CAPTIONFIXEDANGLE       "; break;
        case SDRATTR_CAPTIONANGLE            : aStr="SDRATTR_CAPTIONANGLE            "; break;
        case SDRATTR_CAPTIONGAP              : aStr="SDRATTR_CAPTIONGAP              "; break;
        case SDRATTR_CAPTIONESCDIR           : aStr="SDRATTR_CAPTIONESCDIR           "; break;
        case SDRATTR_CAPTIONESCISREL         : aStr="SDRATTR_CAPTIONESCISREL         "; break;
        case SDRATTR_CAPTIONESCREL           : aStr="SDRATTR_CAPTIONESCREL           "; break;
        case SDRATTR_CAPTIONESCABS           : aStr="SDRATTR_CAPTIONESCABS           "; break;
        case SDRATTR_CAPTIONLINELEN          : aStr="SDRATTR_CAPTIONLINELEN          "; break;
        case SDRATTR_CAPTIONFITLINELEN       : aStr="SDRATTR_CAPTIONFITLINELEN       "; break;
        case SDRATTR_CAPTIONRESERVE1         : aStr="SDRATTR_CAPTIONRESERVE1         "; break;
        case SDRATTR_CAPTIONRESERVE2         : aStr="SDRATTR_CAPTIONRESERVE2         "; break;
        case SDRATTR_CAPTIONRESERVE3         : aStr="SDRATTR_CAPTIONRESERVE3         "; break;
        case SDRATTR_CAPTIONRESERVE4         : aStr="SDRATTR_CAPTIONRESERVE4         "; break;
        case SDRATTR_CAPTIONRESERVE5         : aStr="SDRATTR_CAPTIONRESERVE5         "; break;
        case SDRATTRSET_CAPTION              : aStr="SDRATTRSET_CAPTION              "; break;

        case SDRATTRSET_OUTLINER             : aStr="SDRATTRSET_OUTLINER             "; break;

        case SDRATTR_ECKENRADIUS             : aStr="SDRATTR_ECKENRADIUS             "; break;
        case SDRATTR_TEXT_MINFRAMEHEIGHT     : aStr="SDRATTR_TEXT_MINFRAMEHEIGHT     "; break;
        case SDRATTR_TEXT_AUTOGROWHEIGHT     : aStr="SDRATTR_TEXT_AUTOGROWHEIGHT     "; break;
        case SDRATTR_TEXT_FITTOSIZE          : aStr="SDRATTR_TEXT_FITTOSIZE          "; break;
        case SDRATTR_TEXT_LEFTDIST           : aStr="SDRATTR_TEXT_LEFTDIST           "; break;
        case SDRATTR_TEXT_RIGHTDIST          : aStr="SDRATTR_TEXT_RIGHTDIST          "; break;
        case SDRATTR_TEXT_UPPERDIST          : aStr="SDRATTR_TEXT_UPPERDIST          "; break;
        case SDRATTR_TEXT_LOWERDIST          : aStr="SDRATTR_TEXT_LOWERDIST          "; break;
        case SDRATTR_TEXT_VERTADJUST         : aStr="SDRATTR_TEXT_VERTADJUST         "; break;
        case SDRATTR_TEXT_MAXFRAMEHEIGHT     : aStr="SDRATTR_TEXT_MAXFRAMEHEIGHT     "; break;
        case SDRATTR_TEXT_MINFRAMEWIDTH      : aStr="SDRATTR_TEXT_MINFRAMEWIDTH      "; break;
        case SDRATTR_TEXT_MAXFRAMEWIDTH      : aStr="SDRATTR_TEXT_MAXFRAMEWIDTH      "; break;
        case SDRATTR_TEXT_AUTOGROWWIDTH      : aStr="SDRATTR_TEXT_AUTOGROWWIDTH      "; break;
        case SDRATTR_TEXT_HORZADJUST         : aStr="SDRATTR_TEXT_HORZADJUST         "; break;
        case SDRATTR_TEXT_ANIKIND            : aStr="SDRATTR_TEXT_ANIKIND            "; break;
        case SDRATTR_TEXT_ANIDIRECTION       : aStr="SDRATTR_TEXT_ANIDIRECTION       "; break;
        case SDRATTR_TEXT_ANISTARTINSIDE     : aStr="SDRATTR_TEXT_ANISTARTINSIDE     "; break;
        case SDRATTR_TEXT_ANISTOPINSIDE      : aStr="SDRATTR_TEXT_ANISTOPINSIDE      "; break;
        case SDRATTR_TEXT_ANICOUNT           : aStr="SDRATTR_TEXT_ANICOUNT           "; break;
        case SDRATTR_TEXT_ANIDELAY           : aStr="SDRATTR_TEXT_ANIDELAY           "; break;
        case SDRATTR_TEXT_ANIAMOUNT          : aStr="SDRATTR_TEXT_ANIAMOUNT          "; break;
        case SDRATTR_TEXT_CONTOURFRAME       : aStr="SDRATTR_TEXT_CONTOURFRAME       "; break;
        case SDRATTR_AUTOSHAPE_ADJUSTMENT    : aStr="SDRATTR_AUTOSHAPE_ADJUSTMENT    "; break;
        case SDRATTR_RESERVE14               : aStr="SDRATTR_RESERVE14               "; break;
        case SDRATTR_RESERVE15               : aStr="SDRATTR_RESERVE15               "; break;
        case SDRATTR_RESERVE16               : aStr="SDRATTR_RESERVE16               "; break;
        case SDRATTR_RESERVE17               : aStr="SDRATTR_RESERVE17               "; break;
        case SDRATTR_RESERVE18               : aStr="SDRATTR_RESERVE18               "; break;
        case SDRATTR_RESERVE19               : aStr="SDRATTR_RESERVE19               "; break;
        case SDRATTRSET_MISC                 : aStr="SDRATTRSET_MISC                 "; break;

        case SDRATTR_EDGEKIND                : aStr="SDRATTR_EDGEKIND                "; break;
        case SDRATTR_EDGENODE1HORZDIST       : aStr="SDRATTR_EDGENODE1HORZDIST       "; break;
        case SDRATTR_EDGENODE1VERTDIST       : aStr="SDRATTR_EDGENODE1VERTDIST       "; break;
        case SDRATTR_EDGENODE2HORZDIST       : aStr="SDRATTR_EDGENODE2HORZDIST       "; break;
        case SDRATTR_EDGENODE2VERTDIST       : aStr="SDRATTR_EDGENODE2VERTDIST       "; break;
        case SDRATTR_EDGENODE1GLUEDIST       : aStr="SDRATTR_EDGENODE1GLUEDIST       "; break;
        case SDRATTR_EDGENODE2GLUEDIST       : aStr="SDRATTR_EDGENODE2GLUEDIST       "; break;
        case SDRATTR_EDGELINEDELTAANZ        : aStr="SDRATTR_EDGELINEDELTAANZ        "; break;
        case SDRATTR_EDGELINE1DELTA          : aStr="SDRATTR_EDGELINE1DELTA          "; break;
        case SDRATTR_EDGELINE2DELTA          : aStr="SDRATTR_EDGELINE2DELTA          "; break;
        case SDRATTR_EDGELINE3DELTA          : aStr="SDRATTR_EDGELINE3DELTA          "; break;
        case SDRATTR_EDGERESERVE02           : aStr="SDRATTR_EDGERESERVE02           "; break;
        case SDRATTR_EDGERESERVE03           : aStr="SDRATTR_EDGERESERVE03           "; break;
        case SDRATTR_EDGERESERVE04           : aStr="SDRATTR_EDGERESERVE04           "; break;
        case SDRATTR_EDGERESERVE05           : aStr="SDRATTR_EDGERESERVE05           "; break;
        case SDRATTR_EDGERESERVE06           : aStr="SDRATTR_EDGERESERVE06           "; break;
        case SDRATTR_EDGERESERVE07           : aStr="SDRATTR_EDGERESERVE07           "; break;
        case SDRATTR_EDGERESERVE08           : aStr="SDRATTR_EDGERESERVE08           "; break;
        case SDRATTR_EDGERESERVE09           : aStr="SDRATTR_EDGERESERVE09           "; break;
        case SDRATTRSET_EDGE                 : aStr="SDRATTRSET_EDGE                 "; break;

        case SDRATTR_MEASUREKIND             : aStr="SDRATTR_MEASUREKIND             "; break;
        case SDRATTR_MEASURETEXTHPOS         : aStr="SDRATTR_MEASURETEXTHPOS         "; break;
        case SDRATTR_MEASURETEXTVPOS         : aStr="SDRATTR_MEASURETEXTVPOS         "; break;
        case SDRATTR_MEASURELINEDIST         : aStr="SDRATTR_MEASURELINEDIST         "; break;
        case SDRATTR_MEASUREHELPLINEOVERHANG : aStr="SDRATTR_MEASUREHELPLINEOVERHANG "; break;
        case SDRATTR_MEASUREHELPLINEDIST     : aStr="SDRATTR_MEASUREHELPLINEDIST     "; break;
        case SDRATTR_MEASUREHELPLINE1LEN     : aStr="SDRATTR_MEASUREHELPLINE1LEN     "; break;
        case SDRATTR_MEASUREHELPLINE2LEN     : aStr="SDRATTR_MEASUREHELPLINE2LEN     "; break;
        case SDRATTR_MEASUREBELOWREFEDGE     : aStr="SDRATTR_MEASUREBELOWREFEDGE     "; break;
        case SDRATTR_MEASURETEXTROTA90       : aStr="SDRATTR_MEASURETEXTROTA90       "; break;
        case SDRATTR_MEASURETEXTUPSIDEDOWN   : aStr="SDRATTR_MEASURETEXTUPSIDEDOWN   "; break;
        case SDRATTR_MEASUREOVERHANG         : aStr="SDRATTR_MEASUREOVERHANG         "; break;
        case SDRATTR_MEASUREUNIT             : aStr="SDRATTR_MEASUREUNIT             "; break;
        case SDRATTR_MEASURESCALE            : aStr="SDRATTR_MEASURESCALE            "; break;
        case SDRATTR_MEASURESHOWUNIT         : aStr="SDRATTR_MEASURESHOWUNIT         "; break;
        case SDRATTR_MEASUREFORMATSTRING     : aStr="SDRATTR_MEASUREFORMATSTRING     "; break;
        case SDRATTR_MEASURETEXTAUTOANGLE    : aStr="SDRATTR_MEASURETEXTAUTOANGLE    "; break;
        case SDRATTR_MEASURETEXTAUTOANGLEVIEW: aStr="SDRATTR_MEASURETEXTAUTOANGLEVIEW"; break;
        case SDRATTR_MEASURETEXTISFIXEDANGLE : aStr="SDRATTR_MEASURETEXTISFIXEDANGLE "; break;
        case SDRATTR_MEASURETEXTFIXEDANGLE   : aStr="SDRATTR_MEASURETEXTFIXEDANGLE   "; break;
        case SDRATTR_MEASURERESERVE04        : aStr="SDRATTR_MEASURERESERVE04        "; break;
        case SDRATTR_MEASURERESERVE05        : aStr="SDRATTR_MEASURERESERVE05        "; break;
        case SDRATTR_MEASURERESERVE06        : aStr="SDRATTR_MEASURERESERVE06        "; break;
        case SDRATTR_MEASURERESERVE07        : aStr="SDRATTR_MEASURERESERVE07        "; break;
        case SDRATTRSET_MEASURE              : aStr="SDRATTRSET_MEASURE              "; break;

        case SDRATTR_CIRCKIND                : aStr="SDRATTR_CIRCKIND                "; break;
        case SDRATTR_CIRCSTARTANGLE          : aStr="SDRATTR_CIRCSTARTANGLE          "; break;
        case SDRATTR_CIRCENDANGLE            : aStr="SDRATTR_CIRCENDANGLE            "; break;
        case SDRATTR_CIRCRESERVE0            : aStr="SDRATTR_CIRCRESERVE0            "; break;
        case SDRATTR_CIRCRESERVE1            : aStr="SDRATTR_CIRCRESERVE1            "; break;
        case SDRATTR_CIRCRESERVE2            : aStr="SDRATTR_CIRCRESERVE2            "; break;
        case SDRATTR_CIRCRESERVE3            : aStr="SDRATTR_CIRCRESERVE3            "; break;
        case SDRATTRSET_CIRC                 : aStr="SDRATTRSET_CIRC                 "; break;

        case SDRATTR_OBJMOVEPROTECT          : aStr="SDRATTR_OBJMOVEPROTECT          "; break;
        case SDRATTR_OBJSIZEPROTECT          : aStr="SDRATTR_OBJSIZEPROTECT          "; break;
        case SDRATTR_OBJPRINTABLE            : aStr="SDRATTR_OBJPRINTABLE            "; break;
        case SDRATTR_LAYERID                 : aStr="SDRATTR_LAYERID                 "; break;
        case SDRATTR_LAYERNAME               : aStr="SDRATTR_LAYERNAME               "; break;
        case SDRATTR_OBJECTNAME              : aStr="SDRATTR_OBJECTNAME              "; break;
        case SDRATTR_ALLPOSITIONX            : aStr="SDRATTR_ALLPOSITIONX            "; break;
        case SDRATTR_ALLPOSITIONY            : aStr="SDRATTR_ALLPOSITIONY            "; break;
        case SDRATTR_ALLSIZEWIDTH            : aStr="SDRATTR_ALLSIZEWIDTH            "; break;
        case SDRATTR_ALLSIZEHEIGHT           : aStr="SDRATTR_ALLSIZEHEIGHT           "; break;
        case SDRATTR_ONEPOSITIONX            : aStr="SDRATTR_ONEPOSITIONX            "; break;
        case SDRATTR_ONEPOSITIONY            : aStr="SDRATTR_ONEPOSITIONY            "; break;
        case SDRATTR_ONESIZEWIDTH            : aStr="SDRATTR_ONESIZEWIDTH            "; break;
        case SDRATTR_ONESIZEHEIGHT           : aStr="SDRATTR_ONESIZEHEIGHT           "; break;
        case SDRATTR_LOGICSIZEWIDTH          : aStr="SDRATTR_LOGICSIZEWIDTH          "; break;
        case SDRATTR_LOGICSIZEHEIGHT         : aStr="SDRATTR_LOGICSIZEHEIGHT         "; break;
        case SDRATTR_ROTATEANGLE             : aStr="SDRATTR_ROTATEANGLE             "; break;
        case SDRATTR_SHEARANGLE              : aStr="SDRATTR_SHEARANGLE              "; break;
        case SDRATTR_MOVEX                   : aStr="SDRATTR_MOVEX                   "; break;
        case SDRATTR_MOVEY                   : aStr="SDRATTR_MOVEY                   "; break;
        case SDRATTR_RESIZEXONE              : aStr="SDRATTR_RESIZEXONE              "; break;
        case SDRATTR_RESIZEYONE              : aStr="SDRATTR_RESIZEYONE              "; break;
        case SDRATTR_ROTATEONE               : aStr="SDRATTR_ROTATEONE               "; break;
        case SDRATTR_HORZSHEARONE            : aStr="SDRATTR_HORZSHEARONE            "; break;
        case SDRATTR_VERTSHEARONE            : aStr="SDRATTR_VERTSHEARONE            "; break;
        case SDRATTR_RESIZEXALL              : aStr="SDRATTR_RESIZEXALL              "; break;
        case SDRATTR_RESIZEYALL              : aStr="SDRATTR_RESIZEYALL              "; break;
        case SDRATTR_ROTATEALL               : aStr="SDRATTR_ROTATEALL               "; break;
        case SDRATTR_HORZSHEARALL            : aStr="SDRATTR_HORZSHEARALL            "; break;
        case SDRATTR_VERTSHEARALL            : aStr="SDRATTR_VERTSHEARALL            "; break;
        case SDRATTR_TRANSFORMREF1X          : aStr="SDRATTR_TRANSFORMREF1X          "; break;
        case SDRATTR_TRANSFORMREF1Y          : aStr="SDRATTR_TRANSFORMREF1Y          "; break;
        case SDRATTR_TRANSFORMREF2X          : aStr="SDRATTR_TRANSFORMREF2X          "; break;
        case SDRATTR_TRANSFORMREF2Y          : aStr="SDRATTR_TRANSFORMREF2Y          "; break;

        case SDRATTR_GRAFRED                 : aStr="SDRATTR_GRAFRED                 "; break;
        case SDRATTR_GRAFGREEN               : aStr="SDRATTR_GRAFGREEN               "; break;
        case SDRATTR_GRAFBLUE                : aStr="SDRATTR_GRAFBLUE                "; break;
        case SDRATTR_GRAFLUMINANCE           : aStr="SDRATTR_GRAFLUMINANCE           "; break;
        case SDRATTR_GRAFCONTRAST            : aStr="SDRATTR_GRAFCONTRAST            "; break;
        case SDRATTR_GRAFGAMMA               : aStr="SDRATTR_GRAFGAMMA               "; break;
        case SDRATTR_GRAFTRANSPARENCE        : aStr="SDRATTR_GRAFTRANSPARENCE        "; break;
        case SDRATTR_GRAFINVERT              : aStr="SDRATTR_GRAFINVERT              "; break;
        case SDRATTR_GRAFMODE                : aStr="SDRATTR_GRAFMODE                "; break;
        case SDRATTR_GRAFCROP                : aStr="SDRATTR_GRAFCROP                "; break;
        case SDRATTR_GRAFRESERVE3            : aStr="SDRATTR_GRAFRESERVE3            "; break;
        case SDRATTR_GRAFRESERVE4            : aStr="SDRATTR_GRAFRESERVE4            "; break;
        case SDRATTR_GRAFRESERVE5            : aStr="SDRATTR_GRAFRESERVE5            "; break;
        case SDRATTR_GRAFRESERVE6            : aStr="SDRATTR_GRAFRESERVE6            "; break;
        case SDRATTRSET_GRAF                 : aStr="SDRATTRSET_GRAF                 "; break;

        case EE_PARA_HYPHENATE               : aStr="EE_PARA_HYPHENATE               "; break;
        case EE_PARA_BULLETSTATE             : aStr="EE_PARA_BULLETSTATE             "; break;
        case EE_PARA_OUTLLRSPACE             : aStr="EE_PARA_OUTLLRSPACE             "; break;
        case EE_PARA_OUTLLEVEL               : aStr="EE_PARA_OUTLLEVEL               "; break;
        case EE_PARA_BULLET                  : aStr="EE_PARA_BULLET                  "; break;
        case EE_PARA_LRSPACE                 : aStr="EE_PARA_LRSPACE                 "; break;
        case EE_PARA_ULSPACE                 : aStr="EE_PARA_ULSPACE                 "; break;
        case EE_PARA_SBL                     : aStr="EE_PARA_SBL                     "; break;
        case EE_PARA_JUST                    : aStr="EE_PARA_JUST                    "; break;
        case EE_PARA_TABS                    : aStr="EE_PARA_TABS                    "; break;

        case EE_CHAR_COLOR                   : aStr="EE_CHAR_COLOR                   "; break;
        case EE_CHAR_FONTINFO                : aStr="EE_CHAR_FONTINFO                "; break;
        case EE_CHAR_FONTHEIGHT              : aStr="EE_CHAR_FONTHEIGHT              "; break;
        case EE_CHAR_FONTWIDTH               : aStr="EE_CHAR_FONTWIDTH               "; break;
        case EE_CHAR_WEIGHT                  : aStr="EE_CHAR_WEIGHT                  "; break;
        case EE_CHAR_UNDERLINE               : aStr="EE_CHAR_UNDERLINE               "; break;
        case EE_CHAR_STRIKEOUT               : aStr="EE_CHAR_STRIKEOUT               "; break;
        case EE_CHAR_ITALIC                  : aStr="EE_CHAR_ITALIC                  "; break;
        case EE_CHAR_OUTLINE                 : aStr="EE_CHAR_OUTLINE                 "; break;
        case EE_CHAR_SHADOW                  : aStr="EE_CHAR_SHADOW                  "; break;
        case EE_CHAR_ESCAPEMENT              : aStr="EE_CHAR_ESCAPEMENT              "; break;
        case EE_CHAR_PAIRKERNING             : aStr="EE_CHAR_PAIRKERNING             "; break;
        case EE_CHAR_KERNING                 : aStr="EE_CHAR_KERNING                 "; break;
        case EE_CHAR_WLM                     : aStr="EE_CHAR_WLM                     "; break;
        case EE_FEATURE_TAB                  : aStr="EE_FEATURE_TAB                  "; break;
        case EE_FEATURE_LINEBR               : aStr="EE_FEATURE_LINEBR               "; break;
        case EE_FEATURE_NOTCONV              : aStr="EE_FEATURE_NOTCONV              "; break;
        case EE_FEATURE_FIELD                : aStr="EE_FEATURE_FIELD                "; break;
    } // switch

    aStr.EraseTrailingChars();
#endif // if DBG_UTIL

    rWhichName = aStr;
    return (BOOL)aStr.Len();
}

////////////////////////////////////////////////////////////////////////////////
// FractionItem
////////////////////////////////////////////////////////////////////////////////

TYPEINIT1_AUTOFACTORY(SdrFractionItem,SfxPoolItem);

SdrFractionItem::SdrFractionItem(USHORT nId, SvStream& rIn):
    SfxPoolItem(nId)
{
    INT32 nMul,nDiv;
    rIn>>nMul;
    rIn>>nDiv;
    nValue=Fraction(nMul,nDiv);
}

int __EXPORT SdrFractionItem::operator==(const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
           ((SdrFractionItem&)rCmp).GetValue()==nValue;
}

SfxItemPresentation __EXPORT SdrFractionItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit eCoreMetric,
    SfxMapUnit ePresentationMetric, XubString &rText, const International *) const
{
    if(nValue.IsValid())
    {
        INT32 nDiv = nValue.GetDenominator();
        rText = UniString::CreateFromInt32(nValue.GetNumerator());

        if(nDiv != 1)
        {
            rText += sal_Unicode('/');
            rText += UniString::CreateFromInt32(nDiv);
        }
    }
    else
    {
        rText = UniString();
        rText += sal_Unicode('?');
    }

    if(ePresentation == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        XubString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }

    return ePresentation;
}

SfxPoolItem* __EXPORT SdrFractionItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SdrFractionItem(Which(),rIn);
}

SvStream& __EXPORT SdrFractionItem::Store(SvStream& rOut, USHORT nItemVers) const
{
    rOut<<INT32(nValue.GetNumerator());
    rOut<<INT32(nValue.GetDenominator());
    return rOut;
}

SfxPoolItem* __EXPORT SdrFractionItem::Clone(SfxItemPool *pPool) const
{
    return new SdrFractionItem(Which(),GetValue());
}

#ifdef SDR_ISPOOLABLE
int __EXPORT SdrFractionItem::IsPoolable() const
{
    USHORT nId=Which();
    return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
}
#endif

////////////////////////////////////////////////////////////////////////////////
// ScaleItem
////////////////////////////////////////////////////////////////////////////////

TYPEINIT1_AUTOFACTORY(SdrScaleItem,SdrFractionItem);

SfxItemPresentation __EXPORT SdrScaleItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit eCoreMetric,
    SfxMapUnit ePresentationMetric, XubString &rText, const International *) const
{
    if(GetValue().IsValid())
    {
        INT32 nDiv = GetValue().GetDenominator();

        rText = UniString::CreateFromInt32(GetValue().GetNumerator());
        rText += sal_Unicode(':');
        rText += UniString::CreateFromInt32(nDiv);
    }
    else
    {
        rText = UniString();
        rText += sal_Unicode('?');
    }

    if(ePresentation == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        XubString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }

    return ePresentation;
}

SfxPoolItem* __EXPORT SdrScaleItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SdrScaleItem(Which(),rIn);
}

SfxPoolItem* __EXPORT SdrScaleItem::Clone(SfxItemPool *pPool) const
{
    return new SdrScaleItem(Which(),GetValue());
}

////////////////////////////////////////////////////////////////////////////////
// OnOffItem
////////////////////////////////////////////////////////////////////////////////

TYPEINIT1_AUTOFACTORY(SdrOnOffItem,SfxBoolItem);

SfxPoolItem* __EXPORT SdrOnOffItem::Clone(SfxItemPool* pPool) const
{
    return new SdrOnOffItem(Which(),GetValue());
}

SfxPoolItem* __EXPORT SdrOnOffItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SdrOnOffItem(Which(),rIn);
}

XubString __EXPORT SdrOnOffItem::GetValueTextByVal(BOOL bVal) const
{
    if (bVal) return ImpGetResStr(STR_ItemValON);
    else return ImpGetResStr(STR_ItemValOFF);
}

SfxItemPresentation __EXPORT SdrOnOffItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByVal(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

#ifdef SDR_ISPOOLABLE
int __EXPORT SdrOnOffItem::IsPoolable() const
{
    USHORT nId=Which();
    return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
}
#endif

TYPEINIT1_AUTOFACTORY(SdrYesNoItem,SfxBoolItem);

SfxPoolItem* __EXPORT SdrYesNoItem::Clone(SfxItemPool* pPool) const
{
    return new SdrYesNoItem(Which(),GetValue());
}

SfxPoolItem* __EXPORT SdrYesNoItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SdrYesNoItem(Which(),rIn);
}

XubString __EXPORT SdrYesNoItem::GetValueTextByVal(BOOL bVal) const
{
    if (bVal) return ImpGetResStr(STR_ItemValYES);
    else return ImpGetResStr(STR_ItemValNO);
}

SfxItemPresentation __EXPORT SdrYesNoItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByVal(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

#ifdef SDR_ISPOOLABLE
int __EXPORT SdrYesNoItem::IsPoolable() const
{
    USHORT nId=Which();
    return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
}
#endif

//------------------------------------------------------------
// class SdrPercentItem
//------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SdrPercentItem,SfxUInt16Item);

SfxPoolItem* __EXPORT SdrPercentItem::Clone(SfxItemPool* pPool) const
{
    return new SdrPercentItem(Which(),GetValue());
}

SfxPoolItem* __EXPORT SdrPercentItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SdrPercentItem(Which(),rIn);
}

SfxItemPresentation __EXPORT SdrPercentItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric,
    SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText = UniString::CreateFromInt32(GetValue());
    rText += sal_Unicode('%');

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        XubString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }

    return ePres;
}

#ifdef SDR_ISPOOLABLE
int __EXPORT SdrPercentItem::IsPoolable() const
{
    USHORT nId=Which();
    return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
}
#endif

//------------------------------------------------------------
// class SdrAngleItem
//------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SdrAngleItem,SfxInt32Item);

SfxPoolItem* __EXPORT SdrAngleItem::Clone(SfxItemPool* pPool) const
{
    return new SdrAngleItem(Which(),GetValue());
}

SfxPoolItem* __EXPORT SdrAngleItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SdrAngleItem(Which(),rIn);
}

SfxItemPresentation __EXPORT SdrAngleItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
    XubString& rText, const International * pInternational) const
{
    INT32 nValue(GetValue());
    BOOL bNeg(nValue < 0);

    if(bNeg)
        nValue = -nValue;

    rText = UniString::CreateFromInt32(nValue);

    if(nValue)
    {
        sal_Unicode aUnicodeNull('0');
        xub_StrLen nAnz(2);

        if(!pInternational)
            pInternational = &GetpApp()->GetAppInternational();

        if(pInternational->IsNumLeadingZero())
            nAnz++;

        while(rText.Len() < nAnz)
            rText.Insert(aUnicodeNull, 0);

        xub_StrLen nLen = rText.Len();
        BOOL bNull1(rText.GetChar(nLen-1) == aUnicodeNull);
        BOOL bNull2(bNull1 && rText.GetChar(nLen-2) == aUnicodeNull);

        if(bNull2)
        {
            // keine Nachkommastellen
            rText.Erase(nLen-2);
        }
        else
        {
            sal_Unicode cDec = pInternational->GetNumDecimalSep();
            rText.Insert(cDec, nLen-2);

            if(bNull1)
                rText.Erase(nLen);
        }

        if(bNeg)
            rText.Insert(sal_Unicode('-'), 0);
    }

    rText += sal_Unicode(DEGREE_CHAR);

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        XubString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }

    return ePres;
}

#ifdef SDR_ISPOOLABLE
int __EXPORT SdrAngleItem::IsPoolable() const
{
    USHORT nId=Which();
    return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
}
#endif

//------------------------------------------------------------
// class SdrMetricItem
//------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SdrMetricItem,SfxInt32Item);

SfxPoolItem* __EXPORT SdrMetricItem::Clone(SfxItemPool* pPool) const
{
    return new SdrMetricItem(Which(),GetValue());
}

SfxPoolItem* __EXPORT SdrMetricItem::Create(SvStream& rIn, USHORT nVer) const
{
    return new SdrMetricItem(Which(),rIn);
}

FASTBOOL __EXPORT SdrMetricItem::HasMetrics() const
{
    return TRUE;
}

FASTBOOL __EXPORT SdrMetricItem::ScaleMetrics(long nMul, long nDiv)
{
    if (GetValue()!=0) {
        BigInt aVal(GetValue());
        aVal*=nMul;
        aVal+=nDiv/2; // fuer korrektes Runden
        aVal/=nDiv;
        SetValue(long(aVal));
    }
    return TRUE;
}

SfxItemPresentation __EXPORT SdrMetricItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    long nValue=GetValue();
    SdrFormatter aFmt((MapUnit)eCoreMetric,(MapUnit)ePresMetric);
    aFmt.TakeStr(nValue,rText);
    String aStr;
    aFmt.TakeUnitStr((MapUnit)ePresMetric,aStr);
    rText+=aStr;
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

#ifdef SDR_ISPOOLABLE
int __EXPORT SdrMetricItem::IsPoolable() const
{
    USHORT nId=Which();
    return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ShadowSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(SdrShadowSetItem,SfxSetItem);

SdrShadowSetItem::SdrShadowSetItem(SfxItemSet* pItemSet):
    SfxSetItem(SDRATTRSET_SHADOW,pItemSet)
{
}

SdrShadowSetItem::SdrShadowSetItem(SfxItemPool* pItemPool):
    SfxSetItem(SDRATTRSET_SHADOW,new SfxItemSet(*pItemPool,SDRATTR_SHADOW_FIRST,SDRATTR_SHADOW_LAST))
{
}

SfxPoolItem* __EXPORT SdrShadowSetItem::Clone(SfxItemPool* pToPool) const
{
    return new SdrShadowSetItem(*this,pToPool);
}

SfxPoolItem* __EXPORT SdrShadowSetItem::Create(SvStream& rStream, USHORT nVersion) const
{
    SfxItemSet *pSet=new SfxItemSet(*GetItemSet().GetPool(),SDRATTR_SHADOW_FIRST,SDRATTR_SHADOW_LAST);
    pSet->Load(rStream);
    return new SdrShadowSetItem(pSet);
}

////////////////////////////////////////////////////////////////////////////////
// Items des Legendenobjekts
////////////////////////////////////////////////////////////////////////////////

TYPEINIT1_AUTOFACTORY(SdrCaptionTypeItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrCaptionTypeItem::Clone(SfxItemPool* pPool) const                { return new SdrCaptionTypeItem(*this); }

SfxPoolItem* __EXPORT SdrCaptionTypeItem::Create(SvStream& rIn, USHORT nVer) const       { return new SdrCaptionTypeItem(rIn); }

USHORT __EXPORT SdrCaptionTypeItem::GetValueCount() const { return 4; }

XubString __EXPORT SdrCaptionTypeItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValCAPTIONTYPE1+nPos);
}

SfxItemPresentation __EXPORT SdrCaptionTypeItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

TYPEINIT1_AUTOFACTORY(SdrCaptionEscDirItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrCaptionEscDirItem::Clone(SfxItemPool* pPool) const              { return new SdrCaptionEscDirItem(*this); }

SfxPoolItem* __EXPORT SdrCaptionEscDirItem::Create(SvStream& rIn, USHORT nVer) const     { return new SdrCaptionEscDirItem(rIn); }

USHORT __EXPORT SdrCaptionEscDirItem::GetValueCount() const { return 3; }

XubString __EXPORT SdrCaptionEscDirItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValCAPTIONESCHORI+nPos);
}

SfxItemPresentation __EXPORT SdrCaptionEscDirItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CaptionSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(SdrCaptionSetItem,SfxSetItem);

SfxPoolItem* __EXPORT SdrCaptionSetItem::Clone(SfxItemPool* pToPool) const
{
    return new SdrCaptionSetItem(*this,pToPool);
}

SfxPoolItem* __EXPORT SdrCaptionSetItem::Create(SvStream& rStream, USHORT nVersion) const
{
    SfxItemSet *pSet=new SfxItemSet(*GetItemSet().GetPool(),SDRATTR_CAPTION_FIRST,SDRATTR_CAPTION_LAST);
    pSet->Load(rStream);
    return new SdrCaptionSetItem(pSet);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OutlinerSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(SdrOutlinerSetItem,SfxSetItem);

SdrOutlinerSetItem::SdrOutlinerSetItem(SfxItemPool* pItemPool):
    SfxSetItem(SDRATTRSET_OUTLINER,new SfxItemSet(*pItemPool,EE_ITEMS_START,EE_ITEMS_END))
{
}

SfxPoolItem* __EXPORT SdrOutlinerSetItem::Clone(SfxItemPool* pToPool) const
{
    return new SdrOutlinerSetItem(*this,pToPool);
}

SfxPoolItem* __EXPORT SdrOutlinerSetItem::Create(SvStream& rStream, USHORT nVersion) const
{
    SfxItemSet *pSet=new SfxItemSet(*GetItemSet().GetPool(),EE_ITEMS_START,EE_ITEMS_END);
    pSet->Load(rStream);
    return new SdrOutlinerSetItem(pSet);
}

////////////////////////////////////////////////////////////////////////////////
// MiscItems
////////////////////////////////////////////////////////////////////////////////

// FitToSize
TYPEINIT1_AUTOFACTORY(SdrTextFitToSizeTypeItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrTextFitToSizeTypeItem::Clone(SfxItemPool* pPool) const         { return new SdrTextFitToSizeTypeItem(*this); }

SfxPoolItem* __EXPORT SdrTextFitToSizeTypeItem::Create(SvStream& rIn, USHORT nVer) const { return new SdrTextFitToSizeTypeItem(rIn); }

USHORT __EXPORT SdrTextFitToSizeTypeItem::GetValueCount() const { return 4; }

XubString __EXPORT SdrTextFitToSizeTypeItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValFITTOSIZENONE+nPos);
}

SfxItemPresentation __EXPORT SdrTextFitToSizeTypeItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

int  __EXPORT SdrTextFitToSizeTypeItem::HasBoolValue() const { return TRUE; }

BOOL __EXPORT SdrTextFitToSizeTypeItem::GetBoolValue() const { return GetValue()!=SDRTEXTFIT_NONE; }

void __EXPORT SdrTextFitToSizeTypeItem::SetBoolValue(BOOL bVal) { SetValue(bVal ? SDRTEXTFIT_PROPORTIONAL : SDRTEXTFIT_NONE); }

sal_Bool SdrTextFitToSizeTypeItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    drawing::TextFitToSizeType eFS = (drawing::TextFitToSizeType)GetValue();
    rVal <<= eFS;

    return sal_True;
}

sal_Bool SdrTextFitToSizeTypeItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextFitToSizeType eFS;
    if(!(rVal >>= eFS))
        return sal_False;

    SetValue( (SdrFitToSizeType)eFS );

    return sal_True;
}

TYPEINIT1_AUTOFACTORY(SdrTextVertAdjustItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrTextVertAdjustItem::Clone(SfxItemPool* pPool) const            { return new SdrTextVertAdjustItem(*this); }

SfxPoolItem* __EXPORT SdrTextVertAdjustItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextVertAdjustItem(rIn); }

USHORT __EXPORT SdrTextVertAdjustItem::GetValueCount() const { return 5; }

XubString __EXPORT SdrTextVertAdjustItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTVADJTOP+nPos);
}

SfxItemPresentation __EXPORT SdrTextVertAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrTextVertAdjustItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::TextAdjust)GetValue();
    return sal_True;
}

sal_Bool SdrTextVertAdjustItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextAdjust eAdj;
    if(!(rVal >>= eAdj))
        return sal_False;

    SetValue( (SdrTextVertAdjust)eAdj );

    return sal_True;
}

TYPEINIT1_AUTOFACTORY(SdrTextHorzAdjustItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrTextHorzAdjustItem::Clone(SfxItemPool* pPool) const            { return new SdrTextHorzAdjustItem(*this); }

SfxPoolItem* __EXPORT SdrTextHorzAdjustItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextHorzAdjustItem(rIn); }

USHORT __EXPORT SdrTextHorzAdjustItem::GetValueCount() const { return 5; }

XubString __EXPORT SdrTextHorzAdjustItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTHADJLEFT+nPos);
}

SfxItemPresentation __EXPORT SdrTextHorzAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrTextHorzAdjustItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::TextAdjust)GetValue();
    return sal_True;
}

sal_Bool SdrTextHorzAdjustItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextAdjust eAdj;
    if(!(rVal >>= eAdj))
        return sal_False;

    SetValue( (SdrTextHorzAdjust)eAdj );

    return sal_True;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniKindItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrTextAniKindItem::Clone(SfxItemPool* pPool) const            { return new SdrTextAniKindItem(*this); }

SfxPoolItem* __EXPORT SdrTextAniKindItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniKindItem(rIn); }

USHORT __EXPORT SdrTextAniKindItem::GetValueCount() const { return 5; }

XubString __EXPORT SdrTextAniKindItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTANI_NONE+nPos);
}

SfxItemPresentation __EXPORT SdrTextAniKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrTextAniKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::TextAnimationKind)GetValue();
    return sal_True;
}

sal_Bool SdrTextAniKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextAnimationKind eKind;
    if(!(rVal >>= eKind))
        return sal_False;

    SetValue( (SdrTextAniKind)eKind );

    return sal_True;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniDirectionItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrTextAniDirectionItem::Clone(SfxItemPool* pPool) const            { return new SdrTextAniDirectionItem(*this); }

SfxPoolItem* __EXPORT SdrTextAniDirectionItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniDirectionItem(rIn); }

USHORT __EXPORT SdrTextAniDirectionItem::GetValueCount() const { return 4; }

XubString __EXPORT SdrTextAniDirectionItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTANI_LEFT+nPos);
}

SfxItemPresentation __EXPORT SdrTextAniDirectionItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrTextAniDirectionItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::TextAnimationDirection)GetValue();
    return sal_True;
}

sal_Bool SdrTextAniDirectionItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextAnimationDirection eDir;
    if(!(rVal >>= eDir))
        return sal_False;

    SetValue( (SdrTextAniDirection)eDir );

    return sal_True;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniDelayItem,SfxUInt16Item);

SfxPoolItem* __EXPORT SdrTextAniDelayItem::Clone(SfxItemPool* pPool) const            { return new SdrTextAniDelayItem(*this); }

SfxPoolItem* __EXPORT SdrTextAniDelayItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniDelayItem(rIn); }

SfxItemPresentation __EXPORT SdrTextAniDelayItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
    XubString& rText, const International *) const
{
    rText = UniString::CreateFromInt32(GetValue());
    rText += sal_Unicode('m');
    rText += sal_Unicode('s');

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        XubString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }

    return ePres;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniAmountItem,SfxInt16Item);

SfxPoolItem* __EXPORT SdrTextAniAmountItem::Clone(SfxItemPool* pPool) const            { return new SdrTextAniAmountItem(*this); }

SfxPoolItem* __EXPORT SdrTextAniAmountItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniAmountItem(rIn); }

FASTBOOL __EXPORT SdrTextAniAmountItem::HasMetrics() const
{
    return GetValue()>0;
}

FASTBOOL __EXPORT SdrTextAniAmountItem::ScaleMetrics(long nMul, long nDiv)
{
    if (GetValue()>0) {
        BigInt aVal(GetValue());
        aVal*=nMul;
        aVal+=nDiv/2; // fuer korrektes Runden
        aVal/=nDiv;
        SetValue(short(aVal));
        return TRUE;
    } else return FALSE;
}

SfxItemPresentation __EXPORT SdrTextAniAmountItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
    XubString& rText, const International *) const
{
    INT32 nValue(GetValue());

    if(!nValue)
        nValue = -1L;

    if(nValue < 0)
    {
        sal_Char aText[] = "pixel";

        rText = UniString::CreateFromInt32(-nValue);
        rText += UniString(aText, sizeof(aText-1));
    }
    else
    {
        SdrFormatter aFmt((MapUnit)eCoreMetric, (MapUnit)ePresMetric);
        XubString aStr;

        aFmt.TakeStr(nValue, rText);
        aFmt.TakeUnitStr((MapUnit)ePresMetric, aStr);
        rText += aStr;
    }

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        XubString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }

    return ePres;
}

TYPEINIT1_AUTOFACTORY( SdrAutoShapeAdjustmentItem, SfxPoolItem );

SdrAutoShapeAdjustmentItem::SdrAutoShapeAdjustmentItem() : SfxPoolItem( SDRATTR_AUTOSHAPE_ADJUSTMENT )
{
}

SdrAutoShapeAdjustmentItem::SdrAutoShapeAdjustmentItem( SvStream& rIn, sal_uInt16 nVersion ):
    SfxPoolItem( SDRATTR_AUTOSHAPE_ADJUSTMENT )
{
    if ( nVersion )
    {
        SdrAutoShapeAdjustmentValue aVal;
        sal_uInt32 i, nCount;
        rIn >> nCount;
        for ( i = 0; i < nCount; i++ )
        {
            rIn >> aVal.nValue;
            SetValue( i, aVal );
        }
    }
}

SdrAutoShapeAdjustmentItem::~SdrAutoShapeAdjustmentItem()
{
    void* pPtr;
    for ( pPtr = aAdjustmentValueList.First(); pPtr; pPtr = aAdjustmentValueList.Next() )
        delete (SdrAutoShapeAdjustmentValue*)pPtr;
}

int __EXPORT SdrAutoShapeAdjustmentItem::operator==( const SfxPoolItem& rCmp ) const
{
    int bRet = SfxPoolItem::operator==( rCmp );
    if ( bRet )
    {
        bRet = ((SdrAutoShapeAdjustmentItem&)rCmp).GetCount() == GetCount();
        if ( bRet )
        {
            sal_uInt32 i;
            for ( i = 0; i < GetCount(); i++ )
            {
                if ( ((SdrAutoShapeAdjustmentItem&)rCmp).GetValue( i ).nValue != GetValue( i ).nValue )
                {
                    bRet = 0;
                    break;
                }
            }
        }
    }
    return bRet;
}

SfxItemPresentation __EXPORT SdrAutoShapeAdjustmentItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit eCoreMetric,
    SfxMapUnit ePresentationMetric, XubString &rText, const International *) const
{
    sal_uInt32 i, nCount = GetCount();
    rText.Append( UniString::CreateFromInt32( nCount ) );
    for ( i = 0; i < nCount; i++ )
    {
        rText += sal_Unicode( ' ' );
        rText.Append( UniString::CreateFromInt32( GetValue( i ).nValue ) );
    }
    if ( ePresentation == SFX_ITEM_PRESENTATION_COMPLETE )
    {
        XubString aStr;

        SdrItemPool::TakeItemName( Which(), aStr );
        aStr += sal_Unicode( ' ' );
        rText.Insert( aStr, 0 );
    }
    return ePresentation;
}

SfxPoolItem* __EXPORT SdrAutoShapeAdjustmentItem::Create( SvStream& rIn, sal_uInt16 nItemVersion ) const
{
    return new SdrAutoShapeAdjustmentItem( rIn, nItemVersion );
}

SvStream& __EXPORT SdrAutoShapeAdjustmentItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    if ( nItemVersion )
    {
        sal_uInt32 i, nCount = GetCount();
        rOut << nCount;
        for ( i = 0; i < nCount; i++ )
            rOut << GetValue( i ).nValue;
    }
    return rOut;
}

SfxPoolItem* __EXPORT SdrAutoShapeAdjustmentItem::Clone( SfxItemPool *pPool ) const
{
    sal_uInt32 i;
    SdrAutoShapeAdjustmentItem* pItem = new SdrAutoShapeAdjustmentItem;
    for ( i = 0; i < GetCount(); i++ )
    {
        const SdrAutoShapeAdjustmentValue& rVal = GetValue( i );
        pItem->SetValue( i, rVal );
    }
    return pItem;
}

#ifdef SDR_ISPOOLABLE
int __EXPORT SdrAutoShapeAdjustmentItem::IsPoolable() const
{
    USHORT nId=Which();
    return nId < SDRATTR_NOTPERSIST_FIRST || nId > SDRATTR_NOTPERSIST_LAST;
}
#endif

const SdrAutoShapeAdjustmentValue& SdrAutoShapeAdjustmentItem::GetValue( sal_uInt32 nIndex ) const
{
#ifdef DBG_UTIL
    if ( aAdjustmentValueList.Count() <= nIndex )
        DBG_ERROR( "SdrAutoShapeAdjustemntItem::GetValue - nIndex out of range (SJ)" );
#endif
    return *(SdrAutoShapeAdjustmentValue*)aAdjustmentValueList.GetObject( nIndex );
}

void SdrAutoShapeAdjustmentItem::SetValue( sal_uInt32 nIndex, const SdrAutoShapeAdjustmentValue& rVal )
{
    sal_uInt32 i;
    for ( i = GetCount(); i <= nIndex; i++ )
    {
        SdrAutoShapeAdjustmentValue* pItem = new SdrAutoShapeAdjustmentValue;
        aAdjustmentValueList.Insert( pItem, LIST_APPEND );
    }
    SdrAutoShapeAdjustmentValue& rValue = *(SdrAutoShapeAdjustmentValue*)aAdjustmentValueList.GetObject( nIndex );
    rValue.nValue = rVal.nValue;
}

sal_uInt16 SdrAutoShapeAdjustmentItem::GetVersion( sal_uInt16 nFileFormatVersion ) const
{
    return 1;
}

sal_Bool SdrAutoShapeAdjustmentItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    sal_uInt32 i, nCount = GetCount();
    uno::Sequence< sal_uInt32 > aSequence( nCount );
    if ( nCount )
    {
        sal_uInt32* pPtr = aSequence.getArray();
        for ( i = 0; i < nCount; i++ )
            *pPtr++ = GetValue( i ).nValue;
    }
    rVal <<= aSequence;
    return sal_True;
}

sal_Bool SdrAutoShapeAdjustmentItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    uno::Sequence< sal_uInt32 > aSequence;
    if( !( rVal >>= aSequence ) )
        return sal_False;

    void* pPtr;
    for ( pPtr = aAdjustmentValueList.First(); pPtr; pPtr = aAdjustmentValueList.Next() )
        delete (SdrAutoShapeAdjustmentValue*)pPtr;

    sal_uInt32 i, nCount = aSequence.getLength();
    if ( nCount )
    {
        const sal_uInt32* pPtr = aSequence.getConstArray();
        for ( i = 0; i < nCount; i++ )
        {
            SdrAutoShapeAdjustmentValue* pItem = new SdrAutoShapeAdjustmentValue;
            pItem->nValue = *pPtr++;
            aAdjustmentValueList.Insert( pItem, LIST_APPEND );
        }
    }
    return sal_True;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrMiscSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(SdrMiscSetItem,SfxSetItem);

SfxPoolItem* __EXPORT SdrMiscSetItem::Clone(SfxItemPool* pToPool) const
{
    return new SdrMiscSetItem(*this,pToPool);
}

SfxPoolItem* __EXPORT SdrMiscSetItem::Create(SvStream& rStream, USHORT nVersion) const
{
    SfxItemSet *pSet=new SfxItemSet(*GetItemSet().GetPool(),SDRATTR_MISC_FIRST,SDRATTR_MISC_LAST);
    pSet->Load(rStream);
    return new SdrMiscSetItem(pSet);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Edge
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// EdgeKind
TYPEINIT1_AUTOFACTORY(SdrEdgeKindItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrEdgeKindItem::Clone(SfxItemPool* pPool) const            { return new SdrEdgeKindItem(*this); }

SfxPoolItem* __EXPORT SdrEdgeKindItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrEdgeKindItem(rIn); }

USHORT __EXPORT SdrEdgeKindItem::GetValueCount() const { return 4; }

XubString __EXPORT SdrEdgeKindItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValEDGE_ORTHOLINES+nPos);
}

SfxItemPresentation __EXPORT SdrEdgeKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrEdgeKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    drawing::ConnectorType eCT = drawing::ConnectorType_STANDARD;

    switch( GetValue() )
    {
        case SDREDGE_ORTHOLINES :   eCT = drawing::ConnectorType_STANDARD; break;
        case SDREDGE_THREELINES :   eCT = drawing::ConnectorType_LINES; break;
        case SDREDGE_ONELINE :      eCT = drawing::ConnectorType_LINE;  break;
        case SDREDGE_BEZIER :       eCT = drawing::ConnectorType_CURVE; break;
        case SDREDGE_ARC :          eCT = drawing::ConnectorType_CURVE; break;
        default:
            DBG_ERROR( "SdrEdgeKindItem::QueryValue : unknown enum" );
    }

    rVal <<= eCT;

    return sal_True;
}

sal_Bool SdrEdgeKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::ConnectorType eCT;
    if(!(rVal >>= eCT))
        return sal_False;

    SdrEdgeKind eEK = SDREDGE_ORTHOLINES;
    switch( eCT )
    {
        case drawing::ConnectorType_STANDARD :  eEK = SDREDGE_ORTHOLINES;   break;
        case drawing::ConnectorType_CURVE :     eEK = SDREDGE_BEZIER;       break;
        case drawing::ConnectorType_LINE :       eEK = SDREDGE_ONELINE;     break;
        case drawing::ConnectorType_LINES :      eEK = SDREDGE_THREELINES;   break;
        default:
            DBG_ERROR( "SdrEdgeKindItem::PuValue : unknown enum" );
    }
    SetValue( eEK );

    return sal_True;
}

sal_Bool SdrEdgeNode1HorzDistItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

sal_Bool SdrEdgeNode1HorzDistItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    if(!(rVal >>= nValue))
        return sal_False;

    SetValue( nValue );
    return sal_True;
}

BOOL SdrEdgeNode1VertDistItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

BOOL SdrEdgeNode1VertDistItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    if(!(rVal >>= nValue))
        return sal_False;

    SetValue( nValue );
    return sal_True;
}

BOOL SdrEdgeNode2HorzDistItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

BOOL SdrEdgeNode2HorzDistItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    if(!(rVal >>= nValue))
        return sal_False;

    SetValue( nValue );
    return sal_True;
}

BOOL SdrEdgeNode2VertDistItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

BOOL SdrEdgeNode2VertDistItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    if(!(rVal >>= nValue))
        return sal_False;

    SetValue( nValue );
    return sal_True;
}

BOOL SdrEdgeLine1DeltaItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

BOOL SdrEdgeLine1DeltaItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    if(!(rVal >>= nValue))
        return sal_False;

    SetValue( nValue );
    return sal_True;
}

BOOL SdrEdgeLine2DeltaItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

BOOL SdrEdgeLine2DeltaItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    if(!(rVal >>= nValue))
        return sal_False;

    SetValue( nValue );
    return sal_True;
}

BOOL SdrEdgeLine3DeltaItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

BOOL SdrEdgeLine3DeltaItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nValue;
    if(!(rVal >>= nValue))
        return sal_False;

    SetValue( nValue );
    return sal_True;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrEdgeSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(SdrEdgeSetItem,SfxSetItem);

SfxPoolItem* __EXPORT SdrEdgeSetItem::Clone(SfxItemPool* pToPool) const
{
    return new SdrEdgeSetItem(*this,pToPool);
}

SfxPoolItem* __EXPORT SdrEdgeSetItem::Create(SvStream& rStream, USHORT nVersion) const
{
    SfxItemSet *pSet=new SfxItemSet(*GetItemSet().GetPool(),SDRATTR_EDGE_FIRST,SDRATTR_EDGE_LAST);
    pSet->Load(rStream);
    return new SdrEdgeSetItem(pSet);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Measure
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// MeasureKind
TYPEINIT1_AUTOFACTORY(SdrMeasureKindItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrMeasureKindItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureKindItem(*this); }

SfxPoolItem* __EXPORT SdrMeasureKindItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureKindItem(rIn); }

USHORT __EXPORT SdrMeasureKindItem::GetValueCount() const { return 2; }

XubString __EXPORT SdrMeasureKindItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValMEASURE_STD+nPos);
}

SfxItemPresentation __EXPORT SdrMeasureKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrMeasureKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::MeasureKind)GetValue();
    return sal_True;
}

sal_Bool SdrMeasureKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::MeasureKind eKind;
    if(!(rVal >>= eKind))
        return sal_False;

    SetValue( (SdrMeasureKind)eKind );
    return sal_True;
}

TYPEINIT1_AUTOFACTORY(SdrMeasureTextHPosItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrMeasureTextHPosItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureTextHPosItem(*this); }

SfxPoolItem* __EXPORT SdrMeasureTextHPosItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureTextHPosItem(rIn); }

USHORT __EXPORT SdrMeasureTextHPosItem::GetValueCount() const { return 4; }

XubString __EXPORT SdrMeasureTextHPosItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValMEASURE_TEXTHAUTO+nPos);
}

SfxItemPresentation __EXPORT SdrMeasureTextHPosItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrMeasureTextHPosItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::MeasureTextHorzPos)GetValue();
    return sal_True;
}

sal_Bool SdrMeasureTextHPosItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::MeasureTextHorzPos ePos;
    if(!(rVal >>= ePos))
        return sal_False;

    SetValue( (SdrMeasureTextHPos)ePos );
    return sal_True;
}

TYPEINIT1_AUTOFACTORY(SdrMeasureTextVPosItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrMeasureTextVPosItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureTextVPosItem(*this); }

SfxPoolItem* __EXPORT SdrMeasureTextVPosItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureTextVPosItem(rIn); }

USHORT __EXPORT SdrMeasureTextVPosItem::GetValueCount() const { return 5; }

XubString __EXPORT SdrMeasureTextVPosItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValMEASURE_TEXTVAUTO+nPos);
}

SfxItemPresentation __EXPORT SdrMeasureTextVPosItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrMeasureTextVPosItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::MeasureTextVertPos)GetValue();
    return sal_True;
}

sal_Bool SdrMeasureTextVPosItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::MeasureTextVertPos ePos;
    if(!(rVal >>= ePos))
        return sal_False;

    SetValue( (SdrMeasureTextHPos)ePos );
    return sal_True;
}

TYPEINIT1_AUTOFACTORY(SdrMeasureUnitItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrMeasureUnitItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureUnitItem(*this); }

SfxPoolItem* __EXPORT SdrMeasureUnitItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureUnitItem(rIn); }

USHORT __EXPORT SdrMeasureUnitItem::GetValueCount() const { return 14; }

XubString __EXPORT SdrMeasureUnitItem::GetValueTextByPos(USHORT nPos) const
{
    XubString aRetval;

    if((FieldUnit)nPos == FUNIT_NONE)
    {
        sal_Char aText[] = "default";
        aRetval += UniString(aText, sizeof(aText-1));
    }
    else
    {
        SdrFormatter::TakeUnitStr((FieldUnit)nPos, aRetval);
    }

    return aRetval;
}

SfxItemPresentation __EXPORT SdrMeasureUnitItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrMeasureUnitItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (sal_Int32)GetValue();
    return sal_True;
}

sal_Bool SdrMeasureUnitItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    sal_Int32 nMeasure;
    if(!(rVal >>= nMeasure))
        return sal_False;

    SetValue( (FieldUnit)nMeasure );
    return sal_True;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrMeasureSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(SdrMeasureSetItem,SfxSetItem);

SfxPoolItem* __EXPORT SdrMeasureSetItem::Clone(SfxItemPool* pToPool) const
{
    return new SdrMeasureSetItem(*this,pToPool);
}

SfxPoolItem* __EXPORT SdrMeasureSetItem::Create(SvStream& rStream, USHORT nVersion) const
{
    SfxItemSet *pSet=new SfxItemSet(*GetItemSet().GetPool(),SDRATTR_MEASURE_FIRST,SDRATTR_MEASURE_LAST);
    pSet->Load(rStream);
    return new SdrMeasureSetItem(pSet);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Circ
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// CircKind
TYPEINIT1_AUTOFACTORY(SdrCircKindItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrCircKindItem::Clone(SfxItemPool* pPool) const          { return new SdrCircKindItem(*this); }

SfxPoolItem* __EXPORT SdrCircKindItem::Create(SvStream& rIn, USHORT nVer) const { return new SdrCircKindItem(rIn); }

USHORT __EXPORT SdrCircKindItem::GetValueCount() const { return 4; }

XubString __EXPORT SdrCircKindItem::GetValueTextByPos(USHORT nPos) const
{
    return ImpGetResStr(STR_ItemValCIRC_FULL+nPos);
}

SfxItemPresentation __EXPORT SdrCircKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const International *) const
{
    rText=GetValueTextByPos(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

sal_Bool SdrCircKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::CircleKind)GetValue();
    return sal_True;
}

sal_Bool SdrCircKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::CircleKind eKind;
    if(!(rVal >>= eKind))
        return sal_False;

    SetValue( (SdrCircKind)eKind );
    return sal_True;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrCircSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1(SdrCircSetItem,SfxSetItem);

SfxPoolItem* __EXPORT SdrCircSetItem::Clone(SfxItemPool* pToPool) const
{
    return new SdrCircSetItem(*this,pToPool);
}

SfxPoolItem* __EXPORT SdrCircSetItem::Create(SvStream& rStream, USHORT nVersion) const
{
    SfxItemSet *pSet=new SfxItemSet(*GetItemSet().GetPool(),SDRATTR_CIRC_FIRST,SDRATTR_CIRC_LAST);
    pSet->Load(rStream);
    return new SdrCircSetItem(pSet);
}

//------------------------------------------------------------
// class SdrSignedPercentItem
//------------------------------------------------------------

TYPEINIT1_AUTOFACTORY( SdrSignedPercentItem, SfxInt16Item );


SfxPoolItem* __EXPORT SdrSignedPercentItem::Clone(SfxItemPool* pPool) const
{
    return new SdrSignedPercentItem( Which(), GetValue() );
}

SfxPoolItem* __EXPORT SdrSignedPercentItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrSignedPercentItem( Which(), rIn );
}

SfxItemPresentation __EXPORT SdrSignedPercentItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
    XubString& rText, const International *) const
{
    rText = UniString::CreateFromInt32(GetValue());
    rText += sal_Unicode('%');

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        XubString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }

    return ePres;
}

#ifdef SDR_ISPOOLABLE

int __EXPORT SdrSignedPercentItem::IsPoolable() const
{
    USHORT nId=Which();
    return nId < SDRATTR_NOTPERSIST_FIRST || nId > SDRATTR_NOTPERSIST_LAST;
}
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafSetItem,SfxSetItem );

SfxPoolItem* __EXPORT SdrGrafSetItem::Clone( SfxItemPool* pToPool ) const
{
    return new SdrGrafSetItem( *this, pToPool );
}

SfxPoolItem* __EXPORT SdrGrafSetItem::Create( SvStream& rStream, USHORT nVersion ) const
{
    SfxItemSet* pSet = new SfxItemSet( *GetItemSet().GetPool(), SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST );
    pSet->Load( rStream );
    return new SdrGrafSetItem( pSet );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafRedItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafRedItem, SdrSignedPercentItem );

SfxPoolItem* __EXPORT SdrGrafRedItem::Clone( SfxItemPool* pPool ) const
{
    return new SdrGrafRedItem( *this );
}

SfxPoolItem* __EXPORT SdrGrafRedItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrGrafRedItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafGreenItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafGreenItem, SdrSignedPercentItem );

SfxPoolItem* __EXPORT SdrGrafGreenItem::Clone( SfxItemPool* pPool ) const
{
    return new SdrGrafGreenItem( *this );
}

SfxPoolItem* __EXPORT SdrGrafGreenItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrGrafGreenItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafBlueItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafBlueItem, SdrSignedPercentItem );

SfxPoolItem* __EXPORT SdrGrafBlueItem::Clone( SfxItemPool* pPool ) const
{
    return new SdrGrafBlueItem( *this );
}

SfxPoolItem* __EXPORT SdrGrafBlueItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrGrafBlueItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafLuminanceItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafLuminanceItem, SdrSignedPercentItem );

SfxPoolItem* __EXPORT SdrGrafLuminanceItem::Clone( SfxItemPool* pPool ) const
{
    return new SdrGrafLuminanceItem( *this );
}

SfxPoolItem* __EXPORT SdrGrafLuminanceItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrGrafLuminanceItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafContrastItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafContrastItem, SdrSignedPercentItem );

SfxPoolItem* __EXPORT SdrGrafContrastItem::Clone( SfxItemPool* pPool ) const
{
    return new SdrGrafContrastItem( *this );
}

SfxPoolItem* __EXPORT SdrGrafContrastItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrGrafContrastItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafGamma100Item
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafGamma100Item, SfxUInt32Item );

SfxPoolItem* __EXPORT SdrGrafGamma100Item::Clone( SfxItemPool* pPool ) const
{
    return new SdrGrafGamma100Item( *this );
}

SfxPoolItem* __EXPORT SdrGrafGamma100Item::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrGrafGamma100Item( rIn );
}

sal_Bool SdrGrafGamma100Item::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= ((double)GetValue()) / 100.0;
    return sal_True;
}

sal_Bool SdrGrafGamma100Item::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    double nGamma;
    if(!(rVal >>= nGamma))
        return sal_False;

    SetValue( (UINT32)(nGamma * 100.0  ) );
    return sal_True;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafInvertItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafInvertItem, SdrOnOffItem );

SfxPoolItem* __EXPORT SdrGrafInvertItem::Clone( SfxItemPool* pPool ) const
{
    return new SdrGrafInvertItem( *this );
}

SfxPoolItem* __EXPORT SdrGrafInvertItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrGrafInvertItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafTransparenceItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafTransparenceItem, SdrPercentItem );

SfxPoolItem* __EXPORT SdrGrafTransparenceItem::Clone( SfxItemPool* pPool ) const
{
    return new SdrGrafTransparenceItem( *this );
}

SfxPoolItem* __EXPORT SdrGrafTransparenceItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrGrafTransparenceItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafModeItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafModeItem, SfxEnumItem );

SfxPoolItem* __EXPORT SdrGrafModeItem::Clone(SfxItemPool* pPool) const
{
    return new SdrGrafModeItem( *this );
}

SfxPoolItem* __EXPORT SdrGrafModeItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return new SdrGrafModeItem( rIn );
}

USHORT __EXPORT SdrGrafModeItem::GetValueCount() const
{
    return 4;
}

XubString __EXPORT SdrGrafModeItem::GetValueTextByPos(UINT16 nPos) const
{
    XubString aStr;

    switch(nPos)
    {
        case 1:
        {
            sal_Char aTextGreys[] = "Greys";
            aStr += UniString(aTextGreys, sizeof(aTextGreys-1));
            break;
        }
        case 2:
        {
            sal_Char aTextBlackWhite[] = "Black/White";
            aStr += UniString(aTextBlackWhite, sizeof(aTextBlackWhite-1));
            break;
        }
        case 3:
        {
            sal_Char aTextWatermark[] = "Watermark";
            aStr += UniString(aTextWatermark, sizeof(aTextWatermark-1));
            break;
        }
        default:
        {
            sal_Char aTextStandard[] = "Standard";
            aStr += UniString(aTextStandard, sizeof(aTextStandard-1));
            break;
        }
    }

    return aStr;
}

SfxItemPresentation __EXPORT SdrGrafModeItem::GetPresentation( SfxItemPresentation ePres,
                                                               SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
                                                               XubString& rText, const International *) const
{
    rText = GetValueTextByPos( GetValue() );

    if( ePres == SFX_ITEM_PRESENTATION_COMPLETE )
    {
        String aStr;

        SdrItemPool::TakeItemName( Which(), aStr );
        aStr += sal_Unicode(' ');
        rText.Insert( aStr, 0 );
    }

    return ePres;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafCropItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafCropItem, SvxGrfCrop );

SfxPoolItem* SdrGrafCropItem::Clone( SfxItemPool* pPool ) const
{
    return new SdrGrafCropItem( *this );
}

SfxPoolItem* SdrGrafCropItem::Create( SvStream& rIn, USHORT nVer ) const
{
    return( ( 0 == nVer ) ? Clone( NULL ) : SvxGrfCrop::Create( rIn, nVer ) );
}

USHORT SdrGrafCropItem::GetVersion( USHORT nFileVersion ) const
{
    // GRFCROP_VERSION_MOVETOSVX is 1
    return GRFCROP_VERSION_MOVETOSVX;
}
