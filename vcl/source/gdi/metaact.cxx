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

#include <algorithm>
#include <string.h>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/helpers.hxx>
#include <vcl/outdev.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphictools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>

// ========================================================================

inline void ImplScalePoint( Point& rPt, double fScaleX, double fScaleY )
{
    rPt.X() = FRound( fScaleX * rPt.X() );
    rPt.Y() = FRound( fScaleY * rPt.Y() );
}

// ------------------------------------------------------------------------

inline void ImplScaleRect( Rectangle& rRect, double fScaleX, double fScaleY )
{
    Point aTL( rRect.TopLeft() );
    Point aBR( rRect.BottomRight() );

    ImplScalePoint( aTL, fScaleX, fScaleY );
    ImplScalePoint( aBR, fScaleX, fScaleY );

    rRect = Rectangle( aTL, aBR );
    rRect.Justify();
}

// ------------------------------------------------------------------------

inline void ImplScalePoly( Polygon& rPoly, double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
        ImplScalePoint( rPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

inline void ImplScaleLineInfo( LineInfo& rLineInfo, double fScaleX, double fScaleY )
{
    if( !rLineInfo.IsDefault() )
    {
        const double fScale = ( fabs(fScaleX) + fabs(fScaleY) ) * 0.5;

        rLineInfo.SetWidth( FRound( fScale * rLineInfo.GetWidth() ) );
        rLineInfo.SetDashLen( FRound( fScale * rLineInfo.GetDashLen() ) );
        rLineInfo.SetDotLen( FRound( fScale * rLineInfo.GetDotLen() ) );
        rLineInfo.SetDistance( FRound( fScale * rLineInfo.GetDistance() ) );
    }
}

// ========================================================================

#define COMPAT( _def_rIStm ) VersionCompat aCompat( ( _def_rIStm ), STREAM_READ );
#define WRITE_BASE_COMPAT( _def_rOStm, _def_nVer, _pWriteData )         \
    MetaAction::Write( ( _def_rOStm ), _pWriteData );                   \
    VersionCompat aCompat( ( _def_rOStm ), STREAM_WRITE, ( _def_nVer ) );

// ========================================================================

MetaAction::MetaAction() :
    mnRefCount( 1 ),
    mnType( META_NULL_ACTION )
{
}

// ------------------------------------------------------------------------

MetaAction::MetaAction( sal_uInt16 nType ) :
    mnRefCount( 1 ),
    mnType( nType )
{
}

// ------------------------------------------------------------------------

MetaAction::~MetaAction()
{
}

// ------------------------------------------------------------------------

void MetaAction::Execute( OutputDevice* )
{
}

// ------------------------------------------------------------------------

MetaAction* MetaAction::Clone()
{
    return new MetaAction;
}

// ------------------------------------------------------------------------

void MetaAction::Move( long, long )
{
}

// ------------------------------------------------------------------------

void MetaAction::Scale( double, double )
{
}

// ------------------------------------------------------------------------

sal_Bool MetaAction::Compare( const MetaAction& ) const
{
    return sal_True;
}

// ------------------------------------------------------------------------

void MetaAction::Write( SvStream& rOStm, ImplMetaWriteData* )
{
    rOStm << mnType;
}

// ------------------------------------------------------------------------

void MetaAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    rIStm >> mnType;
}

// ------------------------------------------------------------------------

MetaAction* MetaAction::ReadMetaAction( SvStream& rIStm, ImplMetaReadData* pData )
{
    MetaAction* pAction = NULL;
    sal_uInt16      nType;

    rIStm >> nType;

    switch( nType )
    {
        case( META_NULL_ACTION ): pAction = new MetaAction; break;
        case( META_PIXEL_ACTION ): pAction = new MetaPixelAction; break;
        case( META_POINT_ACTION ): pAction = new MetaPointAction; break;
        case( META_LINE_ACTION ): pAction = new MetaLineAction; break;
        case( META_RECT_ACTION ): pAction = new MetaRectAction; break;
        case( META_ROUNDRECT_ACTION ): pAction = new MetaRoundRectAction; break;
        case( META_ELLIPSE_ACTION ): pAction = new MetaEllipseAction; break;
        case( META_ARC_ACTION ): pAction = new MetaArcAction; break;
        case( META_PIE_ACTION ): pAction = new MetaPieAction; break;
        case( META_CHORD_ACTION ): pAction = new MetaChordAction; break;
        case( META_POLYLINE_ACTION ): pAction = new MetaPolyLineAction; break;
        case( META_POLYGON_ACTION ): pAction = new MetaPolygonAction; break;
        case( META_POLYPOLYGON_ACTION ): pAction = new MetaPolyPolygonAction; break;
        case( META_TEXT_ACTION ): pAction = new MetaTextAction; break;
        case( META_TEXTARRAY_ACTION ): pAction = new MetaTextArrayAction; break;
        case( META_STRETCHTEXT_ACTION ): pAction = new MetaStretchTextAction; break;
        case( META_TEXTRECT_ACTION ): pAction = new MetaTextRectAction; break;
        case( META_TEXTLINE_ACTION ): pAction = new MetaTextLineAction; break;
        case( META_BMP_ACTION ): pAction = new MetaBmpAction; break;
        case( META_BMPSCALE_ACTION ): pAction = new MetaBmpScaleAction; break;
        case( META_BMPSCALEPART_ACTION ): pAction = new MetaBmpScalePartAction; break;
        case( META_BMPEX_ACTION ): pAction = new MetaBmpExAction; break;
        case( META_BMPEXSCALE_ACTION ): pAction = new MetaBmpExScaleAction; break;
        case( META_BMPEXSCALEPART_ACTION ): pAction = new MetaBmpExScalePartAction; break;
        case( META_MASK_ACTION ): pAction = new MetaMaskAction; break;
        case( META_MASKSCALE_ACTION ): pAction = new MetaMaskScaleAction; break;
        case( META_MASKSCALEPART_ACTION ): pAction = new MetaMaskScalePartAction; break;
        case( META_GRADIENT_ACTION ): pAction = new MetaGradientAction; break;
        case( META_GRADIENTEX_ACTION ): pAction = new MetaGradientExAction; break;
        case( META_HATCH_ACTION ): pAction = new MetaHatchAction; break;
        case( META_WALLPAPER_ACTION ): pAction = new MetaWallpaperAction; break;
        case( META_CLIPREGION_ACTION ): pAction = new MetaClipRegionAction; break;
        case( META_ISECTRECTCLIPREGION_ACTION ): pAction = new MetaISectRectClipRegionAction; break;
        case( META_ISECTREGIONCLIPREGION_ACTION ): pAction = new MetaISectRegionClipRegionAction; break;
        case( META_MOVECLIPREGION_ACTION ): pAction = new MetaMoveClipRegionAction; break;
        case( META_LINECOLOR_ACTION ): pAction = new MetaLineColorAction; break;
        case( META_FILLCOLOR_ACTION ): pAction = new MetaFillColorAction; break;
        case( META_TEXTCOLOR_ACTION ): pAction = new MetaTextColorAction; break;
        case( META_TEXTFILLCOLOR_ACTION ): pAction = new MetaTextFillColorAction; break;
        case( META_TEXTLINECOLOR_ACTION ): pAction = new MetaTextLineColorAction; break;
        case( META_OVERLINECOLOR_ACTION ): pAction = new MetaOverlineColorAction; break;
        case( META_TEXTALIGN_ACTION ): pAction = new MetaTextAlignAction; break;
        case( META_MAPMODE_ACTION ): pAction = new MetaMapModeAction; break;
        case( META_FONT_ACTION ): pAction = new MetaFontAction; break;
        case( META_PUSH_ACTION ): pAction = new MetaPushAction; break;
        case( META_POP_ACTION ): pAction = new MetaPopAction; break;
        case( META_RASTEROP_ACTION ): pAction = new MetaRasterOpAction; break;
        case( META_TRANSPARENT_ACTION ): pAction = new MetaTransparentAction; break;
        case( META_FLOATTRANSPARENT_ACTION ): pAction = new MetaFloatTransparentAction; break;
        case( META_EPS_ACTION ): pAction = new MetaEPSAction; break;
        case( META_REFPOINT_ACTION ): pAction = new MetaRefPointAction; break;
        case( META_COMMENT_ACTION ): pAction = new MetaCommentAction; break;
        case( META_LAYOUTMODE_ACTION ): pAction = new MetaLayoutModeAction; break;
        case( META_TEXTLANGUAGE_ACTION ): pAction = new MetaTextLanguageAction; break;

        default:
        {
            // Action ueberlesen durch Kombination Ctor/Dtor,
            // new/delete, weil Compiler sonst vielleicht wegoptimieren
            delete ( new VersionCompat( rIStm, STREAM_READ ) );
        }
        break;
    }

    if( pAction )
        pAction->Read( rIStm, pData );

    return pAction;
}

// ========================================================================

IMPL_META_ACTION( Pixel, META_PIXEL_ACTION )

// ------------------------------------------------------------------------

MetaPixelAction::MetaPixelAction( const Point& rPt, const Color& rColor ) :
    MetaAction  ( META_PIXEL_ACTION ),
    maPt        ( rPt ),
    maColor     ( rColor )
{
}

// ------------------------------------------------------------------------

void MetaPixelAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPixel( maPt, maColor );
}

// ------------------------------------------------------------------------

MetaAction* MetaPixelAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPixelAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaPixelAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaPixelAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaPixelAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPt == ((MetaPixelAction&)rMetaAction).maPt ) &&
           ( maColor == ((MetaPixelAction&)rMetaAction).maColor );
}

// ------------------------------------------------------------------------

void MetaPixelAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maPt;
    maColor.Write( rOStm, sal_True );
}

// ------------------------------------------------------------------------

void MetaPixelAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maPt;
    maColor.Read( rIStm, sal_True );
}

// ========================================================================

IMPL_META_ACTION( Point, META_POINT_ACTION )

// ------------------------------------------------------------------------

MetaPointAction::MetaPointAction( const Point& rPt ) :
    MetaAction  ( META_POINT_ACTION ),
    maPt        ( rPt )
{
}

// ------------------------------------------------------------------------

void MetaPointAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPixel( maPt );
}

// ------------------------------------------------------------------------

MetaAction* MetaPointAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPointAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaPointAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaPointAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaPointAction::Compare( const MetaAction& rMetaAction ) const
{
    return maPt == ((MetaPointAction&)rMetaAction).maPt;
}

// ------------------------------------------------------------------------

void MetaPointAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maPt;
}

// ------------------------------------------------------------------------

void MetaPointAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maPt;
}

// ========================================================================

IMPL_META_ACTION( Line, META_LINE_ACTION )

// ------------------------------------------------------------------------

MetaLineAction::MetaLineAction( const Point& rStart, const Point& rEnd ) :
    MetaAction  ( META_LINE_ACTION ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{
}

// ------------------------------------------------------------------------

MetaLineAction::MetaLineAction( const Point& rStart, const Point& rEnd,
                                const LineInfo& rLineInfo ) :
    MetaAction  ( META_LINE_ACTION ),
    maLineInfo  ( rLineInfo ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{
}

// ------------------------------------------------------------------------

void MetaLineAction::Execute( OutputDevice* pOut )
{
    if( maLineInfo.IsDefault() )
        pOut->DrawLine( maStartPt, maEndPt );
    else
        pOut->DrawLine( maStartPt, maEndPt, maLineInfo );
}

// ------------------------------------------------------------------------

MetaAction* MetaLineAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaLineAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaLineAction::Move( long nHorzMove, long nVertMove )
{
    maStartPt.Move( nHorzMove, nVertMove );
    maEndPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaLineAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maStartPt, fScaleX, fScaleY );
    ImplScalePoint( maEndPt, fScaleX, fScaleY );
    ImplScaleLineInfo( maLineInfo, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaLineAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maLineInfo == ((MetaLineAction&)rMetaAction).maLineInfo ) &&
           ( maStartPt == ((MetaLineAction&)rMetaAction).maStartPt ) &&
           ( maEndPt == ((MetaLineAction&)rMetaAction).maEndPt );
}

// ------------------------------------------------------------------------

void MetaLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 2, pData );

    rOStm << maStartPt << maEndPt;  // Version 1
    rOStm << maLineInfo;            // Version 2
}

// ------------------------------------------------------------------------

void MetaLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );

    // Version 1
    rIStm >> maStartPt >> maEndPt;

    // Version 2
    if( aCompat.GetVersion() >= 2 )
    {
        rIStm >> maLineInfo;
    }
}

// ========================================================================

IMPL_META_ACTION( Rect, META_RECT_ACTION )

// ------------------------------------------------------------------------

MetaRectAction::MetaRectAction( const Rectangle& rRect ) :
    MetaAction  ( META_RECT_ACTION ),
    maRect      ( rRect )
{
}

// ------------------------------------------------------------------------

void MetaRectAction::Execute( OutputDevice* pOut )
{
    pOut->DrawRect( maRect );
}

// ------------------------------------------------------------------------

MetaAction* MetaRectAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaRectAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaRectAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaRectAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaRectAction::Compare( const MetaAction& rMetaAction ) const
{
    return maRect == ((MetaRectAction&)rMetaAction).maRect;
}

// ------------------------------------------------------------------------

void MetaRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRect;
}

// ------------------------------------------------------------------------

void MetaRectAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRect;
}

// ========================================================================

IMPL_META_ACTION( RoundRect, META_ROUNDRECT_ACTION )

// ------------------------------------------------------------------------

MetaRoundRectAction::MetaRoundRectAction( const Rectangle& rRect,
                                          sal_uInt32 nHorzRound, sal_uInt32 nVertRound ) :
    MetaAction  ( META_ROUNDRECT_ACTION ),
    maRect      ( rRect ),
    mnHorzRound ( nHorzRound ),
    mnVertRound ( nVertRound )
{
}

// ------------------------------------------------------------------------

void MetaRoundRectAction::Execute( OutputDevice* pOut )
{
    pOut->DrawRect( maRect, mnHorzRound, mnVertRound );
}

// ------------------------------------------------------------------------

MetaAction* MetaRoundRectAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaRoundRectAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaRoundRectAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaRoundRectAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
    mnHorzRound = FRound( mnHorzRound * fabs(fScaleX) );
    mnVertRound = FRound( mnVertRound * fabs(fScaleY) );
}

// ------------------------------------------------------------------------

sal_Bool MetaRoundRectAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaRoundRectAction&)rMetaAction).maRect ) &&
           ( mnHorzRound == ((MetaRoundRectAction&)rMetaAction).mnHorzRound ) &&
           ( mnVertRound == ((MetaRoundRectAction&)rMetaAction).mnVertRound );
}

// ------------------------------------------------------------------------

void MetaRoundRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRect << mnHorzRound << mnVertRound;
}

// ------------------------------------------------------------------------

void MetaRoundRectAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRect >> mnHorzRound >> mnVertRound;
}

// ========================================================================

IMPL_META_ACTION( Ellipse, META_ELLIPSE_ACTION )

// ------------------------------------------------------------------------

MetaEllipseAction::MetaEllipseAction( const Rectangle& rRect ) :
    MetaAction  ( META_ELLIPSE_ACTION ),
    maRect      ( rRect )
{
}

// ------------------------------------------------------------------------

void MetaEllipseAction::Execute( OutputDevice* pOut )
{
    pOut->DrawEllipse( maRect );
}

// ------------------------------------------------------------------------

MetaAction* MetaEllipseAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaEllipseAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaEllipseAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaEllipseAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaEllipseAction::Compare( const MetaAction& rMetaAction ) const
{
    return maRect == ((MetaEllipseAction&)rMetaAction).maRect;
}

// ------------------------------------------------------------------------

void MetaEllipseAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRect;
}

// ------------------------------------------------------------------------

void MetaEllipseAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRect;
}

// ========================================================================

IMPL_META_ACTION( Arc, META_ARC_ACTION )

// ------------------------------------------------------------------------

MetaArcAction::MetaArcAction( const Rectangle& rRect,
                              const Point& rStart, const Point& rEnd ) :
    MetaAction  ( META_ARC_ACTION ),
    maRect      ( rRect ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{
}

// ------------------------------------------------------------------------

void MetaArcAction::Execute( OutputDevice* pOut )
{
    pOut->DrawArc( maRect, maStartPt, maEndPt );
}

// ------------------------------------------------------------------------

MetaAction* MetaArcAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaArcAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaArcAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move(  nHorzMove, nVertMove );
    maStartPt.Move(  nHorzMove, nVertMove );
    maEndPt.Move(  nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaArcAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
    ImplScalePoint( maStartPt, fScaleX, fScaleY );
    ImplScalePoint( maEndPt, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaArcAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaArcAction&)rMetaAction).maRect ) &&
           ( maStartPt == ((MetaArcAction&)rMetaAction).maStartPt ) &&
           ( maEndPt == ((MetaArcAction&)rMetaAction).maEndPt );
}

// ------------------------------------------------------------------------

void MetaArcAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRect << maStartPt << maEndPt;
}

// ------------------------------------------------------------------------

void MetaArcAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRect >> maStartPt >> maEndPt;
}

// ========================================================================

IMPL_META_ACTION( Pie, META_PIE_ACTION )

// ------------------------------------------------------------------------

MetaPieAction::MetaPieAction( const Rectangle& rRect,
                              const Point& rStart, const Point& rEnd ) :
    MetaAction  ( META_PIE_ACTION ),
    maRect      ( rRect ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{
}

// ------------------------------------------------------------------------

void MetaPieAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPie( maRect, maStartPt, maEndPt );
}

// ------------------------------------------------------------------------

MetaAction* MetaPieAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPieAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaPieAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move(  nHorzMove, nVertMove );
    maStartPt.Move(  nHorzMove, nVertMove );
    maEndPt.Move(  nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaPieAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
    ImplScalePoint( maStartPt, fScaleX, fScaleY );
    ImplScalePoint( maEndPt, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaPieAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaPieAction&)rMetaAction).maRect ) &&
           ( maStartPt == ((MetaPieAction&)rMetaAction).maStartPt ) &&
           ( maEndPt == ((MetaPieAction&)rMetaAction).maEndPt );
}

// ------------------------------------------------------------------------

void MetaPieAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRect << maStartPt << maEndPt;
}

// ------------------------------------------------------------------------

void MetaPieAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRect >> maStartPt >> maEndPt;
}

// ========================================================================

IMPL_META_ACTION( Chord, META_CHORD_ACTION )

// ------------------------------------------------------------------------

MetaChordAction::MetaChordAction( const Rectangle& rRect,
                                  const Point& rStart, const Point& rEnd ) :
    MetaAction  ( META_CHORD_ACTION ),
    maRect      ( rRect ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{
}

// ------------------------------------------------------------------------

void MetaChordAction::Execute( OutputDevice* pOut )
{
    pOut->DrawChord( maRect, maStartPt, maEndPt );
}

// ------------------------------------------------------------------------

MetaAction* MetaChordAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaChordAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaChordAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move(  nHorzMove, nVertMove );
    maStartPt.Move(  nHorzMove, nVertMove );
    maEndPt.Move(  nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaChordAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
    ImplScalePoint( maStartPt, fScaleX, fScaleY );
    ImplScalePoint( maEndPt, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaChordAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaChordAction&)rMetaAction).maRect ) &&
           ( maStartPt == ((MetaChordAction&)rMetaAction).maStartPt ) &&
           ( maEndPt == ((MetaChordAction&)rMetaAction).maEndPt );
}

// ------------------------------------------------------------------------

void MetaChordAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRect << maStartPt << maEndPt;
}

// ------------------------------------------------------------------------

void MetaChordAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRect >> maStartPt >> maEndPt;
}

// ========================================================================

IMPL_META_ACTION( PolyLine, META_POLYLINE_ACTION )

// ------------------------------------------------------------------------

MetaPolyLineAction::MetaPolyLineAction( const Polygon& rPoly ) :
    MetaAction  ( META_POLYLINE_ACTION ),
    maPoly      ( rPoly )
{
}

// ------------------------------------------------------------------------

MetaPolyLineAction::MetaPolyLineAction( const Polygon& rPoly, const LineInfo& rLineInfo ) :
    MetaAction  ( META_POLYLINE_ACTION ),
    maLineInfo  ( rLineInfo ),
    maPoly      ( rPoly )
{
}

// ------------------------------------------------------------------------

void MetaPolyLineAction::Execute( OutputDevice* pOut )
{
    if( maLineInfo.IsDefault() )
        pOut->DrawPolyLine( maPoly );
    else
        pOut->DrawPolyLine( maPoly, maLineInfo );
}

// ------------------------------------------------------------------------

MetaAction* MetaPolyLineAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPolyLineAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaPolyLineAction::Move( long nHorzMove, long nVertMove )
{
    maPoly.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaPolyLineAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoly( maPoly, fScaleX, fScaleY );
    ImplScaleLineInfo( maLineInfo, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaPolyLineAction::Compare( const MetaAction& rMetaAction ) const
{
    sal_Bool bIsEqual = sal_True;
    if ( maLineInfo != ((MetaPolyLineAction&)rMetaAction).maLineInfo )
        bIsEqual = sal_False;
    else
        bIsEqual = maPoly.IsEqual(((MetaPolyLineAction&)rMetaAction).maPoly );
    return bIsEqual;

}

// ------------------------------------------------------------------------

void MetaPolyLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 3, pData );

    Polygon aSimplePoly;
    maPoly.AdaptiveSubdivide( aSimplePoly );

    rOStm << aSimplePoly;                               // Version 1
    rOStm << maLineInfo;                                // Version 2

    sal_uInt8 bHasPolyFlags = maPoly.HasFlags();        // Version 3
    rOStm << bHasPolyFlags;
    if ( bHasPolyFlags )
        maPoly.Write( rOStm );
}

// ------------------------------------------------------------------------

void MetaPolyLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );

    // Version 1
    rIStm >> maPoly;

    // Version 2
    if( aCompat.GetVersion() >= 2 )
        rIStm >> maLineInfo;
    if ( aCompat.GetVersion() >= 3 )
    {
        sal_uInt8 bHasPolyFlags;
        rIStm >> bHasPolyFlags;
        if ( bHasPolyFlags )
            maPoly.Read( rIStm );
    }
}

// ========================================================================

IMPL_META_ACTION( Polygon, META_POLYGON_ACTION )

// ------------------------------------------------------------------------

MetaPolygonAction::MetaPolygonAction( const Polygon& rPoly ) :
    MetaAction  ( META_POLYGON_ACTION ),
    maPoly      ( rPoly )
{
}

// ------------------------------------------------------------------------

void MetaPolygonAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPolygon( maPoly );
}

// ------------------------------------------------------------------------

MetaAction* MetaPolygonAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPolygonAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaPolygonAction::Move( long nHorzMove, long nVertMove )
{
    maPoly.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaPolygonAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoly( maPoly, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaPolygonAction::Compare( const MetaAction& rMetaAction ) const
{
    return maPoly.IsEqual(((MetaPolygonAction&)rMetaAction).maPoly );
}

// ------------------------------------------------------------------------

void MetaPolygonAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 2, pData );

    Polygon aSimplePoly;                            // Version 1
    maPoly.AdaptiveSubdivide( aSimplePoly );
    rOStm << aSimplePoly;

    sal_uInt8 bHasPolyFlags = maPoly.HasFlags();    // Version 2
    rOStm << bHasPolyFlags;
    if ( bHasPolyFlags )
        maPoly.Write( rOStm );
}

// ------------------------------------------------------------------------

void MetaPolygonAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );

    rIStm >> maPoly;                    // Version 1

    if( aCompat.GetVersion() >= 2 )     // Version 2
    {
        sal_uInt8 bHasPolyFlags;
        rIStm >> bHasPolyFlags;
        if ( bHasPolyFlags )
            maPoly.Read( rIStm );
    }
}

// ========================================================================

IMPL_META_ACTION( PolyPolygon, META_POLYPOLYGON_ACTION )

// ------------------------------------------------------------------------

MetaPolyPolygonAction::MetaPolyPolygonAction( const PolyPolygon& rPolyPoly ) :
    MetaAction  ( META_POLYPOLYGON_ACTION ),
    maPolyPoly  ( rPolyPoly )
{
}

// ------------------------------------------------------------------------

void MetaPolyPolygonAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPolyPolygon( maPolyPoly );
}

// ------------------------------------------------------------------------

MetaAction* MetaPolyPolygonAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPolyPolygonAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaPolyPolygonAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaPolyPolygonAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaPolyPolygonAction::Compare( const MetaAction& rMetaAction ) const
{
    return maPolyPoly.IsEqual(((MetaPolyPolygonAction&)rMetaAction).maPolyPoly );
}

// ------------------------------------------------------------------------

void MetaPolyPolygonAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 2, pData );

    sal_uInt16 nNumberOfComplexPolygons = 0;
    sal_uInt16 i, nPolyCount = maPolyPoly.Count();

    Polygon aSimplePoly;                                // Version 1
    rOStm << nPolyCount;
    for ( i = 0; i < nPolyCount; i++ )
    {
        const Polygon& rPoly = maPolyPoly.GetObject( i );
        if ( rPoly.HasFlags() )
            nNumberOfComplexPolygons++;
        rPoly.AdaptiveSubdivide( aSimplePoly );
        rOStm << aSimplePoly;
    }

    rOStm << nNumberOfComplexPolygons;                  // Version 2
    for ( i = 0; nNumberOfComplexPolygons && ( i < nPolyCount ); i++ )
    {
        const Polygon& rPoly = maPolyPoly.GetObject( i );
        if ( rPoly.HasFlags() )
        {
            rOStm << i;
            rPoly.Write( rOStm );

            nNumberOfComplexPolygons--;
        }
    }
}

// ------------------------------------------------------------------------

void MetaPolyPolygonAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maPolyPoly;                // Version 1

    if ( aCompat.GetVersion() >= 2 )    // Version 2
    {
        sal_uInt16 i, nIndex, nNumberOfComplexPolygons;
        rIStm >> nNumberOfComplexPolygons;
        for ( i = 0; i < nNumberOfComplexPolygons; i++ )
        {
            rIStm >> nIndex;
            Polygon aPoly;
            aPoly.Read( rIStm );
            maPolyPoly.Replace( aPoly, nIndex );
        }
    }
}

// ========================================================================

IMPL_META_ACTION( Text, META_TEXT_ACTION )

// ------------------------------------------------------------------------

MetaTextAction::MetaTextAction( const Point& rPt, const rtl::OUString& rStr,
                                sal_uInt16 nIndex, sal_uInt16 nLen ) :
    MetaAction  ( META_TEXT_ACTION ),
    maPt        ( rPt ),
    maStr       ( rStr ),
    mnIndex     ( nIndex ),
    mnLen       ( nLen )
{
}

// ------------------------------------------------------------------------

void MetaTextAction::Execute( OutputDevice* pOut )
{
    pOut->DrawText( maPt, maStr, mnIndex, mnLen );
}

// ------------------------------------------------------------------------

MetaAction* MetaTextAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaTextAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaTextAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaTextAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPt == ((MetaTextAction&)rMetaAction).maPt ) &&
           ( maStr == ((MetaTextAction&)rMetaAction).maStr ) &&
           ( mnIndex == ((MetaTextAction&)rMetaAction).mnIndex ) &&
           ( mnLen == ((MetaTextAction&)rMetaAction).mnLen );
}

// ------------------------------------------------------------------------

void MetaTextAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 2, pData );
    rOStm   << maPt;
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm   << mnIndex;
    rOStm   << mnLen;

    write_lenPrefixed_uInt16s_FromOUString<sal_uInt16>(rOStm, maStr); // version 2
}

// ------------------------------------------------------------------------

void MetaTextAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    COMPAT( rIStm );
    rIStm   >> maPt;
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    rIStm   >> mnIndex;
    rIStm   >> mnLen;

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
        maStr = read_lenPrefixed_uInt16s_ToOUString<sal_uInt16>(rIStm);
}

// ========================================================================

MetaTextArrayAction::MetaTextArrayAction() :
    MetaAction  ( META_TEXTARRAY_ACTION ),
    mpDXAry     ( NULL ),
    mnIndex     ( 0 ),
    mnLen       ( 0 )
{
}

// ------------------------------------------------------------------------

MetaTextArrayAction::MetaTextArrayAction( const MetaTextArrayAction& rAction ) :
    MetaAction  ( META_TEXTARRAY_ACTION ),
    maStartPt   ( rAction.maStartPt ),
    maStr       ( rAction.maStr ),
    mnIndex     ( rAction.mnIndex ),
    mnLen       ( rAction.mnLen )
{
    if( rAction.mpDXAry )
    {
        const sal_uLong nAryLen = mnLen;

        mpDXAry = new sal_Int32[ nAryLen ];
        memcpy( mpDXAry, rAction.mpDXAry, nAryLen * sizeof( sal_Int32 ) );
    }
    else
        mpDXAry = NULL;
}

// ------------------------------------------------------------------------

MetaTextArrayAction::MetaTextArrayAction( const Point& rStartPt,
                                          const rtl::OUString& rStr,
                                          const sal_Int32* pDXAry,
                                          sal_uInt16 nIndex,
                                          sal_uInt16 nLen ) :
    MetaAction  ( META_TEXTARRAY_ACTION ),
    maStartPt   ( rStartPt ),
    maStr       ( rStr ),
    mnIndex     ( nIndex ),
    mnLen       ( ( nLen == STRING_LEN ) ? rStr.getLength() : nLen )
{
    const sal_uLong nAryLen = pDXAry ? mnLen : 0;

    if( nAryLen )
    {
        mpDXAry = new sal_Int32[ nAryLen ];
        memcpy( mpDXAry, pDXAry, nAryLen * sizeof( sal_Int32 ) );
    }
    else
        mpDXAry = NULL;
}

// ------------------------------------------------------------------------

MetaTextArrayAction::~MetaTextArrayAction()
{
    delete[] mpDXAry;
}

// ------------------------------------------------------------------------

void MetaTextArrayAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTextArray( maStartPt, maStr, mpDXAry, mnIndex, mnLen );
}

// ------------------------------------------------------------------------

MetaAction* MetaTextArrayAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextArrayAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaTextArrayAction::Move( long nHorzMove, long nVertMove )
{
    maStartPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaTextArrayAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maStartPt, fScaleX, fScaleY );

    if ( mpDXAry && mnLen )
    {
        for ( sal_uInt16 i = 0, nCount = mnLen; i < nCount; i++ )
            mpDXAry[ i ] = FRound( mpDXAry[ i ] * fabs(fScaleX) );
    }
}

// ------------------------------------------------------------------------

sal_Bool MetaTextArrayAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maStartPt == ((MetaTextArrayAction&)rMetaAction).maStartPt ) &&
           ( maStr == ((MetaTextArrayAction&)rMetaAction).maStr ) &&
           ( mnIndex == ((MetaTextArrayAction&)rMetaAction).mnIndex ) &&
           ( mnLen == ((MetaTextArrayAction&)rMetaAction).mnLen ) &&
           ( memcmp( mpDXAry, ((MetaTextArrayAction&)rMetaAction).mpDXAry, mnLen ) == 0 );
}

// ------------------------------------------------------------------------

void MetaTextArrayAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    const sal_uInt32 nAryLen = mpDXAry ? mnLen : 0;

    WRITE_BASE_COMPAT( rOStm, 2, pData );
    rOStm   << maStartPt;
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm   << mnIndex;
    rOStm   << mnLen;
    rOStm   << nAryLen;

    for( sal_uLong i = 0UL; i < nAryLen; i++ )
        rOStm << mpDXAry[ i ];

    write_lenPrefixed_uInt16s_FromOUString<sal_uInt16>(rOStm, maStr); // version 2
}

// ------------------------------------------------------------------------

void MetaTextArrayAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    sal_uInt32 nAryLen;

    delete[] mpDXAry;

    COMPAT( rIStm );
    rIStm   >> maStartPt;
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    rIStm   >> mnIndex;
    rIStm   >> mnLen;
    rIStm   >> nAryLen;

    if ( mnIndex + mnLen > maStr.getLength() )
    {
        mnIndex = 0;
        mpDXAry = 0;
        return;
    }

    if( nAryLen )
    {
        // #i9762#, #106172# Ensure that DX array is at least mnLen entries long
        if ( mnLen >= nAryLen )
        {
            mpDXAry = new (std::nothrow)sal_Int32[ mnLen ];
            if ( mpDXAry )
            {
                   sal_uLong i;
                for( i = 0UL; i < nAryLen; i++ )
                    rIStm >> mpDXAry[ i ];

                // #106172# setup remainder
                for( ; i < mnLen; i++ )
                    mpDXAry[ i ] = 0;
            }
        }
        else
        {
            mpDXAry = NULL;
            return;
        }
    }
    else
        mpDXAry = NULL;

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
    {
        maStr = read_lenPrefixed_uInt16s_ToOUString<sal_uInt16>(rIStm);

        if ( mnIndex + mnLen > maStr.getLength() )
        {
            mnIndex = 0;
            delete[] mpDXAry, mpDXAry = NULL;
        }
    }
}

// ========================================================================

IMPL_META_ACTION( StretchText, META_STRETCHTEXT_ACTION )

// ------------------------------------------------------------------------

MetaStretchTextAction::MetaStretchTextAction( const Point& rPt, sal_uInt32 nWidth,
                                              const rtl::OUString& rStr,
                                              sal_uInt16 nIndex, sal_uInt16 nLen ) :
    MetaAction  ( META_STRETCHTEXT_ACTION ),
    maPt        ( rPt ),
    maStr       ( rStr ),
    mnWidth     ( nWidth ),
    mnIndex     ( nIndex ),
    mnLen       ( nLen )
{
}

// ------------------------------------------------------------------------

void MetaStretchTextAction::Execute( OutputDevice* pOut )
{
    pOut->DrawStretchText( maPt, mnWidth, maStr, mnIndex, mnLen );
}

// ------------------------------------------------------------------------

MetaAction* MetaStretchTextAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaStretchTextAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaStretchTextAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaStretchTextAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
    mnWidth = (sal_uLong)FRound( mnWidth * fabs(fScaleX) );
}

// ------------------------------------------------------------------------

sal_Bool MetaStretchTextAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPt == ((MetaStretchTextAction&)rMetaAction).maPt ) &&
           ( maStr == ((MetaStretchTextAction&)rMetaAction).maStr ) &&
           ( mnWidth == ((MetaStretchTextAction&)rMetaAction).mnWidth ) &&
           ( mnIndex == ((MetaStretchTextAction&)rMetaAction).mnIndex ) &&
           ( mnLen == ((MetaStretchTextAction&)rMetaAction).mnLen );
}

// ------------------------------------------------------------------------

void MetaStretchTextAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 2, pData );
    rOStm   << maPt;
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm   << mnWidth;
    rOStm   << mnIndex;
    rOStm   << mnLen;

    write_lenPrefixed_uInt16s_FromOUString<sal_uInt16>(rOStm, maStr); // version 2
}

// ------------------------------------------------------------------------

void MetaStretchTextAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    COMPAT( rIStm );
    rIStm   >> maPt;
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    rIStm   >> mnWidth;
    rIStm   >> mnIndex;
    rIStm   >> mnLen;

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
        maStr = read_lenPrefixed_uInt16s_ToOUString<sal_uInt16>(rIStm);
}

// ========================================================================

IMPL_META_ACTION( TextRect, META_TEXTRECT_ACTION )

// ------------------------------------------------------------------------

MetaTextRectAction::MetaTextRectAction( const Rectangle& rRect,
                                        const rtl::OUString& rStr, sal_uInt16 nStyle ) :
    MetaAction  ( META_TEXTRECT_ACTION ),
    maRect      ( rRect ),
    maStr       ( rStr ),
    mnStyle     ( nStyle )
{
}

// ------------------------------------------------------------------------

void MetaTextRectAction::Execute( OutputDevice* pOut )
{
    pOut->DrawText( maRect, maStr, mnStyle );
}

// ------------------------------------------------------------------------

MetaAction* MetaTextRectAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextRectAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaTextRectAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaTextRectAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaTextRectAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaTextRectAction&)rMetaAction).maRect ) &&
           ( maStr == ((MetaTextRectAction&)rMetaAction).maStr ) &&
           ( mnStyle == ((MetaTextRectAction&)rMetaAction).mnStyle );
}

// ------------------------------------------------------------------------

void MetaTextRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 2, pData );
    rOStm   << maRect;
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm   << mnStyle;

    write_lenPrefixed_uInt16s_FromOUString<sal_uInt16>(rOStm, maStr); // version 2
}

// ------------------------------------------------------------------------

void MetaTextRectAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    COMPAT( rIStm );
    rIStm   >> maRect;
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    rIStm   >> mnStyle;

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
        maStr = read_lenPrefixed_uInt16s_ToOUString<sal_uInt16>(rIStm);
}

// ========================================================================

IMPL_META_ACTION( TextLine, META_TEXTLINE_ACTION )

// ------------------------------------------------------------------------

MetaTextLineAction::MetaTextLineAction( const Point& rPos, long nWidth,
                                        FontStrikeout eStrikeout,
                                        FontUnderline eUnderline,
                                        FontUnderline eOverline ) :
    MetaAction  ( META_TEXTLINE_ACTION ),
    maPos       ( rPos ),
    mnWidth     ( nWidth ),
    meStrikeout ( eStrikeout ),
    meUnderline ( eUnderline ),
    meOverline  ( eOverline )
{
}

// ------------------------------------------------------------------------

void MetaTextLineAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTextLine( maPos, mnWidth, meStrikeout, meUnderline, meOverline );
}

// ------------------------------------------------------------------------

MetaAction* MetaTextLineAction::Clone()
{
    MetaAction* pClone = (MetaAction*)new MetaTextLineAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaTextLineAction::Move( long nHorzMove, long nVertMove )
{
    maPos.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaTextLineAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPos, fScaleX, fScaleY );
    mnWidth = FRound( mnWidth * fabs(fScaleX) );
}

// ------------------------------------------------------------------------

sal_Bool MetaTextLineAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPos == ((MetaTextLineAction&)rMetaAction).maPos ) &&
           ( mnWidth == ((MetaTextLineAction&)rMetaAction).mnWidth ) &&
           ( meStrikeout == ((MetaTextLineAction&)rMetaAction).meStrikeout ) &&
           ( meUnderline == ((MetaTextLineAction&)rMetaAction).meUnderline ) &&
           ( meOverline  == ((MetaTextLineAction&)rMetaAction).meOverline );
}

// ------------------------------------------------------------------------

void MetaTextLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 2, pData );

    //#fdo39428 SvStream no longer supports operator<<(long)
    rOStm << maPos;
    rOStm << sal::static_int_cast<sal_Int32>(mnWidth);
    rOStm << static_cast<sal_uInt32>(meStrikeout);
    rOStm << static_cast<sal_uInt32>(meUnderline);
    // new in version 2
    rOStm << static_cast<sal_uInt32>(meOverline);
}

// ------------------------------------------------------------------------

void MetaTextLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );

    //#fdo39428 SvStream no longer supports operator>>(long&)
    sal_uInt32 nTemp;
    sal_Int32 nTemp2;
    rIStm >> maPos;
    rIStm >> nTemp2;
    mnWidth = nTemp2;
    rIStm >> nTemp;
    meStrikeout = (FontStrikeout)nTemp;
    rIStm >> nTemp;
    meUnderline = (FontUnderline)nTemp;
    if ( aCompat.GetVersion() >= 2 ) {
        rIStm >> nTemp;
        meUnderline = (FontUnderline)nTemp;
    }
}

// ========================================================================

IMPL_META_ACTION( Bmp, META_BMP_ACTION )

// ------------------------------------------------------------------------

MetaBmpAction::MetaBmpAction( const Point& rPt, const Bitmap& rBmp ) :
    MetaAction  ( META_BMP_ACTION ),
    maBmp       ( rBmp ),
    maPt        ( rPt )
{
}

// ------------------------------------------------------------------------

void MetaBmpAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmap( maPt, maBmp );
}

// ------------------------------------------------------------------------

MetaAction* MetaBmpAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaBmpAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaBmpAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaBmpAction::Compare( const MetaAction& rMetaAction ) const
{
    return maBmp.IsEqual(((MetaBmpAction&)rMetaAction).maBmp ) &&
           ( maPt == ((MetaBmpAction&)rMetaAction).maPt );
}

// ------------------------------------------------------------------------

void MetaBmpAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmp << maPt;
    }
}

// ------------------------------------------------------------------------

void MetaBmpAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmp >> maPt;
}

// ========================================================================

IMPL_META_ACTION( BmpScale, META_BMPSCALE_ACTION )

// ------------------------------------------------------------------------

MetaBmpScaleAction::MetaBmpScaleAction( const Point& rPt, const Size& rSz,
                                        const Bitmap& rBmp ) :
    MetaAction  ( META_BMPSCALE_ACTION ),
    maBmp       ( rBmp ),
    maPt        ( rPt ),
    maSz        ( rSz )
{
}

// ------------------------------------------------------------------------

void MetaBmpScaleAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmap( maPt, maSz, maBmp );
}

// ------------------------------------------------------------------------

MetaAction* MetaBmpScaleAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpScaleAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaBmpScaleAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaBmpScaleAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPt, maSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPt = aRectangle.TopLeft();
    maSz = aRectangle.GetSize();
}

// ------------------------------------------------------------------------

sal_Bool MetaBmpScaleAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaBmpScaleAction&)rMetaAction).maBmp )) &&
           ( maPt == ((MetaBmpScaleAction&)rMetaAction).maPt ) &&
           ( maSz == ((MetaBmpScaleAction&)rMetaAction).maSz );
}

// ------------------------------------------------------------------------

void MetaBmpScaleAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmp << maPt << maSz;
    }
}

// ------------------------------------------------------------------------

void MetaBmpScaleAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmp >> maPt >> maSz;
}

// ========================================================================

IMPL_META_ACTION( BmpScalePart, META_BMPSCALEPART_ACTION )

// ------------------------------------------------------------------------

MetaBmpScalePartAction::MetaBmpScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                const Point& rSrcPt, const Size& rSrcSz,
                                                const Bitmap& rBmp ) :
    MetaAction  ( META_BMPSCALEPART_ACTION ),
    maBmp       ( rBmp ),
    maDstPt     ( rDstPt ),
    maDstSz     ( rDstSz ),
    maSrcPt     ( rSrcPt ),
    maSrcSz     ( rSrcSz )
{
}

// ------------------------------------------------------------------------

void MetaBmpScalePartAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmap( maDstPt, maDstSz, maSrcPt, maSrcSz, maBmp );
}

// ------------------------------------------------------------------------

MetaAction* MetaBmpScalePartAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpScalePartAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaBmpScalePartAction::Move( long nHorzMove, long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaBmpScalePartAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maDstPt, maDstSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
}

// ------------------------------------------------------------------------

sal_Bool MetaBmpScalePartAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaBmpScalePartAction&)rMetaAction).maBmp )) &&
           ( maDstPt == ((MetaBmpScalePartAction&)rMetaAction).maDstPt ) &&
           ( maDstSz == ((MetaBmpScalePartAction&)rMetaAction).maDstSz ) &&
           ( maSrcPt == ((MetaBmpScalePartAction&)rMetaAction).maSrcPt ) &&
           ( maSrcSz == ((MetaBmpScalePartAction&)rMetaAction).maSrcSz );
}

// ------------------------------------------------------------------------

void MetaBmpScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmp << maDstPt << maDstSz << maSrcPt << maSrcSz;
    }
}

// ------------------------------------------------------------------------

void MetaBmpScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmp >> maDstPt >> maDstSz >> maSrcPt >> maSrcSz;
}

// ========================================================================

IMPL_META_ACTION( BmpEx, META_BMPEX_ACTION )

// ------------------------------------------------------------------------

MetaBmpExAction::MetaBmpExAction( const Point& rPt, const BitmapEx& rBmpEx ) :
    MetaAction  ( META_BMPEX_ACTION ),
    maBmpEx     ( rBmpEx ),
    maPt        ( rPt )
{
}

// ------------------------------------------------------------------------

void MetaBmpExAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmapEx( maPt, maBmpEx );
}

// ------------------------------------------------------------------------

MetaAction* MetaBmpExAction::Clone()
{
    MetaBmpExAction* pClone = new MetaBmpExAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaBmpExAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaBmpExAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaBmpExAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmpEx.IsEqual(((MetaBmpExAction&)rMetaAction).maBmpEx )) &&
           ( maPt == ((MetaBmpExAction&)rMetaAction).maPt );
}

// ------------------------------------------------------------------------

void MetaBmpExAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmpEx << maPt;
    }
}

// ------------------------------------------------------------------------

void MetaBmpExAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmpEx >> maPt;
}

// ========================================================================

IMPL_META_ACTION( BmpExScale, META_BMPEXSCALE_ACTION )

// ------------------------------------------------------------------------

MetaBmpExScaleAction::MetaBmpExScaleAction( const Point& rPt, const Size& rSz,
                                            const BitmapEx& rBmpEx ) :
    MetaAction  ( META_BMPEXSCALE_ACTION ),
    maBmpEx     ( rBmpEx ),
    maPt        ( rPt ),
    maSz        ( rSz )
{
}

// ------------------------------------------------------------------------

void MetaBmpExScaleAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmapEx( maPt, maSz, maBmpEx );
}

// ------------------------------------------------------------------------

MetaAction* MetaBmpExScaleAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpExScaleAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaBmpExScaleAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaBmpExScaleAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPt, maSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPt = aRectangle.TopLeft();
    maSz = aRectangle.GetSize();
}

// ------------------------------------------------------------------------

sal_Bool MetaBmpExScaleAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmpEx.IsEqual(((MetaBmpExScaleAction&)rMetaAction).maBmpEx )) &&
           ( maPt == ((MetaBmpExScaleAction&)rMetaAction).maPt ) &&
           ( maSz == ((MetaBmpExScaleAction&)rMetaAction).maSz );
}

// ------------------------------------------------------------------------

void MetaBmpExScaleAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmpEx << maPt << maSz;
    }
}

// ------------------------------------------------------------------------

void MetaBmpExScaleAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmpEx >> maPt >> maSz;
}

// ========================================================================

IMPL_META_ACTION( BmpExScalePart, META_BMPEXSCALEPART_ACTION )

// ------------------------------------------------------------------------

MetaBmpExScalePartAction::MetaBmpExScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                    const Point& rSrcPt, const Size& rSrcSz,
                                                    const BitmapEx& rBmpEx ) :
    MetaAction  ( META_BMPEXSCALEPART_ACTION ),
    maBmpEx     ( rBmpEx ),
    maDstPt     ( rDstPt ),
    maDstSz     ( rDstSz ),
    maSrcPt     ( rSrcPt ),
    maSrcSz     ( rSrcSz )
{
}

// ------------------------------------------------------------------------

void MetaBmpExScalePartAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmapEx( maDstPt, maDstSz, maSrcPt, maSrcSz, maBmpEx );
}

// ------------------------------------------------------------------------

MetaAction* MetaBmpExScalePartAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpExScalePartAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaBmpExScalePartAction::Move( long nHorzMove, long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaBmpExScalePartAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maDstPt, maDstSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
}

// ------------------------------------------------------------------------

sal_Bool MetaBmpExScalePartAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmpEx.IsEqual(((MetaBmpExScalePartAction&)rMetaAction).maBmpEx )) &&
           ( maDstPt == ((MetaBmpExScalePartAction&)rMetaAction).maDstPt ) &&
           ( maDstSz == ((MetaBmpExScalePartAction&)rMetaAction).maDstSz ) &&
           ( maSrcPt == ((MetaBmpExScalePartAction&)rMetaAction).maSrcPt ) &&
           ( maSrcSz == ((MetaBmpExScalePartAction&)rMetaAction).maSrcSz );
}

// ------------------------------------------------------------------------

void MetaBmpExScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmpEx << maDstPt << maDstSz << maSrcPt << maSrcSz;
    }
}

// ------------------------------------------------------------------------

void MetaBmpExScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmpEx >> maDstPt >> maDstSz >> maSrcPt >> maSrcSz;
}

// ========================================================================

IMPL_META_ACTION( Mask, META_MASK_ACTION )

// ------------------------------------------------------------------------

MetaMaskAction::MetaMaskAction( const Point& rPt,
                                const Bitmap& rBmp,
                                const Color& rColor ) :
    MetaAction  ( META_MASK_ACTION ),
    maBmp       ( rBmp ),
    maColor     ( rColor ),
    maPt        ( rPt )
{
}

// ------------------------------------------------------------------------

void MetaMaskAction::Execute( OutputDevice* pOut )
{
    pOut->DrawMask( maPt, maBmp, maColor );
}

// ------------------------------------------------------------------------

MetaAction* MetaMaskAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMaskAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaMaskAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaMaskAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaMaskAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaMaskAction&)rMetaAction).maBmp )) &&
           ( maColor == ((MetaMaskAction&)rMetaAction).maColor ) &&
           ( maPt == ((MetaMaskAction&)rMetaAction).maPt );
}

// ------------------------------------------------------------------------

void MetaMaskAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmp << maPt;
    }
}

// ------------------------------------------------------------------------

void MetaMaskAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmp >> maPt;
}

// ========================================================================

IMPL_META_ACTION( MaskScale, META_MASKSCALE_ACTION )

// ------------------------------------------------------------------------

MetaMaskScaleAction::MetaMaskScaleAction( const Point& rPt, const Size& rSz,
                                          const Bitmap& rBmp,
                                          const Color& rColor ) :
    MetaAction  ( META_MASKSCALE_ACTION ),
    maBmp       ( rBmp ),
    maColor     ( rColor ),
    maPt        ( rPt ),
    maSz        ( rSz )
{
}

// ------------------------------------------------------------------------

void MetaMaskScaleAction::Execute( OutputDevice* pOut )
{
    pOut->DrawMask( maPt, maSz, maBmp, maColor );
}

// ------------------------------------------------------------------------

MetaAction* MetaMaskScaleAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMaskScaleAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaMaskScaleAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaMaskScaleAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPt, maSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPt = aRectangle.TopLeft();
    maSz = aRectangle.GetSize();
}

// ------------------------------------------------------------------------

sal_Bool MetaMaskScaleAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaMaskScaleAction&)rMetaAction).maBmp )) &&
           ( maColor == ((MetaMaskScaleAction&)rMetaAction).maColor ) &&
           ( maPt == ((MetaMaskScaleAction&)rMetaAction).maPt ) &&
           ( maSz == ((MetaMaskScaleAction&)rMetaAction).maSz );
}

// ------------------------------------------------------------------------

void MetaMaskScaleAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmp << maPt << maSz;
    }
}

// ------------------------------------------------------------------------

void MetaMaskScaleAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmp >> maPt >> maSz;
}

// ========================================================================

IMPL_META_ACTION( MaskScalePart, META_MASKSCALEPART_ACTION )

// ------------------------------------------------------------------------

MetaMaskScalePartAction::MetaMaskScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                  const Point& rSrcPt, const Size& rSrcSz,
                                                  const Bitmap& rBmp,
                                                  const Color& rColor ) :
    MetaAction  ( META_MASKSCALEPART_ACTION ),
    maBmp       ( rBmp ),
    maColor     ( rColor ),
    maDstPt     ( rDstPt ),
    maDstSz     ( rDstSz ),
    maSrcPt     ( rSrcPt ),
    maSrcSz     ( rSrcSz )
{
}

// ------------------------------------------------------------------------

void MetaMaskScalePartAction::Execute( OutputDevice* pOut )
{
    pOut->DrawMask( maDstPt, maDstSz, maSrcPt, maSrcSz, maBmp, maColor );
}

// ------------------------------------------------------------------------

MetaAction* MetaMaskScalePartAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMaskScalePartAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaMaskScalePartAction::Move( long nHorzMove, long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaMaskScalePartAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maDstPt, maDstSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
}

// ------------------------------------------------------------------------

sal_Bool MetaMaskScalePartAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaMaskScalePartAction&)rMetaAction).maBmp )) &&
           ( maColor == ((MetaMaskScalePartAction&)rMetaAction).maColor ) &&
           ( maDstPt == ((MetaMaskScalePartAction&)rMetaAction).maDstPt ) &&
           ( maDstSz == ((MetaMaskScalePartAction&)rMetaAction).maDstSz ) &&
           ( maSrcPt == ((MetaMaskScalePartAction&)rMetaAction).maSrcPt ) &&
           ( maSrcSz == ((MetaMaskScalePartAction&)rMetaAction).maSrcSz );
}

// ------------------------------------------------------------------------

void MetaMaskScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmp;
        maColor.Write( rOStm, sal_True );
        rOStm << maDstPt << maDstSz << maSrcPt << maSrcSz;
    }
}

// ------------------------------------------------------------------------

void MetaMaskScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmp;
    maColor.Read( rIStm, sal_True );
    rIStm >> maDstPt >> maDstSz >> maSrcPt >> maSrcSz;
}

// ========================================================================

IMPL_META_ACTION( Gradient, META_GRADIENT_ACTION )

// ------------------------------------------------------------------------

MetaGradientAction::MetaGradientAction( const Rectangle& rRect, const Gradient& rGradient ) :
    MetaAction  ( META_GRADIENT_ACTION ),
    maRect      ( rRect ),
    maGradient  ( rGradient )
{
}

// ------------------------------------------------------------------------

void MetaGradientAction::Execute( OutputDevice* pOut )
{
    pOut->DrawGradient( maRect, maGradient );
}

// ------------------------------------------------------------------------

MetaAction* MetaGradientAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaGradientAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaGradientAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaGradientAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaGradientAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaGradientAction&)rMetaAction).maRect ) &&
           ( maGradient == ((MetaGradientAction&)rMetaAction).maGradient );
}

// ------------------------------------------------------------------------

void MetaGradientAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRect << maGradient;
}

// ------------------------------------------------------------------------

void MetaGradientAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRect >> maGradient;
}

// ========================================================================

MetaGradientExAction::MetaGradientExAction() :
    MetaAction  ( META_GRADIENTEX_ACTION )
{
}

// ------------------------------------------------------------------------

MetaGradientExAction::MetaGradientExAction( const PolyPolygon& rPolyPoly, const Gradient& rGradient ) :
    MetaAction  ( META_GRADIENTEX_ACTION ),
    maPolyPoly  ( rPolyPoly ),
    maGradient  ( rGradient )
{
}

// ------------------------------------------------------------------------

MetaGradientExAction::~MetaGradientExAction()
{
}

// ------------------------------------------------------------------------

void MetaGradientExAction::Execute( OutputDevice* pOut )
{
    if( pOut->GetConnectMetaFile() )
    {
        Duplicate();
        pOut->GetConnectMetaFile()->AddAction( this );
    }
}

// ------------------------------------------------------------------------

MetaAction* MetaGradientExAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaGradientExAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaGradientExAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaGradientExAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaGradientExAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPolyPoly == ((MetaGradientExAction&)rMetaAction).maPolyPoly ) &&
           ( maGradient == ((MetaGradientExAction&)rMetaAction).maGradient );
}

// ------------------------------------------------------------------------

void MetaGradientExAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );

    // #i105373# see comment at MetaTransparentAction::Write
    PolyPolygon aNoCurvePolyPolygon;
    maPolyPoly.AdaptiveSubdivide(aNoCurvePolyPolygon);

    rOStm << aNoCurvePolyPolygon;
    rOStm << maGradient;
}

// ------------------------------------------------------------------------

void MetaGradientExAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maPolyPoly >> maGradient;
}

// ========================================================================

IMPL_META_ACTION( Hatch, META_HATCH_ACTION )

// ------------------------------------------------------------------------

MetaHatchAction::MetaHatchAction( const PolyPolygon& rPolyPoly, const Hatch& rHatch ) :
    MetaAction  ( META_HATCH_ACTION ),
    maPolyPoly  ( rPolyPoly ),
    maHatch     ( rHatch )
{
}

// ------------------------------------------------------------------------

void MetaHatchAction::Execute( OutputDevice* pOut )
{
    pOut->DrawHatch( maPolyPoly, maHatch );
}

// ------------------------------------------------------------------------

MetaAction* MetaHatchAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaHatchAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaHatchAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaHatchAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaHatchAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPolyPoly == ((MetaHatchAction&)rMetaAction).maPolyPoly ) &&
           ( maHatch == ((MetaHatchAction&)rMetaAction).maHatch );
}

// ------------------------------------------------------------------------

void MetaHatchAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );

    // #i105373# see comment at MetaTransparentAction::Write
    PolyPolygon aNoCurvePolyPolygon;
    maPolyPoly.AdaptiveSubdivide(aNoCurvePolyPolygon);

    rOStm << aNoCurvePolyPolygon;
    rOStm << maHatch;
}

// ------------------------------------------------------------------------

void MetaHatchAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maPolyPoly >> maHatch;
}

// ========================================================================

IMPL_META_ACTION( Wallpaper, META_WALLPAPER_ACTION )

// ------------------------------------------------------------------------

MetaWallpaperAction::MetaWallpaperAction( const Rectangle& rRect,
                                          const Wallpaper& rPaper ) :
    MetaAction  ( META_WALLPAPER_ACTION ),
    maRect      ( rRect ),
    maWallpaper ( rPaper )
{
}

// ------------------------------------------------------------------------

void MetaWallpaperAction::Execute( OutputDevice* pOut )
{
    pOut->DrawWallpaper( maRect, maWallpaper );
}

// ------------------------------------------------------------------------

MetaAction* MetaWallpaperAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaWallpaperAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaWallpaperAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaWallpaperAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaWallpaperAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaWallpaperAction&)rMetaAction).maRect ) &&
           ( maWallpaper == ((MetaWallpaperAction&)rMetaAction).maWallpaper );
}

// ------------------------------------------------------------------------

void MetaWallpaperAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maWallpaper;
}

// ------------------------------------------------------------------------

void MetaWallpaperAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maWallpaper;
}

// ========================================================================

IMPL_META_ACTION( ClipRegion, META_CLIPREGION_ACTION )

// ------------------------------------------------------------------------

MetaClipRegionAction::MetaClipRegionAction( const Region& rRegion, sal_Bool bClip ) :
    MetaAction  ( META_CLIPREGION_ACTION ),
    maRegion    ( rRegion ),
    mbClip      ( bClip )
{
}

// ------------------------------------------------------------------------

void MetaClipRegionAction::Execute( OutputDevice* pOut )
{
    if( mbClip )
        pOut->SetClipRegion( maRegion );
    else
        pOut->SetClipRegion();
}

// ------------------------------------------------------------------------

MetaAction* MetaClipRegionAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaClipRegionAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaClipRegionAction::Move( long nHorzMove, long nVertMove )
{
    maRegion.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    maRegion.Scale( fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaClipRegionAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRegion == ((MetaClipRegionAction&)rMetaAction).maRegion ) &&
           ( mbClip == ((MetaClipRegionAction&)rMetaAction).mbClip );
}

// ------------------------------------------------------------------------

void MetaClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRegion << mbClip;
}

// ------------------------------------------------------------------------

void MetaClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRegion >> mbClip;
}

// ========================================================================

IMPL_META_ACTION( ISectRectClipRegion, META_ISECTRECTCLIPREGION_ACTION )

// ------------------------------------------------------------------------

MetaISectRectClipRegionAction::MetaISectRectClipRegionAction( const Rectangle& rRect ) :
    MetaAction  ( META_ISECTRECTCLIPREGION_ACTION ),
    maRect      ( rRect )
{
}

// ------------------------------------------------------------------------

void MetaISectRectClipRegionAction::Execute( OutputDevice* pOut )
{
    pOut->IntersectClipRegion( maRect );
}

// ------------------------------------------------------------------------

MetaAction* MetaISectRectClipRegionAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaISectRectClipRegionAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaISectRectClipRegionAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaISectRectClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaISectRectClipRegionAction::Compare( const MetaAction& rMetaAction ) const
{
    return maRect == ((MetaISectRectClipRegionAction&)rMetaAction).maRect;
}

// ------------------------------------------------------------------------

void MetaISectRectClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRect;
}

// ------------------------------------------------------------------------

void MetaISectRectClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRect;
}

// ========================================================================

IMPL_META_ACTION( ISectRegionClipRegion, META_ISECTREGIONCLIPREGION_ACTION )

// ------------------------------------------------------------------------

MetaISectRegionClipRegionAction::MetaISectRegionClipRegionAction( const Region& rRegion ) :
    MetaAction  ( META_ISECTREGIONCLIPREGION_ACTION ),
    maRegion    ( rRegion )
{
}

// ------------------------------------------------------------------------

void MetaISectRegionClipRegionAction::Execute( OutputDevice* pOut )
{
    pOut->IntersectClipRegion( maRegion );
}

// ------------------------------------------------------------------------

MetaAction* MetaISectRegionClipRegionAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaISectRegionClipRegionAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaISectRegionClipRegionAction::Move( long nHorzMove, long nVertMove )
{
    maRegion.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaISectRegionClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    maRegion.Scale( fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaISectRegionClipRegionAction::Compare( const MetaAction& rMetaAction ) const
{
    return maRegion == ((MetaISectRegionClipRegionAction&)rMetaAction).maRegion;
}

// ------------------------------------------------------------------------

void MetaISectRegionClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRegion;
}

// ------------------------------------------------------------------------

void MetaISectRegionClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRegion;
}

// ========================================================================

IMPL_META_ACTION( MoveClipRegion, META_MOVECLIPREGION_ACTION )

// ------------------------------------------------------------------------

MetaMoveClipRegionAction::MetaMoveClipRegionAction( long nHorzMove, long nVertMove ) :
    MetaAction  ( META_MOVECLIPREGION_ACTION ),
    mnHorzMove  ( nHorzMove ),
    mnVertMove  ( nVertMove )
{
}

// ------------------------------------------------------------------------

void MetaMoveClipRegionAction::Execute( OutputDevice* pOut )
{
    pOut->MoveClipRegion( mnHorzMove, mnVertMove );
}

// ------------------------------------------------------------------------

MetaAction* MetaMoveClipRegionAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMoveClipRegionAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaMoveClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    mnHorzMove = FRound( mnHorzMove * fScaleX );
    mnVertMove = FRound( mnVertMove * fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaMoveClipRegionAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( mnHorzMove == ((MetaMoveClipRegionAction&)rMetaAction).mnHorzMove ) &&
           ( mnVertMove == ((MetaMoveClipRegionAction&)rMetaAction).mnVertMove );
}

// ------------------------------------------------------------------------

void MetaMoveClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    //#fdo39428 SvStream no longer supports operator<<(long)
    rOStm << sal::static_int_cast<sal_Int32>(mnHorzMove) << sal::static_int_cast<sal_Int32>(mnVertMove);
}

// ------------------------------------------------------------------------

void MetaMoveClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    //#fdo39428 SvStream no longer supports operator>>(long&)
    sal_Int32 nTmpHM(0), nTmpVM(0);
    rIStm >> nTmpHM >> nTmpVM;
    mnHorzMove = nTmpHM;
    mnVertMove = nTmpVM;
}

// ========================================================================

IMPL_META_ACTION( LineColor, META_LINECOLOR_ACTION )

// ------------------------------------------------------------------------

MetaLineColorAction::MetaLineColorAction( const Color& rColor, sal_Bool bSet ) :
    MetaAction  ( META_LINECOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{
}

// ------------------------------------------------------------------------

void MetaLineColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetLineColor( maColor );
    else
        pOut->SetLineColor();
}

// ------------------------------------------------------------------------

MetaAction* MetaLineColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaLineColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaLineColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaLineColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaLineColorAction&)rMetaAction).mbSet );
}

// ------------------------------------------------------------------------

void MetaLineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, sal_True );
    rOStm << mbSet;
}

// ------------------------------------------------------------------------

void MetaLineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, sal_True );
    rIStm >> mbSet;
}

// ========================================================================

IMPL_META_ACTION( FillColor, META_FILLCOLOR_ACTION )

// ------------------------------------------------------------------------

MetaFillColorAction::MetaFillColorAction( const Color& rColor, sal_Bool bSet ) :
    MetaAction  ( META_FILLCOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{
}

// ------------------------------------------------------------------------

void MetaFillColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetFillColor( maColor );
    else
        pOut->SetFillColor();
}

// ------------------------------------------------------------------------

MetaAction* MetaFillColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaFillColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaFillColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaFillColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaFillColorAction&)rMetaAction).mbSet );
}

// ------------------------------------------------------------------------

void MetaFillColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, sal_True );
    rOStm << mbSet;
}

// ------------------------------------------------------------------------

void MetaFillColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, sal_True );
    rIStm >> mbSet;
}

// ========================================================================

IMPL_META_ACTION( TextColor, META_TEXTCOLOR_ACTION )

// ------------------------------------------------------------------------

MetaTextColorAction::MetaTextColorAction( const Color& rColor ) :
    MetaAction  ( META_TEXTCOLOR_ACTION ),
    maColor     ( rColor )
{
}

// ------------------------------------------------------------------------

void MetaTextColorAction::Execute( OutputDevice* pOut )
{
    pOut->SetTextColor( maColor );
}

// ------------------------------------------------------------------------

MetaAction* MetaTextColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaTextColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return maColor == ((MetaTextColorAction&)rMetaAction).maColor;
}

// ------------------------------------------------------------------------

void MetaTextColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, sal_True );
}

// ------------------------------------------------------------------------

void MetaTextColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, sal_True );
}

// ========================================================================

IMPL_META_ACTION( TextFillColor, META_TEXTFILLCOLOR_ACTION )

// ------------------------------------------------------------------------

MetaTextFillColorAction::MetaTextFillColorAction( const Color& rColor, sal_Bool bSet ) :
    MetaAction  ( META_TEXTFILLCOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{
}

// ------------------------------------------------------------------------

void MetaTextFillColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetTextFillColor( maColor );
    else
        pOut->SetTextFillColor();
}

// ------------------------------------------------------------------------

MetaAction* MetaTextFillColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextFillColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaTextFillColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaTextFillColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaTextFillColorAction&)rMetaAction).mbSet );
}

// ------------------------------------------------------------------------

void MetaTextFillColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, sal_True );
    rOStm << mbSet;
}

// ------------------------------------------------------------------------

void MetaTextFillColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, sal_True );
    rIStm >> mbSet;
}

// ========================================================================

IMPL_META_ACTION( TextLineColor, META_TEXTLINECOLOR_ACTION )

// ------------------------------------------------------------------------

MetaTextLineColorAction::MetaTextLineColorAction( const Color& rColor, sal_Bool bSet ) :
    MetaAction  ( META_TEXTLINECOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{
}

// ------------------------------------------------------------------------

void MetaTextLineColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetTextLineColor( maColor );
    else
        pOut->SetTextLineColor();
}

// ------------------------------------------------------------------------

MetaAction* MetaTextLineColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextLineColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaTextLineColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaTextLineColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaTextLineColorAction&)rMetaAction).mbSet );
}

// ------------------------------------------------------------------------

void MetaTextLineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, sal_True );
    rOStm << mbSet;
}

// ------------------------------------------------------------------------

void MetaTextLineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, sal_True );
    rIStm >> mbSet;
}

// ========================================================================

IMPL_META_ACTION( OverlineColor, META_OVERLINECOLOR_ACTION )

// ------------------------------------------------------------------------

MetaOverlineColorAction::MetaOverlineColorAction( const Color& rColor, sal_Bool bSet ) :
    MetaAction  ( META_OVERLINECOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{
}

// ------------------------------------------------------------------------

void MetaOverlineColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetOverlineColor( maColor );
    else
        pOut->SetOverlineColor();
}

// ------------------------------------------------------------------------

MetaAction* MetaOverlineColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaOverlineColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaOverlineColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaOverlineColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaOverlineColorAction&)rMetaAction).mbSet );
}

// ------------------------------------------------------------------------

void MetaOverlineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, sal_True );
    rOStm << mbSet;
}

// ------------------------------------------------------------------------

void MetaOverlineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, sal_True );
    rIStm >> mbSet;
}

// ========================================================================

IMPL_META_ACTION( TextAlign, META_TEXTALIGN_ACTION )

// ------------------------------------------------------------------------

MetaTextAlignAction::MetaTextAlignAction( TextAlign aAlign ) :
    MetaAction  ( META_TEXTALIGN_ACTION ),
    maAlign     ( aAlign )
{
}

// ------------------------------------------------------------------------

void MetaTextAlignAction::Execute( OutputDevice* pOut )
{
    pOut->SetTextAlign( maAlign );
}

// ------------------------------------------------------------------------

MetaAction* MetaTextAlignAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextAlignAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaTextAlignAction::Compare( const MetaAction& rMetaAction ) const
{
    return maAlign == ((MetaTextAlignAction&)rMetaAction).maAlign;
}

// ------------------------------------------------------------------------

void MetaTextAlignAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << (sal_uInt16) maAlign;
}

// ------------------------------------------------------------------------

void MetaTextAlignAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    sal_uInt16 nTmp16;

    COMPAT( rIStm );
    rIStm >> nTmp16; maAlign = (TextAlign) nTmp16;
}

// ========================================================================

IMPL_META_ACTION( MapMode, META_MAPMODE_ACTION )

// ------------------------------------------------------------------------

MetaMapModeAction::MetaMapModeAction( const MapMode& rMapMode ) :
    MetaAction  ( META_MAPMODE_ACTION ),
    maMapMode   ( rMapMode )
{
}

// ------------------------------------------------------------------------

void MetaMapModeAction::Execute( OutputDevice* pOut )
{
    pOut->SetMapMode( maMapMode );
}

// ------------------------------------------------------------------------

MetaAction* MetaMapModeAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMapModeAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaMapModeAction::Scale( double fScaleX, double fScaleY )
{
    Point aPoint( maMapMode.GetOrigin() );

    ImplScalePoint( aPoint, fScaleX, fScaleY );
    maMapMode.SetOrigin( aPoint );
}

// ------------------------------------------------------------------------

sal_Bool MetaMapModeAction::Compare( const MetaAction& rMetaAction ) const
{
    return maMapMode == ((MetaMapModeAction&)rMetaAction).maMapMode;
}

// ------------------------------------------------------------------------

void MetaMapModeAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maMapMode;
}

// ------------------------------------------------------------------------

void MetaMapModeAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maMapMode;
}

// ========================================================================

IMPL_META_ACTION( Font, META_FONT_ACTION )

// ------------------------------------------------------------------------

MetaFontAction::MetaFontAction( const Font& rFont ) :
    MetaAction  ( META_FONT_ACTION ),
    maFont      ( rFont )
{
    // #96876: because RTL_TEXTENCODING_SYMBOL is often set at the StarSymbol font,
    // we change the textencoding to RTL_TEXTENCODING_UNICODE here, which seems
    // to be the right way; changing the textencoding at other sources
    // is too dangerous at the moment
    if( ( ( maFont.GetName().SearchAscii( "StarSymbol" ) != STRING_NOTFOUND )
       || ( maFont.GetName().SearchAscii( "OpenSymbol" ) != STRING_NOTFOUND ) )
     && ( maFont.GetCharSet() != RTL_TEXTENCODING_UNICODE ) )
    {
        maFont.SetCharSet( RTL_TEXTENCODING_UNICODE );
    }
}

// ------------------------------------------------------------------------

void MetaFontAction::Execute( OutputDevice* pOut )
{
    pOut->SetFont( maFont );
}

// ------------------------------------------------------------------------

MetaAction* MetaFontAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaFontAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaFontAction::Scale( double fScaleX, double fScaleY )
{
    const Size aSize(
        FRound(maFont.GetSize().Width() * fabs(fScaleX)),
        FRound(maFont.GetSize().Height() * fabs(fScaleY)));
    maFont.SetSize( aSize );
}

// ------------------------------------------------------------------------

sal_Bool MetaFontAction::Compare( const MetaAction& rMetaAction ) const
{
    return maFont == ((MetaFontAction&)rMetaAction).maFont;
}

// ------------------------------------------------------------------------

void MetaFontAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maFont;
    pData->meActualCharSet = maFont.GetCharSet();
    if ( pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW )
        pData->meActualCharSet = osl_getThreadTextEncoding();
}

// ------------------------------------------------------------------------

void MetaFontAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    COMPAT( rIStm );
    rIStm >> maFont;
    pData->meActualCharSet = maFont.GetCharSet();
    if ( pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW )
        pData->meActualCharSet = osl_getThreadTextEncoding();
}

// ========================================================================

IMPL_META_ACTION( Push, META_PUSH_ACTION )

// ------------------------------------------------------------------------

MetaPushAction::MetaPushAction( sal_uInt16 nFlags ) :
    MetaAction  ( META_PUSH_ACTION ),
    mnFlags     ( nFlags )
{
}

// ------------------------------------------------------------------------

void MetaPushAction::Execute( OutputDevice* pOut )
{
    pOut->Push( mnFlags );
}

// ------------------------------------------------------------------------

MetaAction* MetaPushAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPushAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaPushAction::Compare( const MetaAction& rMetaAction ) const
{
    return mnFlags == ((MetaPushAction&)rMetaAction).mnFlags;
}

// ------------------------------------------------------------------------

void MetaPushAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << mnFlags;
}

// ------------------------------------------------------------------------

void MetaPushAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> mnFlags;
}

// ========================================================================

IMPL_META_ACTION( Pop, META_POP_ACTION )

// ------------------------------------------------------------------------

void MetaPopAction::Execute( OutputDevice* pOut )
{
    pOut->Pop();
}

// ------------------------------------------------------------------------

MetaAction* MetaPopAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPopAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaPopAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
}

// ------------------------------------------------------------------------

void MetaPopAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
}

// ========================================================================

IMPL_META_ACTION( RasterOp, META_RASTEROP_ACTION )

// ------------------------------------------------------------------------

MetaRasterOpAction::MetaRasterOpAction( RasterOp eRasterOp ) :
    MetaAction  ( META_RASTEROP_ACTION ),
    meRasterOp  ( eRasterOp )
{
}

// ------------------------------------------------------------------------

void MetaRasterOpAction::Execute( OutputDevice* pOut )
{
    pOut->SetRasterOp( meRasterOp );
}

// ------------------------------------------------------------------------

MetaAction* MetaRasterOpAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaRasterOpAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaRasterOpAction::Compare( const MetaAction& rMetaAction ) const
{
    return meRasterOp == ((MetaRasterOpAction&)rMetaAction).meRasterOp;
}

// ------------------------------------------------------------------------

void MetaRasterOpAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << (sal_uInt16) meRasterOp;
}

// ------------------------------------------------------------------------

void MetaRasterOpAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    sal_uInt16 nTmp16;

    COMPAT( rIStm );
    rIStm >> nTmp16; meRasterOp = (RasterOp) nTmp16;
}

// ========================================================================

IMPL_META_ACTION( Transparent, META_TRANSPARENT_ACTION )

// ------------------------------------------------------------------------

MetaTransparentAction::MetaTransparentAction( const PolyPolygon& rPolyPoly, sal_uInt16 nTransPercent ) :
    MetaAction      ( META_TRANSPARENT_ACTION ),
    maPolyPoly      ( rPolyPoly ),
    mnTransPercent  ( nTransPercent )
{
}

// ------------------------------------------------------------------------

void MetaTransparentAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTransparent( maPolyPoly, mnTransPercent );
}

// ------------------------------------------------------------------------

MetaAction* MetaTransparentAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTransparentAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaTransparentAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaTransparentAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

sal_Bool MetaTransparentAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPolyPoly == ((MetaTransparentAction&)rMetaAction).maPolyPoly ) &&
           ( mnTransPercent == ((MetaTransparentAction&)rMetaAction).mnTransPercent );
}

// ------------------------------------------------------------------------

void MetaTransparentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );

    // #i105373# The PolyPolygon in this action may be a curve; this
    // was ignored until now what is an error. To make older office
    // versions work with MetaFiles, i opt for applying AdaptiveSubdivide
    // to the PolyPoylgon.
    // The alternative would be to really write the curve information
    // like in MetaPolyPolygonAction::Write (where someone extended it
    // correctly, but not here :-( ).
    // The golden solution would be to combine both, but i think it's
    // not necessary; a good subdivision will be sufficient.
    PolyPolygon aNoCurvePolyPolygon;
    maPolyPoly.AdaptiveSubdivide(aNoCurvePolyPolygon);

    rOStm << aNoCurvePolyPolygon;
    rOStm << mnTransPercent;
}

// ------------------------------------------------------------------------

void MetaTransparentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maPolyPoly;
    rIStm >> mnTransPercent;
}

// ========================================================================

IMPL_META_ACTION( FloatTransparent, META_FLOATTRANSPARENT_ACTION )

// ------------------------------------------------------------------------

MetaFloatTransparentAction::MetaFloatTransparentAction( const GDIMetaFile& rMtf, const Point& rPos,
                                                        const Size& rSize, const Gradient& rGradient ) :
    MetaAction      ( META_FLOATTRANSPARENT_ACTION ),
    maMtf           ( rMtf ),
    maPoint         ( rPos ),
    maSize          ( rSize ),
    maGradient      ( rGradient )
{
}

// ------------------------------------------------------------------------

void MetaFloatTransparentAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTransparent( maMtf, maPoint, maSize, maGradient );
}

// ------------------------------------------------------------------------

MetaAction* MetaFloatTransparentAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaFloatTransparentAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaFloatTransparentAction::Move( long nHorzMove, long nVertMove )
{
    maPoint.Move( nHorzMove, nVertMove );

    // also neeed to move the content metafile
    maMtf.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaFloatTransparentAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPoint, maSize);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPoint = aRectangle.TopLeft();
    maSize = aRectangle.GetSize();
}

// ------------------------------------------------------------------------

sal_Bool MetaFloatTransparentAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maMtf == ((MetaFloatTransparentAction&)rMetaAction).maMtf ) &&
           ( maPoint == ((MetaFloatTransparentAction&)rMetaAction).maPoint ) &&
           ( maSize == ((MetaFloatTransparentAction&)rMetaAction).maSize ) &&
           ( maGradient == ((MetaFloatTransparentAction&)rMetaAction).maGradient );
}

// ------------------------------------------------------------------------

void MetaFloatTransparentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );

    maMtf.Write( rOStm );
    rOStm << maPoint << maSize << maGradient;
}

// ------------------------------------------------------------------------

void MetaFloatTransparentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maMtf >> maPoint >> maSize >> maGradient;
}

// ========================================================================

IMPL_META_ACTION( EPS, META_EPS_ACTION )

// ------------------------------------------------------------------------

MetaEPSAction::MetaEPSAction( const Point& rPoint, const Size& rSize,
                              const GfxLink& rGfxLink, const GDIMetaFile& rSubst ) :
    MetaAction  ( META_EPS_ACTION ),
    maGfxLink   ( rGfxLink ),
    maSubst     ( rSubst ),
    maPoint     ( rPoint ),
    maSize      ( rSize )
{
}

// ------------------------------------------------------------------------

void MetaEPSAction::Execute( OutputDevice* pOut )
{
    pOut->DrawEPS( maPoint, maSize, maGfxLink, &maSubst );
}

// ------------------------------------------------------------------------

MetaAction* MetaEPSAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaEPSAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaEPSAction::Move( long nHorzMove, long nVertMove )
{
    maPoint.Move( nHorzMove, nVertMove );
}

// ------------------------------------------------------------------------

void MetaEPSAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPoint, maSize);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPoint = aRectangle.TopLeft();
    maSize = aRectangle.GetSize();
}

// ------------------------------------------------------------------------

sal_Bool MetaEPSAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maGfxLink.IsEqual(((MetaEPSAction&)rMetaAction).maGfxLink )) &&
           ( maSubst == ((MetaEPSAction&)rMetaAction).maSubst ) &&
           ( maPoint == ((MetaEPSAction&)rMetaAction).maPoint ) &&
           ( maSize == ((MetaEPSAction&)rMetaAction).maSize );
}

// ------------------------------------------------------------------------

void MetaEPSAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maGfxLink;
    rOStm << maPoint;
    rOStm << maSize;
    maSubst.Write( rOStm );
}

// ------------------------------------------------------------------------

void MetaEPSAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maGfxLink;
    rIStm >> maPoint;
    rIStm >> maSize;
    rIStm >> maSubst;
}

// ========================================================================

IMPL_META_ACTION( RefPoint, META_REFPOINT_ACTION )

// ------------------------------------------------------------------------

MetaRefPointAction::MetaRefPointAction( const Point& rRefPoint, sal_Bool bSet ) :
    MetaAction  ( META_REFPOINT_ACTION ),
    maRefPoint  ( rRefPoint ),
    mbSet       ( bSet )
{
}

// ------------------------------------------------------------------------

void MetaRefPointAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetRefPoint( maRefPoint );
    else
        pOut->SetRefPoint();
}

// ------------------------------------------------------------------------

MetaAction* MetaRefPointAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaRefPointAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaRefPointAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRefPoint == ((MetaRefPointAction&)rMetaAction).maRefPoint ) &&
           ( mbSet == ((MetaRefPointAction&)rMetaAction).mbSet );
}

// ------------------------------------------------------------------------

void MetaRefPointAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maRefPoint << mbSet;
}

// ------------------------------------------------------------------------

void MetaRefPointAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maRefPoint >> mbSet;
}

// ========================================================================

MetaCommentAction::MetaCommentAction( sal_Int32 nValue ) :
    MetaAction  ( META_COMMENT_ACTION ),
    mnValue     ( nValue )
{
    ImplInitDynamicData( NULL, 0UL );
}

// ------------------------------------------------------------------------

MetaCommentAction::MetaCommentAction( const MetaCommentAction& rAct ) :
    MetaAction  ( META_COMMENT_ACTION ),
    maComment   ( rAct.maComment ),
    mnValue     ( rAct.mnValue )
{
    ImplInitDynamicData( rAct.mpData, rAct.mnDataSize );
}

// ------------------------------------------------------------------------

MetaCommentAction::MetaCommentAction( const rtl::OString& rComment, sal_Int32 nValue, const sal_uInt8* pData, sal_uInt32 nDataSize ) :
    MetaAction  ( META_COMMENT_ACTION ),
    maComment   ( rComment ),
    mnValue     ( nValue )
{
    ImplInitDynamicData( pData, nDataSize );
}

// ------------------------------------------------------------------------

MetaCommentAction::~MetaCommentAction()
{
    if ( mpData )
        delete[] mpData;
}

// ------------------------------------------------------------------------

void MetaCommentAction::ImplInitDynamicData( const sal_uInt8* pData, sal_uInt32 nDataSize )
{
    if ( nDataSize && pData )
    {
        mnDataSize = nDataSize, mpData = new sal_uInt8[ mnDataSize ];
        memcpy( mpData, pData, mnDataSize );
    }
    else
    {
        mnDataSize = 0;
        mpData = NULL;
    }
}

// ------------------------------------------------------------------------

void MetaCommentAction::Execute( OutputDevice* pOut )
{
    if ( pOut->GetConnectMetaFile() )
    {
        Duplicate();
        pOut->GetConnectMetaFile()->AddAction( this );
    }
}

// ------------------------------------------------------------------------

MetaAction* MetaCommentAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaCommentAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaCommentAction::Move( long nXMove, long nYMove )
{
    if ( nXMove || nYMove )
    {
        if ( mnDataSize && mpData )
        {
            sal_Bool bPathStroke = (maComment == "XPATHSTROKE_SEQ_BEGIN");
            if ( bPathStroke || maComment == "XPATHFILL_SEQ_BEGIN" )
            {
                SvMemoryStream  aMemStm( (void*)mpData, mnDataSize, STREAM_READ );
                SvMemoryStream  aDest;
                if ( bPathStroke )
                {
                    SvtGraphicStroke aStroke;
                    aMemStm >> aStroke;
                    Polygon aPath;
                    aStroke.getPath( aPath );
                    aPath.Move( nXMove, nYMove );
                    aStroke.setPath( aPath );
                    aDest << aStroke;
                }
                else
                {
                    SvtGraphicFill aFill;
                    aMemStm >> aFill;
                    PolyPolygon aPath;
                    aFill.getPath( aPath );
                    aPath.Move( nXMove, nYMove );
                    aFill.setPath( aPath );
                    aDest << aFill;
                }
                delete[] mpData;
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            }
        }
    }
}

// ------------------------------------------------------------------------
// SJ: 25.07.06 #i56656# we are not able to mirrorcertain kind of
// comments properly, especially the XPATHSTROKE and XPATHFILL lead to
// problems, so it is better to remove these comments when mirroring
// FIXME: fake comment to apply the next hunk in the right location
void MetaCommentAction::Scale( double fXScale, double fYScale )
{
    if ( ( fXScale != 1.0 ) || ( fYScale != 1.0 ) )
    {
        if ( mnDataSize && mpData )
        {
            sal_Bool bPathStroke = (maComment == "XPATHSTROKE_SEQ_BEGIN");
            if ( bPathStroke || maComment == "XPATHFILL_SEQ_BEGIN" )
            {
                SvMemoryStream  aMemStm( (void*)mpData, mnDataSize, STREAM_READ );
                SvMemoryStream  aDest;
                if ( bPathStroke )
                {
                    SvtGraphicStroke aStroke;
                    aMemStm >> aStroke;
                    Polygon aPath;
                    aStroke.getPath( aPath );
                    aPath.Scale( fXScale, fYScale );
                    aStroke.setPath( aPath );
                    aDest << aStroke;
                }
                else
                {
                    SvtGraphicFill aFill;
                    aMemStm >> aFill;
                    PolyPolygon aPath;
                    aFill.getPath( aPath );
                    aPath.Scale( fXScale, fYScale );
                    aFill.setPath( aPath );
                    aDest << aFill;
                }
                delete[] mpData;
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            } else if( maComment == "EMF_PLUS_HEADER_INFO" ){
                SvMemoryStream  aMemStm( (void*)mpData, mnDataSize, STREAM_READ );
                SvMemoryStream  aDest;

                sal_Int32 nLeft, nRight, nTop, nBottom;
                sal_Int32 nPixX, nPixY, nMillX, nMillY;
                float m11, m12, m21, m22, mdx, mdy;

                // read data
                aMemStm >> nLeft >> nTop >> nRight >> nBottom;
                aMemStm >> nPixX >> nPixY >> nMillX >> nMillY;
                aMemStm >> m11 >> m12 >> m21 >> m22 >> mdx >> mdy;

                // add scale to the transformation
                m11 *= fXScale;
                m12 *= fXScale;
                m22 *= fYScale;
                m21 *= fYScale;

                // prepare new data
                aDest << nLeft << nTop << nRight << nBottom;
                aDest << nPixX << nPixY << nMillX << nMillY;
                aDest << m11 << m12 << m21 << m22 << mdx << mdy;

                // save them
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            }
        }
    }
}

// ------------------------------------------------------------------------

sal_Bool MetaCommentAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maComment == ((MetaCommentAction&)rMetaAction).maComment ) &&
           ( mnValue == ((MetaCommentAction&)rMetaAction).mnValue ) &&
           ( mnDataSize == ((MetaCommentAction&)rMetaAction).mnDataSize ) &&
           ( memcmp( mpData, ((MetaCommentAction&)rMetaAction).mpData, mnDataSize ) == 0 );
}

// ------------------------------------------------------------------------

void MetaCommentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    write_lenPrefixed_uInt8s_FromOString<sal_uInt16>(rOStm, maComment);
    rOStm << mnValue << mnDataSize;

    if ( mnDataSize )
        rOStm.Write( mpData, mnDataSize );
}

// ------------------------------------------------------------------------

void MetaCommentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maComment = read_lenPrefixed_uInt8s_ToOString<sal_uInt16>(rIStm);
    rIStm >> mnValue >> mnDataSize;

    delete[] mpData;

    if( mnDataSize )
    {
        mpData = new sal_uInt8[ mnDataSize ];
        rIStm.Read( mpData, mnDataSize );
    }
    else
        mpData = NULL;
}

// ========================================================================

IMPL_META_ACTION( LayoutMode, META_LAYOUTMODE_ACTION )

// ------------------------------------------------------------------------

MetaLayoutModeAction::MetaLayoutModeAction( sal_uInt32 nLayoutMode ) :
    MetaAction  ( META_LAYOUTMODE_ACTION ),
    mnLayoutMode( nLayoutMode )
{
}

// ------------------------------------------------------------------------

void MetaLayoutModeAction::Execute( OutputDevice* pOut )
{
    pOut->SetLayoutMode( mnLayoutMode );
}

// ------------------------------------------------------------------------

MetaAction* MetaLayoutModeAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaLayoutModeAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaLayoutModeAction::Compare( const MetaAction& rMetaAction ) const
{
    return mnLayoutMode == ((MetaLayoutModeAction&)rMetaAction).mnLayoutMode;
}

// ------------------------------------------------------------------------

void MetaLayoutModeAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << mnLayoutMode;
}

// ------------------------------------------------------------------------

void MetaLayoutModeAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> mnLayoutMode;
}

// ========================================================================

IMPL_META_ACTION( TextLanguage, META_TEXTLANGUAGE_ACTION )

// ------------------------------------------------------------------------

MetaTextLanguageAction::MetaTextLanguageAction( LanguageType eTextLanguage ) :
    MetaAction  ( META_TEXTLANGUAGE_ACTION ),
    meTextLanguage( eTextLanguage )
{
}

// ------------------------------------------------------------------------

void MetaTextLanguageAction::Execute( OutputDevice* pOut )
{
    pOut->SetDigitLanguage( meTextLanguage );
}

// ------------------------------------------------------------------------

MetaAction* MetaTextLanguageAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextLanguageAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

sal_Bool MetaTextLanguageAction::Compare( const MetaAction& rMetaAction ) const
{
    return meTextLanguage == ((MetaTextLanguageAction&)rMetaAction).meTextLanguage;
}

// ------------------------------------------------------------------------

void MetaTextLanguageAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << meTextLanguage;
}

// ------------------------------------------------------------------------

void MetaTextLanguageAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> meTextLanguage;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
