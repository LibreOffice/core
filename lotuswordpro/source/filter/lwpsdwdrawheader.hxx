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
/*****************************************************************************
 * Change History
 * Mar 2005			Created
 ****************************************************************************/
/**
 * @file
 * For LWP filter architecture prototype
 * The file declares structures and enums used by Lwp-Drawing-Module
 */

#ifndef _LWPSDWRAWHEADER_HXX
#define _LWPSDWRAWHEADER_HXX

#include "lwpheader.hxx"
#include "lwptools.hxx"

const sal_uInt8 DRAW_FACESIZE = 32;
const double THRESHOLD = 0.0001;

enum DrawObjectType
{
    OT_UNDEFINED,
    OT_SELECT	   = 0,
    OT_HAND 	   = 1,
    OT_LINE 	   = 2,
    OT_PERPLINE	   = 3,
    OT_POLYLINE	   = 4,
    OT_POLYGON	   = 5,
    OT_RECT 	   = 6,
    OT_SQUARE	   = 7,
    OT_RNDRECT	   = 8,
    OT_RNDSQUARE   = 9,
    OT_OVAL 	   = 10,
    OT_CIRCLE	   = 11,
    OT_ARC		   = 12,
    OT_TEXT 	   = 13,
    OT_GROUP	   = 14,
    OT_CHART	   = 15,
    OT_METAFILE	   = 16,
    OT_METAFILEIMG = 17,
    OT_BITMAP	   = 18,
    OT_TEXTART     = 19,
    OT_BIGBITMAP   = 20
};

enum DrawFillType
{
    FT_TRANSPARENT	  = 0,
    FT_VLTGRAY		  = 1,
    FT_LTGRAY		  = 2,
    FT_GRAY 		  = 3,
    FT_DKGRAY		  = 4,
    FT_SOLID		  = 5,
    FT_HORZHATCH	  = 6,
    FT_VERTHATCH	  = 7,
    FT_FDIAGHATCH	  = 8,
    FT_BDIAGHATCH	  = 9,
    FT_CROSSHATCH	  = 10,
    FT_DIAGCROSSHATCH = 11,
    FT_PATTERN		  = 12
};

enum DrawLineStyle
{
    LS_SOLID	   = 0,
    LS_DASH 	   = 1,
    LS_DOT		   = 2,
    LS_DASHDOT	   = 3,
    LS_DASHDOTDOT  = 4,
    LS_NULL 	   = 5,
    LS_INSIDEFRAME = 6
};

// Text Attributes as stored in Draw files V1.2 and earlier
enum DrawTextAttribute
{
    TA_BOLD			  = 0x0001,	/* bolded font */
    TA_ITALIC		  = 0x0002,	/* italic font */
    TA_UNDERLINE	  =	0x0004,	/* underlined font */
    TA_WORDUNDERLINE  =	0x0008,	/* broken underline */
    TA_ALLCAPS		  = 0x0010,	/* capitalized font */
    TA_SMALLCAPS	  =	0x0020,	/* all small capital letters */
    TA_DOUBLEUNDER	  =	0x0040,	/* double underline */
    TA_STRIKETHRU	  =	0x0080,	/* strikethru */
    TA_SUPERSCRIPT	  =	0x0100,	/* superscript */
    TA_SUBSCRIPT	  =	0x0200	/* subscript */
};

enum DrawArrowHead
{
    AH_ARROW_NONE = 0,
    AH_ARROW_FULLARROW = 1,
    AH_ARROW_HALFARROW = 2,
    AH_ARROW_LINEARROW = 3,
    AH_ARROW_INVFULLARROW = 4,
    AH_ARROW_INVHALFARROW = 5,
    AH_ARROW_INVLINEARROW = 6,
    AH_ARROW_TEE = 7,
    AH_ARROW_SQUARE = 8,
    AH_ARROW_CIRCLE = 9
};

struct SdwPoint
{
    sal_Int16 x;
    sal_Int16 y;
};

struct SdwColor
{
    sal_uInt8 nR;
    sal_uInt8 nG;
    sal_uInt8 nB;
    sal_uInt8 unused;
};

struct SdwClosedObjStyleRec
{
    sal_uInt8 nLineWidth;
    sal_uInt8 nLineStyle;
    SdwColor aPenColor;
    SdwColor aForeColor;
    SdwColor aBackColor;
    sal_uInt16 nFillType;
    sal_uInt8 pFillPattern[8];
};

struct SdwDrawObjHeader
{
//	sal_uInt8 nType
//	sal_uInt8 nFlags;
    sal_uInt16 nRecLen;
    sal_Int16 nLeft;
    sal_Int16 nTop;
    sal_Int16 nRight;
    sal_Int16 nBottom;
//	sal_uInt16 nextObj;
//	sal_uInt16 prevObj;
};

struct SdwLineRecord
{
    sal_Int16 nStartX;
    sal_Int16 nStartY;
    sal_Int16 nEndX;
    sal_Int16 nEndY;
    sal_uInt8 nLineWidth;
    sal_uInt8 nLineEnd;
    sal_uInt8 nLineStyle;
    SdwColor aPenColor;
};

struct SdwPolyLineRecord
{
    sal_uInt8 nLineWidth;
    sal_uInt8 nLineEnd;
    sal_uInt8 nLineStyle;
    SdwColor aPenColor;
    sal_uInt16 nNumPoints;
};

struct SdwArcRecord
{
    sal_uInt8 nLineWidth;
    sal_uInt8 nLineEnd;
    sal_uInt8 nLineStyle;
    SdwColor aPenColor;
};

struct SdwTextBoxRecord
{
    sal_Int16 nTextWidth;
    sal_Int16 nTextHeight;
    sal_Int16 nTextSize;
    SdwColor aTextColor;
    sal_uInt8 tmpTextFaceName[DRAW_FACESIZE];
    sal_uInt16 nTextAttrs;
    sal_uInt16 nTextCharacterSet;
    sal_Int16 nTextRotation;
    sal_Int16 nTextExtraSpacing;
    sal_uInt8* pTextString;
};

struct SdwFMPATH
{
    sal_uInt16 n;
    SdwPoint* pPts;
};

struct SdwTextArt : public SdwTextBoxRecord
{
    sal_uInt8 nIndex;
    sal_Int16 nRotation;
    sal_uInt16 nTextLen;
    SdwFMPATH aPath[2];
};

struct SdwBmpRecord
{
    sal_uInt16 nTranslation;
    sal_uInt16 nRotation;
    sal_uInt32 nFileSize;
};

struct BmpInfoHeader
{
    sal_uInt32 nHeaderLen;
    sal_uInt16 nWidth;
    sal_uInt16 nHeight;
    sal_uInt16 nPlanes;
    sal_uInt16 nBitCount;
};

struct BmpInfoHeader2
{
    sal_uInt32 nHeaderLen;
    sal_uInt32 nWidth;
    sal_uInt32 nHeight;
    sal_uInt16 nPlanes;
    sal_uInt16 nBitCount;
};

struct DrawingOffsetAndScale
{
    double fOffsetX;
    double fOffsetY;
    double fScaleX;
    double fScaleY;
    double fLeftMargin;
    double fTopMargin;

    DrawingOffsetAndScale()
    {
        fOffsetX = 0.00;
        fOffsetY = 0.00;
        fScaleX = 1.00;
        fScaleY = 1.00;
        fLeftMargin = 0.00;
        fTopMargin = 0.00;
    }
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
