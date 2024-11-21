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

#ifndef INCLUDED_SVX_XDEF_HXX
#define INCLUDED_SVX_XDEF_HXX


#include <svl/typedwhich.hxx>

/*************************************************************************
|*
|* Adding an attribute: Copy line, above which you want to add something,
|* change the label and add it at the end of the following line, too
|*
\************************************************************************/

class SfxMetricItem;
class XFillAttrSetItem;
class XFillBackgroundItem;
class XFillBitmapItem;
class XFillBmpPosItem;
class XFillBmpPosOffsetXItem;
class XFillBmpPosOffsetYItem;
class XFillBmpSizeLogItem;
class XFillBmpSizeYItem;
class XFillBmpStretchItem;
class XFillBmpTileItem;
class XFillBmpTileOffsetXItem;
class XFillBmpTileOffsetYItem;
class XFillColorItem;
class XFillFloatTransparenceItem;
class XFillGradientItem;
class XFillHatchItem;
class XFillStyleItem;
class XFillTransparenceItem;
class XFillUseSlideBackgroundItem;
class XFormTextAdjustItem;
class XFormTextDistanceItem;
class XFormTextHideFormItem;
class XFormTextMirrorItem;
class XFormTextOutlineItem;
class XFormTextShadowColorItem;
class XFormTextShadowItem;
class XFormTextShadowTranspItem;
class XFormTextShadowXValItem;
class XFormTextShadowYValItem;
class XFormTextStartItem;
class XFormTextStyleItem;
class XGradientStepCountItem;
class XLineAttrSetItem;
class XLineCapItem;
class XLineColorItem;
class XLineDashItem;
class XLineEndCenterItem;
class XLineEndItem;
class XLineEndWidthItem;
class XLineJointItem;
class XLineStartCenterItem;
class XLineStartItem;
class XLineStartWidthItem;
class XLineStyleItem;
class XLineTransparenceItem;
class XLineWidthItem;
class XSecondaryFillColorItem;

#define COL_DEFAULT_FONT          ::Color(0xC9211E)         // dark red 2
#define COL_DEFAULT_FRAMELINE     ::Color(0x2A6099)         // blue
#define COL_DEFAULT_HIGHLIGHT     ::Color(0xFFFF00)         // yellow
#define COL_DEFAULT_SHAPE_FILLING ::Color(0x729fcf)         // light blue 2
#define COL_DEFAULT_SHAPE_STROKE  ::Color(0x3465a4)         // dark blue 1

inline constexpr sal_uInt16 XATTR_START = 1000;

inline constexpr sal_uInt16                          XATTR_LINE_FIRST       (XATTR_START);           // 1000
inline constexpr TypedWhichId<XLineStyleItem>        XATTR_LINESTYLE        (XATTR_LINE_FIRST);      // 1000
inline constexpr TypedWhichId<XLineDashItem>         XATTR_LINEDASH         (XATTR_LINE_FIRST + 1);  // 1001
inline constexpr TypedWhichId<XLineWidthItem>        XATTR_LINEWIDTH        (XATTR_LINE_FIRST + 2);  // 1002
inline constexpr TypedWhichId<XLineColorItem>        XATTR_LINECOLOR        (XATTR_LINE_FIRST + 3);  // 1003
inline constexpr TypedWhichId<XLineStartItem>        XATTR_LINESTART        (XATTR_LINE_FIRST + 4);  // 1004
inline constexpr TypedWhichId<XLineEndItem>          XATTR_LINEEND          (XATTR_LINE_FIRST + 5);  // 1005
inline constexpr TypedWhichId<XLineStartWidthItem>   XATTR_LINESTARTWIDTH   (XATTR_LINE_FIRST + 6);  // 1006
inline constexpr TypedWhichId<XLineEndWidthItem>     XATTR_LINEENDWIDTH     (XATTR_LINE_FIRST + 7);  // 1007
inline constexpr TypedWhichId<XLineStartCenterItem>  XATTR_LINESTARTCENTER  (XATTR_LINE_FIRST + 8);  // 1008
inline constexpr TypedWhichId<XLineEndCenterItem>    XATTR_LINEENDCENTER    (XATTR_LINE_FIRST + 9);  // 1009
inline constexpr TypedWhichId<XLineTransparenceItem> XATTR_LINETRANSPARENCE (XATTR_LINE_FIRST + 10); // 1010
inline constexpr TypedWhichId<XLineJointItem>        XATTR_LINEJOINT        (XATTR_LINE_FIRST + 11); // 1011
inline constexpr TypedWhichId<XLineCapItem>          XATTR_LINECAP          (XATTR_LINE_FIRST + 12); // 1012
inline constexpr sal_uInt16                          XATTR_LINE_LAST        (XATTR_LINECAP);         // 1012
inline constexpr TypedWhichId<XLineAttrSetItem>      XATTRSET_LINE          (XATTR_LINE_LAST + 1);   // 1013

inline constexpr sal_uInt16                               XATTR_FILL_FIRST            (XATTRSET_LINE + 1);      // 1014
inline constexpr TypedWhichId<XFillStyleItem>             XATTR_FILLSTYLE             (XATTR_FILL_FIRST);       // 1014
inline constexpr TypedWhichId<XFillColorItem>             XATTR_FILLCOLOR             (XATTR_FILL_FIRST + 1);   // 1015
inline constexpr TypedWhichId<XFillGradientItem>          XATTR_FILLGRADIENT          (XATTR_FILL_FIRST + 2);   // 1016
inline constexpr TypedWhichId<XFillHatchItem>             XATTR_FILLHATCH             (XATTR_FILL_FIRST + 3);   // 1017
inline constexpr TypedWhichId<XFillBitmapItem>            XATTR_FILLBITMAP            (XATTR_FILL_FIRST + 4);   // 1018
inline constexpr TypedWhichId<XFillTransparenceItem>      XATTR_FILLTRANSPARENCE      (XATTR_FILL_FIRST + 5);   // 1019
inline constexpr TypedWhichId<XGradientStepCountItem>     XATTR_GRADIENTSTEPCOUNT     (XATTR_FILL_FIRST + 6);   // 1020
inline constexpr TypedWhichId<XFillBmpTileItem>           XATTR_FILLBMP_TILE          (XATTR_FILL_FIRST + 7);   // 1021
inline constexpr TypedWhichId<XFillBmpPosItem>            XATTR_FILLBMP_POS           (XATTR_FILL_FIRST + 8);   // 1022
inline constexpr TypedWhichId<SfxMetricItem>              XATTR_FILLBMP_SIZEX         (XATTR_FILL_FIRST + 9);   // 1023
inline constexpr TypedWhichId<XFillBmpSizeYItem>          XATTR_FILLBMP_SIZEY         (XATTR_FILL_FIRST + 10);  // 1024
inline constexpr TypedWhichId<XFillFloatTransparenceItem> XATTR_FILLFLOATTRANSPARENCE (XATTR_FILL_FIRST + 11);  // 1025
inline constexpr TypedWhichId<XSecondaryFillColorItem>    XATTR_SECONDARYFILLCOLOR    (XATTR_FILL_FIRST + 12);  // 1026
inline constexpr TypedWhichId<XFillBmpSizeLogItem>        XATTR_FILLBMP_SIZELOG       (XATTR_FILL_FIRST + 13);  // 1027
inline constexpr TypedWhichId<XFillBmpTileOffsetXItem>    XATTR_FILLBMP_TILEOFFSETX   (XATTR_FILL_FIRST + 14);  // 1028
inline constexpr TypedWhichId<XFillBmpTileOffsetYItem>    XATTR_FILLBMP_TILEOFFSETY   (XATTR_FILL_FIRST + 15);  // 1029
inline constexpr TypedWhichId<XFillBmpStretchItem>        XATTR_FILLBMP_STRETCH       (XATTR_FILL_FIRST + 16);  // 1030
inline constexpr TypedWhichId<XFillBmpPosOffsetXItem>     XATTR_FILLBMP_POSOFFSETX    (XATTR_FILL_FIRST + 17);  // 1031
inline constexpr TypedWhichId<XFillBmpPosOffsetYItem>     XATTR_FILLBMP_POSOFFSETY    (XATTR_FILL_FIRST + 18);  // 1032
inline constexpr TypedWhichId<XFillBackgroundItem>        XATTR_FILLBACKGROUND        (XATTR_FILL_FIRST + 19);  // 1033
inline constexpr TypedWhichId<XFillUseSlideBackgroundItem> XATTR_FILLUSESLIDEBACKGROUND (XATTR_FILL_FIRST + 20);// 1034
inline constexpr sal_uInt16                               XATTR_FILL_LAST             (XATTR_FILLUSESLIDEBACKGROUND); // 1034
inline constexpr TypedWhichId<XFillAttrSetItem>           XATTRSET_FILL               (XATTR_FILL_LAST + 1);    // 1035

inline constexpr sal_uInt16                              XATTR_TEXT_FIRST        (XATTRSET_FILL + 1);      // 1036
inline constexpr TypedWhichId<XFormTextStyleItem>        XATTR_FORMTXTSTYLE      (XATTR_TEXT_FIRST);       // 1036
inline constexpr TypedWhichId<XFormTextAdjustItem>       XATTR_FORMTXTADJUST     (XATTR_TEXT_FIRST + 1);   // 1037
inline constexpr TypedWhichId<XFormTextDistanceItem>     XATTR_FORMTXTDISTANCE   (XATTR_TEXT_FIRST + 2);   // 1038
inline constexpr TypedWhichId<XFormTextStartItem>        XATTR_FORMTXTSTART      (XATTR_TEXT_FIRST + 3);   // 1039
inline constexpr TypedWhichId<XFormTextMirrorItem>       XATTR_FORMTXTMIRROR     (XATTR_TEXT_FIRST + 4);   // 1040
inline constexpr TypedWhichId<XFormTextOutlineItem>      XATTR_FORMTXTOUTLINE    (XATTR_TEXT_FIRST + 5);   // 1041
inline constexpr TypedWhichId<XFormTextShadowItem>       XATTR_FORMTXTSHADOW     (XATTR_TEXT_FIRST + 6);   // 1042
inline constexpr TypedWhichId<XFormTextShadowColorItem>  XATTR_FORMTXTSHDWCOLOR  (XATTR_TEXT_FIRST + 7);   // 1043
inline constexpr TypedWhichId<XFormTextShadowXValItem>   XATTR_FORMTXTSHDWXVAL   (XATTR_TEXT_FIRST + 8);   // 1044
inline constexpr TypedWhichId<XFormTextShadowYValItem>   XATTR_FORMTXTSHDWYVAL   (XATTR_TEXT_FIRST + 9);   // 1045
inline constexpr TypedWhichId<XFormTextHideFormItem>     XATTR_FORMTXTHIDEFORM   (XATTR_TEXT_FIRST + 10);  // 1046
inline constexpr TypedWhichId<XFormTextShadowTranspItem> XATTR_FORMTXTSHDWTRANSP (XATTR_TEXT_FIRST + 11);  // 1047
inline constexpr sal_uInt16                              XATTR_TEXT_LAST         (XATTR_FORMTXTSHDWTRANSP);// 1047

inline constexpr sal_uInt16 XATTR_END = XATTR_TEXT_LAST;                                                   // 1047

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
