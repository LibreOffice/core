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
#include <osl/thread.h>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/helpers.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/outdev.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphictools.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <unotools/fontdefs.hxx>

namespace
{

inline void ImplScalePoint( Point& rPt, double fScaleX, double fScaleY )
{
    rPt.X() = FRound( fScaleX * rPt.X() );
    rPt.Y() = FRound( fScaleY * rPt.Y() );
}

inline void ImplScaleRect( Rectangle& rRect, double fScaleX, double fScaleY )
{
    Point aTL( rRect.TopLeft() );
    Point aBR( rRect.BottomRight() );

    ImplScalePoint( aTL, fScaleX, fScaleY );
    ImplScalePoint( aBR, fScaleX, fScaleY );

    rRect = Rectangle( aTL, aBR );
    rRect.Justify();
}

inline void ImplScalePoly( Polygon& rPoly, double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
        ImplScalePoint( rPoly[ i ], fScaleX, fScaleY );
}

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

} //anonymous namespace

MetaAction::MetaAction() :
    mnRefCount( 1 ),
    mnType( META_NULL_ACTION )
{
}

MetaAction::MetaAction( sal_uInt16 nType ) :
    mnRefCount( 1 ),
    mnType( nType )
{
}

MetaAction::~MetaAction()
{
}

void MetaAction::Execute( OutputDevice* )
{
}

MetaAction* MetaAction::Clone()
{
    return new MetaAction;
}

void MetaAction::Move( long, long )
{
}

void MetaAction::Scale( double, double )
{
}

bool MetaAction::Compare( const MetaAction& ) const
{
    return true;
}

void MetaAction::Write( SvStream& rOStm, ImplMetaWriteData* )
{
    rOStm.WriteUInt16( mnType );
}

void MetaAction::Read( SvStream&, ImplMetaReadData* )
{
    // DO NOT read mnType - ReadMetaAction already did that!
}

MetaAction* MetaAction::ReadMetaAction( SvStream& rIStm, ImplMetaReadData* pData )
{
    MetaAction* pAction = NULL;
    sal_uInt16 nType = 0;

    rIStm.ReadUInt16( nType );

    SAL_INFO("vcl.gdi", "ReadMetaAction " << nType);

    switch( nType )
    {
        case META_NULL_ACTION: pAction = new MetaAction; break;
        case META_PIXEL_ACTION: pAction = new MetaPixelAction; break;
        case META_POINT_ACTION: pAction = new MetaPointAction; break;
        case META_LINE_ACTION: pAction = new MetaLineAction; break;
        case META_RECT_ACTION: pAction = new MetaRectAction; break;
        case META_ROUNDRECT_ACTION: pAction = new MetaRoundRectAction; break;
        case META_ELLIPSE_ACTION: pAction = new MetaEllipseAction; break;
        case META_ARC_ACTION: pAction = new MetaArcAction; break;
        case META_PIE_ACTION: pAction = new MetaPieAction; break;
        case META_CHORD_ACTION: pAction = new MetaChordAction; break;
        case META_POLYLINE_ACTION: pAction = new MetaPolyLineAction; break;
        case META_POLYGON_ACTION: pAction = new MetaPolygonAction; break;
        case META_POLYPOLYGON_ACTION: pAction = new MetaPolyPolygonAction; break;
        case META_TEXT_ACTION: pAction = new MetaTextAction; break;
        case META_TEXTARRAY_ACTION: pAction = new MetaTextArrayAction; break;
        case META_STRETCHTEXT_ACTION: pAction = new MetaStretchTextAction; break;
        case META_TEXTRECT_ACTION: pAction = new MetaTextRectAction; break;
        case META_TEXTLINE_ACTION: pAction = new MetaTextLineAction; break;
        case META_BMP_ACTION: pAction = new MetaBmpAction; break;
        case META_BMPSCALE_ACTION: pAction = new MetaBmpScaleAction; break;
        case META_BMPSCALEPART_ACTION: pAction = new MetaBmpScalePartAction; break;
        case META_BMPEX_ACTION: pAction = new MetaBmpExAction; break;
        case META_BMPEXSCALE_ACTION: pAction = new MetaBmpExScaleAction; break;
        case META_BMPEXSCALEPART_ACTION: pAction = new MetaBmpExScalePartAction; break;
        case META_MASK_ACTION: pAction = new MetaMaskAction; break;
        case META_MASKSCALE_ACTION: pAction = new MetaMaskScaleAction; break;
        case META_MASKSCALEPART_ACTION: pAction = new MetaMaskScalePartAction; break;
        case META_GRADIENT_ACTION: pAction = new MetaGradientAction; break;
        case META_GRADIENTEX_ACTION: pAction = new MetaGradientExAction; break;
        case META_HATCH_ACTION: pAction = new MetaHatchAction; break;
        case META_WALLPAPER_ACTION: pAction = new MetaWallpaperAction; break;
        case META_CLIPREGION_ACTION: pAction = new MetaClipRegionAction; break;
        case META_ISECTRECTCLIPREGION_ACTION: pAction = new MetaISectRectClipRegionAction; break;
        case META_ISECTREGIONCLIPREGION_ACTION: pAction = new MetaISectRegionClipRegionAction; break;
        case META_MOVECLIPREGION_ACTION: pAction = new MetaMoveClipRegionAction; break;
        case META_LINECOLOR_ACTION: pAction = new MetaLineColorAction; break;
        case META_FILLCOLOR_ACTION: pAction = new MetaFillColorAction; break;
        case META_TEXTCOLOR_ACTION: pAction = new MetaTextColorAction; break;
        case META_TEXTFILLCOLOR_ACTION: pAction = new MetaTextFillColorAction; break;
        case META_TEXTLINECOLOR_ACTION: pAction = new MetaTextLineColorAction; break;
        case META_OVERLINECOLOR_ACTION: pAction = new MetaOverlineColorAction; break;
        case META_TEXTALIGN_ACTION: pAction = new MetaTextAlignAction; break;
        case META_MAPMODE_ACTION: pAction = new MetaMapModeAction; break;
        case META_FONT_ACTION: pAction = new MetaFontAction; break;
        case META_PUSH_ACTION: pAction = new MetaPushAction; break;
        case META_POP_ACTION: pAction = new MetaPopAction; break;
        case META_RASTEROP_ACTION: pAction = new MetaRasterOpAction; break;
        case META_TRANSPARENT_ACTION: pAction = new MetaTransparentAction; break;
        case META_FLOATTRANSPARENT_ACTION: pAction = new MetaFloatTransparentAction; break;
        case META_EPS_ACTION: pAction = new MetaEPSAction; break;
        case META_REFPOINT_ACTION: pAction = new MetaRefPointAction; break;
        case META_COMMENT_ACTION: pAction = new MetaCommentAction; break;
        case META_LAYOUTMODE_ACTION: pAction = new MetaLayoutModeAction; break;
        case META_TEXTLANGUAGE_ACTION: pAction = new MetaTextLanguageAction; break;

        default:
        {
            VersionCompat aCompat(rIStm, STREAM_READ);
        }
        break;
    }

    if( pAction )
        pAction->Read( rIStm, pData );

    return pAction;
}

MetaPixelAction::MetaPixelAction() :
    MetaAction(META_PIXEL_ACTION)
{}

MetaPixelAction::~MetaPixelAction()
{}

MetaPixelAction::MetaPixelAction( const Point& rPt, const Color& rColor ) :
    MetaAction  ( META_PIXEL_ACTION ),
    maPt        ( rPt ),
    maColor     ( rColor )
{}

void MetaPixelAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPixel( maPt, maColor );
}

MetaAction* MetaPixelAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPixelAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaPixelAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaPixelAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

bool MetaPixelAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPt == ((MetaPixelAction&)rMetaAction).maPt ) &&
           ( maColor == ((MetaPixelAction&)rMetaAction).maColor );
}

void MetaPixelAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WritePair( rOStm, maPt );
    maColor.Write( rOStm, true );
}

void MetaPixelAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPair( rIStm, maPt );
    maColor.Read( rIStm, true );
}

MetaPointAction::MetaPointAction() :
    MetaAction(META_POINT_ACTION)
{}

MetaPointAction::~MetaPointAction()
{}

MetaPointAction::MetaPointAction( const Point& rPt ) :
    MetaAction  ( META_POINT_ACTION ),
    maPt        ( rPt )
{}

void MetaPointAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPixel( maPt );
}

MetaAction* MetaPointAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPointAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaPointAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaPointAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

bool MetaPointAction::Compare( const MetaAction& rMetaAction ) const
{
    return maPt == ((MetaPointAction&)rMetaAction).maPt;
}

void MetaPointAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WritePair( rOStm, maPt );
}

void MetaPointAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPair( rIStm, maPt );
}

MetaLineAction::MetaLineAction() :
    MetaAction(META_LINE_ACTION)
{}

MetaLineAction::~MetaLineAction()
{}

MetaLineAction::MetaLineAction( const Point& rStart, const Point& rEnd ) :
    MetaAction  ( META_LINE_ACTION ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{}

MetaLineAction::MetaLineAction( const Point& rStart, const Point& rEnd,
                                const LineInfo& rLineInfo ) :
    MetaAction  ( META_LINE_ACTION ),
    maLineInfo  ( rLineInfo ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{}

void MetaLineAction::Execute( OutputDevice* pOut )
{
    if( maLineInfo.IsDefault() )
        pOut->DrawLine( maStartPt, maEndPt );
    else
        pOut->DrawLine( maStartPt, maEndPt, maLineInfo );
}

MetaAction* MetaLineAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaLineAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaLineAction::Move( long nHorzMove, long nVertMove )
{
    maStartPt.Move( nHorzMove, nVertMove );
    maEndPt.Move( nHorzMove, nVertMove );
}

void MetaLineAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maStartPt, fScaleX, fScaleY );
    ImplScalePoint( maEndPt, fScaleX, fScaleY );
    ImplScaleLineInfo( maLineInfo, fScaleX, fScaleY );
}

bool MetaLineAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maLineInfo == ((MetaLineAction&)rMetaAction).maLineInfo ) &&
           ( maStartPt == ((MetaLineAction&)rMetaAction).maStartPt ) &&
           ( maEndPt == ((MetaLineAction&)rMetaAction).maEndPt );
}

void MetaLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 2);

    WritePair( rOStm, maStartPt );
    WritePair( rOStm, maEndPt );  // Version 1
    WriteLineInfo( rOStm, maLineInfo );            // Version 2
}

void MetaLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);

    // Version 1
    ReadPair( rIStm, maStartPt );
    ReadPair( rIStm, maEndPt );

    // Version 2
    if( aCompat.GetVersion() >= 2 )
    {
        ReadLineInfo( rIStm, maLineInfo );
    }
}

MetaRectAction::MetaRectAction() :
    MetaAction(META_RECT_ACTION)
{}

MetaRectAction::~MetaRectAction()
{}

MetaRectAction::MetaRectAction( const Rectangle& rRect ) :
    MetaAction  ( META_RECT_ACTION ),
    maRect      ( rRect )
{}

void MetaRectAction::Execute( OutputDevice* pOut )
{
    pOut->DrawRect( maRect );
}

MetaAction* MetaRectAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaRectAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaRectAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaRectAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

bool MetaRectAction::Compare( const MetaAction& rMetaAction ) const
{
    return maRect == ((MetaRectAction&)rMetaAction).maRect;
}

void MetaRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteRectangle( rOStm, maRect );
}

void MetaRectAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRectangle( rIStm, maRect );
}

MetaRoundRectAction::MetaRoundRectAction() :
    MetaAction  ( META_ROUNDRECT_ACTION ),
    mnHorzRound ( 0 ),
    mnVertRound ( 0 )
{}

MetaRoundRectAction::~MetaRoundRectAction()
{}

MetaRoundRectAction::MetaRoundRectAction( const Rectangle& rRect,
                                          sal_uInt32 nHorzRound, sal_uInt32 nVertRound ) :
    MetaAction  ( META_ROUNDRECT_ACTION ),
    maRect      ( rRect ),
    mnHorzRound ( nHorzRound ),
    mnVertRound ( nVertRound )
{}

void MetaRoundRectAction::Execute( OutputDevice* pOut )
{
    pOut->DrawRect( maRect, mnHorzRound, mnVertRound );
}

MetaAction* MetaRoundRectAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaRoundRectAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaRoundRectAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaRoundRectAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
    mnHorzRound = FRound( mnHorzRound * fabs(fScaleX) );
    mnVertRound = FRound( mnVertRound * fabs(fScaleY) );
}

bool MetaRoundRectAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaRoundRectAction&)rMetaAction).maRect ) &&
           ( mnHorzRound == ((MetaRoundRectAction&)rMetaAction).mnHorzRound ) &&
           ( mnVertRound == ((MetaRoundRectAction&)rMetaAction).mnVertRound );
}

void MetaRoundRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteRectangle( rOStm, maRect );
    rOStm.WriteUInt32( mnHorzRound ).WriteUInt32( mnVertRound );
}

void MetaRoundRectAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRectangle( rIStm, maRect ).ReadUInt32( mnHorzRound ).ReadUInt32( mnVertRound );
}

MetaEllipseAction::MetaEllipseAction() :
    MetaAction(META_ELLIPSE_ACTION)
{}

MetaEllipseAction::~MetaEllipseAction()
{}

MetaEllipseAction::MetaEllipseAction( const Rectangle& rRect ) :
    MetaAction  ( META_ELLIPSE_ACTION ),
    maRect      ( rRect )
{}

void MetaEllipseAction::Execute( OutputDevice* pOut )
{
    pOut->DrawEllipse( maRect );
}

MetaAction* MetaEllipseAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaEllipseAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaEllipseAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaEllipseAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

bool MetaEllipseAction::Compare( const MetaAction& rMetaAction ) const
{
    return maRect == ((MetaEllipseAction&)rMetaAction).maRect;
}

void MetaEllipseAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteRectangle( rOStm, maRect );
}

void MetaEllipseAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRectangle( rIStm, maRect );
}

MetaArcAction::MetaArcAction() :
    MetaAction(META_ARC_ACTION)
{}

MetaArcAction::~MetaArcAction()
{}

MetaArcAction::MetaArcAction( const Rectangle& rRect,
                              const Point& rStart, const Point& rEnd ) :
    MetaAction  ( META_ARC_ACTION ),
    maRect      ( rRect ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{}

void MetaArcAction::Execute( OutputDevice* pOut )
{
    pOut->DrawArc( maRect, maStartPt, maEndPt );
}

MetaAction* MetaArcAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaArcAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaArcAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move(  nHorzMove, nVertMove );
    maStartPt.Move(  nHorzMove, nVertMove );
    maEndPt.Move(  nHorzMove, nVertMove );
}

void MetaArcAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
    ImplScalePoint( maStartPt, fScaleX, fScaleY );
    ImplScalePoint( maEndPt, fScaleX, fScaleY );
}

bool MetaArcAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaArcAction&)rMetaAction).maRect ) &&
           ( maStartPt == ((MetaArcAction&)rMetaAction).maStartPt ) &&
           ( maEndPt == ((MetaArcAction&)rMetaAction).maEndPt );
}

void MetaArcAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteRectangle( rOStm, maRect );
    WritePair( rOStm, maStartPt );
    WritePair( rOStm, maEndPt );
}

void MetaArcAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRectangle( rIStm, maRect );
    ReadPair( rIStm, maStartPt );
    ReadPair( rIStm, maEndPt );
}

MetaPieAction::MetaPieAction() :
    MetaAction(META_PIE_ACTION)
{}

MetaPieAction::~MetaPieAction()
{}

MetaPieAction::MetaPieAction( const Rectangle& rRect,
                              const Point& rStart, const Point& rEnd ) :
    MetaAction  ( META_PIE_ACTION ),
    maRect      ( rRect ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{}

void MetaPieAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPie( maRect, maStartPt, maEndPt );
}

MetaAction* MetaPieAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPieAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaPieAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move(  nHorzMove, nVertMove );
    maStartPt.Move(  nHorzMove, nVertMove );
    maEndPt.Move(  nHorzMove, nVertMove );
}

void MetaPieAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
    ImplScalePoint( maStartPt, fScaleX, fScaleY );
    ImplScalePoint( maEndPt, fScaleX, fScaleY );
}

bool MetaPieAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaPieAction&)rMetaAction).maRect ) &&
           ( maStartPt == ((MetaPieAction&)rMetaAction).maStartPt ) &&
           ( maEndPt == ((MetaPieAction&)rMetaAction).maEndPt );
}

void MetaPieAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteRectangle( rOStm, maRect );
    WritePair( rOStm, maStartPt );
    WritePair( rOStm, maEndPt );
}

void MetaPieAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRectangle( rIStm, maRect );
    ReadPair( rIStm, maStartPt );
    ReadPair( rIStm, maEndPt );
}

MetaChordAction::MetaChordAction() :
    MetaAction(META_CHORD_ACTION)
{}

MetaChordAction::~MetaChordAction()
{}

MetaChordAction::MetaChordAction( const Rectangle& rRect,
                                  const Point& rStart, const Point& rEnd ) :
    MetaAction  ( META_CHORD_ACTION ),
    maRect      ( rRect ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{}

void MetaChordAction::Execute( OutputDevice* pOut )
{
    pOut->DrawChord( maRect, maStartPt, maEndPt );
}

MetaAction* MetaChordAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaChordAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaChordAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move(  nHorzMove, nVertMove );
    maStartPt.Move(  nHorzMove, nVertMove );
    maEndPt.Move(  nHorzMove, nVertMove );
}

void MetaChordAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
    ImplScalePoint( maStartPt, fScaleX, fScaleY );
    ImplScalePoint( maEndPt, fScaleX, fScaleY );
}

bool MetaChordAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaChordAction&)rMetaAction).maRect ) &&
           ( maStartPt == ((MetaChordAction&)rMetaAction).maStartPt ) &&
           ( maEndPt == ((MetaChordAction&)rMetaAction).maEndPt );
}

void MetaChordAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteRectangle( rOStm, maRect );
    WritePair( rOStm, maStartPt );
    WritePair( rOStm, maEndPt );
}

void MetaChordAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRectangle( rIStm, maRect );
    ReadPair( rIStm, maStartPt );
    ReadPair( rIStm, maEndPt );
}

MetaPolyLineAction::MetaPolyLineAction() :
    MetaAction(META_POLYLINE_ACTION)
{}

MetaPolyLineAction::~MetaPolyLineAction()
{}

MetaPolyLineAction::MetaPolyLineAction( const Polygon& rPoly ) :
    MetaAction  ( META_POLYLINE_ACTION ),
    maPoly      ( rPoly )
{}

MetaPolyLineAction::MetaPolyLineAction( const Polygon& rPoly, const LineInfo& rLineInfo ) :
    MetaAction  ( META_POLYLINE_ACTION ),
    maLineInfo  ( rLineInfo ),
    maPoly      ( rPoly )
{}

void MetaPolyLineAction::Execute( OutputDevice* pOut )
{
    if( maLineInfo.IsDefault() )
        pOut->DrawPolyLine( maPoly );
    else
        pOut->DrawPolyLine( maPoly, maLineInfo );
}

MetaAction* MetaPolyLineAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPolyLineAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaPolyLineAction::Move( long nHorzMove, long nVertMove )
{
    maPoly.Move( nHorzMove, nVertMove );
}

void MetaPolyLineAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoly( maPoly, fScaleX, fScaleY );
    ImplScaleLineInfo( maLineInfo, fScaleX, fScaleY );
}

bool MetaPolyLineAction::Compare( const MetaAction& rMetaAction ) const
{
    bool bIsEqual = true;
    if ( maLineInfo != ((MetaPolyLineAction&)rMetaAction).maLineInfo )
        bIsEqual = false;
    else
        bIsEqual = maPoly.IsEqual(((MetaPolyLineAction&)rMetaAction).maPoly );
    return bIsEqual;

}

void MetaPolyLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 3);

    Polygon aSimplePoly;
    maPoly.AdaptiveSubdivide( aSimplePoly );

    WritePolygon( rOStm, aSimplePoly );                               // Version 1
    WriteLineInfo( rOStm, maLineInfo );                                // Version 2

    sal_uInt8 bHasPolyFlags = maPoly.HasFlags();        // Version 3
    rOStm.WriteUChar( bHasPolyFlags );
    if ( bHasPolyFlags )
        maPoly.Write( rOStm );
}

void MetaPolyLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);

    // Version 1
    ReadPolygon( rIStm, maPoly );

    // Version 2
    if( aCompat.GetVersion() >= 2 )
        ReadLineInfo( rIStm, maLineInfo );
    if ( aCompat.GetVersion() >= 3 )
    {
        sal_uInt8 bHasPolyFlags(0);
        rIStm.ReadUChar( bHasPolyFlags );
        if ( bHasPolyFlags )
            maPoly.Read( rIStm );
    }
}

MetaPolygonAction::MetaPolygonAction() :
    MetaAction(META_POLYGON_ACTION)
{}

MetaPolygonAction::~MetaPolygonAction()
{}

MetaPolygonAction::MetaPolygonAction( const Polygon& rPoly ) :
    MetaAction  ( META_POLYGON_ACTION ),
    maPoly      ( rPoly )
{}

void MetaPolygonAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPolygon( maPoly );
}

MetaAction* MetaPolygonAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPolygonAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaPolygonAction::Move( long nHorzMove, long nVertMove )
{
    maPoly.Move( nHorzMove, nVertMove );
}

void MetaPolygonAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoly( maPoly, fScaleX, fScaleY );
}

bool MetaPolygonAction::Compare( const MetaAction& rMetaAction ) const
{
    return maPoly.IsEqual(((MetaPolygonAction&)rMetaAction).maPoly );
}

void MetaPolygonAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 2);

    Polygon aSimplePoly;                            // Version 1
    maPoly.AdaptiveSubdivide( aSimplePoly );
    WritePolygon( rOStm, aSimplePoly );

    sal_uInt8 bHasPolyFlags = maPoly.HasFlags();    // Version 2
    rOStm.WriteUChar( bHasPolyFlags );
    if ( bHasPolyFlags )
        maPoly.Write( rOStm );
}

void MetaPolygonAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);

    ReadPolygon( rIStm, maPoly );       // Version 1

    if( aCompat.GetVersion() >= 2 )     // Version 2
    {
        sal_uInt8 bHasPolyFlags(0);
        rIStm.ReadUChar( bHasPolyFlags );
        if ( bHasPolyFlags )
            maPoly.Read( rIStm );
    }
}

MetaPolyPolygonAction::MetaPolyPolygonAction() :
    MetaAction(META_POLYPOLYGON_ACTION)
{}

MetaPolyPolygonAction::~MetaPolyPolygonAction()
{}

MetaPolyPolygonAction::MetaPolyPolygonAction( const PolyPolygon& rPolyPoly ) :
    MetaAction  ( META_POLYPOLYGON_ACTION ),
    maPolyPoly  ( rPolyPoly )
{}

void MetaPolyPolygonAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPolyPolygon( maPolyPoly );
}

MetaAction* MetaPolyPolygonAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPolyPolygonAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaPolyPolygonAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaPolyPolygonAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

bool MetaPolyPolygonAction::Compare( const MetaAction& rMetaAction ) const
{
    return maPolyPoly.IsEqual(((MetaPolyPolygonAction&)rMetaAction).maPolyPoly );
}

void MetaPolyPolygonAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 2);

    sal_uInt16 nNumberOfComplexPolygons = 0;
    sal_uInt16 i, nPolyCount = maPolyPoly.Count();

    Polygon aSimplePoly;                                // Version 1
    rOStm.WriteUInt16( nPolyCount );
    for ( i = 0; i < nPolyCount; i++ )
    {
        const Polygon& rPoly = maPolyPoly.GetObject( i );
        if ( rPoly.HasFlags() )
            nNumberOfComplexPolygons++;
        rPoly.AdaptiveSubdivide( aSimplePoly );
        WritePolygon( rOStm, aSimplePoly );
    }

    rOStm.WriteUInt16( nNumberOfComplexPolygons );                  // Version 2
    for ( i = 0; nNumberOfComplexPolygons && ( i < nPolyCount ); i++ )
    {
        const Polygon& rPoly = maPolyPoly.GetObject( i );
        if ( rPoly.HasFlags() )
        {
            rOStm.WriteUInt16( i );
            rPoly.Write( rOStm );

            nNumberOfComplexPolygons--;
        }
    }
}

void MetaPolyPolygonAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPolyPolygon( rIStm, maPolyPoly );                // Version 1

    if ( aCompat.GetVersion() >= 2 )    // Version 2
    {
        sal_uInt16 nNumberOfComplexPolygons(0);
        rIStm.ReadUInt16( nNumberOfComplexPolygons );
        for (sal_uInt16 i = 0; i < nNumberOfComplexPolygons; ++i)
        {
            sal_uInt16 nIndex(0);
            rIStm.ReadUInt16( nIndex );
            Polygon aPoly;
            aPoly.Read( rIStm );
            if (nIndex >= maPolyPoly.Count())
            {
                SAL_WARN("vcl.gdi", "svm contains polygon index " << nIndex
                         << " outside possible range " << maPolyPoly.Count());
                continue;
            }
            maPolyPoly.Replace( aPoly, nIndex );
        }
    }
}

MetaTextAction::MetaTextAction() :
    MetaAction  ( META_TEXT_ACTION ),
    mnIndex     ( 0 ),
    mnLen       ( 0 )
{}

MetaTextAction::~MetaTextAction()
{}

MetaTextAction::MetaTextAction( const Point& rPt, const OUString& rStr,
                                sal_Int32 nIndex, sal_Int32 nLen ) :
    MetaAction  ( META_TEXT_ACTION ),
    maPt        ( rPt ),
    maStr       ( rStr ),
    mnIndex     ( nIndex ),
    mnLen       ( nLen )
{}

void MetaTextAction::Execute( OutputDevice* pOut )
{
    pOut->DrawText( maPt, maStr, mnIndex, mnLen );
}

MetaAction* MetaTextAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaTextAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaTextAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

bool MetaTextAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPt == ((MetaTextAction&)rMetaAction).maPt ) &&
           ( maStr == ((MetaTextAction&)rMetaAction).maStr ) &&
           ( mnIndex == ((MetaTextAction&)rMetaAction).mnIndex ) &&
           ( mnLen == ((MetaTextAction&)rMetaAction).mnLen );
}

void MetaTextAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 2);
    WritePair( rOStm, maPt );
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm.WriteUInt16(mnIndex);
    rOStm.WriteUInt16(mnLen);

    write_uInt16_lenPrefixed_uInt16s_FromOUString(rOStm, maStr); // version 2
}

void MetaTextAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPair( rIStm, maPt );
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    sal_uInt16 nTmpIndex(0);
    rIStm.ReadUInt16(nTmpIndex);
    mnIndex = nTmpIndex;
    sal_uInt16 nTmpLen(0);
    rIStm.ReadUInt16(nTmpLen);
    mnLen = nTmpLen;

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
        maStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(rIStm);
}

MetaTextArrayAction::MetaTextArrayAction() :
    MetaAction  ( META_TEXTARRAY_ACTION ),
    mpDXAry     ( NULL ),
    mnIndex     ( 0 ),
    mnLen       ( 0 )
{}

MetaTextArrayAction::MetaTextArrayAction( const MetaTextArrayAction& rAction ) :
    MetaAction  ( META_TEXTARRAY_ACTION ),
    maStartPt   ( rAction.maStartPt ),
    maStr       ( rAction.maStr ),
    mnIndex     ( rAction.mnIndex ),
    mnLen       ( rAction.mnLen )
{
    if( rAction.mpDXAry )
    {
        const sal_Int32 nAryLen = mnLen;

        mpDXAry = new sal_Int32[ nAryLen ];
        memcpy( mpDXAry, rAction.mpDXAry, nAryLen * sizeof( sal_Int32 ) );
    }
    else
        mpDXAry = NULL;
}

MetaTextArrayAction::MetaTextArrayAction( const Point& rStartPt,
                                          const OUString& rStr,
                                          const sal_Int32* pDXAry,
                                          sal_Int32 nIndex,
                                          sal_Int32 nLen ) :
    MetaAction  ( META_TEXTARRAY_ACTION ),
    maStartPt   ( rStartPt ),
    maStr       ( rStr ),
    mnIndex     ( nIndex ),
    mnLen       ( nLen )
{
    const sal_Int32 nAryLen = pDXAry ? mnLen : 0;

    if( nAryLen )
    {
        mpDXAry = new sal_Int32[ nAryLen ];
        memcpy( mpDXAry, pDXAry, nAryLen * sizeof( sal_Int32 ) );
    }
    else
        mpDXAry = NULL;
}

MetaTextArrayAction::~MetaTextArrayAction()
{
    delete[] mpDXAry;
}

void MetaTextArrayAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTextArray( maStartPt, maStr, mpDXAry, mnIndex, mnLen );
}

MetaAction* MetaTextArrayAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextArrayAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaTextArrayAction::Move( long nHorzMove, long nVertMove )
{
    maStartPt.Move( nHorzMove, nVertMove );
}

void MetaTextArrayAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maStartPt, fScaleX, fScaleY );

    if ( mpDXAry && mnLen )
    {
        for ( sal_uInt16 i = 0, nCount = mnLen; i < nCount; i++ )
            mpDXAry[ i ] = FRound( mpDXAry[ i ] * fabs(fScaleX) );
    }
}

bool MetaTextArrayAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maStartPt == ((MetaTextArrayAction&)rMetaAction).maStartPt ) &&
           ( maStr == ((MetaTextArrayAction&)rMetaAction).maStr ) &&
           ( mnIndex == ((MetaTextArrayAction&)rMetaAction).mnIndex ) &&
           ( mnLen == ((MetaTextArrayAction&)rMetaAction).mnLen ) &&
           ( memcmp( mpDXAry, ((MetaTextArrayAction&)rMetaAction).mpDXAry, mnLen ) == 0 );
}

void MetaTextArrayAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    const sal_Int32 nAryLen = mpDXAry ? mnLen : 0;

    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 2);
    WritePair( rOStm, maStartPt );
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm.WriteUInt16(mnIndex);
    rOStm.WriteUInt16(mnLen);
    rOStm.WriteInt32(nAryLen);

    for (sal_Int32 i = 0; i < nAryLen; ++i)
        rOStm.WriteInt32( mpDXAry[ i ] );

    write_uInt16_lenPrefixed_uInt16s_FromOUString(rOStm, maStr); // version 2
}

void MetaTextArrayAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    delete[] mpDXAry;

    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPair( rIStm, maStartPt );
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    sal_uInt16 nTmpIndex(0);
    rIStm.ReadUInt16(nTmpIndex);
    mnIndex = nTmpIndex;
    sal_uInt16 nTmpLen(0);
    rIStm.ReadUInt16(nTmpLen);
    mnLen = nTmpLen;
    sal_Int32 nAryLen(0);
    rIStm.ReadInt32(nAryLen);

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
                sal_Int32 i;
                for( i = 0; i < nAryLen; i++ )
                    rIStm.ReadInt32( mpDXAry[ i ] );

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
        maStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(rIStm);

        if ( mnIndex + mnLen > maStr.getLength() )
        {
            mnIndex = 0;
            delete[] mpDXAry, mpDXAry = NULL;
        }
    }
}

MetaStretchTextAction::MetaStretchTextAction() :
    MetaAction  ( META_STRETCHTEXT_ACTION ),
    mnWidth     ( 0 ),
    mnIndex     ( 0 ),
    mnLen       ( 0 )
{}

MetaStretchTextAction::~MetaStretchTextAction()
{}

MetaStretchTextAction::MetaStretchTextAction( const Point& rPt, sal_uInt32 nWidth,
                                              const OUString& rStr,
                                              sal_Int32 nIndex, sal_Int32 nLen ) :
    MetaAction  ( META_STRETCHTEXT_ACTION ),
    maPt        ( rPt ),
    maStr       ( rStr ),
    mnWidth     ( nWidth ),
    mnIndex     ( nIndex ),
    mnLen       ( nLen )
{}

void MetaStretchTextAction::Execute( OutputDevice* pOut )
{
    pOut->DrawStretchText( maPt, mnWidth, maStr, mnIndex, mnLen );
}

MetaAction* MetaStretchTextAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaStretchTextAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaStretchTextAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaStretchTextAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
    mnWidth = (sal_uLong)FRound( mnWidth * fabs(fScaleX) );
}

bool MetaStretchTextAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPt == ((MetaStretchTextAction&)rMetaAction).maPt ) &&
           ( maStr == ((MetaStretchTextAction&)rMetaAction).maStr ) &&
           ( mnWidth == ((MetaStretchTextAction&)rMetaAction).mnWidth ) &&
           ( mnIndex == ((MetaStretchTextAction&)rMetaAction).mnIndex ) &&
           ( mnLen == ((MetaStretchTextAction&)rMetaAction).mnLen );
}

void MetaStretchTextAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 2);
    WritePair( rOStm, maPt );
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm.WriteUInt32( mnWidth );
    rOStm.WriteUInt16( mnIndex );
    rOStm.WriteUInt16( mnLen );

    write_uInt16_lenPrefixed_uInt16s_FromOUString(rOStm, maStr); // version 2
}

void MetaStretchTextAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPair( rIStm, maPt );
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    rIStm.ReadUInt32( mnWidth );
    sal_uInt16 nTmpIndex(0);
    rIStm.ReadUInt16(nTmpIndex);
    mnIndex = nTmpIndex;
    sal_uInt16 nTmpLen(0);
    rIStm.ReadUInt16(nTmpLen);
    mnLen = nTmpLen;

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
        maStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(rIStm);
}

MetaTextRectAction::MetaTextRectAction() :
    MetaAction  ( META_TEXTRECT_ACTION ),
    mnStyle     ( 0 )
{}

MetaTextRectAction::~MetaTextRectAction()
{}

MetaTextRectAction::MetaTextRectAction( const Rectangle& rRect,
                                        const OUString& rStr, sal_uInt16 nStyle ) :
    MetaAction  ( META_TEXTRECT_ACTION ),
    maRect      ( rRect ),
    maStr       ( rStr ),
    mnStyle     ( nStyle )
{}

void MetaTextRectAction::Execute( OutputDevice* pOut )
{
    pOut->DrawText( maRect, maStr, mnStyle );
}

MetaAction* MetaTextRectAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextRectAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaTextRectAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaTextRectAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

bool MetaTextRectAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaTextRectAction&)rMetaAction).maRect ) &&
           ( maStr == ((MetaTextRectAction&)rMetaAction).maStr ) &&
           ( mnStyle == ((MetaTextRectAction&)rMetaAction).mnStyle );
}

void MetaTextRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 2);
    WriteRectangle( rOStm, maRect );
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm.WriteUInt16( mnStyle );

    write_uInt16_lenPrefixed_uInt16s_FromOUString(rOStm, maStr); // version 2
}

void MetaTextRectAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRectangle( rIStm, maRect );
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    rIStm  .ReadUInt16( mnStyle );

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
        maStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(rIStm);
}

MetaTextLineAction::MetaTextLineAction() :
    MetaAction  ( META_TEXTLINE_ACTION ),
    mnWidth     ( 0 ),
    meStrikeout ( STRIKEOUT_NONE ),
    meUnderline ( UNDERLINE_NONE ),
    meOverline  ( UNDERLINE_NONE )
{}

MetaTextLineAction::~MetaTextLineAction()
{}

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
{}

void MetaTextLineAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTextLine( maPos, mnWidth, meStrikeout, meUnderline, meOverline );
}

MetaAction* MetaTextLineAction::Clone()
{
    MetaAction* pClone = (MetaAction*)new MetaTextLineAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaTextLineAction::Move( long nHorzMove, long nVertMove )
{
    maPos.Move( nHorzMove, nVertMove );
}

void MetaTextLineAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPos, fScaleX, fScaleY );
    mnWidth = FRound( mnWidth * fabs(fScaleX) );
}

bool MetaTextLineAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPos == ((MetaTextLineAction&)rMetaAction).maPos ) &&
           ( mnWidth == ((MetaTextLineAction&)rMetaAction).mnWidth ) &&
           ( meStrikeout == ((MetaTextLineAction&)rMetaAction).meStrikeout ) &&
           ( meUnderline == ((MetaTextLineAction&)rMetaAction).meUnderline ) &&
           ( meOverline  == ((MetaTextLineAction&)rMetaAction).meOverline );
}

void MetaTextLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 2);

    //#fdo39428 SvStream no longer supports operator<<(long)
    WritePair( rOStm, maPos );
    rOStm.WriteInt32( sal::static_int_cast<sal_Int32>(mnWidth) );
    rOStm.WriteUInt32( static_cast<sal_uInt32>(meStrikeout) );
    rOStm.WriteUInt32( static_cast<sal_uInt32>(meUnderline) );
    // new in version 2
    rOStm.WriteUInt32( static_cast<sal_uInt32>(meOverline) );
}

void MetaTextLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);

    //#fdo39428 SvStream no longer supports operator>>(long&)
    sal_Int32 nTempWidth(0);
    ReadPair( rIStm, maPos );
    rIStm.ReadInt32( nTempWidth );
    mnWidth = nTempWidth;
    sal_uInt32 nTempStrikeout(0);
    rIStm.ReadUInt32( nTempStrikeout );
    meStrikeout = (FontStrikeout)nTempStrikeout;
    sal_uInt32 nTempUnderline(0);
    rIStm.ReadUInt32( nTempUnderline );
    meUnderline = (FontUnderline)nTempUnderline;
    if ( aCompat.GetVersion() >= 2 ) {
        sal_uInt32 nTempUnderline2(0);
        rIStm.ReadUInt32(nTempUnderline2);
        meUnderline = (FontUnderline)nTempUnderline2;
    }
}

MetaBmpAction::MetaBmpAction() :
    MetaAction(META_BMP_ACTION)
{}

MetaBmpAction::~MetaBmpAction()
{}

MetaBmpAction::MetaBmpAction( const Point& rPt, const Bitmap& rBmp ) :
    MetaAction  ( META_BMP_ACTION ),
    maBmp       ( rBmp ),
    maPt        ( rPt )
{}

void MetaBmpAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmap( maPt, maBmp );
}

MetaAction* MetaBmpAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaBmpAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaBmpAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

bool MetaBmpAction::Compare( const MetaAction& rMetaAction ) const
{
    return maBmp.IsEqual(((MetaBmpAction&)rMetaAction).maBmp ) &&
           ( maPt == ((MetaBmpAction&)rMetaAction).maPt );
}

void MetaBmpAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maPt );
    }
}

void MetaBmpAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maPt );
}

MetaBmpScaleAction::MetaBmpScaleAction() :
    MetaAction(META_BMPSCALE_ACTION)
{}

MetaBmpScaleAction::~MetaBmpScaleAction()
{}

MetaBmpScaleAction::MetaBmpScaleAction( const Point& rPt, const Size& rSz,
                                        const Bitmap& rBmp ) :
    MetaAction  ( META_BMPSCALE_ACTION ),
    maBmp       ( rBmp ),
    maPt        ( rPt ),
    maSz        ( rSz )
{}

void MetaBmpScaleAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmap( maPt, maSz, maBmp );
}

MetaAction* MetaBmpScaleAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpScaleAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaBmpScaleAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaBmpScaleAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPt, maSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPt = aRectangle.TopLeft();
    maSz = aRectangle.GetSize();
}

bool MetaBmpScaleAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaBmpScaleAction&)rMetaAction).maBmp )) &&
           ( maPt == ((MetaBmpScaleAction&)rMetaAction).maPt ) &&
           ( maSz == ((MetaBmpScaleAction&)rMetaAction).maSz );
}

void MetaBmpScaleAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maPt );
        WritePair( rOStm, maSz );
    }
}

void MetaBmpScaleAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maPt );
    ReadPair( rIStm, maSz );
}

MetaBmpScalePartAction::MetaBmpScalePartAction() :
    MetaAction(META_BMPSCALEPART_ACTION)
{}

MetaBmpScalePartAction::~MetaBmpScalePartAction()
{}

MetaBmpScalePartAction::MetaBmpScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                const Point& rSrcPt, const Size& rSrcSz,
                                                const Bitmap& rBmp ) :
    MetaAction  ( META_BMPSCALEPART_ACTION ),
    maBmp       ( rBmp ),
    maDstPt     ( rDstPt ),
    maDstSz     ( rDstSz ),
    maSrcPt     ( rSrcPt ),
    maSrcSz     ( rSrcSz )
{}

void MetaBmpScalePartAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmap( maDstPt, maDstSz, maSrcPt, maSrcSz, maBmp );
}

MetaAction* MetaBmpScalePartAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpScalePartAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaBmpScalePartAction::Move( long nHorzMove, long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

void MetaBmpScalePartAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maDstPt, maDstSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
}

bool MetaBmpScalePartAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaBmpScalePartAction&)rMetaAction).maBmp )) &&
           ( maDstPt == ((MetaBmpScalePartAction&)rMetaAction).maDstPt ) &&
           ( maDstSz == ((MetaBmpScalePartAction&)rMetaAction).maDstSz ) &&
           ( maSrcPt == ((MetaBmpScalePartAction&)rMetaAction).maSrcPt ) &&
           ( maSrcSz == ((MetaBmpScalePartAction&)rMetaAction).maSrcSz );
}

void MetaBmpScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maDstPt );
        WritePair( rOStm, maDstSz );
        WritePair( rOStm, maSrcPt );
        WritePair( rOStm, maSrcSz );
    }
}

void MetaBmpScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maDstPt );
    ReadPair( rIStm, maDstSz );
    ReadPair( rIStm, maSrcPt );
    ReadPair( rIStm, maSrcSz );
}

MetaBmpExAction::MetaBmpExAction() :
    MetaAction(META_BMPEX_ACTION)
{}

MetaBmpExAction::~MetaBmpExAction()
{}

MetaBmpExAction::MetaBmpExAction( const Point& rPt, const BitmapEx& rBmpEx ) :
    MetaAction  ( META_BMPEX_ACTION ),
    maBmpEx     ( rBmpEx ),
    maPt        ( rPt )
{}

void MetaBmpExAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmapEx( maPt, maBmpEx );
}

MetaAction* MetaBmpExAction::Clone()
{
    MetaBmpExAction* pClone = new MetaBmpExAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaBmpExAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaBmpExAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

bool MetaBmpExAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmpEx.IsEqual(((MetaBmpExAction&)rMetaAction).maBmpEx )) &&
           ( maPt == ((MetaBmpExAction&)rMetaAction).maPt );
}

void MetaBmpExAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
        WriteDIBBitmapEx(maBmpEx, rOStm);
        WritePair( rOStm, maPt );
    }
}

void MetaBmpExAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadDIBBitmapEx(maBmpEx, rIStm);
    ReadPair( rIStm, maPt );
}

MetaBmpExScaleAction::MetaBmpExScaleAction() :
    MetaAction(META_BMPEXSCALE_ACTION)
{}

MetaBmpExScaleAction::~MetaBmpExScaleAction()
{}

MetaBmpExScaleAction::MetaBmpExScaleAction( const Point& rPt, const Size& rSz,
                                            const BitmapEx& rBmpEx ) :
    MetaAction  ( META_BMPEXSCALE_ACTION ),
    maBmpEx     ( rBmpEx ),
    maPt        ( rPt ),
    maSz        ( rSz )
{}

void MetaBmpExScaleAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmapEx( maPt, maSz, maBmpEx );
}

MetaAction* MetaBmpExScaleAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpExScaleAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaBmpExScaleAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaBmpExScaleAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPt, maSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPt = aRectangle.TopLeft();
    maSz = aRectangle.GetSize();
}

bool MetaBmpExScaleAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmpEx.IsEqual(((MetaBmpExScaleAction&)rMetaAction).maBmpEx )) &&
           ( maPt == ((MetaBmpExScaleAction&)rMetaAction).maPt ) &&
           ( maSz == ((MetaBmpExScaleAction&)rMetaAction).maSz );
}

void MetaBmpExScaleAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
        WriteDIBBitmapEx(maBmpEx, rOStm);
        WritePair( rOStm, maPt );
        WritePair( rOStm, maSz );
    }
}

void MetaBmpExScaleAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadDIBBitmapEx(maBmpEx, rIStm);
    ReadPair( rIStm, maPt );
    ReadPair( rIStm, maSz );
}

MetaBmpExScalePartAction::MetaBmpExScalePartAction() :
    MetaAction(META_BMPEXSCALEPART_ACTION)
{}

MetaBmpExScalePartAction::~MetaBmpExScalePartAction()
{}

MetaBmpExScalePartAction::MetaBmpExScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                    const Point& rSrcPt, const Size& rSrcSz,
                                                    const BitmapEx& rBmpEx ) :
    MetaAction  ( META_BMPEXSCALEPART_ACTION ),
    maBmpEx     ( rBmpEx ),
    maDstPt     ( rDstPt ),
    maDstSz     ( rDstSz ),
    maSrcPt     ( rSrcPt ),
    maSrcSz     ( rSrcSz )
{}

void MetaBmpExScalePartAction::Execute( OutputDevice* pOut )
{
    pOut->DrawBitmapEx( maDstPt, maDstSz, maSrcPt, maSrcSz, maBmpEx );
}

MetaAction* MetaBmpExScalePartAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaBmpExScalePartAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaBmpExScalePartAction::Move( long nHorzMove, long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

void MetaBmpExScalePartAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maDstPt, maDstSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
}

bool MetaBmpExScalePartAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmpEx.IsEqual(((MetaBmpExScalePartAction&)rMetaAction).maBmpEx )) &&
           ( maDstPt == ((MetaBmpExScalePartAction&)rMetaAction).maDstPt ) &&
           ( maDstSz == ((MetaBmpExScalePartAction&)rMetaAction).maDstSz ) &&
           ( maSrcPt == ((MetaBmpExScalePartAction&)rMetaAction).maSrcPt ) &&
           ( maSrcSz == ((MetaBmpExScalePartAction&)rMetaAction).maSrcSz );
}

void MetaBmpExScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
        WriteDIBBitmapEx(maBmpEx, rOStm);
        WritePair( rOStm, maDstPt );
        WritePair( rOStm, maDstSz );
        WritePair( rOStm, maSrcPt );
        WritePair( rOStm, maSrcSz );
    }
}

void MetaBmpExScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadDIBBitmapEx(maBmpEx, rIStm);
    ReadPair( rIStm, maDstPt );
    ReadPair( rIStm, maDstSz );
    ReadPair( rIStm, maSrcPt );
    ReadPair( rIStm, maSrcSz );
}

MetaMaskAction::MetaMaskAction() :
    MetaAction(META_MASK_ACTION)
{}

MetaMaskAction::~MetaMaskAction()
{}

MetaMaskAction::MetaMaskAction( const Point& rPt,
                                const Bitmap& rBmp,
                                const Color& rColor ) :
    MetaAction  ( META_MASK_ACTION ),
    maBmp       ( rBmp ),
    maColor     ( rColor ),
    maPt        ( rPt )
{}

void MetaMaskAction::Execute( OutputDevice* pOut )
{
    pOut->DrawMask( maPt, maBmp, maColor );
}

MetaAction* MetaMaskAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMaskAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaMaskAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaMaskAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

bool MetaMaskAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaMaskAction&)rMetaAction).maBmp )) &&
           ( maColor == ((MetaMaskAction&)rMetaAction).maColor ) &&
           ( maPt == ((MetaMaskAction&)rMetaAction).maPt );
}

void MetaMaskAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maPt );
    }
}

void MetaMaskAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maPt );
}

MetaMaskScaleAction::MetaMaskScaleAction() :
    MetaAction(META_MASKSCALE_ACTION)
{}

MetaMaskScaleAction::~MetaMaskScaleAction()
{}

MetaMaskScaleAction::MetaMaskScaleAction( const Point& rPt, const Size& rSz,
                                          const Bitmap& rBmp,
                                          const Color& rColor ) :
    MetaAction  ( META_MASKSCALE_ACTION ),
    maBmp       ( rBmp ),
    maColor     ( rColor ),
    maPt        ( rPt ),
    maSz        ( rSz )
{}

void MetaMaskScaleAction::Execute( OutputDevice* pOut )
{
    pOut->DrawMask( maPt, maSz, maBmp, maColor );
}

MetaAction* MetaMaskScaleAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMaskScaleAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaMaskScaleAction::Move( long nHorzMove, long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaMaskScaleAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPt, maSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPt = aRectangle.TopLeft();
    maSz = aRectangle.GetSize();
}

bool MetaMaskScaleAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaMaskScaleAction&)rMetaAction).maBmp )) &&
           ( maColor == ((MetaMaskScaleAction&)rMetaAction).maColor ) &&
           ( maPt == ((MetaMaskScaleAction&)rMetaAction).maPt ) &&
           ( maSz == ((MetaMaskScaleAction&)rMetaAction).maSz );
}

void MetaMaskScaleAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maPt );
        WritePair( rOStm, maSz );
    }
}

void MetaMaskScaleAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maPt );
    ReadPair( rIStm, maSz );
}

MetaMaskScalePartAction::MetaMaskScalePartAction() :
    MetaAction(META_MASKSCALEPART_ACTION)
{}

MetaMaskScalePartAction::~MetaMaskScalePartAction()
{}

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
{}

void MetaMaskScalePartAction::Execute( OutputDevice* pOut )
{
    pOut->DrawMask( maDstPt, maDstSz, maSrcPt, maSrcSz, maBmp, maColor, META_MASKSCALE_ACTION );
}

MetaAction* MetaMaskScalePartAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMaskScalePartAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaMaskScalePartAction::Move( long nHorzMove, long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

void MetaMaskScalePartAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maDstPt, maDstSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
}

bool MetaMaskScalePartAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maBmp.IsEqual(((MetaMaskScalePartAction&)rMetaAction).maBmp )) &&
           ( maColor == ((MetaMaskScalePartAction&)rMetaAction).maColor ) &&
           ( maDstPt == ((MetaMaskScalePartAction&)rMetaAction).maDstPt ) &&
           ( maDstSz == ((MetaMaskScalePartAction&)rMetaAction).maDstSz ) &&
           ( maSrcPt == ((MetaMaskScalePartAction&)rMetaAction).maSrcPt ) &&
           ( maSrcSz == ((MetaMaskScalePartAction&)rMetaAction).maSrcSz );
}

void MetaMaskScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        maColor.Write( rOStm, true );
        WritePair( rOStm, maDstPt );
        WritePair( rOStm, maDstSz );
        WritePair( rOStm, maSrcPt );
        WritePair( rOStm, maSrcSz );
    }
}

void MetaMaskScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadDIB(maBmp, rIStm, true);
    maColor.Read( rIStm, true );
    ReadPair( rIStm, maDstPt );
    ReadPair( rIStm, maDstSz );
    ReadPair( rIStm, maSrcPt );
    ReadPair( rIStm, maSrcSz );
}

MetaGradientAction::MetaGradientAction() :
    MetaAction(META_GRADIENT_ACTION)
{}

MetaGradientAction::~MetaGradientAction()
{}

MetaGradientAction::MetaGradientAction( const Rectangle& rRect, const Gradient& rGradient ) :
    MetaAction  ( META_GRADIENT_ACTION ),
    maRect      ( rRect ),
    maGradient  ( rGradient )
{}

void MetaGradientAction::Execute( OutputDevice* pOut )
{
    pOut->DrawGradient( maRect, maGradient );
}

MetaAction* MetaGradientAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaGradientAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaGradientAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaGradientAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

bool MetaGradientAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaGradientAction&)rMetaAction).maRect ) &&
           ( maGradient == ((MetaGradientAction&)rMetaAction).maGradient );
}

void MetaGradientAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteRectangle( rOStm, maRect );
    WriteGradient( rOStm, maGradient );
}

void MetaGradientAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRectangle( rIStm, maRect );
    ReadGradient( rIStm, maGradient );
}

MetaGradientExAction::MetaGradientExAction() :
    MetaAction  ( META_GRADIENTEX_ACTION )
{}

MetaGradientExAction::MetaGradientExAction( const PolyPolygon& rPolyPoly, const Gradient& rGradient ) :
    MetaAction  ( META_GRADIENTEX_ACTION ),
    maPolyPoly  ( rPolyPoly ),
    maGradient  ( rGradient )
{}

MetaGradientExAction::~MetaGradientExAction()
{}

void MetaGradientExAction::Execute( OutputDevice* pOut )
{
    if( pOut->GetConnectMetaFile() )
    {
        Duplicate();
        pOut->GetConnectMetaFile()->AddAction( this );
    }
}

MetaAction* MetaGradientExAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaGradientExAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaGradientExAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaGradientExAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

bool MetaGradientExAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPolyPoly == ((MetaGradientExAction&)rMetaAction).maPolyPoly ) &&
           ( maGradient == ((MetaGradientExAction&)rMetaAction).maGradient );
}

void MetaGradientExAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);

    // #i105373# see comment at MetaTransparentAction::Write
    PolyPolygon aNoCurvePolyPolygon;
    maPolyPoly.AdaptiveSubdivide(aNoCurvePolyPolygon);

    WritePolyPolygon( rOStm, aNoCurvePolyPolygon );
    WriteGradient( rOStm, maGradient );
}

void MetaGradientExAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPolyPolygon( rIStm, maPolyPoly );
    ReadGradient( rIStm, maGradient );
}

MetaHatchAction::MetaHatchAction() :
    MetaAction(META_HATCH_ACTION)
{}

MetaHatchAction::~MetaHatchAction()
{}

MetaHatchAction::MetaHatchAction( const PolyPolygon& rPolyPoly, const Hatch& rHatch ) :
    MetaAction  ( META_HATCH_ACTION ),
    maPolyPoly  ( rPolyPoly ),
    maHatch     ( rHatch )
{}

void MetaHatchAction::Execute( OutputDevice* pOut )
{
    pOut->DrawHatch( maPolyPoly, maHatch );
}

MetaAction* MetaHatchAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaHatchAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaHatchAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaHatchAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

bool MetaHatchAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPolyPoly == ((MetaHatchAction&)rMetaAction).maPolyPoly ) &&
           ( maHatch == ((MetaHatchAction&)rMetaAction).maHatch );
}

void MetaHatchAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);

    // #i105373# see comment at MetaTransparentAction::Write
    PolyPolygon aNoCurvePolyPolygon;
    maPolyPoly.AdaptiveSubdivide(aNoCurvePolyPolygon);

    WritePolyPolygon( rOStm, aNoCurvePolyPolygon );
    WriteHatch( rOStm, maHatch );
}

void MetaHatchAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPolyPolygon( rIStm, maPolyPoly );
    ReadHatch( rIStm, maHatch );
}

MetaWallpaperAction::MetaWallpaperAction() :
    MetaAction(META_WALLPAPER_ACTION)
{}

MetaWallpaperAction::~MetaWallpaperAction()
{}

MetaWallpaperAction::MetaWallpaperAction( const Rectangle& rRect,
                                          const Wallpaper& rPaper ) :
    MetaAction  ( META_WALLPAPER_ACTION ),
    maRect      ( rRect ),
    maWallpaper ( rPaper )
{}

void MetaWallpaperAction::Execute( OutputDevice* pOut )
{
    pOut->DrawWallpaper( maRect, maWallpaper );
}

MetaAction* MetaWallpaperAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaWallpaperAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaWallpaperAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaWallpaperAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

bool MetaWallpaperAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRect == ((MetaWallpaperAction&)rMetaAction).maRect ) &&
           ( maWallpaper == ((MetaWallpaperAction&)rMetaAction).maWallpaper );
}

void MetaWallpaperAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);

    WriteWallpaper( rOStm, maWallpaper );
}

void MetaWallpaperAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadWallpaper( rIStm, maWallpaper );
}

MetaClipRegionAction::MetaClipRegionAction() :
    MetaAction  ( META_CLIPREGION_ACTION ),
    mbClip      ( false )
{}

MetaClipRegionAction::~MetaClipRegionAction()
{}

MetaClipRegionAction::MetaClipRegionAction( const Region& rRegion, bool bClip ) :
    MetaAction  ( META_CLIPREGION_ACTION ),
    maRegion    ( rRegion ),
    mbClip      ( bClip )
{}

void MetaClipRegionAction::Execute( OutputDevice* pOut )
{
    if( mbClip )
        pOut->SetClipRegion( maRegion );
    else
        pOut->SetClipRegion();
}

MetaAction* MetaClipRegionAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaClipRegionAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaClipRegionAction::Move( long nHorzMove, long nVertMove )
{
    maRegion.Move( nHorzMove, nVertMove );
}

void MetaClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    maRegion.Scale( fScaleX, fScaleY );
}

bool MetaClipRegionAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRegion == ((MetaClipRegionAction&)rMetaAction).maRegion ) &&
           ( mbClip == ((MetaClipRegionAction&)rMetaAction).mbClip );
}

void MetaClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);

    WriteRegion( rOStm, maRegion );
    rOStm.WriteUChar( mbClip );
}

void MetaClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRegion( rIStm, maRegion );
    rIStm.ReadCharAsBool( mbClip );
}

MetaISectRectClipRegionAction::MetaISectRectClipRegionAction() :
    MetaAction(META_ISECTRECTCLIPREGION_ACTION)
{}

MetaISectRectClipRegionAction::~MetaISectRectClipRegionAction()
{}

MetaISectRectClipRegionAction::MetaISectRectClipRegionAction( const Rectangle& rRect ) :
    MetaAction  ( META_ISECTRECTCLIPREGION_ACTION ),
    maRect      ( rRect )
{}

void MetaISectRectClipRegionAction::Execute( OutputDevice* pOut )
{
    pOut->IntersectClipRegion( maRect );
}

MetaAction* MetaISectRectClipRegionAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaISectRectClipRegionAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaISectRectClipRegionAction::Move( long nHorzMove, long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaISectRectClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

bool MetaISectRectClipRegionAction::Compare( const MetaAction& rMetaAction ) const
{
    return maRect == ((MetaISectRectClipRegionAction&)rMetaAction).maRect;
}

void MetaISectRectClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteRectangle( rOStm, maRect );
}

void MetaISectRectClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRectangle( rIStm, maRect );
}

MetaISectRegionClipRegionAction::MetaISectRegionClipRegionAction() :
    MetaAction(META_ISECTREGIONCLIPREGION_ACTION)
{}

MetaISectRegionClipRegionAction::~MetaISectRegionClipRegionAction()
{}

MetaISectRegionClipRegionAction::MetaISectRegionClipRegionAction( const Region& rRegion ) :
    MetaAction  ( META_ISECTREGIONCLIPREGION_ACTION ),
    maRegion    ( rRegion )
{
}

void MetaISectRegionClipRegionAction::Execute( OutputDevice* pOut )
{
    pOut->IntersectClipRegion( maRegion );
}

MetaAction* MetaISectRegionClipRegionAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaISectRegionClipRegionAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaISectRegionClipRegionAction::Move( long nHorzMove, long nVertMove )
{
    maRegion.Move( nHorzMove, nVertMove );
}

void MetaISectRegionClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    maRegion.Scale( fScaleX, fScaleY );
}

bool MetaISectRegionClipRegionAction::Compare( const MetaAction& rMetaAction ) const
{
    return maRegion == ((MetaISectRegionClipRegionAction&)rMetaAction).maRegion;
}

void MetaISectRegionClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteRegion( rOStm, maRegion );
}

void MetaISectRegionClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadRegion( rIStm, maRegion );
}

MetaMoveClipRegionAction::MetaMoveClipRegionAction() :
    MetaAction  ( META_MOVECLIPREGION_ACTION ),
    mnHorzMove  ( 0 ),
    mnVertMove  ( 0 )
{}

MetaMoveClipRegionAction::~MetaMoveClipRegionAction()
{}

MetaMoveClipRegionAction::MetaMoveClipRegionAction( long nHorzMove, long nVertMove ) :
    MetaAction  ( META_MOVECLIPREGION_ACTION ),
    mnHorzMove  ( nHorzMove ),
    mnVertMove  ( nVertMove )
{}

void MetaMoveClipRegionAction::Execute( OutputDevice* pOut )
{
    pOut->MoveClipRegion( mnHorzMove, mnVertMove );
}

MetaAction* MetaMoveClipRegionAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMoveClipRegionAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaMoveClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    mnHorzMove = FRound( mnHorzMove * fScaleX );
    mnVertMove = FRound( mnVertMove * fScaleY );
}

bool MetaMoveClipRegionAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( mnHorzMove == ((MetaMoveClipRegionAction&)rMetaAction).mnHorzMove ) &&
           ( mnVertMove == ((MetaMoveClipRegionAction&)rMetaAction).mnVertMove );
}

void MetaMoveClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    //#fdo39428 SvStream no longer supports operator<<(long)
    rOStm.WriteInt32( sal::static_int_cast<sal_Int32>(mnHorzMove) ).WriteInt32( sal::static_int_cast<sal_Int32>(mnVertMove) );
}

void MetaMoveClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    //#fdo39428 SvStream no longer supports operator>>(long&)
    sal_Int32 nTmpHM(0), nTmpVM(0);
    rIStm.ReadInt32( nTmpHM ).ReadInt32( nTmpVM );
    mnHorzMove = nTmpHM;
    mnVertMove = nTmpVM;
}

MetaLineColorAction::MetaLineColorAction() :
    MetaAction  ( META_LINECOLOR_ACTION ),
    mbSet       ( false )
{}

MetaLineColorAction::~MetaLineColorAction()
{}

MetaLineColorAction::MetaLineColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( META_LINECOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{}

void MetaLineColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetLineColor( maColor );
    else
        pOut->SetLineColor();
}

MetaAction* MetaLineColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaLineColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaLineColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaLineColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaLineColorAction&)rMetaAction).mbSet );
}

void MetaLineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteUChar( mbSet );
}

void MetaLineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaFillColorAction::MetaFillColorAction() :
    MetaAction  ( META_FILLCOLOR_ACTION ),
    mbSet       ( false )
{}

MetaFillColorAction::~MetaFillColorAction()
{}

MetaFillColorAction::MetaFillColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( META_FILLCOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{}

void MetaFillColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetFillColor( maColor );
    else
        pOut->SetFillColor();
}

MetaAction* MetaFillColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaFillColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaFillColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaFillColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaFillColorAction&)rMetaAction).mbSet );
}

void MetaFillColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteUChar( mbSet );
}

void MetaFillColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaTextColorAction::MetaTextColorAction() :
    MetaAction(META_TEXTCOLOR_ACTION)
{}

MetaTextColorAction::~MetaTextColorAction()
{}

MetaTextColorAction::MetaTextColorAction( const Color& rColor ) :
    MetaAction  ( META_TEXTCOLOR_ACTION ),
    maColor     ( rColor )
{}

void MetaTextColorAction::Execute( OutputDevice* pOut )
{
    pOut->SetTextColor( maColor );
}

MetaAction* MetaTextColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaTextColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return maColor == ((MetaTextColorAction&)rMetaAction).maColor;
}

void MetaTextColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    maColor.Write( rOStm, true );
}

void MetaTextColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    maColor.Read( rIStm, true );
}

MetaTextFillColorAction::MetaTextFillColorAction() :
    MetaAction  ( META_TEXTFILLCOLOR_ACTION ),
    mbSet       ( false )
{}

MetaTextFillColorAction::~MetaTextFillColorAction()
{}

MetaTextFillColorAction::MetaTextFillColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( META_TEXTFILLCOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{}

void MetaTextFillColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetTextFillColor( maColor );
    else
        pOut->SetTextFillColor();
}

MetaAction* MetaTextFillColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextFillColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaTextFillColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaTextFillColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaTextFillColorAction&)rMetaAction).mbSet );
}

void MetaTextFillColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteUChar( mbSet );
}

void MetaTextFillColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaTextLineColorAction::MetaTextLineColorAction() :
    MetaAction  ( META_TEXTLINECOLOR_ACTION ),
    mbSet       ( false )
{}

MetaTextLineColorAction::~MetaTextLineColorAction()
{}

MetaTextLineColorAction::MetaTextLineColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( META_TEXTLINECOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{}

void MetaTextLineColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetTextLineColor( maColor );
    else
        pOut->SetTextLineColor();
}

MetaAction* MetaTextLineColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextLineColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaTextLineColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaTextLineColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaTextLineColorAction&)rMetaAction).mbSet );
}

void MetaTextLineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteUChar( mbSet );
}

void MetaTextLineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaOverlineColorAction::MetaOverlineColorAction() :
    MetaAction  ( META_OVERLINECOLOR_ACTION ),
    mbSet       ( false )
{}

MetaOverlineColorAction::~MetaOverlineColorAction()
{}

MetaOverlineColorAction::MetaOverlineColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( META_OVERLINECOLOR_ACTION ),
    maColor     ( rColor ),
    mbSet       ( bSet )
{}

void MetaOverlineColorAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetOverlineColor( maColor );
    else
        pOut->SetOverlineColor();
}

MetaAction* MetaOverlineColorAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaOverlineColorAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaOverlineColorAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maColor == ((MetaOverlineColorAction&)rMetaAction).maColor ) &&
           ( mbSet == ((MetaOverlineColorAction&)rMetaAction).mbSet );
}

void MetaOverlineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteUChar( mbSet );
}

void MetaOverlineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaTextAlignAction::MetaTextAlignAction() :
    MetaAction  ( META_TEXTALIGN_ACTION ),
    maAlign     ( ALIGN_TOP )
{}

MetaTextAlignAction::~MetaTextAlignAction()
{}

MetaTextAlignAction::MetaTextAlignAction( TextAlign aAlign ) :
    MetaAction  ( META_TEXTALIGN_ACTION ),
    maAlign     ( aAlign )
{}

void MetaTextAlignAction::Execute( OutputDevice* pOut )
{
    pOut->SetTextAlign( maAlign );
}

MetaAction* MetaTextAlignAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextAlignAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaTextAlignAction::Compare( const MetaAction& rMetaAction ) const
{
    return maAlign == ((MetaTextAlignAction&)rMetaAction).maAlign;
}

void MetaTextAlignAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    rOStm.WriteUInt16( (sal_uInt16) maAlign );
}

void MetaTextAlignAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    sal_uInt16 nTmp16(0);

    VersionCompat aCompat(rIStm, STREAM_READ);
    rIStm.ReadUInt16( nTmp16 ); maAlign = (TextAlign) nTmp16;
}

MetaMapModeAction::MetaMapModeAction() :
    MetaAction(META_MAPMODE_ACTION)
{}

MetaMapModeAction::~MetaMapModeAction()
{}

MetaMapModeAction::MetaMapModeAction( const MapMode& rMapMode ) :
    MetaAction  ( META_MAPMODE_ACTION ),
    maMapMode   ( rMapMode )
{}

void MetaMapModeAction::Execute( OutputDevice* pOut )
{
    pOut->SetMapMode( maMapMode );
}

MetaAction* MetaMapModeAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaMapModeAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaMapModeAction::Scale( double fScaleX, double fScaleY )
{
    Point aPoint( maMapMode.GetOrigin() );

    ImplScalePoint( aPoint, fScaleX, fScaleY );
    maMapMode.SetOrigin( aPoint );
}

bool MetaMapModeAction::Compare( const MetaAction& rMetaAction ) const
{
    return maMapMode == ((MetaMapModeAction&)rMetaAction).maMapMode;
}

void MetaMapModeAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteMapMode( rOStm, maMapMode );
}

void MetaMapModeAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadMapMode( rIStm, maMapMode );
}

MetaFontAction::MetaFontAction() :
    MetaAction(META_FONT_ACTION)
{}

MetaFontAction::~MetaFontAction()
{}

MetaFontAction::MetaFontAction( const Font& rFont ) :
    MetaAction  ( META_FONT_ACTION ),
    maFont      ( rFont )
{
    // #96876: because RTL_TEXTENCODING_SYMBOL is often set at the StarSymbol font,
    // we change the textencoding to RTL_TEXTENCODING_UNICODE here, which seems
    // to be the right way; changing the textencoding at other sources
    // is too dangerous at the moment
    if ( IsStarSymbol( maFont.GetName() )
        && ( maFont.GetCharSet() != RTL_TEXTENCODING_UNICODE ) )
    {
        maFont.SetCharSet( RTL_TEXTENCODING_UNICODE );
    }
}

void MetaFontAction::Execute( OutputDevice* pOut )
{
    pOut->SetFont( maFont );
}

MetaAction* MetaFontAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaFontAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaFontAction::Scale( double fScaleX, double fScaleY )
{
    const Size aSize(
        FRound(maFont.GetSize().Width() * fabs(fScaleX)),
        FRound(maFont.GetSize().Height() * fabs(fScaleY)));
    maFont.SetSize( aSize );
}

bool MetaFontAction::Compare( const MetaAction& rMetaAction ) const
{
    return maFont == ((MetaFontAction&)rMetaAction).maFont;
}

void MetaFontAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    WriteFont( rOStm, maFont );
    pData->meActualCharSet = maFont.GetCharSet();
    if ( pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW )
        pData->meActualCharSet = osl_getThreadTextEncoding();
}

void MetaFontAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadFont( rIStm, maFont );
    pData->meActualCharSet = maFont.GetCharSet();
    if ( pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW )
        pData->meActualCharSet = osl_getThreadTextEncoding();
}

MetaPushAction::MetaPushAction() :
    MetaAction  ( META_PUSH_ACTION ),
    mnFlags     ( 0 )
{}

MetaPushAction::~MetaPushAction()
{}

MetaPushAction::MetaPushAction( sal_uInt16 nFlags ) :
    MetaAction  ( META_PUSH_ACTION ),
    mnFlags     ( nFlags )
{}

void MetaPushAction::Execute( OutputDevice* pOut )
{
    pOut->Push( mnFlags );
}

MetaAction* MetaPushAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPushAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaPushAction::Compare( const MetaAction& rMetaAction ) const
{
    return mnFlags == ((MetaPushAction&)rMetaAction).mnFlags;
}

void MetaPushAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    rOStm.WriteUInt16( mnFlags );
}

void MetaPushAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    rIStm.ReadUInt16( mnFlags );
}

MetaPopAction::MetaPopAction() :
    MetaAction(META_POP_ACTION)
{}

MetaPopAction::~MetaPopAction()
{}

void MetaPopAction::Execute( OutputDevice* pOut )
{
    pOut->Pop();
}

MetaAction* MetaPopAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaPopAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaPopAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
}

void MetaPopAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
}

MetaRasterOpAction::MetaRasterOpAction() :
    MetaAction  ( META_RASTEROP_ACTION ),
    meRasterOp  ( ROP_OVERPAINT )
{}

MetaRasterOpAction::~MetaRasterOpAction()
{}

MetaRasterOpAction::MetaRasterOpAction( RasterOp eRasterOp ) :
    MetaAction  ( META_RASTEROP_ACTION ),
    meRasterOp  ( eRasterOp )
{
}

void MetaRasterOpAction::Execute( OutputDevice* pOut )
{
    pOut->SetRasterOp( meRasterOp );
}

MetaAction* MetaRasterOpAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaRasterOpAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaRasterOpAction::Compare( const MetaAction& rMetaAction ) const
{
    return meRasterOp == ((MetaRasterOpAction&)rMetaAction).meRasterOp;
}

void MetaRasterOpAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    rOStm.WriteUInt16( (sal_uInt16) meRasterOp );
}

void MetaRasterOpAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    sal_uInt16 nTmp16(0);

    VersionCompat aCompat(rIStm, STREAM_READ);
    rIStm.ReadUInt16( nTmp16 ); meRasterOp = (RasterOp) nTmp16;
}

MetaTransparentAction::MetaTransparentAction() :
    MetaAction      ( META_TRANSPARENT_ACTION ),
    mnTransPercent  ( 0 )
{}

MetaTransparentAction::~MetaTransparentAction()
{}

MetaTransparentAction::MetaTransparentAction( const PolyPolygon& rPolyPoly, sal_uInt16 nTransPercent ) :
    MetaAction      ( META_TRANSPARENT_ACTION ),
    maPolyPoly      ( rPolyPoly ),
    mnTransPercent  ( nTransPercent )
{}

void MetaTransparentAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTransparent( maPolyPoly, mnTransPercent );
}

MetaAction* MetaTransparentAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTransparentAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaTransparentAction::Move( long nHorzMove, long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaTransparentAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

bool MetaTransparentAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maPolyPoly == ((MetaTransparentAction&)rMetaAction).maPolyPoly ) &&
           ( mnTransPercent == ((MetaTransparentAction&)rMetaAction).mnTransPercent );
}

void MetaTransparentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);

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

    WritePolyPolygon( rOStm, aNoCurvePolyPolygon );
    rOStm.WriteUInt16( mnTransPercent );
}

void MetaTransparentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPolyPolygon( rIStm, maPolyPoly );
    rIStm.ReadUInt16( mnTransPercent );
}

MetaFloatTransparentAction::MetaFloatTransparentAction() :
    MetaAction(META_FLOATTRANSPARENT_ACTION)
{}

MetaFloatTransparentAction::~MetaFloatTransparentAction()
{}

MetaFloatTransparentAction::MetaFloatTransparentAction( const GDIMetaFile& rMtf, const Point& rPos,
                                                        const Size& rSize, const Gradient& rGradient ) :
    MetaAction      ( META_FLOATTRANSPARENT_ACTION ),
    maMtf           ( rMtf ),
    maPoint         ( rPos ),
    maSize          ( rSize ),
    maGradient      ( rGradient )
{}

void MetaFloatTransparentAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTransparent( maMtf, maPoint, maSize, maGradient );
}

MetaAction* MetaFloatTransparentAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaFloatTransparentAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaFloatTransparentAction::Move( long nHorzMove, long nVertMove )
{
    maPoint.Move( nHorzMove, nVertMove );
}

void MetaFloatTransparentAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPoint, maSize);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPoint = aRectangle.TopLeft();
    maSize = aRectangle.GetSize();
}

bool MetaFloatTransparentAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maMtf == ((MetaFloatTransparentAction&)rMetaAction).maMtf ) &&
           ( maPoint == ((MetaFloatTransparentAction&)rMetaAction).maPoint ) &&
           ( maSize == ((MetaFloatTransparentAction&)rMetaAction).maSize ) &&
           ( maGradient == ((MetaFloatTransparentAction&)rMetaAction).maGradient );
}

void MetaFloatTransparentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);

    maMtf.Write( rOStm );
    WritePair( rOStm,  maPoint );
    WritePair( rOStm, maSize );
    WriteGradient( rOStm, maGradient );
}

void MetaFloatTransparentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadGDIMetaFile( rIStm, maMtf );
    ReadPair( rIStm, maPoint );
    ReadPair( rIStm, maSize );
    ReadGradient( rIStm, maGradient );
}

MetaEPSAction::MetaEPSAction() :
    MetaAction(META_EPS_ACTION)
{}

MetaEPSAction::~MetaEPSAction()
{}

MetaEPSAction::MetaEPSAction( const Point& rPoint, const Size& rSize,
                              const GfxLink& rGfxLink, const GDIMetaFile& rSubst ) :
    MetaAction  ( META_EPS_ACTION ),
    maGfxLink   ( rGfxLink ),
    maSubst     ( rSubst ),
    maPoint     ( rPoint ),
    maSize      ( rSize )
{}

void MetaEPSAction::Execute( OutputDevice* pOut )
{
    pOut->DrawEPS( maPoint, maSize, maGfxLink, &maSubst );
}

MetaAction* MetaEPSAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaEPSAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

void MetaEPSAction::Move( long nHorzMove, long nVertMove )
{
    maPoint.Move( nHorzMove, nVertMove );
}

void MetaEPSAction::Scale( double fScaleX, double fScaleY )
{
    Rectangle aRectangle(maPoint, maSize);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPoint = aRectangle.TopLeft();
    maSize = aRectangle.GetSize();
}

bool MetaEPSAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maGfxLink.IsEqual(((MetaEPSAction&)rMetaAction).maGfxLink )) &&
           ( maSubst == ((MetaEPSAction&)rMetaAction).maSubst ) &&
           ( maPoint == ((MetaEPSAction&)rMetaAction).maPoint ) &&
           ( maSize == ((MetaEPSAction&)rMetaAction).maSize );
}

void MetaEPSAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);

    WriteGfxLink( rOStm, maGfxLink );
    WritePair( rOStm, maPoint );
    WritePair( rOStm, maSize );
    maSubst.Write( rOStm );
}

void MetaEPSAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadGfxLink( rIStm, maGfxLink );
    ReadPair( rIStm, maPoint );
    ReadPair( rIStm, maSize );
    ReadGDIMetaFile( rIStm, maSubst );
}

MetaRefPointAction::MetaRefPointAction() :
    MetaAction  ( META_REFPOINT_ACTION ),
    mbSet       ( false )
{}

MetaRefPointAction::~MetaRefPointAction()
{}

MetaRefPointAction::MetaRefPointAction( const Point& rRefPoint, bool bSet ) :
    MetaAction  ( META_REFPOINT_ACTION ),
    maRefPoint  ( rRefPoint ),
    mbSet       ( bSet )
{}

void MetaRefPointAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetRefPoint( maRefPoint );
    else
        pOut->SetRefPoint();
}

MetaAction* MetaRefPointAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaRefPointAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaRefPointAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maRefPoint == ((MetaRefPointAction&)rMetaAction).maRefPoint ) &&
           ( mbSet == ((MetaRefPointAction&)rMetaAction).mbSet );
}

void MetaRefPointAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);

    WritePair( rOStm, maRefPoint );
    rOStm.WriteUChar( mbSet );
}

void MetaRefPointAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    ReadPair( rIStm, maRefPoint ).ReadCharAsBool( mbSet );
}

MetaCommentAction::MetaCommentAction( sal_Int32 nValue ) :
    MetaAction  ( META_COMMENT_ACTION ),
    mnValue     ( nValue )
{
    ImplInitDynamicData( NULL, 0UL );
}

MetaCommentAction::MetaCommentAction( const MetaCommentAction& rAct ) :
    MetaAction  ( META_COMMENT_ACTION ),
    maComment   ( rAct.maComment ),
    mnValue     ( rAct.mnValue )
{
    ImplInitDynamicData( rAct.mpData, rAct.mnDataSize );
}

MetaCommentAction::MetaCommentAction( const OString& rComment, sal_Int32 nValue, const sal_uInt8* pData, sal_uInt32 nDataSize ) :
    MetaAction  ( META_COMMENT_ACTION ),
    maComment   ( rComment ),
    mnValue     ( nValue )
{
    ImplInitDynamicData( pData, nDataSize );
}

MetaCommentAction::~MetaCommentAction()
{
    if ( mpData )
        delete[] mpData;
}

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

void MetaCommentAction::Execute( OutputDevice* pOut )
{
    if ( pOut->GetConnectMetaFile() )
    {
        Duplicate();
        pOut->GetConnectMetaFile()->AddAction( this );
    }
}

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
            bool bPathStroke = (maComment == "XPATHSTROKE_SEQ_BEGIN");
            if ( bPathStroke || maComment == "XPATHFILL_SEQ_BEGIN" )
            {
                SvMemoryStream  aMemStm( (void*)mpData, mnDataSize, STREAM_READ );
                SvMemoryStream  aDest;
                if ( bPathStroke )
                {
                    SvtGraphicStroke aStroke;
                    ReadSvtGraphicStroke( aMemStm, aStroke );

                    Polygon aPath;
                    aStroke.getPath( aPath );
                    aPath.Move( nXMove, nYMove );
                    aStroke.setPath( aPath );

                    PolyPolygon aStartArrow;
                    aStroke.getStartArrow(aStartArrow);
                    aStartArrow.Move(nXMove, nYMove);
                    aStroke.setStartArrow(aStartArrow);

                    PolyPolygon aEndArrow;
                    aStroke.getEndArrow(aEndArrow);
                    aEndArrow.Move(nXMove, nYMove);
                    aStroke.setEndArrow(aEndArrow);

                    WriteSvtGraphicStroke( aDest, aStroke );
                }
                else
                {
                    SvtGraphicFill aFill;
                    ReadSvtGraphicFill( aMemStm, aFill );

                    PolyPolygon aPath;
                    aFill.getPath( aPath );
                    aPath.Move( nXMove, nYMove );
                    aFill.setPath( aPath );

                    WriteSvtGraphicFill( aDest, aFill );
                }
                delete[] mpData;
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            }
        }
    }
}

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
            bool bPathStroke = (maComment == "XPATHSTROKE_SEQ_BEGIN");
            if ( bPathStroke || maComment == "XPATHFILL_SEQ_BEGIN" )
            {
                SvMemoryStream  aMemStm( (void*)mpData, mnDataSize, STREAM_READ );
                SvMemoryStream  aDest;
                if ( bPathStroke )
                {
                    SvtGraphicStroke aStroke;
                    ReadSvtGraphicStroke( aMemStm, aStroke );
                    aStroke.scale( fXScale, fYScale );
                    WriteSvtGraphicStroke( aDest, aStroke );
                }
                else
                {
                    SvtGraphicFill aFill;
                    ReadSvtGraphicFill( aMemStm, aFill );
                    PolyPolygon aPath;
                    aFill.getPath( aPath );
                    aPath.Scale( fXScale, fYScale );
                    aFill.setPath( aPath );
                    WriteSvtGraphicFill( aDest, aFill );
                }
                delete[] mpData;
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            } else if( maComment == "EMF_PLUS_HEADER_INFO" ){
                SvMemoryStream  aMemStm( (void*)mpData, mnDataSize, STREAM_READ );
                SvMemoryStream  aDest;

                sal_Int32 nLeft(0), nRight(0), nTop(0), nBottom(0);
                sal_Int32 nPixX(0), nPixY(0), nMillX(0), nMillY(0);
                float m11(0), m12(0), m21(0), m22(0), mdx(0), mdy(0);

                // read data
                aMemStm.ReadInt32( nLeft ).ReadInt32( nTop ).ReadInt32( nRight ).ReadInt32( nBottom );
                aMemStm.ReadInt32( nPixX ).ReadInt32( nPixY ).ReadInt32( nMillX ).ReadInt32( nMillY );
                aMemStm.ReadFloat( m11 ).ReadFloat( m12 ).ReadFloat( m21 ).ReadFloat( m22 ).ReadFloat( mdx ).ReadFloat( mdy );

                // add scale to the transformation
                m11 *= fXScale;
                m12 *= fXScale;
                m22 *= fYScale;
                m21 *= fYScale;

                // prepare new data
                aDest.WriteInt32( nLeft ).WriteInt32( nTop ).WriteInt32( nRight ).WriteInt32( nBottom );
                aDest.WriteInt32( nPixX ).WriteInt32( nPixY ).WriteInt32( nMillX ).WriteInt32( nMillY );
                aDest.WriteFloat( m11 ).WriteFloat( m12 ).WriteFloat( m21 ).WriteFloat( m22 ).WriteFloat( mdx ).WriteFloat( mdy );

                // save them
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            }
        }
    }
}

bool MetaCommentAction::Compare( const MetaAction& rMetaAction ) const
{
    return ( maComment == ((MetaCommentAction&)rMetaAction).maComment ) &&
           ( mnValue == ((MetaCommentAction&)rMetaAction).mnValue ) &&
           ( mnDataSize == ((MetaCommentAction&)rMetaAction).mnDataSize ) &&
           ( memcmp( mpData, ((MetaCommentAction&)rMetaAction).mpData, mnDataSize ) == 0 );
}

void MetaCommentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStm, maComment);
    rOStm.WriteInt32( mnValue ).WriteUInt32( mnDataSize );

    if ( mnDataSize )
        rOStm.Write( mpData, mnDataSize );
}

void MetaCommentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    maComment = read_uInt16_lenPrefixed_uInt8s_ToOString(rIStm);
    rIStm.ReadInt32( mnValue ).ReadUInt32( mnDataSize );

    SAL_INFO("vcl.gdi", "MetaCommentAction::Read " << maComment);

    delete[] mpData;

    if( mnDataSize )
    {
        mpData = new sal_uInt8[ mnDataSize ];
        rIStm.Read( mpData, mnDataSize );
    }
    else
        mpData = NULL;
}

MetaLayoutModeAction::MetaLayoutModeAction() :
    MetaAction  ( META_LAYOUTMODE_ACTION ),
    mnLayoutMode( 0 )
{}

MetaLayoutModeAction::~MetaLayoutModeAction()
{}

MetaLayoutModeAction::MetaLayoutModeAction( sal_uInt32 nLayoutMode ) :
    MetaAction  ( META_LAYOUTMODE_ACTION ),
    mnLayoutMode( nLayoutMode )
{}

void MetaLayoutModeAction::Execute( OutputDevice* pOut )
{
    pOut->SetLayoutMode( mnLayoutMode );
}

MetaAction* MetaLayoutModeAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaLayoutModeAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaLayoutModeAction::Compare( const MetaAction& rMetaAction ) const
{
    return mnLayoutMode == ((MetaLayoutModeAction&)rMetaAction).mnLayoutMode;
}

void MetaLayoutModeAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    rOStm.WriteUInt32( mnLayoutMode );
}

void MetaLayoutModeAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    rIStm.ReadUInt32( mnLayoutMode );
}

MetaTextLanguageAction::MetaTextLanguageAction() :
    MetaAction  ( META_TEXTLANGUAGE_ACTION ),
    meTextLanguage( LANGUAGE_DONTKNOW )
{}

MetaTextLanguageAction::~MetaTextLanguageAction()
{}

MetaTextLanguageAction::MetaTextLanguageAction( LanguageType eTextLanguage ) :
    MetaAction  ( META_TEXTLANGUAGE_ACTION ),
    meTextLanguage( eTextLanguage )
{}

void MetaTextLanguageAction::Execute( OutputDevice* pOut )
{
    pOut->SetDigitLanguage( meTextLanguage );
}

MetaAction* MetaTextLanguageAction::Clone()
{
    MetaAction* pClone = (MetaAction*) new MetaTextLanguageAction( *this );
    pClone->ResetRefCount();
    return pClone;
}

bool MetaTextLanguageAction::Compare( const MetaAction& rMetaAction ) const
{
    return meTextLanguage == ((MetaTextLanguageAction&)rMetaAction).meTextLanguage;
}

void MetaTextLanguageAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, STREAM_WRITE, 1);
    rOStm.WriteUInt16( meTextLanguage );
}

void MetaTextLanguageAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, STREAM_READ);
    rIStm.ReadUInt16( meTextLanguage );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
