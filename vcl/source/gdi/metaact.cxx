/*************************************************************************
 *
 *  $RCSfile: metaact.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-22 17:51:28 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#define _SV_METAACT_CXX
#define ENABLE_BYTESTRING_STREAM_OPERATORS

#include <string.h>

#ifndef _STREAM_HXX
#include <tools/stream.hxx>
#endif
#ifndef _VCOMPAT_HXX
#include <tools/vcompat.hxx>
#endif
#ifndef _SV_OUTDEV_HXX
#include <outdev.hxx>
#endif
#ifndef _SV_SALBTYPE_HXX
#include <salbtype.hxx>
#endif
#ifndef _SV_METAACT_HXX
#include <metaact.hxx>
#endif

// ========================================================================

inline void ImplScalePoint( Point& rPt, double fScaleX, double fScaleY )
{
    rPt.X() = FRound( fScaleX * rPt.X() );
    rPt.Y() = FRound( fScaleY * rPt.Y() );
}

// ------------------------------------------------------------------------

inline void ImplScaleSize( Size& rSz, double fScaleX, double fScaleY )
{
    rSz.Width() = FRound( fScaleX * rSz.Width() );
    rSz.Height() = FRound( fScaleY * rSz.Height() );
}

// ------------------------------------------------------------------------

inline void ImplScaleRect( Rectangle& rRect, double fScaleX, double fScaleY )
{
    Point aTL( rRect.TopLeft() );
    Point aBR( rRect.BottomRight() );

    ImplScalePoint( aTL, fScaleX, fScaleY );
    ImplScalePoint( aBR, fScaleX, fScaleY );

    rRect = Rectangle( aTL, aBR );
}

// ------------------------------------------------------------------------

inline void ImplScalePoly( Polygon& rPoly, double fScaleX, double fScaleY )
{
    for( USHORT i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
        ImplScalePoint( rPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

inline void ImplScaleLineInfo( LineInfo& rLineInfo, double fScaleX, double fScaleY )
{
    if( !rLineInfo.IsDefault() )
    {
        const double fScale = ( fScaleX + fScaleY ) * 0.5;

        rLineInfo.SetWidth( FRound( fScale * rLineInfo.GetWidth() ) );
        rLineInfo.SetDashLen( FRound( fScale * rLineInfo.GetDashLen() ) );
        rLineInfo.SetDotLen( FRound( fScale * rLineInfo.GetDotLen() ) );
        rLineInfo.SetDistance( FRound( fScale * rLineInfo.GetDistance() ) );
    }
}

// ========================================================================

#define COMPAT( _def_rIStm ) VersionCompat aCompat( ( _def_rIStm ), STREAM_READ );
#define COMPAT_VERSION() aCompat.GetVersion()
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

MetaAction::MetaAction( USHORT nType ) :
    mnRefCount( 1 ),
    mnType( nType )
{
}

// ------------------------------------------------------------------------

MetaAction::~MetaAction()
{
}

// ------------------------------------------------------------------------

void MetaAction::Execute( OutputDevice* pOut )
{
}

// ------------------------------------------------------------------------

MetaAction* MetaAction::Clone()
{
    return new MetaAction;
}

// ------------------------------------------------------------------------

void MetaAction::Move( long nHorzMove, long nVertMove )
{
}

// ------------------------------------------------------------------------

void MetaAction::Scale( double fScaleX, double fScaleY )
{
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
    UINT16      nType;

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

void MetaPixelAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maPt;
    maColor.Write( rOStm, TRUE );
}

// ------------------------------------------------------------------------

void MetaPixelAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maPt;
    maColor.Read( rIStm, TRUE );
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
                                          long nHorzRound, long nVertRound ) :
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
    mnHorzRound = FRound( mnHorzRound * fScaleX );
    mnVertRound = FRound( mnVertRound * fScaleY );
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

void MetaPolyLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 2, pData );

    rOStm << maPoly;        // Version 1
    rOStm << maLineInfo;    // Version 2
}

// ------------------------------------------------------------------------

void MetaPolyLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );

    // Version 1
    rIStm >> maPoly;

    // Version 2
    if( aCompat.GetVersion() >= 2 )
    {
        rIStm >> maLineInfo;
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

void MetaPolygonAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maPoly;
}

// ------------------------------------------------------------------------

void MetaPolygonAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maPoly;
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
    for( USHORT i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

void MetaPolyPolygonAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maPolyPoly;
}

// ------------------------------------------------------------------------

void MetaPolyPolygonAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maPolyPoly;
}

// ========================================================================

IMPL_META_ACTION( Text, META_TEXT_ACTION )

// ------------------------------------------------------------------------

MetaTextAction::MetaTextAction( const Point& rPt, const XubString& rStr,
                                USHORT nIndex, USHORT nLen ) :
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

void MetaTextAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm   << maPt;
    rOStm.WriteByteString( maStr, pData->meActualCharSet );
    rOStm   << mnIndex;
    rOStm   << mnLen;
}

// ------------------------------------------------------------------------

void MetaTextAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    COMPAT( rIStm );
    rIStm   >> maPt;
    rIStm.ReadByteString( maStr, pData->meActualCharSet );
    rIStm   >> mnIndex;
    rIStm   >> mnLen;
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
        const ULONG nAryLen = mnLen - 1;

        mpDXAry = new long[ nAryLen ];
        memcpy( mpDXAry, rAction.mpDXAry, nAryLen * sizeof( long ) );
    }
    else
        mpDXAry = NULL;
}

// ------------------------------------------------------------------------

MetaTextArrayAction::MetaTextArrayAction( const Point& rStartPt,
                                          const XubString& rStr,
                                          const long* pDXAry,
                                          USHORT nIndex,
                                          USHORT nLen ) :
    MetaAction  ( META_TEXTARRAY_ACTION ),
    maStartPt   ( rStartPt ),
    maStr       ( rStr ),
    mnIndex     ( nIndex ),
    mnLen       ( ( nLen == STRING_LEN ) ? rStr.Len() : nLen )
{
    const ULONG nAryLen = ( ( mnLen > 1 ) && pDXAry ) ? ( mnLen - 1 ) : 0UL;

    if( nAryLen )
    {
        mpDXAry = new long[ nAryLen ];
        memcpy( mpDXAry, pDXAry, nAryLen * sizeof( long ) );
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
        for ( USHORT i = 0, nCount = mnLen - 1; i < nCount; i++ )
            mpDXAry[ i ] = FRound( mpDXAry[ i ] * fScaleX );
    }
}

// ------------------------------------------------------------------------

void MetaTextArrayAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    const ULONG nAryLen = ( ( mnLen > 1 ) && mpDXAry ) ? ( mnLen - 1 ) : 0UL;

    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm   << maStartPt;
    rOStm.WriteByteString( maStr, pData->meActualCharSet );
    rOStm   << mnIndex;
    rOStm   << mnLen;
    rOStm   << nAryLen;

    for( ULONG i = 0UL; i < nAryLen; i++ )
        rOStm << mpDXAry[ i ];
}

// ------------------------------------------------------------------------

void MetaTextArrayAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    ULONG nAryLen;

    delete[] mpDXAry;

    COMPAT( rIStm );
    rIStm   >> maStartPt;
    rIStm.ReadByteString( maStr, pData->meActualCharSet );
    rIStm   >> mnIndex;
    rIStm   >> mnLen;
    rIStm   >> nAryLen;

    if( nAryLen )
    {
        mpDXAry = new long[ nAryLen ];

        for( ULONG i = 0UL; i < nAryLen; i++ )
            rIStm >> mpDXAry[ i ];
    }
    else
        mpDXAry = NULL;
}

// ========================================================================

IMPL_META_ACTION( StretchText, META_STRETCHTEXT_ACTION )

// ------------------------------------------------------------------------

MetaStretchTextAction::MetaStretchTextAction( const Point& rPt, ULONG nWidth,
                                              const XubString& rStr,
                                              USHORT nIndex, USHORT nLen ) :
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
    mnWidth = (ULONG)FRound( mnWidth * fScaleX );
}

// ------------------------------------------------------------------------

void MetaStretchTextAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm   << maPt;
    rOStm.WriteByteString( maStr, pData->meActualCharSet );
    rOStm   << mnWidth;
    rOStm   << mnIndex;
    rOStm   << mnLen;
}

// ------------------------------------------------------------------------

void MetaStretchTextAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    COMPAT( rIStm );
    rIStm   >> maPt;
    rIStm.ReadByteString( maStr, pData->meActualCharSet );
    rIStm   >> mnWidth;
    rIStm   >> mnIndex;
    rIStm   >> mnLen;
}

// ========================================================================

IMPL_META_ACTION( TextRect, META_TEXTRECT_ACTION )

// ------------------------------------------------------------------------

MetaTextRectAction::MetaTextRectAction( const Rectangle& rRect,
                                        const XubString& rStr, USHORT nStyle ) :
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

void MetaTextRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm   << maRect;
    rOStm.WriteByteString( maStr, pData->meActualCharSet );
    rOStm   << mnStyle;
}

// ------------------------------------------------------------------------

void MetaTextRectAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    COMPAT( rIStm );
    rIStm   >> maRect;
    rIStm.ReadByteString( maStr, pData->meActualCharSet );
    rIStm   >> mnStyle;
}

// ========================================================================

IMPL_META_ACTION( TextLine, META_TEXTLINE_ACTION )

// ------------------------------------------------------------------------

MetaTextLineAction::MetaTextLineAction( const Point& rPos, long nWidth,
                                        FontStrikeout eStrikeout,
                                        FontUnderline eUnderline ) :
    MetaAction  ( META_TEXTLINE_ACTION ),
    maPos       ( rPos ),
    mnWidth     ( nWidth ),
    meStrikeout ( eStrikeout ),
    meUnderline ( eUnderline )
{
}

// ------------------------------------------------------------------------

void MetaTextLineAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTextLine( maPos, mnWidth, meStrikeout, meUnderline );
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
    mnWidth = FRound( mnWidth * fScaleX );
}

// ------------------------------------------------------------------------

void MetaTextLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 2, pData );

    rOStm << maPos;
    rOStm << mnWidth;
    rOStm << (ULONG)meStrikeout;
    rOStm << (ULONG)meUnderline;
}

// ------------------------------------------------------------------------

void MetaTextLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );

    ULONG nTemp;
    rIStm >> maPos;
    rIStm >> mnWidth;
    rIStm >> nTemp;
    meStrikeout = (FontStrikeout)nTemp;
    rIStm >> nTemp;
    meUnderline = (FontUnderline)nTemp;
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
    ImplScalePoint( maPt, fScaleX, fScaleY );
    ImplScaleSize( maSz, fScaleX, fScaleY );
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
    ImplScalePoint( maDstPt, fScaleX, fScaleY );
    ImplScaleSize( maDstSz, fScaleX, fScaleY );
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
    MetaAction* pClone = (MetaAction*) new MetaBmpExAction( *this );
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
    ImplScalePoint( maPt, fScaleX, fScaleY );
    ImplScaleSize( maSz, fScaleX, fScaleY );
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
    ImplScalePoint( maDstPt, fScaleX, fScaleY );
    ImplScaleSize( maDstSz, fScaleX, fScaleY );
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
    ImplScalePoint( maPt, fScaleX, fScaleY );
    ImplScaleSize( maSz, fScaleX, fScaleY );
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
    ImplScalePoint( maDstPt, fScaleX, fScaleY );
    ImplScaleSize( maDstSz, fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

void MetaMaskScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        WRITE_BASE_COMPAT( rOStm, 1, pData );
        rOStm << maBmp;
        maColor.Write( rOStm, TRUE );
        rOStm << maDstPt << maDstSz << maSrcPt << maSrcSz;
    }
}

// ------------------------------------------------------------------------

void MetaMaskScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maBmp;
    maColor.Read( rIStm, TRUE );
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
        pOut->GetConnectMetaFile()->AddAction( Clone() );
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
    for( USHORT i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

void MetaGradientExAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maPolyPoly << maGradient;
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
    for( USHORT i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

void MetaHatchAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maPolyPoly << maHatch;
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

MetaClipRegionAction::MetaClipRegionAction( const Region& rRegion, BOOL bClip ) :
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

void MetaMoveClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << mnHorzMove << mnVertMove;
}

// ------------------------------------------------------------------------

void MetaMoveClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> mnHorzMove >> mnVertMove;
}

// ========================================================================

IMPL_META_ACTION( LineColor, META_LINECOLOR_ACTION )

// ------------------------------------------------------------------------

MetaLineColorAction::MetaLineColorAction( const Color& rColor, BOOL bSet ) :
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

void MetaLineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, TRUE );
    rOStm << mbSet;
}

// ------------------------------------------------------------------------

void MetaLineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, TRUE );
    rIStm >> mbSet;
}

// ========================================================================

IMPL_META_ACTION( FillColor, META_FILLCOLOR_ACTION )

// ------------------------------------------------------------------------

MetaFillColorAction::MetaFillColorAction( const Color& rColor, BOOL bSet ) :
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

void MetaFillColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, TRUE );
    rOStm << mbSet;
}

// ------------------------------------------------------------------------

void MetaFillColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, TRUE );
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

void MetaTextColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, TRUE );
}

// ------------------------------------------------------------------------

void MetaTextColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, TRUE );
}

// ========================================================================

IMPL_META_ACTION( TextFillColor, META_TEXTFILLCOLOR_ACTION )

// ------------------------------------------------------------------------

MetaTextFillColorAction::MetaTextFillColorAction( const Color& rColor, BOOL bSet ) :
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

void MetaTextFillColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, TRUE );
    rOStm << mbSet;
}

// ------------------------------------------------------------------------

void MetaTextFillColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, TRUE );
    rIStm >> mbSet;
}

// ========================================================================

IMPL_META_ACTION( TextLineColor, META_TEXTLINECOLOR_ACTION )

// ------------------------------------------------------------------------

MetaTextLineColorAction::MetaTextLineColorAction( const Color& rColor, BOOL bSet ) :
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

void MetaTextLineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    maColor.Write( rOStm, TRUE );
    rOStm << mbSet;
}

// ------------------------------------------------------------------------

void MetaTextLineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    maColor.Read( rIStm, TRUE );
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

void MetaTextAlignAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << (UINT16) maAlign;
}

// ------------------------------------------------------------------------

void MetaTextAlignAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    UINT16 nTmp16;

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
    Size aSize( maFont.GetSize() );

    ImplScaleSize( aSize, fScaleX, fScaleY );
    maFont.SetSize( aSize );
}

// ------------------------------------------------------------------------

void MetaFontAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maFont;
    pData->meActualCharSet = maFont.GetCharSet();
    if ( pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW )
        pData->meActualCharSet = gsl_getSystemTextEncoding();
}

// ------------------------------------------------------------------------

void MetaFontAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    COMPAT( rIStm );
    rIStm >> maFont;
    pData->meActualCharSet = maFont.GetCharSet();
    if ( pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW )
        pData->meActualCharSet = gsl_getSystemTextEncoding();
}

// ========================================================================

IMPL_META_ACTION( Push, META_PUSH_ACTION )

// ------------------------------------------------------------------------

MetaPushAction::MetaPushAction( USHORT nFlags ) :
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

void MetaRasterOpAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << (UINT16) meRasterOp;
}

// ------------------------------------------------------------------------

void MetaRasterOpAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    UINT16 nTmp16;

    COMPAT( rIStm );
    rIStm >> nTmp16; meRasterOp = (RasterOp) nTmp16;
}

// ========================================================================

IMPL_META_ACTION( Transparent, META_TRANSPARENT_ACTION )

// ------------------------------------------------------------------------

MetaTransparentAction::MetaTransparentAction( const PolyPolygon& rPolyPoly, USHORT nTransPercent ) :
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
    for( USHORT i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

// ------------------------------------------------------------------------

void MetaTransparentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maPolyPoly;
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
}

// ------------------------------------------------------------------------

void MetaFloatTransparentAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPoint, fScaleX, fScaleY );
    ImplScaleSize( maSize, fScaleX, fScaleY );
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
    ImplScalePoint( maPoint, fScaleX, fScaleY );
    ImplScaleSize( maSize, fScaleX, fScaleY );
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

MetaRefPointAction::MetaRefPointAction( const Point& rRefPoint, BOOL bSet ) :
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

MetaCommentAction::MetaCommentAction( long nValue ) :
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

MetaCommentAction::MetaCommentAction( const ByteString& rComment, long nValue, const BYTE* pData, ULONG nDataSize ) :
    MetaAction  ( META_COMMENT_ACTION ),
    maComment   ( rComment ),
    mnValue     ( nValue )
{
    ImplInitDynamicData( pData, nDataSize );
}

// ------------------------------------------------------------------------

MetaCommentAction::MetaCommentAction( const BYTE* pData, ULONG nDataSize ) :
    MetaAction  ( META_COMMENT_ACTION ),
    mnValue     ( 0L )
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

void MetaCommentAction::ImplInitDynamicData( const BYTE* pData, ULONG nDataSize )
{
    if ( nDataSize && pData )
    {
        mnDataSize = nDataSize, mpData = new BYTE[ mnDataSize ];
        HMEMCPY( mpData, pData, mnDataSize );
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
        pOut->GetConnectMetaFile()->AddAction( Clone() );
}

// ------------------------------------------------------------------------

MetaAction* MetaCommentAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaCommentAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

// ------------------------------------------------------------------------

void MetaCommentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    WRITE_BASE_COMPAT( rOStm, 1, pData );
    rOStm << maComment << mnValue << mnDataSize;

    if ( mnDataSize )
        rOStm.Write( mpData, mnDataSize );
}

// ------------------------------------------------------------------------

void MetaCommentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    COMPAT( rIStm );
    rIStm >> maComment >> mnValue >> mnDataSize;

    if( mpData )
        delete[] mpData;

    if( mnDataSize )
    {
        mpData = new BYTE[ mnDataSize ];
        rIStm.Read( mpData, mnDataSize );
    }
    else
        mpData = NULL;
}
