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

#include <svx/frmsel.hxx>
#include <vcl/builder.hxx>

#include <algorithm>
#include <math.h>
#include "frmselimpl.hxx"
#include "AccessibleFrameSelector.hxx"
#include <svx/dialmgr.hxx>

#include <svx/dialogs.hrc>
#include "frmsel.hrc"

#include <tools/rcid.h>

using namespace ::com::sun::star;
using namespace ::editeng;

namespace svx {

using ::com::sun::star::uno::Reference;
using ::com::sun::star::accessibility::XAccessible;

// ============================================================================
// global functions from framebordertype.hxx

FrameBorderType GetFrameBorderTypeFromIndex( size_t nIndex )
{
    DBG_ASSERT( nIndex < (size_t)FRAMEBORDERTYPE_COUNT,
        "svx::GetFrameBorderTypeFromIndex - invalid index" );
    return static_cast< FrameBorderType >( nIndex + 1 );
}

size_t GetIndexFromFrameBorderType( FrameBorderType eBorder )
{
    DBG_ASSERT( eBorder != FRAMEBORDER_NONE,
        "svx::GetIndexFromFrameBorderType - invalid frame border type" );
    return static_cast< size_t >( eBorder ) - 1;
}

// ============================================================================

namespace {

/** Space between outer control border and any graphical element of the control. */
const long FRAMESEL_GEOM_OUTER    = 2;

/** Space between arrows and usable inner area. */
const long FRAMESEL_GEOM_INNER    = 3;

/** Maximum width to draw a frame border style. */
const long FRAMESEL_GEOM_WIDTH    = 9;

/** Additional margin for click area of outer lines. */
const long FRAMESEL_GEOM_ADD_CLICK_OUTER = 5;

/** Additional margin for click area of inner lines. */
const long FRAMESEL_GEOM_ADD_CLICK_INNER = 2;

// ----------------------------------------------------------------------------

/** Returns the corresponding flag for a frame border. */
FrameSelFlags lclGetFlagFromType( FrameBorderType eBorder )
{
    switch( eBorder )
    {
        case FRAMEBORDER_LEFT:      return FRAMESEL_LEFT;
        case FRAMEBORDER_RIGHT:     return FRAMESEL_RIGHT;
        case FRAMEBORDER_TOP:       return FRAMESEL_TOP;
        case FRAMEBORDER_BOTTOM:    return FRAMESEL_BOTTOM;
        case FRAMEBORDER_HOR:       return FRAMESEL_INNER_HOR;
        case FRAMEBORDER_VER:       return FRAMESEL_INNER_VER;
        case FRAMEBORDER_TLBR:      return FRAMESEL_DIAG_TLBR;
        case FRAMEBORDER_BLTR:      return FRAMESEL_DIAG_BLTR;
        case FRAMEBORDER_NONE : break;
    }
    return FRAMESEL_NONE;
}

/** Merges the rSource polypolygon into the rDest polypolygon. */
inline void lclPolyPolyUnion( PolyPolygon& rDest, const PolyPolygon& rSource )
{
    const PolyPolygon aTmp( rDest );
    aTmp.GetUnion( rSource, rDest );
}

} // namespace

// ============================================================================
// FrameBorder
// ============================================================================

FrameBorder::FrameBorder( FrameBorderType eType ) :
    meType( eType ),
    meState( FRAMESTATE_HIDE ),
    meKeyLeft( FRAMEBORDER_NONE ),
    meKeyRight( FRAMEBORDER_NONE ),
    meKeyTop( FRAMEBORDER_NONE ),
    meKeyBottom( FRAMEBORDER_NONE ),
    mbEnabled( false ),
    mbSelected( false )
{
}

void FrameBorder::Enable( FrameSelFlags nFlags )
{
    mbEnabled = (nFlags & lclGetFlagFromType( meType )) != 0;
    if( !mbEnabled )
        SetState( FRAMESTATE_HIDE );
}

void FrameBorder::SetCoreStyle( const SvxBorderLine* pStyle )
{
    if( pStyle )
        maCoreStyle = *pStyle;
    else
        maCoreStyle = SvxBorderLine();

    // from twips to points
    maUIStyle.Set( maCoreStyle, 0.05, FRAMESEL_GEOM_WIDTH );
    meState = maUIStyle.Prim() ? FRAMESTATE_SHOW : FRAMESTATE_HIDE;
}

void FrameBorder::SetState( FrameBorderState eState )
{
    meState = eState;
    switch( meState )
    {
        case FRAMESTATE_SHOW:
            SAL_WARN( "svx.dialog", "svx::FrameBorder::SetState - use SetCoreStyle to make border visible" );
        break;
        case FRAMESTATE_HIDE:
            maCoreStyle = SvxBorderLine();
            maUIStyle.Clear();
        break;
        case FRAMESTATE_DONTCARE:
            maCoreStyle = SvxBorderLine();
            maUIStyle = frame::Style(3, 0, 0, table::BorderLineStyle::SOLID); //OBJ_FRAMESTYLE_DONTCARE
        break;
    }
}

void FrameBorder::AddFocusPolygon( const Polygon& rFocus )
{
    lclPolyPolyUnion( maFocusArea, rFocus );
}

void FrameBorder::MergeFocusToPolyPolygon( PolyPolygon& rPPoly ) const
{
    lclPolyPolyUnion( rPPoly, maFocusArea );
}

void FrameBorder::AddClickRect( const Rectangle& rRect )
{
    lclPolyPolyUnion( maClickArea, Polygon( rRect ) );
}

bool FrameBorder::ContainsClickPoint( const Point& rPos ) const
{
    return Region( maClickArea ).IsInside( rPos );
}

Rectangle FrameBorder::GetClickBoundRect() const
{
    return maClickArea.GetBoundRect();
}

void FrameBorder::SetKeyboardNeighbors(
        FrameBorderType eLeft, FrameBorderType eRight, FrameBorderType eTop, FrameBorderType eBottom )
{
    meKeyLeft = eLeft;
    meKeyRight = eRight;
    meKeyTop = eTop;
    meKeyBottom = eBottom;
}

FrameBorderType FrameBorder::GetKeyboardNeighbor( sal_uInt16 nKeyCode ) const
{
    FrameBorderType eBorder = FRAMEBORDER_NONE;
    switch( nKeyCode )
    {
        case KEY_LEFT:  eBorder = meKeyLeft;      break;
        case KEY_RIGHT: eBorder = meKeyRight;     break;
        case KEY_UP:    eBorder = meKeyTop;       break;
        case KEY_DOWN:  eBorder = meKeyBottom;    break;
        default:        SAL_WARN( "svx.dialog", "svx::FrameBorder::GetKeyboardNeighbor - unknown key code" );
    }
    return eBorder;
}

// ============================================================================
// FrameSelectorImpl
// ============================================================================

FrameSelectorImpl::FrameSelectorImpl( FrameSelector& rFrameSel ) :
    Resource( SVX_RES( RID_SVXSTR_BORDER_CONTROL ) ),
    mrFrameSel( rFrameSel ),
    maILArrows( 16 ),
    maLeft( FRAMEBORDER_LEFT ),
    maRight( FRAMEBORDER_RIGHT ),
    maTop( FRAMEBORDER_TOP ),
    maBottom( FRAMEBORDER_BOTTOM ),
    maHor( FRAMEBORDER_HOR ),
    maVer( FRAMEBORDER_VER ),
    maTLBR( FRAMEBORDER_TLBR ),
    maBLTR( FRAMEBORDER_BLTR ),
    mnFlags( FRAMESEL_OUTER ),
    mnCtrlSize( 0 ),
    mnArrowSize( 0 ),
    mnLine1( 0 ),
    mnLine2( 0 ),
    mnLine3( 0 ),
    mnFocusOffs( 0 ),
    mbHor( false ),
    mbVer( false ),
    mbTLBR( false ),
    mbBLTR( false ),
    mbFullRepaint( true ),
    mbAutoSelect( true ),
    mbClicked( false ),
    mbHCMode( false ),
    mpAccess( 0 ),
    maChildVec( 8, static_cast< a11y::AccFrameSelector* >( 0 ) ),
    mxChildVec( 8 )
{
    FreeResource();

    maAllBorders.resize( FRAMEBORDERTYPE_COUNT, 0 );
    maAllBorders[ GetIndexFromFrameBorderType( FRAMEBORDER_LEFT   ) ] = &maLeft;
    maAllBorders[ GetIndexFromFrameBorderType( FRAMEBORDER_RIGHT  ) ] = &maRight;
    maAllBorders[ GetIndexFromFrameBorderType( FRAMEBORDER_TOP    ) ] = &maTop;
    maAllBorders[ GetIndexFromFrameBorderType( FRAMEBORDER_BOTTOM ) ] = &maBottom;
    maAllBorders[ GetIndexFromFrameBorderType( FRAMEBORDER_HOR    ) ] = &maHor;
    maAllBorders[ GetIndexFromFrameBorderType( FRAMEBORDER_VER    ) ] = &maVer;
    maAllBorders[ GetIndexFromFrameBorderType( FRAMEBORDER_TLBR   ) ] = &maTLBR;
    maAllBorders[ GetIndexFromFrameBorderType( FRAMEBORDER_BLTR   ) ] = &maBLTR;
#if OSL_DEBUG_LEVEL >= 2
    {
        bool bOk = true;
        for( FrameBorderCIter aIt( maAllBorders ); bOk && aIt.Is(); bOk = (*aIt != 0), ++aIt );
        DBG_ASSERT( bOk, "svx::FrameSelectorImpl::FrameSelectorImpl - missing entry in maAllBorders" );
    }
#endif
    //                             left neighbor     right neighbor     upper neighbor    lower neighbor
    maLeft.SetKeyboardNeighbors(   FRAMEBORDER_NONE, FRAMEBORDER_TLBR,  FRAMEBORDER_TOP,  FRAMEBORDER_BOTTOM );
    maRight.SetKeyboardNeighbors(  FRAMEBORDER_BLTR, FRAMEBORDER_NONE,  FRAMEBORDER_TOP,  FRAMEBORDER_BOTTOM );
    maTop.SetKeyboardNeighbors(    FRAMEBORDER_LEFT, FRAMEBORDER_RIGHT, FRAMEBORDER_NONE, FRAMEBORDER_TLBR );
    maBottom.SetKeyboardNeighbors( FRAMEBORDER_LEFT, FRAMEBORDER_RIGHT, FRAMEBORDER_BLTR, FRAMEBORDER_NONE );
    maHor.SetKeyboardNeighbors(    FRAMEBORDER_LEFT, FRAMEBORDER_RIGHT, FRAMEBORDER_TLBR, FRAMEBORDER_BLTR );
    maVer.SetKeyboardNeighbors(    FRAMEBORDER_TLBR, FRAMEBORDER_BLTR,  FRAMEBORDER_TOP,  FRAMEBORDER_BOTTOM );
    maTLBR.SetKeyboardNeighbors(   FRAMEBORDER_LEFT, FRAMEBORDER_VER,   FRAMEBORDER_TOP,  FRAMEBORDER_HOR );
    maBLTR.SetKeyboardNeighbors(   FRAMEBORDER_VER,  FRAMEBORDER_RIGHT, FRAMEBORDER_HOR,  FRAMEBORDER_BOTTOM );

    Initialize(mnFlags);
}

FrameSelectorImpl::~FrameSelectorImpl()
{
    if( mpAccess )
        mpAccess->Invalidate();
    for( AccessibleImplVec::iterator aIt = maChildVec.begin(), aEnd = maChildVec.end(); aIt != aEnd; ++aIt )
        if( *aIt )
            (*aIt)->Invalidate();
}

// initialization -------------------------------------------------------------

void FrameSelectorImpl::Initialize( FrameSelFlags nFlags )
{
    mnFlags = nFlags;

    maEnabBorders.clear();
    for( FrameBorderIter aIt( maAllBorders ); aIt.Is(); ++aIt )
    {
        (*aIt)->Enable( mnFlags );
        if( (*aIt)->IsEnabled() )
            maEnabBorders.push_back( *aIt );
    }
    mbHor = maHor.IsEnabled();
    mbVer = maVer.IsEnabled();
    mbTLBR = maTLBR.IsEnabled();
    mbBLTR = maBLTR.IsEnabled();

    InitVirtualDevice();
}

void FrameSelectorImpl::InitColors()
{
    const StyleSettings& rSett = mrFrameSel.GetSettings().GetStyleSettings();
    maBackCol = rSett.GetFieldColor();
    mbHCMode = rSett.GetHighContrastMode();
    maArrowCol = rSett.GetFieldTextColor();
    maMarkCol.operator=( maBackCol ).Merge( maArrowCol, mbHCMode ? 0x80 : 0xC0 );
    maHCLineCol = rSett.GetLabelTextColor();
}

void FrameSelectorImpl::InitArrowImageList()
{
    /* Build the arrow images bitmap with current colors. */
    Color pColorAry1[3];
    Color pColorAry2[3];
    pColorAry1[0] = Color( 0, 0, 0 );
    pColorAry2[0] = maArrowCol;       // black -> arrow color
    pColorAry1[1] = Color( 0, 255, 0 );
    pColorAry2[1] = maMarkCol;        // green -> marker color
    pColorAry1[2] = Color( 255, 0, 255 );
    pColorAry2[2] = maBackCol;       // magenta -> background

    GetRes( SVX_RES( RID_SVXSTR_BORDER_CONTROL ).SetRT( RSC_RESOURCE ) );
    maILArrows.InsertFromHorizontalBitmap(
        SVX_RES( BMP_FRMSEL_ARROWS ), 16, NULL, pColorAry1, pColorAry2, 3);
    FreeResource();
    DBG_ASSERT( maILArrows.GetImageSize().Height() == maILArrows.GetImageSize().Width(),
        "svx::FrameSelectorImpl::InitArrowImageList - images are not squarish" );
    mnArrowSize = maILArrows.GetImageSize().Height();
}

void FrameSelectorImpl::InitGlobalGeometry()
{
    Size aCtrlSize( mrFrameSel.CalcOutputSize( mrFrameSel.GetSizePixel() ) );
    /*  nMinSize is the lower of width and height (control will always be squarish).
        FRAMESEL_GEOM_OUTER is the minimal distance between inner control border
        and any element. */
    long nMinSize = std::min( aCtrlSize.Width(), aCtrlSize.Height() ) - 2 * FRAMESEL_GEOM_OUTER;
    /*  nFixedSize is the size all existing elements need in one direction:
        the diag. arrow, space betw. arrow and frame border, outer frame border,
        inner frame border, other outer frame border, space betw. frame border
        and arrow, the other arrow. */
    long nFixedSize = 2 * mnArrowSize + 2 * FRAMESEL_GEOM_INNER + 3 * FRAMESEL_GEOM_WIDTH;
    /*  nBetwBordersSize contains the size between an outer and inner frame border (made odd). */
    long nBetwBordersSize = (((nMinSize - nFixedSize) / 2) - 1) | 1;

    /*  The final size of the usable area. */
    mnCtrlSize = 2 * nBetwBordersSize + nFixedSize;
    maVirDev.SetOutputSizePixel( Size( mnCtrlSize, mnCtrlSize ) );

    /*  Center the virtual device in the control. */
    maVirDevPos = Point( (aCtrlSize.Width() - mnCtrlSize) / 2, (aCtrlSize.Height() - mnCtrlSize) / 2 );
}

void FrameSelectorImpl::InitBorderGeometry()
{
    size_t nCol, nCols, nRow, nRows;

    // Global border geometry values ------------------------------------------

    /*  mnLine* is the middle point inside a frame border (i.e. mnLine1 is mid X inside left border). */
    mnLine1 = mnArrowSize + FRAMESEL_GEOM_INNER + FRAMESEL_GEOM_WIDTH / 2;
    mnLine2 = mnCtrlSize / 2;
    mnLine3 = 2 * mnLine2 - mnLine1;

    // Frame helper array -----------------------------------------------------

    maArray.Initialize( mbVer ? 2 : 1, mbHor ? 2 : 1 );
    maArray.SetUseDiagDoubleClipping( true );

    maArray.SetXOffset( mnLine1 );
    maArray.SetAllColWidths( (mbVer ? mnLine2 : mnLine3) - mnLine1 );

    maArray.SetYOffset( mnLine1 );
    maArray.SetAllRowHeights( (mbHor ? mnLine2 : mnLine3) - mnLine1 );

    // Focus polygons ---------------------------------------------------------

    /*  Width for focus rectangles from center of frame borders. */
    mnFocusOffs = FRAMESEL_GEOM_WIDTH / 2 + 1;

    maLeft.ClearFocusArea();
    maVer.ClearFocusArea();
    maRight.ClearFocusArea();
    maTop.ClearFocusArea();
    maHor.ClearFocusArea();
    maBottom.ClearFocusArea();

    maLeft.AddFocusPolygon(   Rectangle( mnLine1 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine1 + mnFocusOffs, mnLine3 + mnFocusOffs ) );
    maVer.AddFocusPolygon(    Rectangle( mnLine2 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine2 + mnFocusOffs, mnLine3 + mnFocusOffs ) );
    maRight.AddFocusPolygon(  Rectangle( mnLine3 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine3 + mnFocusOffs ) );
    maTop.AddFocusPolygon(    Rectangle( mnLine1 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine1 + mnFocusOffs ) );
    maHor.AddFocusPolygon(    Rectangle( mnLine1 - mnFocusOffs, mnLine2 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine2 + mnFocusOffs ) );
    maBottom.AddFocusPolygon( Rectangle( mnLine1 - mnFocusOffs, mnLine3 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine3 + mnFocusOffs ) );

    for( nCol = 0, nCols = maArray.GetColCount(); nCol < nCols; ++nCol )
    {
        for( nRow = 0, nRows = maArray.GetRowCount(); nRow < nRows; ++nRow )
        {
            Rectangle aRect( maArray.GetCellRect( nCol, nRow ) );
            long nDiagFocusOffsX = frame::GetTLDiagOffset( -mnFocusOffs, mnFocusOffs, maArray.GetHorDiagAngle( nCol, nRow ) );
            long nDiagFocusOffsY = frame::GetTLDiagOffset( -mnFocusOffs, mnFocusOffs, maArray.GetVerDiagAngle( nCol, nRow ) );

            std::vector< Point > aFocusVec;
            aFocusVec.push_back( Point( aRect.Left()  - mnFocusOffs,     aRect.Top()    + nDiagFocusOffsY ) );
            aFocusVec.push_back( Point( aRect.Left()  - mnFocusOffs,     aRect.Top()    - mnFocusOffs     ) );
            aFocusVec.push_back( Point( aRect.Left()  + nDiagFocusOffsX, aRect.Top()    - mnFocusOffs     ) );
            aFocusVec.push_back( Point( aRect.Right() + mnFocusOffs,     aRect.Bottom() - nDiagFocusOffsY ) );
            aFocusVec.push_back( Point( aRect.Right() + mnFocusOffs,     aRect.Bottom() + mnFocusOffs     ) );
            aFocusVec.push_back( Point( aRect.Right() - nDiagFocusOffsX, aRect.Bottom() + mnFocusOffs     ) );
            maTLBR.AddFocusPolygon( Polygon( static_cast< sal_uInt16 >( aFocusVec.size() ), &aFocusVec[ 0 ] ) );

            aFocusVec.clear();
            aFocusVec.push_back( Point( aRect.Right() + mnFocusOffs,     aRect.Top()    + nDiagFocusOffsY ) );
            aFocusVec.push_back( Point( aRect.Right() + mnFocusOffs,     aRect.Top()    - mnFocusOffs     ) );
            aFocusVec.push_back( Point( aRect.Right() - nDiagFocusOffsX, aRect.Top()    - mnFocusOffs     ) );
            aFocusVec.push_back( Point( aRect.Left()  - mnFocusOffs,     aRect.Bottom() - nDiagFocusOffsY ) );
            aFocusVec.push_back( Point( aRect.Left()  - mnFocusOffs,     aRect.Bottom() + mnFocusOffs     ) );
            aFocusVec.push_back( Point( aRect.Left()  + nDiagFocusOffsX, aRect.Bottom() + mnFocusOffs     ) );
            maBLTR.AddFocusPolygon( Polygon( static_cast< sal_uInt16 >( aFocusVec.size() ), &aFocusVec[ 0 ] ) );
        }
    }

    // Click areas ------------------------------------------------------------

    for( FrameBorderIter aIt( maAllBorders ); aIt.Is(); ++aIt )
        (*aIt)->ClearClickArea();

    /*  Additional space for click area: is added to the space available to draw
        the frame borders. For instance left frame border:
        - To left, top, and bottom always big additional space (outer area).
        - To right: Dependent on existence of inner vertical frame border
            (if enabled, use less space).
     */
    long nClO = FRAMESEL_GEOM_WIDTH / 2 + FRAMESEL_GEOM_ADD_CLICK_OUTER;
    long nClI = (mbTLBR && mbBLTR) ? (FRAMESEL_GEOM_WIDTH / 2 + FRAMESEL_GEOM_ADD_CLICK_INNER) : nClO;
    long nClH = mbHor ? nClI : nClO;            // additional space dependent of horizontal inner border
    long nClV = mbVer ? nClI : nClO;            // additional space dependent of vertical inner border

    maLeft.AddClickRect(   Rectangle( mnLine1 - nClO, mnLine1 - nClO, mnLine1 + nClV, mnLine3 + nClO ) );
    maVer.AddClickRect(    Rectangle( mnLine2 - nClI, mnLine1 - nClO, mnLine2 + nClI, mnLine3 + nClO ) );
    maRight.AddClickRect(  Rectangle( mnLine3 - nClV, mnLine1 - nClO, mnLine3 + nClO, mnLine3 + nClO ) );
    maTop.AddClickRect(    Rectangle( mnLine1 - nClO, mnLine1 - nClO, mnLine3 + nClO, mnLine1 + nClH ) );
    maHor.AddClickRect(    Rectangle( mnLine1 - nClO, mnLine2 - nClI, mnLine3 + nClO, mnLine2 + nClI ) );
    maBottom.AddClickRect( Rectangle( mnLine1 - nClO, mnLine3 - nClH, mnLine3 + nClO, mnLine3 + nClO ) );

    /*  Diagonal frame borders use the remaining space between outer and inner frame borders. */
    if( mbTLBR || mbBLTR )
    {
        for( nCol = 0, nCols = maArray.GetColCount(); nCol < nCols; ++nCol )
        {
            for( nRow = 0, nRows = maArray.GetRowCount(); nRow < nRows; ++nRow )
            {
                // the usable area between horizonal/vertical frame borders of current quadrant
                Rectangle aRect( maArray.GetCellRect( nCol, nRow ) );
                aRect.Left() += nClV + 1;
                aRect.Right() -= nClV + 1;
                aRect.Top() += nClH + 1;
                aRect.Bottom() -= nClH + 1;

                /*  Both diagonal frame borders enabled. */
                if( mbTLBR && mbBLTR )
                {
                    // single areas
                    Point aMid( aRect.Center() );
                    maTLBR.AddClickRect( Rectangle( aRect.TopLeft(), aMid ) );
                    maTLBR.AddClickRect( Rectangle( aMid + Point( 1, 1 ), aRect.BottomRight() ) );
                    maBLTR.AddClickRect( Rectangle( aRect.Left(), aMid.Y() + 1, aMid.X(), aRect.Bottom() ) );
                    maBLTR.AddClickRect( Rectangle( aMid.X() + 1, aRect.Top(), aRect.Right(), aMid.Y() ) );
                    // centered rectangle for both frame borders
                    Rectangle aMidRect( aRect.TopLeft(), Size( aRect.GetWidth() / 3, aRect.GetHeight() / 3 ) );
                    aMidRect.Move( (aRect.GetWidth() - aMidRect.GetWidth()) / 2, (aRect.GetHeight() - aMidRect.GetHeight()) / 2 );
                    maTLBR.AddClickRect( aMidRect );
                    maBLTR.AddClickRect( aMidRect );
                }
                /*  One of the diagonal frame borders enabled - use entire rectangle. */
                else if( mbTLBR && !mbBLTR )    // top-left to bottom-right only
                    maTLBR.AddClickRect( aRect );
                else if( !mbTLBR && mbBLTR )    // bottom-left to top-right only
                    maBLTR.AddClickRect( aRect );
            }
        }
    }
}

void FrameSelectorImpl::InitVirtualDevice()
{
    // initialize resources
    InitColors();
    InitArrowImageList();

    sizeChanged();
}

void FrameSelectorImpl::sizeChanged()
{
    // initialize geometry
    InitGlobalGeometry();
    InitBorderGeometry();

    // correct background around the used area
    mrFrameSel.SetBackground( Wallpaper( maBackCol ) );
    DoInvalidate( true );
}

// frame border access --------------------------------------------------------

const FrameBorder& FrameSelectorImpl::GetBorder( FrameBorderType eBorder ) const
{
    size_t nIndex = GetIndexFromFrameBorderType( eBorder );
    if( nIndex < maAllBorders.size() )
        return *maAllBorders[ nIndex ];
    SAL_WARN( "svx.dialog", "svx::FrameSelectorImpl::GetBorder - unknown border type" );
    return maTop;
}

FrameBorder& FrameSelectorImpl::GetBorderAccess( FrameBorderType eBorder )
{
    return const_cast< FrameBorder& >( GetBorder( eBorder ) );
}

// drawing --------------------------------------------------------------------

void FrameSelectorImpl::DrawBackground()
{
    // clear the area
    maVirDev.SetLineColor();
    maVirDev.SetFillColor( maBackCol );
    maVirDev.DrawRect( Rectangle( Point( 0, 0 ), maVirDev.GetOutputSizePixel() ) );

    // draw the inner gray (or whatever color) rectangle
    maVirDev.SetLineColor();
    maVirDev.SetFillColor( maMarkCol );
    maVirDev.DrawRect( Rectangle(
        mnLine1 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine3 + mnFocusOffs ) );

    // draw the white space for enabled frame borders
    PolyPolygon aPPoly;
    for( FrameBorderCIter aIt( maEnabBorders ); aIt.Is(); ++aIt )
        (*aIt)->MergeFocusToPolyPolygon( aPPoly );
    aPPoly.Optimize( POLY_OPTIMIZE_CLOSE );
    maVirDev.SetLineColor( maBackCol );
    maVirDev.SetFillColor( maBackCol );
    maVirDev.DrawPolyPolygon( aPPoly );
}

void FrameSelectorImpl::DrawArrows( const FrameBorder& rBorder )
{
    DBG_ASSERT( rBorder.IsEnabled(), "svx::FrameSelectorImpl::DrawArrows - access to disabled border" );

    long nLinePos = 0;
    switch( rBorder.GetType() )
    {
        case FRAMEBORDER_LEFT:
        case FRAMEBORDER_TOP:       nLinePos = mnLine1; break;
        case FRAMEBORDER_VER:
        case FRAMEBORDER_HOR:       nLinePos = mnLine2; break;
        case FRAMEBORDER_RIGHT:
        case FRAMEBORDER_BOTTOM:    nLinePos = mnLine3; break;
        default: ; //prevent warning
    }
    nLinePos -= mnArrowSize / 2;

    long nTLPos = 0;
    long nBRPos = mnCtrlSize - mnArrowSize;
    Point aPos1, aPos2;
    sal_uInt16 nImgId1 = 0, nImgId2 = 0;
    switch( rBorder.GetType() )
    {
        case FRAMEBORDER_LEFT:
        case FRAMEBORDER_RIGHT:
        case FRAMEBORDER_VER:
            aPos1 = Point( nLinePos, nTLPos ); nImgId1 = 1;
            aPos2 = Point( nLinePos, nBRPos ); nImgId2 = 2;
        break;

        case FRAMEBORDER_TOP:
        case FRAMEBORDER_BOTTOM:
        case FRAMEBORDER_HOR:
            aPos1 = Point( nTLPos, nLinePos ); nImgId1 = 3;
            aPos2 = Point( nBRPos, nLinePos ); nImgId2 = 4;
        break;

        case FRAMEBORDER_TLBR:
            aPos1 = Point( nTLPos, nTLPos ); nImgId1 = 5;
            aPos2 = Point( nBRPos, nBRPos ); nImgId2 = 6;
        break;
        case FRAMEBORDER_BLTR:
            aPos1 = Point( nTLPos, nBRPos ); nImgId1 = 7;
            aPos2 = Point( nBRPos, nTLPos ); nImgId2 = 8;
        break;
        default: ; //prevent warning
    }

    // Arrow or marker? Do not draw arrows into disabled control.
    sal_uInt16 nSelectAdd = (mrFrameSel.IsEnabled() && rBorder.IsSelected()) ? 0 : 8;
    maVirDev.DrawImage( aPos1, maILArrows.GetImage( nImgId1 + nSelectAdd ) );
    maVirDev.DrawImage( aPos2, maILArrows.GetImage( nImgId2 + nSelectAdd ) );
}

void FrameSelectorImpl::DrawAllArrows()
{
    for( FrameBorderCIter aIt( maEnabBorders ); aIt.Is(); ++aIt )
        DrawArrows( **aIt );
}

Color FrameSelectorImpl::GetDrawLineColor( const Color& rColor ) const
{
    Color aColor( mbHCMode ? maHCLineCol : rColor );
    if( aColor == maBackCol )
        aColor.Invert();
    return aColor;
}

void FrameSelectorImpl::DrawAllFrameBorders()
{
    // Translate core colors to current UI colors (regards current background and HC mode).
    for( FrameBorderIter aIt( maEnabBorders ); aIt.Is(); ++aIt )
    {
        Color aCoreColorPrim = ((*aIt)->GetState() == FRAMESTATE_DONTCARE) ? maMarkCol : (*aIt)->GetCoreStyle().GetColorOut();
        Color aCoreColorSecn = ((*aIt)->GetState() == FRAMESTATE_DONTCARE) ? maMarkCol : (*aIt)->GetCoreStyle().GetColorIn();
        (*aIt)->SetUIColorPrim( GetDrawLineColor( aCoreColorPrim ) );
        (*aIt)->SetUIColorSecn( GetDrawLineColor( aCoreColorSecn ) );
    }

    // Copy all frame border styles to the helper array
    maArray.SetColumnStyleLeft( 0, maLeft.GetUIStyle() );
    if( mbVer ) maArray.SetColumnStyleLeft( 1, maVer.GetUIStyle() );

    // Invert the style for the right line
    const frame::Style rRightStyle = maRight.GetUIStyle( );
    frame::Style rInvertedRight( rRightStyle.GetColorPrim(),
            rRightStyle.GetColorSecn(), rRightStyle.GetColorGap(),
            rRightStyle.UseGapColor(),
            rRightStyle.Secn(), rRightStyle.Dist(), rRightStyle.Prim( ),
            rRightStyle.Type( ) );
    maArray.SetColumnStyleRight( mbVer ? 1 : 0, rInvertedRight );

    maArray.SetRowStyleTop( 0, maTop.GetUIStyle() );
    if( mbHor )
    {
        // Invert the style for the hor line to match the real borders
        const frame::Style rHorStyle = maHor.GetUIStyle();
        frame::Style rInvertedHor( rHorStyle.GetColorPrim(),
            rHorStyle.GetColorSecn(), rHorStyle.GetColorGap(),
            rHorStyle.UseGapColor(),
            rHorStyle.Secn(), rHorStyle.Dist(), rHorStyle.Prim( ),
            rHorStyle.Type() );
        maArray.SetRowStyleTop( 1, rInvertedHor );
    }

    // Invert the style for the bottom line
    const frame::Style rBottomStyle = maBottom.GetUIStyle( );
    frame::Style rInvertedBottom( rBottomStyle.GetColorPrim(),
            rBottomStyle.GetColorSecn(), rBottomStyle.GetColorGap(),
            rBottomStyle.UseGapColor(),
            rBottomStyle.Secn(), rBottomStyle.Dist(), rBottomStyle.Prim( ),
            rBottomStyle.Type() );
    maArray.SetRowStyleBottom( mbHor ? 1 : 0, rInvertedBottom );

    for( size_t nCol = 0; nCol < maArray.GetColCount(); ++nCol )
        for( size_t nRow = 0; nRow < maArray.GetRowCount(); ++nRow )
            maArray.SetCellStyleDiag( nCol, nRow, maTLBR.GetUIStyle(), maBLTR.GetUIStyle() );

    // Let the helper array draw itself
    maArray.DrawArray( maVirDev );
}

void FrameSelectorImpl::DrawVirtualDevice()
{
    DrawBackground();
    DrawAllArrows();
    DrawAllFrameBorders();
    mbFullRepaint = false;
}

void FrameSelectorImpl::CopyVirDevToControl()
{
    if( mbFullRepaint )
        DrawVirtualDevice();
    mrFrameSel.DrawBitmap( maVirDevPos, maVirDev.GetBitmap( Point( 0, 0 ), maVirDev.GetOutputSizePixel() ) );
}

void FrameSelectorImpl::DrawAllTrackingRects()
{
    PolyPolygon aPPoly;
    if( mrFrameSel.IsAnyBorderSelected() )
    {
        for( SelFrameBorderCIter aIt( maEnabBorders ); aIt.Is(); ++aIt )
            (*aIt)->MergeFocusToPolyPolygon( aPPoly );
        aPPoly.Move( maVirDevPos.X(), maVirDevPos.Y() );
    }
    else
        // no frame border selected -> draw tracking rectangle around entire control
        aPPoly.Insert( Polygon( Rectangle( maVirDevPos, maVirDev.GetOutputSizePixel() ) ) );

    aPPoly.Optimize( POLY_OPTIMIZE_CLOSE );
    for( sal_uInt16 nIdx = 0, nCount = aPPoly.Count(); nIdx < nCount; ++nIdx )
        mrFrameSel.InvertTracking( aPPoly.GetObject( nIdx ), SHOWTRACK_SMALL | SHOWTRACK_WINDOW );
}

Point FrameSelectorImpl::GetDevPosFromMousePos( const Point& rMousePos ) const
{
    return rMousePos - maVirDevPos;
}

void FrameSelectorImpl::DoInvalidate( bool bFullRepaint )
{
    mbFullRepaint |= bFullRepaint;
    mrFrameSel.Invalidate( INVALIDATE_NOERASE );
}

// frame border state and style -----------------------------------------------

void FrameSelectorImpl::SetBorderState( FrameBorder& rBorder, FrameBorderState eState )
{
    DBG_ASSERT( rBorder.IsEnabled(), "svx::FrameSelectorImpl::SetBorderState - access to disabled border" );
    if( eState == FRAMESTATE_SHOW )
        SetBorderCoreStyle( rBorder, &maCurrStyle );
    else
        rBorder.SetState( eState );
    DoInvalidate( true );
}

void FrameSelectorImpl::SetBorderCoreStyle( FrameBorder& rBorder, const SvxBorderLine* pStyle )
{
    DBG_ASSERT( rBorder.IsEnabled(), "svx::FrameSelectorImpl::SetBorderCoreStyle - access to disabled border" );
    rBorder.SetCoreStyle( pStyle );
    DoInvalidate( true );
}

void FrameSelectorImpl::ToggleBorderState( FrameBorder& rBorder )
{
    bool bDontCare = mrFrameSel.SupportsDontCareState();
    switch( rBorder.GetState() )
    {
        // same order as tristate check box: visible -> don't care -> hidden
        case FRAMESTATE_SHOW:
            SetBorderState( rBorder, bDontCare ? FRAMESTATE_DONTCARE : FRAMESTATE_HIDE );
        break;
        case FRAMESTATE_HIDE:
            SetBorderState( rBorder, FRAMESTATE_SHOW );
        break;
        case FRAMESTATE_DONTCARE:
            SetBorderState( rBorder, FRAMESTATE_HIDE );
        break;
    }
}

// frame border selection -----------------------------------------------------

void FrameSelectorImpl::SelectBorder( FrameBorder& rBorder, bool bSelect )
{
    DBG_ASSERT( rBorder.IsEnabled(), "svx::FrameSelectorImpl::SelectBorder - access to disabled border" );
    rBorder.Select( bSelect );
    DrawArrows( rBorder );
    DoInvalidate( false );
}

void FrameSelectorImpl::SilentGrabFocus()
{
    bool bOldAuto = mbAutoSelect;
    mbAutoSelect = false;
    mrFrameSel.GrabFocus();
    mbAutoSelect = bOldAuto;
}

bool FrameSelectorImpl::SelectedBordersEqual() const
{
    bool bEqual = true;
    SelFrameBorderCIter aIt( maEnabBorders );
    if( aIt.Is() )
    {
        const SvxBorderLine& rFirstStyle = (*aIt)->GetCoreStyle();
        for( ++aIt; bEqual && aIt.Is(); ++aIt )
            bEqual = ((*aIt)->GetCoreStyle() == rFirstStyle);
    }
    return bEqual;
}

// ============================================================================
// FrameSelector
// ============================================================================

FrameSelector::FrameSelector(Window* pParent)
    : Control(pParent, WB_BORDER|WB_TABSTOP)
{
    // not in c'tor init list (avoid warning about usage of *this)
    mxImpl.reset( new FrameSelectorImpl( *this ) );
    EnableRTL( false ); // #107808# don't mirror the mouse handling
}

extern "C" SAL_DLLPUBLIC_EXPORT Window* SAL_CALL makeSvxFrameSelector(Window *pParent, VclBuilder::stringmap &)
{
    return new FrameSelector(pParent);
}

FrameSelector::~FrameSelector()
{
}

void FrameSelector::Initialize( FrameSelFlags nFlags )
{
    mxImpl->Initialize( nFlags );
    Show();
}

// enabled frame borders ------------------------------------------------------

bool FrameSelector::IsBorderEnabled( FrameBorderType eBorder ) const
{
    return mxImpl->GetBorder( eBorder ).IsEnabled();
}

sal_Int32 FrameSelector::GetEnabledBorderCount() const
{
    return static_cast< sal_Int32 >( mxImpl->maEnabBorders.size() );
}

FrameBorderType FrameSelector::GetEnabledBorderType( sal_Int32 nIndex ) const
{
    FrameBorderType eBorder = FRAMEBORDER_NONE;
    if( nIndex >= 0 )
    {
        size_t nVecIdx = static_cast< size_t >( nIndex );
        if( nVecIdx < mxImpl->maEnabBorders.size() )
            eBorder = mxImpl->maEnabBorders[ nVecIdx ]->GetType();
    }
    return eBorder;
}

sal_Int32 FrameSelector::GetEnabledBorderIndex( FrameBorderType eBorder ) const
{
    sal_Int32 nIndex = 0;
    for( FrameBorderCIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt, ++nIndex )
        if( (*aIt)->GetType() == eBorder )
            return nIndex;
    return -1;
}

// frame border state and style -----------------------------------------------

bool FrameSelector::SupportsDontCareState() const
{
    return (mxImpl->mnFlags & FRAMESEL_DONTCARE) != 0;
}

FrameBorderState FrameSelector::GetFrameBorderState( FrameBorderType eBorder ) const
{
    return mxImpl->GetBorder( eBorder ).GetState();
}

const SvxBorderLine* FrameSelector::GetFrameBorderStyle( FrameBorderType eBorder ) const
{
    const SvxBorderLine& rStyle = mxImpl->GetBorder( eBorder ).GetCoreStyle();
    // rest of the world uses null pointer for invisible frame border
    return rStyle.GetOutWidth() ? &rStyle : 0;
}

void FrameSelector::ShowBorder( FrameBorderType eBorder, const SvxBorderLine* pStyle )
{
    mxImpl->SetBorderCoreStyle( mxImpl->GetBorderAccess( eBorder ), pStyle );
}

void FrameSelector::SetBorderDontCare( FrameBorderType eBorder )
{
    mxImpl->SetBorderState( mxImpl->GetBorderAccess( eBorder ), FRAMESTATE_DONTCARE );
}

bool FrameSelector::IsAnyBorderVisible() const
{
    bool bIsSet = false;
    for( FrameBorderCIter aIt( mxImpl->maEnabBorders ); !bIsSet && aIt.Is(); ++aIt )
        bIsSet = ((*aIt)->GetState() == FRAMESTATE_SHOW);
    return bIsSet;
}

void FrameSelector::HideAllBorders()
{
    for( FrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
        mxImpl->SetBorderState( **aIt, FRAMESTATE_HIDE );
}

bool FrameSelector::GetVisibleWidth( long& rnWidth, SvxBorderStyle& rnStyle ) const
{
    VisFrameBorderCIter aIt( mxImpl->maEnabBorders );
    if( !aIt.Is() )
        return false;

    const SvxBorderLine& rStyle = (*aIt)->GetCoreStyle();
    bool bFound = true;
    for( ++aIt; bFound && aIt.Is(); ++aIt )
    {
        bFound =
            (rStyle.GetWidth() == (*aIt)->GetCoreStyle().GetWidth()) &&
            (rStyle.GetBorderLineStyle() ==
                (*aIt)->GetCoreStyle().GetBorderLineStyle());
    }

    if( bFound )
    {
        rnWidth = rStyle.GetWidth();
        rnStyle = rStyle.GetBorderLineStyle();
    }
    return bFound;
}

bool FrameSelector::GetVisibleColor( Color& rColor ) const
{
    VisFrameBorderCIter aIt( mxImpl->maEnabBorders );
    if( !aIt.Is() )
        return false;

    const SvxBorderLine& rStyle = (*aIt)->GetCoreStyle();
    bool bFound = true;
    for( ++aIt; bFound && aIt.Is(); ++aIt )
        bFound = (rStyle.GetColor() == (*aIt)->GetCoreStyle().GetColor());

    if( bFound )
        rColor = rStyle.GetColor();
    return bFound;
}

// frame border selection -----------------------------------------------------

const Link& FrameSelector::GetSelectHdl() const
{
    return mxImpl->maSelectHdl;
}

void FrameSelector::SetSelectHdl( const Link& rHdl )
{
    mxImpl->maSelectHdl = rHdl;
}

bool FrameSelector::IsBorderSelected( FrameBorderType eBorder ) const
{
    return mxImpl->GetBorder( eBorder ).IsSelected();
}

void FrameSelector::SelectBorder( FrameBorderType eBorder, bool bSelect )
{
    mxImpl->SelectBorder( mxImpl->GetBorderAccess( eBorder ), bSelect );
}

bool FrameSelector::IsAnyBorderSelected() const
{
    // Construct an iterator for selected borders. If it is valid, there is a selected border.
    return SelFrameBorderCIter( mxImpl->maEnabBorders ).Is();
}

void FrameSelector::SelectAllBorders( bool bSelect )
{
    for( FrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
        mxImpl->SelectBorder( **aIt, bSelect );
}

void FrameSelector::SelectAllVisibleBorders( bool bSelect )
{
    for( VisFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
        mxImpl->SelectBorder( **aIt, bSelect );
}

void FrameSelector::SetStyleToSelection( long nWidth, SvxBorderStyle nStyle )
{
    mxImpl->maCurrStyle.SetBorderLineStyle( nStyle );
    mxImpl->maCurrStyle.SetWidth( nWidth );
    for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
        mxImpl->SetBorderState( **aIt, FRAMESTATE_SHOW );
}

void FrameSelector::SetColorToSelection( const Color& rColor )
{
    mxImpl->maCurrStyle.SetColor( rColor );
    for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
        mxImpl->SetBorderState( **aIt, FRAMESTATE_SHOW );
}

// accessibility --------------------------------------------------------------

Reference< XAccessible > FrameSelector::CreateAccessible()
{
    if( !mxImpl->mxAccess.is() )
        mxImpl->mxAccess = mxImpl->mpAccess =
            new a11y::AccFrameSelector( *this, FRAMEBORDER_NONE );
    return mxImpl->mxAccess;
}

Reference< XAccessible > FrameSelector::GetChildAccessible( FrameBorderType eBorder )
{
    Reference< XAccessible > xRet;
    size_t nVecIdx = static_cast< size_t >( eBorder );
    if( IsBorderEnabled( eBorder ) && (1 <= nVecIdx) && (nVecIdx <= mxImpl->maChildVec.size()) )
    {
        --nVecIdx;
        if( !mxImpl->maChildVec[ nVecIdx ] )
            mxImpl->mxChildVec[ nVecIdx ] = mxImpl->maChildVec[ nVecIdx ] =
                new a11y::AccFrameSelector( *this, eBorder );
        xRet = mxImpl->mxChildVec[ nVecIdx ];
    }
    return xRet;
}

Reference< XAccessible > FrameSelector::GetChildAccessible( sal_Int32 nIndex )
{
    return GetChildAccessible( GetEnabledBorderType( nIndex ) );
}

Reference< XAccessible > FrameSelector::GetChildAccessible( const Point& rPos )
{
    Reference< XAccessible > xRet;
    for( FrameBorderCIter aIt( mxImpl->maEnabBorders ); !xRet.is() && aIt.Is(); ++aIt )
        if( (*aIt)->ContainsClickPoint( rPos ) )
            xRet = GetChildAccessible( (*aIt)->GetType() );
    return xRet;
}

bool FrameSelector::ContainsClickPoint( const Point& rPos ) const
{
    bool bContains = false;
    for( FrameBorderCIter aIt( mxImpl->maEnabBorders ); !bContains && aIt.Is(); ++aIt )
        bContains = (*aIt)->ContainsClickPoint( rPos );
    return bContains;
}

Rectangle FrameSelector::GetClickBoundRect( FrameBorderType eBorder ) const
{
    Rectangle aRect;
    const FrameBorder& rBorder = mxImpl->GetBorder( eBorder );
    if( rBorder.IsEnabled() )
        aRect = rBorder.GetClickBoundRect();
    return aRect;
}

// virtual functions from base class ------------------------------------------

void FrameSelector::Paint( const Rectangle& )
{
    mxImpl->CopyVirDevToControl();
    if( HasFocus() )
        mxImpl->DrawAllTrackingRects();
}

void FrameSelector::MouseButtonDown( const MouseEvent& rMEvt )
{
    /*  Mouse handling:
        * Click on an unselected frame border:
            Set current style/color, make frame border visible, deselect all
            other frame borders.
        * Click on a selected frame border:
            Toggle state of the frame border (visible -> don't care -> hidden),
            deselect all other frame borders.
        * SHIFT+Click or CTRL+Click on an unselected frame border:
            Extend selection, set current style/color to all selected frame
            borders independent of the state/style/color of the borders.
        * SHIFT+Click or CTRL+Click on a selected frame border:
            If all frame borders have same style/color, toggle state of all
            borders (see above), otherwise set current style/color to all
            borders.
        * Click on unused area: Do not modify selection and selected frame
            borders.
     */

    // #107394# do not auto-select a frame border
    mxImpl->SilentGrabFocus();

    if( rMEvt.IsLeft() )
    {
        Point aPos( mxImpl->GetDevPosFromMousePos( rMEvt.GetPosPixel() ) );
        FrameBorderPtrVec aDeselectBorders;

        bool bAnyClicked = false;   // Any frame border clicked?
        bool bNewSelected = false;  // Any unselected frame border selected?

        /*  If frame borders are set to "don't care" and the control does not
            support this state, hide them on first mouse click.
            DR 2004-01-30: Why are the borders set to "don't care" then?!? */
        bool bHideDontCare = !mxImpl->mbClicked && !SupportsDontCareState();

        for( FrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
        {
            if( (*aIt)->ContainsClickPoint( aPos ) )
            {
                // frame border is clicked
                bAnyClicked = true;
                if( !(*aIt)->IsSelected() )
                {
                    bNewSelected = true;
                    mxImpl->SelectBorder( **aIt, true );
                }
            }
            else
            {
                // hide a "don't care" frame border only if it is not clicked
                if( bHideDontCare && ((*aIt)->GetState() == FRAMESTATE_DONTCARE) )
                    mxImpl->SetBorderState( **aIt, FRAMESTATE_HIDE );

                // deselect frame borders not clicked (if SHIFT or CTRL are not pressed)
                if( !rMEvt.IsShift() && !rMEvt.IsMod1() )
                    aDeselectBorders.push_back( *aIt );
            }
        }

        if( bAnyClicked )
        {
            // any valid frame border clicked? -> deselect other frame borders
            for( FrameBorderIter aIt( aDeselectBorders ); aIt.Is(); ++aIt )
                mxImpl->SelectBorder( **aIt, false );

            if( bNewSelected || !mxImpl->SelectedBordersEqual() )
            {
                // new frame border selected, selection extended, or selected borders different? -> show
                for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
                    // SetBorderState() sets current style and color to the frame border
                    mxImpl->SetBorderState( **aIt, FRAMESTATE_SHOW );
            }
            else
            {
                // all selected frame borders are equal -> toggle state
                for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
                    mxImpl->ToggleBorderState( **aIt );
            }

            GetSelectHdl().Call( this );
        }
    }
}

void FrameSelector::KeyInput( const KeyEvent& rKEvt )
{
    bool bHandled = false;
    KeyCode aKeyCode = rKEvt.GetKeyCode();
    if( !aKeyCode.GetModifier() )
    {
        sal_uInt16 nCode = aKeyCode.GetCode();
        switch( nCode )
        {
            case KEY_SPACE:
            {
                for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
                    mxImpl->ToggleBorderState( **aIt );
                bHandled = true;
            }
            break;

            case KEY_UP:
            case KEY_DOWN:
            case KEY_LEFT:
            case KEY_RIGHT:
            {
                if( !mxImpl->maEnabBorders.empty() )
                {
                    // start from first selected frame border
                    SelFrameBorderCIter aIt( mxImpl->maEnabBorders );
                    FrameBorderType eBorder = aIt.Is() ? (*aIt)->GetType() : mxImpl->maEnabBorders.front()->GetType();

                    // search for next enabled frame border
                    do
                    {
                        eBorder = mxImpl->GetBorder( eBorder ).GetKeyboardNeighbor( nCode );
                    }
                    while( (eBorder != FRAMEBORDER_NONE) && !IsBorderEnabled( eBorder ) );

                    // select the frame border
                    if( eBorder != FRAMEBORDER_NONE )
                    {
                        DeselectAllBorders();
                        SelectBorder( eBorder );
                    }
                }
            }
            break;
        }
    }
    if( !bHandled )
        Window::KeyInput(rKEvt);
}

void FrameSelector::GetFocus()
{
    // auto-selection of a frame border, if focus reaches control, and nothing is selected
    if( mxImpl->mbAutoSelect && !IsAnyBorderSelected() && !mxImpl->maEnabBorders.empty() )
        mxImpl->SelectBorder( *mxImpl->maEnabBorders.front(), true );

    mxImpl->DoInvalidate( false );
    if( mxImpl->mxAccess.is() )
        mxImpl->mpAccess->NotifyFocusListeners( sal_True );
    Control::GetFocus();
}

void FrameSelector::LoseFocus()
{
    mxImpl->DoInvalidate( false );
    if( mxImpl->mxAccess.is() )
        mxImpl->mpAccess->NotifyFocusListeners( sal_False );
    Control::LoseFocus();
}

void FrameSelector::DataChanged( const DataChangedEvent& rDCEvt )
{
    Control::DataChanged( rDCEvt );
    if( (rDCEvt.GetType() == DATACHANGED_SETTINGS) && (rDCEvt.GetFlags() & SETTINGS_STYLE) )
        mxImpl->InitVirtualDevice();
}

void FrameSelector::Resize()
{
    Control::Resize();
    mxImpl->sizeChanged();
}

Size FrameSelector::GetOptimalSize() const
{
    return LogicToPixel(Size(61, 65), MAP_APPFONT);
}

// ============================================================================

template< typename Cont, typename Iter, typename Pred >
FrameBorderIterBase< Cont, Iter, Pred >::FrameBorderIterBase( container_type& rCont ) :
    maIt( rCont.begin() ),
    maEnd( rCont.end() )
{
    while( Is() && !maPred( *maIt ) ) ++maIt;
}

template< typename Cont, typename Iter, typename Pred >
FrameBorderIterBase< Cont, Iter, Pred >& FrameBorderIterBase< Cont, Iter, Pred >::operator++()
{
    do { ++maIt; } while( Is() && !maPred( *maIt ) );
    return *this;
}

// ============================================================================

} // namespace svx

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
