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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_svx.hxx"
#include <svx/framelinkarray.hxx>

#include <math.h>
#include <vector>
#include <algorithm>
#include <vcl/outdev.hxx>

namespace svx {
namespace frame {

// ============================================================================


Cell::Cell() :
    mnAddLeft( 0 ),
    mnAddRight( 0 ),
    mnAddTop( 0 ),
    mnAddBottom( 0 ),
    mbMergeOrig( false ),
    mbOverlapX( false ),
    mbOverlapY( false )
{
}

void Cell::MirrorSelfX( bool bMirrorStyles, bool bSwapDiag )
{
    std::swap( maLeft, maRight );
    std::swap( mnAddLeft, mnAddRight );
    if( bMirrorStyles )
    {
        maLeft.MirrorSelf();
        maRight.MirrorSelf();
    }
    if( bSwapDiag )
    {
        std::swap( maTLBR, maBLTR );
        if( bMirrorStyles )
        {
            maTLBR.MirrorSelf();
            maBLTR.MirrorSelf();
        }
    }
}

void Cell::MirrorSelfY( bool bMirrorStyles, bool bSwapDiag )
{
    std::swap( maTop, maBottom );
    std::swap( mnAddTop, mnAddBottom );
    if( bMirrorStyles )
    {
        maTop.MirrorSelf();
        maBottom.MirrorSelf();
    }
    if( bSwapDiag )
        std::swap( maTLBR, maBLTR );
    /*  Do not mirror diagonal styles, because they are oriented vertical.
        Therefore swapping the styles is sufficient for correct behaviour. */
}

// ----------------------------------------------------------------------------


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

// ----------------------------------------------------------------------------

static const Style OBJ_STYLE_NONE;
static const Cell OBJ_CELL_NONE;

const bool DIAG_DBL_CLIP_DEFAULT = false;

// ============================================================================

ArrayImpl::ArrayImpl( size_t nWidth, size_t nHeight, bool bDiagDblClip ) :
    mnWidth( nWidth ),
    mnHeight( nHeight ),
    mnFirstClipCol( 0 ),
    mnFirstClipRow( 0 ),
    mnLastClipCol( nWidth - 1 ),
    mnLastClipRow( nHeight - 1 ),
    mbXCoordsDirty( false ),
    mbYCoordsDirty( false ),
    mbDiagDblClip( bDiagDblClip )
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

Cell& ArrayImpl::GetMergedOriginCellAcc( size_t nCol, size_t nRow )
{
    return GetCellAcc( GetMergedFirstCol( nCol, nRow ), GetMergedFirstRow( nCol, nRow ) );
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

double ArrayImpl::GetHorDiagAngle( size_t nCol, size_t nRow, bool bSimple ) const
{
    double fAngle = 0.0;
    if( IsValidPos( nCol, nRow ) )
    {
        if( bSimple || !GetCell( nCol, nRow ).IsMerged() )
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

double ArrayImpl::GetVerDiagAngle( size_t nCol, size_t nRow, bool bSimple ) const
{
    double fAngle = GetHorDiagAngle( nCol, nRow, bSimple );
    return (fAngle > 0.0) ? (F_PI2 - fAngle) : 0.0;
}

// ============================================================================

class MergedCellIterator
{
public:
    explicit            MergedCellIterator( const Array& rArray, size_t nCol, size_t nRow );

    inline bool         Is() const { return (mnCol <= mnLastCol) && (mnRow <= mnLastRow); }
    inline size_t       Col() const { return mnCol; }
    inline size_t       Row() const { return mnRow; }

    MergedCellIterator& operator++();

private:
    size_t              mnFirstCol;
    size_t              mnFirstRow;
    size_t              mnLastCol;
    size_t              mnLastRow;
    size_t              mnCol;
    size_t              mnRow;
};

// ----------------------------------------------------------------------------

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

// ============================================================================

#define DBG_FRAME_ERROR( funcname, error )              DBG_ERRORFILE( "svx::frame::Array::" funcname " - " error )
#define DBG_FRAME_CHECK( cond, funcname, error )        DBG_ASSERT( cond, "svx::frame::Array::" funcname " - " error )
#define DBG_FRAME_CHECK_COL( col, funcname )            DBG_FRAME_CHECK( (col) < GetColCount(), funcname, "invalid column index" )
#define DBG_FRAME_CHECK_ROW( row, funcname )            DBG_FRAME_CHECK( (row) < GetRowCount(), funcname, "invalid row index" )
#define DBG_FRAME_CHECK_COLROW( col, row, funcname )    DBG_FRAME_CHECK( ((col) < GetColCount()) && ((row) < GetRowCount()), funcname, "invalid cell index" )
#define DBG_FRAME_CHECK_INDEX( index, funcname )        DBG_FRAME_CHECK( (index) < GetCellCount(), funcname, "invalid cell index" )
#define DBG_FRAME_CHECK_COL_1( col, funcname )          DBG_FRAME_CHECK( (col) <= GetColCount(), funcname, "invalid column index" )
#define DBG_FRAME_CHECK_ROW_1( row, funcname )          DBG_FRAME_CHECK( (row) <= GetRowCount(), funcname, "invalid row index" )

// ----------------------------------------------------------------------------

#define CELL( col, row )        mxImpl->GetCell( col, row )
#define CELLACC( col, row )     mxImpl->GetCellAcc( col, row )
#define ORIGCELL( col, row )    mxImpl->GetMergedOriginCell( col, row )
#define ORIGCELLACC( col, row ) mxImpl->GetMergedOriginCellAcc( col, row )

// ----------------------------------------------------------------------------

Array::Array()
{
    Initialize( 0, 0 );
}

Array::Array( size_t nWidth, size_t nHeight )
{
    Initialize( nWidth, nHeight );
}

Array::~Array()
{
}

// array size and column/row indexes ------------------------------------------

void Array::Initialize( size_t nWidth, size_t nHeight )
{
    bool bDiagDblClip = mxImpl.get() ? mxImpl->mbDiagDblClip : DIAG_DBL_CLIP_DEFAULT;
    mxImpl.reset( new ArrayImpl( nWidth, nHeight, bDiagDblClip ) );
}

void Array::Clear()
{
    Initialize( mxImpl->mnWidth, mxImpl->mnHeight );
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

size_t Array::GetColFromIndex( size_t nCellIndex ) const
{
    DBG_FRAME_CHECK_INDEX( nCellIndex, "GetColFromIndex" );
    return mxImpl->mnWidth ? (nCellIndex % mxImpl->mnWidth) : 0;
}

size_t Array::GetRowFromIndex( size_t nCellIndex ) const
{
    DBG_FRAME_CHECK_INDEX( nCellIndex, "GetRowFromIndex" );
    return mxImpl->mnWidth ? (nCellIndex / mxImpl->mnWidth) : 0;
}

size_t Array::GetCellIndex( size_t nCol, size_t nRow, bool bRTL ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetCellIndex" );
    if (bRTL)
        nCol = mxImpl->GetMirrorCol(nCol);
    return mxImpl->GetIndex( nCol, nRow );
}

// cell border styles ---------------------------------------------------------

void Array::SetCellStyleLeft( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleLeft" );
    CELLACC( nCol, nRow ).maLeft = rStyle;
}

void Array::SetCellStyleRight( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleRight" );
    CELLACC( nCol, nRow ).maRight = rStyle;
}

void Array::SetCellStyleTop( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleTop" );
    CELLACC( nCol, nRow ).maTop = rStyle;
}

void Array::SetCellStyleBottom( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleBottom" );
    CELLACC( nCol, nRow ).maBottom = rStyle;
}

void Array::SetCellStyleTLBR( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleTLBR" );
    CELLACC( nCol, nRow ).maTLBR = rStyle;
}

void Array::SetCellStyleBLTR( size_t nCol, size_t nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleBLTR" );
    CELLACC( nCol, nRow ).maBLTR = rStyle;
}

void Array::SetCellStyleDiag( size_t nCol, size_t nRow, const Style& rTLBR, const Style& rBLTR )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleDiag" );
    Cell& rCell = CELLACC( nCol, nRow );
    rCell.maTLBR = rTLBR;
    rCell.maBLTR = rBLTR;
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

const Style& Array::GetCellStyleLeft( size_t nCol, size_t nRow, bool bSimple ) const
{
    // simple: always return own left style
    if( bSimple )
        return CELL( nCol, nRow ).maLeft;
    // outside clipping rows or overlapped in merged cells: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) || mxImpl->IsMergedOverlappedLeft( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // left clipping border: always own left style
    if( nCol == mxImpl->mnFirstClipCol )
        return ORIGCELL( nCol, nRow ).maLeft;
    // right clipping border: always right style of left neighbor cell
    if( nCol == mxImpl->mnLastClipCol + 1 )
        return ORIGCELL( nCol - 1, nRow ).maRight;
    // outside clipping columns: invisible
    if( !mxImpl->IsColInClipRange( nCol ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own left style and right style of left neighbor cell
    return std::max( ORIGCELL( nCol, nRow ).maLeft, ORIGCELL( nCol - 1, nRow ).maRight );
}

const Style& Array::GetCellStyleRight( size_t nCol, size_t nRow, bool bSimple ) const
{
    // simple: always return own right style
    if( bSimple )
        return CELL( nCol, nRow ).maRight;
    // outside clipping rows or overlapped in merged cells: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) || mxImpl->IsMergedOverlappedRight( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // left clipping border: always left style of right neighbor cell
    if( nCol + 1 == mxImpl->mnFirstClipCol )
        return ORIGCELL( nCol + 1, nRow ).maLeft;
    // right clipping border: always own right style
    if( nCol == mxImpl->mnLastClipCol )
        return ORIGCELL( nCol, nRow ).maRight;
    // outside clipping columns: invisible
    if( !mxImpl->IsColInClipRange( nCol ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own right style and left style of right neighbor cell
    return std::max( ORIGCELL( nCol, nRow ).maRight, ORIGCELL( nCol + 1, nRow ).maLeft );
}

const Style& Array::GetCellStyleTop( size_t nCol, size_t nRow, bool bSimple ) const
{
    // simple: always return own top style
    if( bSimple )
        return CELL( nCol, nRow ).maTop;
    // outside clipping columns or overlapped in merged cells: invisible
    if( !mxImpl->IsColInClipRange( nCol ) || mxImpl->IsMergedOverlappedTop( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // top clipping border: always own top style
    if( nRow == mxImpl->mnFirstClipRow )
        return ORIGCELL( nCol, nRow ).maTop;
    // bottom clipping border: always bottom style of top neighbor cell
    if( nRow == mxImpl->mnLastClipRow + 1 )
        return ORIGCELL( nCol, nRow - 1 ).maBottom;
    // outside clipping rows: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own top style and bottom style of top neighbor cell
    return std::max( ORIGCELL( nCol, nRow ).maTop, ORIGCELL( nCol, nRow - 1 ).maBottom );
}

const Style& Array::GetCellStyleBottom( size_t nCol, size_t nRow, bool bSimple ) const
{
    // simple: always return own bottom style
    if( bSimple )
        return CELL( nCol, nRow ).maBottom;
    // outside clipping columns or overlapped in merged cells: invisible
    if( !mxImpl->IsColInClipRange( nCol ) || mxImpl->IsMergedOverlappedBottom( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // top clipping border: always top style of bottom neighbor cell
    if( nRow + 1 == mxImpl->mnFirstClipRow )
        return ORIGCELL( nCol, nRow + 1 ).maTop;
    // bottom clipping border: always own bottom style
    if( nRow == mxImpl->mnLastClipRow )
        return ORIGCELL( nCol, nRow ).maBottom;
    // outside clipping rows: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own bottom style and top style of bottom neighbor cell
    return std::max( ORIGCELL( nCol, nRow ).maBottom, ORIGCELL( nCol, nRow + 1 ).maTop );
}

const Style& Array::GetCellStyleTLBR( size_t nCol, size_t nRow, bool bSimple ) const
{
    return bSimple ? CELL( nCol, nRow ).maTLBR :
        (mxImpl->IsInClipRange( nCol, nRow ) ? ORIGCELL( nCol, nRow ).maTLBR : OBJ_STYLE_NONE);
}

const Style& Array::GetCellStyleBLTR( size_t nCol, size_t nRow, bool bSimple ) const
{
    return bSimple ? CELL( nCol, nRow ).maBLTR :
        (mxImpl->IsInClipRange( nCol, nRow ) ? ORIGCELL( nCol, nRow ).maBLTR : OBJ_STYLE_NONE);
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
        CELL( nFirstCol, nFirstRow ).maTLBR : OBJ_STYLE_NONE;
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
        CELL( mxImpl->GetMergedFirstCol( nCol, nRow ), mxImpl->GetMergedFirstRow( nCol, nRow ) ).maTLBR : OBJ_STYLE_NONE;
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
        CELL( nFirstCol, mxImpl->GetMergedFirstRow( nCol, nRow ) ).maBLTR : OBJ_STYLE_NONE;
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
        CELL( mxImpl->GetMergedFirstCol( nCol, nRow ), nFirstRow ).maBLTR : OBJ_STYLE_NONE;
}

// cell merging ---------------------------------------------------------------

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

void Array::RemoveMergedRange( size_t nCol, size_t nRow )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "RemoveMergedRange" );
    for( MergedCellIterator aIt( *this, nCol, nRow ); aIt.Is(); ++aIt )
    {
        Cell& rCell = CELLACC( aIt.Col(), aIt.Row() );
        rCell.mbMergeOrig = rCell.mbOverlapX = rCell.mbOverlapY = false;
        rCell.mnAddLeft = rCell.mnAddRight = rCell.mnAddTop = rCell.mnAddBottom = 0;
    }
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

bool Array::IsMergedOrigin( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "IsMergedOrigin" );
    return CELL( nCol, nRow ).mbMergeOrig;
}

bool Array::IsMergedOverlapped( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "IsMergedOverlapped" );
    return CELL( nCol, nRow ).IsOverlapped();
}

bool Array::IsMergedOverlappedLeft( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "IsMergedOverlappedLeft" );
    return mxImpl->IsMergedOverlappedLeft( nCol, nRow );
}

bool Array::IsMergedOverlappedRight( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "IsMergedOverlappedRight" );
    return mxImpl->IsMergedOverlappedRight( nCol, nRow );
}

bool Array::IsMergedOverlappedTop( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "IsMergedOverlappedTop" );
    return mxImpl->IsMergedOverlappedTop( nCol, nRow );
}

bool Array::IsMergedOverlappedBottom( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "IsMergedOverlappedBottom" );
    return mxImpl->IsMergedOverlappedBottom( nCol, nRow );
}

void Array::GetMergedOrigin( size_t& rnFirstCol, size_t& rnFirstRow, size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetMergedOrigin" );
    rnFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
    rnFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
}

void Array::GetMergedSize( size_t& rnWidth, size_t& rnHeight, size_t nCol, size_t nRow ) const
{
    size_t nFirstCol, nFirstRow, nLastCol, nLastRow;
    GetMergedRange( nFirstCol, nFirstRow, nLastCol, nLastRow, nCol, nRow );
    rnWidth = nLastCol - nFirstCol + 1;
    rnHeight = nLastRow - nFirstRow + 1;
}

void Array::GetMergedRange( size_t& rnFirstCol, size_t& rnFirstRow,
        size_t& rnLastCol, size_t& rnLastRow, size_t nCol, size_t nRow ) const
{
    GetMergedOrigin( rnFirstCol, rnFirstRow, nCol, nRow );
    rnLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
    rnLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
}

// clipping -------------------------------------------------------------------

void Array::SetClipRange( size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow )
{
    DBG_FRAME_CHECK_COLROW( nFirstCol, nFirstRow, "SetClipRange" );
    DBG_FRAME_CHECK_COLROW( nLastCol, nLastRow, "SetClipRange" );
    mxImpl->mnFirstClipCol = nFirstCol;
    mxImpl->mnFirstClipRow = nFirstRow;
    mxImpl->mnLastClipCol = nLastCol;
    mxImpl->mnLastClipRow = nLastRow;
}

void Array::RemoveClipRange()
{
    if( !mxImpl->maCells.empty() )
        SetClipRange( 0, 0, mxImpl->mnWidth - 1, mxImpl->mnHeight - 1 );
}

bool Array::IsInClipRange( size_t nCol, size_t nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "IsInClipRange" );
    return mxImpl->IsInClipRange( nCol, nRow );
}

Rectangle Array::GetClipRangeRectangle() const
{
    return Rectangle(
        mxImpl->GetColPosition( mxImpl->mnFirstClipCol ),
        mxImpl->GetRowPosition( mxImpl->mnFirstClipRow ),
        mxImpl->GetColPosition( mxImpl->mnLastClipCol + 1 ),
        mxImpl->GetRowPosition( mxImpl->mnLastClipRow + 1 ) );
}

// cell coordinates -----------------------------------------------------------

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

long Array::GetColWidth( size_t nCol ) const
{
    DBG_FRAME_CHECK_COL( nCol, "GetColWidth" );
    return mxImpl->maWidths[ nCol ];
}

long Array::GetColWidth( size_t nFirstCol, size_t nLastCol ) const
{
    DBG_FRAME_CHECK_COL( nFirstCol, "GetColWidth" );
    DBG_FRAME_CHECK_COL( nLastCol, "GetColWidth" );
    return GetColPosition( nLastCol + 1 ) - GetColPosition( nFirstCol );
}

long Array::GetRowHeight( size_t nRow ) const
{
    DBG_FRAME_CHECK_ROW( nRow, "GetRowHeight" );
    return mxImpl->maHeights[ nRow ];
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

Point Array::GetCellPosition( size_t nCol, size_t nRow, bool bSimple ) const
{
    size_t nFirstCol = bSimple ? nCol : mxImpl->GetMergedFirstCol( nCol, nRow );
    size_t nFirstRow = bSimple ? nRow : mxImpl->GetMergedFirstRow( nCol, nRow );
    return Point( GetColPosition( nFirstCol ), GetRowPosition( nFirstRow ) );
}

Size Array::GetCellSize( size_t nCol, size_t nRow, bool bSimple ) const
{
    size_t nFirstCol = bSimple ? nCol : mxImpl->GetMergedFirstCol( nCol, nRow );
    size_t nFirstRow = bSimple ? nRow : mxImpl->GetMergedFirstRow( nCol, nRow );
    size_t nLastCol = bSimple ? nCol : mxImpl->GetMergedLastCol( nCol, nRow );
    size_t nLastRow = bSimple ? nRow : mxImpl->GetMergedLastRow( nCol, nRow );
    return Size( GetColWidth( nFirstCol, nLastCol ) + 1, GetRowHeight( nFirstRow, nLastRow ) + 1 );
}

Rectangle Array::GetCellRect( size_t nCol, size_t nRow, bool bSimple ) const
{
    Rectangle aRect( GetCellPosition( nCol, nRow, bSimple ), GetCellSize( nCol, nRow, bSimple ) );

    // adjust rectangle for partly visible merged cells
    const Cell& rCell = CELL( nCol, nRow );
    if( !bSimple && rCell.IsMerged() )
    {
        aRect.Left() -= rCell.mnAddLeft;
        aRect.Right() += rCell.mnAddRight;
        aRect.Top() -= rCell.mnAddTop;
        aRect.Bottom() += rCell.mnAddBottom;
    }
    return aRect;
}

// diagonal frame borders -----------------------------------------------------

double Array::GetHorDiagAngle( size_t nCol, size_t nRow, bool bSimple ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetHorDiagAngle" );
    return mxImpl->GetHorDiagAngle( nCol, nRow, bSimple );
}

double Array::GetVerDiagAngle( size_t nCol, size_t nRow, bool bSimple ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetVerDiagAngle" );
    return mxImpl->GetVerDiagAngle( nCol, nRow, bSimple );
}

void Array::SetUseDiagDoubleClipping( bool bSet )
{
    mxImpl->mbDiagDblClip = bSet;
}

bool Array::GetUseDiagDoubleClipping() const
{
    return mxImpl->mbDiagDblClip;
}

// mirroring ------------------------------------------------------------------

void Array::MirrorSelfX( bool bMirrorStyles, bool bSwapDiag )
{
    CellVec aNewCells;
    aNewCells.reserve( GetCellCount() );

    size_t nCol, nRow;
    for( nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
    {
        for( nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        {
            aNewCells.push_back( CELL( mxImpl->GetMirrorCol( nCol ), nRow ) );
            aNewCells.back().MirrorSelfX( bMirrorStyles, bSwapDiag );
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

void Array::MirrorSelfY( bool bMirrorStyles, bool bSwapDiag )
{
    CellVec aNewCells;
    aNewCells.reserve( GetCellCount() );

    size_t nCol, nRow;
    for( nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
    {
        for( nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        {
            aNewCells.push_back( CELL( nCol, mxImpl->GetMirrorRow( nRow ) ) );
            aNewCells.back().MirrorSelfY( bMirrorStyles, bSwapDiag );
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
                    nCol, mxImpl->GetMirrorRow( nLastRow ),
                    nLastCol, mxImpl->GetMirrorRow( nRow ) );
            }
        }
    }
    mxImpl->maCells.swap( aNewCells );

    std::reverse( mxImpl->maHeights.begin(), mxImpl->maHeights.end() );
    mxImpl->mbYCoordsDirty = true;
}

// drawing --------------------------------------------------------------------

void Array::DrawCell( OutputDevice& rDev, size_t nCol, size_t nRow, const Color* pForceColor ) const
{
    DrawRange( rDev, nCol, nRow, nCol, nRow, pForceColor );
}

void Array::DrawRange( drawinglayer::processor2d::BaseProcessor2D* pProcessor,
        size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow,
        const Color* pForceColor ) const
{
    DBG_FRAME_CHECK_COLROW( nFirstCol, nFirstRow, "DrawRange" );
    DBG_FRAME_CHECK_COLROW( nLastCol, nLastRow, "DrawRange" );

    size_t nCol, nRow;

    // *** diagonal frame borders ***
    for( nRow = nFirstRow; nRow <= nLastRow; ++nRow )
    {
        for( nCol = nFirstCol; nCol <= nLastCol; ++nCol )
        {
            const Cell& rCell = CELL( nCol, nRow );
            bool bOverlapX = rCell.mbOverlapX;
            bool bOverlapY = rCell.mbOverlapY;
            bool bFirstCol = nCol == nFirstCol;
            bool bFirstRow = nRow == nFirstRow;
            if( (!bOverlapX && !bOverlapY) || (bFirstCol && bFirstRow) ||
                (!bOverlapY && bFirstCol) || (!bOverlapX && bFirstRow) )
            {
                Rectangle aRect( GetCellRect( nCol, nRow ) );
                if( (aRect.GetWidth() > 1) && (aRect.GetHeight() > 1) )
                {
                    size_t _nFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
                    size_t _nFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );

                    const Style aTlbrStyle = GetCellStyleTLBR( _nFirstCol, _nFirstRow, true );
                    if ( aTlbrStyle.GetWidth( ) )
                        pProcessor->process( CreateClippedBorderPrimitives(
                                    aRect.TopLeft(), aRect.BottomRight(),
                                    aTlbrStyle, aRect ) );

                    const Style aBltrStyle = GetCellStyleBLTR( _nFirstCol, _nFirstRow, true );
                    if ( aBltrStyle.GetWidth( ) )
                        pProcessor->process( CreateClippedBorderPrimitives(
                                    aRect.BottomLeft(), aRect.TopRight(),
                                    aBltrStyle, aRect ) );
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
        Point aStartPos( mxImpl->GetColPosition( nFirstCol ), mxImpl->GetRowPosition( nRow ) );
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
            if( !CheckFrameBorderConnectable( *pStart, rCurr,
                    aEndRFromTL, rLFromT, aLFromTR, aEndRFromBL, rLFromB, aLFromBR ) )
            {
                // draw previous frame border
                Point aEndPos( mxImpl->GetColPosition( nCol ), aStartPos.Y() );
                if( pStart->Prim() && (aStartPos.X() <= aEndPos.X()) )
                   pProcessor->process( CreateBorderPrimitives( aStartPos, aEndPos, *pStart,
                        aStartLFromTR, *pStartLFromT, *pStartLFromL, *pStartLFromB, aStartLFromBR,
                        aEndRFromTL, *pEndRFromT, *pEndRFromR, *pEndRFromB, aEndRFromBL, pForceColor ) );

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
        Point aEndPos( mxImpl->GetColPosition( nCol ), aStartPos.Y() );
        if( pStart->Prim() && (aStartPos.X() <= aEndPos.X()) )
            pProcessor->process( CreateBorderPrimitives( aStartPos, aEndPos, *pStart,
                aStartLFromTR, *pStartLFromT, *pStartLFromL, *pStartLFromB, aStartLFromBR,
                aEndRFromTL, *pEndRFromT, *pEndRFromR, *pEndRFromB, aEndRFromBL, pForceColor ) );
    }

    // *** vertical frame borders ***
    for( nCol = nFirstCol; nCol <= nLastCol + 1; ++nCol )
    {
        double fAngle = mxImpl->GetVerDiagAngle( nCol, nFirstRow );
        double fLAngle = mxImpl->GetVerDiagAngle( nCol - 1, nFirstRow );

        // *Start*** variables store the data of the top end of the cached frame border
        Point aStartPos( mxImpl->GetColPosition( nCol ), mxImpl->GetRowPosition( nFirstRow ) );
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
                Point aEndPos( aStartPos.X(), mxImpl->GetRowPosition( nRow ) );
                if( pStart->Prim() && (aStartPos.Y() <= aEndPos.Y()) )
                    pProcessor->process( CreateBorderPrimitives( aEndPos, aStartPos, *pStart,
                        aEndBFromTL, *pEndBFromL, *pEndBFromB, *pEndBFromR, aEndBFromTR,
                        aStartTFromBL, *pStartTFromL, *pStartTFromT, *pStartTFromR, aStartTFromBR, pForceColor ) );

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
        Point aEndPos( aStartPos.X(), mxImpl->GetRowPosition( nRow ) );
        if( pStart->Prim() && (aStartPos.Y() <= aEndPos.Y()) )
            pProcessor->process( CreateBorderPrimitives( aEndPos, aStartPos, *pStart,
                aEndBFromTL, *pEndBFromL, *pEndBFromB, *pEndBFromR, aEndBFromTR,
                aStartTFromBL, *pStartTFromL, *pStartTFromT, *pStartTFromR, aStartTFromBR, pForceColor ) );
    }
}

void Array::DrawRange( OutputDevice& rDev,
        size_t nFirstCol, size_t nFirstRow, size_t nLastCol, size_t nLastRow,
        const Color* pForceColor ) const
{
    DBG_FRAME_CHECK_COLROW( nFirstCol, nFirstRow, "DrawRange" );
    DBG_FRAME_CHECK_COLROW( nLastCol, nLastRow, "DrawRange" );

    size_t nCol, nRow;

    // *** diagonal frame borders ***

    // set clipping region to clip partly visible merged cells
    rDev.Push( PUSH_CLIPREGION );
    rDev.IntersectClipRegion( GetClipRangeRectangle() );
    for( nRow = nFirstRow; nRow <= nLastRow; ++nRow )
    {
        for( nCol = nFirstCol; nCol <= nLastCol; ++nCol )
        {
            const Cell& rCell = CELL( nCol, nRow );
            bool bOverlapX = rCell.mbOverlapX;
            bool bOverlapY = rCell.mbOverlapY;
            bool bFirstCol = nCol == nFirstCol;
            bool bFirstRow = nRow == nFirstRow;
            if( (!bOverlapX && !bOverlapY) || (bFirstCol && bFirstRow) ||
                (!bOverlapY && bFirstCol) || (!bOverlapX && bFirstRow) )
            {
                Rectangle aRect( GetCellRect( nCol, nRow ) );
                if( (aRect.GetWidth() > 1) && (aRect.GetHeight() > 1) )
                {
                    size_t _nFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
                    size_t _nFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
                    size_t _nLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
                    size_t _nLastRow = mxImpl->GetMergedLastRow( nCol, nRow );

                    DrawDiagFrameBorders( rDev, aRect,
                        GetCellStyleTLBR( _nFirstCol, _nFirstRow, true ), GetCellStyleBLTR( _nFirstCol, _nFirstRow, true ),
                        GetCellStyleLeft( _nFirstCol, _nFirstRow ), GetCellStyleTop( _nFirstCol, _nFirstRow ),
                        GetCellStyleRight( _nLastCol, _nLastRow ), GetCellStyleBottom( _nLastCol, _nLastRow ),
                        GetCellStyleLeft( _nFirstCol, _nLastRow ), GetCellStyleBottom( _nFirstCol, _nLastRow ),
                        GetCellStyleRight( _nLastCol, _nFirstRow ), GetCellStyleTop( _nLastCol, _nFirstRow ),
                        pForceColor, mxImpl->mbDiagDblClip );
                }
            }
        }
    }
    rDev.Pop(); // clip region

    // *** horizontal frame borders ***

    for( nRow = nFirstRow; nRow <= nLastRow + 1; ++nRow )
    {
        double fAngle = mxImpl->GetHorDiagAngle( nFirstCol, nRow );
        double fTAngle = mxImpl->GetHorDiagAngle( nFirstCol, nRow - 1 );

        // *Start*** variables store the data of the left end of the cached frame border
        Point aStartPos( mxImpl->GetColPosition( nFirstCol ), mxImpl->GetRowPosition( nRow ) );
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
            if( !CheckFrameBorderConnectable( *pStart, rCurr,
                    aEndRFromTL, rLFromT, aLFromTR, aEndRFromBL, rLFromB, aLFromBR ) )
            {
                // draw previous frame border
                Point aEndPos( mxImpl->GetColPosition( nCol ), aStartPos.Y() );
                if( pStart->Prim() && (aStartPos.X() <= aEndPos.X()) )
                    DrawHorFrameBorder( rDev, aStartPos, aEndPos, *pStart,
                        aStartLFromTR, *pStartLFromT, *pStartLFromL, *pStartLFromB, aStartLFromBR,
                        aEndRFromTL, *pEndRFromT, *pEndRFromR, *pEndRFromB, aEndRFromBL, pForceColor );

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
        Point aEndPos( mxImpl->GetColPosition( nCol ), aStartPos.Y() );
        if( pStart->Prim() && (aStartPos.X() <= aEndPos.X()) )
            DrawHorFrameBorder( rDev, aStartPos, aEndPos, *pStart,
                aStartLFromTR, *pStartLFromT, *pStartLFromL, *pStartLFromB, aStartLFromBR,
                aEndRFromTL, *pEndRFromT, *pEndRFromR, *pEndRFromB, aEndRFromBL, pForceColor );
    }

    // *** vertical frame borders ***

    for( nCol = nFirstCol; nCol <= nLastCol + 1; ++nCol )
    {
        double fAngle = mxImpl->GetVerDiagAngle( nCol, nFirstRow );
        double fLAngle = mxImpl->GetVerDiagAngle( nCol - 1, nFirstRow );

        // *Start*** variables store the data of the top end of the cached frame border
        Point aStartPos( mxImpl->GetColPosition( nCol ), mxImpl->GetRowPosition( nFirstRow ) );
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
                Point aEndPos( aStartPos.X(), mxImpl->GetRowPosition( nRow ) );
                if( pStart->Prim() && (aStartPos.Y() <= aEndPos.Y()) )
                    DrawVerFrameBorder( rDev, aStartPos, aEndPos, *pStart,
                        aStartTFromBL, *pStartTFromL, *pStartTFromT, *pStartTFromR, aStartTFromBR,
                        aEndBFromTL, *pEndBFromL, *pEndBFromB, *pEndBFromR, aEndBFromTR, pForceColor );

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
        Point aEndPos( aStartPos.X(), mxImpl->GetRowPosition( nRow ) );
        if( pStart->Prim() && (aStartPos.Y() <= aEndPos.Y()) )
            DrawVerFrameBorder( rDev, aStartPos, aEndPos, *pStart,
                aStartTFromBL, *pStartTFromL, *pStartTFromT, *pStartTFromR, aStartTFromBR,
                aEndBFromTL, *pEndBFromL, *pEndBFromB, *pEndBFromR, aEndBFromTR, pForceColor );
    }
}

void Array::DrawArray( OutputDevice& rDev, const Color* pForceColor ) const
{
    if( mxImpl->mnWidth && mxImpl->mnHeight )
        DrawRange( rDev, 0, 0, mxImpl->mnWidth - 1, mxImpl->mnHeight - 1, pForceColor );
}

// ----------------------------------------------------------------------------

#undef ORIGCELLACC
#undef ORIGCELL
#undef CELLACC
#undef CELL

// ----------------------------------------------------------------------------

#undef DBG_FRAME_CHECK_ROW_1
#undef DBG_FRAME_CHECK_COL_1
#undef DBG_FRAME_CHECK_INDEX
#undef DBG_FRAME_CHECK_COLROW
#undef DBG_FRAME_CHECK_ROW
#undef DBG_FRAME_CHECK_COL
#undef DBG_FRAME_CHECK
#undef DBG_FRAME_ERROR

// ============================================================================

} // namespace frame
} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
