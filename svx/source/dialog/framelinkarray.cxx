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

#include <svx/framelinkarray.hxx>

#include <math.h>
#include <vector>
#include <algorithm>
#include <vcl/outdev.hxx>
#include <drawinglayer/primitive2d/borderlineprimitive2d.hxx>

namespace svx {
namespace frame {

/// single exclusive friend method to change mpUsingCell at style when style
/// is set at Cell, see friend definition for more info
void exclusiveSetUsigCellAtStyle(Style& rStyle, const Cell* pCell) { rStyle.mpUsingCell = pCell; }

class Cell
{
private:
    Style               maLeft;
    Style               maRight;
    Style               maTop;
    Style               maBottom;
    Style               maTLBR;
    Style               maBLTR;

public:
    long                mnAddLeft;
    long                mnAddRight;
    long                mnAddTop;
    long                mnAddBottom;

    SvxRotateMode       meRotMode;
    double              mfOrientation;

    bool                mbMergeOrig;
    bool                mbOverlapX;
    bool                mbOverlapY;

public:
    explicit            Cell();

    void SetStyleLeft(const Style& rStyle) { maLeft = rStyle; exclusiveSetUsigCellAtStyle(maLeft, this); }
    void SetStyleRight(const Style& rStyle) { maRight = rStyle; exclusiveSetUsigCellAtStyle(maRight, this); }
    void SetStyleTop(const Style& rStyle) { maTop = rStyle; exclusiveSetUsigCellAtStyle(maTop, this); }
    void SetStyleBottom(const Style& rStyle) { maBottom = rStyle; exclusiveSetUsigCellAtStyle(maBottom, this); }
    void SetStyleTLBR(const Style& rStyle) { maTLBR = rStyle; exclusiveSetUsigCellAtStyle(maTLBR, this); }
    void SetStyleBLTR(const Style& rStyle) { maBLTR = rStyle; exclusiveSetUsigCellAtStyle(maBLTR, this); }

    const Style& GetStyleLeft() const { return maLeft; }
    const Style& GetStyleRight() const { return maRight; }
    const Style& GetStyleTop() const { return maTop; }
    const Style& GetStyleBottom() const { return maBottom; }
    const Style& GetStyleTLBR() const { return maTLBR; }
    const Style& GetStyleBLTR() const { return maBLTR; }

    bool                IsMerged() const { return mbMergeOrig || mbOverlapX || mbOverlapY; }
    bool                IsRotated() const { return mfOrientation != 0.0; }

    void                MirrorSelfX();
};

typedef std::vector< long >     LongVec;
typedef std::vector< Cell >     CellVec;

Cell::Cell() :
    mnAddLeft( 0 ),
    mnAddRight( 0 ),
    mnAddTop( 0 ),
    mnAddBottom( 0 ),
    meRotMode(SvxRotateMode::SVX_ROTATE_MODE_STANDARD ),
    mfOrientation( 0.0 ),
    mbMergeOrig( false ),
    mbOverlapX( false ),
    mbOverlapY( false )
{
}

void Cell::MirrorSelfX()
{
    std::swap( maLeft, maRight );
    std::swap( mnAddLeft, mnAddRight );
    maLeft.MirrorSelf();
    maRight.MirrorSelf();
    mfOrientation = -mfOrientation;
}


void lclRecalcCoordVec( LongVec& rCoords, const LongVec& rSizes )
{
    DBG_ASSERT( rCoords.size() == rSizes.size() + 1, "lclRecalcCoordVec - inconsistent vectors" );
    LongVec::iterator aCIt = rCoords.begin();
    LongVec::const_iterator aSIt = rSizes.begin(), aSEnd = rSizes.end();
    for( ; aSIt != aSEnd; ++aCIt, ++aSIt )
        *(aCIt + 1) = *aCIt + *aSIt;
}

void lclSetMergedRange( CellVec& rCells, size_t nWidth, size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow )
{
    for( size_t nCol = nFirstCol; nCol <= nLastCol; ++nCol )
    {
        for( size_t nRow = nFirstRow; nRow <= nLastRow; ++nRow )
        {
            Cell& rCell = rCells[ nRow * nWidth + nCol ];
            rCell.mbMergeOrig = false;
            rCell.mbOverlapX = nCol > nFirstCol;
            rCell.mbOverlapY = nRow > nFirstRow;
        }
    }
    rCells[ nFirstRow * nWidth + nFirstCol ].mbMergeOrig = true;
}


static const Style OBJ_STYLE_NONE;
static const Cell OBJ_CELL_NONE;

struct ArrayImpl
{
    CellVec             maCells;
    LongVec             maWidths;
    LongVec             maHeights;
    mutable LongVec     maXCoords;
    mutable LongVec     maYCoords;
    size_t              mnWidth;
    size_t              mnHeight;
    size_t              mnFirstClipCol;
    size_t              mnFirstClipRow;
    size_t              mnLastClipCol;
    size_t              mnLastClipRow;
    mutable bool        mbXCoordsDirty;
    mutable bool        mbYCoordsDirty;
    bool                mbMayHaveCellRotation;

    explicit            ArrayImpl( size_t nWidth, size_t nHeight );

    bool         IsValidPos( size_t nCol, size_t nRow ) const
                            { return (nCol < mnWidth) && (nRow < mnHeight); }
    size_t       GetIndex( size_t nCol, size_t nRow ) const
                            { return nRow * mnWidth + nCol; }

    const Cell&         GetCell( size_t nCol, size_t nRow ) const;
    Cell&               GetCellAcc( size_t nCol, size_t nRow );

    size_t              GetMergedFirstCol( size_t nCol, size_t nRow ) const;
    size_t              GetMergedFirstRow( size_t nCol, size_t nRow ) const;
    size_t              GetMergedLastCol( size_t nCol, size_t nRow ) const;
    size_t              GetMergedLastRow( size_t nCol, size_t nRow ) const;

    const Cell&         GetMergedOriginCell( size_t nCol, size_t nRow ) const;

    bool                IsMergedOverlappedLeft( size_t nCol, size_t nRow ) const;
    bool                IsMergedOverlappedRight( size_t nCol, size_t nRow ) const;
    bool                IsMergedOverlappedTop( size_t nCol, size_t nRow ) const;
    bool                IsMergedOverlappedBottom( size_t nCol, size_t nRow ) const;

    bool                IsInClipRange( size_t nCol, size_t nRow ) const;
    bool                IsColInClipRange( size_t nCol ) const;
    bool                IsRowInClipRange( size_t nRow ) const;

    size_t       GetMirrorCol( size_t nCol ) const { return mnWidth - nCol - 1; }

    long                GetColPosition( size_t nCol ) const;
    long                GetRowPosition( size_t nRow ) const;

    long                GetColWidth( size_t nFirstCol, size_t nLastCol ) const;
    long                GetRowHeight( size_t nFirstRow, size_t nLastRow ) const;

    double              GetHorDiagAngle( size_t nCol, size_t nRow ) const;
    double              GetVerDiagAngle( size_t nCol, size_t nRow ) const;

    bool                HasCellRotation() const;
};

ArrayImpl::ArrayImpl( size_t nWidth, size_t nHeight ) :
    mnWidth( nWidth ),
    mnHeight( nHeight ),
    mnFirstClipCol( 0 ),
    mnFirstClipRow( 0 ),
    mnLastClipCol( nWidth - 1 ),
    mnLastClipRow( nHeight - 1 ),
    mbXCoordsDirty( false ),
    mbYCoordsDirty( false ),
    mbMayHaveCellRotation( false )
{
    // default-construct all vectors
    maCells.resize( mnWidth * mnHeight );
    maWidths.resize( mnWidth, 0L );
    maHeights.resize( mnHeight, 0L );
    maXCoords.resize( mnWidth + 1, 0L );
    maYCoords.resize( mnHeight + 1, 0L );
}

const Cell& ArrayImpl::GetCell( size_t nCol, size_t nRow ) const
{
    return IsValidPos( nCol, nRow ) ? maCells[ GetIndex( nCol, nRow ) ] : OBJ_CELL_NONE;
}

Cell& ArrayImpl::GetCellAcc( size_t nCol, size_t nRow )
{
    static Cell aDummy;
    return IsValidPos( nCol, nRow ) ? maCells[ GetIndex( nCol, nRow ) ] : aDummy;
}

size_t ArrayImpl::GetMergedFirstCol( size_t nCol, size_t nRow ) const
{
    size_t nFirstCol = nCol;
    while( (nFirstCol > 0) && GetCell( nFirstCol, nRow ).mbOverlapX ) --nFirstCol;
    return nFirstCol;
}

size_t ArrayImpl::GetMergedFirstRow( size_t nCol, size_t nRow ) const
{
    size_t nFirstRow = nRow;
    while( (nFirstRow > 0) && GetCell( nCol, nFirstRow ).mbOverlapY ) --nFirstRow;
    return nFirstRow;
}

size_t ArrayImpl::GetMergedLastCol( size_t nCol, size_t nRow ) const
{
    size_t nLastCol = nCol + 1;
    while( (nLastCol < mnWidth) && GetCell( nLastCol, nRow ).mbOverlapX ) ++nLastCol;
    return nLastCol - 1;
}

size_t ArrayImpl::GetMergedLastRow( size_t nCol, size_t nRow ) const
{
    size_t nLastRow = nRow + 1;
    while( (nLastRow < mnHeight) && GetCell( nCol, nLastRow ).mbOverlapY ) ++nLastRow;
    return nLastRow - 1;
}

const Cell& ArrayImpl::GetMergedOriginCell( size_t nCol, size_t nRow ) const
{
    return GetCell( GetMergedFirstCol( nCol, nRow ), GetMergedFirstRow( nCol, nRow ) );
}

bool ArrayImpl::IsMergedOverlappedLeft( size_t nCol, size_t nRow ) const
{
    const Cell& rCell = GetCell( nCol, nRow );
    return rCell.mbOverlapX || (rCell.mnAddLeft > 0);
}

bool ArrayImpl::IsMergedOverlappedRight( size_t nCol, size_t nRow ) const
{
    return GetCell( nCol + 1, nRow ).mbOverlapX || (GetCell( nCol, nRow ).mnAddRight > 0);
}

bool ArrayImpl::IsMergedOverlappedTop( size_t nCol, size_t nRow ) const
{
    const Cell& rCell = GetCell( nCol, nRow );
    return rCell.mbOverlapY || (rCell.mnAddTop > 0);
}

bool ArrayImpl::IsMergedOverlappedBottom( size_t nCol, size_t nRow ) const
{
    return GetCell( nCol, nRow + 1 ).mbOverlapY || (GetCell( nCol, nRow ).mnAddBottom > 0);
}

bool ArrayImpl::IsColInClipRange( size_t nCol ) const
{
    return (mnFirstClipCol <= nCol) && (nCol <= mnLastClipCol);
}

bool ArrayImpl::IsRowInClipRange( size_t nRow ) const
{
    return (mnFirstClipRow <= nRow) && (nRow <= mnLastClipRow);
}

bool ArrayImpl::IsInClipRange( size_t nCol, size_t nRow ) const
{
    return IsColInClipRange( nCol ) && IsRowInClipRange( nRow );
}

long ArrayImpl::GetColPosition( size_t nCol ) const
{
    if( mbXCoordsDirty )
    {
        lclRecalcCoordVec( maXCoords, maWidths );
        mbXCoordsDirty = false;
    }
    return maXCoords[ nCol ];
}

long ArrayImpl::GetRowPosition( size_t nRow ) const
{
    if( mbYCoordsDirty )
    {
        lclRecalcCoordVec( maYCoords, maHeights );
        mbYCoordsDirty = false;
    }
    return maYCoords[ nRow ];
}

long ArrayImpl::GetColWidth( size_t nFirstCol, size_t nLastCol ) const
{
    return GetColPosition( nLastCol + 1 ) - GetColPosition( nFirstCol );
}

long ArrayImpl::GetRowHeight( size_t nFirstRow, size_t nLastRow ) const
{
    return GetRowPosition( nLastRow + 1 ) - GetRowPosition( nFirstRow );
}

double ArrayImpl::GetHorDiagAngle( size_t nCol, size_t nRow ) const
{
    double fAngle = 0.0;
    if( IsValidPos( nCol, nRow ) )
    {
        if( !GetCell( nCol, nRow ).IsMerged() )
        {
            fAngle = frame::GetHorDiagAngle( maWidths[ nCol ] + 1, maHeights[ nRow ] + 1 );
        }
        else
        {
            // return correct angle for each cell in the merged range
            size_t nFirstCol = GetMergedFirstCol( nCol, nRow );
            size_t nFirstRow = GetMergedFirstRow( nCol, nRow );
            const Cell& rCell = GetCell( nFirstCol, nFirstRow );
            long nWidth = GetColWidth( nFirstCol, GetMergedLastCol( nCol, nRow ) ) + rCell.mnAddLeft + rCell.mnAddRight;
            long nHeight = GetRowHeight( nFirstRow, GetMergedLastRow( nCol, nRow ) ) + rCell.mnAddTop + rCell.mnAddBottom;
            fAngle = frame::GetHorDiagAngle( nWidth + 1, nHeight + 1 );
        }
    }
    return fAngle;
}

double ArrayImpl::GetVerDiagAngle( size_t nCol, size_t nRow ) const
{
    double fAngle = GetHorDiagAngle( nCol, nRow );
    return (fAngle > 0.0) ? (F_PI2 - fAngle) : 0.0;
}

bool ArrayImpl::HasCellRotation() const
{
    // check cell array
    for (const auto& aCell : maCells)
    {
        if (aCell.IsRotated())
        {
            return true;
        }
    }

    return false;
}

class MergedCellIterator
{
public:
    explicit            MergedCellIterator( const Array& rArray, size_t nCol, size_t nRow );

    bool         Is() const { return (mnCol <= mnLastCol) && (mnRow <= mnLastRow); }
    size_t       Col() const { return mnCol; }
    size_t       Row() const { return mnRow; }

    MergedCellIterator& operator++();

private:
    size_t              mnFirstCol;
    size_t              mnFirstRow;
    size_t              mnLastCol;
    size_t              mnLastRow;
    size_t              mnCol;
    size_t              mnRow;
};


MergedCellIterator::MergedCellIterator( const Array& rArray, size_t nCol, size_t nRow )
{
    DBG_ASSERT( rArray.IsMerged( nCol, nRow ), "svx::frame::MergedCellIterator::MergedCellIterator - not in merged range" );
    rArray.GetMergedRange( mnFirstCol, mnFirstRow, mnLastCol, mnLastRow, nCol, nRow );
    mnCol = mnFirstCol;
    mnRow = mnFirstRow;
}

MergedCellIterator& MergedCellIterator::operator++()
{
    DBG_ASSERT( Is(), "svx::frame::MergedCellIterator::operator++() - already invalid" );
    if( ++mnCol > mnLastCol )
    {
        mnCol = mnFirstCol;
        ++mnRow;
    }
    return *this;
}


#define DBG_FRAME_CHECK( cond, funcname, error )        DBG_ASSERT( cond, "svx::frame::Array::" funcname " - " error )
#define DBG_FRAME_CHECK_COL( col, funcname )            DBG_FRAME_CHECK( (col) < GetColCount(), funcname, "invalid column index" )
#define DBG_FRAME_CHECK_ROW( row, funcname )            DBG_FRAME_CHECK( (row) < GetRowCount(), funcname, "invalid row index" )
#define DBG_FRAME_CHECK_COLROW( col, row, funcname )    DBG_FRAME_CHECK( ((col) < GetColCount()) && ((row) < GetRowCount()), funcname, "invalid cell index" )
#define DBG_FRAME_CHECK_COL_1( col, funcname )          DBG_FRAME_CHECK( (col) <= GetColCount(), funcname, "invalid column index" )
#define DBG_FRAME_CHECK_ROW_1( row, funcname )          DBG_FRAME_CHECK( (row) <= GetRowCount(), funcname, "invalid row index" )


#define CELL( col, row )        mxImpl->GetCell( col, row )
#define CELLACC( col, row )     mxImpl->GetCellAcc( col, row )
#define ORIGCELL( col, row )    mxImpl->GetMergedOriginCell( col, row )


Array::Array()
{
    Initialize( 0, 0 );
}

Array::~Array()
{
}

// array size and column/row indexes
void Array::Initialize( size_t nWidth, size_t nHeight )
{
    mxImpl.reset( new ArrayImpl( nWidth, nHeight ) );
}

size_t Array::GetColCount() const
{
    return mxImpl->mnWidth;
}

size_t Array::GetRowCount() const
{
    return mxImpl->mnHeight;
}

size_t Array::GetCellCount() const
{
    return mxImpl->maCells.size();
}

size_t Array::GetCellIndex( size_t nCol, size_t nRow, bool bRTL ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetCellIndex" );
    if (bRTL)
        nCol = mxImpl->GetMirrorCol(nCol);
    return mxImpl->GetIndex( nCol, nRow );
}

// cell border styles
void Array::SetCellStyleLeft( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleLeft" );
    CELLACC( nCol, nRow ).SetStyleLeft(rStyle);
}

void Array::SetCellStyleRight( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleRight" );
    CELLACC( nCol, nRow ).SetStyleRight(rStyle);
}

void Array::SetCellStyleTop( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleTop" );
    CELLACC( nCol, nRow ).SetStyleTop(rStyle);
}

void Array::SetCellStyleBottom( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleBottom" );
    CELLACC( nCol, nRow ).SetStyleBottom(rStyle);
}

void Array::SetCellStyleTLBR( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleTLBR" );
    CELLACC( nCol, nRow ).SetStyleTLBR(rStyle);
}

void Array::SetCellStyleBLTR( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleBLTR" );
    CELLACC( nCol, nRow ).SetStyleBLTR(rStyle);
}

void Array::SetCellStyleDiag( size_t nCol, size_t nRow, const Style& rTLBR, const Style& rBLTR )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleDiag" );
    Cell& rCell = CELLACC( nCol, nRow );
    rCell.SetStyleTLBR(rTLBR);
    rCell.SetStyleBLTR(rBLTR);
}

void Array::SetColumnStyleLeft( size_t nCol, const Style& rStyle )
{
    DBG_FRAME_CHECK_COL( nCol, "SetColumnStyleLeft" );
    for( size_t nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
        SetCellStyleLeft( nCol, nRow, rStyle );
}

void Array::SetColumnStyleRight( size_t nCol, const Style& rStyle )
{
    DBG_FRAME_CHECK_COL( nCol, "SetColumnStyleRight" );
    for( size_t nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
        SetCellStyleRight( nCol, nRow, rStyle );
}

void Array::SetRowStyleTop( size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_ROW( nRow, "SetRowStyleTop" );
    for( size_t nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        SetCellStyleTop( nCol, nRow, rStyle );
}

void Array::SetRowStyleBottom( size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_ROW( nRow, "SetRowStyleBottom" );
    for( size_t nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        SetCellStyleBottom( nCol, nRow, rStyle );
}

void Array::SetCellRotation(size_t nCol, size_t nRow, SvxRotateMode eRotMode, double fOrientation)
{
    DBG_FRAME_CHECK_COLROW(nCol, nRow, "SetCellRotation");
    Cell& rTarget = CELLACC(nCol, nRow);
    rTarget.meRotMode = eRotMode;
    rTarget.mfOrientation = fOrientation;

    if (!mxImpl->mbMayHaveCellRotation)
    {
        // activate once when a cell gets actually rotated to allow fast
        // answering HasCellRotation() calls
        mxImpl->mbMayHaveCellRotation = rTarget.IsRotated();
    }
}

bool Array::HasCellRotation() const
{
    if (!mxImpl->mbMayHaveCellRotation)
    {
        // never set, no need to check
        return false;
    }

    return mxImpl->HasCellRotation();
}

const Style& Array::GetCellStyleLeft( size_t nCol, size_t nRow ) const
{
    // outside clipping rows or overlapped in merged cells: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) || mxImpl->IsMergedOverlappedLeft( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // left clipping border: always own left style
    if( nCol == mxImpl->mnFirstClipCol )
        return ORIGCELL( nCol, nRow ).GetStyleLeft();
    // right clipping border: always right style of left neighbor cell
    if( nCol == mxImpl->mnLastClipCol + 1 )
        return ORIGCELL( nCol - 1, nRow ).GetStyleRight();
    // outside clipping columns: invisible
    if( !mxImpl->IsColInClipRange( nCol ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own left style and right style of left neighbor cell
    return std::max( ORIGCELL( nCol, nRow ).GetStyleLeft(), ORIGCELL( nCol - 1, nRow ).GetStyleRight() );
}

const Style& Array::GetCellStyleRight( size_t nCol, size_t nRow ) const
{
    // outside clipping rows or overlapped in merged cells: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) || mxImpl->IsMergedOverlappedRight( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // left clipping border: always left style of right neighbor cell
    if( nCol + 1 == mxImpl->mnFirstClipCol )
        return ORIGCELL( nCol + 1, nRow ).GetStyleLeft();
    // right clipping border: always own right style
    if( nCol == mxImpl->mnLastClipCol )
        return ORIGCELL( nCol, nRow ).GetStyleRight();
    // outside clipping columns: invisible
    if( !mxImpl->IsColInClipRange( nCol ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own right style and left style of right neighbor cell
    return std::max( ORIGCELL( nCol, nRow ).GetStyleRight(), ORIGCELL( nCol + 1, nRow ).GetStyleLeft() );
}

const Style& Array::GetCellStyleTop( size_t nCol, size_t nRow ) const
{
    // outside clipping columns or overlapped in merged cells: invisible
    if( !mxImpl->IsColInClipRange( nCol ) || mxImpl->IsMergedOverlappedTop( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // top clipping border: always own top style
    if( nRow == mxImpl->mnFirstClipRow )
        return ORIGCELL( nCol, nRow ).GetStyleTop();
    // bottom clipping border: always bottom style of top neighbor cell
    if( nRow == mxImpl->mnLastClipRow + 1 )
        return ORIGCELL( nCol, nRow - 1 ).GetStyleBottom();
    // outside clipping rows: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own top style and bottom style of top neighbor cell
    return std::max( ORIGCELL( nCol, nRow ).GetStyleTop(), ORIGCELL( nCol, nRow - 1 ).GetStyleBottom() );
}

const Style& Array::GetCellStyleBottom( size_t nCol, size_t nRow ) const
{
    // outside clipping columns or overlapped in merged cells: invisible
    if( !mxImpl->IsColInClipRange( nCol ) || mxImpl->IsMergedOverlappedBottom( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // top clipping border: always top style of bottom neighbor cell
    if( nRow + 1 == mxImpl->mnFirstClipRow )
        return ORIGCELL( nCol, nRow + 1 ).GetStyleTop();
    // bottom clipping border: always own bottom style
    if( nRow == mxImpl->mnLastClipRow )
        return ORIGCELL( nCol, nRow ).GetStyleBottom();
    // outside clipping rows: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own bottom style and top style of bottom neighbor cell
    return std::max( ORIGCELL( nCol, nRow ).GetStyleBottom(), ORIGCELL( nCol, nRow + 1 ).GetStyleTop() );
}

const Style& Array::GetCellStyleTLBR( size_t nCol, size_t nRow ) const
{
    return CELL( nCol, nRow ).GetStyleTLBR();
}

const Style& Array::GetCellStyleBLTR( size_t nCol, size_t nRow ) const
{
    return CELL( nCol, nRow ).GetStyleBLTR();
}

const Style& Array::GetCellStyleTL( size_t nCol, size_t nRow ) const
{
    // not in clipping range: always invisible
    if( !mxImpl->IsInClipRange( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // return style only for top-left cell
    size_t nFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
    size_t nFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
    return ((nCol == nFirstCol) && (nRow == nFirstRow)) ?
        CELL( nFirstCol, nFirstRow ).GetStyleTLBR() : OBJ_STYLE_NONE;
}

const Style& Array::GetCellStyleBR( size_t nCol, size_t nRow ) const
{
    // not in clipping range: always invisible
    if( !mxImpl->IsInClipRange( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // return style only for bottom-right cell
    size_t nLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
    size_t nLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
    return ((nCol == nLastCol) && (nRow == nLastRow)) ?
        CELL( mxImpl->GetMergedFirstCol( nCol, nRow ), mxImpl->GetMergedFirstRow( nCol, nRow ) ).GetStyleTLBR() : OBJ_STYLE_NONE;
}

const Style& Array::GetCellStyleBL( size_t nCol, size_t nRow ) const
{
    // not in clipping range: always invisible
    if( !mxImpl->IsInClipRange( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // return style only for bottom-left cell
    size_t nFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
    size_t nLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
    return ((nCol == nFirstCol) && (nRow == nLastRow)) ?
        CELL( nFirstCol, mxImpl->GetMergedFirstRow( nCol, nRow ) ).GetStyleBLTR() : OBJ_STYLE_NONE;
}

const Style& Array::GetCellStyleTR( size_t nCol, size_t nRow ) const
{
    // not in clipping range: always invisible
    if( !mxImpl->IsInClipRange( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // return style only for top-right cell
    size_t nFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
    size_t nLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
    return ((nCol == nLastCol) && (nRow == nFirstRow)) ?
        CELL( mxImpl->GetMergedFirstCol( nCol, nRow ), nFirstRow ).GetStyleBLTR() : OBJ_STYLE_NONE;
}

// cell merging
void Array::SetMergedRange( size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow )
{
    DBG_FRAME_CHECK_COLROW( nFirstCol, nFirstRow, "SetMergedRange" );
    DBG_FRAME_CHECK_COLROW( nLastCol, nLastRow, "SetMergedRange" );
#if OSL_DEBUG_LEVEL >= 2
    {
        bool bFound = false;
        for( size_t nCurrCol = nFirstCol; !bFound && (nCurrCol <= nLastCol); ++nCurrCol )
            for( size_t nCurrRow = nFirstRow; !bFound && (nCurrRow <= nLastRow); ++nCurrRow )
                bFound = CELL( nCurrCol, nCurrRow ).IsMerged();
        DBG_FRAME_CHECK( !bFound, "SetMergedRange", "overlapping merged ranges" );
    }
#endif
    if( mxImpl->IsValidPos( nFirstCol, nFirstRow ) && mxImpl->IsValidPos( nLastCol, nLastRow ) )
        lclSetMergedRange( mxImpl->maCells, mxImpl->mnWidth, nFirstCol, nFirstRow, nLastCol, nLastRow );
}

void Array::SetAddMergedLeftSize( size_t nCol, size_t nRow, long nAddSize )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetAddMergedLeftSize" );
    DBG_FRAME_CHECK( mxImpl->GetMergedFirstCol( nCol, nRow ) == 0, "SetAddMergedLeftSize", "additional border inside array" );
    for( MergedCellIterator aIt( *this, nCol, nRow ); aIt.Is(); ++aIt )
        CELLACC( aIt.Col(), aIt.Row() ).mnAddLeft = nAddSize;
}

void Array::SetAddMergedRightSize( size_t nCol, size_t nRow, long nAddSize )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetAddMergedRightSize" );
    DBG_FRAME_CHECK( mxImpl->GetMergedLastCol( nCol, nRow ) + 1 == mxImpl->mnWidth, "SetAddMergedRightSize", "additional border inside array" );
    for( MergedCellIterator aIt( *this, nCol, nRow ); aIt.Is(); ++aIt )
        CELLACC( aIt.Col(), aIt.Row() ).mnAddRight = nAddSize;
}

void Array::SetAddMergedTopSize( size_t nCol, size_t nRow, long nAddSize )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetAddMergedTopSize" );
    DBG_FRAME_CHECK( mxImpl->GetMergedFirstRow( nCol, nRow ) == 0, "SetAddMergedTopSize", "additional border inside array" );
    for( MergedCellIterator aIt( *this, nCol, nRow ); aIt.Is(); ++aIt )
        CELLACC( aIt.Col(), aIt.Row() ).mnAddTop = nAddSize;
}

void Array::SetAddMergedBottomSize( size_t nCol, size_t nRow, long nAddSize )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetAddMergedBottomSize" );
    DBG_FRAME_CHECK( mxImpl->GetMergedLastRow( nCol, nRow ) + 1 == mxImpl->mnHeight, "SetAddMergedBottomSize", "additional border inside array" );
    for( MergedCellIterator aIt( *this, nCol, nRow ); aIt.Is(); ++aIt )
        CELLACC( aIt.Col(), aIt.Row() ).mnAddBottom = nAddSize;
}

bool Array::IsMerged( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "IsMerged" );
    return CELL( nCol, nRow ).IsMerged();
}

void Array::GetMergedOrigin( size_t& rnFirstCol, size_t& rnFirstRow, size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetMergedOrigin" );
    rnFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
    rnFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
}

void Array::GetMergedRange( size_t& rnFirstCol, size_t& rnFirstRow,
        size_t& rnLastCol, size_t& rnLastRow, size_t nCol, size_t nRow ) const
{
    GetMergedOrigin( rnFirstCol, rnFirstRow, nCol, nRow );
    rnLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
    rnLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
}

// clipping
void Array::SetClipRange( size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow )
{
    DBG_FRAME_CHECK_COLROW( nFirstCol, nFirstRow, "SetClipRange" );
    DBG_FRAME_CHECK_COLROW( nLastCol, nLastRow, "SetClipRange" );
    mxImpl->mnFirstClipCol = nFirstCol;
    mxImpl->mnFirstClipRow = nFirstRow;
    mxImpl->mnLastClipCol = nLastCol;
    mxImpl->mnLastClipRow = nLastRow;
}

// cell coordinates
void Array::SetXOffset( long nXOffset )
{
    mxImpl->maXCoords[ 0 ] = nXOffset;
    mxImpl->mbXCoordsDirty = true;
}

void Array::SetYOffset( long nYOffset )
{
    mxImpl->maYCoords[ 0 ] = nYOffset;
    mxImpl->mbYCoordsDirty = true;
}

void Array::SetColWidth( size_t nCol, long nWidth )
{
    DBG_FRAME_CHECK_COL( nCol, "SetColWidth" );
    mxImpl->maWidths[ nCol ] = nWidth;
    mxImpl->mbXCoordsDirty = true;
}

void Array::SetRowHeight( size_t nRow, long nHeight )
{
    DBG_FRAME_CHECK_ROW( nRow, "SetRowHeight" );
    mxImpl->maHeights[ nRow ] = nHeight;
    mxImpl->mbYCoordsDirty = true;
}

void Array::SetAllColWidths( long nWidth )
{
    std::fill( mxImpl->maWidths.begin(), mxImpl->maWidths.end(), nWidth );
    mxImpl->mbXCoordsDirty = true;
}

void Array::SetAllRowHeights( long nHeight )
{
    std::fill( mxImpl->maHeights.begin(), mxImpl->maHeights.end(), nHeight );
    mxImpl->mbYCoordsDirty = true;
}

long Array::GetColPosition( size_t nCol ) const
{
    DBG_FRAME_CHECK_COL_1( nCol, "GetColPosition" );
    return mxImpl->GetColPosition( nCol );
}

long Array::GetRowPosition( size_t nRow ) const
{
    DBG_FRAME_CHECK_ROW_1( nRow, "GetRowPosition" );
    return mxImpl->GetRowPosition( nRow );
}

long Array::GetColWidth( size_t nFirstCol, size_t nLastCol ) const
{
    DBG_FRAME_CHECK_COL( nFirstCol, "GetColWidth" );
    DBG_FRAME_CHECK_COL( nLastCol, "GetColWidth" );
    return GetColPosition( nLastCol + 1 ) - GetColPosition( nFirstCol );
}

long Array::GetRowHeight( size_t nFirstRow, size_t nLastRow ) const
{
    DBG_FRAME_CHECK_ROW( nFirstRow, "GetRowHeight" );
    DBG_FRAME_CHECK_ROW( nLastRow, "GetRowHeight" );
    return GetRowPosition( nLastRow + 1 ) - GetRowPosition( nFirstRow );
}

long Array::GetWidth() const
{
    return GetColPosition( mxImpl->mnWidth ) - GetColPosition( 0 );
}

long Array::GetHeight() const
{
    return GetRowPosition( mxImpl->mnHeight ) - GetRowPosition( 0 );
}

Point Array::GetCellPosition( size_t nCol, size_t nRow ) const
{
    size_t nFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
    size_t nFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
    return Point( GetColPosition( nFirstCol ), GetRowPosition( nFirstRow ) );
}

Size Array::GetCellSize( size_t nCol, size_t nRow ) const
{
    size_t nFirstCol =  mxImpl->GetMergedFirstCol( nCol, nRow );
    size_t nFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
    size_t nLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
    size_t nLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
    return Size( GetColWidth( nFirstCol, nLastCol ) + 1, GetRowHeight( nFirstRow, nLastRow ) + 1 );
}

tools::Rectangle Array::GetCellRect( size_t nCol, size_t nRow ) const
{
    tools::Rectangle aRect( GetCellPosition( nCol, nRow ), GetCellSize( nCol, nRow ) );

    // adjust rectangle for partly visible merged cells
    const Cell& rCell = CELL( nCol, nRow );
    if( rCell.IsMerged() )
    {
        aRect.Left() -= rCell.mnAddLeft;
        aRect.Right() += rCell.mnAddRight;
        aRect.Top() -= rCell.mnAddTop;
        aRect.Bottom() += rCell.mnAddBottom;
    }
    return aRect;
}

// diagonal frame borders
double Array::GetHorDiagAngle( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetHorDiagAngle" );
    return mxImpl->GetHorDiagAngle( nCol, nRow );
}

double Array::GetVerDiagAngle( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetVerDiagAngle" );
    return mxImpl->GetVerDiagAngle( nCol, nRow );
}

// mirroring
void Array::MirrorSelfX()
{
    CellVec aNewCells;
    aNewCells.reserve( GetCellCount() );

    size_t nCol, nRow;
    for( nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
    {
        for( nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        {
            aNewCells.push_back( CELL( mxImpl->GetMirrorCol( nCol ), nRow ) );
            aNewCells.back().MirrorSelfX();
        }
    }
    for( nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
    {
        for( nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        {
            if( CELL( nCol, nRow ).mbMergeOrig )
            {
                size_t nLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
                size_t nLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
                lclSetMergedRange( aNewCells, mxImpl->mnWidth,
                    mxImpl->GetMirrorCol( nLastCol ), nRow,
                    mxImpl->GetMirrorCol( nCol ), nLastRow );
            }
        }
    }
    mxImpl->maCells.swap( aNewCells );

    std::reverse( mxImpl->maWidths.begin(), mxImpl->maWidths.end() );
    mxImpl->mbXCoordsDirty = true;
}

// drawing

void CreateCoordinateSystemForCell(
    const basegfx::B2DRange& rRange,
    const Cell& rCell,
    basegfx::B2DPoint& rOrigin,
    basegfx::B2DVector& rX,
    basegfx::B2DVector& rY)
{
    // fill in defaults
    rOrigin = rRange.getMinimum();
    rX = basegfx::B2DVector(rRange.getWidth(), 0.0);
    rY = basegfx::B2DVector(0.0, rRange.getHeight());

    if (rCell.IsRotated() && SvxRotateMode::SVX_ROTATE_MODE_STANDARD != rCell.meRotMode)
    {
        // when rotated, adapt values. Get Skew (cos/sin == 1/tan)
        const double fSkew(rRange.getHeight() * (cos(rCell.mfOrientation) / sin(rCell.mfOrientation)));

        switch (rCell.meRotMode)
        {
        case SvxRotateMode::SVX_ROTATE_MODE_TOP:
            // shear Y-Axis
            rY.setX(-fSkew);
            break;
        case SvxRotateMode::SVX_ROTATE_MODE_CENTER:
            // shear origin half, Y full
            rOrigin.setX(rOrigin.getX() + (fSkew * 0.5));
            rY.setX(-fSkew);
            break;
        case SvxRotateMode::SVX_ROTATE_MODE_BOTTOM:
            // shear origin full, Y full
            rOrigin.setX(rOrigin.getX() + fSkew);
            rY.setX(-fSkew);
            break;
        default: // SvxRotateMode::SVX_ROTATE_MODE_STANDARD, already excluded above
            break;
        }
    }
}

void Array::DrawRange( drawinglayer::processor2d::BaseProcessor2D& rProcessor,
        size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow,
        const Color* pForceColor ) const
{
    DBG_FRAME_CHECK_COLROW( nFirstCol, nFirstRow, "DrawRange" );
    DBG_FRAME_CHECK_COLROW( nLastCol, nLastRow, "DrawRange" );

    size_t nCol, nRow;

    // *** diagonal frame borders ***
    for (nRow = nFirstRow; nRow <= nLastRow; ++nRow)
    {
        for (nCol = nFirstCol; nCol <= nLastCol; ++nCol)
        {
            const Cell& rCell = CELL(nCol, nRow);
            bool bOverlapX = rCell.mbOverlapX;
            bool bOverlapY = rCell.mbOverlapY;
            bool bFirstCol = nCol == nFirstCol;
            bool bFirstRow = nRow == nFirstRow;

            if ((!bOverlapX && !bOverlapY) || (bFirstCol && bFirstRow) || (!bOverlapY && bFirstCol) || (!bOverlapX && bFirstRow))
            {
                const tools::Rectangle aRect(GetCellRect(nCol, nRow));

                if ((aRect.GetWidth() > 1) && (aRect.GetHeight() > 1))
                {
                    size_t _nFirstCol = mxImpl->GetMergedFirstCol(nCol, nRow);
                    size_t _nFirstRow = mxImpl->GetMergedFirstRow(nCol, nRow);
                    size_t _nLastCol = mxImpl->GetMergedLastCol(nCol, nRow);
                    size_t _nLastRow = mxImpl->GetMergedLastRow(nCol, nRow);
                    const Style& rTLBR = GetCellStyleTLBR(_nFirstCol, _nFirstRow);
                    const Style& rBLTR = GetCellStyleBLTR(_nFirstCol, _nFirstRow);

                    if (rTLBR.GetWidth() || rBLTR.GetWidth())
                    {
                        drawinglayer::primitive2d::Primitive2DContainer aSequence;
                        const basegfx::B2DRange aRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());
                        basegfx::B2DPoint aOrigin;
                        basegfx::B2DVector aX;
                        basegfx::B2DVector aY;

                        CreateCoordinateSystemForCell(aRange, rCell, aOrigin, aX, aY);

                        CreateDiagFrameBorderPrimitives(
                            aSequence,
                            aOrigin,
                            aX,
                            aY,
                            rTLBR,
                            rBLTR,
                            GetCellStyleLeft(_nFirstCol, _nFirstRow),
                            GetCellStyleTop(_nFirstCol, _nFirstRow),
                            GetCellStyleRight(_nLastCol, _nLastRow),
                            GetCellStyleBottom(_nLastCol, _nLastRow),
                            GetCellStyleLeft(_nFirstCol, _nLastRow),
                            GetCellStyleBottom(_nFirstCol, _nLastRow),
                            GetCellStyleRight(_nLastCol, _nFirstRow),
                            GetCellStyleTop(_nLastCol, _nFirstRow),
                            pForceColor);

                        rProcessor.process(aSequence);
                    }
                }
            }
        }
    }

    // *** horizontal frame borders ***
    for( nRow = nFirstRow; nRow <= nLastRow + 1; ++nRow )
    {
        double fAngle = mxImpl->GetHorDiagAngle( nFirstCol, nRow );
        double fTAngle = mxImpl->GetHorDiagAngle( nFirstCol, nRow - 1 );

        // *Start*** variables store the data of the left end of the cached frame border
        basegfx::B2DPoint aStartPos( mxImpl->GetColPosition( nFirstCol ), mxImpl->GetRowPosition( nRow ) );
        const Style* pStart = &GetCellStyleTop( nFirstCol, nRow );
        DiagStyle aStartLFromTR( GetCellStyleBL( nFirstCol, nRow - 1 ), fTAngle );
        const Style* pStartLFromT = &GetCellStyleLeft( nFirstCol, nRow - 1 );
        const Style* pStartLFromL = &GetCellStyleTop( nFirstCol - 1, nRow );
        const Style* pStartLFromB = &GetCellStyleLeft( nFirstCol, nRow );
        DiagStyle aStartLFromBR( GetCellStyleTL( nFirstCol, nRow ), fAngle );

        // *End*** variables store the data of the right end of the cached frame border
        DiagStyle aEndRFromTL( GetCellStyleBR( nFirstCol, nRow - 1 ), fTAngle );
        const Style* pEndRFromT = &GetCellStyleRight( nFirstCol, nRow - 1 );
        const Style* pEndRFromR = &GetCellStyleTop( nFirstCol + 1, nRow );
        const Style* pEndRFromB = &GetCellStyleRight( nFirstCol, nRow );
        DiagStyle aEndRFromBL( GetCellStyleTR( nFirstCol, nRow ), fAngle );

        for( nCol = nFirstCol + 1; nCol <= nLastCol; ++nCol )
        {
            fAngle = mxImpl->GetHorDiagAngle( nCol, nRow );
            fTAngle = mxImpl->GetHorDiagAngle( nCol, nRow - 1 );

            const Style& rCurr = *pEndRFromR;

            DiagStyle aLFromTR( GetCellStyleBL( nCol, nRow - 1 ), fTAngle );
            const Style& rLFromT = *pEndRFromT;
            const Style& rLFromL = *pStart;
            const Style& rLFromB = *pEndRFromB;
            DiagStyle aLFromBR( GetCellStyleTL( nCol, nRow ), fAngle );

            DiagStyle aRFromTL( GetCellStyleBR( nCol, nRow - 1 ), fTAngle );
            const Style& rRFromT = GetCellStyleRight( nCol, nRow - 1 );
            const Style& rRFromR = GetCellStyleTop( nCol + 1, nRow );
            const Style& rRFromB = GetCellStyleRight( nCol, nRow );
            DiagStyle aRFromBL( GetCellStyleTR( nCol, nRow ), fAngle );

            // check if current frame border can be connected to cached frame border
            if( !CheckFrameBorderConnectable( *pStart, rCurr, aEndRFromTL, rLFromT, aLFromTR, aEndRFromBL, rLFromB, aLFromBR ) )
            {
                // draw previous frame border
                basegfx::B2DPoint aEndPos( mxImpl->GetColPosition( nCol ), aStartPos.getY() );

                if ((pStart->Prim() || pStart->Secn()) && (aStartPos.getX() <= aEndPos.getX()))
                {
                    // prepare defaults for borderline coordinate system
                    basegfx::B2DPoint aOrigin(aStartPos);
                    basegfx::B2DVector aX(aEndPos - aStartPos);
                    basegfx::B2DVector aY(0.0, 1.0);
                    const Cell* pCell = pStart->GetUsingCell();

                    if (pCell && pCell->IsRotated())
                    {
                        // To apply the shear, we need to get the cell range. We have the defining cell,
                        // but there is no call at it to directly get it's range. To get the correct one,
                        // we need to take care if the borderline is at top or bottom, so use pointer
                        // compare here to find out
                        const bool bUpper(&pCell->GetStyleTop() == pStart);
                        const tools::Rectangle aRect(GetCellRect(nCol - 1, bUpper ? nRow : nRow - 1));
                        const basegfx::B2DRange aRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());

                        // adapt to cell coordinate system, including shear
                        CreateCoordinateSystemForCell(aRange, *pCell, aOrigin, aX, aY);

                        if (!bUpper)
                        {
                            // for the lower edge we need to translate to get to the
                            // borderline coordinate system. For the upper one, all is
                            // okay already
                            aOrigin  += aY;
                        }

                        // borderline coordinate system uses normalized 2nd axis
                        aY.normalize();
                    }

                    drawinglayer::primitive2d::Primitive2DContainer aSequence;
                    CreateBorderPrimitives(
                        aSequence,
                        aOrigin,
                        aX,
                        aY,
                        *pStart,
                        aStartLFromTR,
                        *pStartLFromT,
                        *pStartLFromL,
                        *pStartLFromB,
                        aStartLFromBR,
                        aEndRFromTL,
                        *pEndRFromT,
                        *pEndRFromR,
                        *pEndRFromB,
                        aEndRFromBL,
                        pForceColor);
                    rProcessor.process(aSequence);
                }

                // re-init "*Start***" variables
                aStartPos = aEndPos;
                pStart = &rCurr;
                aStartLFromTR = aLFromTR;
                pStartLFromT = &rLFromT;
                pStartLFromL = &rLFromL;
                pStartLFromB = &rLFromB;
                aStartLFromBR = aLFromBR;
            }

            // store current styles in "*End***" variables
            aEndRFromTL = aRFromTL;
            pEndRFromT = &rRFromT;
            pEndRFromR = &rRFromR;
            pEndRFromB = &rRFromB;
            aEndRFromBL = aRFromBL;
        }

        // draw last frame border
        basegfx::B2DPoint aEndPos( mxImpl->GetColPosition( nCol ), aStartPos.getY() );
        if ((pStart->Prim() || pStart->Secn()) && (aStartPos.getX() <= aEndPos.getX()))
        {
            // for description of involved coordinate systems have a look at
            // the first CreateBorderPrimitives call above
            basegfx::B2DPoint aOrigin(aStartPos);
            basegfx::B2DVector aX(aEndPos - aStartPos);
            basegfx::B2DVector aY(0.0, 1.0);
            const Cell* pCell = pStart->GetUsingCell();

            if (pCell && pCell->IsRotated())
            {
                const bool bUpper(&pCell->GetStyleTop() == pStart);
                const tools::Rectangle aRect(GetCellRect(nCol - 1, bUpper ? nRow : nRow - 1));
                const basegfx::B2DRange aRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());

                CreateCoordinateSystemForCell(aRange, *pCell, aOrigin, aX, aY);

                if (!bUpper)
                {
                    aOrigin += aY;
                }

                aY.normalize();
            }

            drawinglayer::primitive2d::Primitive2DContainer aSequence;
            CreateBorderPrimitives(
                aSequence,
                aOrigin,
                aX,
                aY,
                *pStart,
                aStartLFromTR,
                *pStartLFromT,
                *pStartLFromL,
                *pStartLFromB,
                aStartLFromBR,
                aEndRFromTL,
                *pEndRFromT,
                *pEndRFromR,
                *pEndRFromB,
                aEndRFromBL,
                pForceColor);
            rProcessor.process(aSequence);
        }
    }

    // *** vertical frame borders ***
    for( nCol = nFirstCol; nCol <= nLastCol + 1; ++nCol )
    {
        double fAngle = mxImpl->GetVerDiagAngle( nCol, nFirstRow );
        double fLAngle = mxImpl->GetVerDiagAngle( nCol - 1, nFirstRow );

        // *Start*** variables store the data of the top end of the cached frame border
        basegfx::B2DPoint aStartPos( mxImpl->GetColPosition( nCol ), mxImpl->GetRowPosition( nFirstRow ) );
        const Style* pStart = &GetCellStyleLeft( nCol, nFirstRow );
        DiagStyle aStartTFromBL( GetCellStyleTR( nCol - 1, nFirstRow ), fLAngle );
        const Style* pStartTFromL = &GetCellStyleTop( nCol - 1, nFirstRow );
        const Style* pStartTFromT = &GetCellStyleLeft( nCol, nFirstRow - 1 );
        const Style* pStartTFromR = &GetCellStyleTop( nCol, nFirstRow );
        DiagStyle aStartTFromBR( GetCellStyleTL( nCol, nFirstRow ), fAngle );

        // *End*** variables store the data of the bottom end of the cached frame border
        DiagStyle aEndBFromTL( GetCellStyleBR( nCol - 1, nFirstRow ), fLAngle );
        const Style* pEndBFromL = &GetCellStyleBottom( nCol - 1, nFirstRow );
        const Style* pEndBFromB = &GetCellStyleLeft( nCol, nFirstRow + 1 );
        const Style* pEndBFromR = &GetCellStyleBottom( nCol, nFirstRow );
        DiagStyle aEndBFromTR( GetCellStyleBL( nCol, nFirstRow ), fAngle );

        for( nRow = nFirstRow + 1; nRow <= nLastRow; ++nRow )
        {
            fAngle = mxImpl->GetVerDiagAngle( nCol, nRow );
            fLAngle = mxImpl->GetVerDiagAngle( nCol - 1, nRow );

            const Style& rCurr = *pEndBFromB;

            DiagStyle aTFromBL( GetCellStyleTR( nCol - 1, nRow ), fLAngle );
            const Style& rTFromL = *pEndBFromL;
            const Style& rTFromT = *pStart;
            const Style& rTFromR = *pEndBFromR;
            DiagStyle aTFromBR( GetCellStyleTL( nCol, nRow ), fAngle );

            DiagStyle aBFromTL( GetCellStyleBR( nCol - 1, nRow ), fLAngle );
            const Style& rBFromL = GetCellStyleBottom( nCol - 1, nRow );
            const Style& rBFromB = GetCellStyleLeft( nCol, nRow + 1 );
            const Style& rBFromR = GetCellStyleBottom( nCol, nRow );
            DiagStyle aBFromTR( GetCellStyleBL( nCol, nRow ), fAngle );

            // check if current frame border can be connected to cached frame border
            if( !CheckFrameBorderConnectable( *pStart, rCurr,
                    aEndBFromTL, rTFromL, aTFromBL, aEndBFromTR, rTFromR, aTFromBR ) )
            {
                // draw previous frame border
                basegfx::B2DPoint aEndPos( aStartPos.getX(), mxImpl->GetRowPosition( nRow ) );
                if ((pStart->Prim() || pStart->Secn()) && (aStartPos.getY() <= aEndPos.getY()))
                {
                    // for description of involved coordinate systems have a look at
                    // the first CreateBorderPrimitives call above. Additionally adapt to vertical
                    basegfx::B2DPoint aOrigin(aStartPos);
                    basegfx::B2DVector aX(aEndPos - aStartPos);
                    basegfx::B2DVector aY(-1.0, 0.0);
                    const Cell* pCell = pStart->GetUsingCell();

                    if (pCell && pCell->IsRotated())
                    {
                        const bool bLeft(&pCell->GetStyleLeft() == pStart);
                        const tools::Rectangle aRect(GetCellRect(bLeft ? nCol : nCol - 1, nRow - 1));
                        const basegfx::B2DRange aRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());

                        CreateCoordinateSystemForCell(aRange, *pCell, aOrigin, aX, aY);

                        if (!bLeft)
                        {
                            aOrigin += aX;
                        }

                        // The *coordinate system* of the edge has to be given, which for vertical
                        // lines uses the Y-Vector as X-Axis and the X-Vector as Y-Axis, so swap both
                        // and mirror aX to keep the same orientation (should be (-1.0, 0.0) for
                        // horizontal lines anyways, this could be used as test here, checked in debug mode)
                        std::swap(aX, aY);
                        aY.normalize();
                        aY = -aY;
                    }

                    drawinglayer::primitive2d::Primitive2DContainer aSequence;
                    CreateBorderPrimitives(
                        // This replaces DrawVerFrameBorder which went from top to bottom. To be able to use
                        // the same method as for horizontal (CreateBorderPrimitives), the given borders
                        // have to be rearranged. Best is to look at the explanations of parameters in
                        // framelink.hxx and the former calls to DrawVerFrameBorder and it's parameters.
                        // In principle, the order of the five TFrom and BFrom has to be
                        // inverted to get the same orientation. Before, EndPos and StartPos were changed
                        // which avoids the reordering, but also leads to inverted line patters for vertical
                        // lines.
                        aSequence,
                        aOrigin,
                        aX,
                        aY,
                        *pStart,
                        aStartTFromBR,
                        *pStartTFromR,
                        *pStartTFromT,
                        *pStartTFromL,
                        aStartTFromBL,
                        aEndBFromTR,
                        *pEndBFromR,
                        *pEndBFromB,
                        *pEndBFromL,
                        aEndBFromTL,
                        pForceColor);
                    rProcessor.process(aSequence);
                }

                // re-init "*Start***" variables
                aStartPos = aEndPos;
                pStart = &rCurr;
                aStartTFromBL = aTFromBL;
                pStartTFromL = &rTFromL;
                pStartTFromT = &rTFromT;
                pStartTFromR = &rTFromR;
                aStartTFromBR = aTFromBR;
            }

            // store current styles in "*End***" variables
            aEndBFromTL = aBFromTL;
            pEndBFromL = &rBFromL;
            pEndBFromB = &rBFromB;
            pEndBFromR = &rBFromR;
            aEndBFromTR = aBFromTR;
        }

        // draw last frame border
        basegfx::B2DPoint aEndPos( aStartPos.getX(), mxImpl->GetRowPosition( nRow ) );
        if ((pStart->Prim() || pStart->Secn()) && (aStartPos.getY() <= aEndPos.getY()))
        {
            // for description of involved coordinate systems have a look at
            // the first CreateBorderPrimitives call above, adapt to vertical
            basegfx::B2DPoint aOrigin(aStartPos);
            basegfx::B2DVector aX(aEndPos - aStartPos);
            basegfx::B2DVector aY(-1.0, 0.0);
            const Cell* pCell = pStart->GetUsingCell();

            if (pCell && pCell->IsRotated())
            {
                const bool bLeft(&pCell->GetStyleLeft() == pStart);
                const tools::Rectangle aRect(GetCellRect(bLeft ? nCol : nCol - 1, nRow - 1));
                const basegfx::B2DRange aRange(aRect.Left(), aRect.Top(), aRect.Right(), aRect.Bottom());

                CreateCoordinateSystemForCell(aRange, *pCell, aOrigin, aX, aY);

                if (!bLeft)
                {
                    aOrigin += aX;
                }

                // The *coordinate system* of the edge has to be given, which for vertical
                // lines uses the Y-Vector as X-Axis and the X-Vector as Y-Axis, so swap both
                // and mirror aX to keep the same orientation (should be (-1.0, 0.0) for horizontal lines anyways)
                std::swap(aX, aY);
                aY.normalize();
                aY = -aY;
            }

            drawinglayer::primitive2d::Primitive2DContainer aSequence;
            CreateBorderPrimitives(
                // also reordered, see call to CreateBorderPrimitives above
                aSequence,
                aOrigin,
                aX,
                aY,
                *pStart,
                aStartTFromBR,
                *pStartTFromR,
                *pStartTFromT,
                *pStartTFromL,
                aStartTFromBL,
                aEndBFromTR,
                *pEndBFromR,
                *pEndBFromB,
                *pEndBFromL,
                aEndBFromTL,
                pForceColor);
            rProcessor.process(aSequence);
        }
    }
}

void Array::DrawArray(drawinglayer::processor2d::BaseProcessor2D& rProcessor) const
{
    if (mxImpl->mnWidth && mxImpl->mnHeight)
        DrawRange(rProcessor, 0, 0, mxImpl->mnWidth - 1, mxImpl->mnHeight - 1, nullptr);
}

#undef ORIGCELL
#undef CELLACC
#undef CELL


#undef DBG_FRAME_CHECK_ROW_1
#undef DBG_FRAME_CHECK_COL_1
#undef DBG_FRAME_CHECK_COLROW
#undef DBG_FRAME_CHECK_ROW
#undef DBG_FRAME_CHECK_COL
#undef DBG_FRAME_CHECK


}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
