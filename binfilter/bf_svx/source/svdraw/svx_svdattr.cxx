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

#define ITEMID_GRF_CROP 0

#ifndef _COM_SUN_STAR_DRAWING_TEXTFITTOSIZETYPE_HPP_
#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTHORIZONTALADJUST_HPP_
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#endif
#ifndef _COM_SUN_STAR_DRAWING_TEXTVERTICALADJUST_HPP_
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
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

#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>

#ifndef _APP_HXX //autogen
#include <vcl/svapp.hxx>
#endif

#ifndef _XDEF_HXX
#include "xdef.hxx"
#endif

#include "svdattr.hxx"
#include "svdattrx.hxx"
#include "svdpool.hxx"

#ifndef _SOLAR_HRC
#include <bf_svtools/solar.hrc>
#endif

#ifndef _SVX_XMLCNITM_HXX
#include "xmlcnitm.hxx"
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
#include "writingmodeitem.hxx"

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

#ifndef _SVX3DITEMS_HXX
#include <svx3ditems.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002 
#endif
namespace binfilter {

using namespace ::rtl;
using namespace ::com::sun::star;

/*************************************************************************
|*
|* Konstruktor
|*
\************************************************************************/

/*N*/ SdrItemPool::SdrItemPool(USHORT nAttrStart, USHORT nAttrEnd, FASTBOOL bLoadRefCounts):
/*N*/ 	XOutdevItemPool(nAttrStart,nAttrEnd,bLoadRefCounts)
/*N*/ {
/*N*/ 	Ctor(NULL,nAttrStart,nAttrEnd);
/*N*/ }

/*N*/ SdrItemPool::SdrItemPool(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd, FASTBOOL bLoadRefCounts):
/*N*/ 	XOutdevItemPool(pMaster,nAttrStart,nAttrEnd,bLoadRefCounts)
/*N*/ {
/*N*/ 	Ctor(pMaster,nAttrStart,nAttrEnd);
/*N*/ }

/*N*/ void SdrItemPool::Ctor(SfxItemPool* pMaster, USHORT nAttrStart, USHORT nAttrEnd)
/*N*/ {
/*N*/ 	if (pMaster==NULL) {
/*N*/ 		pMaster=this;
/*N*/ 	}
/*N*/ 
/*N*/ 	Color aNullCol(RGB_Color(COL_BLACK));
/*N*/ 	XubString aEmptyStr;
/*N*/ 
/*N*/ 	USHORT i;
/*N*/ 	for (i=SDRATTR_NOTPERSIST_FIRST; i<=SDRATTR_NOTPERSIST_LAST; i++) {
/*N*/ 		pItemInfos[i-SDRATTR_START]._nFlags=0;
/*N*/ 	}
/*N*/ 	// Schatten
/*N*/ 	ppPoolDefaults[SDRATTR_SHADOW            -SDRATTR_START]=new SdrShadowItem;
/*N*/ 	ppPoolDefaults[SDRATTR_SHADOWCOLOR       -SDRATTR_START]=new SdrShadowColorItem(aEmptyStr,aNullCol);
/*N*/ 	ppPoolDefaults[SDRATTR_SHADOWXDIST       -SDRATTR_START]=new SdrShadowXDistItem;
/*N*/ 	ppPoolDefaults[SDRATTR_SHADOWYDIST       -SDRATTR_START]=new SdrShadowYDistItem;
/*N*/ 	ppPoolDefaults[SDRATTR_SHADOWTRANSPARENCE-SDRATTR_START]=new SdrShadowTransparenceItem;
/*N*/ 	ppPoolDefaults[SDRATTR_SHADOW3D          -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOW3D    );
/*N*/ 	ppPoolDefaults[SDRATTR_SHADOWPERSP       -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOWPERSP );
/*N*/ 	for (i=SDRATTR_SHADOWRESERVE1; i<=SDRATTR_SHADOWRESERVE5; i++) {
/*N*/ 		ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
/*N*/ 	}
/*N*/ 
/*N*/ 	ppPoolDefaults[SDRATTRSET_SHADOW-SDRATTR_START]=new SdrShadowSetItem(pMaster);
/*N*/ 
/*N*/ 	// SID_ATTR_FILL_SHADOW = SID_SVX_START+299 = SID_LIB_START+299 = 10299
/*N*/ 	pItemInfos[SDRATTR_SHADOW-SDRATTR_START]._nSID=SID_ATTR_FILL_SHADOW;
/*N*/ 
/*N*/ 	// Legendenobjekt
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONTYPE      -SDRATTR_START]=new SdrCaptionTypeItem      ;
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONFIXEDANGLE-SDRATTR_START]=new SdrCaptionFixedAngleItem;
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONANGLE     -SDRATTR_START]=new SdrCaptionAngleItem     ;
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONGAP       -SDRATTR_START]=new SdrCaptionGapItem       ;
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONESCDIR    -SDRATTR_START]=new SdrCaptionEscDirItem    ;
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONESCISREL  -SDRATTR_START]=new SdrCaptionEscIsRelItem  ;
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONESCREL    -SDRATTR_START]=new SdrCaptionEscRelItem    ;
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONESCABS    -SDRATTR_START]=new SdrCaptionEscAbsItem    ;
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONLINELEN   -SDRATTR_START]=new SdrCaptionLineLenItem   ;
/*N*/ 	ppPoolDefaults[SDRATTR_CAPTIONFITLINELEN-SDRATTR_START]=new SdrCaptionFitLineLenItem;
/*N*/ 	for (i=SDRATTR_CAPTIONRESERVE1; i<=SDRATTR_CAPTIONRESERVE5; i++) {
/*N*/ 		ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
/*N*/ 	}
/*N*/ 	ppPoolDefaults[SDRATTRSET_CAPTION-SDRATTR_START]=new SdrCaptionSetItem(pMaster);
/*N*/ 
/*N*/ 	// Outliner-Attribute
/*N*/ 	ppPoolDefaults[SDRATTRSET_OUTLINER-SDRATTR_START]=new SdrOutlinerSetItem(pMaster);
/*N*/ 
/*N*/ 	// Misc-Attribute
/*N*/ 	ppPoolDefaults[SDRATTR_ECKENRADIUS			-SDRATTR_START]=new SdrEckenradiusItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_MINFRAMEHEIGHT	-SDRATTR_START]=new SdrTextMinFrameHeightItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_AUTOGROWHEIGHT	-SDRATTR_START]=new SdrTextAutoGrowHeightItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_FITTOSIZE		-SDRATTR_START]=new SdrTextFitToSizeTypeItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_LEFTDIST		-SDRATTR_START]=new SdrTextLeftDistItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_RIGHTDIST		-SDRATTR_START]=new SdrTextRightDistItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_UPPERDIST		-SDRATTR_START]=new SdrTextUpperDistItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_LOWERDIST		-SDRATTR_START]=new SdrTextLowerDistItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_VERTADJUST		-SDRATTR_START]=new SdrTextVertAdjustItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_MAXFRAMEHEIGHT	-SDRATTR_START]=new SdrTextMaxFrameHeightItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_MINFRAMEWIDTH	-SDRATTR_START]=new SdrTextMinFrameWidthItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_MAXFRAMEWIDTH	-SDRATTR_START]=new SdrTextMaxFrameWidthItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_AUTOGROWWIDTH	-SDRATTR_START]=new SdrTextAutoGrowWidthItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_HORZADJUST		-SDRATTR_START]=new SdrTextHorzAdjustItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_ANIKIND			-SDRATTR_START]=new SdrTextAniKindItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_ANIDIRECTION	-SDRATTR_START]=new SdrTextAniDirectionItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_ANISTARTINSIDE	-SDRATTR_START]=new SdrTextAniStartInsideItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_ANISTOPINSIDE	-SDRATTR_START]=new SdrTextAniStopInsideItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_ANICOUNT		-SDRATTR_START]=new SdrTextAniCountItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_ANIDELAY		-SDRATTR_START]=new SdrTextAniDelayItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_ANIAMOUNT		-SDRATTR_START]=new SdrTextAniAmountItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXT_CONTOURFRAME	-SDRATTR_START]=new SdrTextContourFrameItem;
/*N*/ 	ppPoolDefaults[SDRATTR_AUTOSHAPE_ADJUSTMENT -SDRATTR_START]=new SdrAutoShapeAdjustmentItem;
/*N*/ #ifndef SVX_LIGHT
/*N*/ 	ppPoolDefaults[SDRATTR_XMLATTRIBUTES -SDRATTR_START]=new SvXMLAttrContainerItem( SDRATTR_XMLATTRIBUTES );
/*N*/ #else
/*N*/     // no need to have alien attributes persistent in the player
/*N*/ 	ppPoolDefaults[SDRATTR_XMLATTRIBUTES -SDRATTR_START]=new SfxVoidItem( SDRATTR_XMLATTRIBUTES );
/*N*/ #endif // #ifndef SVX_LIGHT
/*N*/ 	for (i=SDRATTR_RESERVE15; i<=SDRATTR_RESERVE19; i++) {
/*N*/ 		ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
/*N*/ 	}
/*N*/ 	ppPoolDefaults[SDRATTRSET_MISC-SDRATTR_START]=new SdrMiscSetItem(pMaster);
/*N*/ 
/*N*/ 	pItemInfos[SDRATTR_TEXT_FITTOSIZE-SDRATTR_START]._nSID=SID_ATTR_TEXT_FITTOSIZE;
/*N*/ 
/*N*/ 	// Objektverbinder
/*N*/ 	ppPoolDefaults[SDRATTR_EDGEKIND         -SDRATTR_START]=new SdrEdgeKindItem;
/*N*/ 	long nDefEdgeDist=500; // Erstmal hart defaulted fuer Draw (100TH_MM). hier muss noch der MapMode beruecksichtigt werden.
/*N*/ 	ppPoolDefaults[SDRATTR_EDGENODE1HORZDIST-SDRATTR_START]=new SdrEdgeNode1HorzDistItem(nDefEdgeDist);
/*N*/ 	ppPoolDefaults[SDRATTR_EDGENODE1VERTDIST-SDRATTR_START]=new SdrEdgeNode1VertDistItem(nDefEdgeDist);
/*N*/ 	ppPoolDefaults[SDRATTR_EDGENODE2HORZDIST-SDRATTR_START]=new SdrEdgeNode2HorzDistItem(nDefEdgeDist);
/*N*/ 	ppPoolDefaults[SDRATTR_EDGENODE2VERTDIST-SDRATTR_START]=new SdrEdgeNode2VertDistItem(nDefEdgeDist);
/*N*/ 	ppPoolDefaults[SDRATTR_EDGENODE1GLUEDIST-SDRATTR_START]=new SdrEdgeNode1GlueDistItem;
/*N*/ 	ppPoolDefaults[SDRATTR_EDGENODE2GLUEDIST-SDRATTR_START]=new SdrEdgeNode2GlueDistItem;
/*N*/ 	ppPoolDefaults[SDRATTR_EDGELINEDELTAANZ -SDRATTR_START]=new SdrEdgeLineDeltaAnzItem;
/*N*/ 	ppPoolDefaults[SDRATTR_EDGELINE1DELTA   -SDRATTR_START]=new SdrEdgeLine1DeltaItem;
/*N*/ 	ppPoolDefaults[SDRATTR_EDGELINE2DELTA   -SDRATTR_START]=new SdrEdgeLine2DeltaItem;
/*N*/ 	ppPoolDefaults[SDRATTR_EDGELINE3DELTA   -SDRATTR_START]=new SdrEdgeLine3DeltaItem;
/*N*/ 	for (i=SDRATTR_EDGERESERVE02; i<=SDRATTR_EDGERESERVE09; i++) {
/*N*/ 		ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
/*N*/ 	}
/*N*/ 	ppPoolDefaults[SDRATTRSET_EDGE-SDRATTR_START]=new SdrEdgeSetItem(pMaster);
/*N*/ 
/*N*/ 	 // Bemassungsobjekt
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREKIND             -SDRATTR_START]=new SdrMeasureKindItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURETEXTHPOS         -SDRATTR_START]=new SdrMeasureTextHPosItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURETEXTVPOS         -SDRATTR_START]=new SdrMeasureTextVPosItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURELINEDIST         -SDRATTR_START]=new SdrMeasureLineDistItem(800);
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREHELPLINEOVERHANG -SDRATTR_START]=new SdrMeasureHelplineOverhangItem(200);
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREHELPLINEDIST     -SDRATTR_START]=new SdrMeasureHelplineDistItem(100);
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREHELPLINE1LEN     -SDRATTR_START]=new SdrMeasureHelpline1LenItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREHELPLINE2LEN     -SDRATTR_START]=new SdrMeasureHelpline2LenItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREBELOWREFEDGE     -SDRATTR_START]=new SdrMeasureBelowRefEdgeItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURETEXTROTA90       -SDRATTR_START]=new SdrMeasureTextRota90Item;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURETEXTUPSIDEDOWN   -SDRATTR_START]=new SdrMeasureTextUpsideDownItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREOVERHANG         -SDRATTR_START]=new SdrMeasureOverhangItem(600);
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREUNIT             -SDRATTR_START]=new SdrMeasureUnitItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURESCALE            -SDRATTR_START]=new SdrMeasureScaleItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURESHOWUNIT         -SDRATTR_START]=new SdrMeasureShowUnitItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREFORMATSTRING     -SDRATTR_START]=new SdrMeasureFormatStringItem();
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLE    -SDRATTR_START]=new SdrMeasureTextAutoAngleItem();
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLEVIEW-SDRATTR_START]=new SdrMeasureTextAutoAngleViewItem();
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURETEXTISFIXEDANGLE -SDRATTR_START]=new SdrMeasureTextIsFixedAngleItem();
/*N*/ 	ppPoolDefaults[SDRATTR_MEASURETEXTFIXEDANGLE   -SDRATTR_START]=new SdrMeasureTextFixedAngleItem();
/*N*/ 	ppPoolDefaults[SDRATTR_MEASUREDECIMALPLACES    -SDRATTR_START]=new SdrMeasureDecimalPlacesItem();
/*N*/ 	for (i=SDRATTR_MEASURERESERVE05; i<=SDRATTR_MEASURERESERVE07; i++) {
/*N*/ 		ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
/*N*/ 	}
/*N*/ 	ppPoolDefaults[SDRATTRSET_MEASURE-SDRATTR_START]=new SdrMeasureSetItem(pMaster);
/*N*/ 
/*N*/ 	 // Kreis
/*N*/ 	ppPoolDefaults[SDRATTR_CIRCKIND      -SDRATTR_START]=new SdrCircKindItem;
/*N*/ 	ppPoolDefaults[SDRATTR_CIRCSTARTANGLE-SDRATTR_START]=new SdrCircStartAngleItem;
/*N*/ 	ppPoolDefaults[SDRATTR_CIRCENDANGLE  -SDRATTR_START]=new SdrCircEndAngleItem;
/*N*/ 	for (i=SDRATTR_CIRCRESERVE0; i<=SDRATTR_CIRCRESERVE3; i++) {
/*N*/ 		ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
/*N*/ 	}
/*N*/ 	ppPoolDefaults[SDRATTRSET_CIRC-SDRATTR_START]=new SdrCircSetItem(pMaster);
/*N*/ 
/*N*/ 	 // Nichtpersistente-Items
/*N*/ 	ppPoolDefaults[SDRATTR_OBJMOVEPROTECT -SDRATTR_START]=new SdrObjMoveProtectItem;
/*N*/ 	ppPoolDefaults[SDRATTR_OBJSIZEPROTECT -SDRATTR_START]=new SdrObjSizeProtectItem;
/*N*/ 	ppPoolDefaults[SDRATTR_OBJPRINTABLE   -SDRATTR_START]=new SdrObjPrintableItem;
/*N*/ 	ppPoolDefaults[SDRATTR_LAYERID        -SDRATTR_START]=new SdrLayerIdItem;
/*N*/ 	ppPoolDefaults[SDRATTR_LAYERNAME      -SDRATTR_START]=new SdrLayerNameItem;
/*N*/ 	ppPoolDefaults[SDRATTR_OBJECTNAME     -SDRATTR_START]=new SdrObjectNameItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ALLPOSITIONX   -SDRATTR_START]=new SdrAllPositionXItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ALLPOSITIONY   -SDRATTR_START]=new SdrAllPositionYItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ALLSIZEWIDTH   -SDRATTR_START]=new SdrAllSizeWidthItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ALLSIZEHEIGHT  -SDRATTR_START]=new SdrAllSizeHeightItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ONEPOSITIONX   -SDRATTR_START]=new SdrOnePositionXItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ONEPOSITIONY   -SDRATTR_START]=new SdrOnePositionYItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ONESIZEWIDTH   -SDRATTR_START]=new SdrOneSizeWidthItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ONESIZEHEIGHT  -SDRATTR_START]=new SdrOneSizeHeightItem;
/*N*/ 	ppPoolDefaults[SDRATTR_LOGICSIZEWIDTH -SDRATTR_START]=new SdrLogicSizeWidthItem;
/*N*/ 	ppPoolDefaults[SDRATTR_LOGICSIZEHEIGHT-SDRATTR_START]=new SdrLogicSizeHeightItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ROTATEANGLE    -SDRATTR_START]=new SdrRotateAngleItem;
/*N*/ 	ppPoolDefaults[SDRATTR_SHEARANGLE     -SDRATTR_START]=new SdrShearAngleItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MOVEX          -SDRATTR_START]=new SdrMoveXItem;
/*N*/ 	ppPoolDefaults[SDRATTR_MOVEY          -SDRATTR_START]=new SdrMoveYItem;
/*N*/ 	ppPoolDefaults[SDRATTR_RESIZEXONE     -SDRATTR_START]=new SdrResizeXOneItem;
/*N*/ 	ppPoolDefaults[SDRATTR_RESIZEYONE     -SDRATTR_START]=new SdrResizeYOneItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ROTATEONE      -SDRATTR_START]=new SdrRotateOneItem;
/*N*/ 	ppPoolDefaults[SDRATTR_HORZSHEARONE   -SDRATTR_START]=new SdrHorzShearOneItem;
/*N*/ 	ppPoolDefaults[SDRATTR_VERTSHEARONE   -SDRATTR_START]=new SdrVertShearOneItem;
/*N*/ 	ppPoolDefaults[SDRATTR_RESIZEXALL     -SDRATTR_START]=new SdrResizeXAllItem;
/*N*/ 	ppPoolDefaults[SDRATTR_RESIZEYALL     -SDRATTR_START]=new SdrResizeYAllItem;
/*N*/ 	ppPoolDefaults[SDRATTR_ROTATEALL      -SDRATTR_START]=new SdrRotateAllItem;
/*N*/ 	ppPoolDefaults[SDRATTR_HORZSHEARALL   -SDRATTR_START]=new SdrHorzShearAllItem;
/*N*/ 	ppPoolDefaults[SDRATTR_VERTSHEARALL   -SDRATTR_START]=new SdrVertShearAllItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TRANSFORMREF1X -SDRATTR_START]=new SdrTransformRef1XItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TRANSFORMREF1Y -SDRATTR_START]=new SdrTransformRef1YItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TRANSFORMREF2X -SDRATTR_START]=new SdrTransformRef2XItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TRANSFORMREF2Y -SDRATTR_START]=new SdrTransformRef2YItem;
/*N*/ 	ppPoolDefaults[SDRATTR_TEXTDIRECTION  -SDRATTR_START]=new SvxWritingModeItem;
/*N*/ 
/*N*/ 	for (i=SDRATTR_NOTPERSISTRESERVE2; i<=SDRATTR_NOTPERSISTRESERVE15; i++) {
/*N*/ 		ppPoolDefaults[i-SDRATTR_START]=new SfxVoidItem(i);
/*N*/ 	}
/*N*/ 
/*N*/ 	// Grafik
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFRED				- SDRATTR_START] = new SdrGrafRedItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFGREEN			- SDRATTR_START] = new SdrGrafGreenItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFBLUE			- SDRATTR_START] = new SdrGrafBlueItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFLUMINANCE		- SDRATTR_START] = new SdrGrafLuminanceItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFCONTRAST		- SDRATTR_START] = new SdrGrafContrastItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFGAMMA			- SDRATTR_START] = new SdrGrafGamma100Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFTRANSPARENCE	- SDRATTR_START] = new SdrGrafTransparenceItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFINVERT			- SDRATTR_START] = new SdrGrafInvertItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFMODE			- SDRATTR_START] = new SdrGrafModeItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_GRAFCROP			- SDRATTR_START] = new SdrGrafCropItem;
/*N*/ 	for( i = SDRATTR_GRAFRESERVE3; i <= SDRATTR_GRAFRESERVE6; i++ )
/*N*/ 		ppPoolDefaults[ i - SDRATTR_START ] = new SfxVoidItem( i );
/*N*/ 	ppPoolDefaults[ SDRATTRSET_GRAF - SDRATTR_START ] = new SdrGrafSetItem( pMaster );
/*N*/ 	pItemInfos[SDRATTR_GRAFCROP-SDRATTR_START]._nSID=SID_ATTR_GRAF_CROP;
/*N*/ 
/*N*/ 	// 3D Object Attr (28092000 AW)
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_PERCENT_DIAGONAL - SDRATTR_START ] = new Svx3DPercentDiagonalItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_BACKSCALE - SDRATTR_START ] = new Svx3DBackscaleItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_DEPTH - SDRATTR_START ] = new Svx3DDepthItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_HORZ_SEGS - SDRATTR_START ] = new Svx3DHorizontalSegmentsItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_VERT_SEGS - SDRATTR_START ] = new Svx3DVerticalSegmentsItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_END_ANGLE - SDRATTR_START ] = new Svx3DEndAngleItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_DOUBLE_SIDED - SDRATTR_START ] = new Svx3DDoubleSidedItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_NORMALS_KIND - SDRATTR_START ] = new Svx3DNormalsKindItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_NORMALS_INVERT - SDRATTR_START ] = new Svx3DNormalsInvertItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_X - SDRATTR_START ] = new Svx3DTextureProjectionXItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_Y - SDRATTR_START ] = new Svx3DTextureProjectionYItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_SHADOW_3D - SDRATTR_START ] = new Svx3DShadow3DItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_MAT_COLOR - SDRATTR_START ] = new Svx3DMaterialColorItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_MAT_EMISSION - SDRATTR_START ] = new Svx3DMaterialEmissionItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR - SDRATTR_START ] = new Svx3DMaterialSpecularItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY - SDRATTR_START ] = new Svx3DMaterialSpecularIntensityItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_KIND - SDRATTR_START ] = new Svx3DTextureKindItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_MODE - SDRATTR_START ] = new Svx3DTextureModeItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_FILTER - SDRATTR_START ] = new Svx3DTextureFilterItem;
/*N*/ 
/*N*/ 	// #107245# Add new items for 3d objects
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_NORMALS - SDRATTR_START ] = new Svx3DSmoothNormalsItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_LIDS - SDRATTR_START ] = new Svx3DSmoothLidsItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_CHARACTER_MODE - SDRATTR_START ] = new Svx3DCharacterModeItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_CLOSE_FRONT - SDRATTR_START ] = new Svx3DCloseFrontItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DOBJ_CLOSE_BACK - SDRATTR_START ] = new Svx3DCloseBackItem;
/*N*/ 
/*N*/ 	// #107245# Start with SDRATTR_3DOBJ_RESERVED_06 now
/*N*/ 	for( i = SDRATTR_3DOBJ_RESERVED_06; i <= SDRATTR_3DOBJ_RESERVED_20; i++ )
/*N*/ 		ppPoolDefaults[ i - SDRATTR_START ] = new SfxVoidItem( i );
/*N*/ 
/*N*/ 	// 3D Scene Attr (28092000 AW)
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_PERSPECTIVE - SDRATTR_START ] = new Svx3DPerspectiveItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_DISTANCE - SDRATTR_START ] = new Svx3DDistanceItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_FOCAL_LENGTH - SDRATTR_START ] = new Svx3DFocalLengthItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_TWO_SIDED_LIGHTING - SDRATTR_START ] = new Svx3DTwoSidedLightingItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_1 - SDRATTR_START ] = new Svx3DLightcolor1Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_2 - SDRATTR_START ] = new Svx3DLightcolor2Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_3 - SDRATTR_START ] = new Svx3DLightcolor3Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_4 - SDRATTR_START ] = new Svx3DLightcolor4Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_5 - SDRATTR_START ] = new Svx3DLightcolor5Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_6 - SDRATTR_START ] = new Svx3DLightcolor6Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_7 - SDRATTR_START ] = new Svx3DLightcolor7Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_8 - SDRATTR_START ] = new Svx3DLightcolor8Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_AMBIENTCOLOR - SDRATTR_START ] = new Svx3DAmbientcolorItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_1 - SDRATTR_START ] = new Svx3DLightOnOff1Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_2 - SDRATTR_START ] = new Svx3DLightOnOff2Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_3 - SDRATTR_START ] = new Svx3DLightOnOff3Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_4 - SDRATTR_START ] = new Svx3DLightOnOff4Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_5 - SDRATTR_START ] = new Svx3DLightOnOff5Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_6 - SDRATTR_START ] = new Svx3DLightOnOff6Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_7 - SDRATTR_START ] = new Svx3DLightOnOff7Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_8 - SDRATTR_START ] = new Svx3DLightOnOff8Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_1 - SDRATTR_START ] = new Svx3DLightDirection1Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_2 - SDRATTR_START ] = new Svx3DLightDirection2Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_3 - SDRATTR_START ] = new Svx3DLightDirection3Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_4 - SDRATTR_START ] = new Svx3DLightDirection4Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_5 - SDRATTR_START ] = new Svx3DLightDirection5Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_6 - SDRATTR_START ] = new Svx3DLightDirection6Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_7 - SDRATTR_START ] = new Svx3DLightDirection7Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_8 - SDRATTR_START ] = new Svx3DLightDirection8Item;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_SHADOW_SLANT - SDRATTR_START ] = new Svx3DShadowSlantItem;
/*N*/ 	ppPoolDefaults[ SDRATTR_3DSCENE_SHADE_MODE - SDRATTR_START ] = new Svx3DShadeModeItem;
/*N*/ 
/*N*/ 	for( i = SDRATTR_3DSCENE_RESERVED_01; i <= SDRATTR_3DSCENE_RESERVED_20; i++ )
/*N*/ 		ppPoolDefaults[ i - SDRATTR_START ] = new SfxVoidItem( i );
/*N*/ 
/*N*/ 	////////////////////////////////
/*N*/ #ifdef DBG_UTIL
/*N*/ 	UINT16 nAnz(SDRATTR_END-SDRATTR_START + 1);
/*N*/ 
/*N*/ 	for(UINT16 nNum = 0; nNum < nAnz; nNum++)
/*N*/ 	{
/*N*/ 		const SfxPoolItem* pItem = ppPoolDefaults[nNum];
/*N*/ 
/*N*/ 		if(!pItem)
/*N*/ 		{
/*?*/ 			ByteString aStr("PoolDefaultItem not set: ");
/*?*/ 
/*?*/ 			aStr += "Num=";
/*?*/ 			aStr += ByteString::CreateFromInt32( nNum );
/*?*/ 			aStr += "Which=";
/*?*/ 			aStr += ByteString::CreateFromInt32( nNum + 1000 );
/*?*/ 
/*?*/ 			DBG_ERROR(aStr.GetBuffer());
/*N*/ 		}
/*N*/ 		else if(pItem->Which() != nNum + 1000)
/*N*/ 		{
/*?*/ 			ByteString aStr("PoolDefaultItem has wrong WhichId: ");
/*?*/ 
/*?*/ 			aStr += "Num=";
/*?*/ 			aStr += ByteString::CreateFromInt32( nNum );
/*?*/ 			aStr += " Which=";
/*?*/ 			aStr += ByteString::CreateFromInt32( pItem->Which() );
/*?*/ 
/*?*/ 			DBG_ERROR(aStr.GetBuffer());
/*N*/ 		}
/*N*/ 	}
/*N*/ #endif
/*N*/ 
/*N*/ 	if (nAttrStart==SDRATTR_START && nAttrEnd==SDRATTR_END) {
/*N*/ 		SetDefaults(ppPoolDefaults);
/*N*/ 		SetItemInfos(pItemInfos);
/*N*/ 	}
/*N*/ }

/*************************************************************************
|*
|* copy ctor, sorgt dafuer, dass die static defaults gecloned werden
|*            (Parameter 2 = TRUE)
|*
\************************************************************************/

/*N*/ SdrItemPool::SdrItemPool(const SdrItemPool& rPool):
/*N*/ 	XOutdevItemPool(rPool)
/*N*/ {
/*N*/ }

/*************************************************************************
|*
|* Clone()
|*
\************************************************************************/

/*N*/ SfxItemPool* __EXPORT SdrItemPool::Clone() const
/*N*/ {
/*N*/ 	return new SdrItemPool(*this);
/*N*/ }

/*************************************************************************
|*
|* Destruktor
|*
\************************************************************************/

/*N*/ SdrItemPool::~SdrItemPool()
/*N*/ {
/*N*/ 	Delete(); // erstmal den 'dtor' des SfxItemPools rufen
/*N*/ 	// und nun meine eigenen statischen Defaults abraeumen
/*N*/ 	if (ppPoolDefaults!=NULL) {
/*N*/ 		unsigned nBeg=SDRATTR_SHADOW-SDRATTR_START;
/*N*/ 		unsigned nEnd=SDRATTR_END-SDRATTR_START;
/*N*/ 		for (unsigned i=nBeg; i<=nEnd; i++) {
/*N*/ 			SetRefCount(*ppPoolDefaults[i],0);
/*N*/ 			delete ppPoolDefaults[i];
/*N*/ 			ppPoolDefaults[i]=NULL;
/*N*/ 		}
/*N*/ 	}
/*N*/ 	// Vor dem zerstoeren die Pools ggf. voneinander trennen
/*N*/ 	SetSecondaryPool(NULL);
/*N*/ }




////////////////////////////////////////////////////////////////////////////////
// FractionItem
////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1_AUTOFACTORY(SdrFractionItem,SfxPoolItem);

/*N*/ SdrFractionItem::SdrFractionItem(USHORT nId, SvStream& rIn):
/*N*/ 	SfxPoolItem(nId)
/*N*/ {
/*N*/ 	INT32 nMul,nDiv;
/*N*/ 	rIn>>nMul;
/*N*/ 	rIn>>nDiv;
/*N*/ 	nValue=Fraction(nMul,nDiv);
/*N*/ }

/*N*/ int __EXPORT SdrFractionItem::operator==(const SfxPoolItem& rCmp) const
/*N*/ {
/*N*/ 	return SfxPoolItem::operator==(rCmp) &&
/*N*/ 		   ((SdrFractionItem&)rCmp).GetValue()==nValue;
/*N*/ }



/*?*/ SvStream& __EXPORT SdrFractionItem::Store(SvStream& rOut, USHORT nItemVers) const
/*?*/ {
/*?*/ 	rOut<<INT32(nValue.GetNumerator());
/*?*/ 	rOut<<INT32(nValue.GetDenominator());
/*?*/ 	return rOut;
/*?*/ }

/*N*/ SfxPoolItem* __EXPORT SdrFractionItem::Clone(SfxItemPool *pPool) const
/*N*/ {
/*N*/ 	return new SdrFractionItem(Which(),GetValue());
/*N*/ }

#ifdef SDR_ISPOOLABLE
/*?*/ int __EXPORT SdrFractionItem::IsPoolable() const
/*?*/ {
/*?*/ 	USHORT nId=Which();
/*?*/ 	return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
/*?*/ }
#endif

////////////////////////////////////////////////////////////////////////////////
// ScaleItem
////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1_AUTOFACTORY(SdrScaleItem,SdrFractionItem);


/*N*/ SfxPoolItem* __EXPORT SdrScaleItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new SdrScaleItem(Which(),rIn);
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrScaleItem::Clone(SfxItemPool *pPool) const
/*N*/ {
/*N*/ 	return new SdrScaleItem(Which(),GetValue());
/*N*/ }

////////////////////////////////////////////////////////////////////////////////
// OnOffItem
////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1_AUTOFACTORY(SdrOnOffItem,SfxBoolItem);

/*N*/ SfxPoolItem* __EXPORT SdrOnOffItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new SdrOnOffItem(Which(),GetValue());
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrOnOffItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new SdrOnOffItem(Which(),rIn);
/*N*/ }



#ifdef SDR_ISPOOLABLE
/*?*/ int __EXPORT SdrOnOffItem::IsPoolable() const
/*?*/ {
/*?*/ 	USHORT nId=Which();
/*?*/ 	return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
/*?*/ }
#endif

/*N*/ TYPEINIT1_AUTOFACTORY(SdrYesNoItem,SfxBoolItem);

/*N*/ SfxPoolItem* __EXPORT SdrYesNoItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new SdrYesNoItem(Which(),GetValue());
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrYesNoItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new SdrYesNoItem(Which(),rIn);
/*N*/ }



#ifdef SDR_ISPOOLABLE
/*?*/ int __EXPORT SdrYesNoItem::IsPoolable() const
/*?*/ {
/*?*/ 	USHORT nId=Which();
/*?*/ 	return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
/*?*/ }
#endif

//------------------------------------------------------------
// class SdrPercentItem
//------------------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY(SdrPercentItem,SfxUInt16Item);

/*N*/ SfxPoolItem* __EXPORT SdrPercentItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new SdrPercentItem(Which(),GetValue());
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrPercentItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new SdrPercentItem(Which(),rIn);
/*N*/ }


#ifdef SDR_ISPOOLABLE
/*?*/ int __EXPORT SdrPercentItem::IsPoolable() const
/*?*/ {
/*?*/ 	USHORT nId=Which();
/*?*/ 	return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
/*?*/ }
#endif

//------------------------------------------------------------
// class SdrAngleItem
//------------------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY(SdrAngleItem,SfxInt32Item);

/*N*/ SfxPoolItem* __EXPORT SdrAngleItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new SdrAngleItem(Which(),GetValue());
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrAngleItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new SdrAngleItem(Which(),rIn);
/*N*/ }


#ifdef SDR_ISPOOLABLE
/*?*/ int __EXPORT SdrAngleItem::IsPoolable() const
/*?*/ {
/*?*/ 	USHORT nId=Which();
/*?*/ 	return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
/*?*/ }
#endif

//------------------------------------------------------------
// class SdrMetricItem
//------------------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY(SdrMetricItem,SfxInt32Item);

/*N*/ SfxPoolItem* __EXPORT SdrMetricItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new SdrMetricItem(Which(),GetValue());
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrMetricItem::Create(SvStream& rIn, USHORT nVer) const
/*N*/ {
/*N*/ 	return new SdrMetricItem(Which(),rIn);
/*N*/ }




#ifdef SDR_ISPOOLABLE
/*?*/ int __EXPORT SdrMetricItem::IsPoolable() const
/*?*/ {
/*?*/ 	USHORT nId=Which();
/*?*/ 	return nId<SDRATTR_NOTPERSIST_FIRST || nId>SDRATTR_NOTPERSIST_LAST;
/*?*/ }
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// ShadowSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1(SdrShadowSetItem,SfxSetItem);

/*N*/ SdrShadowSetItem::SdrShadowSetItem(SfxItemSet* pItemSet):
/*N*/ 	SfxSetItem(SDRATTRSET_SHADOW,pItemSet)
/*N*/ {
/*N*/ }

/*N*/ SdrShadowSetItem::SdrShadowSetItem(SfxItemPool* pItemPool):
/*N*/ 	SfxSetItem(SDRATTRSET_SHADOW,new SfxItemSet(*pItemPool,SDRATTR_SHADOW_FIRST,SDRATTR_SHADOW_LAST))
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrShadowSetItem::Clone(SfxItemPool* pToPool) const
/*N*/ {
/*N*/ 	return new SdrShadowSetItem(*this,pToPool);
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrShadowSetItem::Create(SvStream& rStream, USHORT nVersion) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
/*N*/ 		SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST);
/*N*/ 	pSet->Load(rStream);
/*N*/ 	return new SdrShadowSetItem(pSet);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////
// Items des Legendenobjekts
////////////////////////////////////////////////////////////////////////////////

/*N*/ TYPEINIT1_AUTOFACTORY(SdrCaptionTypeItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrCaptionTypeItem::Clone(SfxItemPool* pPool) const                { return new SdrCaptionTypeItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrCaptionTypeItem::Create(SvStream& rIn, USHORT nVer) const       { return new SdrCaptionTypeItem(rIn); }

/*N*/ USHORT __EXPORT SdrCaptionTypeItem::GetValueCount() const { return 4; }



/*N*/ TYPEINIT1_AUTOFACTORY(SdrCaptionEscDirItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrCaptionEscDirItem::Clone(SfxItemPool* pPool) const              { return new SdrCaptionEscDirItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrCaptionEscDirItem::Create(SvStream& rIn, USHORT nVer) const     { return new SdrCaptionEscDirItem(rIn); }

/*N*/ USHORT __EXPORT SdrCaptionEscDirItem::GetValueCount() const { return 3; }



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// CaptionSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1(SdrCaptionSetItem,SfxSetItem);

/*N*/ SfxPoolItem* __EXPORT SdrCaptionSetItem::Clone(SfxItemPool* pToPool) const
/*N*/ {
/*N*/ 	return new SdrCaptionSetItem(*this,pToPool);
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrCaptionSetItem::Create(SvStream& rStream, USHORT nVersion) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
/*N*/ 		SDRATTR_CAPTION_FIRST, SDRATTR_CAPTION_LAST);
/*N*/ 	pSet->Load(rStream);
/*N*/ 	return new SdrCaptionSetItem(pSet);
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// OutlinerSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1(SdrOutlinerSetItem,SfxSetItem);

/*N*/ SdrOutlinerSetItem::SdrOutlinerSetItem(SfxItemPool* pItemPool):
/*N*/ 	SfxSetItem(SDRATTRSET_OUTLINER,new SfxItemSet(*pItemPool,EE_ITEMS_START,EE_ITEMS_END))
/*N*/ {
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrOutlinerSetItem::Clone(SfxItemPool* pToPool) const
/*N*/ {
/*N*/ 	return new SdrOutlinerSetItem(*this,pToPool);
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrOutlinerSetItem::Create(SvStream& rStream, USHORT nVersion) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
/*N*/ 		EE_ITEMS_START, EE_ITEMS_END);
/*N*/ 	pSet->Load(rStream);
/*N*/ 	return new SdrOutlinerSetItem(pSet);
/*N*/ }

////////////////////////////////////////////////////////////////////////////////
// MiscItems
////////////////////////////////////////////////////////////////////////////////

// FitToSize
/*N*/ TYPEINIT1_AUTOFACTORY(SdrTextFitToSizeTypeItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrTextFitToSizeTypeItem::Clone(SfxItemPool* pPool) const         { return new SdrTextFitToSizeTypeItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrTextFitToSizeTypeItem::Create(SvStream& rIn, USHORT nVer) const { return new SdrTextFitToSizeTypeItem(rIn); }

/*N*/ USHORT __EXPORT SdrTextFitToSizeTypeItem::GetValueCount() const { return 4; }






/*N*/ sal_Bool SdrTextFitToSizeTypeItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	drawing::TextFitToSizeType eFS = (drawing::TextFitToSizeType)GetValue();
/*N*/ 	rVal <<= eFS;
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool SdrTextFitToSizeTypeItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	drawing::TextFitToSizeType eFS;
/*N*/ 	if(!(rVal >>= eFS))
/*N*/ 	{
/*?*/ 		sal_Int32 nEnum;
/*?*/ 		if(!(rVal >>= nEnum))
/*?*/ 			return sal_False;
/*?*/ 
/*?*/ 		eFS = (drawing::TextFitToSizeType) nEnum;
/*N*/ 	}

/*N*/ 	SetValue( (SdrFitToSizeType)eFS );

/*N*/ 	return sal_True;
/*N*/ }

/*N*/ TYPEINIT1_AUTOFACTORY(SdrTextVertAdjustItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrTextVertAdjustItem::Clone(SfxItemPool* pPool) const            { return new SdrTextVertAdjustItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrTextVertAdjustItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextVertAdjustItem(rIn); }

/*N*/ USHORT __EXPORT SdrTextVertAdjustItem::GetValueCount() const { return 5; }



/*N*/ sal_Bool SdrTextVertAdjustItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::TextVerticalAdjust)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool SdrTextVertAdjustItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	drawing::TextVerticalAdjust eAdj;
/*N*/ 	if(!(rVal >>= eAdj))
/*N*/ 	{
/*?*/ 		sal_Int32 nEnum;
/*?*/ 		if(!(rVal >>= nEnum))
/*?*/ 			return sal_False;
/*?*/ 
/*?*/ 		eAdj = (drawing::TextVerticalAdjust)nEnum;
/*N*/ 	}
/*N*/ 
/*N*/ 	SetValue( (SdrTextVertAdjust)eAdj );
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ TYPEINIT1_AUTOFACTORY(SdrTextHorzAdjustItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrTextHorzAdjustItem::Clone(SfxItemPool* pPool) const            { return new SdrTextHorzAdjustItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrTextHorzAdjustItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextHorzAdjustItem(rIn); }

/*N*/ USHORT __EXPORT SdrTextHorzAdjustItem::GetValueCount() const { return 5; }

/*N*/ XubString __EXPORT SdrTextHorzAdjustItem::GetValueTextByPos(USHORT nPos) const
/*N*/ {DBG_BF_ASSERT(0, "STRIP");XubString a; return a; //STRIP001 
/*N*/ }


/*N*/ sal_Bool SdrTextHorzAdjustItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::TextHorizontalAdjust)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool SdrTextHorzAdjustItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	drawing::TextHorizontalAdjust eAdj;
/*N*/ 	if(!(rVal >>= eAdj))
/*N*/ 	{
/*?*/ 		sal_Int32 nEnum;
/*?*/ 		if(!(rVal >>= nEnum))
/*?*/ 			return sal_False;
/*?*/ 
/*?*/ 		eAdj = (drawing::TextHorizontalAdjust)nEnum;
/*N*/ 	}

/*N*/ 	SetValue( (SdrTextHorzAdjust)eAdj );

/*N*/ 	return sal_True;
/*N*/ }

/*N*/ TYPEINIT1_AUTOFACTORY(SdrTextAniKindItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrTextAniKindItem::Clone(SfxItemPool* pPool) const            { return new SdrTextAniKindItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrTextAniKindItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniKindItem(rIn); }

/*N*/ USHORT __EXPORT SdrTextAniKindItem::GetValueCount() const { return 5; }



/*N*/ sal_Bool SdrTextAniKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::TextAnimationKind)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool SdrTextAniKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	drawing::TextAnimationKind eKind;
/*N*/ 	if(!(rVal >>= eKind))
/*N*/ 	{
/*?*/ 		sal_Int32 nEnum;
/*?*/ 		if(!(rVal >>= nEnum))
/*?*/ 			return sal_False;
/*?*/ 		eKind = (drawing::TextAnimationKind)nEnum;
/*N*/ 	}
/*N*/ 
/*N*/ 	SetValue( (SdrTextAniKind)eKind );
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ TYPEINIT1_AUTOFACTORY(SdrTextAniDirectionItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrTextAniDirectionItem::Clone(SfxItemPool* pPool) const            { return new SdrTextAniDirectionItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrTextAniDirectionItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniDirectionItem(rIn); }

/*N*/ USHORT __EXPORT SdrTextAniDirectionItem::GetValueCount() const { return 4; }



/*N*/ sal_Bool SdrTextAniDirectionItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= (drawing::TextAnimationDirection)GetValue();
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool SdrTextAniDirectionItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	drawing::TextAnimationDirection eDir;
/*N*/ 	if(!(rVal >>= eDir))
/*N*/ 	{
/*?*/ 		sal_Int32 nEnum;
/*?*/ 		if(!(rVal >>= nEnum))
/*?*/ 			return sal_False;
/*?*/ 
/*?*/ 		eDir = (drawing::TextAnimationDirection)nEnum;
/*N*/ 	}
/*N*/ 
/*N*/ 	SetValue( (SdrTextAniDirection)eDir );
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ TYPEINIT1_AUTOFACTORY(SdrTextAniDelayItem,SfxUInt16Item);
/*N*/ 
/*N*/ SfxPoolItem* __EXPORT SdrTextAniDelayItem::Clone(SfxItemPool* pPool) const            { return new SdrTextAniDelayItem(*this); }
/*N*/ 
/*N*/ SfxPoolItem* __EXPORT SdrTextAniDelayItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniDelayItem(rIn); }


/*N*/ TYPEINIT1_AUTOFACTORY(SdrTextAniAmountItem,SfxInt16Item);
/*N*/ 
/*N*/ SfxPoolItem* __EXPORT SdrTextAniAmountItem::Clone(SfxItemPool* pPool) const            { return new SdrTextAniAmountItem(*this); }
/*N*/ 
/*N*/ SfxPoolItem* __EXPORT SdrTextAniAmountItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniAmountItem(rIn); }




/*N*/ TYPEINIT1_AUTOFACTORY( SdrAutoShapeAdjustmentItem, SfxPoolItem );

/*N*/ SdrAutoShapeAdjustmentItem::SdrAutoShapeAdjustmentItem() : SfxPoolItem( SDRATTR_AUTOSHAPE_ADJUSTMENT )
/*N*/ {
/*N*/ }

/*N*/ SdrAutoShapeAdjustmentItem::SdrAutoShapeAdjustmentItem( SvStream& rIn, sal_uInt16 nVersion ):
/*N*/ 	SfxPoolItem( SDRATTR_AUTOSHAPE_ADJUSTMENT )
/*N*/ {
/*N*/ 	if ( nVersion )
/*N*/ 	{
/*N*/ 		SdrAutoShapeAdjustmentValue aVal;
/*N*/ 		sal_uInt32 i, nCount;
/*N*/ 		rIn >> nCount;
/*N*/ 		for ( i = 0; i < nCount; i++ )
/*N*/ 		{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 		}
/*N*/ 	}
/*N*/ }

/*N*/ SdrAutoShapeAdjustmentItem::~SdrAutoShapeAdjustmentItem()
/*N*/ {
/*N*/ 	void* pPtr;
/*N*/ 	for ( pPtr = aAdjustmentValueList.First(); pPtr; pPtr = aAdjustmentValueList.Next() )
/*?*/ 		delete (SdrAutoShapeAdjustmentValue*)pPtr;
/*N*/ }

/*N*/ int __EXPORT SdrAutoShapeAdjustmentItem::operator==( const SfxPoolItem& rCmp ) const
/*N*/ {
/*N*/ 	int bRet = SfxPoolItem::operator==( rCmp );
/*N*/ 	if ( bRet )
/*N*/ 	{
/*N*/ 		bRet = ((SdrAutoShapeAdjustmentItem&)rCmp).GetCount() == GetCount();
/*N*/ 		if ( bRet )
/*N*/ 		{
/*N*/ 			sal_uInt32 i;
/*N*/ 			for ( i = 0; i < GetCount(); i++ )
/*N*/ 			{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 			}
/*N*/ 		}
/*N*/ 	}
/*N*/ 	return bRet;
/*N*/ }


/*N*/ SfxPoolItem* __EXPORT SdrAutoShapeAdjustmentItem::Create( SvStream& rIn, sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	return new SdrAutoShapeAdjustmentItem( rIn, nItemVersion );
/*N*/ }

/*N*/ SvStream& __EXPORT SdrAutoShapeAdjustmentItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
/*N*/ {
/*N*/ 	if ( nItemVersion )
/*N*/ 	{
/*N*/ 		sal_uInt32 i, nCount = GetCount();
/*N*/ 		rOut << nCount;
/*N*/ 		for ( i = 0; i < nCount; i++ )
/*?*/ 		{DBG_BF_ASSERT(0, "STRIP"); }//STRIP001 	rOut << GetValue( i ).nValue;
/*N*/ 	}
/*N*/ 	return rOut;
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrAutoShapeAdjustmentItem::Clone( SfxItemPool *pPool ) const
/*N*/ {
/*N*/ 	sal_uInt32 i;
/*N*/ 	SdrAutoShapeAdjustmentItem* pItem = new SdrAutoShapeAdjustmentItem;
/*N*/ 	for ( i = 0; i < GetCount(); i++ )
/*N*/ 	{DBG_BF_ASSERT(0, "STRIP"); //STRIP001 
/*N*/ 	}
/*N*/ 	return pItem;
/*N*/ }

#ifdef SDR_ISPOOLABLE
/*?*/ int __EXPORT SdrAutoShapeAdjustmentItem::IsPoolable() const
/*?*/ {
/*?*/ 	USHORT nId=Which();
/*?*/ 	return nId < SDRATTR_NOTPERSIST_FIRST || nId > SDRATTR_NOTPERSIST_LAST;
/*?*/ }
#endif



/*N*/ sal_uInt16 SdrAutoShapeAdjustmentItem::GetVersion( sal_uInt16 nFileFormatVersion ) const
/*N*/ {
/*N*/ 	return 1;
/*N*/ }




//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrMiscSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1(SdrMiscSetItem,SfxSetItem);

/*N*/ SfxPoolItem* __EXPORT SdrMiscSetItem::Clone(SfxItemPool* pToPool) const
/*N*/ {
/*N*/ 	return new SdrMiscSetItem(*this,pToPool);
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrMiscSetItem::Create(SvStream& rStream, USHORT nVersion) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
/*N*/ 		SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);
/*N*/ 	pSet->Load(rStream);
/*N*/ 	return new SdrMiscSetItem(pSet);
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Edge
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// EdgeKind
/*N*/ TYPEINIT1_AUTOFACTORY(SdrEdgeKindItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrEdgeKindItem::Clone(SfxItemPool* pPool) const            { return new SdrEdgeKindItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrEdgeKindItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrEdgeKindItem(rIn); }

/*N*/ USHORT __EXPORT SdrEdgeKindItem::GetValueCount() const { return 4; }



/*N*/ sal_Bool SdrEdgeKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	drawing::ConnectorType eCT = drawing::ConnectorType_STANDARD;
/*N*/ 
/*N*/ 	switch( GetValue() )
/*N*/ 	{
/*N*/ 		case SDREDGE_ORTHOLINES :   eCT = drawing::ConnectorType_STANDARD; break;
/*N*/ 		case SDREDGE_THREELINES :   eCT = drawing::ConnectorType_LINES;	break;
/*N*/ 		case SDREDGE_ONELINE :		eCT = drawing::ConnectorType_LINE;	break;
/*N*/ 		case SDREDGE_BEZIER :		eCT = drawing::ConnectorType_CURVE;	break;
/*N*/ 		case SDREDGE_ARC :          eCT = drawing::ConnectorType_CURVE;	break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR( "SdrEdgeKindItem::QueryValue : unknown enum" );
/*N*/ 	}
/*N*/ 
/*N*/ 	rVal <<= eCT;
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool SdrEdgeKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	drawing::ConnectorType eCT;
/*N*/ 	if(!(rVal >>= eCT))
/*N*/ 	{
/*?*/ 		sal_Int32 nEnum;
/*?*/ 		if(!(rVal >>= nEnum))
/*?*/ 			return sal_False;
/*?*/ 
/*?*/ 		eCT = (drawing::ConnectorType)nEnum;
/*N*/ 	}
/*N*/ 
/*N*/ 	SdrEdgeKind eEK = SDREDGE_ORTHOLINES;
/*N*/ 	switch( eCT )
/*N*/ 	{
/*N*/ 		case drawing::ConnectorType_STANDARD :	eEK = SDREDGE_ORTHOLINES;	break;
/*N*/ 		case drawing::ConnectorType_CURVE :		eEK = SDREDGE_BEZIER;		break;
/*N*/ 		case drawing::ConnectorType_LINE :       eEK = SDREDGE_ONELINE;		break;
/*N*/ 		case drawing::ConnectorType_LINES :      eEK = SDREDGE_THREELINES;   break;
/*N*/ 		default:
/*N*/ 			DBG_ERROR( "SdrEdgeKindItem::PuValue : unknown enum" );
/*N*/ 	}
/*N*/ 	SetValue( eEK );
/*N*/ 
/*N*/ 	return sal_True;
/*N*/ }















//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrEdgeSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1(SdrEdgeSetItem,SfxSetItem);

/*N*/ SfxPoolItem* __EXPORT SdrEdgeSetItem::Clone(SfxItemPool* pToPool) const
/*N*/ {
/*N*/ 	return new SdrEdgeSetItem(*this,pToPool);
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrEdgeSetItem::Create(SvStream& rStream, USHORT nVersion) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
/*N*/ 		SDRATTR_EDGE_FIRST, SDRATTR_EDGE_LAST);
/*N*/ 	pSet->Load(rStream);
/*N*/ 	return new SdrEdgeSetItem(pSet);
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Measure
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// MeasureKind
/*N*/ TYPEINIT1_AUTOFACTORY(SdrMeasureKindItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrMeasureKindItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureKindItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrMeasureKindItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureKindItem(rIn); }

/*N*/ USHORT __EXPORT SdrMeasureKindItem::GetValueCount() const { return 2; }





/*N*/ TYPEINIT1_AUTOFACTORY(SdrMeasureTextHPosItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrMeasureTextHPosItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureTextHPosItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrMeasureTextHPosItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureTextHPosItem(rIn); }

/*N*/ USHORT __EXPORT SdrMeasureTextHPosItem::GetValueCount() const { return 4; }





/*N*/ TYPEINIT1_AUTOFACTORY(SdrMeasureTextVPosItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrMeasureTextVPosItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureTextVPosItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrMeasureTextVPosItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureTextVPosItem(rIn); }

/*N*/ USHORT __EXPORT SdrMeasureTextVPosItem::GetValueCount() const { return 5; }





/*N*/ TYPEINIT1_AUTOFACTORY(SdrMeasureUnitItem,SfxEnumItem);

/*N*/ SfxPoolItem* __EXPORT SdrMeasureUnitItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureUnitItem(*this); }

/*N*/ SfxPoolItem* __EXPORT SdrMeasureUnitItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureUnitItem(rIn); }

/*N*/ USHORT __EXPORT SdrMeasureUnitItem::GetValueCount() const { return 14; }





//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrMeasureSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1(SdrMeasureSetItem,SfxSetItem);

/*N*/ SfxPoolItem* __EXPORT SdrMeasureSetItem::Clone(SfxItemPool* pToPool) const
/*N*/ {
/*N*/ 	return new SdrMeasureSetItem(*this,pToPool);
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrMeasureSetItem::Create(SvStream& rStream, USHORT nVersion) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
/*N*/ 		SDRATTR_MEASURE_FIRST, SDRATTR_MEASURE_LAST);
/*N*/ 	pSet->Load(rStream);
/*N*/ 	return new SdrMeasureSetItem(pSet);
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Circ
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// CircKind
/*N*/ TYPEINIT1_AUTOFACTORY(SdrCircKindItem,SfxEnumItem);
/*N*/ 
/*N*/ SfxPoolItem* __EXPORT SdrCircKindItem::Clone(SfxItemPool* pPool) const          { return new SdrCircKindItem(*this); }
/*N*/ 
/*N*/ SfxPoolItem* __EXPORT SdrCircKindItem::Create(SvStream& rIn, USHORT nVer) const { return new SdrCircKindItem(rIn); }

/*N*/ USHORT __EXPORT SdrCircKindItem::GetValueCount() const { return 4; }




/*N*/ sal_Bool SdrCircKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	drawing::CircleKind eKind;
/*N*/ 	if(!(rVal >>= eKind))
/*N*/ 	{
/*?*/ 		sal_Int32 nEnum;
/*?*/ 		if(!(rVal >>= nEnum))
/*?*/ 			return sal_False;
/*?*/ 
/*?*/ 		eKind = (drawing::CircleKind)nEnum;
/*N*/ 	}
/*N*/ 
/*N*/ 	SetValue( (SdrCircKind)eKind );
/*N*/ 	return sal_True;
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrCircSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1(SdrCircSetItem,SfxSetItem);

/*N*/ SfxPoolItem* __EXPORT SdrCircSetItem::Clone(SfxItemPool* pToPool) const
/*N*/ {
/*N*/ 	return new SdrCircSetItem(*this,pToPool);
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrCircSetItem::Create(SvStream& rStream, USHORT nVersion) const
/*N*/ {
/*N*/ 	SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
/*N*/ 		SDRATTR_CIRC_FIRST, SDRATTR_CIRC_LAST);
/*N*/ 	pSet->Load(rStream);
/*N*/ 	return new SdrCircSetItem(pSet);
/*N*/ }

//------------------------------------------------------------
// class SdrSignedPercentItem
//------------------------------------------------------------

/*N*/ TYPEINIT1_AUTOFACTORY( SdrSignedPercentItem, SfxInt16Item );





#ifdef SDR_ISPOOLABLE

/*?*/ int __EXPORT SdrSignedPercentItem::IsPoolable() const
/*?*/ {
/*?*/ 	USHORT nId=Which();
/*?*/ 	return nId < SDRATTR_NOTPERSIST_FIRST || nId > SDRATTR_NOTPERSIST_LAST;
/*?*/ }
#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafSetItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafSetItem,SfxSetItem );

/*N*/ SfxPoolItem* __EXPORT SdrGrafSetItem::Clone( SfxItemPool* pToPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafSetItem( *this, pToPool );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafSetItem::Create( SvStream& rStream, USHORT nVersion ) const
/*N*/ {
/*N*/ 	SfxItemSet* pSet = new SfxItemSet(*GetItemSet().GetPool(),
/*N*/ 		SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST );
/*N*/ 	pSet->Load( rStream );
/*N*/ 	return new SdrGrafSetItem( pSet );
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafRedItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafRedItem, SdrSignedPercentItem );

/*N*/ SfxPoolItem* __EXPORT SdrGrafRedItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafRedItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafRedItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SdrGrafRedItem( rIn );
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafGreenItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafGreenItem, SdrSignedPercentItem );

/*N*/ SfxPoolItem* __EXPORT SdrGrafGreenItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafGreenItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafGreenItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SdrGrafGreenItem( rIn );
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafBlueItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafBlueItem, SdrSignedPercentItem );

/*N*/ SfxPoolItem* __EXPORT SdrGrafBlueItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafBlueItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafBlueItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SdrGrafBlueItem( rIn );
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafLuminanceItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafLuminanceItem, SdrSignedPercentItem );
/*N*/ 
/*N*/ SfxPoolItem* __EXPORT SdrGrafLuminanceItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafLuminanceItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafLuminanceItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SdrGrafLuminanceItem( rIn );
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafContrastItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafContrastItem, SdrSignedPercentItem );
/*N*/ 
/*N*/ SfxPoolItem* __EXPORT SdrGrafContrastItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafContrastItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafContrastItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SdrGrafContrastItem( rIn );
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafGamma100Item
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafGamma100Item, SfxUInt32Item );

/*N*/ SfxPoolItem* __EXPORT SdrGrafGamma100Item::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafGamma100Item( *this );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafGamma100Item::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SdrGrafGamma100Item( rIn );
/*N*/ }

/*N*/ sal_Bool SdrGrafGamma100Item::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
/*N*/ {
/*N*/ 	rVal <<= ((double)GetValue()) / 100.0;
/*N*/ 	return sal_True;
/*N*/ }

/*N*/ sal_Bool SdrGrafGamma100Item::PutValue( const uno::Any& rVal, BYTE nMemberId )
/*N*/ {
/*N*/ 	double nGamma;
/*N*/ 	if(!(rVal >>= nGamma))
/*N*/ 		return sal_False;
/*N*/ 
/*N*/ 	SetValue( (UINT32)(nGamma * 100.0  ) );
/*N*/ 	return sal_True;
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafInvertItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafInvertItem, SdrOnOffItem );

/*N*/ SfxPoolItem* __EXPORT SdrGrafInvertItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafInvertItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafInvertItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SdrGrafInvertItem( rIn );
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafTransparenceItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafTransparenceItem, SdrPercentItem );
/*N*/ 
/*N*/ SfxPoolItem* __EXPORT SdrGrafTransparenceItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafTransparenceItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafTransparenceItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SdrGrafTransparenceItem( rIn );
/*N*/ }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafModeItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafModeItem, SfxEnumItem );

/*N*/ SfxPoolItem* __EXPORT SdrGrafModeItem::Clone(SfxItemPool* pPool) const
/*N*/ {
/*N*/ 	return new SdrGrafModeItem( *this );
/*N*/ }

/*N*/ SfxPoolItem* __EXPORT SdrGrafModeItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return new SdrGrafModeItem( rIn );
/*N*/ }

/*N*/ USHORT __EXPORT SdrGrafModeItem::GetValueCount() const
/*N*/ {
/*N*/ 	return 4;
/*N*/ }



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafCropItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/*N*/ TYPEINIT1( SdrGrafCropItem, SvxGrfCrop );

/*N*/ SfxPoolItem* SdrGrafCropItem::Clone( SfxItemPool* pPool ) const
/*N*/ {
/*N*/ 	return new SdrGrafCropItem( *this );
/*N*/ }
/*N*/ 
/*N*/ SfxPoolItem* SdrGrafCropItem::Create( SvStream& rIn, USHORT nVer ) const
/*N*/ {
/*N*/ 	return( ( 0 == nVer ) ? Clone( NULL ) : SvxGrfCrop::Create( rIn, nVer ) );
/*N*/ }
/*N*/ 
/*N*/ USHORT SdrGrafCropItem::GetVersion( USHORT nFileVersion ) const
/*N*/ {
/*N*/ 	// GRFCROP_VERSION_MOVETOSVX is 1
/*N*/ 	return GRFCROP_VERSION_MOVETOSVX;
/*N*/ }
}
