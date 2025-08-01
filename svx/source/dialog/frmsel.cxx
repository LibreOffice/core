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

#include <config_wasm_strip.h>

#include <sal/config.h>

#include <o3tl/safeint.hxx>
#include <svx/frmsel.hxx>
#include <vcl/event.hxx>
#include <sal/log.hxx>
#include <tools/debug.hxx>
#include <svtools/colorcfg.hxx>

#include <algorithm>
#include <math.h>

#include <frmselimpl.hxx>
#include <AccessibleFrameSelector.hxx>
#include <com/sun/star/accessibility/AccessibleEventId.hpp>
#include <com/sun/star/accessibility/AccessibleStateType.hpp>
#include <vcl/settings.hxx>
#include <vcl/svapp.hxx>
#include <drawinglayer/processor2d/processor2dtools.hxx>
#include <drawinglayer/processor2d/baseprocessor2d.hxx>

#include <bitmaps.hlst>

using namespace ::com::sun::star;
using namespace ::editeng;

namespace svx {

using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Any;
using ::com::sun::star::accessibility::XAccessible;
using namespace ::com::sun::star::accessibility;

// global functions from framebordertype.hxx

FrameBorderType GetFrameBorderTypeFromIndex( size_t nIndex )
{
    assert(nIndex < o3tl::make_unsigned(FRAMEBORDERTYPE_COUNT)
           && "svx::GetFrameBorderTypeFromIndex - invalid index");
    return static_cast< FrameBorderType >( nIndex + 1 );
}

size_t GetIndexFromFrameBorderType( FrameBorderType eBorder )
{
    DBG_ASSERT( eBorder != FrameBorderType::NONE,
        "svx::GetIndexFromFrameBorderType - invalid frame border type" );
    return static_cast< size_t >( eBorder ) - 1;
}

namespace
{

/** Space between outer control border and any graphical element of the control. */
const tools::Long FRAMESEL_GEOM_OUTER    = 2;

/** Space between arrows and usable inner area. */
const tools::Long FRAMESEL_GEOM_INNER    = 3;

/** Maximum width to draw a frame border style. */
const tools::Long FRAMESEL_GEOM_WIDTH    = 9;

/** Additional margin for click area of outer lines. */
const tools::Long FRAMESEL_GEOM_ADD_CLICK_OUTER = 5;

/** Additional margin for click area of inner lines. */
const tools::Long FRAMESEL_GEOM_ADD_CLICK_INNER = 2;


/** Returns the corresponding flag for a frame border. */
FrameSelFlags lclGetFlagFromType( FrameBorderType eBorder )
{
    switch( eBorder )
    {
        case FrameBorderType::Left:      return FrameSelFlags::Left;
        case FrameBorderType::Right:     return FrameSelFlags::Right;
        case FrameBorderType::Top:       return FrameSelFlags::Top;
        case FrameBorderType::Bottom:    return FrameSelFlags::Bottom;
        case FrameBorderType::Horizontal:       return FrameSelFlags::InnerHorizontal;
        case FrameBorderType::Vertical:       return FrameSelFlags::InnerVertical;
        case FrameBorderType::TLBR:      return FrameSelFlags::DiagonalTLBR;
        case FrameBorderType::BLTR:      return FrameSelFlags::DiagonalBLTR;
        case FrameBorderType::NONE : break;
    }
    return FrameSelFlags::NONE;
}

/** Merges the rSource polypolygon into the rDest polypolygon. */
void lclPolyPolyUnion( tools::PolyPolygon& rDest, const tools::PolyPolygon& rSource )
{
    const tools::PolyPolygon aTmp( rDest );
    aTmp.GetUnion( rSource, rDest );
}

} // namespace

FrameBorder::FrameBorder( FrameBorderType eType ) :
    meType( eType ),
    meState( FrameBorderState::Hide ),
    meKeyLeft( FrameBorderType::NONE ),
    meKeyRight( FrameBorderType::NONE ),
    meKeyTop( FrameBorderType::NONE ),
    meKeyBottom( FrameBorderType::NONE ),
    mbEnabled( false ),
    mbSelected( false )
{
}

void FrameBorder::Enable( FrameSelFlags nFlags )
{
    mbEnabled = bool(nFlags & lclGetFlagFromType( meType ));
    if( !mbEnabled )
        SetState( FrameBorderState::Hide );
}

void FrameBorder::SetCoreStyle( const SvxBorderLine* pStyle )
{
    if( pStyle )
        maCoreStyle = *pStyle;
    else
        maCoreStyle = SvxBorderLine();

    // from twips to points
    maUIStyle.Set( &maCoreStyle, FrameBorder::GetDefaultPatternScale(), FRAMESEL_GEOM_WIDTH );
    meState = maUIStyle.IsUsed() ? FrameBorderState::Show : FrameBorderState::Hide;
}

void FrameBorder::SetState( FrameBorderState eState )
{
    meState = eState;
    switch( meState )
    {
        case FrameBorderState::Show:
            SAL_WARN( "svx.dialog", "svx::FrameBorder::SetState - use SetCoreStyle to make border visible" );
        break;
        case FrameBorderState::Hide:
            maCoreStyle = SvxBorderLine();
            maUIStyle.Clear();
        break;
        case FrameBorderState::DontCare:
            maCoreStyle = SvxBorderLine();
            maUIStyle = frame::Style(3, 0, 0, SvxBorderLineStyle::SOLID, FrameBorder::GetDefaultPatternScale()); //OBJ_FRAMESTYLE_DONTCARE
        break;
    }
}

void FrameBorder::AddFocusPolygon( const tools::Polygon& rFocus )
{
    lclPolyPolyUnion( maFocusArea, tools::PolyPolygon(rFocus) );
}

void FrameBorder::MergeFocusToPolyPolygon( tools::PolyPolygon& rPPoly ) const
{
    lclPolyPolyUnion( rPPoly, maFocusArea );
}

void FrameBorder::AddClickRect( const tools::Rectangle& rRect )
{
    lclPolyPolyUnion( maClickArea, tools::PolyPolygon( rRect ) );
}

bool FrameBorder::ContainsClickPoint( const Point& rPos ) const
{
    return vcl::Region( maClickArea ).Contains( rPos );
}

tools::Rectangle FrameBorder::GetClickBoundRect() const
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
    FrameBorderType eBorder = FrameBorderType::NONE;
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

FrameSelectorImpl::FrameSelectorImpl( FrameSelector& rFrameSel ) :
    mrFrameSel( rFrameSel ),
    mpVirDev( VclPtr<VirtualDevice>::Create() ),
    maLeft( FrameBorderType::Left ),
    maRight( FrameBorderType::Right ),
    maTop( FrameBorderType::Top ),
    maBottom( FrameBorderType::Bottom ),
    maHor( FrameBorderType::Horizontal ),
    maVer( FrameBorderType::Vertical ),
    maTLBR( FrameBorderType::TLBR ),
    maBLTR( FrameBorderType::BLTR ),
    mnFlags( FrameSelFlags::Outer ),
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
    mbHCMode( false )
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    ,maChildVec(FRAMEBORDERTYPE_COUNT)
#endif
{
    maAllBorders.resize( FRAMEBORDERTYPE_COUNT, nullptr );
    maAllBorders[ GetIndexFromFrameBorderType( FrameBorderType::Left   ) ] = &maLeft;
    maAllBorders[ GetIndexFromFrameBorderType( FrameBorderType::Right  ) ] = &maRight;
    maAllBorders[ GetIndexFromFrameBorderType( FrameBorderType::Top    ) ] = &maTop;
    maAllBorders[ GetIndexFromFrameBorderType( FrameBorderType::Bottom ) ] = &maBottom;
    maAllBorders[ GetIndexFromFrameBorderType( FrameBorderType::Horizontal    ) ] = &maHor;
    maAllBorders[ GetIndexFromFrameBorderType( FrameBorderType::Vertical    ) ] = &maVer;
    maAllBorders[ GetIndexFromFrameBorderType( FrameBorderType::TLBR   ) ] = &maTLBR;
    maAllBorders[ GetIndexFromFrameBorderType( FrameBorderType::BLTR   ) ] = &maBLTR;
#if OSL_DEBUG_LEVEL >= 2
    {
        bool bOk = true;
        for( FrameBorderCIter aIt( maAllBorders ); bOk && aIt.Is(); bOk = (*aIt != 0), ++aIt );
        DBG_ASSERT( bOk, "svx::FrameSelectorImpl::FrameSelectorImpl - missing entry in maAllBorders" );
    }
#endif
    //                             left neighbor     right neighbor     upper neighbor    lower neighbor
    maLeft.SetKeyboardNeighbors(   FrameBorderType::NONE, FrameBorderType::TLBR,  FrameBorderType::Top,  FrameBorderType::Bottom );
    maRight.SetKeyboardNeighbors(  FrameBorderType::BLTR, FrameBorderType::NONE,  FrameBorderType::Top,  FrameBorderType::Bottom );
    maTop.SetKeyboardNeighbors(    FrameBorderType::Left, FrameBorderType::Right, FrameBorderType::NONE, FrameBorderType::TLBR );
    maBottom.SetKeyboardNeighbors( FrameBorderType::Left, FrameBorderType::Right, FrameBorderType::BLTR, FrameBorderType::NONE );
    maHor.SetKeyboardNeighbors(    FrameBorderType::Left, FrameBorderType::Right, FrameBorderType::TLBR, FrameBorderType::BLTR );
    maVer.SetKeyboardNeighbors(    FrameBorderType::TLBR, FrameBorderType::BLTR,  FrameBorderType::Top,  FrameBorderType::Bottom );
    maTLBR.SetKeyboardNeighbors(   FrameBorderType::Left, FrameBorderType::Vertical,   FrameBorderType::Top,  FrameBorderType::Horizontal );
    maBLTR.SetKeyboardNeighbors(   FrameBorderType::Vertical,  FrameBorderType::Right, FrameBorderType::Horizontal,  FrameBorderType::Bottom );

    Initialize(mnFlags);
}

FrameSelectorImpl::~FrameSelectorImpl()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    for( auto& rpChild : maChildVec )
        if( rpChild.is() )
        {
            rpChild->Invalidate();
            rpChild->dispose();
        }
#endif
}

// initialization
void FrameSelectorImpl::Initialize( FrameSelFlags nFlags )
{
    mnFlags = nFlags;

    maEnabBorders.clear();
    for (FrameBorder* pBorder : maAllBorders)
    {
        pBorder->Enable(mnFlags);
        if (pBorder->IsEnabled())
            maEnabBorders.push_back(pBorder);
    }
    mbHor = maHor.IsEnabled();
    mbVer = maVer.IsEnabled();
    mbTLBR = maTLBR.IsEnabled();
    mbBLTR = maBLTR.IsEnabled();

    InitVirtualDevice();
}

void FrameSelectorImpl::InitColors()
{
    const StyleSettings& rSettings = Application::GetSettings().GetStyleSettings();
    svtools::ColorConfig aColorConfig;
    maBackCol = aColorConfig.GetColorValue(svtools::DOCCOLOR).nColor;
    mbHCMode = rSettings.GetHighContrastMode();
    maArrowCol = aColorConfig.GetColorValue(svtools::DOCBOUNDARIES).nColor;
    maMarkCol = aColorConfig.GetColorValue(svtools::TABLEBOUNDARIES).nColor;
    maHCLineCol = COL_BLACK;
}

constexpr OUString aImageIds[] =
{
    RID_SVXBMP_FRMSEL_ARROW1,
    RID_SVXBMP_FRMSEL_ARROW2,
    RID_SVXBMP_FRMSEL_ARROW3,
    RID_SVXBMP_FRMSEL_ARROW4,
    RID_SVXBMP_FRMSEL_ARROW5,
    RID_SVXBMP_FRMSEL_ARROW6,
    RID_SVXBMP_FRMSEL_ARROW7,
    RID_SVXBMP_FRMSEL_ARROW8,
    RID_SVXBMP_FRMSEL_ARROW9,
    RID_SVXBMP_FRMSEL_ARROW10,
    RID_SVXBMP_FRMSEL_ARROW11,
    RID_SVXBMP_FRMSEL_ARROW12,
    RID_SVXBMP_FRMSEL_ARROW13,
    RID_SVXBMP_FRMSEL_ARROW14,
    RID_SVXBMP_FRMSEL_ARROW15,
    RID_SVXBMP_FRMSEL_ARROW16
};

void FrameSelectorImpl::InitArrowImageList()
{
    maArrows.clear();

    /* Build the arrow images bitmap with current colors. */
    Color pColorAry1[3];
    Color pColorAry2[3];
    pColorAry1[0] = Color( 0, 0, 0 );
    pColorAry2[0] = maArrowCol;       // black -> arrow color
    pColorAry1[1] = Color( 0, 255, 0 );
    pColorAry2[1] = maMarkCol;        // green -> marker color
    pColorAry1[2] = Color( 255, 0, 255 );
    pColorAry2[2] = maBackCol;       // magenta -> background

    assert(SAL_N_ELEMENTS(aImageIds) == 16);
    for (size_t i = 0; i < SAL_N_ELEMENTS(aImageIds); ++i)
    {
        BitmapEx aBmpEx { aImageIds[i] };
        aBmpEx.Replace(pColorAry1, pColorAry2, 3);
        maArrows.emplace_back(aBmpEx);
    }
    assert(maArrows.size() == 16);

    mnArrowSize = maArrows[0].GetSizePixel().Height();
}

void FrameSelectorImpl::InitGlobalGeometry()
{
    Size aCtrlSize(mrFrameSel.GetOutputSizePixel());
    /*  nMinSize is the lower of width and height (control will always be squarish).
        FRAMESEL_GEOM_OUTER is the minimal distance between inner control border
        and any element. */
    tools::Long nMinSize = std::min( aCtrlSize.Width(), aCtrlSize.Height() ) - 2 * FRAMESEL_GEOM_OUTER;
    /*  nFixedSize is the size all existing elements need in one direction:
        the diag. arrow, space betw. arrow and frame border, outer frame border,
        inner frame border, other outer frame border, space betw. frame border
        and arrow, the other arrow. */
    tools::Long nFixedSize = 2 * mnArrowSize + 2 * FRAMESEL_GEOM_INNER + 3 * FRAMESEL_GEOM_WIDTH;
    /*  nBetwBordersSize contains the size between an outer and inner frame border (made odd). */
    tools::Long nBetwBordersSize = (((nMinSize - nFixedSize) / 2) - 1) | 1;

    /*  The final size of the usable area. At least do not get negative */
    mnCtrlSize = 2 * nBetwBordersSize + nFixedSize;
    mnCtrlSize = std::max(mnCtrlSize, static_cast<tools::Long>(0));
    mpVirDev->SetOutputSizePixel( Size( mnCtrlSize, mnCtrlSize ) );

    /*  Center the virtual device in the control. */
    maVirDevPos = Point( (aCtrlSize.Width() - mnCtrlSize) / 2, (aCtrlSize.Height() - mnCtrlSize) / 2 );
}

void FrameSelectorImpl::InitBorderGeometry()
{
    size_t nCol, nCols, nRow, nRows;

    // Global border geometry values
    /*  mnLine* is the middle point inside a frame border (i.e. mnLine1 is mid X inside left border). */
    mnLine1 = mnArrowSize + FRAMESEL_GEOM_INNER + FRAMESEL_GEOM_WIDTH / 2;
    mnLine2 = mnCtrlSize / 2;
    mnLine3 = 2 * mnLine2 - mnLine1;

    // Frame helper array
    maArray.Initialize( mbVer ? 2 : 1, mbHor ? 2 : 1 );

    maArray.SetXOffset( mnLine1 );
    maArray.SetAllColWidths( (mbVer ? mnLine2 : mnLine3) - mnLine1 );

    maArray.SetYOffset( mnLine1 );
    maArray.SetAllRowHeights( (mbHor ? mnLine2 : mnLine3) - mnLine1 );

    // Focus polygons
    /*  Width for focus rectangles from center of frame borders. */
    mnFocusOffs = FRAMESEL_GEOM_WIDTH / 2 + 1;

    maLeft.ClearFocusArea();
    maVer.ClearFocusArea();
    maRight.ClearFocusArea();
    maTop.ClearFocusArea();
    maHor.ClearFocusArea();
    maBottom.ClearFocusArea();

    maLeft.AddFocusPolygon(   tools::Polygon(tools::Rectangle( mnLine1 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine1 + mnFocusOffs, mnLine3 + mnFocusOffs )) );
    maVer.AddFocusPolygon(    tools::Polygon(tools::Rectangle( mnLine2 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine2 + mnFocusOffs, mnLine3 + mnFocusOffs )) );
    maRight.AddFocusPolygon(  tools::Polygon(tools::Rectangle( mnLine3 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine3 + mnFocusOffs )) );
    maTop.AddFocusPolygon(    tools::Polygon(tools::Rectangle( mnLine1 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine1 + mnFocusOffs )) );
    maHor.AddFocusPolygon(    tools::Polygon(tools::Rectangle( mnLine1 - mnFocusOffs, mnLine2 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine2 + mnFocusOffs )) );
    maBottom.AddFocusPolygon( tools::Polygon(tools::Rectangle( mnLine1 - mnFocusOffs, mnLine3 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine3 + mnFocusOffs )) );

    maTLBR.ClearFocusArea();
    maBLTR.ClearFocusArea();

    for( nCol = 0, nCols = maArray.GetColCount(); nCol < nCols; ++nCol )
    {
        for( nRow = 0, nRows = maArray.GetRowCount(); nRow < nRows; ++nRow )
        {
            const basegfx::B2DRange aCellRange(maArray.GetCellRange( nCol, nRow ));
            const tools::Rectangle aRect(
                basegfx::fround<tools::Long>(aCellRange.getMinX()), basegfx::fround<tools::Long>(aCellRange.getMinY()),
                basegfx::fround<tools::Long>(aCellRange.getMaxX()), basegfx::fround<tools::Long>(aCellRange.getMaxY()));
            const double fHorDiagAngle(atan2(fabs(aCellRange.getHeight()), fabs(aCellRange.getWidth())));
            const double fVerDiagAngle(fHorDiagAngle > 0.0 ? M_PI_2 - fHorDiagAngle : 0.0);
            const tools::Long nDiagFocusOffsX(basegfx::fround<tools::Long>(-mnFocusOffs / tan(fHorDiagAngle) + mnFocusOffs / sin(fHorDiagAngle)));
            const tools::Long nDiagFocusOffsY(basegfx::fround<tools::Long>(-mnFocusOffs / tan(fVerDiagAngle) + mnFocusOffs / sin(fVerDiagAngle)));

            std::vector< Point > aFocusVec;
            aFocusVec.emplace_back( aRect.Left()  - mnFocusOffs,     aRect.Top()    + nDiagFocusOffsY );
            aFocusVec.emplace_back( aRect.Left()  - mnFocusOffs,     aRect.Top()    - mnFocusOffs     );
            aFocusVec.emplace_back( aRect.Left()  + nDiagFocusOffsX, aRect.Top()    - mnFocusOffs     );
            aFocusVec.emplace_back( aRect.Right() + mnFocusOffs,     aRect.Bottom() - nDiagFocusOffsY );
            aFocusVec.emplace_back( aRect.Right() + mnFocusOffs,     aRect.Bottom() + mnFocusOffs     );
            aFocusVec.emplace_back( aRect.Right() - nDiagFocusOffsX, aRect.Bottom() + mnFocusOffs     );
            maTLBR.AddFocusPolygon( tools::Polygon( static_cast< sal_uInt16 >( aFocusVec.size() ), aFocusVec.data() ) );

            aFocusVec.clear();
            aFocusVec.emplace_back( aRect.Right() + mnFocusOffs,     aRect.Top()    + nDiagFocusOffsY );
            aFocusVec.emplace_back( aRect.Right() + mnFocusOffs,     aRect.Top()    - mnFocusOffs     );
            aFocusVec.emplace_back( aRect.Right() - nDiagFocusOffsX, aRect.Top()    - mnFocusOffs     );
            aFocusVec.emplace_back( aRect.Left()  - mnFocusOffs,     aRect.Bottom() - nDiagFocusOffsY );
            aFocusVec.emplace_back( aRect.Left()  - mnFocusOffs,     aRect.Bottom() + mnFocusOffs     );
            aFocusVec.emplace_back( aRect.Left()  + nDiagFocusOffsX, aRect.Bottom() + mnFocusOffs     );
            maBLTR.AddFocusPolygon( tools::Polygon( static_cast< sal_uInt16 >( aFocusVec.size() ), aFocusVec.data() ) );
        }
    }

    // Click areas
    for (FrameBorder* pBorder : maAllBorders)
        pBorder->ClearClickArea();

    /*  Additional space for click area: is added to the space available to draw
        the frame borders. For instance left frame border:
        - To left, top, and bottom always big additional space (outer area).
        - To right: Dependent on existence of inner vertical frame border
            (if enabled, use less space).
     */
    tools::Long nClO = FRAMESEL_GEOM_WIDTH / 2 + FRAMESEL_GEOM_ADD_CLICK_OUTER;
    tools::Long nClI = (mbTLBR && mbBLTR) ? (FRAMESEL_GEOM_WIDTH / 2 + FRAMESEL_GEOM_ADD_CLICK_INNER) : nClO;
    tools::Long nClH = mbHor ? nClI : nClO;            // additional space dependent of horizontal inner border
    tools::Long nClV = mbVer ? nClI : nClO;            // additional space dependent of vertical inner border

    maLeft.AddClickRect(   tools::Rectangle( mnLine1 - nClO, mnLine1 - nClO, mnLine1 + nClV, mnLine3 + nClO ) );
    maVer.AddClickRect(    tools::Rectangle( mnLine2 - nClI, mnLine1 - nClO, mnLine2 + nClI, mnLine3 + nClO ) );
    maRight.AddClickRect(  tools::Rectangle( mnLine3 - nClV, mnLine1 - nClO, mnLine3 + nClO, mnLine3 + nClO ) );
    maTop.AddClickRect(    tools::Rectangle( mnLine1 - nClO, mnLine1 - nClO, mnLine3 + nClO, mnLine1 + nClH ) );
    maHor.AddClickRect(    tools::Rectangle( mnLine1 - nClO, mnLine2 - nClI, mnLine3 + nClO, mnLine2 + nClI ) );
    maBottom.AddClickRect( tools::Rectangle( mnLine1 - nClO, mnLine3 - nClH, mnLine3 + nClO, mnLine3 + nClO ) );

    /*  Diagonal frame borders use the remaining space between outer and inner frame borders. */
    if( !(mbTLBR || mbBLTR) )
        return;

    for( nCol = 0, nCols = maArray.GetColCount(); nCol < nCols; ++nCol )
    {
        for( nRow = 0, nRows = maArray.GetRowCount(); nRow < nRows; ++nRow )
        {
            // the usable area between horizontal/vertical frame borders of current quadrant
            const basegfx::B2DRange aCellRange(maArray.GetCellRange( nCol, nRow ));
            const tools::Rectangle aRect(
                basegfx::fround<tools::Long>(aCellRange.getMinX()) + nClV + 1, basegfx::fround<tools::Long>(aCellRange.getMinY()) + nClH + 1,
                basegfx::fround<tools::Long>(aCellRange.getMaxX()) - nClV + 1, basegfx::fround<tools::Long>(aCellRange.getMaxY()) - nClH + 1);

            /*  Both diagonal frame borders enabled. */
            if( mbTLBR && mbBLTR )
            {
                // single areas
                Point aMid( aRect.Center() );
                maTLBR.AddClickRect( tools::Rectangle( aRect.TopLeft(), aMid ) );
                maTLBR.AddClickRect( tools::Rectangle( aMid + Point( 1, 1 ), aRect.BottomRight() ) );
                maBLTR.AddClickRect( tools::Rectangle( aRect.Left(), aMid.Y() + 1, aMid.X(), aRect.Bottom() ) );
                maBLTR.AddClickRect( tools::Rectangle( aMid.X() + 1, aRect.Top(), aRect.Right(), aMid.Y() ) );
                // centered rectangle for both frame borders
                tools::Rectangle aMidRect( aRect.TopLeft(), Size( aRect.GetWidth() / 3, aRect.GetHeight() / 3 ) );
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

    DoInvalidate( true );
}

// frame border access
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

// drawing
void FrameSelectorImpl::DrawBackground()
{
    // clear the area
    mpVirDev->SetLineColor();
    mpVirDev->SetFillColor( maBackCol );
    mpVirDev->DrawRect( tools::Rectangle( Point( 0, 0 ), mpVirDev->GetOutputSizePixel() ) );

    // draw the inner gray (or whatever color) rectangle
    mpVirDev->SetLineColor();
    mpVirDev->SetFillColor( maMarkCol );
    mpVirDev->DrawRect( tools::Rectangle(
        mnLine1 - mnFocusOffs, mnLine1 - mnFocusOffs, mnLine3 + mnFocusOffs, mnLine3 + mnFocusOffs ) );

    // draw the white space for enabled frame borders
    tools::PolyPolygon aPPoly;
    for (const FrameBorder* pBorder : maEnabBorders)
        pBorder->MergeFocusToPolyPolygon(aPPoly);
    aPPoly.Optimize( PolyOptimizeFlags::CLOSE );
    mpVirDev->SetLineColor( maBackCol );
    mpVirDev->SetFillColor( maBackCol );
    mpVirDev->DrawPolyPolygon( aPPoly );
}

void FrameSelectorImpl::DrawArrows( const FrameBorder& rBorder )
{
    DBG_ASSERT( rBorder.IsEnabled(), "svx::FrameSelectorImpl::DrawArrows - access to disabled border" );

    tools::Long nLinePos = 0;
    switch( rBorder.GetType() )
    {
        case FrameBorderType::Left:
        case FrameBorderType::Top:       nLinePos = mnLine1; break;
        case FrameBorderType::Vertical:
        case FrameBorderType::Horizontal:       nLinePos = mnLine2; break;
        case FrameBorderType::Right:
        case FrameBorderType::Bottom:    nLinePos = mnLine3; break;
        default: ; //prevent warning
    }
    nLinePos -= mnArrowSize / 2;

    tools::Long nTLPos = 0;
    tools::Long nBRPos = mnCtrlSize - mnArrowSize;
    Point aPos1, aPos2;
    int nImgIndex1 = -1, nImgIndex2 = -1;
    switch( rBorder.GetType() )
    {
        case FrameBorderType::Left:
        case FrameBorderType::Right:
        case FrameBorderType::Vertical:
            aPos1 = Point( nLinePos, nTLPos ); nImgIndex1 = 0;
            aPos2 = Point( nLinePos, nBRPos ); nImgIndex2 = 1;
        break;

        case FrameBorderType::Top:
        case FrameBorderType::Bottom:
        case FrameBorderType::Horizontal:
            aPos1 = Point( nTLPos, nLinePos ); nImgIndex1 = 2;
            aPos2 = Point( nBRPos, nLinePos ); nImgIndex2 = 3;
        break;

        case FrameBorderType::TLBR:
            aPos1 = Point( nTLPos, nTLPos ); nImgIndex1 = 4;
            aPos2 = Point( nBRPos, nBRPos ); nImgIndex2 = 5;
        break;
        case FrameBorderType::BLTR:
            aPos1 = Point( nTLPos, nBRPos ); nImgIndex1 = 6;
            aPos2 = Point( nBRPos, nTLPos ); nImgIndex2 = 7;
        break;
        default: ; //prevent warning
    }

    // Arrow or marker? Do not draw arrows into disabled control.
    sal_uInt16 nSelectAdd = (mrFrameSel.IsEnabled() && rBorder.IsSelected()) ? 0 : 8;
    if (nImgIndex1 >= 0)
        mpVirDev->DrawImage(aPos1, maArrows[nImgIndex1 + nSelectAdd]);
    if (nImgIndex2 >= 0)
        mpVirDev->DrawImage(aPos2, maArrows[nImgIndex2 + nSelectAdd]);
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
    for (FrameBorder* pBorder : maEnabBorders)
    {
        Color aCoreColorPrim = (pBorder->GetState() == FrameBorderState::DontCare) ? maMarkCol : pBorder->GetCoreStyle().GetColorOut();
        Color aCoreColorSecn = (pBorder->GetState() == FrameBorderState::DontCare) ? maMarkCol : pBorder->GetCoreStyle().GetColorIn();
        pBorder->SetUIColorPrim(GetDrawLineColor(aCoreColorPrim));
        pBorder->SetUIColorSecn(GetDrawLineColor(aCoreColorSecn));
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
            rRightStyle.Type( ), rRightStyle.PatternScale() );
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
            rHorStyle.Type(), rHorStyle.PatternScale() );
        maArray.SetRowStyleTop( 1, rInvertedHor );
    }

    // Invert the style for the bottom line
    const frame::Style rBottomStyle = maBottom.GetUIStyle( );
    frame::Style rInvertedBottom( rBottomStyle.GetColorPrim(),
            rBottomStyle.GetColorSecn(), rBottomStyle.GetColorGap(),
            rBottomStyle.UseGapColor(),
            rBottomStyle.Secn(), rBottomStyle.Dist(), rBottomStyle.Prim( ),
            rBottomStyle.Type(), rBottomStyle.PatternScale() );
    maArray.SetRowStyleBottom( mbHor ? 1 : 0, rInvertedBottom );

    for( sal_Int32 nCol = 0; nCol < maArray.GetColCount(); ++nCol )
        for( sal_Int32 nRow = 0; nRow < maArray.GetRowCount(); ++nRow )
            maArray.SetCellStyleDiag( nCol, nRow, maTLBR.GetUIStyle(), maBLTR.GetUIStyle() );

    // This is used in the dialog/control for 'Border' attributes. When using
    // the original paint below instead of primitives, the advantage currently
    // is the correct visualization of diagonal line(s) including overlaying,
    // but the rest is bad. Since the edit views use primitives and the preview
    // should be 'real' I opt for also changing this to primitives. I will
    // keep the old solution and add a switch (above) based on a static bool so
    // that interested people may test this out in the debugger.
    // This is one more hint to enhance the primitive visualization further to
    // support diagonals better - that's the way to go.
    const drawinglayer::geometry::ViewInformation2D aNewViewInformation2D;
    std::unique_ptr<drawinglayer::processor2d::BaseProcessor2D> pProcessor2D(
        drawinglayer::processor2d::createPixelProcessor2DFromOutputDevice(
            *mpVirDev,
            aNewViewInformation2D));

    pProcessor2D->process(maArray.CreateB2DPrimitiveArray());
    pProcessor2D.reset();
}

void FrameSelectorImpl::DrawVirtualDevice()
{
    DrawBackground();
    for (const FrameBorder* pBorder : maEnabBorders)
        DrawArrows(*pBorder);
    DrawAllFrameBorders();
    mbFullRepaint = false;
}

void FrameSelectorImpl::CopyVirDevToControl(vcl::RenderContext& rRenderContext)
{
    if (mbFullRepaint)
        DrawVirtualDevice();
    rRenderContext.DrawBitmapEx(maVirDevPos, mpVirDev->GetBitmap(Point(0, 0), mpVirDev->GetOutputSizePixel()));
}

void FrameSelectorImpl::DrawAllTrackingRects(vcl::RenderContext& rRenderContext)
{
    tools::PolyPolygon aPPoly;
    if (mrFrameSel.IsAnyBorderSelected())
    {
        for(SelFrameBorderCIter aIt( maEnabBorders ); aIt.Is(); ++aIt)
            (*aIt)->MergeFocusToPolyPolygon(aPPoly);
        aPPoly.Move(maVirDevPos.X(), maVirDevPos.Y());
    }
    else
        // no frame border selected -> draw tracking rectangle around entire control
        aPPoly.Insert( tools::Polygon(tools::Rectangle(maVirDevPos, mpVirDev->GetOutputSizePixel())));

    aPPoly.Optimize(PolyOptimizeFlags::CLOSE);

    for(sal_uInt16 nIdx = 0, nCount = aPPoly.Count(); nIdx < nCount; ++nIdx)
        rRenderContext.Invert(aPPoly.GetObject(nIdx), InvertFlags::TrackFrame);
}

Point FrameSelectorImpl::GetDevPosFromMousePos( const Point& rMousePos ) const
{
    return rMousePos - maVirDevPos;
}

void FrameSelectorImpl::DoInvalidate( bool bFullRepaint )
{
    mbFullRepaint |= bFullRepaint;
    mrFrameSel.Invalidate();
}

// frame border state and style
void FrameSelectorImpl::SetBorderState( FrameBorder& rBorder, FrameBorderState eState )
{
    DBG_ASSERT( rBorder.IsEnabled(), "svx::FrameSelectorImpl::SetBorderState - access to disabled border" );

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    Any aOld;
    Any aNew;
    Any& rMod = eState == FrameBorderState::Show ? aNew : aOld;
    rMod <<= AccessibleStateType::CHECKED;
    rtl::Reference< a11y::AccFrameSelectorChild > xRet;
    size_t nVecIdx = static_cast< size_t >( rBorder.GetType() );
    if( GetBorder(rBorder.GetType()).IsEnabled() && (1 <= nVecIdx) && (nVecIdx <= maChildVec.size()) )
        xRet = maChildVec[ --nVecIdx ].get();
#endif

    if( eState == FrameBorderState::Show )
        SetBorderCoreStyle( rBorder, &maCurrStyle );
    else
        rBorder.SetState( eState );

#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if (xRet.is())
        xRet->NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOld, aNew );
#endif

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
        case FrameBorderState::Show:
            SetBorderState( rBorder, bDontCare ? FrameBorderState::DontCare : FrameBorderState::Hide );
        break;
        case FrameBorderState::Hide:
            SetBorderState( rBorder, FrameBorderState::Show );
        break;
        case FrameBorderState::DontCare:
            SetBorderState( rBorder, FrameBorderState::Hide );
        break;
    }
}

// frame border selection
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

FrameSelector::FrameSelector()
{
}

void FrameSelector::SetDrawingArea(weld::DrawingArea* pDrawingArea)
{
    CustomWidgetController::SetDrawingArea(pDrawingArea);
    mxImpl.reset( new FrameSelectorImpl( *this ) );
    Size aPrefSize = pDrawingArea->get_ref_device().LogicToPixel(Size(61, 65), MapMode(MapUnit::MapAppFont));
    pDrawingArea->set_size_request(aPrefSize.Width(), aPrefSize.Height());
    EnableRTL( false ); // #107808# don't mirror the mouse handling
}

FrameSelector::~FrameSelector()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if( mxAccess.is() )
        mxAccess->Invalidate();
#endif
}

void FrameSelector::Initialize( FrameSelFlags nFlags )
{
    mxImpl->Initialize( nFlags );
    Show();
}

// enabled frame borders
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
    FrameBorderType eBorder = FrameBorderType::NONE;
    if( nIndex >= 0 )
    {
        size_t nVecIdx = static_cast< size_t >( nIndex );
        if( nVecIdx < mxImpl->maEnabBorders.size() )
            eBorder = mxImpl->maEnabBorders[ nVecIdx ]->GetType();
    }
    return eBorder;
}

// frame border state and style
bool FrameSelector::SupportsDontCareState() const
{
    return bool(mxImpl->mnFlags & FrameSelFlags::DontCare);
}

FrameBorderState FrameSelector::GetFrameBorderState( FrameBorderType eBorder ) const
{
    return mxImpl->GetBorder( eBorder ).GetState();
}

const SvxBorderLine* FrameSelector::GetFrameBorderStyle( FrameBorderType eBorder ) const
{
    const SvxBorderLine& rStyle = mxImpl->GetBorder( eBorder ).GetCoreStyle();
    // rest of the world uses null pointer for invisible frame border
    return rStyle.GetOutWidth() ? &rStyle : nullptr;
}

void FrameSelector::ShowBorder( FrameBorderType eBorder, const SvxBorderLine* pStyle )
{
    mxImpl->SetBorderCoreStyle( mxImpl->GetBorderAccess( eBorder ), pStyle );
}

void FrameSelector::SetBorderDontCare( FrameBorderType eBorder )
{
    mxImpl->SetBorderState( mxImpl->GetBorderAccess( eBorder ), FrameBorderState::DontCare );
}

bool FrameSelector::IsAnyBorderVisible() const
{
    return std::any_of(mxImpl->maEnabBorders.begin(), mxImpl->maEnabBorders.end(),
                       [](const FrameBorder* pBorder)
                       { return pBorder->GetState() == FrameBorderState::Show; });
}

void FrameSelector::HideAllBorders()
{
    for (FrameBorder* pBorder : mxImpl->maEnabBorders)
        mxImpl->SetBorderState(*pBorder, FrameBorderState::Hide);
}

bool FrameSelector::GetVisibleWidth( tools::Long& rnWidth, SvxBorderLineStyle& rnStyle ) const
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

// frame border selection
const Link<LinkParamNone*,void>& FrameSelector::GetSelectHdl() const
{
    return mxImpl->maSelectHdl;
}

void FrameSelector::SetSelectHdl( const Link<LinkParamNone*,void>& rHdl )
{
    mxImpl->maSelectHdl = rHdl;
}

bool FrameSelector::IsBorderSelected( FrameBorderType eBorder ) const
{
    return mxImpl->GetBorder( eBorder ).IsSelected();
}

void FrameSelector::SelectBorder(FrameBorderType eBorder, bool bFocus)
{
    mxImpl->SelectBorder( mxImpl->GetBorderAccess( eBorder ), true/*bSelect*/ );
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if (bFocus)
    {
        rtl::Reference< a11y::AccFrameSelectorChild > xRet = GetChildAccessible(eBorder);
        if (xRet.is())
        {
            Any aOldValue, aNewValue;
            aNewValue <<= AccessibleStateType::FOCUSED;
            xRet->NotifyAccessibleEvent( AccessibleEventId::STATE_CHANGED, aOldValue, aNewValue );
        }
    }
#else
    (void) bFocus;
#endif
}

bool FrameSelector::IsAnyBorderSelected() const
{
    // Construct an iterator for selected borders. If it is valid, there is a selected border.
    return SelFrameBorderCIter( mxImpl->maEnabBorders ).Is();
}

void FrameSelector::SelectAllBorders( bool bSelect )
{
    for (FrameBorder* pBorder : mxImpl->maEnabBorders)
        mxImpl->SelectBorder(*pBorder, bSelect);
}

void FrameSelector::SelectAllVisibleBorders()
{
    for( VisFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
        mxImpl->SelectBorder( **aIt, true/*bSelect*/ );
}

void FrameSelector::SetStyleToSelection( tools::Long nWidth, SvxBorderLineStyle nStyle )
{
    mxImpl->maCurrStyle.SetBorderLineStyle( nStyle );
    mxImpl->maCurrStyle.SetWidth( nWidth );
    for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
        mxImpl->SetBorderState( **aIt, FrameBorderState::Show );
}

void FrameSelector::SetColorToSelection(const Color& rColor, model::ComplexColor const& rComplexColor)
{
    mxImpl->maCurrStyle.SetColor(rColor);
    mxImpl->maCurrStyle.setComplexColor(rComplexColor);

    for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
        mxImpl->SetBorderState( **aIt, FrameBorderState::Show );
}

SvxBorderLineStyle FrameSelector::getCurrentStyleLineStyle() const
{
    return mxImpl->maCurrStyle.GetBorderLineStyle();
}

// accessibility
rtl::Reference<comphelper::OAccessible> FrameSelector::CreateAccessible()
{
#if !ENABLE_WASM_STRIP_ACCESSIBILITY
    if( !mxAccess.is() )
        mxAccess = new a11y::AccFrameSelector(*this);
#endif
    return mxAccess;
}

rtl::Reference< a11y::AccFrameSelectorChild > FrameSelector::GetChildAccessible( FrameBorderType eBorder )
{
    rtl::Reference< a11y::AccFrameSelectorChild > xRet;
    size_t nVecIdx = static_cast< size_t >( eBorder );
    if( IsBorderEnabled( eBorder ) && (1 <= nVecIdx) && (nVecIdx <= mxImpl->maChildVec.size()) )
    {
        --nVecIdx;
        if( !mxImpl->maChildVec[ nVecIdx ].is() )
            mxImpl->maChildVec[ nVecIdx ] = new a11y::AccFrameSelectorChild( *this, eBorder );
        xRet = mxImpl->maChildVec[ nVecIdx ].get();
    }
    return xRet;
}

rtl::Reference<comphelper::OAccessible> FrameSelector::GetChildAccessible(sal_Int32 nIndex)
{
    return GetChildAccessible( GetEnabledBorderType( nIndex ) );
}

rtl::Reference<comphelper::OAccessible> FrameSelector::GetChildAccessible(const Point& rPos)
{
    for (const FrameBorder* pBorder : mxImpl->maEnabBorders)
    {
        if (pBorder->ContainsClickPoint(rPos))
        {
            rtl::Reference<a11y::AccFrameSelectorChild> pChild = GetChildAccessible(pBorder->GetType());
            if (pChild.is())
                return pChild;
        }
    }

    return {};
}

tools::Rectangle FrameSelector::GetClickBoundRect( FrameBorderType eBorder ) const
{
    tools::Rectangle aRect;
    const FrameBorder& rBorder = mxImpl->GetBorder( eBorder );
    if( rBorder.IsEnabled() )
        aRect = rBorder.GetClickBoundRect();
    return aRect;
}

// virtual functions from base class
void FrameSelector::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle&)
{
    mxImpl->CopyVirDevToControl(rRenderContext);
    if (HasFocus())
        mxImpl->DrawAllTrackingRects(rRenderContext);
}

bool FrameSelector::MouseButtonDown( const MouseEvent& rMEvt )
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
        bool bHideDontCare = !SupportsDontCareState();

        for (FrameBorder* pBorder : mxImpl->maEnabBorders)
        {
            if (pBorder->ContainsClickPoint(aPos))
            {
                // frame border is clicked
                bAnyClicked = true;
                if (!pBorder->IsSelected())
                {
                    bNewSelected = true;
                    SelectBorder(pBorder->GetType(), true);
                }
            }
            else
            {
                // hide a "don't care" frame border only if it is not clicked
                if (bHideDontCare && (pBorder->GetState() == FrameBorderState::DontCare))
                    mxImpl->SetBorderState(*pBorder, FrameBorderState::Hide);

                // deselect frame borders not clicked (if SHIFT or CTRL are not pressed)
                if( !rMEvt.IsShift() && !rMEvt.IsMod1() )
                    aDeselectBorders.push_back(pBorder);
            }
        }

        if( bAnyClicked )
        {
            // any valid frame border clicked? -> deselect other frame borders
            for (FrameBorder* pBorder : aDeselectBorders)
                mxImpl->SelectBorder(*pBorder, false);

            if( bNewSelected || !mxImpl->SelectedBordersEqual() )
            {
                // new frame border selected, selection extended, or selected borders different? -> show
                for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
                    // SetBorderState() sets current style and color to the frame border
                    mxImpl->SetBorderState( **aIt, FrameBorderState::Show );
            }
            else
            {
                // all selected frame borders are equal -> toggle state
                for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
                    mxImpl->ToggleBorderState( **aIt );
            }

            GetSelectHdl().Call( nullptr );
        }
    }

    return true;
}

bool FrameSelector::KeyInput( const KeyEvent& rKEvt )
{
    bool bHandled = false;
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();
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
                    while( (eBorder != FrameBorderType::NONE) && !IsBorderEnabled( eBorder ) );

                    // select the frame border
                    if( eBorder != FrameBorderType::NONE )
                    {
                        DeselectAllBorders();
                        SelectBorder(eBorder, true);
                    }
                    bHandled = true;
                }
            }
            break;
        }
    }
    if (bHandled)
        return true;
    return CustomWidgetController::KeyInput(rKEvt);
}

void FrameSelector::GetFocus()
{
    // auto-selection of a frame border, if focus reaches control, and nothing is selected
    if( mxImpl->mbAutoSelect && !IsAnyBorderSelected() && !mxImpl->maEnabBorders.empty() )
        mxImpl->SelectBorder( *mxImpl->maEnabBorders.front(), true );

    mxImpl->DoInvalidate( false );
    if (IsAnyBorderSelected())
    {
        FrameBorderType borderType = FrameBorderType::NONE;
        if (mxImpl->maLeft.IsSelected())
            borderType = FrameBorderType::Left;
        else if (mxImpl->maRight.IsSelected())
            borderType = FrameBorderType::Right;
        else if (mxImpl->maTop.IsSelected())
            borderType = FrameBorderType::Top;
        else if (mxImpl->maBottom.IsSelected())
            borderType = FrameBorderType::Bottom;
        else if (mxImpl->maHor.IsSelected())
            borderType = FrameBorderType::Horizontal;
        else if (mxImpl->maVer.IsSelected())
            borderType = FrameBorderType::Vertical;
        else if (mxImpl->maTLBR.IsSelected())
            borderType = FrameBorderType::TLBR;
        else if (mxImpl->maBLTR.IsSelected())
            borderType = FrameBorderType::BLTR;
        SelectBorder(borderType, true);
    }
    for( SelFrameBorderIter aIt( mxImpl->maEnabBorders ); aIt.Is(); ++aIt )
            mxImpl->SetBorderState( **aIt, FrameBorderState::Show );
    CustomWidgetController::GetFocus();
}

void FrameSelector::LoseFocus()
{
    mxImpl->DoInvalidate( false );
    CustomWidgetController::LoseFocus();
}

void FrameSelector::StyleUpdated()
{
    mxImpl->InitVirtualDevice();
    CustomWidgetController::StyleUpdated();
}

void FrameSelector::Resize()
{
    CustomWidgetController::Resize();
    mxImpl->sizeChanged();
}

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

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
