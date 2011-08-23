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

#define ITEMID_GRF_CROP 0

#include <com/sun/star/drawing/TextFitToSizeType.hpp>
#include <com/sun/star/drawing/TextHorizontalAdjust.hpp>
#include <com/sun/star/drawing/TextVerticalAdjust.hpp>
#include <com/sun/star/drawing/TextAnimationKind.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/drawing/TextAnimationDirection.hpp>
#include <com/sun/star/drawing/ConnectorType.hpp>
#include <com/sun/star/drawing/MeasureKind.hpp>
#include <com/sun/star/drawing/MeasureTextHorzPos.hpp>
#include <com/sun/star/drawing/MeasureTextVertPos.hpp>
#include <com/sun/star/drawing/CircleKind.hpp>
#include <com/sun/star/uno/Sequence.hxx>

#include <unotools/intlwrapper.hxx>
#include <comphelper/processfactory.hxx>

#include <vcl/svapp.hxx>

#include "xdef.hxx"

#include "svdattr.hxx"
#include "svdattrx.hxx"
#include "svdpool.hxx"

#include <bf_svtools/solar.hrc>

#include "xmlcnitm.hxx"

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

#include <tools/bigint.hxx>

#include <tools/stream.hxx>

#include <xtxasit.hxx>
#include <xfillit0.hxx>
#include <xflclit.hxx>
#include <xlineit0.hxx>

#include <svx3ditems.hxx>
#include <legacysmgr/legacy_binfilters_smgr.hxx>    //STRIP002

namespace binfilter {

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
#ifndef SVX_LIGHT
    ppPoolDefaults[SDRATTR_XMLATTRIBUTES -SDRATTR_START]=new SvXMLAttrContainerItem( SDRATTR_XMLATTRIBUTES );
#else
    // no need to have alien attributes persistent in the player
    ppPoolDefaults[SDRATTR_XMLATTRIBUTES -SDRATTR_START]=new SfxVoidItem( SDRATTR_XMLATTRIBUTES );
#endif // #ifndef SVX_LIGHT
    for (i=SDRATTR_RESERVE15; i<=SDRATTR_RESERVE19; i++) {
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
    ppPoolDefaults[SDRATTR_MEASUREDECIMALPLACES    -SDRATTR_START]=new SdrMeasureDecimalPlacesItem();
    for (i=SDRATTR_MEASURERESERVE05; i<=SDRATTR_MEASURERESERVE07; i++) {
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
    ppPoolDefaults[SDRATTR_TEXTDIRECTION  -SDRATTR_START]=new SvxWritingModeItem;

    for (i=SDRATTR_NOTPERSISTRESERVE2; i<=SDRATTR_NOTPERSISTRESERVE15; i++) {
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
    pItemInfos[SDRATTR_GRAFCROP-SDRATTR_START]._nSID=SID_ATTR_GRAF_CROP;

    // 3D Object Attr (28092000 AW)
    ppPoolDefaults[ SDRATTR_3DOBJ_PERCENT_DIAGONAL - SDRATTR_START ] = new Svx3DPercentDiagonalItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_BACKSCALE - SDRATTR_START ] = new Svx3DBackscaleItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_DEPTH - SDRATTR_START ] = new Svx3DDepthItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_HORZ_SEGS - SDRATTR_START ] = new Svx3DHorizontalSegmentsItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_VERT_SEGS - SDRATTR_START ] = new Svx3DVerticalSegmentsItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_END_ANGLE - SDRATTR_START ] = new Svx3DEndAngleItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_DOUBLE_SIDED - SDRATTR_START ] = new Svx3DDoubleSidedItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_NORMALS_KIND - SDRATTR_START ] = new Svx3DNormalsKindItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_NORMALS_INVERT - SDRATTR_START ] = new Svx3DNormalsInvertItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_X - SDRATTR_START ] = new Svx3DTextureProjectionXItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_Y - SDRATTR_START ] = new Svx3DTextureProjectionYItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_SHADOW_3D - SDRATTR_START ] = new Svx3DShadow3DItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_MAT_COLOR - SDRATTR_START ] = new Svx3DMaterialColorItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_MAT_EMISSION - SDRATTR_START ] = new Svx3DMaterialEmissionItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR - SDRATTR_START ] = new Svx3DMaterialSpecularItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY - SDRATTR_START ] = new Svx3DMaterialSpecularIntensityItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_KIND - SDRATTR_START ] = new Svx3DTextureKindItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_MODE - SDRATTR_START ] = new Svx3DTextureModeItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_FILTER - SDRATTR_START ] = new Svx3DTextureFilterItem;

    // #107245# Add new items for 3d objects
    ppPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_NORMALS - SDRATTR_START ] = new Svx3DSmoothNormalsItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_LIDS - SDRATTR_START ] = new Svx3DSmoothLidsItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_CHARACTER_MODE - SDRATTR_START ] = new Svx3DCharacterModeItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_CLOSE_FRONT - SDRATTR_START ] = new Svx3DCloseFrontItem;
    ppPoolDefaults[ SDRATTR_3DOBJ_CLOSE_BACK - SDRATTR_START ] = new Svx3DCloseBackItem;

    // #107245# Start with SDRATTR_3DOBJ_RESERVED_06 now
    for( i = SDRATTR_3DOBJ_RESERVED_06; i <= SDRATTR_3DOBJ_RESERVED_20; i++ )
        ppPoolDefaults[ i - SDRATTR_START ] = new SfxVoidItem( i );

    // 3D Scene Attr (28092000 AW)
    ppPoolDefaults[ SDRATTR_3DSCENE_PERSPECTIVE - SDRATTR_START ] = new Svx3DPerspectiveItem;
    ppPoolDefaults[ SDRATTR_3DSCENE_DISTANCE - SDRATTR_START ] = new Svx3DDistanceItem;
    ppPoolDefaults[ SDRATTR_3DSCENE_FOCAL_LENGTH - SDRATTR_START ] = new Svx3DFocalLengthItem;
    ppPoolDefaults[ SDRATTR_3DSCENE_TWO_SIDED_LIGHTING - SDRATTR_START ] = new Svx3DTwoSidedLightingItem;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_1 - SDRATTR_START ] = new Svx3DLightcolor1Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_2 - SDRATTR_START ] = new Svx3DLightcolor2Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_3 - SDRATTR_START ] = new Svx3DLightcolor3Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_4 - SDRATTR_START ] = new Svx3DLightcolor4Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_5 - SDRATTR_START ] = new Svx3DLightcolor5Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_6 - SDRATTR_START ] = new Svx3DLightcolor6Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_7 - SDRATTR_START ] = new Svx3DLightcolor7Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_8 - SDRATTR_START ] = new Svx3DLightcolor8Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_AMBIENTCOLOR - SDRATTR_START ] = new Svx3DAmbientcolorItem;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_1 - SDRATTR_START ] = new Svx3DLightOnOff1Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_2 - SDRATTR_START ] = new Svx3DLightOnOff2Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_3 - SDRATTR_START ] = new Svx3DLightOnOff3Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_4 - SDRATTR_START ] = new Svx3DLightOnOff4Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_5 - SDRATTR_START ] = new Svx3DLightOnOff5Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_6 - SDRATTR_START ] = new Svx3DLightOnOff6Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_7 - SDRATTR_START ] = new Svx3DLightOnOff7Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_8 - SDRATTR_START ] = new Svx3DLightOnOff8Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_1 - SDRATTR_START ] = new Svx3DLightDirection1Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_2 - SDRATTR_START ] = new Svx3DLightDirection2Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_3 - SDRATTR_START ] = new Svx3DLightDirection3Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_4 - SDRATTR_START ] = new Svx3DLightDirection4Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_5 - SDRATTR_START ] = new Svx3DLightDirection5Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_6 - SDRATTR_START ] = new Svx3DLightDirection6Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_7 - SDRATTR_START ] = new Svx3DLightDirection7Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_8 - SDRATTR_START ] = new Svx3DLightDirection8Item;
    ppPoolDefaults[ SDRATTR_3DSCENE_SHADOW_SLANT - SDRATTR_START ] = new Svx3DShadowSlantItem;
    ppPoolDefaults[ SDRATTR_3DSCENE_SHADE_MODE - SDRATTR_START ] = new Svx3DShadeModeItem;

    for( i = SDRATTR_3DSCENE_RESERVED_01; i <= SDRATTR_3DSCENE_RESERVED_20; i++ )
        ppPoolDefaults[ i - SDRATTR_START ] = new SfxVoidItem( i );

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
            aStr += ByteString::CreateFromInt32( nNum );
            aStr += "Which=";
            aStr += ByteString::CreateFromInt32( nNum + 1000 );

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
            SetRefCount(*ppPoolDefaults[i],0);
            delete ppPoolDefaults[i];
            ppPoolDefaults[i]=NULL;
        }
    }
    // Vor dem zerstoeren die Pools ggf. voneinander trennen
    SetSecondaryPool(NULL);
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
    SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
                                      SDRATTR_SHADOW_FIRST, SDRATTR_SHADOW_LAST);
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



TYPEINIT1_AUTOFACTORY(SdrCaptionEscDirItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrCaptionEscDirItem::Clone(SfxItemPool* pPool) const              { return new SdrCaptionEscDirItem(*this); }

SfxPoolItem* __EXPORT SdrCaptionEscDirItem::Create(SvStream& rIn, USHORT nVer) const     { return new SdrCaptionEscDirItem(rIn); }

USHORT __EXPORT SdrCaptionEscDirItem::GetValueCount() const { return 3; }



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
    SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
                                      SDRATTR_CAPTION_FIRST, SDRATTR_CAPTION_LAST);
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
    SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
                                      EE_ITEMS_START, EE_ITEMS_END);
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


bool SdrTextFitToSizeTypeItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    drawing::TextFitToSizeType eFS = (drawing::TextFitToSizeType)GetValue();
    rVal <<= eFS;

    return true;
}

bool SdrTextFitToSizeTypeItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextFitToSizeType eFS;
    if(!(rVal >>= eFS))
    {
        sal_Int32 nEnum;
        if(!(rVal >>= nEnum))
            return false;

        eFS = (drawing::TextFitToSizeType) nEnum;
    }

    SetValue( (SdrFitToSizeType)eFS );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextVertAdjustItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrTextVertAdjustItem::Clone(SfxItemPool* pPool) const            { return new SdrTextVertAdjustItem(*this); }

SfxPoolItem* __EXPORT SdrTextVertAdjustItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextVertAdjustItem(rIn); }

USHORT __EXPORT SdrTextVertAdjustItem::GetValueCount() const { return 5; }



bool SdrTextVertAdjustItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::TextVerticalAdjust)GetValue();
    return true;
}

bool SdrTextVertAdjustItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextVerticalAdjust eAdj;
    if(!(rVal >>= eAdj))
    {
        sal_Int32 nEnum;
        if(!(rVal >>= nEnum))
            return false;

        eAdj = (drawing::TextVerticalAdjust)nEnum;
    }

    SetValue( (SdrTextVertAdjust)eAdj );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextHorzAdjustItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrTextHorzAdjustItem::Clone(SfxItemPool* pPool) const { return new SdrTextHorzAdjustItem(*this); }

SfxPoolItem* __EXPORT SdrTextHorzAdjustItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextHorzAdjustItem(rIn); }

USHORT __EXPORT SdrTextHorzAdjustItem::GetValueCount() const { return 5; }

XubString __EXPORT SdrTextHorzAdjustItem::GetValueTextByPos(USHORT nPos) const
{DBG_BF_ASSERT(0, "STRIP");XubString a; return a; //STRIP001
}


bool SdrTextHorzAdjustItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::TextHorizontalAdjust)GetValue();
    return sal_True;
}

bool SdrTextHorzAdjustItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextHorizontalAdjust eAdj;
    if(!(rVal >>= eAdj))
    {
        sal_Int32 nEnum;
        if(!(rVal >>= nEnum))
            return false;

        eAdj = (drawing::TextHorizontalAdjust)nEnum;
    }

    SetValue( (SdrTextHorzAdjust)eAdj );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniKindItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrTextAniKindItem::Clone(SfxItemPool* pPool) const { return new SdrTextAniKindItem(*this); }

SfxPoolItem* __EXPORT SdrTextAniKindItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniKindItem(rIn); }

USHORT __EXPORT SdrTextAniKindItem::GetValueCount() const { return 5; }



bool SdrTextAniKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::TextAnimationKind)GetValue();
    return true;
}

bool SdrTextAniKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextAnimationKind eKind;
    if(!(rVal >>= eKind))
    {
        sal_Int32 nEnum;
        if(!(rVal >>= nEnum))
            return false;
        eKind = (drawing::TextAnimationKind)nEnum;
    }

    SetValue( (SdrTextAniKind)eKind );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniDirectionItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrTextAniDirectionItem::Clone(SfxItemPool* pPool) const { return new SdrTextAniDirectionItem(*this); }

SfxPoolItem* __EXPORT SdrTextAniDirectionItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniDirectionItem(rIn); }

USHORT __EXPORT SdrTextAniDirectionItem::GetValueCount() const { return 4; }



bool SdrTextAniDirectionItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= (drawing::TextAnimationDirection)GetValue();
    return true;
}

bool SdrTextAniDirectionItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::TextAnimationDirection eDir;
    if(!(rVal >>= eDir))
    {
        sal_Int32 nEnum;
        if(!(rVal >>= nEnum))
            return false;

        eDir = (drawing::TextAnimationDirection)nEnum;
    }

    SetValue( (SdrTextAniDirection)eDir );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniDelayItem,SfxUInt16Item);

SfxPoolItem* __EXPORT SdrTextAniDelayItem::Clone(SfxItemPool* pPool) const { return new SdrTextAniDelayItem(*this); }

SfxPoolItem* __EXPORT SdrTextAniDelayItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniDelayItem(rIn); }


TYPEINIT1_AUTOFACTORY(SdrTextAniAmountItem,SfxInt16Item);

SfxPoolItem* __EXPORT SdrTextAniAmountItem::Clone(SfxItemPool* pPool) const { return new SdrTextAniAmountItem(*this); }

SfxPoolItem* __EXPORT SdrTextAniAmountItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrTextAniAmountItem(rIn); }




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
        {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
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
            {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
            }
        }
    }
    return bRet;
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
        {DBG_BF_ASSERT(0, "STRIP"); }//STRIP001     rOut << GetValue( i ).nValue;
    }
    return rOut;
}

SfxPoolItem* __EXPORT SdrAutoShapeAdjustmentItem::Clone( SfxItemPool *pPool ) const
{
    sal_uInt32 i;
    SdrAutoShapeAdjustmentItem* pItem = new SdrAutoShapeAdjustmentItem;
    for ( i = 0; i < GetCount(); i++ )
    {DBG_BF_ASSERT(0, "STRIP"); //STRIP001
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


sal_uInt16 SdrAutoShapeAdjustmentItem::GetVersion( sal_uInt16 nFileFormatVersion ) const
{
    return 1;
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
    SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
                                      SDRATTR_MISC_FIRST, SDRATTR_MISC_LAST);
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



bool SdrEdgeKindItem::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
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

    return true;
}

bool SdrEdgeKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::ConnectorType eCT;
    if(!(rVal >>= eCT))
    {
        sal_Int32 nEnum;
        if(!(rVal >>= nEnum))
            return false;

        eCT = (drawing::ConnectorType)nEnum;
    }

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

    return true;
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
    SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
                                      SDRATTR_EDGE_FIRST, SDRATTR_EDGE_LAST);
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





TYPEINIT1_AUTOFACTORY(SdrMeasureTextHPosItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrMeasureTextHPosItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureTextHPosItem(*this); }

SfxPoolItem* __EXPORT SdrMeasureTextHPosItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureTextHPosItem(rIn); }

USHORT __EXPORT SdrMeasureTextHPosItem::GetValueCount() const { return 4; }


TYPEINIT1_AUTOFACTORY(SdrMeasureTextVPosItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrMeasureTextVPosItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureTextVPosItem(*this); }

SfxPoolItem* __EXPORT SdrMeasureTextVPosItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureTextVPosItem(rIn); }

USHORT __EXPORT SdrMeasureTextVPosItem::GetValueCount() const { return 5; }





TYPEINIT1_AUTOFACTORY(SdrMeasureUnitItem,SfxEnumItem);

SfxPoolItem* __EXPORT SdrMeasureUnitItem::Clone(SfxItemPool* pPool) const            { return new SdrMeasureUnitItem(*this); }

SfxPoolItem* __EXPORT SdrMeasureUnitItem::Create(SvStream& rIn, USHORT nVer) const   { return new SdrMeasureUnitItem(rIn); }

USHORT __EXPORT SdrMeasureUnitItem::GetValueCount() const { return 14; }





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
    SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
                                      SDRATTR_MEASURE_FIRST, SDRATTR_MEASURE_LAST);
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


bool SdrCircKindItem::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    drawing::CircleKind eKind;
    if(!(rVal >>= eKind))
    {
        sal_Int32 nEnum;
        if(!(rVal >>= nEnum))
            return false;

        eKind = (drawing::CircleKind)nEnum;
    }

    SetValue( (SdrCircKind)eKind );
    return true;
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
    SfxItemSet *pSet = new SfxItemSet(*GetItemSet().GetPool(),
                                      SDRATTR_CIRC_FIRST, SDRATTR_CIRC_LAST);
    pSet->Load(rStream);
    return new SdrCircSetItem(pSet);
}

//------------------------------------------------------------
// class SdrSignedPercentItem
//------------------------------------------------------------

TYPEINIT1_AUTOFACTORY( SdrSignedPercentItem, SfxInt16Item );

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
    SfxItemSet* pSet = new SfxItemSet(*GetItemSet().GetPool(),
                                      SDRATTR_GRAF_FIRST, SDRATTR_GRAF_LAST );
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

bool SdrGrafGamma100Item::QueryValue( uno::Any& rVal, BYTE nMemberId ) const
{
    rVal <<= ((double)GetValue()) / 100.0;
    return true;
}

bool SdrGrafGamma100Item::PutValue( const uno::Any& rVal, BYTE nMemberId )
{
    double nGamma;
    if(!(rVal >>= nGamma))
        return false;

    SetValue( (UINT32)(nGamma * 100.0  ) );
    return true;
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
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
