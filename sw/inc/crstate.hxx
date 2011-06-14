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
#ifndef _CRSTATE_HXX
#define _CRSTATE_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <tools/gen.hxx>
#include <swtypes.hxx>
#include <swrect.hxx>


enum SwFillMode
{
    FILL_TAB,       // default, fill with tabs
    FILL_SPACE,     // fill with spaces and tabs
    FILL_MARGIN,    // only align left, center, right
    FILL_INDENT     // by left paragraph indention
};

struct SwFillCrsrPos
{
    SwRect aCrsr;           // position and size of the ShadowCursor
    sal_uInt16 nParaCnt;        // number of paragraphs to insert
    sal_uInt16 nTabCnt;         // number of tabs respectively size of indentation
    sal_uInt16 nSpaceCnt;       // number of spaces to insert
    sal_uInt16 nColumnCnt;      // number of necessary column breaks
    sal_Int16  eOrient;      // paragraph alignment
    SwFillMode eMode;       // desired fill-up rule
    SwFillCrsrPos( SwFillMode eMd = FILL_TAB ) :
        nParaCnt( 0 ), nTabCnt( 0 ), nSpaceCnt( 0 ), nColumnCnt( 0 ),
        eOrient( com::sun::star::text::HoriOrientation::NONE ), eMode( eMd )
    {}
};

// Multiportion types: two lines, bidirectional, 270 degrees rotation,
//                     ruby portion and 90 degrees rotation
#define MT_TWOLINE  0
#define MT_BIDI     1
#define MT_ROT_270  3
#define MT_RUBY     4
#define MT_ROT_90   7

struct Sw2LinesPos
{
    SwRect aLine;           // Position and size of the line
    SwRect aPortion;        // Position and size of the multi portion
    SwRect aPortion2;       // needed for nested multi portions
    sal_uInt8 nMultiType;        // Multiportion type
};

/**
 *  SwSpecialPos. This structure is used to pass some additional information
 *  during the call of SwTxtFrm::GetCharRect(). An SwSpecialPos defines a position
 *  inside a portion which does not have a representation in the core string or
 *  which is only represented by one position,  e.g., field portions,
 *  number portions, ergo sum and quo vadis portions.
 *
 *  nCharOfst       - The offset inside the special portion. Fields and its
 *                    follow fields are treated as one long special portion.
 *  nLineOfst       - The number of lines between the beginning of the special
 *                    portion and nCharOfst. A line offset required to be
 *                    nCharOfst relative to the beginning of the line.
 *  nExtendRange    - Setting this identifies portions which are in front or
 *                    behind the core string (number portion, quo vadis)
 *
 *  Examples 1)
 *
 *      Get the position of the second character inside a number portion:
 *          nCharOfst = 2; nLineOfst = 0; nExtendRange = SP_EXTEND_RANGE_BEFORE;
 *          Call SwTxtFrm:::GetCharRect with core string position 0.
 *
 *  Example 2)
 *
 *      Field A - Length = 5
 *      Follow field B - Length = 9
 *      Get the position of the third character in follow field B, core position
 *      of field A is 33.
 *          nCharOfst = 7; nLineOfst = 0; nExtendRange = SP_EXTEND_RANGE_NONE;
 *          Call SwTxtFrm:::GetCharRect with core string position 33.
 */

#define SP_EXTEND_RANGE_NONE    0
#define SP_EXTEND_RANGE_BEFORE  1
#define SP_EXTEND_RANGE_BEHIND  2

struct SwSpecialPos
{
    xub_StrLen nCharOfst;
    sal_uInt16 nLineOfst;
    sal_uInt8 nExtendRange;

    // #i27615#
    SwSpecialPos() : nCharOfst(0), nLineOfst(0),
                     nExtendRange(SP_EXTEND_RANGE_NONE)
    {}
};

// CrsrTravelling-States (for GetCrsrOfst)
enum CrsrMoveState
{
    MV_NONE,            // default
    MV_UPDOWN,          // Crsr Up/Down
    MV_RIGHTMARGIN,     // at right margin
    MV_LEFTMARGIN,      // at left margin
    MV_SETONLYTEXT,     // stay with the cursor inside text
    MV_TBLSEL           // not in repeated headlines
};

// struct for later extensions
struct SwCrsrMoveState
{
    SwFillCrsrPos   *pFill;     // for automatic filling with tabs etc
    Sw2LinesPos     *p2Lines;   // for selections inside/around 2line portions
    SwSpecialPos*   pSpecialPos; // for positions inside fields
    Point aRealHeight;          // contains then the position/height of the cursor
    CrsrMoveState eState;
    sal_uInt8            nCursorBidiLevel;
    sal_Bool bStop          :1;
    sal_Bool bRealHeight    :1;     // should the real height be calculated?
    sal_Bool bFieldInfo     :1;     // should be fields recognized?
    sal_Bool bPosCorr       :1;     // Point had to be corrected
    sal_Bool bFtnNoInfo     :1;     // recognized footnote numbering
    sal_Bool bExactOnly     :1;     // let GetCrsrOfst look for exact matches only,
                                // i.e. never let it run into GetCntntPos
    sal_Bool bFillRet       :1;     // only used temporary in FillMode
    sal_Bool bSetInReadOnly :1;     // ReadOnly areas may be entered
    sal_Bool bRealWidth     :1;     // Calculation of the width required
    sal_Bool b2Lines        :1;     // Check 2line portions and fill p2Lines
    sal_Bool bNoScroll      :1;     // No scrolling of undersized textframes
    sal_Bool bPosMatchesBounds :1;  // GetCrsrOfst should not return the next
                                // position if screen position is inside second
                                // have of bound rect

    sal_Bool bCntntCheck :1; // --> FME 2005-05-13 #i43742# Cursor position over content? <--

    // #i27615#
    /**
       cursor in front of label
     */
    sal_Bool bInFrontOfLabel :1;
    sal_Bool bInNumPortion   :1;     // point is in number portion #i23726#
    int nInNumPostionOffset;     // distance from number portion's start

    SwCrsrMoveState( CrsrMoveState eSt = MV_NONE ) :
        pFill( NULL ),
        p2Lines( NULL ),
        pSpecialPos( NULL ),
        eState( eSt ),
        nCursorBidiLevel( 0 ),
        bStop( sal_False ),
        bRealHeight( sal_False ),
        bFieldInfo( sal_False ),
        bPosCorr( sal_False ),
        bFtnNoInfo( sal_False ),
        bExactOnly( sal_False ),
        bSetInReadOnly( sal_False ),
        bRealWidth( sal_False ),
        b2Lines( sal_False ),
        bNoScroll( sal_False ),
        bPosMatchesBounds( sal_False ),
        bCntntCheck( sal_False ), // #i43742#
        bInFrontOfLabel( sal_False ), // #i27615#
        bInNumPortion(sal_False), // #i26726#
        nInNumPostionOffset(0) // #i26726#
    {}
    SwCrsrMoveState( SwFillCrsrPos *pInitFill ) :
        pFill( pInitFill ),
        pSpecialPos( NULL ),
        eState( MV_SETONLYTEXT ),
        nCursorBidiLevel( 0 ),
        bStop( sal_False ),
        bRealHeight( sal_False ),
        bFieldInfo( sal_False ),
        bPosCorr( sal_False ),
        bFtnNoInfo( sal_False ),
        bExactOnly( sal_False ),
        bSetInReadOnly( sal_False ),
        bRealWidth( sal_False ),
        b2Lines( sal_False ),
        bNoScroll( sal_False ),
        bPosMatchesBounds( sal_False ),
        bCntntCheck( sal_False ), // #i43742#
        bInFrontOfLabel( sal_False ), // #i27615#
        bInNumPortion(sal_False), // #i23726#
        nInNumPostionOffset(0) // #i23726#
    {}
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
