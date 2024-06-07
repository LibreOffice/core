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
#include <set>
#include <unordered_set>
#include <algorithm>
#include <o3tl/hash_combine.hxx>
#include <tools/debug.hxx>
#include <tools/gen.hxx>
#include <vcl/canvastools.hxx>
#include <svx/sdr/primitive2d/sdrframeborderprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>

//#define OPTICAL_CHECK_CLIPRANGE_FOR_MERGED_CELL
#ifdef OPTICAL_CHECK_CLIPRANGE_FOR_MERGED_CELL
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <drawinglayer/primitive2d/PolygonHairlinePrimitive2D.hxx>
#endif

namespace svx::frame {

namespace {

class Cell final
{
private:
    Style               maLeft;
    Style               maRight;
    Style               maTop;
    Style               maBottom;
    Style               maTLBR;
    Style               maBLTR;

    basegfx::B2DHomMatrix HelperCreateB2DHomMatrixFromB2DRange(
        const basegfx::B2DRange& rRange ) const;

public:
    sal_Int32                mnAddLeft;
    sal_Int32                mnAddRight;
    sal_Int32                mnAddTop;
    sal_Int32                mnAddBottom;

    SvxRotateMode       meRotMode;
    double              mfOrientation;

    bool                mbMergeOrig;
    bool                mbOverlapX;
    bool                mbOverlapY;

public:
    explicit Cell();
    explicit Cell(const Cell&) = default;

    bool operator==( const Cell& ) const;
    size_t hashCode() const;

    void SetStyleLeft(const Style& rStyle) { maLeft = rStyle; }
    void SetStyleRight(const Style& rStyle) { maRight = rStyle; }
    void SetStyleTop(const Style& rStyle) { maTop = rStyle; }
    void SetStyleBottom(const Style& rStyle) { maBottom = rStyle; }
    void SetStyleTLBR(const Style& rStyle) { maTLBR = rStyle; }
    void SetStyleBLTR(const Style& rStyle) { maBLTR = rStyle; }

    const Style& GetStyleLeft() const { return maLeft; }
    const Style& GetStyleRight() const { return maRight; }
    const Style& GetStyleTop() const { return maTop; }
    const Style& GetStyleBottom() const { return maBottom; }
    const Style& GetStyleTLBR() const { return maTLBR; }
    const Style& GetStyleBLTR() const { return maBLTR; }

    bool                IsMerged() const { return mbMergeOrig || mbOverlapX || mbOverlapY; }
    bool                IsRotated() const { return mfOrientation != 0.0; }

    void                MirrorSelfX();

    basegfx::B2DHomMatrix CreateCoordinateSystemSingleCell(
        const Array& rArray, sal_Int32 nCol, sal_Int32 nRow ) const;
    basegfx::B2DHomMatrix CreateCoordinateSystemMergedCell(
        const Array& rArray, sal_Int32 nColLeft, sal_Int32 nRowTop, sal_Int32 nColRight, sal_Int32 nRowBottom ) const;
};

}

typedef std::vector< const Cell* >     CellVec;

basegfx::B2DHomMatrix Cell::HelperCreateB2DHomMatrixFromB2DRange(
    const basegfx::B2DRange& rRange ) const
{
    if( rRange.isEmpty() )
        return basegfx::B2DHomMatrix();

    basegfx::B2DPoint aOrigin(rRange.getMinimum());
    basegfx::B2DVector aX(rRange.getWidth(), 0.0);
    basegfx::B2DVector aY(0.0, rRange.getHeight());

    if (IsRotated() && SvxRotateMode::SVX_ROTATE_MODE_STANDARD != meRotMode )
    {
        // tdf#143377 We need to limit applying Skew to geometry since the closer
        // we get to 0.0 or PI the more sin(mfOrientation) will get to zero and the
        // huger the Skew effect will be. For that, use an epsilon-radius of 1/2
        // degree around the dangerous points 0.0 and PI.

        // Snap to modulo to [0.0 .. 2PI[ to make compare easier
        const double fSnapped(::basegfx::snapToZeroRange(mfOrientation, M_PI * 2.0));

        // As a compromise, allow up to 1/2 degree
        static const double fMinAng(M_PI/360.0);

        // Check if Skew makes sense or would be too huge
        const bool bForbidSkew(
            fSnapped < fMinAng || // range [0.0 .. fMinAng]
            fSnapped > (M_PI * 2.0) - fMinAng || // range [PI-fMinAng .. 2PI[
            fabs(fSnapped - M_PI) < fMinAng); // range [PI-fMinAng .. PI+fMinAng]

        if(!bForbidSkew)
        {
            // when rotated, adapt values. Get Skew (cos/sin == 1/tan)
            const double fSkew(aY.getY() * (cos(mfOrientation) / sin(mfOrientation)));

            switch (meRotMode)
            {
            case SvxRotateMode::SVX_ROTATE_MODE_TOP:
                // shear Y-Axis
                aY.setX(-fSkew);
                break;
            case SvxRotateMode::SVX_ROTATE_MODE_CENTER:
                // shear origin half, Y full
                aOrigin.setX(aOrigin.getX() + (fSkew * 0.5));
                aY.setX(-fSkew);
                break;
            case SvxRotateMode::SVX_ROTATE_MODE_BOTTOM:
                // shear origin full, Y full
                aOrigin.setX(aOrigin.getX() + fSkew);
                aY.setX(-fSkew);
                break;
            default: // SvxRotateMode::SVX_ROTATE_MODE_STANDARD, already excluded above
                break;
            }
        }
    }

    // use column vectors as coordinate axes, homogen column for translation
    return basegfx::utils::createCoordinateSystemTransform( aOrigin, aX, aY );
}

basegfx::B2DHomMatrix Cell::CreateCoordinateSystemSingleCell(
    const Array& rArray, sal_Int32 nCol, sal_Int32 nRow) const
{
    const Point aPoint( rArray.GetColPosition( nCol ), rArray.GetRowPosition( nRow ) );
    const Size aSize( rArray.GetColWidth( nCol, nCol ) + 1, rArray.GetRowHeight( nRow, nRow ) + 1 );
    const basegfx::B2DRange aRange( vcl::unotools::b2DRectangleFromRectangle( tools::Rectangle( aPoint, aSize ) ) );

    return HelperCreateB2DHomMatrixFromB2DRange( aRange );
}

basegfx::B2DHomMatrix Cell::CreateCoordinateSystemMergedCell(
    const Array& rArray, sal_Int32 nColLeft, sal_Int32 nRowTop, sal_Int32 nColRight, sal_Int32 nRowBottom) const
{
    basegfx::B2DRange aRange( rArray.GetB2DRange(
        nColLeft, nRowTop, nColRight, nRowBottom ) );

    // adjust rectangle for partly visible merged cells
    if( IsMerged() )
    {
        // not *sure* what exactly this is good for,
        // it is just a hard set extension at merged cells,
        // probably *should* be included in the above extended
        // GetColPosition/GetColWidth already. This might be
        // added due to GetColPosition/GetColWidth not working
        // correctly over PageChanges (if used), but not sure.
        aRange.expand(
            basegfx::B2DRange(
                aRange.getMinX() - mnAddLeft,
                aRange.getMinY() - mnAddTop,
                aRange.getMaxX() + mnAddRight,
                aRange.getMaxY() + mnAddBottom ) );
    }

    return HelperCreateB2DHomMatrixFromB2DRange( aRange );
}

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

bool Cell::operator==(const Cell& rOther) const
{
    if (this == &rOther)
        // ptr compare (same instance)
        return true;

    return maLeft == rOther.maLeft
        && maRight == rOther.maRight
        && maTop == rOther.maTop
        && maBottom == rOther.maBottom
        && maTLBR == rOther.maTLBR
        && maBLTR == rOther.maBLTR
        && mnAddLeft == rOther.mnAddLeft
        && mnAddRight == rOther.mnAddRight
        && mnAddTop == rOther.mnAddTop
        && mnAddBottom == rOther.mnAddBottom
        && meRotMode == rOther.meRotMode
        && mfOrientation == rOther.mfOrientation
        && mbMergeOrig == rOther.mbMergeOrig
        && mbOverlapX == rOther.mbOverlapX
        && mbOverlapY == rOther.mbOverlapY;
}

size_t Cell::hashCode() const
{
    std::size_t seed = 0;
    o3tl::hash_combine(seed, maLeft.hashCode());
    o3tl::hash_combine(seed, maRight.hashCode());
    o3tl::hash_combine(seed, maTop.hashCode());
    o3tl::hash_combine(seed, maBottom.hashCode());
    o3tl::hash_combine(seed, maTLBR.hashCode());
    o3tl::hash_combine(seed, maBLTR.hashCode());
    o3tl::hash_combine(seed, mnAddLeft);
    o3tl::hash_combine(seed, mnAddRight);
    o3tl::hash_combine(seed, mnAddTop);
    o3tl::hash_combine(seed, mnAddBottom);
    o3tl::hash_combine(seed, meRotMode);
    o3tl::hash_combine(seed, mfOrientation);
    o3tl::hash_combine(seed, mbMergeOrig);
    o3tl::hash_combine(seed, mbOverlapX);
    o3tl::hash_combine(seed, mbOverlapY);
    return seed;
}

void Cell::MirrorSelfX()
{
    std::swap( maLeft, maRight );
    std::swap( mnAddLeft, mnAddRight );
    maLeft.MirrorSelf();
    maRight.MirrorSelf();
    mfOrientation = -mfOrientation;
}


static void lclRecalcCoordVec( std::vector<sal_Int32>& rCoords, const std::vector<sal_Int32>& rSizes )
{
    DBG_ASSERT( rCoords.size() == rSizes.size() + 1, "lclRecalcCoordVec - inconsistent vectors" );
    auto aCIt = rCoords.begin();
    for( const auto& rSize : rSizes )
    {
        *(aCIt + 1) = *aCIt + rSize;
        ++aCIt;
    }
}

const Style OBJ_STYLE_NONE;
const Cell OBJ_CELL_NONE;

/** use hashing to speed up finding duplicates */
namespace
{
struct RegisteredCellHash
{
    size_t operator()(Cell* const pCell) const
    {
        return pCell->hashCode();
    }
};

struct RegisteredCellEquals
{
    bool operator()(Cell* const pCell1, Cell* const pCell2) const
    {
        return *pCell1 == *pCell2;
    }
};
}

struct ArrayImpl
{
    std::unordered_set<Cell*, RegisteredCellHash, RegisteredCellEquals> maRegisteredCells;
    CellVec             maCells;
    std::vector<sal_Int32>   maWidths;
    std::vector<sal_Int32>   maHeights;
    mutable std::vector<sal_Int32>     maXCoords;
    mutable std::vector<sal_Int32>     maYCoords;
    sal_Int32           mnWidth;
    sal_Int32           mnHeight;
    sal_Int32           mnFirstClipCol;
    sal_Int32           mnFirstClipRow;
    sal_Int32           mnLastClipCol;
    sal_Int32           mnLastClipRow;
    mutable bool        mbXCoordsDirty;
    mutable bool        mbYCoordsDirty;
    bool                mbMayHaveCellRotation;

    explicit            ArrayImpl( sal_Int32 nWidth, sal_Int32 nHeight );
    ~ArrayImpl();

    bool         IsValidPos( sal_Int32 nCol, sal_Int32 nRow ) const
                            { return (nCol < mnWidth) && (nRow < mnHeight); }
    sal_Int32       GetIndex( sal_Int32 nCol, sal_Int32 nRow ) const
                            { return nRow * mnWidth + nCol; }

    const Cell*         GetCell( sal_Int32 nCol, sal_Int32 nRow ) const;
    void                PutCell( sal_Int32 nCol, sal_Int32 nRow, const Cell& );

    sal_Int32              GetMergedFirstCol( sal_Int32 nCol, sal_Int32 nRow ) const;
    sal_Int32              GetMergedFirstRow( sal_Int32 nCol, sal_Int32 nRow ) const;
    sal_Int32              GetMergedLastCol( sal_Int32 nCol, sal_Int32 nRow ) const;
    sal_Int32              GetMergedLastRow( sal_Int32 nCol, sal_Int32 nRow ) const;

    const Cell*         GetMergedOriginCell( sal_Int32 nCol, sal_Int32 nRow ) const;
    const Cell*         GetMergedLastCell( sal_Int32 nCol, sal_Int32 nRow ) const;

    bool                IsMergedOverlappedLeft( sal_Int32 nCol, sal_Int32 nRow ) const;
    bool                IsMergedOverlappedRight( sal_Int32 nCol, sal_Int32 nRow ) const;
    bool                IsMergedOverlappedTop( sal_Int32 nCol, sal_Int32 nRow ) const;
    bool                IsMergedOverlappedBottom( sal_Int32 nCol, sal_Int32 nRow ) const;

    bool                IsInClipRange( sal_Int32 nCol, sal_Int32 nRow ) const;
    bool                IsColInClipRange( sal_Int32 nCol ) const;
    bool                IsRowInClipRange( sal_Int32 nRow ) const;

    bool                OverlapsClipRange( sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow ) const;

    sal_Int32           GetMirrorCol( sal_Int32 nCol ) const { return mnWidth - nCol - 1; }

    sal_Int32           GetColPosition( sal_Int32 nCol ) const;
    sal_Int32           GetRowPosition( sal_Int32 nRow ) const;

    bool                HasCellRotation() const;

    const Cell* createOrFind(const Cell& rCell);
};

static void lclSetMergedRange( ArrayImpl& rImpl, CellVec& rCells, sal_Int32 nWidth, sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow )
{
    for( sal_Int32 nCol = nFirstCol; nCol <= nLastCol; ++nCol )
    {
        for( sal_Int32 nRow = nFirstRow; nRow <= nLastRow; ++nRow )
        {
            const Cell* pCell = rCells[ nRow * nWidth + nCol ];
            Cell aTempCell(*pCell);
            aTempCell.mbMergeOrig = false;
            aTempCell.mbOverlapX = nCol > nFirstCol;
            aTempCell.mbOverlapY = nRow > nFirstRow;
            rCells[ nRow * nWidth + nCol ] = rImpl.createOrFind(aTempCell);
        }
    }
    Cell aTempCell(*rCells[ nFirstRow * nWidth + nFirstCol ]);
    aTempCell.mbMergeOrig = true;
    rCells[ nFirstRow * nWidth + nFirstCol ] = rImpl.createOrFind(aTempCell);
}

ArrayImpl::ArrayImpl( sal_Int32 nWidth, sal_Int32 nHeight ) :
    maRegisteredCells(),
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
    const Cell* pDefaultCell = createOrFind(Cell());
    // default-construct all vectors
    maCells.resize( mnWidth * mnHeight, pDefaultCell );
    maWidths.resize( mnWidth, 0 );
    maHeights.resize( mnHeight, 0 );
    maXCoords.resize( mnWidth + 1, 0 );
    maYCoords.resize( mnHeight + 1, 0 );
}

ArrayImpl::~ArrayImpl()
{
    for (auto* pCell : maRegisteredCells)
        delete pCell;
}

const Cell* ArrayImpl::createOrFind(const Cell& rCell)
{
    auto it = maRegisteredCells.find(const_cast<Cell*>(&rCell));
    if (it != maRegisteredCells.end())
        return *it;

    Cell* pRetval(new Cell(rCell));
    maRegisteredCells.insert(pRetval);
    return pRetval;
}

const Cell* ArrayImpl::GetCell( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return IsValidPos( nCol, nRow ) ? maCells[ GetIndex( nCol, nRow ) ] : &OBJ_CELL_NONE;
}

void ArrayImpl::PutCell( sal_Int32 nCol, sal_Int32 nRow, const Cell & rCell )
{
    if (IsValidPos( nCol, nRow ))
        maCells[ GetIndex( nCol, nRow ) ] = createOrFind(rCell);
}

sal_Int32 ArrayImpl::GetMergedFirstCol( sal_Int32 nCol, sal_Int32 nRow ) const
{
    sal_Int32 nFirstCol = nCol;
    while( (nFirstCol > 0) && GetCell( nFirstCol, nRow )->mbOverlapX ) --nFirstCol;
    return nFirstCol;
}

sal_Int32 ArrayImpl::GetMergedFirstRow( sal_Int32 nCol, sal_Int32 nRow ) const
{
    sal_Int32 nFirstRow = nRow;
    while( (nFirstRow > 0) && GetCell( nCol, nFirstRow )->mbOverlapY ) --nFirstRow;
    return nFirstRow;
}

sal_Int32 ArrayImpl::GetMergedLastCol( sal_Int32 nCol, sal_Int32 nRow ) const
{
    sal_Int32 nLastCol = nCol + 1;
    while( (nLastCol < mnWidth) && GetCell( nLastCol, nRow )->mbOverlapX ) ++nLastCol;
    return nLastCol - 1;
}

sal_Int32 ArrayImpl::GetMergedLastRow( sal_Int32 nCol, sal_Int32 nRow ) const
{
    sal_Int32 nLastRow = nRow + 1;
    while( (nLastRow < mnHeight) && GetCell( nCol, nLastRow )->mbOverlapY ) ++nLastRow;
    return nLastRow - 1;
}

const Cell* ArrayImpl::GetMergedOriginCell( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return GetCell( GetMergedFirstCol( nCol, nRow ), GetMergedFirstRow( nCol, nRow ) );
}

const Cell* ArrayImpl::GetMergedLastCell( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return GetCell( GetMergedLastCol( nCol, nRow ), GetMergedLastRow( nCol, nRow ) );
}

bool ArrayImpl::IsMergedOverlappedLeft( sal_Int32 nCol, sal_Int32 nRow ) const
{
    const Cell* pCell(GetCell( nCol, nRow ));
    return pCell->mbOverlapX || (pCell->mnAddLeft > 0);
}

bool ArrayImpl::IsMergedOverlappedRight( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return GetCell( nCol + 1, nRow )->mbOverlapX || (GetCell( nCol, nRow )->mnAddRight > 0);
}

bool ArrayImpl::IsMergedOverlappedTop( sal_Int32 nCol, sal_Int32 nRow ) const
{
    const Cell* pCell(GetCell( nCol, nRow ));
    return pCell->mbOverlapY || (pCell->mnAddTop > 0);
}

bool ArrayImpl::IsMergedOverlappedBottom( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return GetCell( nCol, nRow + 1 )->mbOverlapY || (GetCell( nCol, nRow )->mnAddBottom > 0);
}

bool ArrayImpl::IsColInClipRange( sal_Int32 nCol ) const
{
    return (mnFirstClipCol <= nCol) && (nCol <= mnLastClipCol);
}

bool ArrayImpl::IsRowInClipRange( sal_Int32 nRow ) const
{
    return (mnFirstClipRow <= nRow) && (nRow <= mnLastClipRow);
}

bool ArrayImpl::OverlapsClipRange( sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow ) const
{
    if(nLastCol < mnFirstClipCol)
        return false;

    if(nFirstCol > mnLastClipCol)
        return false;

    if(nLastRow < mnFirstClipRow)
        return false;

    if(nFirstRow > mnLastClipRow)
        return false;

    return true;
}

bool ArrayImpl::IsInClipRange( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return IsColInClipRange( nCol ) && IsRowInClipRange( nRow );
}

sal_Int32 ArrayImpl::GetColPosition( sal_Int32 nCol ) const
{
    if( mbXCoordsDirty )
    {
        lclRecalcCoordVec( maXCoords, maWidths );
        mbXCoordsDirty = false;
    }
    return maXCoords[ nCol ];
}

sal_Int32 ArrayImpl::GetRowPosition( sal_Int32 nRow ) const
{
    if( mbYCoordsDirty )
    {
        lclRecalcCoordVec( maYCoords, maHeights );
        mbYCoordsDirty = false;
    }
    return maYCoords[ nRow ];
}

bool ArrayImpl::HasCellRotation() const
{
    // check cell array
    for (const auto& aCell : maCells)
    {
        if (aCell->IsRotated())
        {
            return true;
        }
    }

    return false;
}

namespace {

class MergedCellIterator
{
public:
    explicit            MergedCellIterator( const Array& rArray, sal_Int32 nCol, sal_Int32 nRow );

    bool         Is() const { return (mnCol <= mnLastCol) && (mnRow <= mnLastRow); }
    sal_Int32       Col() const { return mnCol; }
    sal_Int32       Row() const { return mnRow; }

    MergedCellIterator& operator++();

private:
    sal_Int32              mnFirstCol;
    sal_Int32              mnFirstRow;
    sal_Int32              mnLastCol;
    sal_Int32              mnLastRow;
    sal_Int32              mnCol;
    sal_Int32              mnRow;
};

}

MergedCellIterator::MergedCellIterator( const Array& rArray, sal_Int32 nCol, sal_Int32 nRow )
{
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

Array::Array()
{
    Initialize( 0, 0 );
}

Array::~Array()
{
}

// array size and column/row indexes
void Array::Initialize( sal_Int32 nWidth, sal_Int32 nHeight )
{
    mxImpl.reset( new ArrayImpl( nWidth, nHeight ) );
}

sal_Int32 Array::GetColCount() const
{
    return mxImpl->mnWidth;
}

sal_Int32 Array::GetRowCount() const
{
    return mxImpl->mnHeight;
}

sal_Int32 Array::GetCellCount() const
{
    return mxImpl->maCells.size();
}

sal_Int32 Array::GetCellIndex( sal_Int32 nCol, sal_Int32 nRow, bool bRTL ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetCellIndex" );
    if (bRTL)
        nCol = mxImpl->GetMirrorCol(nCol);
    return mxImpl->GetIndex( nCol, nRow );
}

// cell border styles
void Array::SetCellStyleLeft( sal_Int32 nCol, sal_Int32 nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleLeft" );
    const Cell* pTempCell(mxImpl->GetCell(nCol, nRow));
    if (pTempCell->GetStyleLeft() == rStyle)
        return;
    Cell aTempCell(*pTempCell);
    aTempCell.SetStyleLeft(rStyle);
    mxImpl->PutCell( nCol, nRow, aTempCell );
}

void Array::SetCellStyleRight( sal_Int32 nCol, sal_Int32 nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleRight" );
    const Cell* pTempCell(mxImpl->GetCell(nCol, nRow));
    if (pTempCell->GetStyleRight() == rStyle)
        return;
    Cell aTempCell(*pTempCell);
    aTempCell.SetStyleRight(rStyle);
    mxImpl->PutCell( nCol, nRow, aTempCell );
}

void Array::SetCellStyleTop( sal_Int32 nCol, sal_Int32 nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleTop" );
    const Cell* pTempCell(mxImpl->GetCell(nCol, nRow));
    if (pTempCell->GetStyleTop() == rStyle)
        return;
    Cell aTempCell(*pTempCell);
    aTempCell.SetStyleTop(rStyle);
    mxImpl->PutCell( nCol, nRow, aTempCell );
}

void Array::SetCellStyleBottom( sal_Int32 nCol, sal_Int32 nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleBottom" );
    const Cell* pTempCell(mxImpl->GetCell(nCol, nRow));
    if (pTempCell->GetStyleBottom() == rStyle)
        return;
    Cell aTempCell(*pTempCell);
    aTempCell.SetStyleBottom(rStyle);
    mxImpl->PutCell( nCol, nRow, aTempCell );
}

void Array::SetCellStyleTLBR( sal_Int32 nCol, sal_Int32 nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleTLBR" );
    const Cell* pTempCell(mxImpl->GetCell(nCol, nRow));
    if (pTempCell->GetStyleTLBR() == rStyle)
        return;
    Cell aTempCell(*pTempCell);
    aTempCell.SetStyleTLBR(rStyle);
    mxImpl->PutCell( nCol, nRow, aTempCell );
}

void Array::SetCellStyleBLTR( sal_Int32 nCol, sal_Int32 nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleBLTR" );
    const Cell* pTempCell(mxImpl->GetCell(nCol, nRow));
    if (pTempCell->GetStyleBLTR() == rStyle)
        return;
    Cell aTempCell(*pTempCell);
    aTempCell.SetStyleBLTR(rStyle);
    mxImpl->PutCell( nCol, nRow, aTempCell );
}

void Array::SetCellStyleDiag( sal_Int32 nCol, sal_Int32 nRow, const Style& rTLBR, const Style& rBLTR )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetCellStyleDiag" );
    const Cell* pTempCell(mxImpl->GetCell(nCol, nRow));
    if (pTempCell->GetStyleTLBR() == rTLBR && pTempCell->GetStyleBLTR() == rBLTR)
        return;
    Cell aTempCell(*pTempCell);
    aTempCell.SetStyleTLBR(rTLBR);
    aTempCell.SetStyleBLTR(rBLTR);
    mxImpl->PutCell( nCol, nRow, aTempCell );
}

void Array::SetColumnStyleLeft( sal_Int32 nCol, const Style& rStyle )
{
    DBG_FRAME_CHECK_COL( nCol, "SetColumnStyleLeft" );
    for( sal_Int32 nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
        SetCellStyleLeft( nCol, nRow, rStyle );
}

void Array::SetColumnStyleRight( sal_Int32 nCol, const Style& rStyle )
{
    DBG_FRAME_CHECK_COL( nCol, "SetColumnStyleRight" );
    for( sal_Int32 nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
        SetCellStyleRight( nCol, nRow, rStyle );
}

void Array::SetRowStyleTop( sal_Int32 nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_ROW( nRow, "SetRowStyleTop" );
    for( sal_Int32 nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        SetCellStyleTop( nCol, nRow, rStyle );
}

void Array::SetRowStyleBottom( sal_Int32 nRow, const Style& rStyle )
{
    DBG_FRAME_CHECK_ROW( nRow, "SetRowStyleBottom" );
    for( sal_Int32 nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        SetCellStyleBottom( nCol, nRow, rStyle );
}

void Array::SetCellRotation(sal_Int32 nCol, sal_Int32 nRow, SvxRotateMode eRotMode, double fOrientation)
{
    DBG_FRAME_CHECK_COLROW(nCol, nRow, "SetCellRotation");
    const Cell* pTempCell(mxImpl->GetCell(nCol, nRow));
    if (pTempCell->meRotMode == eRotMode && pTempCell->mfOrientation == fOrientation)
        return;
    Cell aTempCell(*pTempCell);
    aTempCell.meRotMode = eRotMode;
    aTempCell.mfOrientation = fOrientation;
    mxImpl->PutCell( nCol, nRow, aTempCell );

    if (!mxImpl->mbMayHaveCellRotation)
    {
        // activate once when a cell gets actually rotated to allow fast
        // answering HasCellRotation() calls
        mxImpl->mbMayHaveCellRotation = aTempCell.IsRotated();
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

const Style& Array::GetCellStyleLeft( sal_Int32 nCol, sal_Int32 nRow ) const
{
    // outside clipping rows or overlapped in merged cells: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) || mxImpl->IsMergedOverlappedLeft( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // left clipping border: always own left style
    if( nCol == mxImpl->mnFirstClipCol )
        return mxImpl->GetMergedOriginCell( nCol, nRow )->GetStyleLeft();
    // right clipping border: always right style of left neighbor cell
    if( nCol == mxImpl->mnLastClipCol + 1 )
        return mxImpl->GetMergedOriginCell( nCol - 1, nRow )->GetStyleRight();
    // outside clipping columns: invisible
    if( !mxImpl->IsColInClipRange( nCol ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own left style and right style of left neighbor cell
    return std::max( mxImpl->GetMergedOriginCell( nCol, nRow )->GetStyleLeft(), mxImpl->GetMergedOriginCell( nCol - 1, nRow )->GetStyleRight() );
}

const Style& Array::GetCellStyleRight( sal_Int32 nCol, sal_Int32 nRow ) const
{
    // outside clipping rows or overlapped in merged cells: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) || mxImpl->IsMergedOverlappedRight( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // left clipping border: always left style of right neighbor cell
    if( nCol + 1 == mxImpl->mnFirstClipCol )
        return mxImpl->GetMergedOriginCell( nCol + 1, nRow )->GetStyleLeft();
    // right clipping border: always own right style
    if( nCol == mxImpl->mnLastClipCol )
        return mxImpl->GetMergedLastCell( nCol, nRow )->GetStyleRight();
    // outside clipping columns: invisible
    if( !mxImpl->IsColInClipRange( nCol ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own right style and left style of right neighbor cell
    return std::max( mxImpl->GetMergedOriginCell( nCol, nRow )->GetStyleRight(), mxImpl->GetMergedOriginCell( nCol + 1, nRow )->GetStyleLeft() );
}

const Style& Array::GetCellStyleTop( sal_Int32 nCol, sal_Int32 nRow ) const
{
    // outside clipping columns or overlapped in merged cells: invisible
    if( !mxImpl->IsColInClipRange( nCol ) || mxImpl->IsMergedOverlappedTop( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // top clipping border: always own top style
    if( nRow == mxImpl->mnFirstClipRow )
        return mxImpl->GetMergedOriginCell( nCol, nRow )->GetStyleTop();
    // bottom clipping border: always bottom style of top neighbor cell
    if( nRow == mxImpl->mnLastClipRow + 1 )
        return mxImpl->GetMergedOriginCell( nCol, nRow - 1 )->GetStyleBottom();
    // outside clipping rows: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own top style and bottom style of top neighbor cell
    return std::max( mxImpl->GetMergedOriginCell( nCol, nRow )->GetStyleTop(), mxImpl->GetMergedOriginCell( nCol, nRow - 1 )->GetStyleBottom() );
}

const Style& Array::GetCellStyleBottom( sal_Int32 nCol, sal_Int32 nRow ) const
{
    // outside clipping columns or overlapped in merged cells: invisible
    if( !mxImpl->IsColInClipRange( nCol ) || mxImpl->IsMergedOverlappedBottom( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // top clipping border: always top style of bottom neighbor cell
    if( nRow + 1 == mxImpl->mnFirstClipRow )
        return mxImpl->GetMergedOriginCell( nCol, nRow + 1 )->GetStyleTop();
    // bottom clipping border: always own bottom style
    if( nRow == mxImpl->mnLastClipRow )
        return mxImpl->GetMergedLastCell( nCol, nRow )->GetStyleBottom();
    // outside clipping rows: invisible
    if( !mxImpl->IsRowInClipRange( nRow ) )
        return OBJ_STYLE_NONE;
    // inside clipping range: maximum of own bottom style and top style of bottom neighbor cell
    return std::max( mxImpl->GetMergedOriginCell( nCol, nRow )->GetStyleBottom(), mxImpl->GetMergedOriginCell( nCol, nRow + 1 )->GetStyleTop() );
}

const Style& Array::GetCellStyleTLBR( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return mxImpl->GetCell( nCol, nRow )->GetStyleTLBR();
}

const Style& Array::GetCellStyleBLTR( sal_Int32 nCol, sal_Int32 nRow ) const
{
    return mxImpl->GetCell( nCol, nRow )->GetStyleBLTR();
}

const Style& Array::GetCellStyleTL( sal_Int32 nCol, sal_Int32 nRow ) const
{
    // not in clipping range: always invisible
    if( !mxImpl->IsInClipRange( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // return style only for top-left cell
    sal_Int32 nFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
    sal_Int32 nFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
    return ((nCol == nFirstCol) && (nRow == nFirstRow)) ?
        mxImpl->GetCell( nFirstCol, nFirstRow )->GetStyleTLBR() : OBJ_STYLE_NONE;
}

const Style& Array::GetCellStyleBR( sal_Int32 nCol, sal_Int32 nRow ) const
{
    // not in clipping range: always invisible
    if( !mxImpl->IsInClipRange( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // return style only for bottom-right cell
    sal_Int32 nLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
    sal_Int32 nLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
    return ((nCol == nLastCol) && (nRow == nLastRow)) ?
        mxImpl->GetCell( mxImpl->GetMergedFirstCol( nCol, nRow ), mxImpl->GetMergedFirstRow( nCol, nRow ) )->GetStyleTLBR() : OBJ_STYLE_NONE;
}

const Style& Array::GetCellStyleBL( sal_Int32 nCol, sal_Int32 nRow ) const
{
    // not in clipping range: always invisible
    if( !mxImpl->IsInClipRange( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // return style only for bottom-left cell
    sal_Int32 nFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
    sal_Int32 nLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
    return ((nCol == nFirstCol) && (nRow == nLastRow)) ?
        mxImpl->GetCell( nFirstCol, mxImpl->GetMergedFirstRow( nCol, nRow ) )->GetStyleBLTR() : OBJ_STYLE_NONE;
}

const Style& Array::GetCellStyleTR( sal_Int32 nCol, sal_Int32 nRow ) const
{
    // not in clipping range: always invisible
    if( !mxImpl->IsInClipRange( nCol, nRow ) )
        return OBJ_STYLE_NONE;
    // return style only for top-right cell
    sal_Int32 nFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
    sal_Int32 nLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
    return ((nCol == nLastCol) && (nRow == nFirstRow)) ?
        mxImpl->GetCell( mxImpl->GetMergedFirstCol( nCol, nRow ), nFirstRow )->GetStyleBLTR() : OBJ_STYLE_NONE;
}

// cell merging
void Array::SetMergedRange( sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow )
{
    DBG_FRAME_CHECK_COLROW( nFirstCol, nFirstRow, "SetMergedRange" );
    DBG_FRAME_CHECK_COLROW( nLastCol, nLastRow, "SetMergedRange" );
#if OSL_DEBUG_LEVEL >= 2
    {
        bool bFound = false;
        for( sal_Int32 nCurrCol = nFirstCol; !bFound && (nCurrCol <= nLastCol); ++nCurrCol )
            for( sal_Int32 nCurrRow = nFirstRow; !bFound && (nCurrRow <= nLastRow); ++nCurrRow )
                bFound = mxImpl->GetCell( nCurrCol, nCurrRow )->IsMerged();
        DBG_FRAME_CHECK( !bFound, "SetMergedRange", "overlapping merged ranges" );
    }
#endif
    if( mxImpl->IsValidPos( nFirstCol, nFirstRow ) && mxImpl->IsValidPos( nLastCol, nLastRow ) )
        lclSetMergedRange( *mxImpl, mxImpl->maCells, mxImpl->mnWidth, nFirstCol, nFirstRow, nLastCol, nLastRow );
}

void Array::SetAddMergedLeftSize( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nAddSize )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetAddMergedLeftSize" );
    DBG_FRAME_CHECK( mxImpl->GetMergedFirstCol( nCol, nRow ) == 0, "SetAddMergedLeftSize", "additional border inside array" );
    for( MergedCellIterator aIt( *this, nCol, nRow ); aIt.Is(); ++aIt )
    {
        const Cell* pTempCell(mxImpl->GetCell(aIt.Col(), aIt.Row()));
        if (pTempCell->mnAddLeft == nAddSize)
            return;
        Cell aTempCell(*pTempCell);
        aTempCell.mnAddLeft = nAddSize;
        mxImpl->PutCell( aIt.Col(), aIt.Row(), aTempCell );
    }
}

void Array::SetAddMergedRightSize( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nAddSize )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetAddMergedRightSize" );
    DBG_FRAME_CHECK( mxImpl->GetMergedLastCol( nCol, nRow ) + 1 == mxImpl->mnWidth, "SetAddMergedRightSize", "additional border inside array" );
    for( MergedCellIterator aIt( *this, nCol, nRow ); aIt.Is(); ++aIt )
    {
        const Cell* pTempCell(mxImpl->GetCell(aIt.Col(), aIt.Row()));
        if (pTempCell->mnAddRight == nAddSize)
            return;
        Cell aTempCell(*pTempCell);
        aTempCell.mnAddRight = nAddSize;
        mxImpl->PutCell( aIt.Col(), aIt.Row(), aTempCell );
    }
}

void Array::SetAddMergedTopSize( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nAddSize )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetAddMergedTopSize" );
    DBG_FRAME_CHECK( mxImpl->GetMergedFirstRow( nCol, nRow ) == 0, "SetAddMergedTopSize", "additional border inside array" );
    for( MergedCellIterator aIt( *this, nCol, nRow ); aIt.Is(); ++aIt )
    {
        const Cell* pTempCell(mxImpl->GetCell(aIt.Col(), aIt.Row()));
        if (pTempCell->mnAddTop == nAddSize)
            return;
        Cell aTempCell(*pTempCell);
        aTempCell.mnAddTop = nAddSize;
        mxImpl->PutCell( aIt.Col(), aIt.Row(), aTempCell );
    }
}

void Array::SetAddMergedBottomSize( sal_Int32 nCol, sal_Int32 nRow, sal_Int32 nAddSize )
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "SetAddMergedBottomSize" );
    DBG_FRAME_CHECK( mxImpl->GetMergedLastRow( nCol, nRow ) + 1 == mxImpl->mnHeight, "SetAddMergedBottomSize", "additional border inside array" );
    for( MergedCellIterator aIt( *this, nCol, nRow ); aIt.Is(); ++aIt )
    {
        const Cell* pTempCell(mxImpl->GetCell(aIt.Col(), aIt.Row()));
        if (pTempCell->mnAddBottom == nAddSize)
            return;
        Cell aTempCell(*pTempCell);
        aTempCell.mnAddBottom = nAddSize;
        mxImpl->PutCell( aIt.Col(), aIt.Row(), aTempCell );
    }
}

bool Array::IsMerged( sal_Int32 nCol, sal_Int32 nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "IsMerged" );
    return mxImpl->GetCell( nCol, nRow )->IsMerged();
}

void Array::GetMergedOrigin( sal_Int32& rnFirstCol, sal_Int32& rnFirstRow, sal_Int32 nCol, sal_Int32 nRow ) const
{
    DBG_FRAME_CHECK_COLROW( nCol, nRow, "GetMergedOrigin" );
    rnFirstCol = mxImpl->GetMergedFirstCol( nCol, nRow );
    rnFirstRow = mxImpl->GetMergedFirstRow( nCol, nRow );
}

void Array::GetMergedRange( sal_Int32& rnFirstCol, sal_Int32& rnFirstRow,
        sal_Int32& rnLastCol, sal_Int32& rnLastRow, sal_Int32 nCol, sal_Int32 nRow ) const
{
    GetMergedOrigin( rnFirstCol, rnFirstRow, nCol, nRow );
    rnLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
    rnLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
}

// clipping
void Array::SetClipRange( sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow )
{
    DBG_FRAME_CHECK_COLROW( nFirstCol, nFirstRow, "SetClipRange" );
    DBG_FRAME_CHECK_COLROW( nLastCol, nLastRow, "SetClipRange" );
    mxImpl->mnFirstClipCol = nFirstCol;
    mxImpl->mnFirstClipRow = nFirstRow;
    mxImpl->mnLastClipCol = nLastCol;
    mxImpl->mnLastClipRow = nLastRow;
}

// cell coordinates
void Array::SetXOffset( sal_Int32 nXOffset )
{
    mxImpl->maXCoords[ 0 ] = nXOffset;
    mxImpl->mbXCoordsDirty = true;
}

void Array::SetYOffset( sal_Int32 nYOffset )
{
    mxImpl->maYCoords[ 0 ] = nYOffset;
    mxImpl->mbYCoordsDirty = true;
}

void Array::SetColWidth( sal_Int32 nCol, sal_Int32 nWidth )
{
    DBG_FRAME_CHECK_COL( nCol, "SetColWidth" );
    mxImpl->maWidths[ nCol ] = nWidth;
    mxImpl->mbXCoordsDirty = true;
}

void Array::SetRowHeight( sal_Int32 nRow, sal_Int32 nHeight )
{
    DBG_FRAME_CHECK_ROW( nRow, "SetRowHeight" );
    mxImpl->maHeights[ nRow ] = nHeight;
    mxImpl->mbYCoordsDirty = true;
}

void Array::SetAllColWidths( sal_Int32 nWidth )
{
    std::fill( mxImpl->maWidths.begin(), mxImpl->maWidths.end(), nWidth );
    mxImpl->mbXCoordsDirty = true;
}

void Array::SetAllRowHeights( sal_Int32 nHeight )
{
    std::fill( mxImpl->maHeights.begin(), mxImpl->maHeights.end(), nHeight );
    mxImpl->mbYCoordsDirty = true;
}

sal_Int32 Array::GetColPosition( sal_Int32 nCol ) const
{
    DBG_FRAME_CHECK_COL_1( nCol, "GetColPosition" );
    return mxImpl->GetColPosition( nCol );
}

sal_Int32 Array::GetRowPosition( sal_Int32 nRow ) const
{
    DBG_FRAME_CHECK_ROW_1( nRow, "GetRowPosition" );
    return mxImpl->GetRowPosition( nRow );
}

sal_Int32 Array::GetColWidth( sal_Int32 nFirstCol, sal_Int32 nLastCol ) const
{
    DBG_FRAME_CHECK_COL( nFirstCol, "GetColWidth" );
    DBG_FRAME_CHECK_COL( nLastCol, "GetColWidth" );
    return GetColPosition( nLastCol + 1 ) - GetColPosition( nFirstCol );
}

sal_Int32 Array::GetRowHeight( sal_Int32 nFirstRow, sal_Int32 nLastRow ) const
{
    DBG_FRAME_CHECK_ROW( nFirstRow, "GetRowHeight" );
    DBG_FRAME_CHECK_ROW( nLastRow, "GetRowHeight" );
    return GetRowPosition( nLastRow + 1 ) - GetRowPosition( nFirstRow );
}

sal_Int32 Array::GetWidth() const
{
    return GetColPosition( mxImpl->mnWidth ) - GetColPosition( 0 );
}

sal_Int32 Array::GetHeight() const
{
    return GetRowPosition( mxImpl->mnHeight ) - GetRowPosition( 0 );
}

basegfx::B2DRange Array::GetCellRange( sal_Int32 nCol, sal_Int32 nRow ) const
{
    // get the Range of the fully expanded cell (if merged)
    const sal_Int32 nFirstCol(mxImpl->GetMergedFirstCol( nCol, nRow ));
    const sal_Int32 nFirstRow(mxImpl->GetMergedFirstRow( nCol, nRow ));
    const sal_Int32 nLastCol(mxImpl->GetMergedLastCol( nCol, nRow ));
    const sal_Int32 nLastRow(mxImpl->GetMergedLastRow( nCol, nRow ));
    const Point aPoint( GetColPosition( nFirstCol ), GetRowPosition( nFirstRow ) );
    const Size aSize( GetColWidth( nFirstCol, nLastCol ) + 1, GetRowHeight( nFirstRow, nLastRow ) + 1 );
    tools::Rectangle aRect(aPoint, aSize);

    // adjust rectangle for partly visible merged cells
    const Cell* pCell(mxImpl->GetCell( nCol, nRow ));

    if( pCell->IsMerged() )
    {
        // not *sure* what exactly this is good for,
        // it is just a hard set extension at merged cells,
        // probably *should* be included in the above extended
        // GetColPosition/GetColWidth already. This might be
        // added due to GetColPosition/GetColWidth not working
        // correctly over PageChanges (if used), but not sure.
        aRect.AdjustLeft( -(pCell->mnAddLeft) );
        aRect.AdjustRight(pCell->mnAddRight );
        aRect.AdjustTop( -(pCell->mnAddTop) );
        aRect.AdjustBottom(pCell->mnAddBottom );
    }

    return vcl::unotools::b2DRectangleFromRectangle(aRect);
}

// return output range of given row/col range in logical coordinates
basegfx::B2DRange Array::GetB2DRange(sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow) const
{
    const Point aPoint( GetColPosition( nFirstCol ), GetRowPosition( nFirstRow ) );
    const Size aSize( GetColWidth( nFirstCol, nLastCol ) + 1, GetRowHeight( nFirstRow, nLastRow ) + 1 );

    return vcl::unotools::b2DRectangleFromRectangle(tools::Rectangle(aPoint, aSize));
}

// mirroring
void Array::MirrorSelfX()
{
    CellVec aNewCells;
    aNewCells.reserve( GetCellCount() );

    sal_Int32 nCol, nRow;
    for( nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
    {
        for( nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        {
            Cell aTempCell(*mxImpl->GetCell(mxImpl->GetMirrorCol( nCol ), nRow));
            aTempCell.MirrorSelfX();
            aNewCells.push_back( mxImpl->createOrFind(aTempCell) );
        }
    }
    for( nRow = 0; nRow < mxImpl->mnHeight; ++nRow )
    {
        for( nCol = 0; nCol < mxImpl->mnWidth; ++nCol )
        {
            if( mxImpl->GetCell( nCol, nRow )->mbMergeOrig )
            {
                sal_Int32 nLastCol = mxImpl->GetMergedLastCol( nCol, nRow );
                sal_Int32 nLastRow = mxImpl->GetMergedLastRow( nCol, nRow );
                lclSetMergedRange( *mxImpl, aNewCells, mxImpl->mnWidth,
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
static void HelperCreateHorizontalEntry(
    const Array& rArray,
    const Style& rStyle,
    sal_Int32 col,
    sal_Int32 row,
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rX,
    const basegfx::B2DVector& rY,
    drawinglayer::primitive2d::SdrFrameBorderDataVector& rData,
    bool bUpper,
    const Color* pForceColor)
{
    // prepare SdrFrameBorderData
    rData.emplace_back(
        bUpper ? rOrigin : basegfx::B2DPoint(rOrigin + rY),
        rX,
        rStyle,
        pForceColor);
    drawinglayer::primitive2d::SdrFrameBorderData& rInstance(rData.back());

    // get involved styles at start
    const Style& rStartFromTR(rArray.GetCellStyleBL( col, row - 1 ));
    const Style& rStartLFromT(rArray.GetCellStyleLeft( col, row - 1 ));
    const Style& rStartLFromL(rArray.GetCellStyleTop( col - 1, row ));
    const Style& rStartLFromB(rArray.GetCellStyleLeft( col, row ));
    const Style& rStartFromBR(rArray.GetCellStyleTL( col, row ));

    rInstance.addSdrConnectStyleData(true, rStartFromTR, rX - rY, false);
    rInstance.addSdrConnectStyleData(true, rStartLFromT, -rY, true);
    rInstance.addSdrConnectStyleData(true, rStartLFromL, -rX, true);
    rInstance.addSdrConnectStyleData(true, rStartLFromB, rY, false);
    rInstance.addSdrConnectStyleData(true, rStartFromBR, rX + rY, false);

    // get involved styles at end
    const Style& rEndFromTL(rArray.GetCellStyleBR( col, row - 1 ));
    const Style& rEndRFromT(rArray.GetCellStyleRight( col, row - 1 ));
    const Style& rEndRFromR(rArray.GetCellStyleTop( col + 1, row ));
    const Style& rEndRFromB(rArray.GetCellStyleRight( col, row ));
    const Style& rEndFromBL(rArray.GetCellStyleTR( col, row ));

    rInstance.addSdrConnectStyleData(false, rEndFromTL, -rX - rY, true);
    rInstance.addSdrConnectStyleData(false, rEndRFromT, -rY, true);
    rInstance.addSdrConnectStyleData(false, rEndRFromR, rX, false);
    rInstance.addSdrConnectStyleData(false, rEndRFromB, rY, false);
    rInstance.addSdrConnectStyleData(false, rEndFromBL, rY - rX, true);
}

static void HelperCreateVerticalEntry(
    const Array& rArray,
    const Style& rStyle,
    sal_Int32 col,
    sal_Int32 row,
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rX,
    const basegfx::B2DVector& rY,
    drawinglayer::primitive2d::SdrFrameBorderDataVector& rData,
    bool bLeft,
    const Color* pForceColor)
{
    // prepare SdrFrameBorderData
    rData.emplace_back(
        bLeft ? rOrigin : basegfx::B2DPoint(rOrigin + rX),
        rY,
        rStyle,
        pForceColor);
    drawinglayer::primitive2d::SdrFrameBorderData& rInstance(rData.back());

    // get involved styles at start
    const Style& rStartFromBL(rArray.GetCellStyleTR( col - 1, row ));
    const Style& rStartTFromL(rArray.GetCellStyleTop( col - 1, row ));
    const Style& rStartTFromT(rArray.GetCellStyleLeft( col, row - 1 ));
    const Style& rStartTFromR(rArray.GetCellStyleTop( col, row ));
    const Style& rStartFromBR(rArray.GetCellStyleTL( col, row ));

    rInstance.addSdrConnectStyleData(true, rStartFromBR, rX + rY, false);
    rInstance.addSdrConnectStyleData(true, rStartTFromR, rX, false);
    rInstance.addSdrConnectStyleData(true, rStartTFromT, -rY, true);
    rInstance.addSdrConnectStyleData(true, rStartTFromL, -rX, true);
    rInstance.addSdrConnectStyleData(true, rStartFromBL, rY - rX, true);

    // get involved styles at end
    const Style& rEndFromTL(rArray.GetCellStyleBR( col - 1, row ));
    const Style& rEndBFromL(rArray.GetCellStyleBottom( col - 1, row ));
    const Style& rEndBFromB(rArray.GetCellStyleLeft( col, row + 1 ));
    const Style& rEndBFromR(rArray.GetCellStyleBottom( col, row ));
    const Style& rEndFromTR(rArray.GetCellStyleBL( col, row ));

    rInstance.addSdrConnectStyleData(false, rEndFromTR, rX - rY, false);
    rInstance.addSdrConnectStyleData(false, rEndBFromR, rX, false);
    rInstance.addSdrConnectStyleData(false, rEndBFromB, rY, false);
    rInstance.addSdrConnectStyleData(false, rEndBFromL, -rX, true);
    rInstance.addSdrConnectStyleData(false, rEndFromTL, -rY - rX, true);
}

static void HelperClipLine(
    basegfx::B2DPoint& rStart,
    basegfx::B2DVector& rDirection,
    const basegfx::B2DRange& rClipRange)
{
    basegfx::B2DPolygon aLine({rStart, rStart + rDirection});
    const basegfx::B2DPolyPolygon aResultPP(
        basegfx::utils::clipPolygonOnRange(
            aLine,
            rClipRange,
            true, // bInside
            true)); // bStroke

    if(aResultPP.count() > 0)
    {
        const basegfx::B2DPolygon aResultP(aResultPP.getB2DPolygon(0));

        if(aResultP.count() > 0)
        {
            const basegfx::B2DPoint aResultStart(aResultP.getB2DPoint(0));
            const basegfx::B2DPoint aResultEnd(aResultP.getB2DPoint(aResultP.count() - 1));

            if(aResultStart != aResultEnd)
            {
                rStart = aResultStart;
                rDirection = aResultEnd - aResultStart;
            }
        }
    }
}

static void HelperCreateTLBREntry(
    const Array& rArray,
    const Style& rStyle,
    drawinglayer::primitive2d::SdrFrameBorderDataVector& rData,
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rX,
    const basegfx::B2DVector& rY,
    sal_Int32 nColLeft,
    sal_Int32 nColRight,
    sal_Int32 nRowTop,
    sal_Int32 nRowBottom,
    const Color* pForceColor,
    const basegfx::B2DRange* pClipRange)
{
    if(rStyle.IsUsed())
    {
        /// prepare geometry line data
        basegfx::B2DPoint aStart(rOrigin);
        basegfx::B2DVector aDirection(rX + rY);

        /// check if we need to clip geometry line data and do it
        if(nullptr != pClipRange)
        {
            HelperClipLine(aStart, aDirection, *pClipRange);
        }

        /// top-left and bottom-right Style Tables
        rData.emplace_back(
            aStart,
            aDirection,
            rStyle,
            pForceColor);
        drawinglayer::primitive2d::SdrFrameBorderData& rInstance(rData.back());

        /// Fill top-left Style Table
        const Style& rTLFromRight(rArray.GetCellStyleTop(nColLeft, nRowTop));
        const Style& rTLFromBottom(rArray.GetCellStyleLeft(nColLeft, nRowTop));

        rInstance.addSdrConnectStyleData(true, rTLFromRight, rX, false);
        rInstance.addSdrConnectStyleData(true, rTLFromBottom, rY, false);

        /// Fill bottom-right Style Table
        const Style& rBRFromBottom(rArray.GetCellStyleRight(nColRight, nRowBottom));
        const Style& rBRFromLeft(rArray.GetCellStyleBottom(nColRight, nRowBottom));

        rInstance.addSdrConnectStyleData(false, rBRFromBottom, -rY, true);
        rInstance.addSdrConnectStyleData(false, rBRFromLeft, -rX, true);
    }
}

static void HelperCreateBLTREntry(
    const Array& rArray,
    const Style& rStyle,
    drawinglayer::primitive2d::SdrFrameBorderDataVector& rData,
    const basegfx::B2DPoint& rOrigin,
    const basegfx::B2DVector& rX,
    const basegfx::B2DVector& rY,
    sal_Int32 nColLeft,
    sal_Int32 nColRight,
    sal_Int32 nRowTop,
    sal_Int32 nRowBottom,
    const Color* pForceColor,
    const basegfx::B2DRange* pClipRange)
{
    if(rStyle.IsUsed())
    {
        /// prepare geometry line data
        basegfx::B2DPoint aStart(rOrigin + rY);
        basegfx::B2DVector aDirection(rX - rY);

        /// check if we need to clip geometry line data and do it
        if(nullptr != pClipRange)
        {
            HelperClipLine(aStart, aDirection, *pClipRange);
        }

        /// bottom-left and top-right Style Tables
        rData.emplace_back(
            aStart,
            aDirection,
            rStyle,
            pForceColor);
        drawinglayer::primitive2d::SdrFrameBorderData& rInstance(rData.back());

        /// Fill bottom-left Style Table
        const Style& rBLFromTop(rArray.GetCellStyleLeft(nColLeft, nRowBottom));
        const Style& rBLFromBottom(rArray.GetCellStyleBottom(nColLeft, nRowBottom));

        rInstance.addSdrConnectStyleData(true, rBLFromTop, -rY, true);
        rInstance.addSdrConnectStyleData(true, rBLFromBottom, rX, false);

        /// Fill top-right Style Table
        const Style& rTRFromLeft(rArray.GetCellStyleTop(nColRight, nRowTop));
        const Style& rTRFromBottom(rArray.GetCellStyleRight(nColRight, nRowTop));

        rInstance.addSdrConnectStyleData(false, rTRFromLeft, -rX, true);
        rInstance.addSdrConnectStyleData(false, rTRFromBottom, rY, false);
    }
}

drawinglayer::primitive2d::Primitive2DContainer Array::CreateB2DPrimitiveRange(
    sal_Int32 nFirstCol, sal_Int32 nFirstRow, sal_Int32 nLastCol, sal_Int32 nLastRow,
    const Color* pForceColor ) const
{
    DBG_FRAME_CHECK_COLROW( nFirstCol, nFirstRow, "CreateB2DPrimitiveRange" );
    DBG_FRAME_CHECK_COLROW( nLastCol, nLastRow, "CreateB2DPrimitiveRange" );

#ifdef OPTICAL_CHECK_CLIPRANGE_FOR_MERGED_CELL
    std::vector<basegfx::B2DRange> aClipRanges;
#endif

    // It may be necessary to extend the loop ranges by one cell to the outside,
    // when possible. This is needed e.g. when there is in Calc a Cell with an
    // upper CellBorder using DoubleLine and that is right/left connected upwards
    // to also DoubleLine. These upper DoubleLines will be extended to meet the
    // lower of the upper CellBorder and thus have graphical parts that are
    // displayed one cell below and right/left of the target cell - analog to
    // other examples in all other directions.
    // It would be possible to explicitly test this (if possible by indices at all)
    // looping and testing the styles in the outer cells to detect this, but since
    // for other usages (e.g. UI) usually nFirstRow==0 and nLastRow==GetRowCount()-1
    // (and analog for Col) it is okay to just expand the range when available.
    // Do *not* change nFirstRow/nLastRow due to these needed to the boolean tests
    // below (!)
    // Checked usages, this method is used in Calc EditView/Print/Export stuff and
    // in UI (Dialog), not for Writer Tables and Draw/Impress tables. All usages
    // seem okay with this change, so I will add it.
    const sal_Int32 nStartRow(nFirstRow > 0 ? nFirstRow - 1 : nFirstRow);
    const sal_Int32 nEndRow(nLastRow < GetRowCount() - 1 ? nLastRow + 1 : nLastRow);
    const sal_Int32 nStartCol(nFirstCol > 0 ? nFirstCol - 1 : nFirstCol);
    const sal_Int32 nEndCol(nLastCol < GetColCount() - 1 ? nLastCol + 1 : nLastCol);

    // prepare SdrFrameBorderDataVector
    drawinglayer::primitive2d::SdrFrameBorderDataVector aData;

    // remember for which merged cells crossed lines were already created. To
    // do so, hold the sal_Int32 cell index in a set for fast check
    std::unordered_set< sal_Int32 > aMergedCells;

    for (sal_Int32 nRow(nStartRow); nRow <= nEndRow; ++nRow)
    {
        for (sal_Int32 nCol(nStartCol); nCol <= nEndCol; ++nCol)
        {
            // get Cell and CoordinateSystem (*only* for this Cell, do *not* expand for
            // merged cells (!)), check if used (non-empty vectors)
            const Cell* pCell(mxImpl->GetCell(nCol, nRow));
            basegfx::B2DHomMatrix aCoordinateSystem(pCell->CreateCoordinateSystemSingleCell(*this, nCol, nRow));
            basegfx::B2DVector aX(basegfx::utils::getColumn(aCoordinateSystem, 0));
            basegfx::B2DVector aY(basegfx::utils::getColumn(aCoordinateSystem, 1));

            // get needed local values
            basegfx::B2DPoint aOrigin(basegfx::utils::getColumn(aCoordinateSystem, 2));
            const bool bOverlapX(pCell->mbOverlapX);
            const bool bFirstCol(nCol == nFirstCol);

            // handle rotation: If cell is rotated, handle lower/right edge inside
            // this local geometry due to the created CoordinateSystem already representing
            // the needed transformations.
            const bool bRotated(pCell->IsRotated());

            // Additionally avoid double-handling by suppressing handling when self not rotated,
            // but above/left is rotated and thus already handled. Two directly connected
            // rotated will paint/create both edges, they might be rotated differently.
            const bool bSuppressLeft(!bRotated && nCol > nFirstCol && mxImpl->GetCell(nCol - 1, nRow)->IsRotated());
            const bool bSuppressAbove(!bRotated && nRow > nFirstRow && mxImpl->GetCell(nCol, nRow - 1)->IsRotated());

            if(!aX.equalZero() && !aY.equalZero())
            {
                // additionally needed local values
                const bool bOverlapY(pCell->mbOverlapY);
                const bool bLastCol(nCol == nLastCol);
                const bool bFirstRow(nRow == nFirstRow);
                const bool bLastRow(nRow == nLastRow);

                // create upper line for this Cell
                if ((!bOverlapY         // true for first line in merged cells or cells
                    || bFirstRow)       // true for non_Calc usages of this tooling
                    && !bSuppressAbove) // true when above is not rotated, so edge is already handled (see bRotated)
                {
                    // get CellStyle - method will take care to get the correct one, e.g.
                    // for merged cells (it uses mxImpl->GetMergedOriginCell that works with topLeft's of these)
                    const Style& rTop(GetCellStyleTop(nCol, nRow));

                    if(rTop.IsUsed())
                    {
                        HelperCreateHorizontalEntry(*this, rTop, nCol, nRow, aOrigin, aX, aY, aData, true, pForceColor);
                    }
                }

                // create lower line for this Cell
                if (bLastRow       // true for non_Calc usages of this tooling
                    || bRotated)   // true if cell is rotated, handle lower edge in local geometry
                {
                    const Style& rBottom(GetCellStyleBottom(nCol, nRow));

                    if(rBottom.IsUsed())
                    {
                        HelperCreateHorizontalEntry(*this, rBottom, nCol, nRow + 1, aOrigin, aX, aY, aData, false, pForceColor);
                    }
                }

                // create left line for this Cell
                if ((!bOverlapX         // true for first column in merged cells or cells
                    || bFirstCol)       // true for non_Calc usages of this tooling
                    && !bSuppressLeft)  // true when left is not rotated, so edge is already handled (see bRotated)
                {
                    const Style& rLeft(GetCellStyleLeft(nCol, nRow));

                    if(rLeft.IsUsed())
                    {
                        HelperCreateVerticalEntry(*this, rLeft, nCol, nRow, aOrigin, aX, aY, aData, true, pForceColor);
                    }
                }

                // create right line for this Cell
                if (bLastCol        // true for non_Calc usages of this tooling
                    || bRotated)    // true if cell is rotated, handle right edge in local geometry
                {
                    const Style& rRight(GetCellStyleRight(nCol, nRow));

                    if(rRight.IsUsed())
                    {
                        HelperCreateVerticalEntry(*this, rRight, nCol + 1, nRow, aOrigin, aX, aY, aData, false, pForceColor);
                    }
                }

                // tdf#126269 check for crossed lines, these need special treatment, especially
                // for merged cells (see comments in task). Separate treatment of merged and
                // non-merged cells to allow better handling of both types
                if(pCell->IsMerged())
                {
                    // first check if this merged cell was already handled. To do so,
                    // calculate and use the index of the TopLeft cell
                    sal_Int32 nColLeft(nCol), nRowTop(nRow), nColRight(nCol), nRowBottom(nRow);
                    GetMergedRange(nColLeft, nRowTop, nColRight, nRowBottom, nCol, nRow);
                    const sal_Int32 nIndexOfMergedCell(mxImpl->GetIndex(nColLeft, nRowTop));

                    auto aItInsertedPair = aMergedCells.insert(nIndexOfMergedCell);
                    if(aItInsertedPair.second)
                    {
                        // not found, so not yet handled.

                        // Get and check if diagonal styles are used
                        // Note: For GetCellStyleBLTR below I tried to use nRowBottom
                        //       as Y-value what seemed more logical, but that
                        //       is wrong. Despite defining a line starting at
                        //       bottom-left, the Style is defined in the cell at top-left
                        const Style& rTLBR(GetCellStyleTLBR(nColLeft, nRowTop));
                        const Style& rBLTR(GetCellStyleBLTR(nColLeft, nRowTop));

                        if(rTLBR.IsUsed() || rBLTR.IsUsed())
                        {
                            // test if merged cell overlaps ClipRange at all (needs visualization)
                            if(mxImpl->OverlapsClipRange(nColLeft, nRowTop, nColRight, nRowBottom))
                            {
                                // when merged, get extended coordinate system and derived values
                                // for the full range of this merged cell. Only work with rMergedCell
                                // (which is the top-left single cell of the merged cell) from here on
                                aCoordinateSystem = mxImpl->GetCell(nColLeft, nRowTop)->CreateCoordinateSystemMergedCell(
                                    *this, nColLeft, nRowTop, nColRight, nRowBottom);
                                aX = basegfx::utils::getColumn(aCoordinateSystem, 0);
                                aY = basegfx::utils::getColumn(aCoordinateSystem, 1);
                                aOrigin = basegfx::utils::getColumn(aCoordinateSystem, 2);

                                // check if clip is needed
                                basegfx::B2DRange aClipRange;

                                // first use row/col ClipTest for raw check
                                bool bNeedToClip(
                                    !mxImpl->IsColInClipRange(nColLeft) ||
                                    !mxImpl->IsRowInClipRange(nRowTop) ||
                                    !mxImpl->IsColInClipRange(nColRight) ||
                                    !mxImpl->IsRowInClipRange(nRowBottom));

                                if(bNeedToClip)
                                {
                                    // now get ClipRange and CellRange in logical coordinates
                                    aClipRange = GetB2DRange(
                                        mxImpl->mnFirstClipCol, mxImpl->mnFirstClipRow,
                                        mxImpl->mnLastClipCol, mxImpl->mnLastClipRow);

                                    basegfx::B2DRange aCellRange(
                                        GetB2DRange(
                                            nColLeft, nRowTop,
                                            nColRight, nRowBottom));

                                    // intersect these to get the target ClipRange, ensure
                                    // that clip is needed
                                    aClipRange.intersect(aCellRange);
                                    bNeedToClip = !aClipRange.isEmpty();

#ifdef OPTICAL_CHECK_CLIPRANGE_FOR_MERGED_CELL
                                    aClipRanges.push_back(aClipRange);
#endif
                                }

                                // create top-left to bottom-right geometry
                                HelperCreateTLBREntry(*this, rTLBR, aData, aOrigin, aX, aY,
                                    nColLeft, nRowTop, nColRight, nRowBottom, pForceColor,
                                    bNeedToClip ? &aClipRange : nullptr);

                                // create bottom-left to top-right geometry
                                HelperCreateBLTREntry(*this, rBLTR, aData, aOrigin, aX, aY,
                                    nColLeft, nRowTop, nColRight, nRowBottom, pForceColor,
                                    bNeedToClip ? &aClipRange : nullptr);
                            }
                        }
                    }
                }
                else
                {
                    // must be in clipping range: else not visible. This
                    // already clips completely for non-merged cells
                    if( mxImpl->IsInClipRange( nCol, nRow ) )
                    {
                        // get and check if diagonal styles are used
                        const Style& rTLBR(GetCellStyleTLBR(nCol, nRow));
                        const Style& rBLTR(GetCellStyleBLTR(nCol, nRow));

                        if(rTLBR.IsUsed() || rBLTR.IsUsed())
                        {
                            HelperCreateTLBREntry(*this, rTLBR, aData, aOrigin, aX, aY,
                                nCol, nRow, nCol, nRow, pForceColor, nullptr);

                            HelperCreateBLTREntry(*this, rBLTR, aData, aOrigin, aX, aY,
                                nCol, nRow, nCol, nRow, pForceColor, nullptr);
                        }
                    }
                }
            }
            else if(!aY.equalZero())
            {
                // cell has height, but no width. Create left vertical line for this Cell
                if ((!bOverlapX         // true for first column in merged cells or cells
                    || bFirstCol)       // true for non_Calc usages of this tooling
                    && !bSuppressLeft)  // true when left is not rotated, so edge is already handled (see bRotated)
                {
                    const Style& rLeft(GetCellStyleLeft(nCol, nRow));

                    if (rLeft.IsUsed())
                    {
                        HelperCreateVerticalEntry(*this, rLeft, nCol, nRow, aOrigin, aX, aY, aData, true, pForceColor);
                    }
                }
            }
            else
            {
                // Cell has *no* size, thus no visualization
            }
        }
    }

    // create instance of SdrFrameBorderPrimitive2D if
    // SdrFrameBorderDataVector is used
    drawinglayer::primitive2d::Primitive2DContainer aSequence;

    if(!aData.empty())
    {
        aSequence.append(
            drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::SdrFrameBorderPrimitive2D(
                    std::move(aData),
                    true)));    // force visualization to minimal one discrete unit (pixel)
    }

#ifdef OPTICAL_CHECK_CLIPRANGE_FOR_MERGED_CELL
    for(auto const& rClipRange : aClipRanges)
    {
        // draw ClipRange in yellow to allow simple interactive optical control in office
        aSequence.append(
            drawinglayer::primitive2d::Primitive2DReference(
                new drawinglayer::primitive2d::PolygonHairlinePrimitive2D(
                    basegfx::utils::createPolygonFromRect(rClipRange),
                    basegfx::BColor(1.0, 1.0, 0.0))));
    }
#endif

    return aSequence;
}

drawinglayer::primitive2d::Primitive2DContainer Array::CreateB2DPrimitiveArray() const
{
    drawinglayer::primitive2d::Primitive2DContainer aPrimitives;

    if (mxImpl->mnWidth && mxImpl->mnHeight)
    {
        aPrimitives = CreateB2DPrimitiveRange(0, 0, mxImpl->mnWidth - 1, mxImpl->mnHeight - 1, nullptr);
    }

    return aPrimitives;
}

#undef DBG_FRAME_CHECK_ROW_1
#undef DBG_FRAME_CHECK_COL_1
#undef DBG_FRAME_CHECK_COLROW
#undef DBG_FRAME_CHECK_ROW
#undef DBG_FRAME_CHECK_COL
#undef DBG_FRAME_CHECK

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
