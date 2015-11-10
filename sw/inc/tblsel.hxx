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

#include <swtable.hxx>
#include <swrect.hxx>
#include "swdllapi.h"

#include <o3tl/sorted_vector.hxx>

#include <memory>
#include <deque>
#include <vector>

class SwCrsrShell;
class SwCursor;
class SwTableCursor;
class SwFrm;
class SwTabFrm;
class SwTableBox;
class SwTableLine;
class SwLayoutFrm;
class SwPaM;
class SwNode;
class SwTable;
class SwUndoTableMerge;
class SwCellFrm;

typedef ::std::deque< SwCellFrm* > SwCellFrms;

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
typedef sal_uInt16 SwTableSearchType;
namespace nsSwTableSearchType
{
    const SwTableSearchType TBLSEARCH_NONE = 0x1;       // No extension.
    const SwTableSearchType TBLSEARCH_ROW  = 0x2;       // Extend to rows.
    const SwTableSearchType TBLSEARCH_COL  = 0x3;       // Extend to columns.

    // As flag to the other values!
    const SwTableSearchType TBLSEARCH_PROTECT = 0x8;      // Collect protected boxes too.
    const SwTableSearchType TBLSEARCH_NO_UNION_CORRECT = 0x10; // Do not correct collected Union.
}

SW_DLLPUBLIC void GetTableSel( const SwCrsrShell& rShell, SwSelBoxes& rBoxes,
                const SwTableSearchType = nsSwTableSearchType::TBLSEARCH_NONE );

void GetTableSel( const SwCursor& rCrsr, SwSelBoxes& rBoxes,
                const SwTableSearchType = nsSwTableSearchType::TBLSEARCH_NONE );

// As before, but don't start from selection but from Start- EndFrms.
void GetTableSel( const SwLayoutFrm* pStart, const SwLayoutFrm* pEnd,
                SwSelBoxes& rBoxes, SwCellFrms* pCells,
                const SwTableSearchType = nsSwTableSearchType::TBLSEARCH_NONE );

// As before but directly via PaMs.
void GetTableSelCrs( const SwCrsrShell& rShell, SwSelBoxes& rBoxes );
void GetTableSelCrs( const SwTableCursor& rTableCrsr, SwSelBoxes& rBoxes );

// Collect boxes relevant for auto sum.
bool GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

// Check if the SelBoxes contains protected Boxes.
bool HasProtectedCells( const SwSelBoxes& rBoxes );

// Check if selection is balanced.
bool ChkChartSel( const SwNode& rSttNd, const SwNode& rEndNd);

// Check if cell is part of SSelection.
// (Became a function, in order to make sure that GetTableSel() and MakeTableCrsr()
// have always the same concept of the selection.
bool IsFrmInTableSel( const SwRect& rUnion, const SwFrm* pCell );

// Determine boxes to be merged.
// In this process the rectangle gets "adapted" on the base of the layout,
// i.e. boxes are added if some overlap at the sides.
// Additionally a new box is created and filled with the relevant content.
void GetMergeSel( const SwPaM& rPam, SwSelBoxes& rBoxes,
                  SwTableBox** ppMergeBox, SwUndoTableMerge* pUndo = nullptr );

// Check if selected boxes allow for a valid merge.
sal_uInt16 CheckMergeSel( const SwPaM& rPam );
sal_uInt16 CheckMergeSel( const SwSelBoxes& rBoxes );

bool IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam );

// Check if Split or InsertCol lead to a box becoming smaller than MINLAY.
bool CheckSplitCells( const SwCrsrShell& rShell, sal_uInt16 nDiv,
                        const SwTableSearchType = nsSwTableSearchType::TBLSEARCH_NONE );
bool CheckSplitCells( const SwCursor& rCrsr, sal_uInt16 nDiv,
                        const SwTableSearchType = nsSwTableSearchType::TBLSEARCH_NONE );

// For working on tab selection also for split tables.
class SwSelUnion
{
    SwRect   m_aUnion;        // The rectangle enclosing the selection.
    SwTabFrm *m_pTable;       // The (Follow-)Table for the Union.

public:
    SwSelUnion( const SwRect &rRect, SwTabFrm *pTab ) :
        m_aUnion( rRect ), m_pTable( pTab ) {}

    const SwRect&   GetUnion() const { return m_aUnion; }
          SwRect&   GetUnion()       { return m_aUnion; }
    const SwTabFrm *GetTable() const { return m_pTable; }
          SwTabFrm *GetTable()       { return m_pTable; }
};

// Determines tables affected by a table selection and union rectangles
// of the selection (also for split tables)
typedef std::vector<SwSelUnion> SwSelUnions;

// Gets the tables involved in a table selection and the union-rectangles of the selections
// - also for split tables.
// If a parameter is passed that != nsSwTableSearchType::TBLSEARCH_NONE
// the selection is extended in the given direction.
void MakeSelUnions( SwSelUnions&, const SwLayoutFrm *pStart,
                    const SwLayoutFrm *pEnd,
                    const SwTableSearchType = nsSwTableSearchType::TBLSEARCH_NONE );

// These classes copy the current table selections (rBoxes) into a
// separate structure while keeping the table structure.

class _FndBox;
class _FndLine;

typedef std::vector<std::unique_ptr<_FndBox>> FndBoxes_t;
typedef std::vector<std::unique_ptr<_FndLine>> FndLines_t;

class _FndBox
{
    SwTableBox* pBox;
    FndLines_t m_Lines;
    _FndLine* pUpper;

    SwTableLine *pLineBefore;   // For deleting/restoring the layout.
    SwTableLine *pLineBehind;

    _FndBox(_FndBox const&) = delete;
    _FndBox& operator=(_FndBox const&) = delete;

public:
    _FndBox( SwTableBox* pB, _FndLine* pFL ) :
        pBox(pB), pUpper(pFL), pLineBefore( nullptr ), pLineBehind( nullptr ) {}

    const FndLines_t&   GetLines() const    { return m_Lines; }
        FndLines_t&     GetLines()          { return m_Lines; }
    const SwTableBox*   GetBox() const      { return pBox; }
        SwTableBox*     GetBox()            { return pBox; }
    const _FndLine*     GetUpper() const    { return pUpper; }
        _FndLine*       GetUpper()          { return pUpper; }

    void SetTableLines( const SwSelBoxes &rBoxes, const SwTable &rTable );
    void SetTableLines( const SwTable &rTable );
    //Add an input param to identify if acc table should be disposed
    void DelFrms ( SwTable &rTable, bool bAccTableDispose = false );
    void MakeFrms( SwTable &rTable );
    void MakeNewFrms( SwTable &rTable, const sal_uInt16 nNumber,
                                       const bool bBehind );
    bool AreLinesToRestore( const SwTable &rTable ) const;

    void ClearLineBehind() { pLineBehind = nullptr; }
};

class _FndLine
{
    SwTableLine* pLine;
    FndBoxes_t m_Boxes;
    _FndBox* pUpper;

    _FndLine(_FndLine const&) = delete;
    _FndLine& operator=(_FndLine const&) = delete;

public:
    _FndLine(SwTableLine* pL, _FndBox* pFB=nullptr) : pLine(pL), pUpper(pFB) {}
    const FndBoxes_t&   GetBoxes() const    { return m_Boxes; }
        FndBoxes_t&     GetBoxes()          { return m_Boxes; }
    const SwTableLine*  GetLine() const     { return pLine; }
        SwTableLine*    GetLine()           { return pLine; }
    const _FndBox*      GetUpper() const    { return pUpper; }
        _FndBox*        GetUpper()          { return pUpper; }

    void SetUpper( _FndBox* pUp ) { pUpper = pUp; }
};

struct _FndPara
{
    const SwSelBoxes& rBoxes;
    _FndLine* pFndLine;
    _FndBox* pFndBox;

    _FndPara( const SwSelBoxes& rBxs, _FndBox* pFB )
        : rBoxes(rBxs), pFndLine(nullptr), pFndBox(pFB) {}
    _FndPara( const _FndPara& rPara, _FndBox* pFB )
        : rBoxes(rPara.rBoxes), pFndLine(rPara.pFndLine), pFndBox(pFB) {}
    _FndPara( const _FndPara& rPara, _FndLine* pFL )
        : rBoxes(rPara.rBoxes), pFndLine(pFL), pFndBox(rPara.pFndBox) {}
};

SW_DLLPUBLIC void ForEach_FndLineCopyCol(SwTableLines& rLines, _FndPara* pFndPara );

#endif // INCLUDED_SW_INC_TBLSEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
