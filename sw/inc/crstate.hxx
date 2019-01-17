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
#ifndef INCLUDED_SW_INC_CRSTATE_HXX
#define INCLUDED_SW_INC_CRSTATE_HXX

#include <com/sun/star/text/HoriOrientation.hpp>
#include <tools/gen.hxx>
#include "swrect.hxx"

enum SwFillMode
{
    FILL_TAB,       ///< default, fill with tabs
    FILL_TAB_SPACE, ///< fill with spaces and tabs
    FILL_SPACE,     ///< fill with spaces
    FILL_MARGIN,    ///< only align left, center, right
    FILL_INDENT     ///< by left paragraph indention
};

struct SwFillCursorPos
{
    SwRect aCursor;           ///< position and size of the ShadowCursor
    sal_uInt16 nParaCnt;        ///< number of paragraphs to insert
    sal_uInt16 nTabCnt;         ///< number of tabs respectively size of indentation
    sal_uInt16 nSpaceCnt;       ///< number of spaces to insert
    sal_uInt16 nSpaceOnlyCnt;   ///< number of spaces to insert ("only spaces, no tabs" mode)
    sal_uInt16 nColumnCnt;      ///< number of necessary column breaks
    sal_Int16  eOrient;      ///< paragraph alignment
    SwFillMode const eMode; ///< desired fill-up rule
    SwFillCursorPos( SwFillMode eMd ) :
        nParaCnt( 0 ), nTabCnt( 0 ), nSpaceCnt( 0 ), nSpaceOnlyCnt(0), nColumnCnt( 0 ),
        eOrient( css::text::HoriOrientation::NONE ), eMode( eMd )
    {}
};

// Multiportion types: two lines, bidirectional, 270 degrees rotation,
//                     ruby portion and 90 degrees rotation
enum class MultiPortionType : sal_uInt8
{
        TWOLINE  = 0,
        BIDI     = 1,
        ROT_270  = 3,
        RUBY     = 4,
        ROT_90   = 7,
};

struct Sw2LinesPos
{
    SwRect aLine;           ///< Position and size of the line
    SwRect aPortion;        ///< Position and size of the multi portion
    SwRect aPortion2;       ///< needed for nested multi portions
    MultiPortionType nMultiType;  ///< Multiportion type
};

/**
 *  SwSpecialPos. This structure is used to pass some additional information
 *  during the call of SwTextFrame::GetCharRect(). An SwSpecialPos defines a position
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
 *          Call SwTextFrame:::GetCharRect with core string position 0.
 *
 *  Example 2)
 *
 *      Field A - Length = 5
 *      Follow field B - Length = 9
 *      Get the position of the third character in follow field B, core position
 *      of field A is 33.
 *          nCharOfst = 7; nLineOfst = 0; nExtendRange = SP_EXTEND_RANGE_NONE;
 *          Call SwTextFrame:::GetCharRect with core string position 33.
 */

enum class SwSPExtendRange : sal_uInt8
{
    NONE, BEFORE, BEHIND
};

struct SwSpecialPos
{
    sal_Int32 nCharOfst;
    sal_uInt16 nLineOfst;
    SwSPExtendRange nExtendRange;

    // #i27615#
    SwSpecialPos() : nCharOfst(0), nLineOfst(0),
                     nExtendRange(SwSPExtendRange::NONE)
    {}
};

// CursorTravelling-States (for GetCursorOfst)
enum CursorMoveState
{
    MV_NONE,            ///< default
    MV_UPDOWN,          ///< Cursor Up/Down
    MV_RIGHTMARGIN,     ///< at right margin
    MV_LEFTMARGIN,      ///< at left margin
    MV_SETONLYTEXT,     ///< stay with the cursor inside text
    MV_TBLSEL           ///< not in repeated headlines
};

// struct for later extensions
struct SwCursorMoveState
{
    SwFillCursorPos   *m_pFill;     ///< for automatic filling with tabs etc
    std::unique_ptr<Sw2LinesPos> m_p2Lines;   ///< for selections inside/around 2line portions
    SwSpecialPos*   m_pSpecialPos; ///< for positions inside fields
    Point m_aRealHeight;          ///< contains then the position/height of the cursor
    CursorMoveState m_eState;
    sal_uInt8            m_nCursorBidiLevel;
    bool m_bStop;
    bool m_bRealHeight;           ///< should the real height be calculated?
    bool m_bFieldInfo;            ///< should be fields recognized?
    bool m_bPosCorr;              ///< Point had to be corrected
    bool m_bFootnoteNoInfo;            ///< recognized footnote numbering
    bool m_bExactOnly;            /**< let GetCursorOfst look for exact matches only,
                                         i.e. never let it run into GetContentPos */
    bool m_bFillRet;              ///< only used temporary in FillMode
    bool m_bSetInReadOnly;        ///< ReadOnly areas may be entered
    bool m_bRealWidth;            ///< Calculation of the width required
    bool m_b2Lines;               ///< Check 2line portions and fill p2Lines
    bool m_bNoScroll;             ///< No scrolling of undersized textframes
    bool m_bPosMatchesBounds;         /**< GetCursorOfst should not return the next
                                       position if screen position is inside second
                                       have of bound rect */

    bool m_bContentCheck;           // #i43742# Cursor position over content?

    // #i27615#
    /**
       cursor in front of label
     */
    bool m_bInFrontOfLabel;
    bool m_bInNumPortion;         ///< point is in number portion #i23726#
    int m_nInNumPortionOffset;        ///< distance from number portion's start

    SwCursorMoveState( CursorMoveState eSt = MV_NONE ) :
        m_pFill( nullptr ),
        m_pSpecialPos( nullptr ),
        m_eState( eSt ),
        m_nCursorBidiLevel( 0 ),
        m_bStop( false ),
        m_bRealHeight( false ),
        m_bFieldInfo( false ),
        m_bPosCorr( false ),
        m_bFootnoteNoInfo( false ),
        m_bExactOnly( false ),
        m_bFillRet( false ),
        m_bSetInReadOnly( false ),
        m_bRealWidth( false ),
        m_b2Lines( false ),
        m_bNoScroll( false ),
        m_bPosMatchesBounds( false ),
        m_bContentCheck( false ), // #i43742#
        m_bInFrontOfLabel( false ), // #i27615#
        m_bInNumPortion(false), // #i26726#
        m_nInNumPortionOffset(0) // #i26726#
    {}
    SwCursorMoveState( SwFillCursorPos *pInitFill ) :
        m_pFill( pInitFill ),
        m_pSpecialPos( nullptr ),
        m_eState( MV_SETONLYTEXT ),
        m_nCursorBidiLevel( 0 ),
        m_bStop( false ),
        m_bRealHeight( false ),
        m_bFieldInfo( false ),
        m_bPosCorr( false ),
        m_bFootnoteNoInfo( false ),
        m_bExactOnly( false ),
        m_bFillRet( false ),
        m_bSetInReadOnly( false ),
        m_bRealWidth( false ),
        m_b2Lines( false ),
        m_bNoScroll( false ),
        m_bPosMatchesBounds( false ),
        m_bContentCheck( false ), // #i43742#
        m_bInFrontOfLabel( false ), // #i27615#
        m_bInNumPortion(false), // #i23726#
        m_nInNumPortionOffset(0) // #i23726#
    {}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
