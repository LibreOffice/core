/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
/*************************************************************************
 * @file
 * enum const defines.
 ************************************************************************/
/*************************************************************************
 * Change History
 * 2005-01-21 create this file.
 ************************************************************************/
#ifndef		_XFDEFS_HXX
#define		_XFDEFS_HXX

#include	"sal/types.h"

enum	enumXFContent
{
    enumXFContentUnknown,
    enumXFContentText,
    enumXFContentSpan,
    enumXFContentPara,
    enumXFContentHeading,
    enumXFContentTable,
    enumXFContentTableCell,
    enumXFContentTableRow,
    enumXFContentTableCol,
    enumXFContentGraphics,
    enumXFContentFrame,
    enumXFContentContainer,
};

enum	enumXFStyle
{
    enumXFStyleUnknown,
    enumXFStyleText,
    enumXFStylePara,
    enumXFStyleList,
    enumXFStyleSection,
    enumXFStyleGraphics,
    enumXFStylePageMaster,
    enumXFStyleMasterPage,
    enumXFStyleDate,
    enumXFStyleTime,
    enumXFStylePercent,
    enumXFStyleNumber,
    enumXFStyleCurrency,
    enumXFStyleTable,
    enumXFStyleTableCell,
    enumXFStyleTableRow,
    enumXFStyleTableCol,
    enumXFStyleOutline,
    enumXFStyleStrokeDash,
    enumXFStyleArea,
    enumXFStyleArrow,
    enumXFStyleRuby,
    enumXFStyleDefaultPara,
};

enum enumXFFrameType
{
    enumXFFrameUnknown,
    enumXFFrameDrawing,
    enumXFFrameImage,
    enumXFFrameTextbox,
};

enum enumXFUnderline
{
    enumXFUnderlineNone = 0,
    enumXFUnderlineSingle,
    enumXFUnderlineDouble,
    enumXFUnderlineDotted,
    enumXFUnderlineDash,
    enumXFUnderlineLongDash,
    enumXFUnderlineDotDash,
    enumXFUnderlineDotDotDash,
    enumXFUnderlineWave,
    enumXFUnderlineBold,
    enumXFUnderlineBoldDotted,
    enumXFUnderlineBoldDash,
    enumXFUnderlineBoldLongDash,
    enumXFUnderlineBoldDotDash,
    enumXFUnderlineBoldDotDotDash,
    enumXFUnderlineBoldWave,
    enumXFUnderlineDoubleWave,
    enumXFUnderlineSmallWave
};

enum enumXFRelief
{
    enumXFReliefNone = 0,
    enumXFReliefEngraved,
    enumXFReliefEmbossed,
};

enum enumXFCrossout
{
    enumXFCrossoutNone = 0,
    enumXFCrossoutSignel,
    enumXFCrossoutDouble,
    enumXFCrossoutThick,
    enumXFCrossoutSlash,
    enumXFCrossoutX
};

enum enumXFTransform
{
    enumXFTransformNone = 0,
    enumXFTransformUpper,
    enumXFTransformLower,
    enumXFTransformCapitalize,
    enumXFTransformSmallCaps
};

enum enumXFEmphasize
{
    enumXFEmphasizeNone = 0,
    enumXFEmphasizeDot,
    enumXFEmphasizeCircle,
    enumXFEmphasizeDisc,
    enumXFEmphasizeAccent
};


enum enumXFAlignType
{
    enumXFAlignNone = 0,
    enumXFAlignStart,
    enumXFAlignCenter,
    enumXFAlignEnd,
    enumXFAlignJustify,
    enumXFAlignTop,
    enumXFAlignMiddle,
    enumXFAlignBottom,
    enumXFALignMargins,
};

enum enumXFShadowPos
{
    enumXFShadowNone,
    enumXFShadowRightBottom,
    enumXFShadowRightTop,
    enumXFShadowLeftBottom,
    enumXFShadowLeftTop
};

enum enumXFBorder
{
    enumXFBorderNone,
    enumXFBorderLeft,
    enumXFBorderRight,
    enumXFBorderTop,
    enumXFBorderBottom,
};

/**************************************************************************
 * @descr
 * Line height type.
 * enumLHHeight: height of the line,in "cm".
 * enumLHLeast: the height may be adjusted,but has a min value.
 * enumLHPercent: the height is a centain times of the normal height.
 * enumLHSpace: space between lines.
 *************************************************************************/
enum enumLHType
{
    enumLHNone,
    enumLHHeight,
    enumLHLeast,
    enumLHPercent,
    enumLHSpace,
};

enum enumXFBreaks
{
    enumXFBreakAuto,
    enumXFBreakBefPage,
    enumXFBreakBefColumn,
    enumXFBreakAftPage,
    enumXFBreakAftColumn,
    enumXFBreakKeepWithNext,
};

enum enumXFTab
{
    enumXFTabNone,
    enumXFTabLeft,
    enumXFTabCenter,
    enumXFTabRight,
    enumXFTabChar,
};

enum enumXFDatePart
{
    enumXFDateUnknown,
    enumXFDateYear,
    enumXFDateMonth,
    enumXFDateMonthDay,
    enumXFDateWeekDay,
    enumXFDateYearWeek,
    enumXFDateEra,
    enumXFDateQuarter,
    enumXFDateHour,
    enumXFDateMinute,
    enumXFDateSecond,
    enumXFDateAmPm,
    enumXFDateText
};

enum enumXFAnchor
{
    enumXFAnchorNone,
    enumXFAnchorPara,
    enumXFAnchorPage,
    enumXFAnchorChar,
    enumXFAnchorAsChar,
    enumXFAnchorFrame
};

enum enumXFWrap
{
    enumXFWrapNone,
    enumXFWrapLeft,
    enumXFWrapRight,
    enumXFWrapParallel,
    enumXFWrapRunThrough,
    enumXFWrapBest,
    enumXFWrapBackground
};

enum enumXFTextDir
{
    enumXFTextDirNone,
    enumXFTextDirLR,
    enumXFTextDirLR_TB,
    enumXFTextDirPage,
    enumXFTextDirRL,
    enumXFTextDirRL_TB,
    enumXFTextDirTB,
    enumXFTextDirTB_LR,
    enumXFTextDirTB_RL
};

enum enumXFFrameXRel
{
    enumXFFrameXRelPage,
    enumXFFrameXRelPageContent,
    enumXFFrameXRelPageEndMargin,
    enumXFFrameXRelPageStartMargin,
    enumXFFrameXRelPara,
    enumXFFrameXRelParaContent,
    enumXFFrameXRelParaEndMargin,
    enumXFFrameXRelParaStartMargin,
    enumXFFrameXRelChar,
    enumXFFrameXRelFrame,
    enumXFFrameXRelFrameContent,
    enumXFFrameXRelFrameEndMargin,
    enumXFFrameXRelFrameStartMargin
};

enum enumXFFrameXPos
{
    enumXFFrameXPosLeft,
    enumXFFrameXPosRight,
    enumXFFrameXPosCenter,
    enumXFFrameXPosFromLeft
};

enum enumXFFrameYRel
{
    enumXFFrameYRelBaseLine,
    enumXFFrameYRelChar,
    enumXFFrameYRelFrame,
    enumXFFrameYRelFrameContent,
    enumXFFrameYRelLine,
    enumXFFrameYRelPage,
    enumXFFrameYRelPageContent,
    enumXFFrameYRelPara,
    enumXFFrameYRelParaContent,
    enumXFFrameYRelText
};

enum enumXFFrameYPos
{
    enumXFFrameYPosTop,
    enumXFFrameYPosMiddle,
    enumXFFrameYPosBottom,
    enumXFFrameYPosFromTop,
    enumXFFrameYPosBelow,
};

enum enumXFTableColVisible
{
    enumXFTableColVisibleYes,
    enumXFTableColVisibleCollapse,
    enumXFTableColVisibleFilter,
};

enum enumXFBookmarkRef
{
    enumXFBookmarkNone,
    enumXFBookmarkPage,
    enumXFBookmarkChapter,
    enumXFBookmarkDir,
    enumXFBookmarkDef
};

enum enumXFDrawKind
{
    enumXFDrawKindFull,
    enumXFDrawKindSection,
    enumXFDrawKindCut,
    enumXFDrawKindArc
};

enum enumXFLineStyle
{
    enumXFLineNone,
    enumXFLineSolid,
    enumXFLineDot,
    enumXFLineDash,
    enumXFLineDotDash,
    enumXFLineDashDot
};

enum enumXFAreaStyle
{
    enumXFAreaNone,
    enumXFAreaSolid,
    enumXFAreaGradient,
    enumXFAreaBitmap,
    enumXFAreaHatch
};

enum enumXFAreaLineStyle
{
    enumXFAreaLineSingle,
    enumXFAreaLineCrossed,
    enumXFAreaLineTriple
};

enum enumXFColorMode
{
    enumXFColorStandard,
    enumXFColorGreyscale,
    enumXFColorMono,
    enumXFColorWatermark
};

enum enumXFPageUsage
{
    enumXFPageUsageNone,
    enumXFPageUsageAll,
    enumXFPageUsageLeft,
    enumXFPageUsageRight,
    enumXFPageUsageMirror
};

enum enumXFValueType
{
    enumXFValueTypeNone,
    enumXFValueTypeBoolean,
    enumXFValueTypeCurrency,
    enumXFValueTypeDate,
    enumXFValueTypeFloat,
    enumXFValueTypePercentage,
    enumXFValueTypeString,
    enumXFValueTypeTime,
};

enum enumXFNumberType
{
    enumXFNumberNumber,
    enumXFNumberPercent,
    enuMXFNumberCurrency,
    enumXFNumberScientific,
    enumXFText,
};

enum enumXFLineNumberPos
{
    enumXFLineNumberLeft,
    enumXFLineNumberRight,
    enumXFLineNumberInner,
    enumXFLineNumberOutter,
};

enum enumXFEntry
{
    enumXFEntryTOC,
    enumXFEntryAlphabetical,
    enumXFEntryUserIndex,
};

enum enumXFIndex
{
    enumXFIndexTOC,
    enumXFIndexAlphabetical,
    enumXFIndexUserIndex,
    enumXFIndexObject,
    enumXFIndexIllustration,
    enumXFIndexTableIndex,
};

enum enumXFIndexTemplate
{
    enumXFIndexTemplateUnknown,
    enumXFIndexTemplateChapter,
    enumXFIndexTemplateText,
    enumXFIndexTemplateTab,
    enumXFIndexTemplatePage,
    enumXFIndexTemplateLinkStart,
    enumXFIndexTemplateLinkEnd,
    enumXFIndexTemplateSpan,
    enumXFIndexTemplateBibliography,
};

struct XFDateTime
{
    sal_Int32 nYear;
    sal_Int32 nMonth;
    sal_Int32 nDay;
    sal_Int32 nHour;
    sal_Int32 nMinute;
    sal_Int32 nSecond;
    sal_Int32 nMillSecond;
};

enum enumXFRubyPosition
{
    enumXFRubyLeft = 4,
    enumXFRubyRight = 5,
    enumXFRubyCenter = 2,
    enumXFRubyTop = 1,
    enumXFRubyBottom = 3,
};
enum enumXFFWStyle
{
    enumXFFWOff,
    enumXFFWRotate,
    enumXFFWUpright,
    enumXFFWSlantX,//slant-x
    enumXFFWSlantY//slant-y
};

enum enumXFFWAdjust
{
    enumXFFWAdjustAutosize,//autosize
    enumXFFWAdjustLeft,
    enumXFFWAdjustCenter,//center
    enumXFFWAdustRight//right
};

enum enumXFFWShadow
{
    enumXFFWShadowNone,
    enumXFFWShadowNormal,
    enumXFFWShadowVertical,
    enumXFFWShadowSlant
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
