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
#include <svx/svdattr.hxx>
#include <svx/svdattrx.hxx>
#include <svx/svdpool.hxx>

#include <svl/solar.hrc>
#include "editeng/xmlcnitm.hxx"

#include <svx/svxids.hrc>
#include <svx/xtable.hxx>    // for RGB_Color()
#include "svx/svditext.hxx"
#include <svx/svdmodel.hxx>  // for DEGREE_CHAR
#include <svx/svdtrans.hxx>
#include "svx/svdglob.hxx"  // string cache
#include "svx/svdstr.hrc"
#include <svx/sdgcpitm.hxx>
#include <editeng/adjitem.hxx>
#include <svx/sdtfchim.hxx>
#include <editeng/writingmodeitem.hxx>
#include <tools/bigint.hxx>
#include <tools/stream.hxx>

#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/svx3ditems.hxx>

#define ITEMID_BOX SDRATTR_TABLE_BORDER
#define ITEMID_BOXINFO SDRATTR_TABLE_BORDER_INNER
#include "editeng/boxitem.hxx"

#define ITEMID_SHADOW SDRATTR_TABLE_BORDER_SHADOW
#include "editeng/shaditem.hxx"

#define ITEMID_LINE 0
#include "editeng/bolnitem.hxx"

using namespace ::rtl;
using namespace ::com::sun::star;

/*************************************************************************
|*
|* Constructor
|*
\************************************************************************/

SdrItemPool::SdrItemPool(
    SfxItemPool* _pMaster,
    sal_Bool bLoadRefCounts)
:   XOutdevItemPool(_pMaster, SDRATTR_START, SDRATTR_END, bLoadRefCounts)
{
    // prepare some constants
    const Color aNullCol(RGB_Color(COL_BLACK));
    const XubString aEmptyStr;
    const sal_Int32 nDefEdgeDist(500L); // Defaulting hard for Draw (100TH_MM) currently. MapMode will have to be taken into account in the future.

    // init the non-persistent items
    for(sal_uInt16 i(SDRATTR_NOTPERSIST_FIRST); i <= SDRATTR_NOTPERSIST_LAST; i++)
    {
        mpLocalItemInfos[i - SDRATTR_START]._nFlags=0;
    }

    // init own PoolDefaults
    mppLocalPoolDefaults[SDRATTR_SHADOW            -SDRATTR_START]=new SdrShadowItem;
    mppLocalPoolDefaults[SDRATTR_SHADOWCOLOR       -SDRATTR_START]=new SdrShadowColorItem(aEmptyStr,aNullCol);
    mppLocalPoolDefaults[SDRATTR_SHADOWXDIST       -SDRATTR_START]=new SdrShadowXDistItem;
    mppLocalPoolDefaults[SDRATTR_SHADOWYDIST       -SDRATTR_START]=new SdrShadowYDistItem;
    mppLocalPoolDefaults[SDRATTR_SHADOWTRANSPARENCE-SDRATTR_START]=new SdrShadowTransparenceItem;
    mppLocalPoolDefaults[SDRATTR_SHADOW3D          -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOW3D    );
    mppLocalPoolDefaults[SDRATTR_SHADOWPERSP       -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOWPERSP );
    mppLocalPoolDefaults[SDRATTR_CAPTIONTYPE      -SDRATTR_START]=new SdrCaptionTypeItem      ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONFIXEDANGLE-SDRATTR_START]=new SdrCaptionFixedAngleItem;
    mppLocalPoolDefaults[SDRATTR_CAPTIONANGLE     -SDRATTR_START]=new SdrCaptionAngleItem     ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONGAP       -SDRATTR_START]=new SdrCaptionGapItem       ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONESCDIR    -SDRATTR_START]=new SdrCaptionEscDirItem    ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONESCISREL  -SDRATTR_START]=new SdrCaptionEscIsRelItem  ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONESCREL    -SDRATTR_START]=new SdrCaptionEscRelItem    ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONESCABS    -SDRATTR_START]=new SdrCaptionEscAbsItem    ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONLINELEN   -SDRATTR_START]=new SdrCaptionLineLenItem   ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONFITLINELEN-SDRATTR_START]=new SdrCaptionFitLineLenItem;
    mppLocalPoolDefaults[SDRATTR_ECKENRADIUS            -SDRATTR_START]=new SdrEckenradiusItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_MINFRAMEHEIGHT    -SDRATTR_START]=new SdrTextMinFrameHeightItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_AUTOGROWHEIGHT    -SDRATTR_START]=new SdrTextAutoGrowHeightItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_FITTOSIZE     -SDRATTR_START]=new SdrTextFitToSizeTypeItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_LEFTDIST      -SDRATTR_START]=new SdrTextLeftDistItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_RIGHTDIST     -SDRATTR_START]=new SdrTextRightDistItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_UPPERDIST     -SDRATTR_START]=new SdrTextUpperDistItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_LOWERDIST     -SDRATTR_START]=new SdrTextLowerDistItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_VERTADJUST        -SDRATTR_START]=new SdrTextVertAdjustItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_MAXFRAMEHEIGHT    -SDRATTR_START]=new SdrTextMaxFrameHeightItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_MINFRAMEWIDTH -SDRATTR_START]=new SdrTextMinFrameWidthItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_MAXFRAMEWIDTH -SDRATTR_START]=new SdrTextMaxFrameWidthItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_AUTOGROWWIDTH -SDRATTR_START]=new SdrTextAutoGrowWidthItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_HORZADJUST        -SDRATTR_START]=new SdrTextHorzAdjustItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANIKIND           -SDRATTR_START]=new SdrTextAniKindItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANIDIRECTION  -SDRATTR_START]=new SdrTextAniDirectionItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANISTARTINSIDE    -SDRATTR_START]=new SdrTextAniStartInsideItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANISTOPINSIDE -SDRATTR_START]=new SdrTextAniStopInsideItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANICOUNT      -SDRATTR_START]=new SdrTextAniCountItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANIDELAY      -SDRATTR_START]=new SdrTextAniDelayItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANIAMOUNT     -SDRATTR_START]=new SdrTextAniAmountItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_CONTOURFRAME  -SDRATTR_START]=new SdrTextContourFrameItem;
    mppLocalPoolDefaults[SDRATTR_CUSTOMSHAPE_ADJUSTMENT -SDRATTR_START]=new SdrCustomShapeAdjustmentItem;
    mppLocalPoolDefaults[SDRATTR_XMLATTRIBUTES -SDRATTR_START]=new SvXMLAttrContainerItem( SDRATTR_XMLATTRIBUTES );
    mppLocalPoolDefaults[SDRATTR_TEXT_USEFIXEDCELLHEIGHT -SDRATTR_START]=new SdrTextFixedCellHeightItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_WORDWRAP         -SDRATTR_START]=new SdrTextWordWrapItem( sal_True );
    mppLocalPoolDefaults[SDRATTR_TEXT_AUTOGROWSIZE     -SDRATTR_START]=new SdrTextAutoGrowSizeItem;
    mppLocalPoolDefaults[SDRATTR_EDGEKIND         -SDRATTR_START]=new SdrEdgeKindItem;
    mppLocalPoolDefaults[SDRATTR_EDGENODE1HORZDIST-SDRATTR_START]=new SdrEdgeNode1HorzDistItem(nDefEdgeDist);
    mppLocalPoolDefaults[SDRATTR_EDGENODE1VERTDIST-SDRATTR_START]=new SdrEdgeNode1VertDistItem(nDefEdgeDist);
    mppLocalPoolDefaults[SDRATTR_EDGENODE2HORZDIST-SDRATTR_START]=new SdrEdgeNode2HorzDistItem(nDefEdgeDist);
    mppLocalPoolDefaults[SDRATTR_EDGENODE2VERTDIST-SDRATTR_START]=new SdrEdgeNode2VertDistItem(nDefEdgeDist);
    mppLocalPoolDefaults[SDRATTR_EDGENODE1GLUEDIST-SDRATTR_START]=new SdrEdgeNode1GlueDistItem;
    mppLocalPoolDefaults[SDRATTR_EDGENODE2GLUEDIST-SDRATTR_START]=new SdrEdgeNode2GlueDistItem;
    mppLocalPoolDefaults[SDRATTR_EDGELINEDELTAANZ -SDRATTR_START]=new SdrEdgeLineDeltaAnzItem;
    mppLocalPoolDefaults[SDRATTR_EDGELINE1DELTA   -SDRATTR_START]=new SdrEdgeLine1DeltaItem;
    mppLocalPoolDefaults[SDRATTR_EDGELINE2DELTA   -SDRATTR_START]=new SdrEdgeLine2DeltaItem;
    mppLocalPoolDefaults[SDRATTR_EDGELINE3DELTA   -SDRATTR_START]=new SdrEdgeLine3DeltaItem;
    mppLocalPoolDefaults[SDRATTR_MEASUREKIND             -SDRATTR_START]=new SdrMeasureKindItem;
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTHPOS         -SDRATTR_START]=new SdrMeasureTextHPosItem;
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTVPOS         -SDRATTR_START]=new SdrMeasureTextVPosItem;
    mppLocalPoolDefaults[SDRATTR_MEASURELINEDIST         -SDRATTR_START]=new SdrMeasureLineDistItem(800);
    mppLocalPoolDefaults[SDRATTR_MEASUREHELPLINEOVERHANG -SDRATTR_START]=new SdrMeasureHelplineOverhangItem(200);
    mppLocalPoolDefaults[SDRATTR_MEASUREHELPLINEDIST     -SDRATTR_START]=new SdrMeasureHelplineDistItem(100);
    mppLocalPoolDefaults[SDRATTR_MEASUREHELPLINE1LEN     -SDRATTR_START]=new SdrMeasureHelpline1LenItem;
    mppLocalPoolDefaults[SDRATTR_MEASUREHELPLINE2LEN     -SDRATTR_START]=new SdrMeasureHelpline2LenItem;
    mppLocalPoolDefaults[SDRATTR_MEASUREBELOWREFEDGE     -SDRATTR_START]=new SdrMeasureBelowRefEdgeItem;
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTROTA90       -SDRATTR_START]=new SdrMeasureTextRota90Item;
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTUPSIDEDOWN   -SDRATTR_START]=new SdrMeasureTextUpsideDownItem;
    mppLocalPoolDefaults[SDRATTR_MEASUREOVERHANG         -SDRATTR_START]=new SdrMeasureOverhangItem(600);
    mppLocalPoolDefaults[SDRATTR_MEASUREUNIT             -SDRATTR_START]=new SdrMeasureUnitItem;
    mppLocalPoolDefaults[SDRATTR_MEASURESCALE            -SDRATTR_START]=new SdrMeasureScaleItem;
    mppLocalPoolDefaults[SDRATTR_MEASURESHOWUNIT         -SDRATTR_START]=new SdrMeasureShowUnitItem;
    mppLocalPoolDefaults[SDRATTR_MEASUREFORMATSTRING     -SDRATTR_START]=new SdrMeasureFormatStringItem();
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLE    -SDRATTR_START]=new SdrMeasureTextAutoAngleItem();
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLEVIEW-SDRATTR_START]=new SdrMeasureTextAutoAngleViewItem();
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTISFIXEDANGLE -SDRATTR_START]=new SdrMeasureTextIsFixedAngleItem();
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTFIXEDANGLE   -SDRATTR_START]=new SdrMeasureTextFixedAngleItem();
    mppLocalPoolDefaults[SDRATTR_MEASUREDECIMALPLACES    -SDRATTR_START]=new SdrMeasureDecimalPlacesItem();
    mppLocalPoolDefaults[SDRATTR_CIRCKIND      -SDRATTR_START]=new SdrCircKindItem;
    mppLocalPoolDefaults[SDRATTR_CIRCSTARTANGLE-SDRATTR_START]=new SdrCircStartAngleItem;
    mppLocalPoolDefaults[SDRATTR_CIRCENDANGLE  -SDRATTR_START]=new SdrCircEndAngleItem;
    mppLocalPoolDefaults[SDRATTR_OBJMOVEPROTECT -SDRATTR_START]=new SdrObjMoveProtectItem;
    mppLocalPoolDefaults[SDRATTR_OBJSIZEPROTECT -SDRATTR_START]=new SdrObjSizeProtectItem;
    mppLocalPoolDefaults[SDRATTR_OBJPRINTABLE   -SDRATTR_START]=new SdrObjPrintableItem;
    mppLocalPoolDefaults[SDRATTR_OBJVISIBLE     -SDRATTR_START]=new SdrObjVisibleItem;
    mppLocalPoolDefaults[SDRATTR_LAYERID        -SDRATTR_START]=new SdrLayerIdItem;
    mppLocalPoolDefaults[SDRATTR_LAYERNAME      -SDRATTR_START]=new SdrLayerNameItem;
    mppLocalPoolDefaults[SDRATTR_OBJECTNAME     -SDRATTR_START]=new SdrObjectNameItem;
    mppLocalPoolDefaults[SDRATTR_ALLPOSITIONX   -SDRATTR_START]=new SdrAllPositionXItem;
    mppLocalPoolDefaults[SDRATTR_ALLPOSITIONY   -SDRATTR_START]=new SdrAllPositionYItem;
    mppLocalPoolDefaults[SDRATTR_ALLSIZEWIDTH   -SDRATTR_START]=new SdrAllSizeWidthItem;
    mppLocalPoolDefaults[SDRATTR_ALLSIZEHEIGHT  -SDRATTR_START]=new SdrAllSizeHeightItem;
    mppLocalPoolDefaults[SDRATTR_ONEPOSITIONX   -SDRATTR_START]=new SdrOnePositionXItem;
    mppLocalPoolDefaults[SDRATTR_ONEPOSITIONY   -SDRATTR_START]=new SdrOnePositionYItem;
    mppLocalPoolDefaults[SDRATTR_ONESIZEWIDTH   -SDRATTR_START]=new SdrOneSizeWidthItem;
    mppLocalPoolDefaults[SDRATTR_ONESIZEHEIGHT  -SDRATTR_START]=new SdrOneSizeHeightItem;
    mppLocalPoolDefaults[SDRATTR_LOGICSIZEWIDTH -SDRATTR_START]=new SdrLogicSizeWidthItem;
    mppLocalPoolDefaults[SDRATTR_LOGICSIZEHEIGHT-SDRATTR_START]=new SdrLogicSizeHeightItem;
    mppLocalPoolDefaults[SDRATTR_ROTATEANGLE    -SDRATTR_START]=new SdrRotateAngleItem;
    mppLocalPoolDefaults[SDRATTR_SHEARANGLE     -SDRATTR_START]=new SdrShearAngleItem;
    mppLocalPoolDefaults[SDRATTR_MOVEX          -SDRATTR_START]=new SdrMoveXItem;
    mppLocalPoolDefaults[SDRATTR_MOVEY          -SDRATTR_START]=new SdrMoveYItem;
    mppLocalPoolDefaults[SDRATTR_RESIZEXONE     -SDRATTR_START]=new SdrResizeXOneItem;
    mppLocalPoolDefaults[SDRATTR_RESIZEYONE     -SDRATTR_START]=new SdrResizeYOneItem;
    mppLocalPoolDefaults[SDRATTR_ROTATEONE      -SDRATTR_START]=new SdrRotateOneItem;
    mppLocalPoolDefaults[SDRATTR_HORZSHEARONE   -SDRATTR_START]=new SdrHorzShearOneItem;
    mppLocalPoolDefaults[SDRATTR_VERTSHEARONE   -SDRATTR_START]=new SdrVertShearOneItem;
    mppLocalPoolDefaults[SDRATTR_RESIZEXALL     -SDRATTR_START]=new SdrResizeXAllItem;
    mppLocalPoolDefaults[SDRATTR_RESIZEYALL     -SDRATTR_START]=new SdrResizeYAllItem;
    mppLocalPoolDefaults[SDRATTR_ROTATEALL      -SDRATTR_START]=new SdrRotateAllItem;
    mppLocalPoolDefaults[SDRATTR_HORZSHEARALL   -SDRATTR_START]=new SdrHorzShearAllItem;
    mppLocalPoolDefaults[SDRATTR_VERTSHEARALL   -SDRATTR_START]=new SdrVertShearAllItem;
    mppLocalPoolDefaults[SDRATTR_TRANSFORMREF1X -SDRATTR_START]=new SdrTransformRef1XItem;
    mppLocalPoolDefaults[SDRATTR_TRANSFORMREF1Y -SDRATTR_START]=new SdrTransformRef1YItem;
    mppLocalPoolDefaults[SDRATTR_TRANSFORMREF2X -SDRATTR_START]=new SdrTransformRef2XItem;
    mppLocalPoolDefaults[SDRATTR_TRANSFORMREF2Y -SDRATTR_START]=new SdrTransformRef2YItem;
    mppLocalPoolDefaults[SDRATTR_TEXTDIRECTION  -SDRATTR_START]=new SvxWritingModeItem(com::sun::star::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION);
    mppLocalPoolDefaults[ SDRATTR_GRAFRED               - SDRATTR_START] = new SdrGrafRedItem;
    mppLocalPoolDefaults[ SDRATTR_GRAFGREEN         - SDRATTR_START] = new SdrGrafGreenItem;
    mppLocalPoolDefaults[ SDRATTR_GRAFBLUE          - SDRATTR_START] = new SdrGrafBlueItem;
    mppLocalPoolDefaults[ SDRATTR_GRAFLUMINANCE     - SDRATTR_START] = new SdrGrafLuminanceItem;
    mppLocalPoolDefaults[ SDRATTR_GRAFCONTRAST      - SDRATTR_START] = new SdrGrafContrastItem;
    mppLocalPoolDefaults[ SDRATTR_GRAFGAMMA         - SDRATTR_START] = new SdrGrafGamma100Item;
    mppLocalPoolDefaults[ SDRATTR_GRAFTRANSPARENCE  - SDRATTR_START] = new SdrGrafTransparenceItem;
    mppLocalPoolDefaults[ SDRATTR_GRAFINVERT            - SDRATTR_START] = new SdrGrafInvertItem;
    mppLocalPoolDefaults[ SDRATTR_GRAFMODE          - SDRATTR_START] = new SdrGrafModeItem;
    mppLocalPoolDefaults[ SDRATTR_GRAFCROP          - SDRATTR_START] = new SdrGrafCropItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_PERCENT_DIAGONAL - SDRATTR_START ] = new Svx3DPercentDiagonalItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_BACKSCALE - SDRATTR_START ] = new Svx3DBackscaleItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_DEPTH - SDRATTR_START ] = new Svx3DDepthItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_HORZ_SEGS - SDRATTR_START ] = new Svx3DHorizontalSegmentsItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_VERT_SEGS - SDRATTR_START ] = new Svx3DVerticalSegmentsItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_END_ANGLE - SDRATTR_START ] = new Svx3DEndAngleItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_DOUBLE_SIDED - SDRATTR_START ] = new Svx3DDoubleSidedItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_NORMALS_KIND - SDRATTR_START ] = new Svx3DNormalsKindItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_NORMALS_INVERT - SDRATTR_START ] = new Svx3DNormalsInvertItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_X - SDRATTR_START ] = new Svx3DTextureProjectionXItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_Y - SDRATTR_START ] = new Svx3DTextureProjectionYItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_SHADOW_3D - SDRATTR_START ] = new Svx3DShadow3DItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_MAT_COLOR - SDRATTR_START ] = new Svx3DMaterialColorItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_MAT_EMISSION - SDRATTR_START ] = new Svx3DMaterialEmissionItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR - SDRATTR_START ] = new Svx3DMaterialSpecularItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY - SDRATTR_START ] = new Svx3DMaterialSpecularIntensityItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_KIND - SDRATTR_START ] = new Svx3DTextureKindItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_MODE - SDRATTR_START ] = new Svx3DTextureModeItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_FILTER - SDRATTR_START ] = new Svx3DTextureFilterItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_NORMALS - SDRATTR_START ] = new Svx3DSmoothNormalsItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_LIDS - SDRATTR_START ] = new Svx3DSmoothLidsItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_CHARACTER_MODE - SDRATTR_START ] = new Svx3DCharacterModeItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_CLOSE_FRONT - SDRATTR_START ] = new Svx3DCloseFrontItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_CLOSE_BACK - SDRATTR_START ] = new Svx3DCloseBackItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY - SDRATTR_START ] = new Svx3DReducedLineGeometryItem;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_PERSPECTIVE - SDRATTR_START ] = new Svx3DPerspectiveItem;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_DISTANCE - SDRATTR_START ] = new Svx3DDistanceItem;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_FOCAL_LENGTH - SDRATTR_START ] = new Svx3DFocalLengthItem;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_TWO_SIDED_LIGHTING - SDRATTR_START ] = new Svx3DTwoSidedLightingItem;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_1 - SDRATTR_START ] = new Svx3DLightcolor1Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_2 - SDRATTR_START ] = new Svx3DLightcolor2Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_3 - SDRATTR_START ] = new Svx3DLightcolor3Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_4 - SDRATTR_START ] = new Svx3DLightcolor4Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_5 - SDRATTR_START ] = new Svx3DLightcolor5Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_6 - SDRATTR_START ] = new Svx3DLightcolor6Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_7 - SDRATTR_START ] = new Svx3DLightcolor7Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_8 - SDRATTR_START ] = new Svx3DLightcolor8Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_AMBIENTCOLOR - SDRATTR_START ] = new Svx3DAmbientcolorItem;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_1 - SDRATTR_START ] = new Svx3DLightOnOff1Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_2 - SDRATTR_START ] = new Svx3DLightOnOff2Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_3 - SDRATTR_START ] = new Svx3DLightOnOff3Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_4 - SDRATTR_START ] = new Svx3DLightOnOff4Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_5 - SDRATTR_START ] = new Svx3DLightOnOff5Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_6 - SDRATTR_START ] = new Svx3DLightOnOff6Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_7 - SDRATTR_START ] = new Svx3DLightOnOff7Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_8 - SDRATTR_START ] = new Svx3DLightOnOff8Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_1 - SDRATTR_START ] = new Svx3DLightDirection1Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_2 - SDRATTR_START ] = new Svx3DLightDirection2Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_3 - SDRATTR_START ] = new Svx3DLightDirection3Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_4 - SDRATTR_START ] = new Svx3DLightDirection4Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_5 - SDRATTR_START ] = new Svx3DLightDirection5Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_6 - SDRATTR_START ] = new Svx3DLightDirection6Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_7 - SDRATTR_START ] = new Svx3DLightDirection7Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_8 - SDRATTR_START ] = new Svx3DLightDirection8Item;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_SHADOW_SLANT - SDRATTR_START ] = new Svx3DShadowSlantItem;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_SHADE_MODE - SDRATTR_START ] = new Svx3DShadeModeItem;
    mppLocalPoolDefaults[ SDRATTR_CUSTOMSHAPE_ENGINE - SDRATTR_START ] = new SdrCustomShapeEngineItem;
    mppLocalPoolDefaults[ SDRATTR_CUSTOMSHAPE_DATA - SDRATTR_START ] = new SdrCustomShapeDataItem;
    mppLocalPoolDefaults[ SDRATTR_CUSTOMSHAPE_GEOMETRY - SDRATTR_START ] = new SdrCustomShapeGeometryItem;
    mppLocalPoolDefaults[ SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL - SDRATTR_START ] = new SdrCustomShapeReplacementURLItem;

    SvxBoxItem* pboxItem = new SvxBoxItem( SDRATTR_TABLE_BORDER );
    pboxItem->SetDistance( 100 );
    mppLocalPoolDefaults[ SDRATTR_TABLE_BORDER - SDRATTR_START ] = pboxItem;

    SvxBoxInfoItem* pBoxInfoItem = new SvxBoxInfoItem( SDRATTR_TABLE_BORDER_INNER );

    pBoxInfoItem->SetTable( sal_True );
    pBoxInfoItem->SetDist( sal_True);        // always show margin field
    pBoxInfoItem->SetValid( VALID_DISABLE, sal_True ); // some lines may have DontCare state only in tables

    mppLocalPoolDefaults[ SDRATTR_TABLE_BORDER_INNER - SDRATTR_START ] =  pBoxInfoItem;
    mppLocalPoolDefaults[ SDRATTR_TABLE_BORDER_TLBR - SDRATTR_START ] = new SvxLineItem( SDRATTR_TABLE_BORDER_TLBR );
    mppLocalPoolDefaults[ SDRATTR_TABLE_BORDER_BLTR - SDRATTR_START ] = new SvxLineItem( SDRATTR_TABLE_BORDER_BLTR );

    // set own ItemInfos
    mpLocalItemInfos[SDRATTR_SHADOW-SDRATTR_START]._nSID=SID_ATTR_FILL_SHADOW;
    mpLocalItemInfos[SDRATTR_TEXT_FITTOSIZE-SDRATTR_START]._nSID=SID_ATTR_TEXT_FITTOSIZE;
    mpLocalItemInfos[SDRATTR_GRAFCROP-SDRATTR_START]._nSID=SID_ATTR_GRAF_CROP;

    mpLocalItemInfos[SDRATTR_TABLE_BORDER - SDRATTR_START ]._nSID = SID_ATTR_BORDER_OUTER;
    mpLocalItemInfos[SDRATTR_TABLE_BORDER_INNER - SDRATTR_START ]._nSID = SID_ATTR_BORDER_INNER;
    mpLocalItemInfos[SDRATTR_TABLE_BORDER_TLBR - SDRATTR_START ]._nSID = SID_ATTR_BORDER_DIAG_TLBR;
    mpLocalItemInfos[SDRATTR_TABLE_BORDER_BLTR - SDRATTR_START ]._nSID = SID_ATTR_BORDER_DIAG_BLTR;

    // it's my own creation level, set Defaults and ItemInfos
    SetDefaults(mppLocalPoolDefaults);
    SetItemInfos(mpLocalItemInfos);
}

/*************************************************************************
|*
|* copy ctor, so that static defaults are cloned
|*            (Parameter 2 = sal_True)
|*
\************************************************************************/

SdrItemPool::SdrItemPool(const SdrItemPool& rPool)
:   XOutdevItemPool(rPool)
{
}

/*************************************************************************
|*
|* Clone()
|*
\************************************************************************/

SfxItemPool* SdrItemPool::Clone() const
{
    return new SdrItemPool(*this);
}

/*************************************************************************
|*
|* Destructor
|*
\************************************************************************/

SdrItemPool::~SdrItemPool()
{
    // dtor of SfxItemPool
    Delete();

    // clear own static Defaults
    if(mppLocalPoolDefaults)
    {
        const sal_uInt16 nBeg(SDRATTR_SHADOW_FIRST - SDRATTR_START);
        const sal_uInt16 nEnd2(SDRATTR_END - SDRATTR_START);

        for(sal_uInt16 i(nBeg); i <= nEnd2; i++)
        {
            SetRefCount(*mppLocalPoolDefaults[i],0);
            delete mppLocalPoolDefaults[i];
            mppLocalPoolDefaults[i] = 0L;
        }
    }

    // split pools before destroying
    SetSecondaryPool(NULL);
}

SfxItemPresentation SdrItemPool::GetPresentation(
              const SfxPoolItem& rItem, SfxItemPresentation ePresentation,
              SfxMapUnit ePresentationMetric, XubString& rText,
              const IntlWrapper * pIntlWrapper) const
{
    if (!IsInvalidItem(&rItem)) {
        sal_uInt16 nWhich=rItem.Which();
        if (nWhich>=SDRATTR_SHADOW_FIRST && nWhich<=SDRATTR_END) {
            rItem.GetPresentation(SFX_ITEM_PRESENTATION_NAMELESS,
                        GetMetric(nWhich),ePresentationMetric,rText,
                        pIntlWrapper);
            String aStr;

            TakeItemName(nWhich, aStr);
            aStr += sal_Unicode(' ');
            rText.Insert(aStr, 0);

            return ePresentation;
        }
    }
    return XOutdevItemPool::GetPresentation(rItem,ePresentation,ePresentationMetric,rText,pIntlWrapper);
}

void SdrItemPool::TakeItemName(sal_uInt16 nWhich, String& rItemName)
{
    ResMgr* pResMgr = ImpGetResMgr();
    sal_uInt16  nResId = SIP_UNKNOWN_ATTR;

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
        case XATTR_SECONDARYFILLCOLOR   : nResId = SIP_XA_SECONDARYFILLCOLOR;break;
        case XATTR_FILLBMP_SIZELOG      : nResId = SIP_XA_FILLBMP_SIZELOG;break;
        case XATTR_FILLBMP_TILEOFFSETX  : nResId = SIP_XA_FILLBMP_TILEOFFSETX;break;
        case XATTR_FILLBMP_TILEOFFSETY  : nResId = SIP_XA_FILLBMP_TILEOFFSETY;break;
        case XATTR_FILLBMP_STRETCH      : nResId = SIP_XA_FILLBMP_STRETCH;break;
        case XATTR_FILLBMP_POSOFFSETX   : nResId = SIP_XA_FILLBMP_POSOFFSETX;break;
        case XATTR_FILLBMP_POSOFFSETY   : nResId = SIP_XA_FILLBMP_POSOFFSETY;break;
        case XATTR_FILLBACKGROUND       : nResId = SIP_XA_FILLBACKGROUND;break;

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

        case SDRATTR_SHADOW            : nResId = SIP_SA_SHADOW;break;
        case SDRATTR_SHADOWCOLOR       : nResId = SIP_SA_SHADOWCOLOR;break;
        case SDRATTR_SHADOWXDIST       : nResId = SIP_SA_SHADOWXDIST;break;
        case SDRATTR_SHADOWYDIST       : nResId = SIP_SA_SHADOWYDIST;break;
        case SDRATTR_SHADOWTRANSPARENCE: nResId = SIP_SA_SHADOWTRANSPARENCE;break;
        case SDRATTR_SHADOW3D          : nResId = SIP_SA_SHADOW3D;break;
        case SDRATTR_SHADOWPERSP       : nResId = SIP_SA_SHADOWPERSP;break;

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
        case SDRATTR_CUSTOMSHAPE_ADJUSTMENT : nResId = SIP_SA_CUSTOMSHAPE_ADJUSTMENT;break;
        case SDRATTR_XMLATTRIBUTES          : nResId = SIP_SA_XMLATTRIBUTES;break;
        case SDRATTR_TEXT_USEFIXEDCELLHEIGHT: nResId = SIP_SA_TEXT_USEFIXEDCELLHEIGHT;break;
        case SDRATTR_TEXT_WORDWRAP          : nResId = SIP_SA_WORDWRAP;break;
        case SDRATTR_TEXT_AUTOGROWSIZE      : nResId = SIP_SA_AUTOGROWSIZE;break;

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
        case SDRATTR_MEASUREDECIMALPLACES    : nResId = SIP_SA_MEASUREDECIMALPLACES;break;

        case SDRATTR_CIRCKIND      : nResId = SIP_SA_CIRCKIND;break;
        case SDRATTR_CIRCSTARTANGLE: nResId = SIP_SA_CIRCSTARTANGLE;break;
        case SDRATTR_CIRCENDANGLE  : nResId = SIP_SA_CIRCENDANGLE;break;

        case SDRATTR_OBJMOVEPROTECT : nResId = SIP_SA_OBJMOVEPROTECT;break;
        case SDRATTR_OBJSIZEPROTECT : nResId = SIP_SA_OBJSIZEPROTECT;break;
        case SDRATTR_OBJPRINTABLE   : nResId = SIP_SA_OBJPRINTABLE;break;
        case SDRATTR_OBJVISIBLE     : nResId = SIP_SA_OBJVISIBLE;break;
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
        case EE_CHAR_OVERLINE   : nResId = SIP_EE_CHAR_OVERLINE;break;
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

    rItemName = String( ResId( nResId, *pResMgr ) );
}

////////////////////////////////////////////////////////////////////////////////
// FractionItem
////////////////////////////////////////////////////////////////////////////////

TYPEINIT1_AUTOFACTORY(SdrFractionItem,SfxPoolItem);

SdrFractionItem::SdrFractionItem(sal_uInt16 nId, SvStream& rIn):
    SfxPoolItem(nId)
{
    sal_Int32 nMul,nDiv;
    rIn>>nMul;
    rIn>>nDiv;
    nValue=Fraction(nMul,nDiv);
}

int SdrFractionItem::operator==(const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
           ((SdrFractionItem&)rCmp).GetValue()==nValue;
}

SfxItemPresentation SdrFractionItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit /*eCoreMetric*/,
    SfxMapUnit /*ePresentationMetric*/, XubString &rText, const IntlWrapper *) const
{
    if(nValue.IsValid())
    {
        sal_Int32 nDiv = nValue.GetDenominator();
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

SfxPoolItem* SdrFractionItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrFractionItem(Which(),rIn);
}

SvStream& SdrFractionItem::Store(SvStream& rOut, sal_uInt16 /*nItemVers*/) const
{
    rOut<<sal_Int32(nValue.GetNumerator());
    rOut<<sal_Int32(nValue.GetDenominator());
    return rOut;
}

SfxPoolItem* SdrFractionItem::Clone(SfxItemPool * /*pPool*/) const
{
    return new SdrFractionItem(Which(),GetValue());
}

////////////////////////////////////////////////////////////////////////////////
// ScaleItem
////////////////////////////////////////////////////////////////////////////////

TYPEINIT1_AUTOFACTORY(SdrScaleItem,SdrFractionItem);

SfxItemPresentation SdrScaleItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit /*eCoreMetric*/,
    SfxMapUnit /*ePresentationMetric*/, XubString &rText, const IntlWrapper *) const
{
    if(GetValue().IsValid())
    {
        sal_Int32 nDiv = GetValue().GetDenominator();

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

SfxPoolItem* SdrScaleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrScaleItem(Which(),rIn);
}

SfxPoolItem* SdrScaleItem::Clone(SfxItemPool * /*pPool*/) const
{
    return new SdrScaleItem(Which(),GetValue());
}

////////////////////////////////////////////////////////////////////////////////
// OnOffItem
////////////////////////////////////////////////////////////////////////////////

TYPEINIT1_AUTOFACTORY(SdrOnOffItem,SfxBoolItem);

SfxPoolItem* SdrOnOffItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrOnOffItem(Which(),GetValue());
}

SfxPoolItem* SdrOnOffItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrOnOffItem(Which(),rIn);
}

rtl::OUString SdrOnOffItem::GetValueTextByVal(sal_Bool bVal) const
{
    if (bVal)
        return ImpGetResStr(STR_ItemValON);
    return ImpGetResStr(STR_ItemValOFF);
}

SfxItemPresentation SdrOnOffItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
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

TYPEINIT1_AUTOFACTORY(SdrYesNoItem,SfxBoolItem);

SfxPoolItem* SdrYesNoItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrYesNoItem(Which(),GetValue());
}

SfxPoolItem* SdrYesNoItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrYesNoItem(Which(),rIn);
}

rtl::OUString SdrYesNoItem::GetValueTextByVal(sal_Bool bVal) const
{
    if (bVal)
        return ImpGetResStr(STR_ItemValYES);
    return ImpGetResStr(STR_ItemValNO);
}

SfxItemPresentation SdrYesNoItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
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

//------------------------------------------------------------
// class SdrPercentItem
//------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SdrPercentItem,SfxUInt16Item);

SfxPoolItem* SdrPercentItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrPercentItem(Which(),GetValue());
}

SfxPoolItem* SdrPercentItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrPercentItem(Which(),rIn);
}

SfxItemPresentation SdrPercentItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreMetric*/,
    SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
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

//------------------------------------------------------------
// class SdrAngleItem
//------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SdrAngleItem,SfxInt32Item);

SfxPoolItem* SdrAngleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrAngleItem(Which(),GetValue());
}

SfxPoolItem* SdrAngleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrAngleItem(Which(),rIn);
}

SfxItemPresentation SdrAngleItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
    XubString& rText, const IntlWrapper * pIntlWrapper) const
{
    sal_Int32 nValue(GetValue());
    sal_Bool bNeg(nValue < 0);

    if(bNeg)
        nValue = -nValue;

    rText = UniString::CreateFromInt32(nValue);

    if(nValue)
    {
        sal_Unicode aUnicodeNull('0');
        xub_StrLen nAnz(2);

        const IntlWrapper* pMyIntlWrapper = NULL;
        if(!pIntlWrapper)
            pIntlWrapper = pMyIntlWrapper = new IntlWrapper(
                ::comphelper::getProcessServiceFactory(),
                Application::GetSettings().GetLanguage() );

        if(pIntlWrapper->getLocaleData()->isNumLeadingZero())
            nAnz++;

        while(rText.Len() < nAnz)
            rText.Insert(aUnicodeNull, 0);

        xub_StrLen nLen = rText.Len();
        sal_Bool bNull1(rText.GetChar(nLen-1) == aUnicodeNull);
        sal_Bool bNull2(bNull1 && rText.GetChar(nLen-2) == aUnicodeNull);

        if(bNull2)
        {
            // no decimal place(s)
            rText.Erase(nLen-2);
        }
        else
        {
            sal_Unicode cDec =
                pIntlWrapper->getLocaleData()->getNumDecimalSep()[0];
            rText.Insert(cDec, nLen-2);

            if(bNull1)
                rText.Erase(nLen);
        }

        if(bNeg)
            rText.Insert(sal_Unicode('-'), 0);

        if ( pMyIntlWrapper )
        {
            delete pMyIntlWrapper;
            pIntlWrapper = NULL;
        }
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

//------------------------------------------------------------
// class SdrMetricItem
//------------------------------------------------------------

TYPEINIT1_AUTOFACTORY(SdrMetricItem,SfxInt32Item);

SfxPoolItem* SdrMetricItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrMetricItem(Which(),GetValue());
}

SfxPoolItem* SdrMetricItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrMetricItem(Which(),rIn);
}

bool SdrMetricItem::HasMetrics() const
{
    return sal_True;
}

bool SdrMetricItem::ScaleMetrics(long nMul, long nDiv)
{
    if (GetValue()!=0) {
        BigInt aVal(GetValue());
        aVal*=nMul;
        aVal+=nDiv/2; // to round accurately
        aVal/=nDiv;
        SetValue(long(aVal));
    }
    return sal_True;
}

SfxItemPresentation SdrMetricItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, XubString& rText, const IntlWrapper *) const
{
    long nValue=GetValue();
    SdrFormatter aFmt((MapUnit)eCoreMetric,(MapUnit)ePresMetric);
    aFmt.TakeStr(nValue,rText);
    String aStr;
    aFmt.TakeUnitStr((MapUnit)ePresMetric,aStr);
    rText+=aStr;
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr2;

        SdrItemPool::TakeItemName(Which(), aStr2);
        aStr2 += sal_Unicode(' ');
        rText.Insert(aStr2, 0);
    }
    return ePres;
}

////////////////////////////////////////////////////////////////////////////////
// items of the legend object
////////////////////////////////////////////////////////////////////////////////

TYPEINIT1_AUTOFACTORY(SdrCaptionTypeItem,SfxEnumItem);

SfxPoolItem* SdrCaptionTypeItem::Clone(SfxItemPool* /*pPool*/) const                { return new SdrCaptionTypeItem(*this); }

SfxPoolItem* SdrCaptionTypeItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const       { return new SdrCaptionTypeItem(rIn); }

sal_uInt16 SdrCaptionTypeItem::GetValueCount() const { return 4; }

rtl::OUString SdrCaptionTypeItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValCAPTIONTYPE1+nPos);
}

SfxItemPresentation SdrCaptionTypeItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

TYPEINIT1_AUTOFACTORY(SdrCaptionEscDirItem,SfxEnumItem);

SfxPoolItem* SdrCaptionEscDirItem::Clone(SfxItemPool* /*pPool*/) const              { return new SdrCaptionEscDirItem(*this); }

SfxPoolItem* SdrCaptionEscDirItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const     { return new SdrCaptionEscDirItem(rIn); }

sal_uInt16 SdrCaptionEscDirItem::GetValueCount() const { return 3; }

rtl::OUString SdrCaptionEscDirItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValCAPTIONESCHORI+nPos);
}

SfxItemPresentation SdrCaptionEscDirItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

////////////////////////////////////////////////////////////////////////////////
// MiscItems
////////////////////////////////////////////////////////////////////////////////

// FitToSize
TYPEINIT1_AUTOFACTORY(SdrTextFitToSizeTypeItem,SfxEnumItem);

SfxPoolItem* SdrTextFitToSizeTypeItem::Clone(SfxItemPool* /*pPool*/) const         { return new SdrTextFitToSizeTypeItem(*this); }

SfxPoolItem* SdrTextFitToSizeTypeItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const { return new SdrTextFitToSizeTypeItem(rIn); }

sal_uInt16 SdrTextFitToSizeTypeItem::GetValueCount() const { return 4; }

rtl::OUString SdrTextFitToSizeTypeItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValFITTOSIZENONE+nPos);
}

SfxItemPresentation SdrTextFitToSizeTypeItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

int SdrTextFitToSizeTypeItem::HasBoolValue() const { return sal_True; }

sal_Bool SdrTextFitToSizeTypeItem::GetBoolValue() const { return GetValue()!=SDRTEXTFIT_NONE; }

void SdrTextFitToSizeTypeItem::SetBoolValue(sal_Bool bVal) { SetValue(sal::static_int_cast< sal_uInt16 >(bVal ? SDRTEXTFIT_PROPORTIONAL : SDRTEXTFIT_NONE)); }

bool SdrTextFitToSizeTypeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    drawing::TextFitToSizeType eFS = (drawing::TextFitToSizeType)GetValue();
    rVal <<= eFS;

    return true;
}

bool SdrTextFitToSizeTypeItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextFitToSizeType eFS;
    if(!(rVal >>= eFS))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eFS = (drawing::TextFitToSizeType) nEnum;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( (SdrFitToSizeType)eFS ) );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextVertAdjustItem,SfxEnumItem);

SfxPoolItem* SdrTextVertAdjustItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextVertAdjustItem(*this); }

SfxPoolItem* SdrTextVertAdjustItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextVertAdjustItem(rIn); }

sal_uInt16 SdrTextVertAdjustItem::GetValueCount() const { return 5; }

rtl::OUString SdrTextVertAdjustItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTVADJTOP+nPos);
}

SfxItemPresentation SdrTextVertAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrTextVertAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextVerticalAdjust)GetValue();
    return true;
}

bool SdrTextVertAdjustItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextVerticalAdjust eAdj;
    if(!(rVal >>= eAdj))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eAdj = (drawing::TextVerticalAdjust)nEnum;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( (SdrTextVertAdjust)eAdj ) );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextHorzAdjustItem,SfxEnumItem);

SfxPoolItem* SdrTextHorzAdjustItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextHorzAdjustItem(*this); }

SfxPoolItem* SdrTextHorzAdjustItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextHorzAdjustItem(rIn); }

sal_uInt16 SdrTextHorzAdjustItem::GetValueCount() const { return 5; }

rtl::OUString SdrTextHorzAdjustItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTHADJLEFT+nPos);
}

SfxItemPresentation SdrTextHorzAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrTextHorzAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextHorizontalAdjust)GetValue();
    return true;
}

bool SdrTextHorzAdjustItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextHorizontalAdjust eAdj;
    if(!(rVal >>= eAdj))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eAdj = (drawing::TextHorizontalAdjust)nEnum;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( (SdrTextHorzAdjust)eAdj ) );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniKindItem,SfxEnumItem);

SfxPoolItem* SdrTextAniKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniKindItem(*this); }

SfxPoolItem* SdrTextAniKindItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextAniKindItem(rIn); }

sal_uInt16 SdrTextAniKindItem::GetValueCount() const { return 5; }

rtl::OUString SdrTextAniKindItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTANI_NONE+nPos);
}

SfxItemPresentation SdrTextAniKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrTextAniKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextAnimationKind)GetValue();
    return true;
}

bool SdrTextAniKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextAnimationKind eKind;
    if(!(rVal >>= eKind))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;
        eKind = (drawing::TextAnimationKind)nEnum;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( (SdrTextAniKind)eKind ) );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniDirectionItem,SfxEnumItem);

SfxPoolItem* SdrTextAniDirectionItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniDirectionItem(*this); }

SfxPoolItem* SdrTextAniDirectionItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextAniDirectionItem(rIn); }

sal_uInt16 SdrTextAniDirectionItem::GetValueCount() const { return 4; }

rtl::OUString SdrTextAniDirectionItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTANI_LEFT+nPos);
}

SfxItemPresentation SdrTextAniDirectionItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrTextAniDirectionItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::TextAnimationDirection)GetValue();
    return true;
}

bool SdrTextAniDirectionItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::TextAnimationDirection eDir;
    if(!(rVal >>= eDir))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eDir = (drawing::TextAnimationDirection)nEnum;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( (SdrTextAniDirection)eDir ) );

    return true;
}

TYPEINIT1_AUTOFACTORY(SdrTextAniDelayItem,SfxUInt16Item);

SfxPoolItem* SdrTextAniDelayItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniDelayItem(*this); }

SfxPoolItem* SdrTextAniDelayItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextAniDelayItem(rIn); }

SfxItemPresentation SdrTextAniDelayItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
    XubString& rText, const IntlWrapper *) const
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

SfxPoolItem* SdrTextAniAmountItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniAmountItem(*this); }

SfxPoolItem* SdrTextAniAmountItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextAniAmountItem(rIn); }

bool SdrTextAniAmountItem::HasMetrics() const
{
    return GetValue()>0;
}

bool SdrTextAniAmountItem::ScaleMetrics(long nMul, long nDiv)
{
    if (GetValue()>0) {
        BigInt aVal(GetValue());
        aVal*=nMul;
        aVal+=nDiv/2; // to round accurately
        aVal/=nDiv;
        SetValue(short(aVal));
        return sal_True;
    } else return sal_False;
}

SfxItemPresentation SdrTextAniAmountItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
    XubString& rText, const IntlWrapper *) const
{
    sal_Int32 nValue(GetValue());

    if(!nValue)
        nValue = -1L;

    if(nValue < 0)
    {
        rText = UniString::CreateFromInt32(-nValue);
        rText += UniString(RTL_CONSTASCII_USTRINGPARAM("pixel"));
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

TYPEINIT1_AUTOFACTORY( SdrTextFixedCellHeightItem, SfxBoolItem );
SdrTextFixedCellHeightItem::SdrTextFixedCellHeightItem( sal_Bool bUseFixedCellHeight )
    : SfxBoolItem( SDRATTR_TEXT_USEFIXEDCELLHEIGHT, bUseFixedCellHeight )
{
}
SdrTextFixedCellHeightItem::SdrTextFixedCellHeightItem( SvStream & rStream, sal_uInt16 nVersion )
    : SfxBoolItem( SDRATTR_TEXT_USEFIXEDCELLHEIGHT, sal_False )
{
    if ( nVersion )
    {
        sal_Bool bValue;
        rStream >> bValue;
        SetValue( bValue );
    }
}
SfxItemPresentation SdrTextFixedCellHeightItem::GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresentationMetric*/,
                                        String &rText, const IntlWrapper * ) const
{
    rText = GetValueTextByVal( GetValue() );
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE)
    {
        String aStr;
        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}
SfxPoolItem* SdrTextFixedCellHeightItem::Create( SvStream& rIn, sal_uInt16 nItemVersion ) const
{
    return new SdrTextFixedCellHeightItem( rIn, nItemVersion );
}
SvStream& SdrTextFixedCellHeightItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    if ( nItemVersion )
    {
        sal_Bool bValue = (sal_Bool)GetValue();
        rOut << bValue;
    }
    return rOut;
}
SfxPoolItem* SdrTextFixedCellHeightItem::Clone( SfxItemPool * /*pPool*/) const
{
    return new SdrTextFixedCellHeightItem( GetValue() );
}
sal_uInt16 SdrTextFixedCellHeightItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}
bool SdrTextFixedCellHeightItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    sal_Bool bValue = (sal_Bool)GetValue();
    rVal <<= bValue;
    return true;
}
bool SdrTextFixedCellHeightItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Bool bValue = sal_Bool();
    if( !( rVal >>= bValue ) )
        return false;
    SetValue( bValue );
    return true;
}

TYPEINIT1_AUTOFACTORY( SdrCustomShapeAdjustmentItem, SfxPoolItem );

SdrCustomShapeAdjustmentItem::SdrCustomShapeAdjustmentItem() : SfxPoolItem( SDRATTR_CUSTOMSHAPE_ADJUSTMENT )
{
}

SdrCustomShapeAdjustmentItem::SdrCustomShapeAdjustmentItem( SvStream& rIn, sal_uInt16 nVersion ):
    SfxPoolItem( SDRATTR_CUSTOMSHAPE_ADJUSTMENT )
{
    if ( nVersion )
    {
        SdrCustomShapeAdjustmentValue aVal;
        sal_uInt32 i, nCount;
        rIn >> nCount;
        for ( i = 0; i < nCount; i++ )
        {
            rIn >> aVal.nValue;
            SetValue( i, aVal );
        }
    }
}

SdrCustomShapeAdjustmentItem::~SdrCustomShapeAdjustmentItem()
{
}

int SdrCustomShapeAdjustmentItem::operator==( const SfxPoolItem& rCmp ) const
{
    int bRet = SfxPoolItem::operator==( rCmp );
    if ( bRet )
    {
        bRet = GetCount() == ((SdrCustomShapeAdjustmentItem&)rCmp).GetCount();

        if (bRet)
        {
            for (sal_uInt32 i = 0; i < GetCount(); ++i)
                if (aAdjustmentValueList[i].nValue != ((SdrCustomShapeAdjustmentItem&)rCmp).aAdjustmentValueList[i].nValue)
                    return false;
        }
    }
    return bRet;
}

SfxItemPresentation SdrCustomShapeAdjustmentItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit /*eCoreMetric*/,
    SfxMapUnit /*ePresentationMetric*/, XubString &rText, const IntlWrapper *) const
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

SfxPoolItem* SdrCustomShapeAdjustmentItem::Create( SvStream& rIn, sal_uInt16 nItemVersion ) const
{
    return new SdrCustomShapeAdjustmentItem( rIn, nItemVersion );
}

SvStream& SdrCustomShapeAdjustmentItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
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

SfxPoolItem* SdrCustomShapeAdjustmentItem::Clone( SfxItemPool * /*pPool*/) const
{
    SdrCustomShapeAdjustmentItem* pItem = new SdrCustomShapeAdjustmentItem;

    if (pItem)
        pItem->aAdjustmentValueList = aAdjustmentValueList;

    return pItem;
}

const SdrCustomShapeAdjustmentValue& SdrCustomShapeAdjustmentItem::GetValue( sal_uInt32 nIndex ) const
{
#ifdef DBG_UTIL
    if ( aAdjustmentValueList.size() <= nIndex )
        OSL_FAIL( "SdrCustomShapeAdjustemntItem::GetValue - nIndex out of range (SJ)" );
#endif
    return aAdjustmentValueList[nIndex];
}

void SdrCustomShapeAdjustmentItem::SetValue( sal_uInt32 nIndex, const SdrCustomShapeAdjustmentValue& rVal )
{
    for (sal_uInt32 i = aAdjustmentValueList.size(); i <= nIndex; i++ )
        aAdjustmentValueList.push_back(SdrCustomShapeAdjustmentValue());

    aAdjustmentValueList[nIndex] = rVal;
}

sal_uInt16 SdrCustomShapeAdjustmentItem::GetVersion( sal_uInt16 /*nFileFormatVersion*/) const
{
    return 1;
}

bool SdrCustomShapeAdjustmentItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    sal_uInt32 i, nCount = GetCount();
    uno::Sequence< sal_Int32 > aSequence( nCount );
    if ( nCount )
    {
        sal_Int32* pPtr = aSequence.getArray();
        for ( i = 0; i < nCount; i++ )
            *pPtr++ = GetValue( i ).nValue;
    }
    rVal <<= aSequence;
    return true;
}

bool SdrCustomShapeAdjustmentItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    uno::Sequence< sal_Int32 > aSequence;
    if( !( rVal >>= aSequence ) )
        return false;

    aAdjustmentValueList.clear();

    sal_uInt32 i, nCount = aSequence.getLength();
    if ( nCount )
    {
        SdrCustomShapeAdjustmentValue val;
        const sal_Int32* pPtr2 = aSequence.getConstArray();
        for ( i = 0; i < nCount; i++ )
        {
            val.nValue = *pPtr2++;
            aAdjustmentValueList.push_back(val);
        }
    }
    return true;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Edge
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

// EdgeKind
TYPEINIT1_AUTOFACTORY(SdrEdgeKindItem,SfxEnumItem);

SfxPoolItem* SdrEdgeKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrEdgeKindItem(*this); }

SfxPoolItem* SdrEdgeKindItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrEdgeKindItem(rIn); }

sal_uInt16 SdrEdgeKindItem::GetValueCount() const { return 4; }

rtl::OUString SdrEdgeKindItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValEDGE_ORTHOLINES+nPos);
}

SfxItemPresentation SdrEdgeKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrEdgeKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
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
            OSL_FAIL( "SdrEdgeKindItem::QueryValue : unknown enum" );
    }

    rVal <<= eCT;

    return true;
}

bool SdrEdgeKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::ConnectorType eCT;
    if(!(rVal >>= eCT))
    {
        sal_Int32 nEnum = 0;
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
            OSL_FAIL( "SdrEdgeKindItem::PuValue : unknown enum" );
    }
    SetValue( sal::static_int_cast< sal_uInt16 >( eEK ) );

    return true;
}

bool SdrEdgeNode1HorzDistItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool SdrEdgeNode1HorzDistItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

bool SdrEdgeNode1VertDistItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool SdrEdgeNode1VertDistItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

bool SdrEdgeNode2HorzDistItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool SdrEdgeNode2HorzDistItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

bool SdrEdgeNode2VertDistItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool SdrEdgeNode2VertDistItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

bool SdrEdgeLine1DeltaItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool SdrEdgeLine1DeltaItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

bool SdrEdgeLine2DeltaItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool SdrEdgeLine2DeltaItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

bool SdrEdgeLine3DeltaItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool SdrEdgeLine3DeltaItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nValue = 0;
    if(!(rVal >>= nValue))
        return false;

    SetValue( nValue );
    return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Measure
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1_AUTOFACTORY(SdrMeasureKindItem,SfxEnumItem);

SfxPoolItem* SdrMeasureKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureKindItem(*this); }

SfxPoolItem* SdrMeasureKindItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrMeasureKindItem(rIn); }

sal_uInt16 SdrMeasureKindItem::GetValueCount() const { return 2; }

rtl::OUString SdrMeasureKindItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValMEASURE_STD+nPos);
}

SfxItemPresentation SdrMeasureKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrMeasureKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::MeasureKind)GetValue();
    return true;
}

bool SdrMeasureKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::MeasureKind eKind;
    if(!(rVal >>= eKind))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eKind = (drawing::MeasureKind)nEnum;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( (SdrMeasureKind)eKind ) );
    return true;
}

TYPEINIT1_AUTOFACTORY(SdrMeasureTextHPosItem,SfxEnumItem);

SfxPoolItem* SdrMeasureTextHPosItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureTextHPosItem(*this); }

SfxPoolItem* SdrMeasureTextHPosItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrMeasureTextHPosItem(rIn); }

sal_uInt16 SdrMeasureTextHPosItem::GetValueCount() const { return 4; }

rtl::OUString SdrMeasureTextHPosItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValMEASURE_TEXTHAUTO+nPos);
}

SfxItemPresentation SdrMeasureTextHPosItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrMeasureTextHPosItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::MeasureTextHorzPos)GetValue();
    return true;
}

bool SdrMeasureTextHPosItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::MeasureTextHorzPos ePos;
    if(!(rVal >>= ePos))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        ePos = (drawing::MeasureTextHorzPos)nEnum;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( (SdrMeasureTextHPos)ePos ) );
    return true;
}

TYPEINIT1_AUTOFACTORY(SdrMeasureTextVPosItem,SfxEnumItem);

SfxPoolItem* SdrMeasureTextVPosItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureTextVPosItem(*this); }

SfxPoolItem* SdrMeasureTextVPosItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrMeasureTextVPosItem(rIn); }

sal_uInt16 SdrMeasureTextVPosItem::GetValueCount() const { return 5; }

rtl::OUString SdrMeasureTextVPosItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValMEASURE_TEXTVAUTO+nPos);
}

SfxItemPresentation SdrMeasureTextVPosItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrMeasureTextVPosItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::MeasureTextVertPos)GetValue();
    return true;
}

bool SdrMeasureTextVPosItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::MeasureTextVertPos ePos;
    if(!(rVal >>= ePos))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        ePos = (drawing::MeasureTextVertPos)nEnum;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( (SdrMeasureTextHPos)ePos ) );
    return true;
}

TYPEINIT1_AUTOFACTORY(SdrMeasureUnitItem,SfxEnumItem);

SfxPoolItem* SdrMeasureUnitItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureUnitItem(*this); }

SfxPoolItem* SdrMeasureUnitItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrMeasureUnitItem(rIn); }

sal_uInt16 SdrMeasureUnitItem::GetValueCount() const { return 14; }

rtl::OUString SdrMeasureUnitItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    XubString aRetval;

    if((FieldUnit)nPos == FUNIT_NONE)
        aRetval = rtl::OUString("default");
    else
        SdrFormatter::TakeUnitStr((FieldUnit)nPos, aRetval);

    return aRetval;
}

SfxItemPresentation SdrMeasureUnitItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrMeasureUnitItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (sal_Int32)GetValue();
    return true;
}

bool SdrMeasureUnitItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nMeasure = 0;
    if(!(rVal >>= nMeasure))
        return false;

    SetValue( sal::static_int_cast< sal_uInt16 >( (FieldUnit)nMeasure ) );
    return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Circ
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1_AUTOFACTORY(SdrCircKindItem,SfxEnumItem);

SfxPoolItem* SdrCircKindItem::Clone(SfxItemPool* /*pPool*/) const          { return new SdrCircKindItem(*this); }

SfxPoolItem* SdrCircKindItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const { return new SdrCircKindItem(rIn); }

sal_uInt16 SdrCircKindItem::GetValueCount() const { return 4; }

rtl::OUString SdrCircKindItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValCIRC_FULL+nPos);
}

SfxItemPresentation SdrCircKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, XubString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        String aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aStr += sal_Unicode(' ');
        rText.Insert(aStr, 0);
    }
    return ePres;
}

bool SdrCircKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= (drawing::CircleKind)GetValue();
    return true;
}

bool SdrCircKindItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    drawing::CircleKind eKind;
    if(!(rVal >>= eKind))
    {
        sal_Int32 nEnum = 0;
        if(!(rVal >>= nEnum))
            return false;

        eKind = (drawing::CircleKind)nEnum;
    }

    SetValue( sal::static_int_cast< sal_uInt16 >( (SdrCircKind)eKind ) );
    return true;
}

//------------------------------------------------------------
// class SdrSignedPercentItem
//------------------------------------------------------------

TYPEINIT1_AUTOFACTORY( SdrSignedPercentItem, SfxInt16Item );


SfxPoolItem* SdrSignedPercentItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrSignedPercentItem( Which(), GetValue() );
}

SfxPoolItem* SdrSignedPercentItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrSignedPercentItem( Which(), rIn );
}

SfxItemPresentation SdrSignedPercentItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
    XubString& rText, const IntlWrapper *) const
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

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafRedItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafRedItem, SdrSignedPercentItem );

SfxPoolItem* SdrGrafRedItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafRedItem( *this );
}

SfxPoolItem* SdrGrafRedItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafRedItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafGreenItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafGreenItem, SdrSignedPercentItem );

SfxPoolItem* SdrGrafGreenItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafGreenItem( *this );
}

SfxPoolItem* SdrGrafGreenItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafGreenItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafBlueItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafBlueItem, SdrSignedPercentItem );

SfxPoolItem* SdrGrafBlueItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafBlueItem( *this );
}

SfxPoolItem* SdrGrafBlueItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafBlueItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafLuminanceItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafLuminanceItem, SdrSignedPercentItem );

SfxPoolItem* SdrGrafLuminanceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafLuminanceItem( *this );
}

SfxPoolItem* SdrGrafLuminanceItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafLuminanceItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafContrastItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafContrastItem, SdrSignedPercentItem );

SfxPoolItem* SdrGrafContrastItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafContrastItem( *this );
}

SfxPoolItem* SdrGrafContrastItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafContrastItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafGamma100Item
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafGamma100Item, SfxUInt32Item );

SfxPoolItem* SdrGrafGamma100Item::Clone( SfxItemPool* /*pPool */) const
{
    return new SdrGrafGamma100Item( *this );
}

SfxPoolItem* SdrGrafGamma100Item::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafGamma100Item( rIn );
}

bool SdrGrafGamma100Item::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= ((double)GetValue()) / 100.0;
    return true;
}

bool SdrGrafGamma100Item::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    double nGamma = 0;
    if(!(rVal >>= nGamma))
        return false;

    SetValue( (sal_uInt32)(nGamma * 100.0  ) );
    return true;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafInvertItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafInvertItem, SdrOnOffItem );

SfxPoolItem* SdrGrafInvertItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafInvertItem( *this );
}

SfxPoolItem* SdrGrafInvertItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafInvertItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafTransparenceItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafTransparenceItem, SdrPercentItem );

SfxPoolItem* SdrGrafTransparenceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafTransparenceItem( *this );
}

SfxPoolItem* SdrGrafTransparenceItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafTransparenceItem( rIn );
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// SdrGrafModeItem
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

TYPEINIT1( SdrGrafModeItem, SfxEnumItem );

SfxPoolItem* SdrGrafModeItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrGrafModeItem( *this );
}

SfxPoolItem* SdrGrafModeItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafModeItem( rIn );
}

sal_uInt16 SdrGrafModeItem::GetValueCount() const
{
    return 4;
}

rtl::OUString SdrGrafModeItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    rtl::OUString aStr;

    switch(nPos)
    {
        case 1:
        {
            aStr = "Greys";
            break;
        }
        case 2:
        {
            aStr = "Black/White";
            break;
        }
        case 3:
        {
            aStr = "Watermark";
            break;
        }
        default:
        {
            aStr = "Standard";
            break;
        }
    }

    return aStr;
}

SfxItemPresentation SdrGrafModeItem::GetPresentation( SfxItemPresentation ePres,
                                                               SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
                                                               XubString& rText, const IntlWrapper *) const
{
    rText = GetValueTextByPos( sal::static_int_cast< sal_uInt16 >( GetValue() ) );

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

SfxPoolItem* SdrGrafCropItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafCropItem( *this );
}

SfxPoolItem* SdrGrafCropItem::Create( SvStream& rIn, sal_uInt16 nVer ) const
{
    return( ( 0 == nVer ) ? Clone( NULL ) : SvxGrfCrop::Create( rIn, nVer ) );
}

sal_uInt16 SdrGrafCropItem::GetVersion( sal_uInt16 /*nFileVersion*/) const
{
    // GRFCROP_VERSION_MOVETOSVX is 1
    return GRFCROP_VERSION_MOVETOSVX;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
