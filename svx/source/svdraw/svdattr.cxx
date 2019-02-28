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

#include <editeng/boxitem.hxx>
#include <editeng/lineitem.hxx>
#include <editeng/shaditem.hxx>
#include <editeng/xmlcnitm.hxx>
#include <editeng/adjustitem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/writingmodeitem.hxx>
#include <editeng/charrotateitem.hxx>
#include <osl/diagnose.h>
#include <i18nutil/unicode.hxx>
#include <svl/solar.hrc>
#include <tools/bigint.hxx>
#include <tools/stream.hxx>
#include <unotools/intlwrapper.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

#include <svx/strings.hrc>
#include <svx/dialmgr.hxx>
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
#include <sxopitm.hxx>
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
#include <libxml/xmlwriter.h>

using namespace ::com::sun::star;

SdrItemPool::SdrItemPool(
    SfxItemPool* _pMaster)
:   XOutdevItemPool(_pMaster)
{
    // prepare some constants
    const Color aNullCol(COL_BLACK);
    const sal_Int32 nDefEdgeDist(500); // Defaulting hard for Draw (100TH_MM) currently. MapMode will have to be taken into account in the future.

    // init the non-persistent items
    for(sal_uInt16 i(SDRATTR_NOTPERSIST_FIRST); i <= SDRATTR_NOTPERSIST_LAST; i++)
    {
        mpLocalItemInfos[i - SDRATTR_START]._bPoolable = false;
    }

    // init own PoolDefaults
    std::vector<SfxPoolItem*>& rPoolDefaults = *mpLocalPoolDefaults;
    rPoolDefaults[SDRATTR_SHADOW            -SDRATTR_START]=new SdrOnOffItem(SDRATTR_SHADOW, false);
    rPoolDefaults[SDRATTR_SHADOWCOLOR       -SDRATTR_START]=new XColorItem(SDRATTR_SHADOWCOLOR, aNullCol);
    rPoolDefaults[SDRATTR_SHADOWXDIST       -SDRATTR_START]=new SdrMetricItem(SDRATTR_SHADOWXDIST, 0);
    rPoolDefaults[SDRATTR_SHADOWYDIST       -SDRATTR_START]=new SdrMetricItem(SDRATTR_SHADOWYDIST, 0);
    rPoolDefaults[SDRATTR_SHADOWTRANSPARENCE-SDRATTR_START]=new SdrPercentItem(SDRATTR_SHADOWTRANSPARENCE, 0);
    rPoolDefaults[SDRATTR_SHADOW3D          -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOW3D    );
    rPoolDefaults[SDRATTR_SHADOWPERSP       -SDRATTR_START]=new SfxVoidItem(SDRATTR_SHADOWPERSP );
    rPoolDefaults[SDRATTR_CAPTIONTYPE      -SDRATTR_START]=new SdrCaptionTypeItem      ;
    rPoolDefaults[SDRATTR_CAPTIONFIXEDANGLE-SDRATTR_START]=new SdrOnOffItem(SDRATTR_CAPTIONFIXEDANGLE, true);
    rPoolDefaults[SDRATTR_CAPTIONANGLE     -SDRATTR_START]=new SdrCaptionAngleItem     ;
    rPoolDefaults[SDRATTR_CAPTIONGAP       -SDRATTR_START]=new SdrCaptionGapItem       ;
    rPoolDefaults[SDRATTR_CAPTIONESCDIR    -SDRATTR_START]=new SdrCaptionEscDirItem    ;
    rPoolDefaults[SDRATTR_CAPTIONESCISREL  -SDRATTR_START]=new SdrCaptionEscIsRelItem  ;
    rPoolDefaults[SDRATTR_CAPTIONESCREL    -SDRATTR_START]=new SdrCaptionEscRelItem    ;
    rPoolDefaults[SDRATTR_CAPTIONESCABS    -SDRATTR_START]=new SdrCaptionEscAbsItem    ;
    rPoolDefaults[SDRATTR_CAPTIONLINELEN   -SDRATTR_START]=new SdrCaptionLineLenItem   ;
    rPoolDefaults[SDRATTR_CAPTIONFITLINELEN-SDRATTR_START]=new SdrCaptionFitLineLenItem;
    rPoolDefaults[SDRATTR_ECKENRADIUS            -SDRATTR_START]=new SdrMetricItem(SDRATTR_ECKENRADIUS, 0);
    rPoolDefaults[SDRATTR_TEXT_MINFRAMEHEIGHT    -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_MINFRAMEHEIGHT, 0);
    rPoolDefaults[SDRATTR_TEXT_AUTOGROWHEIGHT    -SDRATTR_START]=new SdrOnOffItem(SDRATTR_TEXT_AUTOGROWHEIGHT, true);
    rPoolDefaults[SDRATTR_TEXT_FITTOSIZE     -SDRATTR_START]=new SdrTextFitToSizeTypeItem;
    rPoolDefaults[SDRATTR_TEXT_LEFTDIST      -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_LEFTDIST, 0);
    rPoolDefaults[SDRATTR_TEXT_RIGHTDIST     -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_RIGHTDIST, 0);
    rPoolDefaults[SDRATTR_TEXT_UPPERDIST     -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_UPPERDIST, 0);
    rPoolDefaults[SDRATTR_TEXT_LOWERDIST     -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_LOWERDIST, 0);
    rPoolDefaults[SDRATTR_TEXT_VERTADJUST        -SDRATTR_START]=new SdrTextVertAdjustItem;
    rPoolDefaults[SDRATTR_TEXT_MAXFRAMEHEIGHT    -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_MAXFRAMEHEIGHT, 0);
    rPoolDefaults[SDRATTR_TEXT_MINFRAMEWIDTH -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_MINFRAMEWIDTH, 0);
    rPoolDefaults[SDRATTR_TEXT_MAXFRAMEWIDTH -SDRATTR_START]=new SdrMetricItem(SDRATTR_TEXT_MAXFRAMEWIDTH, 0);
    rPoolDefaults[SDRATTR_TEXT_AUTOGROWWIDTH -SDRATTR_START]=new SdrOnOffItem(SDRATTR_TEXT_AUTOGROWWIDTH, false);
    rPoolDefaults[SDRATTR_TEXT_HORZADJUST        -SDRATTR_START]=new SdrTextHorzAdjustItem;
    rPoolDefaults[SDRATTR_TEXT_ANIKIND           -SDRATTR_START]=new SdrTextAniKindItem;
    rPoolDefaults[SDRATTR_TEXT_ANIDIRECTION  -SDRATTR_START]=new SdrTextAniDirectionItem;
    rPoolDefaults[SDRATTR_TEXT_ANISTARTINSIDE    -SDRATTR_START]=new SdrTextAniStartInsideItem;
    rPoolDefaults[SDRATTR_TEXT_ANISTOPINSIDE -SDRATTR_START]=new SdrTextAniStopInsideItem;
    rPoolDefaults[SDRATTR_TEXT_ANICOUNT      -SDRATTR_START]=new SdrTextAniCountItem;
    rPoolDefaults[SDRATTR_TEXT_ANIDELAY      -SDRATTR_START]=new SdrTextAniDelayItem;
    rPoolDefaults[SDRATTR_TEXT_ANIAMOUNT     -SDRATTR_START]=new SdrTextAniAmountItem;
    rPoolDefaults[SDRATTR_TEXT_CONTOURFRAME  -SDRATTR_START]=new SdrOnOffItem(SDRATTR_TEXT_CONTOURFRAME, false);
    rPoolDefaults[SDRATTR_XMLATTRIBUTES -SDRATTR_START]=new SvXMLAttrContainerItem( SDRATTR_XMLATTRIBUTES );
    rPoolDefaults[SDRATTR_TEXT_CHAINNEXTNAME    -SDRATTR_START]=new SfxStringItem(SDRATTR_TEXT_CHAINNEXTNAME, "");
    rPoolDefaults[SDRATTR_TEXT_USEFIXEDCELLHEIGHT -SDRATTR_START]=new SdrTextFixedCellHeightItem;
    rPoolDefaults[SDRATTR_TEXT_WORDWRAP         -SDRATTR_START]=new SdrOnOffItem(SDRATTR_TEXT_WORDWRAP, true);
    rPoolDefaults[SDRATTR_EDGEKIND         -SDRATTR_START]=new SdrEdgeKindItem;
    rPoolDefaults[SDRATTR_EDGENODE1HORZDIST-SDRATTR_START]=new SdrEdgeNode1HorzDistItem(nDefEdgeDist);
    rPoolDefaults[SDRATTR_EDGENODE1VERTDIST-SDRATTR_START]=new SdrEdgeNode1VertDistItem(nDefEdgeDist);
    rPoolDefaults[SDRATTR_EDGENODE2HORZDIST-SDRATTR_START]=new SdrEdgeNode2HorzDistItem(nDefEdgeDist);
    rPoolDefaults[SDRATTR_EDGENODE2VERTDIST-SDRATTR_START]=new SdrEdgeNode2VertDistItem(nDefEdgeDist);
    rPoolDefaults[SDRATTR_EDGENODE1GLUEDIST-SDRATTR_START]=new SdrEdgeNode1GlueDistItem;
    rPoolDefaults[SDRATTR_EDGENODE2GLUEDIST-SDRATTR_START]=new SdrEdgeNode2GlueDistItem;
    rPoolDefaults[SDRATTR_EDGELINEDELTACOUNT-SDRATTR_START]=new SdrEdgeLineDeltaCountItem;
    rPoolDefaults[SDRATTR_EDGELINE1DELTA   -SDRATTR_START]=new SdrMetricItem(SDRATTR_EDGELINE1DELTA, 0);
    rPoolDefaults[SDRATTR_EDGELINE2DELTA   -SDRATTR_START]=new SdrMetricItem(SDRATTR_EDGELINE2DELTA, 0);
    rPoolDefaults[SDRATTR_EDGELINE3DELTA   -SDRATTR_START]=new SdrMetricItem(SDRATTR_EDGELINE3DELTA, 0);
    rPoolDefaults[SDRATTR_MEASUREKIND             -SDRATTR_START]=new SdrMeasureKindItem;
    rPoolDefaults[SDRATTR_MEASURETEXTHPOS         -SDRATTR_START]=new SdrMeasureTextHPosItem;
    rPoolDefaults[SDRATTR_MEASURETEXTVPOS         -SDRATTR_START]=new SdrMeasureTextVPosItem;
    rPoolDefaults[SDRATTR_MEASURELINEDIST         -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASURELINEDIST, 800);
    rPoolDefaults[SDRATTR_MEASUREHELPLINEOVERHANG -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASUREHELPLINEOVERHANG, 200);
    rPoolDefaults[SDRATTR_MEASUREHELPLINEDIST     -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASUREHELPLINEDIST, 100);
    rPoolDefaults[SDRATTR_MEASUREHELPLINE1LEN     -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASUREHELPLINE1LEN, 0);
    rPoolDefaults[SDRATTR_MEASUREHELPLINE2LEN     -SDRATTR_START]=new SdrMetricItem(SDRATTR_MEASUREHELPLINE2LEN, 0);
    rPoolDefaults[SDRATTR_MEASUREBELOWREFEDGE     -SDRATTR_START]=new SdrMeasureBelowRefEdgeItem;
    rPoolDefaults[SDRATTR_MEASURETEXTROTA90       -SDRATTR_START]=new SdrMeasureTextRota90Item;
    rPoolDefaults[SDRATTR_MEASURETEXTUPSIDEDOWN   -SDRATTR_START]=new SdrMeasureTextUpsideDownItem;
    rPoolDefaults[SDRATTR_MEASUREOVERHANG         -SDRATTR_START]=new SdrMeasureOverhangItem(600);
    rPoolDefaults[SDRATTR_MEASUREUNIT             -SDRATTR_START]=new SdrMeasureUnitItem;
    rPoolDefaults[SDRATTR_MEASURESCALE            -SDRATTR_START]=new SdrMeasureScaleItem;
    rPoolDefaults[SDRATTR_MEASURESHOWUNIT         -SDRATTR_START]=new SdrYesNoItem(SDRATTR_MEASURESHOWUNIT, false);
    rPoolDefaults[SDRATTR_MEASUREFORMATSTRING     -SDRATTR_START]=new SdrMeasureFormatStringItem();
    rPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLE    -SDRATTR_START]=new SdrMeasureTextAutoAngleItem();
    rPoolDefaults[SDRATTR_MEASURETEXTAUTOANGLEVIEW-SDRATTR_START]=new SdrMeasureTextAutoAngleViewItem();
    rPoolDefaults[SDRATTR_MEASURETEXTISFIXEDANGLE -SDRATTR_START]=new SdrMeasureTextIsFixedAngleItem();
    rPoolDefaults[SDRATTR_MEASURETEXTFIXEDANGLE   -SDRATTR_START]=new SdrMeasureTextFixedAngleItem();
    rPoolDefaults[SDRATTR_MEASUREDECIMALPLACES    -SDRATTR_START]=new SdrMeasureDecimalPlacesItem();
    rPoolDefaults[SDRATTR_CIRCKIND      -SDRATTR_START]=new SdrCircKindItem;
    rPoolDefaults[SDRATTR_CIRCSTARTANGLE-SDRATTR_START]=new SdrAngleItem(SDRATTR_CIRCSTARTANGLE, 0);
    rPoolDefaults[SDRATTR_CIRCENDANGLE  -SDRATTR_START]=new SdrAngleItem(SDRATTR_CIRCENDANGLE, 36000);
    rPoolDefaults[SDRATTR_OBJMOVEPROTECT -SDRATTR_START]=new SdrYesNoItem(SDRATTR_OBJMOVEPROTECT, false);
    rPoolDefaults[SDRATTR_OBJSIZEPROTECT -SDRATTR_START]=new SdrYesNoItem(SDRATTR_OBJSIZEPROTECT, false);
    rPoolDefaults[SDRATTR_OBJPRINTABLE   -SDRATTR_START]=new SdrObjPrintableItem;
    rPoolDefaults[SDRATTR_OBJVISIBLE     -SDRATTR_START]=new SdrObjVisibleItem;
    rPoolDefaults[SDRATTR_LAYERID        -SDRATTR_START]=new SdrLayerIdItem(SdrLayerID(0));
    rPoolDefaults[SDRATTR_LAYERNAME      -SDRATTR_START]=new SdrLayerNameItem;
    rPoolDefaults[SDRATTR_OBJECTNAME     -SDRATTR_START]=new SfxStringItem(SDRATTR_OBJECTNAME);
    rPoolDefaults[SDRATTR_ALLPOSITIONX   -SDRATTR_START]=new SdrAllPositionXItem;
    rPoolDefaults[SDRATTR_ALLPOSITIONY   -SDRATTR_START]=new SdrAllPositionYItem;
    rPoolDefaults[SDRATTR_ALLSIZEWIDTH   -SDRATTR_START]=new SdrAllSizeWidthItem;
    rPoolDefaults[SDRATTR_ALLSIZEHEIGHT  -SDRATTR_START]=new SdrAllSizeHeightItem;
    rPoolDefaults[SDRATTR_ONEPOSITIONX   -SDRATTR_START]=new SdrOnePositionXItem;
    rPoolDefaults[SDRATTR_ONEPOSITIONY   -SDRATTR_START]=new SdrOnePositionYItem;
    rPoolDefaults[SDRATTR_ONESIZEWIDTH   -SDRATTR_START]=new SdrOneSizeWidthItem;
    rPoolDefaults[SDRATTR_ONESIZEHEIGHT  -SDRATTR_START]=new SdrOneSizeHeightItem;
    rPoolDefaults[SDRATTR_LOGICSIZEWIDTH -SDRATTR_START]=new SdrLogicSizeWidthItem;
    rPoolDefaults[SDRATTR_LOGICSIZEHEIGHT-SDRATTR_START]=new SdrLogicSizeHeightItem;
    rPoolDefaults[SDRATTR_ROTATEANGLE    -SDRATTR_START]=new SdrAngleItem(SDRATTR_ROTATEANGLE, 0);
    rPoolDefaults[SDRATTR_SHEARANGLE     -SDRATTR_START]=new SdrShearAngleItem;
    rPoolDefaults[SDRATTR_MOVEX          -SDRATTR_START]=new SdrMoveXItem;
    rPoolDefaults[SDRATTR_MOVEY          -SDRATTR_START]=new SdrMoveYItem;
    rPoolDefaults[SDRATTR_RESIZEXONE     -SDRATTR_START]=new SdrResizeXOneItem;
    rPoolDefaults[SDRATTR_RESIZEYONE     -SDRATTR_START]=new SdrResizeYOneItem;
    rPoolDefaults[SDRATTR_ROTATEONE      -SDRATTR_START]=new SdrRotateOneItem;
    rPoolDefaults[SDRATTR_HORZSHEARONE   -SDRATTR_START]=new SdrHorzShearOneItem;
    rPoolDefaults[SDRATTR_VERTSHEARONE   -SDRATTR_START]=new SdrVertShearOneItem;
    rPoolDefaults[SDRATTR_RESIZEXALL     -SDRATTR_START]=new SdrResizeXAllItem;
    rPoolDefaults[SDRATTR_RESIZEYALL     -SDRATTR_START]=new SdrResizeYAllItem;
    rPoolDefaults[SDRATTR_ROTATEALL      -SDRATTR_START]=new SdrRotateAllItem;
    rPoolDefaults[SDRATTR_HORZSHEARALL   -SDRATTR_START]=new SdrHorzShearAllItem;
    rPoolDefaults[SDRATTR_VERTSHEARALL   -SDRATTR_START]=new SdrVertShearAllItem;
    rPoolDefaults[SDRATTR_TRANSFORMREF1X -SDRATTR_START]=new SdrTransformRef1XItem;
    rPoolDefaults[SDRATTR_TRANSFORMREF1Y -SDRATTR_START]=new SdrTransformRef1YItem;
    rPoolDefaults[SDRATTR_TRANSFORMREF2X -SDRATTR_START]=new SdrTransformRef2XItem;
    rPoolDefaults[SDRATTR_TRANSFORMREF2Y -SDRATTR_START]=new SdrTransformRef2YItem;
    rPoolDefaults[SDRATTR_TEXTDIRECTION  -SDRATTR_START]=new SvxWritingModeItem(css::text::WritingMode_LR_TB, SDRATTR_TEXTDIRECTION);
    rPoolDefaults[ SDRATTR_GRAFRED               - SDRATTR_START] = new SdrGrafRedItem;
    rPoolDefaults[ SDRATTR_GRAFGREEN         - SDRATTR_START] = new SdrGrafGreenItem;
    rPoolDefaults[ SDRATTR_GRAFBLUE          - SDRATTR_START] = new SdrGrafBlueItem;
    rPoolDefaults[ SDRATTR_GRAFLUMINANCE     - SDRATTR_START] = new SdrGrafLuminanceItem;
    rPoolDefaults[ SDRATTR_GRAFCONTRAST      - SDRATTR_START] = new SdrGrafContrastItem;
    rPoolDefaults[ SDRATTR_GRAFGAMMA         - SDRATTR_START] = new SdrGrafGamma100Item;
    rPoolDefaults[ SDRATTR_GRAFTRANSPARENCE  - SDRATTR_START] = new SdrGrafTransparenceItem;
    rPoolDefaults[ SDRATTR_GRAFINVERT            - SDRATTR_START] = new SdrGrafInvertItem;
    rPoolDefaults[ SDRATTR_GRAFMODE          - SDRATTR_START] = new SdrGrafModeItem;
    rPoolDefaults[ SDRATTR_GRAFCROP          - SDRATTR_START] = new SdrGrafCropItem;
    rPoolDefaults[ SDRATTR_3DOBJ_PERCENT_DIAGONAL - SDRATTR_START ] = new SfxUInt16Item(SDRATTR_3DOBJ_PERCENT_DIAGONAL, 10);
    rPoolDefaults[ SDRATTR_3DOBJ_BACKSCALE - SDRATTR_START ] = new SfxUInt16Item(SDRATTR_3DOBJ_BACKSCALE, 100);
    rPoolDefaults[ SDRATTR_3DOBJ_DEPTH - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DOBJ_DEPTH, 1000);
    rPoolDefaults[ SDRATTR_3DOBJ_HORZ_SEGS - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DOBJ_HORZ_SEGS, 24);
    rPoolDefaults[ SDRATTR_3DOBJ_VERT_SEGS - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DOBJ_VERT_SEGS, 24);
    rPoolDefaults[ SDRATTR_3DOBJ_END_ANGLE - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DOBJ_END_ANGLE, 3600);
    rPoolDefaults[ SDRATTR_3DOBJ_DOUBLE_SIDED - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DOBJ_DOUBLE_SIDED, false);
    rPoolDefaults[ SDRATTR_3DOBJ_NORMALS_KIND - SDRATTR_START ] = new Svx3DNormalsKindItem;
    rPoolDefaults[ SDRATTR_3DOBJ_NORMALS_INVERT - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DOBJ_NORMALS_INVERT, false);
    rPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_X - SDRATTR_START ] = new Svx3DTextureProjectionXItem;
    rPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_PROJ_Y - SDRATTR_START ] = new Svx3DTextureProjectionYItem;
    rPoolDefaults[ SDRATTR_3DOBJ_SHADOW_3D - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DOBJ_SHADOW_3D, false);
    rPoolDefaults[ SDRATTR_3DOBJ_MAT_COLOR - SDRATTR_START ] = new SvxColorItem(Color(0x0000b8ff), SDRATTR_3DOBJ_MAT_COLOR);
    rPoolDefaults[ SDRATTR_3DOBJ_MAT_EMISSION - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DOBJ_MAT_EMISSION);
    rPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR - SDRATTR_START ] = new SvxColorItem(Color(0x00ffffff), SDRATTR_3DOBJ_MAT_SPECULAR);
    rPoolDefaults[ SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY - SDRATTR_START ] = new SfxUInt16Item(SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY, 15);
    rPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_KIND - SDRATTR_START ] = new Svx3DTextureKindItem;
    rPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_MODE - SDRATTR_START ] = new Svx3DTextureModeItem;
    rPoolDefaults[ SDRATTR_3DOBJ_TEXTURE_FILTER - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DOBJ_TEXTURE_FILTER, false);
    rPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_NORMALS - SDRATTR_START ] = new Svx3DSmoothNormalsItem;
    rPoolDefaults[ SDRATTR_3DOBJ_SMOOTH_LIDS - SDRATTR_START ] = new Svx3DSmoothLidsItem;
    rPoolDefaults[ SDRATTR_3DOBJ_CHARACTER_MODE - SDRATTR_START ] = new Svx3DCharacterModeItem;
    rPoolDefaults[ SDRATTR_3DOBJ_CLOSE_FRONT - SDRATTR_START ] = new Svx3DCloseFrontItem;
    rPoolDefaults[ SDRATTR_3DOBJ_CLOSE_BACK - SDRATTR_START ] = new Svx3DCloseBackItem;
    rPoolDefaults[ SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY - SDRATTR_START ] = new Svx3DReducedLineGeometryItem;
    rPoolDefaults[ SDRATTR_3DSCENE_PERSPECTIVE - SDRATTR_START ] = new Svx3DPerspectiveItem;
    rPoolDefaults[ SDRATTR_3DSCENE_DISTANCE - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DSCENE_DISTANCE, 100);
    rPoolDefaults[ SDRATTR_3DSCENE_FOCAL_LENGTH - SDRATTR_START ] = new SfxUInt32Item(SDRATTR_3DSCENE_FOCAL_LENGTH, 100);
    rPoolDefaults[ SDRATTR_3DSCENE_TWO_SIDED_LIGHTING - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_TWO_SIDED_LIGHTING, false);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_1 - SDRATTR_START ] = new SvxColorItem(Color(0xffcccccc), SDRATTR_3DSCENE_LIGHTCOLOR_1);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_2 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_2);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_3 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_3);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_4 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_4);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_5 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_5);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_6 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_6);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_7 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_7);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTCOLOR_8 - SDRATTR_START ] = new SvxColorItem(Color(0x00000000), SDRATTR_3DSCENE_LIGHTCOLOR_8);
    rPoolDefaults[ SDRATTR_3DSCENE_AMBIENTCOLOR - SDRATTR_START ] = new SvxColorItem(Color(0x00666666), SDRATTR_3DSCENE_AMBIENTCOLOR);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_1 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_1, true);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_2 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_2, false);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_3 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_3, false);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_4 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_4, false);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_5 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_5, false);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_6 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_6, false);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_7 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_7, false);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTON_8 - SDRATTR_START ] = new SfxBoolItem(SDRATTR_3DSCENE_LIGHTON_8, false);
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_1 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_1, basegfx::B3DVector(0.57735026918963, 0.57735026918963, 0.57735026918963));
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_2 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_2, basegfx::B3DVector(0.0,0.0,1.0));
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_3 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_3, basegfx::B3DVector(0.0,0.0,1.0));
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_4 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_4, basegfx::B3DVector(0.0,0.0,1.0));
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_5 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_5, basegfx::B3DVector(0.0,0.0,1.0));
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_6 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_6, basegfx::B3DVector(0.0,0.0,1.0));
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_7 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_7, basegfx::B3DVector(0.0,0.0,1.0));
    rPoolDefaults[ SDRATTR_3DSCENE_LIGHTDIRECTION_8 - SDRATTR_START ] = new SvxB3DVectorItem(SDRATTR_3DSCENE_LIGHTDIRECTION_8, basegfx::B3DVector(0.0,0.0,1.0));
    rPoolDefaults[ SDRATTR_3DSCENE_SHADOW_SLANT - SDRATTR_START ] = new SfxUInt16Item(SDRATTR_3DSCENE_SHADOW_SLANT, 0);
    rPoolDefaults[ SDRATTR_3DSCENE_SHADE_MODE - SDRATTR_START ] = new Svx3DShadeModeItem;
    rPoolDefaults[ SDRATTR_CUSTOMSHAPE_ENGINE - SDRATTR_START ] = new SfxStringItem(SDRATTR_CUSTOMSHAPE_ENGINE, "");
    rPoolDefaults[ SDRATTR_CUSTOMSHAPE_DATA - SDRATTR_START ] = new SfxStringItem(SDRATTR_CUSTOMSHAPE_DATA, "");
    rPoolDefaults[ SDRATTR_CUSTOMSHAPE_GEOMETRY - SDRATTR_START ] = new SdrCustomShapeGeometryItem;

    SvxBoxItem* pboxItem = new SvxBoxItem( SDRATTR_TABLE_BORDER );
    pboxItem->SetAllDistances( 100 );
    rPoolDefaults[ SDRATTR_TABLE_BORDER - SDRATTR_START ] = pboxItem;

    SvxBoxInfoItem* pBoxInfoItem = new SvxBoxInfoItem( SDRATTR_TABLE_BORDER_INNER );

    pBoxInfoItem->SetTable( true );
    pBoxInfoItem->SetDist( true);        // always show margin field
    pBoxInfoItem->SetValid( SvxBoxInfoItemValidFlags::DISABLE ); // some lines may have DontCare state only in tables

    rPoolDefaults[ SDRATTR_TABLE_BORDER_INNER - SDRATTR_START ] =  pBoxInfoItem;
    rPoolDefaults[ SDRATTR_TABLE_BORDER_TLBR - SDRATTR_START ] = new SvxLineItem( SDRATTR_TABLE_BORDER_TLBR );
    rPoolDefaults[ SDRATTR_TABLE_BORDER_BLTR - SDRATTR_START ] = new SvxLineItem( SDRATTR_TABLE_BORDER_BLTR );
    rPoolDefaults[ SDRATTR_TABLE_TEXT_ROTATION - SDRATTR_START ] = new SvxTextRotateItem(0, SDRATTR_TABLE_TEXT_ROTATION);

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
    SetDefaults(mpLocalPoolDefaults);
    SetItemInfos(mpLocalItemInfos.get());
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
    // split pools before destroying
    SetSecondaryPool(nullptr);
}

bool SdrItemPool::GetPresentation(
              const SfxPoolItem& rItem,
              MapUnit ePresentationMetric, OUString& rText,
              const IntlWrapper& rIntlWrapper) const
{
    if (!IsInvalidItem(&rItem)) {
        sal_uInt16 nWhich=rItem.Which();
        if (nWhich>=SDRATTR_SHADOW_FIRST && nWhich<=SDRATTR_END) {
            rItem.GetPresentation(SfxItemPresentation::Nameless,
                        GetMetric(nWhich),ePresentationMetric,rText,
                        rIntlWrapper);
            OUString aStr;

            TakeItemName(nWhich, aStr);
            rText = aStr + " " + rText;

            return true;
        }
    }
    return XOutdevItemPool::GetPresentation(rItem,ePresentationMetric,rText,rIntlWrapper);
}

void SdrItemPool::TakeItemName(sal_uInt16 nWhich, OUString& rItemName)
{
    const char* pResId = SIP_UNKNOWN_ATTR;

    switch (nWhich)
    {
        case XATTR_LINESTYLE        : pResId = SIP_XA_LINESTYLE;break;
        case XATTR_LINEDASH         : pResId = SIP_XA_LINEDASH;break;
        case XATTR_LINEWIDTH        : pResId = SIP_XA_LINEWIDTH;break;
        case XATTR_LINECOLOR        : pResId = SIP_XA_LINECOLOR;break;
        case XATTR_LINESTART        : pResId = SIP_XA_LINESTART;break;
        case XATTR_LINEEND          : pResId = SIP_XA_LINEEND;break;
        case XATTR_LINESTARTWIDTH   : pResId = SIP_XA_LINESTARTWIDTH;break;
        case XATTR_LINEENDWIDTH     : pResId = SIP_XA_LINEENDWIDTH;break;
        case XATTR_LINESTARTCENTER  : pResId = SIP_XA_LINESTARTCENTER;break;
        case XATTR_LINEENDCENTER    : pResId = SIP_XA_LINEENDCENTER;break;
        case XATTR_LINETRANSPARENCE : pResId = SIP_XA_LINETRANSPARENCE;break;
        case XATTR_LINEJOINT        : pResId = SIP_XA_LINEJOINT;break;
        case XATTRSET_LINE          : pResId = SIP_XATTRSET_LINE;break;

        case XATTR_FILLSTYLE            : pResId = SIP_XA_FILLSTYLE;break;
        case XATTR_FILLCOLOR            : pResId = SIP_XA_FILLCOLOR;break;
        case XATTR_FILLGRADIENT         : pResId = SIP_XA_FILLGRADIENT;break;
        case XATTR_FILLHATCH            : pResId = SIP_XA_FILLHATCH;break;
        case XATTR_FILLBITMAP           : pResId = SIP_XA_FILLBITMAP;break;
        case XATTR_FILLTRANSPARENCE     : pResId = SIP_XA_FILLTRANSPARENCE;break;
        case XATTR_GRADIENTSTEPCOUNT    : pResId = SIP_XA_GRADIENTSTEPCOUNT;break;
        case XATTR_FILLBMP_TILE         : pResId = SIP_XA_FILLBMP_TILE;break;
        case XATTR_FILLBMP_POS          : pResId = SIP_XA_FILLBMP_POS;break;
        case XATTR_FILLBMP_SIZEX        : pResId = SIP_XA_FILLBMP_SIZEX;break;
        case XATTR_FILLBMP_SIZEY        : pResId = SIP_XA_FILLBMP_SIZEY;break;
        case XATTR_FILLFLOATTRANSPARENCE: pResId = SIP_XA_FILLFLOATTRANSPARENCE;break;
        case XATTR_SECONDARYFILLCOLOR   : pResId = SIP_XA_SECONDARYFILLCOLOR;break;
        case XATTR_FILLBMP_SIZELOG      : pResId = SIP_XA_FILLBMP_SIZELOG;break;
        case XATTR_FILLBMP_TILEOFFSETX  : pResId = SIP_XA_FILLBMP_TILEOFFSETX;break;
        case XATTR_FILLBMP_TILEOFFSETY  : pResId = SIP_XA_FILLBMP_TILEOFFSETY;break;
        case XATTR_FILLBMP_STRETCH      : pResId = SIP_XA_FILLBMP_STRETCH;break;
        case XATTR_FILLBMP_POSOFFSETX   : pResId = SIP_XA_FILLBMP_POSOFFSETX;break;
        case XATTR_FILLBMP_POSOFFSETY   : pResId = SIP_XA_FILLBMP_POSOFFSETY;break;
        case XATTR_FILLBACKGROUND       : pResId = SIP_XA_FILLBACKGROUND;break;

        case XATTRSET_FILL             : pResId = SIP_XATTRSET_FILL;break;

        case XATTR_FORMTXTSTYLE     : pResId = SIP_XA_FORMTXTSTYLE;break;
        case XATTR_FORMTXTADJUST    : pResId = SIP_XA_FORMTXTADJUST;break;
        case XATTR_FORMTXTDISTANCE  : pResId = SIP_XA_FORMTXTDISTANCE;break;
        case XATTR_FORMTXTSTART     : pResId = SIP_XA_FORMTXTSTART;break;
        case XATTR_FORMTXTMIRROR    : pResId = SIP_XA_FORMTXTMIRROR;break;
        case XATTR_FORMTXTOUTLINE   : pResId = SIP_XA_FORMTXTOUTLINE;break;
        case XATTR_FORMTXTSHADOW    : pResId = SIP_XA_FORMTXTSHADOW;break;
        case XATTR_FORMTXTSHDWCOLOR : pResId = SIP_XA_FORMTXTSHDWCOLOR;break;
        case XATTR_FORMTXTSHDWXVAL  : pResId = SIP_XA_FORMTXTSHDWXVAL;break;
        case XATTR_FORMTXTSHDWYVAL  : pResId = SIP_XA_FORMTXTSHDWYVAL;break;
        case XATTR_FORMTXTHIDEFORM  : pResId = SIP_XA_FORMTXTHIDEFORM;break;
        case XATTR_FORMTXTSHDWTRANSP: pResId = SIP_XA_FORMTXTSHDWTRANSP;break;

        case SDRATTR_SHADOW            : pResId = SIP_SA_SHADOW;break;
        case SDRATTR_SHADOWCOLOR       : pResId = SIP_SA_SHADOWCOLOR;break;
        case SDRATTR_SHADOWXDIST       : pResId = SIP_SA_SHADOWXDIST;break;
        case SDRATTR_SHADOWYDIST       : pResId = SIP_SA_SHADOWYDIST;break;
        case SDRATTR_SHADOWTRANSPARENCE: pResId = SIP_SA_SHADOWTRANSPARENCE;break;
        case SDRATTR_SHADOW3D          : pResId = SIP_SA_SHADOW3D;break;
        case SDRATTR_SHADOWPERSP       : pResId = SIP_SA_SHADOWPERSP;break;

        case SDRATTR_CAPTIONTYPE      : pResId = SIP_SA_CAPTIONTYPE;break;
        case SDRATTR_CAPTIONFIXEDANGLE: pResId = SIP_SA_CAPTIONFIXEDANGLE;break;
        case SDRATTR_CAPTIONANGLE     : pResId = SIP_SA_CAPTIONANGLE;break;
        case SDRATTR_CAPTIONGAP       : pResId = SIP_SA_CAPTIONGAP;break;
        case SDRATTR_CAPTIONESCDIR    : pResId = SIP_SA_CAPTIONESCDIR;break;
        case SDRATTR_CAPTIONESCISREL  : pResId = SIP_SA_CAPTIONESCISREL;break;
        case SDRATTR_CAPTIONESCREL    : pResId = SIP_SA_CAPTIONESCREL;break;
        case SDRATTR_CAPTIONESCABS    : pResId = SIP_SA_CAPTIONESCABS;break;
        case SDRATTR_CAPTIONLINELEN   : pResId = SIP_SA_CAPTIONLINELEN;break;
        case SDRATTR_CAPTIONFITLINELEN: pResId = SIP_SA_CAPTIONFITLINELEN;break;

        case SDRATTR_ECKENRADIUS            : pResId = SIP_SA_ECKENRADIUS;break;
        case SDRATTR_TEXT_MINFRAMEHEIGHT    : pResId = SIP_SA_TEXT_MINFRAMEHEIGHT;break;
        case SDRATTR_TEXT_AUTOGROWHEIGHT    : pResId = SIP_SA_TEXT_AUTOGROWHEIGHT;break;
        case SDRATTR_TEXT_FITTOSIZE         : pResId = SIP_SA_TEXT_FITTOSIZE;break;
        case SDRATTR_TEXT_LEFTDIST          : pResId = SIP_SA_TEXT_LEFTDIST;break;
        case SDRATTR_TEXT_RIGHTDIST         : pResId = SIP_SA_TEXT_RIGHTDIST;break;
        case SDRATTR_TEXT_UPPERDIST         : pResId = SIP_SA_TEXT_UPPERDIST;break;
        case SDRATTR_TEXT_LOWERDIST         : pResId = SIP_SA_TEXT_LOWERDIST;break;
        case SDRATTR_TEXT_VERTADJUST        : pResId = SIP_SA_TEXT_VERTADJUST;break;
        case SDRATTR_TEXT_MAXFRAMEHEIGHT    : pResId = SIP_SA_TEXT_MAXFRAMEHEIGHT;break;
        case SDRATTR_TEXT_MINFRAMEWIDTH     : pResId = SIP_SA_TEXT_MINFRAMEWIDTH;break;
        case SDRATTR_TEXT_MAXFRAMEWIDTH     : pResId = SIP_SA_TEXT_MAXFRAMEWIDTH;break;
        case SDRATTR_TEXT_AUTOGROWWIDTH     : pResId = SIP_SA_TEXT_AUTOGROWWIDTH;break;
        case SDRATTR_TEXT_HORZADJUST        : pResId = SIP_SA_TEXT_HORZADJUST;break;
        case SDRATTR_TEXT_ANIKIND           : pResId = SIP_SA_TEXT_ANIKIND;break;
        case SDRATTR_TEXT_ANIDIRECTION      : pResId = SIP_SA_TEXT_ANIDIRECTION;break;
        case SDRATTR_TEXT_ANISTARTINSIDE    : pResId = SIP_SA_TEXT_ANISTARTINSIDE;break;
        case SDRATTR_TEXT_ANISTOPINSIDE     : pResId = SIP_SA_TEXT_ANISTOPINSIDE;break;
        case SDRATTR_TEXT_ANICOUNT          : pResId = SIP_SA_TEXT_ANICOUNT;break;
        case SDRATTR_TEXT_ANIDELAY          : pResId = SIP_SA_TEXT_ANIDELAY;break;
        case SDRATTR_TEXT_ANIAMOUNT         : pResId = SIP_SA_TEXT_ANIAMOUNT;break;
        case SDRATTR_TEXT_CONTOURFRAME      : pResId = SIP_SA_TEXT_CONTOURFRAME;break;
        case SDRATTR_XMLATTRIBUTES          : pResId = SIP_SA_XMLATTRIBUTES;break;
        case SDRATTR_TEXT_USEFIXEDCELLHEIGHT: pResId = SIP_SA_TEXT_USEFIXEDCELLHEIGHT;break;
        case SDRATTR_TEXT_WORDWRAP          : pResId = SIP_SA_WORDWRAP;break;
        case SDRATTR_TEXT_CHAINNEXTNAME     : pResId = SIP_SA_CHAINNEXTNAME;break;

        case SDRATTR_EDGEKIND           : pResId = SIP_SA_EDGEKIND;break;
        case SDRATTR_EDGENODE1HORZDIST  : pResId = SIP_SA_EDGENODE1HORZDIST;break;
        case SDRATTR_EDGENODE1VERTDIST  : pResId = SIP_SA_EDGENODE1VERTDIST;break;
        case SDRATTR_EDGENODE2HORZDIST  : pResId = SIP_SA_EDGENODE2HORZDIST;break;
        case SDRATTR_EDGENODE2VERTDIST  : pResId = SIP_SA_EDGENODE2VERTDIST;break;
        case SDRATTR_EDGENODE1GLUEDIST  : pResId = SIP_SA_EDGENODE1GLUEDIST;break;
        case SDRATTR_EDGENODE2GLUEDIST  : pResId = SIP_SA_EDGENODE2GLUEDIST;break;
        case SDRATTR_EDGELINEDELTACOUNT : pResId = SIP_SA_EDGELINEDELTACOUNT;break;
        case SDRATTR_EDGELINE1DELTA     : pResId = SIP_SA_EDGELINE1DELTA;break;
        case SDRATTR_EDGELINE2DELTA     : pResId = SIP_SA_EDGELINE2DELTA;break;
        case SDRATTR_EDGELINE3DELTA     : pResId = SIP_SA_EDGELINE3DELTA;break;

        case SDRATTR_MEASUREKIND             : pResId = SIP_SA_MEASUREKIND;break;
        case SDRATTR_MEASURETEXTHPOS         : pResId = SIP_SA_MEASURETEXTHPOS;break;
        case SDRATTR_MEASURETEXTVPOS         : pResId = SIP_SA_MEASURETEXTVPOS;break;
        case SDRATTR_MEASURELINEDIST         : pResId = SIP_SA_MEASURELINEDIST;break;
        case SDRATTR_MEASUREHELPLINEOVERHANG : pResId = SIP_SA_MEASUREHELPLINEOVERHANG;break;
        case SDRATTR_MEASUREHELPLINEDIST     : pResId = SIP_SA_MEASUREHELPLINEDIST;break;
        case SDRATTR_MEASUREHELPLINE1LEN     : pResId = SIP_SA_MEASUREHELPLINE1LEN;break;
        case SDRATTR_MEASUREHELPLINE2LEN     : pResId = SIP_SA_MEASUREHELPLINE2LEN;break;
        case SDRATTR_MEASUREBELOWREFEDGE     : pResId = SIP_SA_MEASUREBELOWREFEDGE;break;
        case SDRATTR_MEASURETEXTROTA90       : pResId = SIP_SA_MEASURETEXTROTA90;break;
        case SDRATTR_MEASURETEXTUPSIDEDOWN   : pResId = SIP_SA_MEASURETEXTUPSIDEDOWN;break;
        case SDRATTR_MEASUREOVERHANG         : pResId = SIP_SA_MEASUREOVERHANG;break;
        case SDRATTR_MEASUREUNIT             : pResId = SIP_SA_MEASUREUNIT;break;
        case SDRATTR_MEASURESCALE            : pResId = SIP_SA_MEASURESCALE;break;
        case SDRATTR_MEASURESHOWUNIT         : pResId = SIP_SA_MEASURESHOWUNIT;break;
        case SDRATTR_MEASUREFORMATSTRING     : pResId = SIP_SA_MEASUREFORMATSTRING;break;
        case SDRATTR_MEASURETEXTAUTOANGLE    : pResId = SIP_SA_MEASURETEXTAUTOANGLE;break;
        case SDRATTR_MEASURETEXTAUTOANGLEVIEW: pResId = SIP_SA_MEASURETEXTAUTOANGLEVIEW;break;
        case SDRATTR_MEASURETEXTISFIXEDANGLE : pResId = SIP_SA_MEASURETEXTISFIXEDANGLE;break;
        case SDRATTR_MEASURETEXTFIXEDANGLE   : pResId = SIP_SA_MEASURETEXTFIXEDANGLE;break;
        case SDRATTR_MEASUREDECIMALPLACES    : pResId = SIP_SA_MEASUREDECIMALPLACES;break;

        case SDRATTR_CIRCKIND      : pResId = SIP_SA_CIRCKIND;break;
        case SDRATTR_CIRCSTARTANGLE: pResId = SIP_SA_CIRCSTARTANGLE;break;
        case SDRATTR_CIRCENDANGLE  : pResId = SIP_SA_CIRCENDANGLE;break;

        case SDRATTR_OBJMOVEPROTECT : pResId = SIP_SA_OBJMOVEPROTECT;break;
        case SDRATTR_OBJSIZEPROTECT : pResId = SIP_SA_OBJSIZEPROTECT;break;
        case SDRATTR_OBJPRINTABLE   : pResId = SIP_SA_OBJPRINTABLE;break;
        case SDRATTR_OBJVISIBLE     : pResId = SIP_SA_OBJVISIBLE;break;
        case SDRATTR_LAYERID        : pResId = SIP_SA_LAYERID;break;
        case SDRATTR_LAYERNAME      : pResId = SIP_SA_LAYERNAME;break;
        case SDRATTR_OBJECTNAME     : pResId = SIP_SA_OBJECTNAME;break;
        case SDRATTR_ALLPOSITIONX   : pResId = SIP_SA_ALLPOSITIONX;break;
        case SDRATTR_ALLPOSITIONY   : pResId = SIP_SA_ALLPOSITIONY;break;
        case SDRATTR_ALLSIZEWIDTH   : pResId = SIP_SA_ALLSIZEWIDTH;break;
        case SDRATTR_ALLSIZEHEIGHT  : pResId = SIP_SA_ALLSIZEHEIGHT;break;
        case SDRATTR_ONEPOSITIONX   : pResId = SIP_SA_ONEPOSITIONX;break;
        case SDRATTR_ONEPOSITIONY   : pResId = SIP_SA_ONEPOSITIONY;break;
        case SDRATTR_ONESIZEWIDTH   : pResId = SIP_SA_ONESIZEWIDTH;break;
        case SDRATTR_ONESIZEHEIGHT  : pResId = SIP_SA_ONESIZEHEIGHT;break;
        case SDRATTR_LOGICSIZEWIDTH : pResId = SIP_SA_LOGICSIZEWIDTH;break;
        case SDRATTR_LOGICSIZEHEIGHT: pResId = SIP_SA_LOGICSIZEHEIGHT;break;
        case SDRATTR_ROTATEANGLE    : pResId = SIP_SA_ROTATEANGLE;break;
        case SDRATTR_SHEARANGLE     : pResId = SIP_SA_SHEARANGLE;break;
        case SDRATTR_MOVEX          : pResId = SIP_SA_MOVEX;break;
        case SDRATTR_MOVEY          : pResId = SIP_SA_MOVEY;break;
        case SDRATTR_RESIZEXONE     : pResId = SIP_SA_RESIZEXONE;break;
        case SDRATTR_RESIZEYONE     : pResId = SIP_SA_RESIZEYONE;break;
        case SDRATTR_ROTATEONE      : pResId = SIP_SA_ROTATEONE;break;
        case SDRATTR_HORZSHEARONE   : pResId = SIP_SA_HORZSHEARONE;break;
        case SDRATTR_VERTSHEARONE   : pResId = SIP_SA_VERTSHEARONE;break;
        case SDRATTR_RESIZEXALL     : pResId = SIP_SA_RESIZEXALL;break;
        case SDRATTR_RESIZEYALL     : pResId = SIP_SA_RESIZEYALL;break;
        case SDRATTR_ROTATEALL      : pResId = SIP_SA_ROTATEALL;break;
        case SDRATTR_HORZSHEARALL   : pResId = SIP_SA_HORZSHEARALL;break;
        case SDRATTR_VERTSHEARALL   : pResId = SIP_SA_VERTSHEARALL;break;
        case SDRATTR_TRANSFORMREF1X : pResId = SIP_SA_TRANSFORMREF1X;break;
        case SDRATTR_TRANSFORMREF1Y : pResId = SIP_SA_TRANSFORMREF1Y;break;
        case SDRATTR_TRANSFORMREF2X : pResId = SIP_SA_TRANSFORMREF2X;break;
        case SDRATTR_TRANSFORMREF2Y : pResId = SIP_SA_TRANSFORMREF2Y;break;

        case SDRATTR_GRAFRED            : pResId = SIP_SA_GRAFRED;break;
        case SDRATTR_GRAFGREEN          : pResId = SIP_SA_GRAFGREEN;break;
        case SDRATTR_GRAFBLUE           : pResId = SIP_SA_GRAFBLUE;break;
        case SDRATTR_GRAFLUMINANCE      : pResId = SIP_SA_GRAFLUMINANCE;break;
        case SDRATTR_GRAFCONTRAST       : pResId = SIP_SA_GRAFCONTRAST;break;
        case SDRATTR_GRAFGAMMA          : pResId = SIP_SA_GRAFGAMMA;break;
        case SDRATTR_GRAFTRANSPARENCE   : pResId = SIP_SA_GRAFTRANSPARENCE;break;
        case SDRATTR_GRAFINVERT         : pResId = SIP_SA_GRAFINVERT;break;
        case SDRATTR_GRAFMODE           : pResId = SIP_SA_GRAFMODE;break;
        case SDRATTR_GRAFCROP           : pResId = SIP_SA_GRAFCROP;break;

        case EE_PARA_HYPHENATE  : pResId = SIP_EE_PARA_HYPHENATE;break;
        case EE_PARA_BULLETSTATE: pResId = SIP_EE_PARA_BULLETSTATE;break;
        case EE_PARA_OUTLLRSPACE: pResId = SIP_EE_PARA_OUTLLRSPACE;break;
        case EE_PARA_OUTLLEVEL  : pResId = SIP_EE_PARA_OUTLLEVEL;break;
        case EE_PARA_BULLET     : pResId = SIP_EE_PARA_BULLET;break;
        case EE_PARA_LRSPACE    : pResId = SIP_EE_PARA_LRSPACE;break;
        case EE_PARA_ULSPACE    : pResId = SIP_EE_PARA_ULSPACE;break;
        case EE_PARA_SBL        : pResId = SIP_EE_PARA_SBL;break;
        case EE_PARA_JUST       : pResId = SIP_EE_PARA_JUST;break;
        case EE_PARA_TABS       : pResId = SIP_EE_PARA_TABS;break;

        case EE_CHAR_COLOR      : pResId = SIP_EE_CHAR_COLOR;break;
        case EE_CHAR_FONTINFO   : pResId = SIP_EE_CHAR_FONTINFO;break;
        case EE_CHAR_FONTHEIGHT : pResId = SIP_EE_CHAR_FONTHEIGHT;break;
        case EE_CHAR_FONTWIDTH  : pResId = SIP_EE_CHAR_FONTWIDTH;break;
        case EE_CHAR_WEIGHT     : pResId = SIP_EE_CHAR_WEIGHT;break;
        case EE_CHAR_UNDERLINE  : pResId = SIP_EE_CHAR_UNDERLINE;break;
        case EE_CHAR_OVERLINE   : pResId = SIP_EE_CHAR_OVERLINE;break;
        case EE_CHAR_STRIKEOUT  : pResId = SIP_EE_CHAR_STRIKEOUT;break;
        case EE_CHAR_ITALIC     : pResId = SIP_EE_CHAR_ITALIC;break;
        case EE_CHAR_OUTLINE    : pResId = SIP_EE_CHAR_OUTLINE;break;
        case EE_CHAR_SHADOW     : pResId = SIP_EE_CHAR_SHADOW;break;
        case EE_CHAR_ESCAPEMENT : pResId = SIP_EE_CHAR_ESCAPEMENT;break;
        case EE_CHAR_PAIRKERNING: pResId = SIP_EE_CHAR_PAIRKERNING;break;
        case EE_CHAR_KERNING    : pResId = SIP_EE_CHAR_KERNING;break;
        case EE_CHAR_WLM        : pResId = SIP_EE_CHAR_WLM;break;
        case EE_FEATURE_TAB     : pResId = SIP_EE_FEATURE_TAB;break;
        case EE_FEATURE_LINEBR  : pResId = SIP_EE_FEATURE_LINEBR;break;
        case EE_FEATURE_NOTCONV : pResId = SIP_EE_FEATURE_NOTCONV;break;
        case EE_FEATURE_FIELD   : pResId = SIP_EE_FEATURE_FIELD;break;
    } // switch

    rItemName = SvxResId(pResId);
}


// FractionItem


bool SdrFractionItem::operator==(const SfxPoolItem& rCmp) const
{
    return SfxPoolItem::operator==(rCmp) &&
           static_cast<const SdrFractionItem&>(rCmp).GetValue()==nValue;
}

bool SdrFractionItem::GetPresentation(
    SfxItemPresentation ePresentation, MapUnit /*eCoreMetric*/,
    MapUnit /*ePresentationMetric*/, OUString &rText, const IntlWrapper&) const
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

    if(ePresentation == SfxItemPresentation::Complete)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
        return true;
    }
    else if(ePresentation == SfxItemPresentation::Nameless)
        return true;

    return false;
}

SfxPoolItem* SdrFractionItem::Clone(SfxItemPool * /*pPool*/) const
{
    return new SdrFractionItem(Which(),GetValue());
}


// ScaleItem


bool SdrScaleItem::GetPresentation(
    SfxItemPresentation ePresentation, MapUnit /*eCoreMetric*/,
    MapUnit /*ePresentationMetric*/, OUString &rText, const IntlWrapper&) const
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

    if(ePresentation == SfxItemPresentation::Complete)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }

    return true;
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

OUString SdrOnOffItem::GetValueTextByVal(bool bVal) const
{
    if (bVal)
        return SvxResId(STR_ItemValON);
    return SvxResId(STR_ItemValOFF);
}

bool SdrOnOffItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByVal(GetValue());
    if (ePres==SfxItemPresentation::Complete) {
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

OUString SdrYesNoItem::GetValueTextByVal(bool bVal) const
{
    if (bVal)
        return SvxResId(STR_ItemValYES);
    return SvxResId(STR_ItemValNO);
}

bool SdrYesNoItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByVal(GetValue());
    if (ePres==SfxItemPresentation::Complete) {
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

bool SdrPercentItem::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreMetric*/,
    MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText = unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());

    if(ePres == SfxItemPresentation::Complete)
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

bool SdrAngleItem::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
    OUString& rText, const IntlWrapper& rIntlWrapper) const
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
                rIntlWrapper.getLocaleData()->getNumDecimalSep()[0];
            aText.insert(nLen-2, cDec);

            if(bNull1)
                aText.remove(nLen, aText.getLength()-nLen);
        }

        if(bNeg)
            aText.insert(0, '-');
    }

    aText.insert(aText.getLength(), sal_Unicode(DEGREE_CHAR));

    if(ePres == SfxItemPresentation::Complete)
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
                      MapUnit eCoreMetric, MapUnit ePresMetric, OUString& rText, const IntlWrapper&) const
{
    long nValue=GetValue();
    SdrFormatter aFmt(eCoreMetric,ePresMetric);
    aFmt.TakeStr(nValue,rText);
    OUString aStr;
    SdrFormatter::TakeUnitStr(ePresMetric,aStr);
    rText += " " + aStr;
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr2;

        SdrItemPool::TakeItemName(Which(), aStr2);
        rText = aStr2 + " " + rText;
    }
    return true;
}


// items of the legend object


SfxPoolItem* SdrCaptionTypeItem::Clone(SfxItemPool* /*pPool*/) const                { return new SdrCaptionTypeItem(*this); }

sal_uInt16 SdrCaptionTypeItem::GetValueCount() const { return 4; }

OUString SdrCaptionTypeItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALCAPTIONTYPES[] =
    {
        STR_ItemValCAPTIONTYPE1,
        STR_ItemValCAPTIONTYPE2,
        STR_ItemValCAPTIONTYPE3,
        STR_ItemValCAPTIONTYPE4
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALCAPTIONTYPES) && "wrong pos!");
    return SvxResId(ITEMVALCAPTIONTYPES[nPos]);
}

bool SdrCaptionTypeItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}


SfxPoolItem* SdrCaptionEscDirItem::Clone(SfxItemPool* /*pPool*/) const              { return new SdrCaptionEscDirItem(*this); }

sal_uInt16 SdrCaptionEscDirItem::GetValueCount() const { return 3; }

OUString SdrCaptionEscDirItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALCAPTIONTYPES[] =
    {
        STR_ItemValCAPTIONESCHORI,
        STR_ItemValCAPTIONESCVERT,
        STR_ItemValCAPTIONESCBESTFIT
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALCAPTIONTYPES) && "wrong pos!");
    return SvxResId(ITEMVALCAPTIONTYPES[nPos]);
}

bool SdrCaptionEscDirItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
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

sal_uInt16 SdrTextFitToSizeTypeItem::GetValueCount() const { return 4; }

OUString SdrTextFitToSizeTypeItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALFITTISIZETYPES[] =
    {
        STR_ItemValFITTOSIZENONE,
        STR_ItemValFITTOSIZEPROP,
        STR_ItemValFITTOSIZEALLLINES,
        STR_ItemValFITTOSIZERESIZEAT
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALFITTISIZETYPES) && "wrong pos!");
    return SvxResId(ITEMVALFITTISIZETYPES[nPos]);
}

bool SdrTextFitToSizeTypeItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrTextFitToSizeTypeItem::HasBoolValue() const { return true; }

bool SdrTextFitToSizeTypeItem::GetBoolValue() const { return GetValue() != drawing::TextFitToSizeType_NONE; }

void SdrTextFitToSizeTypeItem::SetBoolValue(bool bVal)
{
    SetValue(bVal ? drawing::TextFitToSizeType_PROPORTIONAL : drawing::TextFitToSizeType_NONE);
}

bool SdrTextFitToSizeTypeItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    drawing::TextFitToSizeType eFS = GetValue();
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

        eFS = static_cast<drawing::TextFitToSizeType>(nEnum);
    }

    SetValue(eFS);

    return true;
}


SfxPoolItem* SdrTextVertAdjustItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextVertAdjustItem(*this); }

sal_uInt16 SdrTextVertAdjustItem::GetValueCount() const { return 5; }

OUString SdrTextVertAdjustItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALTEXTVADJTYPES[] =
    {
        STR_ItemValTEXTVADJTOP,
        STR_ItemValTEXTVADJCENTER,
        STR_ItemValTEXTVADJBOTTOM,
        STR_ItemValTEXTVADJBLOCK,
        STR_ItemValTEXTVADJSTRETCH
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALTEXTVADJTYPES) && "wrong pos!");
    return SvxResId(ITEMVALTEXTVADJTYPES[nPos]);
}

bool SdrTextVertAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrTextVertAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextVerticalAdjust>(GetValue());
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

        eAdj = static_cast<drawing::TextVerticalAdjust>(nEnum);
    }

    SetValue( static_cast<SdrTextVertAdjust>(eAdj) );

    return true;
}

void SdrTextVertAdjustItem::dumpAsXml(xmlTextWriterPtr pWriter) const
{
    xmlTextWriterStartElement(pWriter, BAD_CAST("SdrTextVertAdjustItem"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("whichId"), BAD_CAST(OString::number(Which()).getStr()));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST("value"), BAD_CAST(OString::number(GetValue()).getStr()));
    xmlTextWriterEndElement(pWriter);
}

SfxPoolItem* SdrTextHorzAdjustItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextHorzAdjustItem(*this); }

sal_uInt16 SdrTextHorzAdjustItem::GetValueCount() const { return 5; }

OUString SdrTextHorzAdjustItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALTEXTHADJTYPES[] =
    {
        STR_ItemValTEXTHADJLEFT,
        STR_ItemValTEXTHADJCENTER,
        STR_ItemValTEXTHADJRIGHT,
        STR_ItemValTEXTHADJBLOCK,
        STR_ItemValTEXTHADJSTRETCH
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALTEXTHADJTYPES) && "wrong pos!");
    return SvxResId(ITEMVALTEXTHADJTYPES[nPos]);
}

bool SdrTextHorzAdjustItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrTextHorzAdjustItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextHorizontalAdjust>(GetValue());
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

        eAdj = static_cast<drawing::TextHorizontalAdjust>(nEnum);
    }

    SetValue( static_cast<SdrTextHorzAdjust>(eAdj) );

    return true;
}


SfxPoolItem* SdrTextAniKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniKindItem(*this); }

sal_uInt16 SdrTextAniKindItem::GetValueCount() const { return 5; }

OUString SdrTextAniKindItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALTEXTANITYPES[] =
    {
        STR_ItemValTEXTANI_NONE,
        STR_ItemValTEXTANI_BLINK,
        STR_ItemValTEXTANI_SCROLL,
        STR_ItemValTEXTANI_ALTERNATE,
        STR_ItemValTEXTANI_SLIDE
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALTEXTANITYPES) && "wrong pos!");
    return SvxResId(ITEMVALTEXTANITYPES[nPos]);
}

bool SdrTextAniKindItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrTextAniKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextAnimationKind>(GetValue());
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
        eKind = static_cast<drawing::TextAnimationKind>(nEnum);
    }

    SetValue( static_cast<SdrTextAniKind>(eKind) );

    return true;
}


SfxPoolItem* SdrTextAniDirectionItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniDirectionItem(*this); }

sal_uInt16 SdrTextAniDirectionItem::GetValueCount() const { return 4; }

OUString SdrTextAniDirectionItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALTEXTANITYPES[] =
    {
        STR_ItemValTEXTANI_LEFT,
        STR_ItemValTEXTANI_UP,
        STR_ItemValTEXTANI_RIGHT,
        STR_ItemValTEXTANI_DOWN
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALTEXTANITYPES) && "wrong pos!");
    return SvxResId(ITEMVALTEXTANITYPES[nPos]);
}

bool SdrTextAniDirectionItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrTextAniDirectionItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::TextAnimationDirection>(GetValue());
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

        eDir = static_cast<drawing::TextAnimationDirection>(nEnum);
    }

    SetValue( static_cast<SdrTextAniDirection>(eDir) );

    return true;
}


SfxPoolItem* SdrTextAniDelayItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniDelayItem(*this); }

bool SdrTextAniDelayItem::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
    OUString& rText, const IntlWrapper&) const
{
    rText = OUString::number(GetValue()) + "ms";

    if(ePres == SfxItemPresentation::Complete)
    {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }

    return true;
}


SfxPoolItem* SdrTextAniAmountItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrTextAniAmountItem(*this); }

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
    SfxItemPresentation ePres, MapUnit eCoreMetric, MapUnit ePresMetric,
    OUString& rText, const IntlWrapper&) const
{
    sal_Int32 nValue(GetValue());

    if(!nValue)
        nValue = -1;

    if(nValue < 0)
    {
        rText = OUString::number(-nValue) + "pixel";
    }
    else
    {
        SdrFormatter aFmt(eCoreMetric, ePresMetric);
        OUString aStr;

        aFmt.TakeStr(nValue, rText);
        SdrFormatter::TakeUnitStr(ePresMetric, aStr);
        rText += aStr;
    }

    if(ePres == SfxItemPresentation::Complete)
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
bool SdrTextFixedCellHeightItem::GetPresentation( SfxItemPresentation ePres,
                                    MapUnit /*eCoreMetric*/, MapUnit /*ePresentationMetric*/,
                                    OUString &rText, const IntlWrapper& ) const
{
    rText = GetValueTextByVal( GetValue() );
    if (ePres==SfxItemPresentation::Complete)
    {
        OUString aStr;
        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
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

// EdgeKind

SfxPoolItem* SdrEdgeKindItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrEdgeKindItem(*this); }

sal_uInt16 SdrEdgeKindItem::GetValueCount() const { return 4; }

OUString SdrEdgeKindItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALEDGES[] =
    {
        STR_ItemValEDGE_ORTHOLINES,
        STR_ItemValEDGE_THREELINES,
        STR_ItemValEDGE_ONELINE,
        STR_ItemValEDGE_BEZIER
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALEDGES) && "wrong pos!");
    return SvxResId(ITEMVALEDGES[nPos]);
}

bool SdrEdgeKindItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
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
        case SdrEdgeKind::OrthoLines :   eCT = drawing::ConnectorType_STANDARD; break;
        case SdrEdgeKind::ThreeLines :   eCT = drawing::ConnectorType_LINES; break;
        case SdrEdgeKind::OneLine :      eCT = drawing::ConnectorType_LINE;  break;
        case SdrEdgeKind::Bezier :       eCT = drawing::ConnectorType_CURVE; break;
        case SdrEdgeKind::Arc :          eCT = drawing::ConnectorType_CURVE; break;
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

        eCT = static_cast<drawing::ConnectorType>(nEnum);
    }

    SdrEdgeKind eEK = SdrEdgeKind::OrthoLines;
    switch( eCT )
    {
        case drawing::ConnectorType_STANDARD :  eEK = SdrEdgeKind::OrthoLines;   break;
        case drawing::ConnectorType_CURVE :     eEK = SdrEdgeKind::Bezier;       break;
        case drawing::ConnectorType_LINE :       eEK = SdrEdgeKind::OneLine;     break;
        case drawing::ConnectorType_LINES :      eEK = SdrEdgeKind::ThreeLines;   break;
        default:
            OSL_FAIL( "SdrEdgeKindItem::PuValue : unknown enum" );
    }
    SetValue( eEK );

    return true;
}

bool SdrEdgeNode1HorzDistItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
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
    rVal <<= GetValue();
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
    rVal <<= GetValue();
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
    rVal <<= GetValue();
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

sal_uInt16 SdrMeasureKindItem::GetValueCount() const { return 2; }

OUString SdrMeasureKindItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALMEASURETYPES[] =
    {
        STR_ItemValMEASURE_STD,
        STR_ItemValMEASURE_RADIUS
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALMEASURETYPES) && "wrong pos!");
    return SvxResId(ITEMVALMEASURETYPES[nPos]);
}

bool SdrMeasureKindItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrMeasureKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::MeasureKind>(GetValue());
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

        eKind = static_cast<drawing::MeasureKind>(nEnum);
    }

    SetValue( static_cast<SdrMeasureKind>(eKind) );
    return true;
}


SfxPoolItem* SdrMeasureTextHPosItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureTextHPosItem(*this); }

sal_uInt16 SdrMeasureTextHPosItem::GetValueCount() const { return 4; }

OUString SdrMeasureTextHPosItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALMEASURETEXTTYPES[] =
    {
        STR_ItemValMEASURE_TEXTHAUTO,
        STR_ItemValMEASURE_TEXTLEFTOUTSIDE,
        STR_ItemValMEASURE_TEXTINSIDE,
        STR_ItemValMEASURE_TEXTRIGHTOUTSID
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALMEASURETEXTTYPES) && "wrong pos!");
    return SvxResId(ITEMVALMEASURETEXTTYPES[nPos]);
}

bool SdrMeasureTextHPosItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrMeasureTextHPosItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
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

        ePos = static_cast<drawing::MeasureTextHorzPos>(nEnum);
    }

    SetValue(ePos);
    return true;
}


SfxPoolItem* SdrMeasureTextVPosItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureTextVPosItem(*this); }

sal_uInt16 SdrMeasureTextVPosItem::GetValueCount() const { return 5; }

OUString SdrMeasureTextVPosItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALMEASURETEXTTYPES[] =
    {
        STR_ItemValMEASURE_TEXTVAUTO,
        STR_ItemValMEASURE_ABOVE,
        STR_ItemValMEASURETEXT_BREAKEDLINE,
        STR_ItemValMEASURE_BELOW,
        STR_ItemValMEASURETEXT_VERTICALCEN
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALMEASURETEXTTYPES) && "wrong pos!");
    return SvxResId(ITEMVALMEASURETEXTTYPES[nPos]);
}

bool SdrMeasureTextVPosItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrMeasureTextVPosItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= GetValue();
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

        ePos = static_cast<drawing::MeasureTextVertPos>(nEnum);
    }

    SetValue(ePos);
    return true;
}

SfxPoolItem* SdrMeasureUnitItem::Clone(SfxItemPool* /*pPool*/) const            { return new SdrMeasureUnitItem(*this); }

sal_uInt16 SdrMeasureUnitItem::GetValueCount() const { return 14; }

OUString SdrMeasureUnitItem::GetValueTextByPos(sal_uInt16 nPos)
{
    OUString aRetval;

    if(static_cast<FieldUnit>(nPos) == FieldUnit::NONE)
        aRetval = "default";
    else
        SdrFormatter::TakeUnitStr(static_cast<FieldUnit>(nPos), aRetval);

    return aRetval;
}

bool SdrMeasureUnitItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrMeasureUnitItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<sal_Int32>(GetValue());
    return true;
}

bool SdrMeasureUnitItem::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    sal_Int32 nMeasure = 0;
    if(!(rVal >>= nMeasure))
        return false;

    SetValue( static_cast<FieldUnit>(nMeasure) );
    return true;
}


SfxPoolItem* SdrCircKindItem::Clone(SfxItemPool* /*pPool*/) const          { return new SdrCircKindItem(*this); }

sal_uInt16 SdrCircKindItem::GetValueCount() const { return 4; }

OUString SdrCircKindItem::GetValueTextByPos(sal_uInt16 nPos)
{
    static const char* ITEMVALCIRCTYPES[] =
    {
        STR_ItemValCIRC_FULL,
        STR_ItemValCIRC_SECT,
        STR_ItemValCIRC_CUT,
        STR_ItemValCIRC_ARC
    };
    assert(nPos < SAL_N_ELEMENTS(ITEMVALCIRCTYPES) && "wrong pos!");
    return SvxResId(ITEMVALCIRCTYPES[nPos]);
}

bool SdrCircKindItem::GetPresentation(SfxItemPresentation ePres,
                      MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/, OUString& rText, const IntlWrapper&) const
{
    rText=GetValueTextByPos(sal::static_int_cast< sal_uInt16 >(GetValue()));
    if (ePres==SfxItemPresentation::Complete) {
        OUString aStr;

        SdrItemPool::TakeItemName(Which(), aStr);
        rText = aStr + " " + rText;
    }
    return true;
}

bool SdrCircKindItem::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<drawing::CircleKind>(GetValue());
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

        eKind = static_cast<drawing::CircleKind>(nEnum);
    }

    SetValue( static_cast<SdrCircKind>(eKind) );
    return true;
}


// class SdrSignedPercentItem


SfxPoolItem* SdrSignedPercentItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrSignedPercentItem( Which(), GetValue() );
}

bool SdrSignedPercentItem::GetPresentation(
    SfxItemPresentation ePres, MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
    OUString& rText, const IntlWrapper&) const
{
    rText = unicode::formatPercent(GetValue(),
        Application::GetSettings().GetUILanguageTag());

    if(ePres == SfxItemPresentation::Complete)
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


SfxPoolItem* SdrGrafGreenItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafGreenItem( *this );
}


SfxPoolItem* SdrGrafBlueItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafBlueItem( *this );
}


SfxPoolItem* SdrGrafLuminanceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafLuminanceItem( *this );
}


SfxPoolItem* SdrGrafContrastItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafContrastItem( *this );
}


SfxPoolItem* SdrGrafGamma100Item::Clone( SfxItemPool* /*pPool */) const
{
    return new SdrGrafGamma100Item( *this );
}

bool SdrGrafGamma100Item::QueryValue( uno::Any& rVal, sal_uInt8 /*nMemberId*/) const
{
    rVal <<= static_cast<double>(GetValue()) / 100.0;
    return true;
}

bool SdrGrafGamma100Item::PutValue( const uno::Any& rVal, sal_uInt8 /*nMemberId*/)
{
    double nGamma = 0;
    if(!(rVal >>= nGamma))
        return false;

    SetValue( static_cast<sal_uInt32>(nGamma * 100.0  ) );
    return true;
}


SfxPoolItem* SdrGrafInvertItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafInvertItem( *this );
}


SfxPoolItem* SdrGrafTransparenceItem::Clone( SfxItemPool* /*pPool*/) const
{
    return new SdrGrafTransparenceItem( *this );
}


SfxPoolItem* SdrGrafModeItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrGrafModeItem( *this );
}

sal_uInt16 SdrGrafModeItem::GetValueCount() const
{
    return 4;
}

OUString SdrGrafModeItem::GetValueTextByPos(sal_uInt16 nPos)
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
                                                               MapUnit /*eCoreMetric*/, MapUnit /*ePresMetric*/,
                                                               OUString& rText, const IntlWrapper&) const
{
    rText = GetValueTextByPos( sal::static_int_cast< sal_uInt16 >( GetValue() ) );

    if( ePres == SfxItemPresentation::Complete )
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

SfxPoolItem* SdrLayerIdItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrLayerIdItem(*this);
}

SfxPoolItem* SdrLayerNameItem::Clone(SfxItemPool* /*pPool*/) const
{
    return new SdrLayerNameItem(*this);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
