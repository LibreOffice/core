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

#include "editeng/boxitem.hxx"
#include "editeng/lineitem.hxx"
#include "editeng/shaditem.hxx"
#include "editeng/xmlcnitm.hxx"
#include <comphelper/processfactory.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/writingmodeitem.hxx>
#include <i18nutil/unicode.hxx>
#include <svl/solar.hrc>
#include <tools/bigint.hxx>
#include <tools/stream.hxx>
#include <unotools/intlwrapper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include "svdglob.hxx"
#include "svx/svdstr.hrc"

#include <svx/sdgcpitm.hxx>
#include <svx/sdtfchim.hxx>
#include <svx/svdattr.hxx>
#include <svx/svdmodel.hxx>
#include <svx/svdpool.hxx>
#include <svx/svdtrans.hxx>
#include <svx/svx3ditems.hxx>
#include <svx/svxids.hrc>
#include <sxallitm.hxx>
#include <svx/sxcaitm.hxx>
#include <svx/sxcecitm.hxx>
#include <svx/sxcgitm.hxx>
#include <svx/sxciaitm.hxx>
#include <sxcikitm.hxx>
#include <svx/sxcllitm.hxx>
#include <svx/sxctitm.hxx>
#include <svx/sxekitm.hxx>
#include <svx/sxelditm.hxx>
#include <svx/sxenditm.hxx>
#include <svx/sxfiitm.hxx>
#include <sxlayitm.hxx>
#include <sxlogitm.hxx>
#include <svx/sxmbritm.hxx>
#include <svx/sxmfsitm.hxx>
#include <sxmkitm.hxx>
#include <svx/sxmlhitm.hxx>
#include <sxmoitm.hxx>
#include <sxmovitm.hxx>
#include <sxmsitm.hxx>
#include <sxmtaitm.hxx>
#include <svx/sxmtfitm.hxx>
#include <svx/sxmtpitm.hxx>
#include <svx/sxmtritm.hxx>
#include <svx/sxmuitm.hxx>
#include <sxoneitm.hxx>
#include <sxonitm.hxx>
#include <sxopitm.hxx>
#include <sxraitm.hxx>
#include <sxreaitm.hxx>
#include <sxreoitm.hxx>
#include <sxroaitm.hxx>
#include <sxrooitm.hxx>
#include <sxsaitm.hxx>
#include <sxsalitm.hxx>
#include <svx/sxsiitm.hxx>
#include <sxsoitm.hxx>
#include <sxtraitm.hxx>
#include <svx/xfillit0.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xtable.hxx>

using namespace ::com::sun::star;

SdrItemPool::SdrItemPool(
    SfxItemPool* _pMaster,
    bool bLoadRefCounts)
:   XOutdevItemPool(_pMaster, SDRATTR_START, SDRATTR_END, bLoadRefCounts)
{
    // prepare some constants
    const Color aNullCol(RGB_Color(COL_BLACK));
    const sal_Int32 nDefEdgeDist(500L); // Defaulting hard for Draw (100TH_MM) currently. MapMode will have to be taken into account in the future.

    // init the non-persistent items
    for(sal_uInt16 i(SDRATTR_NOTPERSIST_FIRST); i <= SDRATTR_NOTPERSIST_LAST; i++)
    {
        mpLocalItemInfos[i - SDRATTR_START]._bPoolable = false;
    }

    // init own PoolDefaults
    mppLocalPoolDefaults[SDRATTR_SHADOW            -SDRATTR_START]=new SdrOnOffItem(SDRATTR_SHADOW, false);
    mppLocalPoolDefaults[SDRATTR_SHADOWCOLOR       -SDRATTR_START]=new XColorItem(SDRATTR_SHADOWCOLOR, aNullCol);
    mppLocalPoolDefaults[SDRATTR_SHADOWXDIST       -SDRATTR_START]=new SdrMetricItem(SDRATTR_SHADOWXDIST, 0);
    mppLocalPoolDefaults[SDRATTR_SHADOWYDIST       -SDRATTR_START]=new SdrMetricItem(SDRATTR_SHADOWYDIST, 0);
    mppLocalPoolDefaults[SDRATTR_SHADOWTRANSPARENCE-SDRATTR_START]=new SdrPercentItem(SDRATTR_SHADOWTRANSPARENCE, 0);
    mppLocalPoolDefaults[SDRATTR_SHADOW3D          -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOW3D    );
    mppLocalPoolDefaults[SDRATTR_SHADOWPERSP       -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOWPERSP );
    mppLocalPoolDefaults[SDRATTR_CAPTIONTYPE      -SDRATTR_START]=new SdrCaptionTypeItem      ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONFIXEDANGLE-SDRATTR_START]=new SdrOnOffItem(SDRATTR_CAPTIONFIXEDANGLE, true);
    mppLocalPoolDefaults[SDRATTR_CAPTIONANGLE     -SDRATTR_START]=new SdrCaptionAngleItem     ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONGAP       -SDRATTR_START]=new SdrCaptionGapItem       ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONESCDIR    -SDRATTR_START]=new SdrCaptionEscDirItem    ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONESCISREL  -SDRATTR_START]=new SdrCaptionEscIsRelItem  ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONESCREL    -SDRATTR_START]=new SdrCaptionEscRelItem    ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONESCABS    -SDRATTR_START]=new SdrCaptionEscAbsItem    ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONLINELEN   -SDRATTR_START]=new SdrCaptionLineLenItem   ;
    mppLocalPoolDefaults[SDRATTR_CAPTIONFITLINELEN-SDRATTR_START]=new SdrCaptionFitLineLenItem;
    mppLocalPoolDefaults[SDRATTR_ECKENRADIUS            -SDRATTR_START]=new SdrMetricItem(SDRATTR_ECKENRADIUS, 0);
    mppLocalPoolDefaults[SDRATTR_TEXT_MINFRAMEHEIGHT    -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT, 0);
    mppLocalPoolDefaults[SDRATTR_TEXT_AUTOGROWHEIGHT    -SDRATTR_START]=new SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, true);
    mppLocalPoolDefaults[SDRATTR_TEXT_FITTOSIZE     -SDRATTR_START]=new SdrTextFitToSizeTypeItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_LEFTDIST      -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_LEFTDIST, 0);
    mppLocalPoolDefaults[SDRATTR_TEXT_RIGHTDIST     -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_RIGHTDIST, 0);
    mppLocalPoolDefaults[SDRATTR_TEXT_UPPERDIST     -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_UPPERDIST, 0);
    mppLocalPoolDefaults[SDRATTR_TEXT_LOWERDIST     -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_LOWERDIST, 0);
    mppLocalPoolDefaults[SDRATTR_TEXT_VERTADJUST        -SDRATTR_START]=new SdrTextVertAdjustItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_MAXFRAMEHEIGHT    -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_MAXFRAMEHEIGHT, 0);
    mppLocalPoolDefaults[SDRATTR_TEXT_MINFRAMEWIDTH -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH, 0);
    mppLocalPoolDefaults[SDRATTR_TEXT_MAXFRAMEWIDTH -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_MAXFRAMEWIDTH, 0);
    mppLocalPoolDefaults[SDRATTR_TEXT_AUTOGROWWIDTH -SDRATTR_START]=new SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, false);
    mppLocalPoolDefaults[SDRATTR_TEXT_HORZADJUST        -SDRATTR_START]=new SdrTextHorzAdjustItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANIKIND           -SDRATTR_START]=new SdrTextAniKindItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANIDIRECTION  -SDRATTR_START]=new SdrTextAniDirectionItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANISTARTINSIDE    -SDRATTR_START]=new SdrTextAniStartInsideItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANISTOPINSIDE -SDRATTR_START]=new SdrTextAniStopInsideItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANICOUNT      -SDRATTR_START]=new SdrTextAniCountItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANIDELAY      -SDRATTR_START]=new SdrTextAniDelayItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_ANIAMOUNT     -SDRATTR_START]=new SdrTextAniAmountItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_CONTOURFRAME  -SDRATTR_START]=new SdrOnOffItem(SDRATTR_TEXT_CONTOURFRAME, false);
    mppLocalPoolDefaults[SDRATTR_CUSTOMSHAPE_ADJUSTMENT -SDRATTR_START]=new SdrCustomShapeAdjustmentItem;
    mppLocalPoolDefaults[SDRATTR_XMLATTRIBUTES -SDRATTR_START]=new SvXMLAttrContainerItem( SDRATTR_XMLATTRIBUTES );
    mppLocalPoolDefaults[SDRATTR_TEXT_CHAINNEXTNAME    -SDRATTR_START]=new SfxStringItem(SDRATTR_TEXT_CHAINNEXTNAME, "");
    mppLocalPoolDefaults[SDRATTR_TEXT_USEFIXEDCELLHEIGHT -SDRATTR_START]=new SdrTextFixedCellHeightItem;
    mppLocalPoolDefaults[SDRATTR_TEXT_WORDWRAP         -SDRATTR_START]=new SdrOnOffItem(SDRATTR_TEXT_WORDWRAP, true);
    mppLocalPoolDefaults[SDRATTR_TEXT_AUTOGROWSIZE     -SDRATTR_START]=new SdrOnOffItem(SDRATTR_TEXT_AUTOGROWSIZE, false);
    mppLocalPoolDefaults[SDRATTR_EDGEKIND         -SDRATTR_START]=new SdrEdgeKindItem;
    mppLocalPoolDefaults[SDRATTR_EDGENODE1HORZDIST-SDRATTR_START]=new SdrEdgeNode1HorzDistItem(nDefEdgeDist);
    mppLocalPoolDefaults[SDRATTR_EDGENODE1VERTDIST-SDRATTR_START]=new SdrEdgeNode1VertDistItem(nDefEdgeDist);
    mppLocalPoolDefaults[SDRATTR_EDGENODE2HORZDIST-SDRATTR_START]=new SdrEdgeNode2HorzDistItem(nDefEdgeDist);
    mppLocalPoolDefaults[SDRATTR_EDGENODE2VERTDIST-SDRATTR_START]=new SdrEdgeNode2VertDistItem(nDefEdgeDist);
    mppLocalPoolDefaults[SDRATTR_EDGENODE1GLUEDIST-SDRATTR_START]=new SdrEdgeNode1GlueDistItem;
    mppLocalPoolDefaults[SDRATTR_EDGENODE2GLUEDIST-SDRATTR_START]=new SdrEdgeNode2GlueDistItem;
    mppLocalPoolDefaults[SDRATTR_EDGELINEDELTAANZ -SDRATTR_START]=new SdrEdgeLineDeltaCountItem;
    mppLocalPoolDefaults[SDRATTR_EDGELINE1DELTA   -SDRATTR_START]=new SdrMetricItem(SDRATTR_EDGELINE1DELTA, 0);
    mppLocalPoolDefaults[SDRATTR_EDGELINE2DELTA   -SDRATTR_START]=new SdrMetricItem(SDRATTR_EDGELINE2DELTA, 0);
    mppLocalPoolDefaults[SDRATTR_EDGELINE3DELTA   -SDRATTR_START]=new SdrMetricItem(SDRATTR_EDGELINE3DELTA, 0);
    mppLocalPoolDefaults[SDRATTR_MEASUREKIND             -SDRATTR_START]=new SdrMeasureKindItem;
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTHPOS         -SDRATTR_START]=new SdrMeasureTextHPosItem;
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTVPOS         -SDRATTR_START]=new SdrMeasureTextVPosItem;
    mppLocalPoolDefaults[SDRATTR_MEASURELINEDIST         -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASURELINEDIST, 800);
    mppLocalPoolDefaults[SDRATTR_MEASUREHELPLINEOVERHANG -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASUREHELPLINEOVERHANG, 200);
    mppLocalPoolDefaults[SDRATTR_MEASUREHELPLINEDIST     -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASUREHELPLINEDIST, 100);
    mppLocalPoolDefaults[SDRATTR_MEASUREHELPLINE1LEN     -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASUREHELPLINE1LEN, 0);
    mppLocalPoolDefaults[SDRATTR_MEASUREHELPLINE2LEN     -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASUREHELPLINE2LEN, 0);
    mppLocalPoolDefaults[SDRATTR_MEASUREBELOWREFEDGE     -SDRATTR_START]=new SdrMeasureBelowRefEdgeItem;
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTROTA90       -SDRATTR_START]=new SdrMeasureTextRota90Item;
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTUPSIDEDOWN   -SDRATTR_START]=new SdrMeasureTextUpsideDownItem;
    mppLocalPoolDefaults[SDRATTR_MEASUREOVERHANG         -SDRATTR_START]=new SdrMeasureOverhangItem(600);
    mppLocalPoolDefaults[SDRATTR_MEASUREUNIT             -SDRATTR_START]=new SdrMeasureUnitItem;
    mppLocalPoolDefaults[SDRATTR_MEASURESCALE            -SDRATTR_START]=new SdrMeasureScaleItem;
    mppLocalPoolDefaults[SDRATTR_MEASURESHOWUNIT         -SDRATTR_START]=new SdrYesNoItem(SDRATTR_MEASURESHOWUNIT, false);
    mppLocalPoolDefaults[SDRATTR_MEASUREFORMATSTRING     -SDRATTR_START]=new SdrMeasureFormatStringItem();
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLE    -SDRATTR_START]=new SdrMeasureTextAutoAngleItem();
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLEVIEW-SDRATTR_START]=new SdrMeasureTextAutoAngleViewItem();
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTISFIXEDANGLE -SDRATTR_START]=new SdrMeasureTextIsFixedAngleItem();
    mppLocalPoolDefaults[SDRATTR_MEASURETEXTFIXEDANGLE   -SDRATTR_START]=new SdrMeasureTextFixedAngleItem();
    mppLocalPoolDefaults[SDRATTR_MEASUREDECIMALPLACES    -SDRATTR_START]=new SdrMeasureDecimalPlacesItem();
    mppLocalPoolDefaults[SDRATTR_CIRCKIND      -SDRATTR_START]=new SdrCircKindItem;
    mppLocalPoolDefaults[SDRATTR_CIRCSTARTANGLE-SDRATTR_START]=new SdrAngleItem(SDRATTR_CIRCSTARTANGLE, 0);
    mppLocalPoolDefaults[SDRATTR_CIRCENDANGLE  -SDRATTR_START]=new SdrAngleItem(SDRATTR_CIRCENDANGLE, 36000);
    mppLocalPoolDefaults[SDRATTR_OBJMOVEPROTECT -SDRATTR_START]=new SdrYesNoItem(SDRATTR_OBJMOVEPROTECT, false);
    mppLocalPoolDefaults[SDRATTR_OBJSIZEPROTECT -SDRATTR_START]=new SdrYesNoItem(SDRATTR_OBJSIZEPROTECT, false);
    mppLocalPoolDefaults[SDRATTR_OBJPRINTABLE   -SDRATTR_START]=new SdrObjPrintableItem;
    mppLocalPoolDefaults[SDRATTR_OBJVISIBLE     -SDRATTR_START]=new SdrObjVisibleItem;
    mppLocalPoolDefaults[SDRATTR_LAYERID        -SDRATTR_START]=new SdrLayerIdItem;
    mppLocalPoolDefaults[SDRATTR_LAYERNAME      -SDRATTR_START]=new SdrLayerNameItem;
    mppLocalPoolDefaults[SDRATTR_OBJECTNAME     -SDRATTR_START]=new SfxStringItem(SDRATTR_OBJECTNAME);
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
    mppLocalPoolDefaults[SDRATTR_ROTATEANGLE    -SDRATTR_START]=new SdrAngleItem(SDRATTR_ROTATEANGLE, 0);
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
    mppLocalPoolDefaults[SDRATTR_TEXTDIRECTION  -SDRATTR_START]=new SvxWritingModeItem(css::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION);
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
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_PERCENT_DIAGONAL - SDRATTR_START ] = new SfxUInt16Item(SDRATTR_3DOBJ_PERCENT_DIAGONAL, 10);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_BACKSCALE - SDRATTR_START ] = new SfxUInt16Item(SDRATTR_3DOBJ_BACKSCALE, 100);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_DEPTH - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, 1000);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_HORZ_SEGS - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DOBJ_HORZ_SEGS, 24);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_VERT_SEGS - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DOBJ_VERT_SEGS, 24);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_END_ANGLE - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DOBJ_END_ANGLE, 3600);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_DOUBLE_SIDED - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, false);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_NORMALS_KIND - SDRATTR_START ] = new Svx3DNormalsKindItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_NORMALS_INVERT - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DOBJ_NORMALS_INVERT, false);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_X - SDRATTR_START ] = new Svx3DTextureProjectionXItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_Y - SDRATTR_START ] = new Svx3DTextureProjectionYItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_SHADOW_3D - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DOBJ_SHADOW_3D, false);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_MAT_COLOR - SDRATTR_START ] = new SvxColorItem(Color(0x0000b8ff), SDRATTR_3DOBJ_MAT_COLOR);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_MAT_EMISSION - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DOBJ_MAT_EMISSION);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR - SDRATTR_START ] = new SvxColorItem(Color(0x00ffffff), SDRATTR_3DOBJ_MAT_SPECULAR);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY - SDRATTR_START ] = new SfxUInt16Item(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY, 15);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_KIND - SDRATTR_START ] = new Svx3DTextureKindItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_MODE - SDRATTR_START ] = new Svx3DTextureModeItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_FILTER - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DOBJ_TEXTURE_FILTER, false);
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_NORMALS - SDRATTR_START ] = new Svx3DSmoothNormalsItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_LIDS - SDRATTR_START ] = new Svx3DSmoothLidsItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_CHARACTER_MODE - SDRATTR_START ] = new Svx3DCharacterModeItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_CLOSE_FRONT - SDRATTR_START ] = new Svx3DCloseFrontItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_CLOSE_BACK - SDRATTR_START ] = new Svx3DCloseBackItem;
    mppLocalPoolDefaults[ SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY - SDRATTR_START ] = new Svx3DReducedLineGeometryItem;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_PERSPECTIVE - SDRATTR_START ] = new Svx3DPerspectiveItem;
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_DISTANCE - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DSCENE_DISTANCE, 100);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_FOCAL_LENGTH - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DSCENE_FOCAL_LENGTH, 100);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_TWO_SIDED_LIGHTING - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, false);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_1 - SDRATTR_START ] = new SvxColorItem(Color(0xffcccccc), SDRATTR_3DSCENE_LIGHTCOLOR_1);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_2 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_2);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_3 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_3);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_4 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_4);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_5 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_5);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_6 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_6);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_7 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_7);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_8 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_8);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_AMBIENTCOLOR - SDRATTR_START ] = new SvxColorItem(Color(0x00666666), SDRATTR_3DSCENE_AMBIENTCOLOR);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_1 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_1, true);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_2 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_2, false);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_3 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_3, false);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_4 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_4, false);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_5 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_5, false);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_6 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_6, false);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_7 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_7, false);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_8 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_8, false);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_1 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_1, basegfx::B3DVector(0.57735026918963, 0.57735026918963, 0.57735026918963));
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_2 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_2, basegfx::B3DVector(0.0,0.0,1.0));
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_3 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_3, basegfx::B3DVector(0.0,0.0,1.0));
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_4 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_4, basegfx::B3DVector(0.0,0.0,1.0));
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_5 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_5, basegfx::B3DVector(0.0,0.0,1.0));
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_6 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_6, basegfx::B3DVector(0.0,0.0,1.0));
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_7 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_7, basegfx::B3DVector(0.0,0.0,1.0));
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_8 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_8, basegfx::B3DVector(0.0,0.0,1.0));
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_SHADOW_SLANT - SDRATTR_START ] = new SfxUInt16Item(SDRATTR_3DSCENE_SHADOW_SLANT, 0);
    mppLocalPoolDefaults[ SDRATTR_3DSCENE_SHADE_MODE - SDRATTR_START ] = new Svx3DShadeModeItem;
    mppLocalPoolDefaults[ SDRATTR_CUSTOMSHAPE_ENGINE - SDRATTR_START ] = new SfxStringItem(SDRATTR_CUSTOMSHAPE_ENGINE, "");
    mppLocalPoolDefaults[ SDRATTR_CUSTOMSHAPE_DATA - SDRATTR_START ] = new SfxStringItem(SDRATTR_CUSTOMSHAPE_DATA, "");
    mppLocalPoolDefaults[ SDRATTR_CUSTOMSHAPE_GEOMETRY - SDRATTR_START ] = new SdrCustomShapeGeometryItem;
    mppLocalPoolDefaults[ SDRATTR_CUSTOMSHAPE_REPLACEMENT_URL - SDRATTR_START ] = new SdrCustomShapeReplacementURLItem;

    SvxBoxItem* pboxItem = new SvxBoxItem( SDRATTR_TABLE_BORDER );
    pboxItem->SetDistance( 100 );
    mppLocalPoolDefaults[ SDRATTR_TABLE_BORDER - SDRATTR_START ] = pboxItem;

    SvxBoxInfoItem* pBoxInfoItem = new SvxBoxInfoItem( SDRATTR_TABLE_BORDER_INNER );

    pBoxInfoItem->SetTable( true );
    pBoxInfoItem->SetDist( true);        // always show margin field
    pBoxInfoItem->SetValid( SvxBoxInfoItemValidFlags::DISABLE ); // some lines may have DontCare state only in tables

    mppLocalPoolDefaults[ SDRATTR_TABLE_BORDER_INNER - SDRATTR_START ] =  pBoxInfoItem;
    mppLocalPoolDefaults[ SDRATTR_TABLE_BORDER_TLBR - SDRATTR_START ] = new SvxLineItem( SDRATTR_TABLE_BORDER_TLBR );
    mppLocalPoolDefaults[ SDRATTR_TABLE_BORDER_BLTR - SDRATTR_START ] = new SvxLineItem( SDRATTR_TABLE_BORDER_BLTR );

    // set own ItemInfos
    mpLocalItemInfos[SDRATTR_SHADOW-SDRATTR_START]._nSID=SID_ATTR_FILL_SHADOW;
    mpLocalItemInfos[SDRATTR_SHADOWCOLOR-SDRATTR_START]._nSID=SID_ATTR_SHADOW_COLOR;
    mpLocalItemInfos[SDRATTR_SHADOWTRANSPARENCE-SDRATTR_START]._nSID=SID_ATTR_SHADOW_TRANSPARENCE;
    mpLocalItemInfos[SDRATTR_SHADOWXDIST-SDRATTR_START]._nSID=SID_ATTR_SHADOW_XDISTANCE;
    mpLocalItemInfos[SDRATTR_SHADOWYDIST-SDRATTR_START]._nSID=SID_ATTR_SHADOW_YDISTANCE;
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

// copy ctor, so that static defaults are cloned
//            (Parameter 2 = sal_True)
SdrItemPool::SdrItemPool(const SdrItemPool& rPool)
:   XOutdevItemPool(rPool)
{
}

SfxItemPool* SdrItemPool::Clone() const
{
    return new SdrItemPool(*this);
}

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
            mppLocalPoolDefaults[i] = nullptr;
        }
    }

    // split pools before destroying
    SetSecondaryPool(nullptr);
}

bool SdrItemPool::GetPresentation(
              const SfxPoolItem& rItem,
              SfxMapUnit ePresentationMetric, OUString& rText,
              const IntlWrapper * pIntlWrapper) const
{
    if (!IsInvalidItem(&rItem)) {
        sal_uInt16 nWhich=rItem.Which();
        if (nWhich>=SDRATTR_SHADOW_FIRST && nWhich<=SDRATTR_END) {
            rItem.GetPresentation(SFX_ITEM_PRESENTATION_NAMELESS,
                        GetMetric(nWhich),ePresentationMetric,rText,
                        pIntlWrapper);
            OUString aStr;

            TakeItemName(nWhich, aStr);
            rText = aStr + " " + rText;

            return true;
        }
    }
    return XOutdevItemPool::GetPresentation(rItem,ePresentationMetric,rText,pIntlWrapper);
}

void SdrItemPool::TakeItemName(sal_uInt16 nWhich, OUString& rItemName)
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
        case SDRATTR_TEXT_CHAINNEXTNAME     : nResId = SIP_SA_CHAINNEXTNAME;break;
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

    rItemName = ResId( nResId, *pResMgr );
}


// FractionItem


SdrFractionItem::SdrFractionItem(sal_uInt16 nId, SvStream& rIn):
    SfxPoolItem(nId)
{
    sal_Int32 nMul,nDiv;
    rIn.ReadInt32( nMul );
    rIn.ReadInt32( nDiv );
    nValue=Fraction(nMul,nDiv);
}

bool SdrFractionItem::operator==(const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
           static_cast<const SdrFractionItem&>(rCmp).GetValue()==nValue;
}

bool SdrFractionItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit /*eCoreMetric*/,
    SfxMapUnit /*ePresentationMetric*/, OUString &rText, const IntlWrapper *) const
{
    if(nValue.IsValid())
    {
        sal_Int32 nDiv = nValue.GetDenominator();
        rText = OUString::number(nValue.GetNumerator());

        if(nDiv != 1)
        {
            rText = rText + "/" + OUString::number(nDiv);
        }
    }
    else
    {
        rText = "?";
    }

    if(ePresentation == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
        return true;
    }
    else if(ePresentation == SFX_ITEM_PRESENTATION_NAMELESS)
        return true;

    return false;
}

SfxPoolItem* SdrFractionItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrFractionItem(Which(),rIn);
}

SvStream& SdrFractionItem::Store(SvStream& rOut, sal_uInt16 /*nItemVers*/) const
{
    rOut.WriteInt32( nValue.GetNumerator() );
    rOut.WriteInt32( nValue.GetDenominator() );
    return rOut;
}

SfxPoolItem* SdrFractionItem::Clone(SfxItemPool * /*pPool*/) const
{
    return new SdrFractionItem(Which(),GetValue());
}


// ScaleItem


bool SdrScaleItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit /*eCoreMetric*/,
    SfxMapUnit /*ePresentationMetric*/, OUString &rText, const IntlWrapper *) const
{
    if(GetValue().IsValid())
    {
        sal_Int32 nDiv = GetValue().GetDenominator();

        rText = OUString::number(GetValue().GetNumerator()) + ":" + OUString::number(nDiv);
    }
    else
    {
        rText = "?";
    }

    if(ePresentation == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }

    return true;
}

SfxPoolItem* SdrScaleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrScaleItem(Which(),rIn);
}

SfxPoolItem* SdrScaleItem::Clone(SfxItemPool * /*pPool*/) const
{
    return new SdrScaleItem(Which(),GetValue());
}


// OnOffItem


SfxPoolItem* SdrOnOffItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrOnOffItem(Which(),GetValue());
}

SfxPoolItem* SdrOnOffItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrOnOffItem(Which(),rIn);
}

OUString SdrOnOffItem::GetValueTextByVal(bool bVal) const
{
    if (bVal)
        return ImpGetResStr(STR_ItemValON);
    return ImpGetResStr(STR_ItemValOFF);
}

bool SdrOnOffItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByVal(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}


SfxPoolItem* SdrYesNoItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrYesNoItem(Which(),GetValue());
}

SfxPoolItem* SdrYesNoItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrYesNoItem(Which(),rIn);
}

OUString SdrYesNoItem::GetValueTextByVal(bool bVal) const
{
    if (bVal)
        return ImpGetResStr(STR_ItemValYES);
    return ImpGetResStr(STR_ItemValNO);
}

bool SdrYesNoItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByVal(GetValue());
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}


// class SdrPercentItem


SfxPoolItem* SdrPercentItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrPercentItem(Which(),GetValue());
}

SfxPoolItem* SdrPercentItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrPercentItem(Which(),rIn);
}

bool SdrPercentItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreMetric*/,
    SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText = unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }

    return true;
}


// class SdrAngleItem


SfxPoolItem* SdrAngleItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrAngleItem(Which(),GetValue());
}

SfxPoolItem* SdrAngleItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrAngleItem(Which(),rIn);
}

bool SdrAngleItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
    OUString& rText, const IntlWrapper * pIntlWrapper) const
{
    sal_Int32 nValue(GetValue());
    bool bNeg(nValue < 0);

    if(bNeg)
        nValue = -nValue;

    OUStringBuffer aText = OUString::number(nValue);

    if(nValue)
    {
        sal_Unicode aUnicodeNull('0');
        sal_Int32 nCount(2);

        const IntlWrapper* pMyIntlWrapper = nullptr;
        if(!pIntlWrapper)
            pIntlWrapper = pMyIntlWrapper = new IntlWrapper(
                Application::GetSettings().GetLanguageTag() );

        if(LocaleDataWrapper::isNumLeadingZero())
            nCount++;

        while(aText.getLength() < nCount)
            aText.insert(0, aUnicodeNull);

        sal_Int32 nLen = aText.getLength();
        bool bNull1(aText[nLen-1] == aUnicodeNull);
        bool bNull2(bNull1 && aText[nLen-2] == aUnicodeNull);

        if(bNull2)
        {
            // no decimal place(s)
            sal_Int32 idx = nLen-2;
            aText.remove(idx, aText.getLength()-idx);
        }
        else
        {
            sal_Unicode cDec =
                pIntlWrapper->getLocaleData()->getNumDecimalSep()[0];
            aText.insert(nLen-2, cDec);

            if(bNull1)
                aText.remove(nLen, aText.getLength()-nLen);
        }

        if(bNeg)
            aText.insert(0, '-');

        if ( pMyIntlWrapper )
        {
            delete pMyIntlWrapper;
            pIntlWrapper = nullptr;
        }
    }

    aText.insert(aText.getLength(), sal_Unicode(DEGREE_CHAR));

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        aText.insert(0, ' ');
        aText.insert(0, aStr);
    }

    rText = aText.makeStringAndClear();
    return true;
}


// class SdrMetricItem


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
    return true;
}

void SdrMetricItem::ScaleMetrics(long nMul, long nDiv)
{
    if (GetValue()!=0) {
        BigInt aVal(GetValue());
        aVal*=nMul;
        aVal+=nDiv/2; // to round accurately
        aVal/=nDiv;
        SetValue(long(aVal));
    }
}

bool SdrMetricItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric, OUString& rText, const IntlWrapper *) const
{
    long nValue=GetValue();
    SdrFormatter aFmt((MapUnit)eCoreMetric,(MapUnit)ePresMetric);
    aFmt.TakeStr(nValue,rText);
    OUString aStr;
    SdrFormatter::TakeUnitStr((MapUnit)ePresMetric,aStr);
    rText += " " + aStr;
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr2;

        SdrItemPool::TakeItemName(Which(), aStr2);
        rText = aStr2 + " " + rText;
    }
    return true;
}


// items of the legend object


SfxPoolItem* SdrCaptionTypeItem::Clone(SfxItemPool* /*pPool*/) const                { return new SdrCaptionTypeItem(*this); }

SfxPoolItem* SdrCaptionTypeItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const       { return new SdrCaptionTypeItem(rIn); }

sal_uInt16 SdrCaptionTypeItem::GetValueCount() const { return 4; }

OUString SdrCaptionTypeItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValCAPTIONTYPE1+nPos);
}

bool SdrCaptionTypeItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}


SfxPoolItem* SdrCaptionEscDirItem::Clone(SfxItemPool* /*pPool*/) const              { return new SdrCaptionEscDirItem(*this); }

SfxPoolItem* SdrCaptionEscDirItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const     { return new SdrCaptionEscDirItem(rIn); }

sal_uInt16 SdrCaptionEscDirItem::GetValueCount() const { return 3; }

OUString SdrCaptionEscDirItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValCAPTIONESCHORI+nPos);
}

bool SdrCaptionEscDirItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}


// MiscItems


// FitToSize

SfxPoolItem* SdrTextFitToSizeTypeItem::CreateDefault() { return new SdrTextFitToSizeTypeItem; }

SfxPoolItem* SdrTextFitToSizeTypeItem::Clone(SfxItemPool* /*pPool*/) const         { return new SdrTextFitToSizeTypeItem(*this); }

SfxPoolItem* SdrTextFitToSizeTypeItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const { return new SdrTextFitToSizeTypeItem(rIn); }

sal_uInt16 SdrTextFitToSizeTypeItem::GetValueCount() const { return 4; }

OUString SdrTextFitToSizeTypeItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValFITTOSIZENONE+nPos);
}

bool SdrTextFitToSizeTypeItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrTextFitToSizeTypeItem::HasBoolValue() const { return true; }

bool SdrTextFitToSizeTypeItem::GetBoolValue() const { return GetValue()!=SDRTEXTFIT_NONE; }

void SdrTextFitToSizeTypeItem::SetBoolValue(bool bVal)
{
    SetValue(sal::static_int_cast< sal_uInt16 >(bVal ? SDRTEXTFIT_PROPORTIONAL : SDRTEXTFIT_NONE));
}

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


SfxPoolItem* SdrTextVertAdjustItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextVertAdjustItem(*this); }

SfxPoolItem* SdrTextVertAdjustItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextVertAdjustItem(rIn); }

sal_uInt16 SdrTextVertAdjustItem::GetValueCount() const { return 5; }

OUString SdrTextVertAdjustItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTVADJTOP+nPos);
}

bool SdrTextVertAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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


SfxPoolItem* SdrTextHorzAdjustItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextHorzAdjustItem(*this); }

SfxPoolItem* SdrTextHorzAdjustItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextHorzAdjustItem(rIn); }

sal_uInt16 SdrTextHorzAdjustItem::GetValueCount() const { return 5; }

OUString SdrTextHorzAdjustItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTHADJLEFT+nPos);
}

bool SdrTextHorzAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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


SfxPoolItem* SdrTextAniKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniKindItem(*this); }

SfxPoolItem* SdrTextAniKindItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextAniKindItem(rIn); }

sal_uInt16 SdrTextAniKindItem::GetValueCount() const { return 5; }

OUString SdrTextAniKindItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTANI_NONE+nPos);
}

bool SdrTextAniKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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


SfxPoolItem* SdrTextAniDirectionItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniDirectionItem(*this); }

SfxPoolItem* SdrTextAniDirectionItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextAniDirectionItem(rIn); }

sal_uInt16 SdrTextAniDirectionItem::GetValueCount() const { return 4; }

OUString SdrTextAniDirectionItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValTEXTANI_LEFT+nPos);
}

bool SdrTextAniDirectionItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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


SfxPoolItem* SdrTextAniDelayItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniDelayItem(*this); }

SfxPoolItem* SdrTextAniDelayItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextAniDelayItem(rIn); }

bool SdrTextAniDelayItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
    OUString& rText, const IntlWrapper *) const
{
    rText = OUString::number(GetValue()) + "ms";

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }

    return true;
}


SfxPoolItem* SdrTextAniAmountItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniAmountItem(*this); }

SfxPoolItem* SdrTextAniAmountItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrTextAniAmountItem(rIn); }

bool SdrTextAniAmountItem::HasMetrics() const
{
    return GetValue()>0;
}

void SdrTextAniAmountItem::ScaleMetrics(long nMul, long nDiv)
{
    if (GetValue()>0) {
        BigInt aVal(GetValue());
        aVal*=nMul;
        aVal+=nDiv/2; // to round accurately
        aVal/=nDiv;
        SetValue(short(aVal));
    }
}

bool SdrTextAniAmountItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit eCoreMetric, SfxMapUnit ePresMetric,
    OUString& rText, const IntlWrapper *) const
{
    sal_Int32 nValue(GetValue());

    if(!nValue)
        nValue = -1L;

    if(nValue < 0)
    {
        rText = OUString::number(-nValue) + "pixel";
    }
    else
    {
        SdrFormatter aFmt((MapUnit)eCoreMetric, (MapUnit)ePresMetric);
        OUString aStr;

        aFmt.TakeStr(nValue, rText);
        SdrFormatter::TakeUnitStr((MapUnit)ePresMetric, aStr);
        rText += aStr;
    }

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }

    return true;
}


SdrTextFixedCellHeightItem::SdrTextFixedCellHeightItem( bool bUseFixedCellHeight )
    : SfxBoolItem( SDRATTR_TEXT_USEFIXEDCELLHEIGHT, bUseFixedCellHeight )
{
}
SdrTextFixedCellHeightItem::SdrTextFixedCellHeightItem( SvStream & rStream, sal_uInt16 nVersion )
    : SfxBoolItem( SDRATTR_TEXT_USEFIXEDCELLHEIGHT, false )
{
    if ( nVersion )
    {
        bool bValue;
        rStream.ReadCharAsBool( bValue );
        SetValue( bValue );
    }
}
bool SdrTextFixedCellHeightItem::GetPresentation( SfxItemPresentation ePres,
                                    SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresentationMetric*/,
                                    OUString &rText, const IntlWrapper * ) const
{
    rText = GetValueTextByVal( GetValue() );
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE)
    {
        OUString aStr;
        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}
SfxPoolItem* SdrTextFixedCellHeightItem::Create( SvStream& rIn, sal_uInt16 nItemVersion ) const
{
    return new SdrTextFixedCellHeightItem( rIn, nItemVersion );
}
SvStream& SdrTextFixedCellHeightItem::Store( SvStream& rOut, sal_uInt16 nItemVersion ) const
{
    if ( nItemVersion )
    {
        bool bValue = GetValue();
        rOut.WriteBool( bValue );
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
    bool bValue = GetValue();
    rVal <<= bValue;
    return true;
}
bool SdrTextFixedCellHeightItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    bool bValue;
    if( !( rVal >>= bValue ) )
        return false;
    SetValue( bValue );
    return true;
}


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
        rIn.ReadUInt32( nCount );
        for ( i = 0; i < nCount; i++ )
        {
            rIn.ReadUInt32( aVal.nValue );
            SetValue( i, aVal );
        }
    }
}

SdrCustomShapeAdjustmentItem::~SdrCustomShapeAdjustmentItem()
{
}

bool SdrCustomShapeAdjustmentItem::operator==( const SfxPoolItem& rCmp ) const
{
    bool bRet = SfxPoolItem::operator==( rCmp );
    if ( bRet )
    {
        bRet = GetCount() == static_cast<const SdrCustomShapeAdjustmentItem&>(rCmp).GetCount();

        if (bRet)
        {
            for (sal_uInt32 i = 0; i < GetCount(); ++i)
                if (aAdjustmentValueList[i].nValue != static_cast<const SdrCustomShapeAdjustmentItem&>(rCmp).aAdjustmentValueList[i].nValue)
                    return false;
        }
    }
    return bRet;
}

bool SdrCustomShapeAdjustmentItem::GetPresentation(
    SfxItemPresentation ePresentation, SfxMapUnit /*eCoreMetric*/,
    SfxMapUnit /*ePresentationMetric*/, OUString &rText, const IntlWrapper *) const
{
    sal_uInt32 i, nCount = GetCount();
    rText += OUString::number( nCount );
    for ( i = 0; i < nCount; i++ )
    {
        rText = rText + " " + OUString::number( GetValue( i ).nValue );
    }
    if ( ePresentation == SFX_ITEM_PRESENTATION_COMPLETE )
    {
        OUString aStr;

        SdrItemPool::TakeItemName( Which(), aStr );
        rText = aStr + " " + rText;
    }
    return true;
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
        rOut.WriteUInt32( nCount );
        for ( i = 0; i < nCount; i++ )
            rOut.WriteUInt32( GetValue( i ).nValue );
    }
    return rOut;
}

SfxPoolItem* SdrCustomShapeAdjustmentItem::Clone( SfxItemPool * /*pPool*/) const
{
    SdrCustomShapeAdjustmentItem* pItem = new SdrCustomShapeAdjustmentItem;
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

// EdgeKind

SfxPoolItem* SdrEdgeKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrEdgeKindItem(*this); }

SfxPoolItem* SdrEdgeKindItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrEdgeKindItem(rIn); }

sal_uInt16 SdrEdgeKindItem::GetValueCount() const { return 4; }

OUString SdrEdgeKindItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValEDGE_ORTHOLINES+nPos);
}

bool SdrEdgeKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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

SfxPoolItem* SdrEdgeNode1HorzDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode1HorzDistItem(*this);
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

SfxPoolItem* SdrEdgeNode1VertDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode1VertDistItem(*this);
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

SfxPoolItem* SdrEdgeNode2HorzDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode2HorzDistItem(*this);
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

SfxPoolItem* SdrEdgeNode2VertDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode2VertDistItem(*this);
}

SfxPoolItem* SdrEdgeNode1GlueDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode1GlueDistItem(*this);
}

SfxPoolItem* SdrEdgeNode2GlueDistItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrEdgeNode2GlueDistItem(*this);
}

SfxPoolItem* SdrMeasureKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureKindItem(*this); }

SfxPoolItem* SdrMeasureKindItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrMeasureKindItem(rIn); }

sal_uInt16 SdrMeasureKindItem::GetValueCount() const { return 2; }

OUString SdrMeasureKindItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValMEASURE_STD+nPos);
}

bool SdrMeasureKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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


SfxPoolItem* SdrMeasureTextHPosItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureTextHPosItem(*this); }

SfxPoolItem* SdrMeasureTextHPosItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrMeasureTextHPosItem(rIn); }

sal_uInt16 SdrMeasureTextHPosItem::GetValueCount() const { return 4; }

OUString SdrMeasureTextHPosItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValMEASURE_TEXTHAUTO+nPos);
}

bool SdrMeasureTextHPosItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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


SfxPoolItem* SdrMeasureTextVPosItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureTextVPosItem(*this); }

SfxPoolItem* SdrMeasureTextVPosItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrMeasureTextVPosItem(rIn); }

sal_uInt16 SdrMeasureTextVPosItem::GetValueCount() const { return 5; }

OUString SdrMeasureTextVPosItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValMEASURE_TEXTVAUTO+nPos);
}

bool SdrMeasureTextVPosItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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


SfxPoolItem* SdrMeasureUnitItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureUnitItem(*this); }

SfxPoolItem* SdrMeasureUnitItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const   { return new SdrMeasureUnitItem(rIn); }

sal_uInt16 SdrMeasureUnitItem::GetValueCount() const { return 14; }

OUString SdrMeasureUnitItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    OUString aRetval;

    if((FieldUnit)nPos == FUNIT_NONE)
        aRetval = "default";
    else
        SdrFormatter::TakeUnitStr((FieldUnit)nPos, aRetval);

    return aRetval;
}

bool SdrMeasureUnitItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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


SfxPoolItem* SdrCircKindItem::Clone(SfxItemPool* /*pPool*/) const          { return new SdrCircKindItem(*this); }

SfxPoolItem* SdrCircKindItem::Create(SvStream& rIn, sal_uInt16 /*nVer*/) const { return new SdrCircKindItem(rIn); }

sal_uInt16 SdrCircKindItem::GetValueCount() const { return 4; }

OUString SdrCircKindItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    return ImpGetResStr(STR_ItemValCIRC_FULL+nPos);
}

bool SdrCircKindItem::GetPresentation(SfxItemPresentation ePres,
                      SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper *) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SFX_ITEM_PRESENTATION_COMPLETE) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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


// class SdrSignedPercentItem


SfxPoolItem* SdrSignedPercentItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrSignedPercentItem( Which(), GetValue() );
}

SfxPoolItem* SdrSignedPercentItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrSignedPercentItem( Which(), rIn );
}

bool SdrSignedPercentItem::GetPresentation(
    SfxItemPresentation ePres, SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
    OUString& rText, const IntlWrapper *) const
{
    rText = unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());

    if(ePres == SFX_ITEM_PRESENTATION_COMPLETE)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }

    return true;
}


SfxPoolItem* SdrGrafRedItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafRedItem( *this );
}

SfxPoolItem* SdrGrafRedItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafRedItem( rIn );
}


SfxPoolItem* SdrGrafGreenItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafGreenItem( *this );
}

SfxPoolItem* SdrGrafGreenItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafGreenItem( rIn );
}


SfxPoolItem* SdrGrafBlueItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafBlueItem( *this );
}

SfxPoolItem* SdrGrafBlueItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafBlueItem( rIn );
}


SfxPoolItem* SdrGrafLuminanceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafLuminanceItem( *this );
}

SfxPoolItem* SdrGrafLuminanceItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafLuminanceItem( rIn );
}


SfxPoolItem* SdrGrafContrastItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafContrastItem( *this );
}

SfxPoolItem* SdrGrafContrastItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafContrastItem( rIn );
}


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


SfxPoolItem* SdrGrafInvertItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafInvertItem( *this );
}

SfxPoolItem* SdrGrafInvertItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafInvertItem( rIn );
}


SfxPoolItem* SdrGrafTransparenceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafTransparenceItem( *this );
}

SfxPoolItem* SdrGrafTransparenceItem::Create( SvStream& rIn, sal_uInt16 /*nVer*/) const
{
    return new SdrGrafTransparenceItem( rIn );
}


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

OUString SdrGrafModeItem::GetValueTextByPos(sal_uInt16 nPos) const
{
    OUString aStr;

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

bool SdrGrafModeItem::GetPresentation( SfxItemPresentation ePres,
                                                               SfxMapUnit /*eCoreMetric*/, SfxMapUnit /*ePresMetric*/,
                                                               OUString& rText, const IntlWrapper *) const
{
    rText = GetValueTextByPos( sal::static_int_cast< sal_uInt16 >( GetValue() ) );

    if( ePres == SFX_ITEM_PRESENTATION_COMPLETE )
    {
        OUString aStr;

        SdrItemPool::TakeItemName( Which(), aStr );
        rText = aStr + " " + rText;
    }

    return true;
}


SfxPoolItem* SdrGrafCropItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafCropItem( *this );
}

SfxPoolItem* SdrGrafCropItem::Create( SvStream& rIn, sal_uInt16 nVer ) const
{
    return( ( 0 == nVer ) ? Clone() : SvxGrfCrop::Create( rIn, nVer ) );
}

sal_uInt16 SdrGrafCropItem::GetVersion( sal_uInt16 /*nFileVersion*/) const
{
    // GRFCROP_VERSION_MOVETOSVX is 1
    return GRFCROP_VERSION_MOVETOSVX;
}
SdrTextAniStartInsideItem::~SdrTextAniStartInsideItem()
{
}
SfxPoolItem* SdrTextAniStartInsideItem::Clone(SfxItemPool* ) const
{
    return new SdrTextAniStartInsideItem(*this);
}
SdrTextAniStopInsideItem::~SdrTextAniStopInsideItem()
{
}
SfxPoolItem* SdrTextAniStopInsideItem::Clone(SfxItemPool* ) const
{
    return new SdrTextAniStopInsideItem(*this);
}
SdrCaptionEscIsRelItem::~SdrCaptionEscIsRelItem()
{
}
SfxPoolItem* SdrCaptionEscIsRelItem::Clone(SfxItemPool* ) const
{
    return new SdrCaptionEscIsRelItem(*this);
}
SdrCaptionEscRelItem::~SdrCaptionEscRelItem()
{
}
SfxPoolItem* SdrCaptionEscRelItem::Clone(SfxItemPool*) const
{
    return new SdrCaptionEscRelItem(*this);
}
SdrCaptionFitLineLenItem::~SdrCaptionFitLineLenItem()
{
}
SfxPoolItem* SdrCaptionFitLineLenItem::Clone(SfxItemPool* ) const
{
    return new SdrCaptionFitLineLenItem(*this);
}
SdrCaptionLineLenItem::~SdrCaptionLineLenItem()
{
}
SfxPoolItem* SdrCaptionLineLenItem::Clone(SfxItemPool*) const
{
    return new SdrCaptionLineLenItem(*this);
}
SdrMeasureBelowRefEdgeItem::~SdrMeasureBelowRefEdgeItem()
{
}
SfxPoolItem* SdrMeasureBelowRefEdgeItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureBelowRefEdgeItem(*this);
}
SdrMeasureTextIsFixedAngleItem::~SdrMeasureTextIsFixedAngleItem()
{
}
SfxPoolItem* SdrMeasureTextIsFixedAngleItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureTextIsFixedAngleItem(*this);
}
SdrMeasureTextFixedAngleItem::~SdrMeasureTextFixedAngleItem()
{
}
SfxPoolItem* SdrMeasureTextFixedAngleItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureTextFixedAngleItem(*this);
}
SdrMeasureDecimalPlacesItem::~SdrMeasureDecimalPlacesItem()
{
}
SfxPoolItem* SdrMeasureDecimalPlacesItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureDecimalPlacesItem(*this);
}
SdrMeasureTextRota90Item::~SdrMeasureTextRota90Item()
{
}
SfxPoolItem* SdrMeasureTextRota90Item::Clone(SfxItemPool* ) const
{
    return new SdrMeasureTextRota90Item(*this);
}
SdrMeasureTextUpsideDownItem::~SdrMeasureTextUpsideDownItem()
{
}
SfxPoolItem* SdrMeasureTextUpsideDownItem::Clone(SfxItemPool* ) const
{
    return new SdrMeasureTextUpsideDownItem(*this);
}
SdrCustomShapeReplacementURLItem::~SdrCustomShapeReplacementURLItem()
{
}
SfxPoolItem* SdrCustomShapeReplacementURLItem::Clone( SfxItemPool*) const
{
    return new SdrCustomShapeReplacementURLItem(*this);
}

SfxPoolItem* SdrLayerIdItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrLayerIdItem(*this);
}

SfxPoolItem* SdrLayerNameItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrLayerNameItem(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
