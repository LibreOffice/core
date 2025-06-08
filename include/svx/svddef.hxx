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

#ifndef INCLUDED_SVX_SVDDEF_HXX
#define INCLUDED_SVX_SVDDEF_HXX

#include <svx/xdef.hxx>
#include <svl/typedwhich.hxx>

/*************************************************************************/
// SdrItemPool V2, 09-11-1995:
//      7 new Items for XLineAttrSetItem
//      7 new Items for XFillAttrSetItem
//      8 new Items for SdrShadowSetItem
//      5 new Items for SdrCaptionSetItem
//     25 new Items for SdrMiscSetItem
//      6 new Items for SdrNotPersist
// SdrItemPool V3, 29-02-1996
//     17 new Items for XFillAttrSetItem (Bitmapfill)
// SdrItemPool V4, 10-08-1996
//     45 new Items for Edge and Measure (Connector and Measuring object)
// SdrItemPool V4+, 18-10-1996
//      8 new Items for CircItems
//     18 new Items for NotPersist + Refactoring there
// SdrItemPool V4++, 13-11-1996
//      2 new Items for NotPersist+(MoveX/Y)
// SdrItemPool V4+++, 07-06-1999
//     15 new Items for NotPersist (Reserve)
//     15 new Items for GrafItems

//   WhichMapping:
//     V1 Range = 1000..1065  (66)
//     V2 Range = 1000..1123 (124) (58 new Whiches)
//     V3 Range = 1000..1140 (141) (17 new Whiches)
//     V4 Range = 1000..1185 (186) (45 new Whiches)
//     Map V2:  10           6          20           4          11           6           9
//     V1:  1000..1009, 1010..1015, 1016..1035, 1036..1039, 1040..1050, 1051..1056  1057..1065
//     V2:  1000..1009, 1017..1022, 1030..1049, 1050..1053, 1062..1072, 1078..1083, 1109..1117, and 6 new ones appended at the end
//          |<------------ XOut ------------>|  |<----------- SvDraw ----------->
//     Map V3:
//     V2:  1000..1029, 1030..1123
//     V3:  1000..1029, 1047..1140
//     Map V4:
//     V3:  1000..1126, 1127..1140
//     V4:  1000..1126, 1172..1185

class SdrAllPositionXItem;
class SdrAllPositionYItem;
class SdrAllSizeHeightItem;
class SdrAllSizeWidthItem;
class SdrAngleItem;
class SdrCaptionEscAbsItem;
class SdrCaptionEscDirItem;
class SdrCaptionEscIsRelItem;
class SdrCaptionEscRelItem;
class SdrCaptionFitLineLenItem;
class SdrCaptionLineLenItem;
class SdrCaptionTypeItem;
class SdrCircKindItem;
class SdrCustomShapeGeometryItem;
class SdrEdgeKindItem;
class SdrEdgeLineDeltaCountItem;
class SdrEdgeNode1GlueDistItem;
class SdrEdgeNode1HorzDistItem;
class SdrEdgeNode1VertDistItem;
class SdrEdgeNode2GlueDistItem;
class SdrEdgeNode2HorzDistItem;
class SdrEdgeNode2VertDistItem;
class SdrGrafBlueItem;
class SdrGrafContrastItem;
class SdrGrafCropItem;
class SdrGrafGamma100Item;
class SdrGrafGreenItem;
class SdrGrafInvertItem;
class SdrGrafLuminanceItem;
class SdrGrafModeItem;
class SdrGrafRedItem;
class SdrGrafTransparenceItem;
class SdrHorzShearAllItem;
class SdrHorzShearOneItem;
class SdrLayerIdItem;
class SdrLayerNameItem;
class SdrLogicSizeHeightItem;
class SdrLogicSizeWidthItem;
class SdrMeasureBelowRefEdgeItem;
class SdrMeasureDecimalPlacesItem;
class SdrMeasureFormatStringItem;
class SdrMeasureKindItem;
class SdrMeasureOverhangItem;
class SdrMeasureTextAutoAngleItem;
class SdrMeasureTextFixedAngleItem;
class SdrMeasureTextHPosItem;
class SdrMeasureTextIsFixedAngleItem;
class SdrMeasureTextRota90Item;
class SdrMeasureTextVPosItem;
class SdrMeasureUnitItem;
class SdrMetricItem;
class SdrMoveXItem;
class SdrMoveYItem;
class SdrObjPrintableItem;
class SdrObjVisibleItem;
class SdrOnOffItem;
class SdrOnePositionXItem;
class SdrOnePositionYItem;
class SdrOneSizeHeightItem;
class SdrOneSizeWidthItem;
class SdrPercentItem;
class SdrResizeXAllItem;
class SdrResizeXOneItem;
class SdrResizeYAllItem;
class SdrResizeYOneItem;
class SdrRotateOneItem;
class SdrShearAngleItem;
class SdrTextFitToSizeTypeItem;
class SdrTextFixedCellHeightItem;
class SdrTextVertAdjustItem;
class SdrTransformRef1XItem;
class SdrTransformRef1YItem;
class SdrTransformRef2XItem;
class SdrTransformRef2YItem;
class SdrVertShearAllItem;
class SdrVertShearOneItem;
class SdrYesNoItem;
class SfxBoolItem;
class SfxGrabBagItem;
class SfxInt16Item;
class SfxUInt16Item;
class SfxUInt32Item;
class SfxStringItem;
class SfxVoidItem;
class Svx3DCharacterModeItem;
class Svx3DCloseBackItem;
class Svx3DCloseFrontItem;
class Svx3DNormalsKindItem;
class Svx3DPerspectiveItem;
class Svx3DReducedLineGeometryItem;
class Svx3DShadeModeItem;
class Svx3DSmoothLidsItem;
class Svx3DSmoothNormalsItem;
class Svx3DTextureProjectionXItem;
class Svx3DTextureProjectionYItem;
class SvxB3DVectorItem;
class SvxBoxInfoItem;
class SvxBoxItem;
class SvxColorItem;
class SvxLineItem;
class SvxRectangleAlignmentItem;
class SvxTextRotateItem;
class SvxWritingModeItem;
class XColorItem;
class SdrTextAniDirectionItem;
class SdrTextAniKindItem;
class SdrTextHorzAdjustItem;
class SdrTextAniAmountItem;
class SdrTextAniCountItem;
class SdrTextAniStartInsideItem;
class SdrTextAniStopInsideItem;
class SdrTextAniDelayItem;
class SdrMeasureScaleItem;
class SdrRotateAllItem;
class Svx3DTextureKindItem;
class Svx3DTextureModeItem;
class SvXMLAttrContainerItem;
class SvxFrameDirectionItem;

inline constexpr sal_uInt16 SDRATTR_START (XATTR_START);                    /* 1000   */
inline constexpr sal_uInt16                   SDRATTR_SHADOW_FIRST       (XATTR_END + 1);                           // 1048
inline constexpr TypedWhichId<SdrOnOffItem>   SDRATTR_SHADOW             (SDRATTR_SHADOW_FIRST+ 0);                 // 1048
inline constexpr TypedWhichId<XColorItem>     SDRATTR_SHADOWCOLOR        (SDRATTR_SHADOW_FIRST+ 1);                 // 1049
inline constexpr TypedWhichId<SdrMetricItem>  SDRATTR_SHADOWXDIST        (SDRATTR_SHADOW_FIRST+ 2);                 // 1050
inline constexpr TypedWhichId<SdrMetricItem>  SDRATTR_SHADOWYDIST        (SDRATTR_SHADOW_FIRST+ 3);                 // 1051
inline constexpr TypedWhichId<SdrPercentItem> SDRATTR_SHADOWTRANSPARENCE (SDRATTR_SHADOW_FIRST+ 4);                 // 1052
inline constexpr TypedWhichId<SfxVoidItem>    SDRATTR_SHADOW3D           (SDRATTR_SHADOW_FIRST+ 5);                 // 1053
inline constexpr TypedWhichId<SfxVoidItem>    SDRATTR_SHADOWPERSP        (SDRATTR_SHADOW_FIRST+ 6);                 // 1054
inline constexpr TypedWhichId<SdrMetricItem>  SDRATTR_SHADOWSIZEX        (SDRATTR_SHADOW_FIRST+ 7);                 // 1055
inline constexpr TypedWhichId<SdrMetricItem>  SDRATTR_SHADOWSIZEY        (SDRATTR_SHADOW_FIRST+ 8);                 // 1056
inline constexpr TypedWhichId<SdrMetricItem>  SDRATTR_SHADOWBLUR         (SDRATTR_SHADOW_FIRST+ 9);                 // 1057
inline constexpr TypedWhichId<SvxRectangleAlignmentItem> SDRATTR_SHADOWALIGNMENT(SDRATTR_SHADOW_FIRST + 10);        // 1058
inline constexpr sal_uInt16                   SDRATTR_SHADOW_LAST        (SDRATTR_SHADOWALIGNMENT);                 // 1058

inline constexpr sal_uInt16                             SDRATTR_CAPTION_FIRST     (SDRATTR_SHADOW_LAST + 1);        // 1059
inline constexpr TypedWhichId<SdrCaptionTypeItem>       SDRATTR_CAPTIONTYPE       (SDRATTR_CAPTION_FIRST+ 0);       // 1059
inline constexpr TypedWhichId<SdrOnOffItem>             SDRATTR_CAPTIONFIXEDANGLE (SDRATTR_CAPTION_FIRST+ 1);       // 1060
inline constexpr TypedWhichId<SdrAngleItem>             SDRATTR_CAPTIONANGLE      (SDRATTR_CAPTION_FIRST+ 2);       // 1061
inline constexpr TypedWhichId<SdrMetricItem>            SDRATTR_CAPTIONGAP        (SDRATTR_CAPTION_FIRST+ 3);       // 1062
inline constexpr TypedWhichId<SdrCaptionEscDirItem>     SDRATTR_CAPTIONESCDIR     (SDRATTR_CAPTION_FIRST+ 4);       // 1063
inline constexpr TypedWhichId<SdrCaptionEscIsRelItem>   SDRATTR_CAPTIONESCISREL   (SDRATTR_CAPTION_FIRST+ 5);       // 1064
inline constexpr TypedWhichId<SdrCaptionEscRelItem>     SDRATTR_CAPTIONESCREL     (SDRATTR_CAPTION_FIRST+ 6);       // 1065
inline constexpr TypedWhichId<SdrCaptionEscAbsItem>     SDRATTR_CAPTIONESCABS     (SDRATTR_CAPTION_FIRST+ 7);       // 1066
inline constexpr TypedWhichId<SdrCaptionLineLenItem>    SDRATTR_CAPTIONLINELEN    (SDRATTR_CAPTION_FIRST+ 8);       // 1067
inline constexpr TypedWhichId<SdrCaptionFitLineLenItem> SDRATTR_CAPTIONFITLINELEN (SDRATTR_CAPTION_FIRST+ 9);       // 1068
inline constexpr sal_uInt16                             SDRATTR_CAPTION_LAST      (SDRATTR_CAPTIONFITLINELEN);      // 1068

inline constexpr sal_uInt16                               SDRATTR_MISC_FIRST              (SDRATTR_CAPTION_LAST + 1);   // 1069
inline constexpr TypedWhichId<SdrMetricItem>              SDRATTR_CORNER_RADIUS           (SDRATTR_MISC_FIRST + 0);     // 1069 long, def=0
inline constexpr TypedWhichId<SdrMetricItem>              SDRATTR_TEXT_MINFRAMEHEIGHT     (SDRATTR_MISC_FIRST + 1);     // 1070 long, def=0
inline constexpr TypedWhichId<SdrOnOffItem>               SDRATTR_TEXT_AUTOGROWHEIGHT     (SDRATTR_MISC_FIRST + 2);     // 1071 sal_Bool, def=sal_True
inline constexpr TypedWhichId<SdrTextFitToSizeTypeItem>   SDRATTR_TEXT_FITTOSIZE          (SDRATTR_MISC_FIRST + 3);     // 1072 enum, def=css::drawing::TextFitToSizeType_NONE
inline constexpr TypedWhichId<SdrMetricItem>              SDRATTR_TEXT_LEFTDIST           (SDRATTR_MISC_FIRST + 4);     // 1073 long, def=0
inline constexpr TypedWhichId<SdrMetricItem>              SDRATTR_TEXT_RIGHTDIST          (SDRATTR_MISC_FIRST + 5);     // 1074 long, def=0
inline constexpr TypedWhichId<SdrMetricItem>              SDRATTR_TEXT_UPPERDIST          (SDRATTR_MISC_FIRST + 6);     // 1075 long, def=0
inline constexpr TypedWhichId<SdrMetricItem>              SDRATTR_TEXT_LOWERDIST          (SDRATTR_MISC_FIRST + 7);     // 1076 long, def=0
inline constexpr TypedWhichId<SdrTextVertAdjustItem>      SDRATTR_TEXT_VERTADJUST         (SDRATTR_MISC_FIRST + 8);     // 1077 enum, def=SDRTEXTVERTADJUST_TOP
inline constexpr TypedWhichId<SdrMetricItem>              SDRATTR_TEXT_MAXFRAMEHEIGHT     (SDRATTR_MISC_FIRST + 9);     // 1078 long, def=0
inline constexpr TypedWhichId<SdrMetricItem>              SDRATTR_TEXT_MINFRAMEWIDTH      (SDRATTR_MISC_FIRST +10);     // 1079 long, def=0
inline constexpr TypedWhichId<SdrMetricItem>              SDRATTR_TEXT_MAXFRAMEWIDTH      (SDRATTR_MISC_FIRST +11);     // 1080 long, def=0
inline constexpr TypedWhichId<SdrOnOffItem>               SDRATTR_TEXT_AUTOGROWWIDTH      (SDRATTR_MISC_FIRST +12);     // 1081 sal_Bool, def=sal_False
inline constexpr TypedWhichId<SdrTextHorzAdjustItem>      SDRATTR_TEXT_HORZADJUST         (SDRATTR_MISC_FIRST +13);     // 1082 enum, def=SDRTEXTHORZADJUST_LEFT
inline constexpr TypedWhichId<SdrTextAniKindItem>         SDRATTR_TEXT_ANIKIND            (SDRATTR_MISC_FIRST +14);     // 1083 enum, def=SdrTextAniKind::NONE - Pool V2/V4
inline constexpr TypedWhichId<SdrTextAniDirectionItem>    SDRATTR_TEXT_ANIDIRECTION       (SDRATTR_MISC_FIRST +15);     // 1084 enum, def=SdrTextAniDirection::Left - Pool V2/V4
inline constexpr TypedWhichId<SdrTextAniStartInsideItem>  SDRATTR_TEXT_ANISTARTINSIDE     (SDRATTR_MISC_FIRST +16);     // 1085 sal_Bool, def=sal_False - Pool V2/V4
inline constexpr TypedWhichId<SdrTextAniStopInsideItem>   SDRATTR_TEXT_ANISTOPINSIDE      (SDRATTR_MISC_FIRST +17);     // 1086 sal_Bool, def=sal_False - Pool V2/V4
inline constexpr TypedWhichId<SdrTextAniCountItem>        SDRATTR_TEXT_ANICOUNT           (SDRATTR_MISC_FIRST +18);     // 1087 sal_uInt16, def=0   - Pool V2/V4
inline constexpr TypedWhichId<SdrTextAniDelayItem>        SDRATTR_TEXT_ANIDELAY           (SDRATTR_MISC_FIRST +19);     // 1088 sal_uInt16, def=0   - Pool V2/V4
inline constexpr TypedWhichId<SdrTextAniAmountItem>       SDRATTR_TEXT_ANIAMOUNT          (SDRATTR_MISC_FIRST +20);     // 1089 sal_uInt16, def=0   - Pool V2/V4
inline constexpr TypedWhichId<SdrOnOffItem>               SDRATTR_TEXT_CONTOURFRAME       (SDRATTR_MISC_FIRST +21);     // 1090 Pool V2
inline constexpr TypedWhichId<SvXMLAttrContainerItem>     SDRATTR_XMLATTRIBUTES           (SDRATTR_MISC_FIRST +22);     // 1091 Pool V2
inline constexpr TypedWhichId<SdrTextFixedCellHeightItem> SDRATTR_TEXT_USEFIXEDCELLHEIGHT (SDRATTR_MISC_FIRST +23);     // 1092 Pool V2
inline constexpr TypedWhichId<SdrOnOffItem>               SDRATTR_TEXT_WORDWRAP           (SDRATTR_MISC_FIRST +24);     // 1093 Pool V2
inline constexpr TypedWhichId<SfxStringItem>              SDRATTR_TEXT_CHAINNEXTNAME      (SDRATTR_MISC_FIRST +25);     // 1094 Pool V2
inline constexpr TypedWhichId<SdrOnOffItem>               SDRATTR_TEXT_CLIPVERTOVERFLOW   (SDRATTR_MISC_FIRST +26);     // 1095
inline constexpr sal_uInt16                               SDRATTR_MISC_LAST               (SDRATTR_TEXT_CLIPVERTOVERFLOW); // 1095

inline constexpr sal_uInt16                              SDRATTR_EDGE_FIRST         (SDRATTR_MISC_LAST + 1);    // 1096
inline constexpr TypedWhichId<SdrEdgeKindItem>           SDRATTR_EDGEKIND           (SDRATTR_EDGE_FIRST + 0);   // 1096
inline constexpr TypedWhichId<SdrEdgeNode1HorzDistItem>  SDRATTR_EDGENODE1HORZDIST  (SDRATTR_EDGE_FIRST + 1);   // 1097
inline constexpr TypedWhichId<SdrEdgeNode1VertDistItem>  SDRATTR_EDGENODE1VERTDIST  (SDRATTR_EDGE_FIRST + 2);   // 1098
inline constexpr TypedWhichId<SdrEdgeNode2HorzDistItem>  SDRATTR_EDGENODE2HORZDIST  (SDRATTR_EDGE_FIRST + 3);   // 1099
inline constexpr TypedWhichId<SdrEdgeNode2VertDistItem>  SDRATTR_EDGENODE2VERTDIST  (SDRATTR_EDGE_FIRST + 4);   // 1100
inline constexpr TypedWhichId<SdrEdgeNode1GlueDistItem>  SDRATTR_EDGENODE1GLUEDIST  (SDRATTR_EDGE_FIRST + 5);   // 1101
inline constexpr TypedWhichId<SdrEdgeNode2GlueDistItem>  SDRATTR_EDGENODE2GLUEDIST  (SDRATTR_EDGE_FIRST + 6);   // 1102
inline constexpr TypedWhichId<SdrEdgeLineDeltaCountItem> SDRATTR_EDGELINEDELTACOUNT (SDRATTR_EDGE_FIRST + 7);   // 1103
inline constexpr TypedWhichId<SdrMetricItem>             SDRATTR_EDGELINE1DELTA     (SDRATTR_EDGE_FIRST + 8);   // 1104
inline constexpr TypedWhichId<SdrMetricItem>             SDRATTR_EDGELINE2DELTA     (SDRATTR_EDGE_FIRST + 9);   // 1105
inline constexpr TypedWhichId<SdrMetricItem>             SDRATTR_EDGELINE3DELTA     (SDRATTR_EDGE_FIRST +10);   // 1106
inline constexpr sal_uInt16                              SDRATTR_EDGE_LAST          (SDRATTR_EDGELINE3DELTA);   // 1106

inline constexpr sal_uInt16                                   SDRATTR_MEASURE_FIRST            (SDRATTR_EDGE_LAST + 1);        // 1107
inline constexpr TypedWhichId<SdrMeasureKindItem>             SDRATTR_MEASUREKIND              (SDRATTR_MEASURE_FIRST+ 0);     // 1107
inline constexpr TypedWhichId<SdrMeasureTextHPosItem>         SDRATTR_MEASURETEXTHPOS          (SDRATTR_MEASURE_FIRST+ 1);     // 1108
inline constexpr TypedWhichId<SdrMeasureTextVPosItem>         SDRATTR_MEASURETEXTVPOS          (SDRATTR_MEASURE_FIRST+ 2);     // 1109
inline constexpr TypedWhichId<SdrMetricItem>                  SDRATTR_MEASURELINEDIST          (SDRATTR_MEASURE_FIRST+ 3);     // 1110
inline constexpr TypedWhichId<SdrMetricItem>                  SDRATTR_MEASUREHELPLINEOVERHANG  (SDRATTR_MEASURE_FIRST+ 4);     // 1111
inline constexpr TypedWhichId<SdrMetricItem>                  SDRATTR_MEASUREHELPLINEDIST      (SDRATTR_MEASURE_FIRST+ 5);     // 1112
inline constexpr TypedWhichId<SdrMetricItem>                  SDRATTR_MEASUREHELPLINE1LEN      (SDRATTR_MEASURE_FIRST+ 6);     // 1113
inline constexpr TypedWhichId<SdrMetricItem>                  SDRATTR_MEASUREHELPLINE2LEN      (SDRATTR_MEASURE_FIRST+ 7);     // 1114
inline constexpr TypedWhichId<SdrMeasureBelowRefEdgeItem>     SDRATTR_MEASUREBELOWREFEDGE      (SDRATTR_MEASURE_FIRST+ 8);     // 1115
inline constexpr TypedWhichId<SdrMeasureTextRota90Item>       SDRATTR_MEASURETEXTROTA90        (SDRATTR_MEASURE_FIRST+ 9);     // 1116
inline constexpr TypedWhichId<SdrYesNoItem>                   SDRATTR_MEASURETEXTUPSIDEDOWN    (SDRATTR_MEASURE_FIRST+10);     // 1117
inline constexpr TypedWhichId<SdrMeasureOverhangItem>         SDRATTR_MEASUREOVERHANG          (SDRATTR_MEASURE_FIRST+11);     // 1118
inline constexpr TypedWhichId<SdrMeasureUnitItem>             SDRATTR_MEASUREUNIT              (SDRATTR_MEASURE_FIRST+12);     // 1119
inline constexpr TypedWhichId<SdrMeasureScaleItem>            SDRATTR_MEASURESCALE             (SDRATTR_MEASURE_FIRST+13);     // 1120
inline constexpr TypedWhichId<SdrYesNoItem>                   SDRATTR_MEASURESHOWUNIT          (SDRATTR_MEASURE_FIRST+14);     // 1121
inline constexpr TypedWhichId<SdrMeasureFormatStringItem>     SDRATTR_MEASUREFORMATSTRING      (SDRATTR_MEASURE_FIRST+15);     // 1122
inline constexpr TypedWhichId<SdrMeasureTextAutoAngleItem>    SDRATTR_MEASURETEXTAUTOANGLE     (SDRATTR_MEASURE_FIRST+16);     // 1123
inline constexpr TypedWhichId<SdrAngleItem>                   SDRATTR_MEASURETEXTAUTOANGLEVIEW (SDRATTR_MEASURE_FIRST+17);     // 1124
inline constexpr TypedWhichId<SdrMeasureTextIsFixedAngleItem> SDRATTR_MEASURETEXTISFIXEDANGLE  (SDRATTR_MEASURE_FIRST+18);     // 1125
inline constexpr TypedWhichId<SdrMeasureTextFixedAngleItem>   SDRATTR_MEASURETEXTFIXEDANGLE    (SDRATTR_MEASURE_FIRST+19);     // 1126
inline constexpr TypedWhichId<SdrMeasureDecimalPlacesItem>    SDRATTR_MEASUREDECIMALPLACES     (SDRATTR_MEASURE_FIRST+20);     // 1127
inline constexpr sal_uInt16                                   SDRATTR_MEASURE_LAST             (SDRATTR_MEASUREDECIMALPLACES); // 1127

inline constexpr sal_uInt16                    SDRATTR_CIRC_FIRST     (SDRATTR_MEASURE_LAST+ 1); // 1128
inline constexpr TypedWhichId<SdrCircKindItem> SDRATTR_CIRCKIND       (SDRATTR_CIRC_FIRST+ 0);   // 1128
inline constexpr TypedWhichId<SdrAngleItem>    SDRATTR_CIRCSTARTANGLE (SDRATTR_CIRC_FIRST+ 1);   // 1129
inline constexpr TypedWhichId<SdrAngleItem>    SDRATTR_CIRCENDANGLE   (SDRATTR_CIRC_FIRST+ 2);   // 1130
inline constexpr sal_uInt16                    SDRATTR_CIRC_LAST      (SDRATTR_CIRCENDANGLE);    // 1130

// Only for Interfacing, not yet implemented. / * + * V4 /
inline constexpr sal_uInt16                           SDRATTR_NOTPERSIST_FIRST (SDRATTR_CIRC_LAST + 1);      // 1131
inline constexpr TypedWhichId<SdrYesNoItem>           SDRATTR_OBJMOVEPROTECT  (SDRATTR_NOTPERSIST_FIRST+ 0); // 1131  sal_Bool,   def=sal_False
inline constexpr TypedWhichId<SdrYesNoItem>           SDRATTR_OBJSIZEPROTECT  (SDRATTR_NOTPERSIST_FIRST+ 1); // 1132  sal_Bool,   def=sal_False
inline constexpr TypedWhichId<SdrObjPrintableItem>    SDRATTR_OBJPRINTABLE    (SDRATTR_NOTPERSIST_FIRST+ 2); // 1133  sal_Bool,   def=sal_True
inline constexpr TypedWhichId<SdrLayerIdItem>         SDRATTR_LAYERID         (SDRATTR_NOTPERSIST_FIRST+ 3); // 1134  sal_uInt16, def=0
inline constexpr TypedWhichId<SdrLayerNameItem>       SDRATTR_LAYERNAME       (SDRATTR_NOTPERSIST_FIRST+ 4); // 1135  String, def=""
inline constexpr TypedWhichId<SfxStringItem>          SDRATTR_OBJECTNAME      (SDRATTR_NOTPERSIST_FIRST+ 5); // 1136  String, def=""
inline constexpr TypedWhichId<SdrAllPositionXItem>    SDRATTR_ALLPOSITIONX    (SDRATTR_NOTPERSIST_FIRST+ 6); // 1137  long,   def=0
inline constexpr TypedWhichId<SdrAllPositionYItem>    SDRATTR_ALLPOSITIONY    (SDRATTR_NOTPERSIST_FIRST+ 7); // 1138  long,   def=0
inline constexpr TypedWhichId<SdrAllSizeWidthItem>    SDRATTR_ALLSIZEWIDTH    (SDRATTR_NOTPERSIST_FIRST+ 8); // 1139  long,   def=0
inline constexpr TypedWhichId<SdrAllSizeHeightItem>   SDRATTR_ALLSIZEHEIGHT   (SDRATTR_NOTPERSIST_FIRST+ 9); // 1140  long,   def=0
inline constexpr TypedWhichId<SdrOnePositionXItem>    SDRATTR_ONEPOSITIONX    (SDRATTR_NOTPERSIST_FIRST+10); // 1141  long,   def=0
inline constexpr TypedWhichId<SdrOnePositionYItem>    SDRATTR_ONEPOSITIONY    (SDRATTR_NOTPERSIST_FIRST+11); // 1142  long,   def=0
inline constexpr TypedWhichId<SdrOneSizeWidthItem>    SDRATTR_ONESIZEWIDTH    (SDRATTR_NOTPERSIST_FIRST+12); // 1143  long,   def=0
inline constexpr TypedWhichId<SdrOneSizeHeightItem>   SDRATTR_ONESIZEHEIGHT   (SDRATTR_NOTPERSIST_FIRST+13); // 1144  long,   def=0
inline constexpr TypedWhichId<SdrLogicSizeWidthItem>  SDRATTR_LOGICSIZEWIDTH  (SDRATTR_NOTPERSIST_FIRST+14); // 1145  long,   def=0
inline constexpr TypedWhichId<SdrLogicSizeHeightItem> SDRATTR_LOGICSIZEHEIGHT (SDRATTR_NOTPERSIST_FIRST+15); // 1146  long,   def=0
inline constexpr TypedWhichId<SdrAngleItem>           SDRATTR_ROTATEANGLE     (SDRATTR_NOTPERSIST_FIRST+16); // 1147  long,   def=0
inline constexpr TypedWhichId<SdrShearAngleItem>      SDRATTR_SHEARANGLE      (SDRATTR_NOTPERSIST_FIRST+17); // 1148  long,   def=0
inline constexpr TypedWhichId<SdrMoveXItem>           SDRATTR_MOVEX           (SDRATTR_NOTPERSIST_FIRST+18); // 1149  long,   def=0
inline constexpr TypedWhichId<SdrMoveYItem>           SDRATTR_MOVEY           (SDRATTR_NOTPERSIST_FIRST+19); // 1150  long,   def=0
inline constexpr TypedWhichId<SdrResizeXOneItem>      SDRATTR_RESIZEXONE      (SDRATTR_NOTPERSIST_FIRST+20); // 1151  long,   def=0
inline constexpr TypedWhichId<SdrResizeYOneItem>      SDRATTR_RESIZEYONE      (SDRATTR_NOTPERSIST_FIRST+21); // 1152  long,   def=0
inline constexpr TypedWhichId<SdrRotateOneItem>       SDRATTR_ROTATEONE       (SDRATTR_NOTPERSIST_FIRST+22); // 1153  long,   def=0
inline constexpr TypedWhichId<SdrHorzShearOneItem>    SDRATTR_HORZSHEARONE    (SDRATTR_NOTPERSIST_FIRST+23); // 1154  long,   def=0
inline constexpr TypedWhichId<SdrVertShearOneItem>    SDRATTR_VERTSHEARONE    (SDRATTR_NOTPERSIST_FIRST+24); // 1155  long,   def=0
inline constexpr TypedWhichId<SdrResizeXAllItem>      SDRATTR_RESIZEXALL      (SDRATTR_NOTPERSIST_FIRST+25); // 1156  long,   def=0
inline constexpr TypedWhichId<SdrResizeYAllItem>      SDRATTR_RESIZEYALL      (SDRATTR_NOTPERSIST_FIRST+26); // 1157  long,   def=0
inline constexpr TypedWhichId<SdrRotateAllItem>       SDRATTR_ROTATEALL       (SDRATTR_NOTPERSIST_FIRST+27); // 1158  long,   def=0
inline constexpr TypedWhichId<SdrHorzShearAllItem>    SDRATTR_HORZSHEARALL    (SDRATTR_NOTPERSIST_FIRST+28); // 1159  long,   def=0
inline constexpr TypedWhichId<SdrVertShearAllItem>    SDRATTR_VERTSHEARALL    (SDRATTR_NOTPERSIST_FIRST+29); // 1160  long,   def=0
inline constexpr TypedWhichId<SdrTransformRef1XItem>  SDRATTR_TRANSFORMREF1X  (SDRATTR_NOTPERSIST_FIRST+30); // 1161  long,   def=0
inline constexpr TypedWhichId<SdrTransformRef1YItem>  SDRATTR_TRANSFORMREF1Y  (SDRATTR_NOTPERSIST_FIRST+31); // 1162  long,   def=0
inline constexpr TypedWhichId<SdrTransformRef2XItem>  SDRATTR_TRANSFORMREF2X  (SDRATTR_NOTPERSIST_FIRST+32); // 1163  long,   def=0
inline constexpr TypedWhichId<SdrTransformRef2YItem>  SDRATTR_TRANSFORMREF2Y  (SDRATTR_NOTPERSIST_FIRST+33); // 1164  long,   def=0
inline constexpr TypedWhichId<SvxWritingModeItem>     SDRATTR_TEXTDIRECTION   (SDRATTR_NOTPERSIST_FIRST+34); // 1165  long,   def=0
inline constexpr TypedWhichId<SdrObjVisibleItem>      SDRATTR_OBJVISIBLE      (SDRATTR_NOTPERSIST_FIRST+35); // 1166
inline constexpr sal_uInt16                           SDRATTR_NOTPERSIST_LAST (SDRATTR_OBJVISIBLE);          // 1166
// no SetItem for this

inline constexpr sal_uInt16                            SDRATTR_GRAF_FIRST       (SDRATTR_NOTPERSIST_LAST+1); // 1167
inline constexpr TypedWhichId<SdrGrafRedItem>          SDRATTR_GRAFRED          (SDRATTR_GRAF_FIRST+0);      // 1167
inline constexpr TypedWhichId<SdrGrafGreenItem>        SDRATTR_GRAFGREEN        (SDRATTR_GRAF_FIRST+1);      // 1168
inline constexpr TypedWhichId<SdrGrafBlueItem>         SDRATTR_GRAFBLUE         (SDRATTR_GRAF_FIRST+2);      // 1169
inline constexpr TypedWhichId<SdrGrafLuminanceItem>    SDRATTR_GRAFLUMINANCE    (SDRATTR_GRAF_FIRST+3);      // 1170
inline constexpr TypedWhichId<SdrGrafContrastItem>     SDRATTR_GRAFCONTRAST     (SDRATTR_GRAF_FIRST+4);      // 1171
inline constexpr TypedWhichId<SdrGrafGamma100Item>     SDRATTR_GRAFGAMMA        (SDRATTR_GRAF_FIRST+5);      // 1172
inline constexpr TypedWhichId<SdrGrafTransparenceItem> SDRATTR_GRAFTRANSPARENCE (SDRATTR_GRAF_FIRST+6);      // 1173
inline constexpr TypedWhichId<SdrGrafInvertItem>       SDRATTR_GRAFINVERT       (SDRATTR_GRAF_FIRST+7);      // 1174
inline constexpr TypedWhichId<SdrGrafModeItem>         SDRATTR_GRAFMODE         (SDRATTR_GRAF_FIRST+8);      // 1175
inline constexpr TypedWhichId<SdrGrafCropItem>         SDRATTR_GRAFCROP         (SDRATTR_GRAF_FIRST+9);      // 1176
inline constexpr sal_uInt16                            SDRATTR_GRAF_LAST        (SDRATTR_GRAFCROP);          // 1176

inline constexpr sal_uInt16 SDRATTR_3D_FIRST (SDRATTR_GRAF_LAST + 1);     // 1177

inline constexpr sal_uInt16                                SDRATTR_3DOBJ_FIRST                  (SDRATTR_3D_FIRST);          // 1177
inline constexpr TypedWhichId<SfxUInt16Item>               SDRATTR_3DOBJ_PERCENT_DIAGONAL       (SDRATTR_3DOBJ_FIRST + 0);   // 1177
inline constexpr TypedWhichId<SfxUInt16Item>               SDRATTR_3DOBJ_BACKSCALE              (SDRATTR_3DOBJ_FIRST + 1);   // 1178
inline constexpr TypedWhichId<SfxUInt32Item>               SDRATTR_3DOBJ_DEPTH                  (SDRATTR_3DOBJ_FIRST + 2);   // 1179
inline constexpr TypedWhichId<SfxUInt32Item>               SDRATTR_3DOBJ_HORZ_SEGS              (SDRATTR_3DOBJ_FIRST + 3);   // 1180
inline constexpr TypedWhichId<SfxUInt32Item>               SDRATTR_3DOBJ_VERT_SEGS              (SDRATTR_3DOBJ_FIRST + 4);   // 1181
inline constexpr TypedWhichId<SfxUInt16Item>               SDRATTR_3DOBJ_END_ANGLE              (SDRATTR_3DOBJ_FIRST + 5);   // 1182
inline constexpr TypedWhichId<SfxBoolItem>                 SDRATTR_3DOBJ_DOUBLE_SIDED           (SDRATTR_3DOBJ_FIRST + 6);   // 1183
inline constexpr TypedWhichId<Svx3DNormalsKindItem>        SDRATTR_3DOBJ_NORMALS_KIND           (SDRATTR_3DOBJ_FIRST + 7);   // 1184
inline constexpr TypedWhichId<SfxBoolItem>                 SDRATTR_3DOBJ_NORMALS_INVERT         (SDRATTR_3DOBJ_FIRST + 8);   // 1185
inline constexpr TypedWhichId<Svx3DTextureProjectionXItem> SDRATTR_3DOBJ_TEXTURE_PROJ_X         (SDRATTR_3DOBJ_FIRST + 9);   // 1186
inline constexpr TypedWhichId<Svx3DTextureProjectionYItem> SDRATTR_3DOBJ_TEXTURE_PROJ_Y         (SDRATTR_3DOBJ_FIRST + 10);  // 1187
inline constexpr TypedWhichId<SfxBoolItem>                 SDRATTR_3DOBJ_SHADOW_3D              (SDRATTR_3DOBJ_FIRST + 11);  // 1188
inline constexpr TypedWhichId<SvxColorItem>                SDRATTR_3DOBJ_MAT_COLOR              (SDRATTR_3DOBJ_FIRST + 12);  // 1189
inline constexpr TypedWhichId<SvxColorItem>                SDRATTR_3DOBJ_MAT_EMISSION           (SDRATTR_3DOBJ_FIRST + 13);  // 1190
inline constexpr TypedWhichId<SvxColorItem>                SDRATTR_3DOBJ_MAT_SPECULAR           (SDRATTR_3DOBJ_FIRST + 14);  // 1191
inline constexpr TypedWhichId<SfxUInt16Item>               SDRATTR_3DOBJ_MAT_SPECULAR_INTENSITY (SDRATTR_3DOBJ_FIRST + 15);  // 1192
inline constexpr TypedWhichId<Svx3DTextureKindItem>        SDRATTR_3DOBJ_TEXTURE_KIND           (SDRATTR_3DOBJ_FIRST + 16);  // 1193
inline constexpr TypedWhichId<Svx3DTextureModeItem>        SDRATTR_3DOBJ_TEXTURE_MODE           (SDRATTR_3DOBJ_FIRST + 17);  // 1194
inline constexpr TypedWhichId<SfxBoolItem>                 SDRATTR_3DOBJ_TEXTURE_FILTER         (SDRATTR_3DOBJ_FIRST + 18);  // 1195

// #107245# New items for 3d objects use former range SDRATTR_3DOBJ_RESERVED_01
// up to SDRATTR_3DOBJ_RESERVED_05
inline constexpr TypedWhichId<Svx3DSmoothNormalsItem> SDRATTR_3DOBJ_SMOOTH_NORMALS (SDRATTR_3DOBJ_FIRST + 19);  // 1196
inline constexpr TypedWhichId<Svx3DSmoothLidsItem>    SDRATTR_3DOBJ_SMOOTH_LIDS    (SDRATTR_3DOBJ_FIRST + 20);  // 1197
inline constexpr TypedWhichId<Svx3DCharacterModeItem> SDRATTR_3DOBJ_CHARACTER_MODE (SDRATTR_3DOBJ_FIRST + 21);  // 1198
inline constexpr TypedWhichId<Svx3DCloseFrontItem>    SDRATTR_3DOBJ_CLOSE_FRONT    (SDRATTR_3DOBJ_FIRST + 22);  // 1199
inline constexpr TypedWhichId<Svx3DCloseBackItem>     SDRATTR_3DOBJ_CLOSE_BACK     (SDRATTR_3DOBJ_FIRST + 23);  // 1200

// #i28528#
// Added extra Item (Bool) for chart2 to be able to show reduced line geometry
inline constexpr TypedWhichId<Svx3DReducedLineGeometryItem> SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY (SDRATTR_3DOBJ_FIRST + 24);  // 1201

inline constexpr sal_uInt16 SDRATTR_3DOBJ_LAST (SDRATTR_3DOBJ_REDUCED_LINE_GEOMETRY);   // 1201

inline constexpr sal_uInt16                         SDRATTR_3DSCENE_FIRST              (SDRATTR_3DOBJ_LAST + 1);        // 1202
inline constexpr TypedWhichId<Svx3DPerspectiveItem> SDRATTR_3DSCENE_PERSPECTIVE        (SDRATTR_3DSCENE_FIRST + 0);     // 1202
inline constexpr TypedWhichId<SfxUInt32Item>        SDRATTR_3DSCENE_DISTANCE           (SDRATTR_3DSCENE_FIRST + 1);     // 1203
inline constexpr TypedWhichId<SfxUInt32Item>        SDRATTR_3DSCENE_FOCAL_LENGTH       (SDRATTR_3DSCENE_FIRST + 2);     // 1204
inline constexpr TypedWhichId<SfxBoolItem>          SDRATTR_3DSCENE_TWO_SIDED_LIGHTING (SDRATTR_3DSCENE_FIRST + 3);     // 1205
inline constexpr TypedWhichId<SvxColorItem>         SDRATTR_3DSCENE_LIGHTCOLOR_1       (SDRATTR_3DSCENE_FIRST + 4);     // 1206
inline constexpr TypedWhichId<SvxColorItem>         SDRATTR_3DSCENE_LIGHTCOLOR_2       (SDRATTR_3DSCENE_FIRST + 5);     // 1207
inline constexpr TypedWhichId<SvxColorItem>         SDRATTR_3DSCENE_LIGHTCOLOR_3       (SDRATTR_3DSCENE_FIRST + 6);     // 1208
inline constexpr TypedWhichId<SvxColorItem>         SDRATTR_3DSCENE_LIGHTCOLOR_4       (SDRATTR_3DSCENE_FIRST + 7);     // 1209
inline constexpr TypedWhichId<SvxColorItem>         SDRATTR_3DSCENE_LIGHTCOLOR_5       (SDRATTR_3DSCENE_FIRST + 8);     // 1210
inline constexpr TypedWhichId<SvxColorItem>         SDRATTR_3DSCENE_LIGHTCOLOR_6       (SDRATTR_3DSCENE_FIRST + 9);     // 1211
inline constexpr TypedWhichId<SvxColorItem>         SDRATTR_3DSCENE_LIGHTCOLOR_7       (SDRATTR_3DSCENE_FIRST + 10);    // 1212
inline constexpr TypedWhichId<SvxColorItem>         SDRATTR_3DSCENE_LIGHTCOLOR_8       (SDRATTR_3DSCENE_FIRST + 11);    // 1213
inline constexpr TypedWhichId<SvxColorItem>         SDRATTR_3DSCENE_AMBIENTCOLOR       (SDRATTR_3DSCENE_FIRST + 12);    // 1214
inline constexpr TypedWhichId<SfxBoolItem>          SDRATTR_3DSCENE_LIGHTON_1          (SDRATTR_3DSCENE_FIRST + 13);    // 1215
inline constexpr TypedWhichId<SfxBoolItem>          SDRATTR_3DSCENE_LIGHTON_2          (SDRATTR_3DSCENE_FIRST + 14);    // 1216
inline constexpr TypedWhichId<SfxBoolItem>          SDRATTR_3DSCENE_LIGHTON_3          (SDRATTR_3DSCENE_FIRST + 15);    // 1217
inline constexpr TypedWhichId<SfxBoolItem>          SDRATTR_3DSCENE_LIGHTON_4          (SDRATTR_3DSCENE_FIRST + 16);    // 1218
inline constexpr TypedWhichId<SfxBoolItem>          SDRATTR_3DSCENE_LIGHTON_5          (SDRATTR_3DSCENE_FIRST + 17);    // 1219
inline constexpr TypedWhichId<SfxBoolItem>          SDRATTR_3DSCENE_LIGHTON_6          (SDRATTR_3DSCENE_FIRST + 18);    // 1220
inline constexpr TypedWhichId<SfxBoolItem>          SDRATTR_3DSCENE_LIGHTON_7          (SDRATTR_3DSCENE_FIRST + 19);    // 1221
inline constexpr TypedWhichId<SfxBoolItem>          SDRATTR_3DSCENE_LIGHTON_8          (SDRATTR_3DSCENE_FIRST + 20);    // 1222
inline constexpr TypedWhichId<SvxB3DVectorItem>     SDRATTR_3DSCENE_LIGHTDIRECTION_1   (SDRATTR_3DSCENE_FIRST + 21);    // 1223
inline constexpr TypedWhichId<SvxB3DVectorItem>     SDRATTR_3DSCENE_LIGHTDIRECTION_2   (SDRATTR_3DSCENE_FIRST + 22);    // 1224
inline constexpr TypedWhichId<SvxB3DVectorItem>     SDRATTR_3DSCENE_LIGHTDIRECTION_3   (SDRATTR_3DSCENE_FIRST + 23);    // 1225
inline constexpr TypedWhichId<SvxB3DVectorItem>     SDRATTR_3DSCENE_LIGHTDIRECTION_4   (SDRATTR_3DSCENE_FIRST + 24);    // 1226
inline constexpr TypedWhichId<SvxB3DVectorItem>     SDRATTR_3DSCENE_LIGHTDIRECTION_5   (SDRATTR_3DSCENE_FIRST + 25);    // 1227
inline constexpr TypedWhichId<SvxB3DVectorItem>     SDRATTR_3DSCENE_LIGHTDIRECTION_6   (SDRATTR_3DSCENE_FIRST + 26);    // 1228
inline constexpr TypedWhichId<SvxB3DVectorItem>     SDRATTR_3DSCENE_LIGHTDIRECTION_7   (SDRATTR_3DSCENE_FIRST + 27);    // 1229
inline constexpr TypedWhichId<SvxB3DVectorItem>     SDRATTR_3DSCENE_LIGHTDIRECTION_8   (SDRATTR_3DSCENE_FIRST + 28);    // 1230
inline constexpr TypedWhichId<SfxUInt16Item>        SDRATTR_3DSCENE_SHADOW_SLANT       (SDRATTR_3DSCENE_FIRST + 29);    // 1231
inline constexpr TypedWhichId<Svx3DShadeModeItem>   SDRATTR_3DSCENE_SHADE_MODE         (SDRATTR_3DSCENE_FIRST + 30);    // 1232
inline constexpr sal_uInt16                         SDRATTR_3DSCENE_LAST               (SDRATTR_3DSCENE_SHADE_MODE);    // 1232
inline constexpr sal_uInt16                         SDRATTR_3D_LAST                    (SDRATTR_3DSCENE_LAST);          // 1232

inline constexpr sal_uInt16                               SDRATTR_CUSTOMSHAPE_FIRST    (SDRATTR_3D_LAST + 1);           // 1233
inline constexpr TypedWhichId<SfxStringItem>              SDRATTR_CUSTOMSHAPE_ENGINE   (SDRATTR_CUSTOMSHAPE_FIRST + 0); // 1233
inline constexpr TypedWhichId<SfxStringItem>              SDRATTR_CUSTOMSHAPE_DATA     (SDRATTR_CUSTOMSHAPE_FIRST + 1); // 1234
inline constexpr TypedWhichId<SdrCustomShapeGeometryItem> SDRATTR_CUSTOMSHAPE_GEOMETRY (SDRATTR_CUSTOMSHAPE_FIRST + 2); // 1235
inline constexpr sal_uInt16                               SDRATTR_CUSTOMSHAPE_LAST     (SDRATTR_CUSTOMSHAPE_GEOMETRY);  // 1235

inline constexpr sal_uInt16                      SDRATTR_TABLE_FIRST         (SDRATTR_CUSTOMSHAPE_LAST+1); // 1236
inline constexpr TypedWhichId<SvxBoxItem>        SDRATTR_TABLE_BORDER        (SDRATTR_TABLE_FIRST+0);      // 1236
inline constexpr TypedWhichId<SvxBoxInfoItem>    SDRATTR_TABLE_BORDER_INNER  (SDRATTR_TABLE_FIRST+1);      // 1237
inline constexpr TypedWhichId<SvxLineItem>       SDRATTR_TABLE_BORDER_TLBR   (SDRATTR_TABLE_FIRST+2);      // 1238
inline constexpr TypedWhichId<SvxLineItem>       SDRATTR_TABLE_BORDER_BLTR   (SDRATTR_TABLE_FIRST+3);      // 1239
inline constexpr TypedWhichId<SvxTextRotateItem> SDRATTR_TABLE_TEXT_ROTATION (SDRATTR_TABLE_FIRST+4);      // 1240
inline constexpr TypedWhichId<SfxGrabBagItem>    SDRATTR_TABLE_CELL_GRABBAG  (SDRATTR_TABLE_FIRST+5);      // 1241
inline constexpr sal_uInt16                      SDRATTR_TABLE_LAST          (SDRATTR_TABLE_CELL_GRABBAG); // 1241

inline constexpr sal_uInt16                      SDRATTR_GLOW_FIRST (SDRATTR_TABLE_LAST+1);                // 1242
inline constexpr TypedWhichId<SdrMetricItem>     SDRATTR_GLOW_RADIUS(SDRATTR_GLOW_FIRST+0);                // 1242
inline constexpr TypedWhichId<XColorItem>        SDRATTR_GLOW_COLOR (SDRATTR_GLOW_FIRST+1);                // 1243
inline constexpr TypedWhichId<SdrPercentItem>    SDRATTR_GLOW_TRANSPARENCY(SDRATTR_GLOW_FIRST+2);          // 1244
inline constexpr sal_uInt16                      SDRATTR_GLOW_LAST(SDRATTR_GLOW_TRANSPARENCY);             // 1244

inline constexpr sal_uInt16                      SDRATTR_SOFTEDGE_FIRST(SDRATTR_GLOW_LAST+1);              // 1245
inline constexpr TypedWhichId<SdrMetricItem>     SDRATTR_SOFTEDGE_RADIUS(SDRATTR_SOFTEDGE_FIRST+0);        // 1245
inline constexpr sal_uInt16                      SDRATTR_SOFTEDGE_LAST(SDRATTR_SOFTEDGE_RADIUS);           // 1245

inline constexpr sal_uInt16                      SDRATTR_GLOW_TEXT_FIRST(SDRATTR_SOFTEDGE_LAST+1);         // 1246
inline constexpr TypedWhichId<SdrMetricItem>     SDRATTR_GLOW_TEXT_RADIUS(SDRATTR_GLOW_TEXT_FIRST+0);      // 1246
inline constexpr TypedWhichId<XColorItem>        SDRATTR_GLOW_TEXT_COLOR(SDRATTR_GLOW_TEXT_FIRST+1);       // 1247
inline constexpr TypedWhichId<SdrPercentItem>    SDRATTR_GLOW_TEXT_TRANSPARENCY(SDRATTR_GLOW_TEXT_FIRST+2);// 1248
inline constexpr sal_uInt16                      SDRATTR_GLOW_TEXT_LAST(SDRATTR_GLOW_TEXT_TRANSPARENCY);   // 1248

inline constexpr sal_uInt16                      SDRATTR_TEXTCOLUMNS_FIRST(SDRATTR_GLOW_TEXT_LAST+1);      // 1249
inline constexpr TypedWhichId<SfxInt16Item>      SDRATTR_TEXTCOLUMNS_NUMBER(SDRATTR_TEXTCOLUMNS_FIRST+0);  // 1249
inline constexpr TypedWhichId<SdrMetricItem>     SDRATTR_TEXTCOLUMNS_SPACING(SDRATTR_TEXTCOLUMNS_FIRST+1); // 1250
inline constexpr sal_uInt16                      SDRATTR_TEXTCOLUMNS_LAST(SDRATTR_TEXTCOLUMNS_SPACING);    // 1250

inline constexpr sal_uInt16                          SDRATTR_WRITINGMODE2_FIRST(SDRATTR_TEXTCOLUMNS_LAST+1);// 1251
inline constexpr TypedWhichId<SvxFrameDirectionItem> SDRATTR_WRITINGMODE2(SDRATTR_WRITINGMODE2_FIRST+0);    // 1251
inline constexpr sal_uInt16                          SDRATTR_WRITINGMODE2_LAST(SDRATTR_WRITINGMODE2);       // 1251

inline constexpr sal_uInt16                      SDRATTR_EDGEOOXMLCURVE_FIRST(SDRATTR_WRITINGMODE2_LAST+1);// 1252
inline constexpr TypedWhichId<SfxBoolItem>       SDRATTR_EDGEOOXMLCURVE(SDRATTR_EDGEOOXMLCURVE_FIRST+0);   // 1252
inline constexpr sal_uInt16                      SDRATTR_EDGEOOXMLCURVE_LAST(SDRATTR_EDGEOOXMLCURVE);      // 1252

inline constexpr sal_uInt16 SDRATTR_END (SDRATTR_EDGEOOXMLCURVE_LAST);      // 1252

#endif // INCLUDED_SVX_SVDDEF_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
