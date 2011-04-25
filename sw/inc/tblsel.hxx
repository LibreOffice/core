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
#ifndef _TBLSEL_HXX
#define _TBLSEL_HXX

#include <svl/svarray.hxx>
#include <swtable.hxx>
#include <swrect.hxx>
#include "swdllapi.h"

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
class SwUndoTblMerge;
class SwCellFrm;

SV_DECL_PTRARR( SwCellFrms, SwCellFrm*, 16, 16 )
SV_DECL_PTRARR_SORT( SwSelBoxes, SwTableBoxPtr, 10, 20 )


// Collects all boxes in table that are selected.
// Selection gets extended in given direction according to enum-parameter.
// Boxes are collected via the Layout; works correctly if tables are split.
// (Cf. MakeSelUnions().)
typedef sal_uInt16 SwTblSearchType;
namespace nsSwTblSearchType
{
    const SwTblSearchType TBLSEARCH_NONE = 0x1;       // No extension.
    const SwTblSearchType TBLSEARCH_ROW  = 0x2;       // Extend to rows.
    const SwTblSearchType TBLSEARCH_COL  = 0x3;       // Extend to columns.

    // As flag to the other values!
    const SwTblSearchType TBLSEARCH_PROTECT = 0x8;      // Collect protected boxes too.
    const SwTblSearchType TBLSEARCH_NO_UNION_CORRECT = 0x10; // Do not correct collected Union.
}

SW_DLLPUBLIC void GetTblSel( const SwCrsrShell& rShell, SwSelBoxes& rBoxes,
                const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );

void GetTblSel( const SwCursor& rCrsr, SwSelBoxes& rBoxes,
                const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );


// As before, but don't start from selection but from Start- EndFrms.
void GetTblSel( const SwLayoutFrm* pStart, const SwLayoutFrm* pEnd,
                SwSelBoxes& rBoxes, SwCellFrms* pCells,
                const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );

// As before but directly via PaMs.
void GetTblSelCrs( const SwCrsrShell& rShell, SwSelBoxes& rBoxes );
void GetTblSelCrs( const SwTableCursor& rTblCrsr, SwSelBoxes& rBoxes );

// Collect boxes relevant for auto sum.
sal_Bool GetAutoSumSel( const SwCrsrShell&, SwCellFrms& );

// Check if the SelBoxes contains protected Boxes.
sal_Bool HasProtectedCells( const SwSelBoxes& rBoxes );

// Check if selection is balanced.
SV_DECL_PTRARR( SwChartBoxes, SwTableBoxPtr, 16, 16)
SV_DECL_PTRARR_DEL( SwChartLines, SwChartBoxes*, 25, 50)

sal_Bool ChkChartSel( const SwNode& rSttNd, const SwNode& rEndNd,
                    SwChartLines* pGetCLines = 0 );

// Check if cell is part of SSelection.
// (Became a function, in order to make sure that GetTblSel() and MakeTblCrsr()
// have always the same concept of the selection.
sal_Bool IsFrmInTblSel( const SwRect& rUnion, const SwFrm* pCell );

// Determine boxes to be merged.
// In this process the rectangle gets "adapted" on the base of the layout,
// i.e. boxes are added if some overlap at the sides.
// Additionally a new box is created and filled with the relevant content.
void GetMergeSel( const SwPaM& rPam, SwSelBoxes& rBoxes,
                  SwTableBox** ppMergeBox, SwUndoTblMerge* pUndo = 0 );

// Check if selected boxes allow for a valid merge.
sal_uInt16 CheckMergeSel( const SwPaM& rPam );
sal_uInt16 CheckMergeSel( const SwSelBoxes& rBoxes );

sal_Bool IsEmptyBox( const SwTableBox& rBox, SwPaM& rPam );

// Check if Split or InsertCol lead to a box becoming smaller than MINLAY.
sal_Bool CheckSplitCells( const SwCrsrShell& rShell, sal_uInt16 nDiv,
                        const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );
sal_Bool CheckSplitCells( const SwCursor& rCrsr, sal_uInt16 nDiv,
                        const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );

// For working on tab selection also for split tables.
class SwSelUnion
{
    SwRect   aUnion;        // The rectangle enclosing the selection.
    SwTabFrm *pTable;       // The (Follow-)Table for the Union.

public:
    SwSelUnion( const SwRect &rRect, SwTabFrm *pTab ) :
        aUnion( rRect ), pTable( pTab ) {}

    const SwRect&   GetUnion() const { return aUnion; }
          SwRect&   GetUnion()       { return aUnion; }
    const SwTabFrm *GetTable() const { return pTable; }
          SwTabFrm *GetTable()       { return pTable; }
};

SV_DECL_PTRARR_DEL( SwSelUnions, SwSelUnion*, 10, 20 )

// Gets the tables involved in a table selection and the union-rectangles of the selections
// - also for split tables.
// If a parameter is passed that != nsSwTblSearchType::TBLSEARCH_NONE
// the selection is extended in the given direction.
void MakeSelUnions( SwSelUnions&, const SwLayoutFrm *pStart,
                    const SwLayoutFrm *pEnd,
                    const SwTblSearchType = nsSwTblSearchType::TBLSEARCH_NONE );



// These classes copy the current table selections (rBoxes) into a
// separate structure while keeping the table structure.

class _FndBox;
class _FndLine;

SV_DECL_PTRARR_DEL( _FndBoxes, _FndBox*, 10, 20 )
SV_DECL_PTRARR_DEL( _FndLines, _FndLine*,10, 20 )

class _FndBox
{
    SwTableBox* pBox;
    _FndLines aLines;
    _FndLine* pUpper;

    SwTableLine *pLineBefore;   // For deleting/restoring the layout.
    SwTableLine *pLineBehind;

public:
    _FndBox( SwTableBox* pB, _FndLine* pFL ) :
        pBox(pB), pUpper(pFL), pLineBefore( 0 ), pLineBehind( 0 ) {}

    const _FndLines&    GetLines() const    { return aLines; }
        _FndLines&      GetLines()          { return aLines; }
    const SwTableBox*   GetBox() const      { return pBox; }
        SwTableBox*     GetBox()            { return pBox; }
    const _FndLine*     GetUpper() const    { return pUpper; }
        _FndLine*       GetUpper()          { return pUpper; }

    void SetTableLines( const SwSelBoxes &rBoxes, const SwTable &rTable );
    void SetTableLines( const SwTable &rTable );
    void DelFrms ( SwTable &rTable );
    void MakeFrms( SwTable &rTable );
    void MakeNewFrms( SwTable &rTable, const sal_uInt16 nNumber,
                                       const sal_Bool bBehind );
    sal_Bool AreLinesToRestore( const SwTable &rTable ) const;

    void ClearLineBehind() { pLineBehind = 0; }
};


class _FndLine
{
    SwTableLine* pLine;
    _FndBoxes aBoxes;
    _FndBox* pUpper;
public:
    _FndLine(SwTableLine* pL, _FndBox* pFB=0) : pLine(pL), pUpper(pFB) {}
    const _FndBoxes&    GetBoxes() const    { return aBoxes; }
        _FndBoxes&      GetBoxes()          { return aBoxes; }
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
        : rBoxes(rBxs), pFndLine(0), pFndBox(pFB) {}
    _FndPara( const _FndPara& rPara, _FndBox* pFB )
        : rBoxes(rPara.rBoxes), pFndLine(rPara.pFndLine), pFndBox(pFB) {}
    _FndPara( const _FndPara& rPara, _FndLine* pFL )
        : rBoxes(rPara.rBoxes), pFndLine(pFL), pFndBox(rPara.pFndBox) {}
};

sal_Bool _FndBoxCopyCol( const SwTableBox*& rpBox, void* pPara );
SW_DLLPUBLIC sal_Bool _FndLineCopyCol( const SwTableLine*& rpLine, void* pPara );


#endif  //  _TBLSEL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
