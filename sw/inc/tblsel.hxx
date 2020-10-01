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
#ifndef INCLUDED_SW_INC_TBLSEL_HXX
#define INCLUDED_SW_INC_TBLSEL_HXX

#include "swtable.hxx"
#include "swrect.hxx"
#include "swdllapi.h"

#include <o3tl/sorted_vector.hxx>

#include <memory>
#include <deque>
#include <vector>

class SwCursorShell;
class SwCursor;
class SwTableCursor;
class SwFrame;
class SwTabFrame;
class SwLayoutFrame;
class SwPaM;
class SwNode;
class SwUndoTableMerge;
class SwCellFrame;

typedef std::deque< SwCellFrame* > SwCellFrames;

struct CompareSwSelBoxes
{
    bool operator()(SwTableBox* const& lhs, SwTableBox* const& rhs) const
    {
        return lhs->GetSttIdx() < rhs->GetSttIdx();
    }
};
class SwSelBoxes : public o3tl::sorted_vector<SwTableBox*, CompareSwSelBoxes> {};

// Collects all boxes in table that are selected.
// Selection gets extended in given direction according to enum-parameter.
// Boxes are collected via the Layout; works correctly if tables are split.
// (Cf. MakeSelUnions().)
enum class SwTableSearchType : sal_uInt16
{
    NONE           = 0x01, // No extension.
    Row            = 0x02, // Extend to rows.
    Col            = 0x03, // Extend to columns.

    // As flags to the other values!
    Protect        = 0x08, // Collect protected boxes too.
    NoUnionCorrect = 0x10, // Do not correct collected Union.
};
namespace o3tl {
    template<> struct typed_flags<SwTableSearchType> : is_typed_flags<SwTableSearchType, 0x1b> {};
}

SW_DLLPUBLIC void GetTableSel( const SwCursorShell& rShell, SwSelBoxes& rBoxes,
                const SwTableSearchType = SwTableSearchType::NONE );

void GetTableSel( const SwCursor& rCursor, SwSelBoxes& rBoxes,
                const SwTableSearchType = SwTableSearchType::NONE );

// As before, but don't start from selection but from Start- EndFrames.
void GetTableSel( const SwLayoutFrame* pStart, const SwLayoutFrame* pEnd,
                SwSelBoxes& rBoxes, SwCellFrames* pCells,
                const SwTableSearchType = SwTableSearchType::NONE );

// As before but directly via PaMs.
void GetTableSelCrs( const SwCursorShell& rShell, SwSelBoxes& rBoxes );
void GetTableSelCrs( const SwTableCursor& rTableCursor, SwSelBoxes& rBoxes );

// Collect boxes relevant for auto sum.
bool GetAutoSumSel( const SwCursorShell&, SwCellFrames& );

// Check if the SelBoxes contains protected Boxes.
bool HasProtectedCells( const SwSelBoxes& rBoxes );

// Check if selection is balanced.
bool ChkChartSel( const SwNode& rSttNd, const SwNode& rEndNd);

// Check if cell is part of SSelection.
// (Became a function, in order to make sure that GetTableSel() and MakeTableCursor()
// have always the same concept of the selection.
bool IsFrameInTableSel( const SwRect& rUnion, const SwFrame* pCell );

// Determine boxes to be merged.
// In this process the rectangle gets "adapted" on the base of the layout,
// i.e. boxes are added if some overlap at the sides.
// Additionally a new box is created and filled with the relevant content.
void GetMergeSel( const SwPaM& rPam, SwSelBoxes& rBoxes,
                  SwTableBox** ppMergeBox, SwUndoTableMerge* pUndo );

// Check if selected boxes allow for a valid merge.
TableMergeErr CheckMergeSel( const SwPaM& rPam );
TableMergeErr CheckMergeSel( const SwSelBoxes& rBoxes );

bool IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam );

// Check if Split or InsertCol lead to a box becoming smaller than MINLAY.
bool CheckSplitCells( const SwCursorShell& rShell, sal_uInt16 nDiv,
                        const SwTableSearchType );
bool CheckSplitCells( const SwCursor& rCursor, sal_uInt16 nDiv,
                        const SwTableSearchType );

// For working on tab selection also for split tables.
class SwSelUnion
{
    SwRect   m_aUnion;        // The rectangle enclosing the selection.
    SwTabFrame *m_pTable;       // The (Follow-)Table for the Union.

public:
    SwSelUnion( const SwRect &rRect, SwTabFrame *pTab ) :
        m_aUnion( rRect ), m_pTable( pTab ) {}

    const SwRect&   GetUnion() const { return m_aUnion; }
          SwRect&   GetUnion()       { return m_aUnion; }
    const SwTabFrame *GetTable() const { return m_pTable; }
          SwTabFrame *GetTable()       { return m_pTable; }
};

// Determines tables affected by a table selection and union rectangles
// of the selection (also for split tables)
typedef std::vector<SwSelUnion> SwSelUnions;

// Gets the tables involved in a table selection and the union-rectangles of the selections
// - also for split tables.
// If a parameter is passed that != SwTableSearchType::NONE
// the selection is extended in the given direction.
void MakeSelUnions( SwSelUnions&, const SwLayoutFrame *pStart,
                    const SwLayoutFrame *pEnd,
                    const SwTableSearchType = SwTableSearchType::NONE );

// These classes copy the current table selections (rBoxes) into a
// separate structure while keeping the table structure.

class FndBox_;
class FndLine_;

typedef std::vector<std::unique_ptr<FndBox_>> FndBoxes_t;
typedef std::vector<std::unique_ptr<FndLine_>> FndLines_t;

class FndBox_
{
    SwTableBox* m_pBox;
    FndLines_t m_Lines;
    FndLine_* m_pUpper;

    SwTableLine *m_pLineBefore; ///< For deleting/restoring the layout.
    SwTableLine *m_pLineBehind; ///< For deleting/restoring the layout.

    FndBox_(FndBox_ const&) = delete;
    FndBox_& operator=(FndBox_ const&) = delete;

public:
    FndBox_( SwTableBox* pB, FndLine_* pFL ) :
        m_pBox(pB), m_pUpper(pFL), m_pLineBefore( nullptr ), m_pLineBehind( nullptr ) {}

    const FndLines_t&   GetLines() const    { return m_Lines; }
        FndLines_t&     GetLines()          { return m_Lines; }
    const SwTableBox*   GetBox() const      { return m_pBox; }
        SwTableBox*     GetBox()            { return m_pBox; }
    const FndLine_*     GetUpper() const    { return m_pUpper; }
        FndLine_*       GetUpper()          { return m_pUpper; }

    void SetTableLines( const SwSelBoxes &rBoxes, const SwTable &rTable );
    void SetTableLines( const SwTable &rTable );
    //Add an input param to identify if acc table should be disposed
    void DelFrames ( SwTable &rTable );
    void MakeFrames( SwTable &rTable );
    void MakeNewFrames( SwTable &rTable, const sal_uInt16 nNumber,
                                       const bool bBehind );
    bool AreLinesToRestore( const SwTable &rTable ) const;

    void ClearLineBehind() { m_pLineBehind = nullptr; }
};

class FndLine_
{
    SwTableLine* m_pLine;
    FndBoxes_t m_Boxes;
    FndBox_* m_pUpper;

    FndLine_(FndLine_ const&) = delete;
    FndLine_& operator=(FndLine_ const&) = delete;

public:
    FndLine_(SwTableLine* pL, FndBox_* pFB) : m_pLine(pL), m_pUpper(pFB) {}
    const FndBoxes_t&   GetBoxes() const    { return m_Boxes; }
        FndBoxes_t&     GetBoxes()          { return m_Boxes; }
    const SwTableLine*  GetLine() const     { return m_pLine; }
        SwTableLine*    GetLine()           { return m_pLine; }
    const FndBox_*      GetUpper() const    { return m_pUpper; }
        FndBox_*        GetUpper()          { return m_pUpper; }

    void SetUpper( FndBox_* pUp ) { m_pUpper = pUp; }
};

struct FndPara
{
    const SwSelBoxes& rBoxes;
    FndLine_* pFndLine;
    FndBox_* pFndBox;

    FndPara( const SwSelBoxes& rBxs, FndBox_* pFB )
        : rBoxes(rBxs), pFndLine(nullptr), pFndBox(pFB) {}
    FndPara( const FndPara& rPara, FndBox_* pFB )
        : rBoxes(rPara.rBoxes), pFndLine(rPara.pFndLine), pFndBox(pFB) {}
    FndPara( const FndPara& rPara, FndLine_* pFL )
        : rBoxes(rPara.rBoxes), pFndLine(pFL), pFndBox(rPara.pFndBox) {}
};

/** This creates a structure mirroring the SwTable structure that contains all
    rows and non-leaf boxes (as pointers to SwTableBox/SwTableLine, not copies),
    plus the leaf boxes that are selected by pFndPara->rBoxes
 */
SW_DLLPUBLIC void ForEach_FndLineCopyCol(SwTableLines& rLines, FndPara* pFndPara );

#endif // INCLUDED_SW_INC_TBLSEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
