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
#include <stdio.h>
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

const char *
meta_action_name(MetaActionType nMetaAction)
{
#ifndef SAL_LOG_INFO
    (void) nMetaAction;
    return "";
#else
    switch( nMetaAction )
    {
    case MetaActionType::NONE: return "NULL";
    case MetaActionType::PIXEL: return "PIXEL";
    case MetaActionType::POINT: return "POINT";
    case MetaActionType::LINE: return "LINE";
    case MetaActionType::RECT: return "RECT";
    case MetaActionType::ROUNDRECT: return "ROUNDRECT";
    case MetaActionType::ELLIPSE: return "ELLIPSE";
    case MetaActionType::ARC: return "ARC";
    case MetaActionType::PIE: return "PIE";
    case MetaActionType::CHORD: return "CHORD";
    case MetaActionType::POLYLINE: return "POLYLINE";
    case MetaActionType::POLYGON: return "POLYGON";
    case MetaActionType::POLYPOLYGON: return "POLYPOLYGON";
    case MetaActionType::TEXT: return "TEXT";
    case MetaActionType::TEXTARRAY: return "TEXTARRAY";
    case MetaActionType::STRETCHTEXT: return "STRETCHTEXT";
    case MetaActionType::TEXTRECT: return "TEXTRECT";
    case MetaActionType::BMP: return "BMP";
    case MetaActionType::BMPSCALE: return "BMPSCALE";
    case MetaActionType::BMPSCALEPART: return "BMPSCALEPART";
    case MetaActionType::BMPEX: return "BMPEX";
    case MetaActionType::BMPEXSCALE: return "BMPEXSCALE";
    case MetaActionType::BMPEXSCALEPART: return "BMPEXSCALEPART";
    case MetaActionType::MASK: return "MASK";
    case MetaActionType::MASKSCALE: return "MASKSCALE";
    case MetaActionType::MASKSCALEPART: return "MASKSCALEPART";
    case MetaActionType::GRADIENT: return "GRADIENT";
    case MetaActionType::HATCH: return "HATCH";
    case MetaActionType::WALLPAPER: return "WALLPAPER";
    case MetaActionType::CLIPREGION: return "CLIPREGION";
    case MetaActionType::ISECTRECTCLIPREGION: return "ISECTRECTCLIPREGION";
    case MetaActionType::ISECTREGIONCLIPREGION: return "ISECTREGIONCLIPREGION";
    case MetaActionType::MOVECLIPREGION: return "MOVECLIPREGION";
    case MetaActionType::LINECOLOR: return "LINECOLOR";
    case MetaActionType::FILLCOLOR: return "FILLCOLOR";
    case MetaActionType::TEXTCOLOR: return "TEXTCOLOR";
    case MetaActionType::TEXTFILLCOLOR: return "TEXTFILLCOLOR";
    case MetaActionType::TEXTALIGN: return "TEXTALIGN";
    case MetaActionType::MAPMODE: return "MAPMODE";
    case MetaActionType::FONT: return "FONT";
    case MetaActionType::PUSH: return "PUSH";
    case MetaActionType::POP: return "POP";
    case MetaActionType::RASTEROP: return "RASTEROP";
    case MetaActionType::TRANSPARENT: return "TRANSPARENT";
    case MetaActionType::EPS: return "EPS";
    case MetaActionType::REFPOINT: return "REFPOINT";
    case MetaActionType::TEXTLINECOLOR: return "TEXTLINECOLOR";
    case MetaActionType::TEXTLINE: return "TEXTLINE";
    case MetaActionType::FLOATTRANSPARENT: return "FLOATTRANSPARENT";
    case MetaActionType::GRADIENTEX: return "GRADIENTEX";
    case MetaActionType::LAYOUTMODE: return "LAYOUTMODE";
    case MetaActionType::TEXTLANGUAGE: return "TEXTLANGUAGE";
    case MetaActionType::OVERLINECOLOR: return "OVERLINECOLOR";
    case MetaActionType::COMMENT: return "COMMENT";
    default:
        // Yes, return a pointer to a static buffer. This is a very
        // local debugging output function, so no big deal.
        static char buffer[6];
        sprintf(buffer, "%u", static_cast<unsigned int>(nMetaAction));
        return buffer;
    }
#endif
}

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
    mnType( MetaActionType::NONE )
{
}

MetaAction::MetaAction( MetaActionType nType ) :
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
    rOStm.WriteUInt16( static_cast<sal_uInt16>(mnType) );
}

void MetaAction::Read( SvStream&, ImplMetaReadData* )
{
    // DO NOT read mnType - ReadMetaAction already did that!
}

MetaAction* MetaAction::ReadMetaAction( SvStream& rIStm, ImplMetaReadData* pData )
{
    MetaAction* pAction = NULL;
    sal_uInt16 nTmp = 0;
    rIStm.ReadUInt16( nTmp );
    MetaActionType nType = static_cast<MetaActionType>(nTmp);

    SAL_INFO("vcl.gdi", "ReadMetaAction " << meta_action_name( nType ));

    switch( nType )
    {
        case MetaActionType::NONE: pAction = new MetaAction; break;
        case MetaActionType::PIXEL: pAction = new MetaPixelAction; break;
        case MetaActionType::POINT: pAction = new MetaPointAction; break;
        case MetaActionType::LINE: pAction = new MetaLineAction; break;
        case MetaActionType::RECT: pAction = new MetaRectAction; break;
        case MetaActionType::ROUNDRECT: pAction = new MetaRoundRectAction; break;
        case MetaActionType::ELLIPSE: pAction = new MetaEllipseAction; break;
        case MetaActionType::ARC: pAction = new MetaArcAction; break;
        case MetaActionType::PIE: pAction = new MetaPieAction; break;
        case MetaActionType::CHORD: pAction = new MetaChordAction; break;
        case MetaActionType::POLYLINE: pAction = new MetaPolyLineAction; break;
        case MetaActionType::POLYGON: pAction = new MetaPolygonAction; break;
        case MetaActionType::POLYPOLYGON: pAction = new MetaPolyPolygonAction; break;
        case MetaActionType::TEXT: pAction = new MetaTextAction; break;
        case MetaActionType::TEXTARRAY: pAction = new MetaTextArrayAction; break;
        case MetaActionType::STRETCHTEXT: pAction = new MetaStretchTextAction; break;
        case MetaActionType::TEXTRECT: pAction = new MetaTextRectAction; break;
        case MetaActionType::TEXTLINE: pAction = new MetaTextLineAction; break;
        case MetaActionType::BMP: pAction = new MetaBmpAction; break;
        case MetaActionType::BMPSCALE: pAction = new MetaBmpScaleAction; break;
        case MetaActionType::BMPSCALEPART: pAction = new MetaBmpScalePartAction; break;
        case MetaActionType::BMPEX: pAction = new MetaBmpExAction; break;
        case MetaActionType::BMPEXSCALE: pAction = new MetaBmpExScaleAction; break;
        case MetaActionType::BMPEXSCALEPART: pAction = new MetaBmpExScalePartAction; break;
        case MetaActionType::MASK: pAction = new MetaMaskAction; break;
        case MetaActionType::MASKSCALE: pAction = new MetaMaskScaleAction; break;
        case MetaActionType::MASKSCALEPART: pAction = new MetaMaskScalePartAction; break;
        case MetaActionType::GRADIENT: pAction = new MetaGradientAction; break;
        case MetaActionType::GRADIENTEX: pAction = new MetaGradientExAction; break;
        case MetaActionType::HATCH: pAction = new MetaHatchAction; break;
        case MetaActionType::WALLPAPER: pAction = new MetaWallpaperAction; break;
        case MetaActionType::CLIPREGION: pAction = new MetaClipRegionAction; break;
        case MetaActionType::ISECTRECTCLIPREGION: pAction = new MetaISectRectClipRegionAction; break;
        case MetaActionType::ISECTREGIONCLIPREGION: pAction = new MetaISectRegionClipRegionAction; break;
        case MetaActionType::MOVECLIPREGION: pAction = new MetaMoveClipRegionAction; break;
        case MetaActionType::LINECOLOR: pAction = new MetaLineColorAction; break;
        case MetaActionType::FILLCOLOR: pAction = new MetaFillColorAction; break;
        case MetaActionType::TEXTCOLOR: pAction = new MetaTextColorAction; break;
        case MetaActionType::TEXTFILLCOLOR: pAction = new MetaTextFillColorAction; break;
        case MetaActionType::TEXTLINECOLOR: pAction = new MetaTextLineColorAction; break;
        case MetaActionType::OVERLINECOLOR: pAction = new MetaOverlineColorAction; break;
        case MetaActionType::TEXTALIGN: pAction = new MetaTextAlignAction; break;
        case MetaActionType::MAPMODE: pAction = new MetaMapModeAction; break;
        case MetaActionType::FONT: pAction = new MetaFontAction; break;
        case MetaActionType::PUSH: pAction = new MetaPushAction; break;
        case MetaActionType::POP: pAction = new MetaPopAction; break;
        case MetaActionType::RASTEROP: pAction = new MetaRasterOpAction; break;
        case MetaActionType::TRANSPARENT: pAction = new MetaTransparentAction; break;
        case MetaActionType::FLOATTRANSPARENT: pAction = new MetaFloatTransparentAction; break;
        case MetaActionType::EPS: pAction = new MetaEPSAction; break;
        case MetaActionType::REFPOINT: pAction = new MetaRefPointAction; break;
        case MetaActionType::COMMENT: pAction = new MetaCommentAction; break;
        case MetaActionType::LAYOUTMODE: pAction = new MetaLayoutModeAction; break;
        case MetaActionType::TEXTLANGUAGE: pAction = new MetaTextLanguageAction; break;

        default:
        {
            VersionCompat aCompat(rIStm, StreamMode::READ);
        }
        break;
    }

    if( pAction )
        pAction->Read( rIStm, pData );

    return pAction;
}

MetaPixelAction::MetaPixelAction() :
    MetaAction(MetaActionType::PIXEL)
{}

MetaPixelAction::~MetaPixelAction()
{}

MetaPixelAction::MetaPixelAction( const Point& rPt, const Color& rColor ) :
    MetaAction  ( MetaActionType::PIXEL ),
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
    return ( maPt == static_cast<const MetaPixelAction&>(rMetaAction).maPt ) &&
           ( maColor == static_cast<const MetaPixelAction&>(rMetaAction).maColor );
}

void MetaPixelAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WritePair( rOStm, maPt );
    maColor.Write( rOStm, true );
}

void MetaPixelAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadPair( rIStm, maPt );
    maColor.Read( rIStm, true );
}

MetaPointAction::MetaPointAction() :
    MetaAction(MetaActionType::POINT)
{}

MetaPointAction::~MetaPointAction()
{}

MetaPointAction::MetaPointAction( const Point& rPt ) :
    MetaAction  ( MetaActionType::POINT ),
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
    return maPt == static_cast<const MetaPointAction&>(rMetaAction).maPt;
}

void MetaPointAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WritePair( rOStm, maPt );
}

void MetaPointAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadPair( rIStm, maPt );
}

MetaLineAction::MetaLineAction() :
    MetaAction(MetaActionType::LINE)
{}

MetaLineAction::~MetaLineAction()
{}

MetaLineAction::MetaLineAction( const Point& rStart, const Point& rEnd ) :
    MetaAction  ( MetaActionType::LINE ),
    maStartPt   ( rStart ),
    maEndPt     ( rEnd )
{}

MetaLineAction::MetaLineAction( const Point& rStart, const Point& rEnd,
                                const LineInfo& rLineInfo ) :
    MetaAction  ( MetaActionType::LINE ),
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
    return ( maLineInfo == static_cast<const MetaLineAction&>(rMetaAction).maLineInfo ) &&
           ( maStartPt == static_cast<const MetaLineAction&>(rMetaAction).maStartPt ) &&
           ( maEndPt == static_cast<const MetaLineAction&>(rMetaAction).maEndPt );
}

void MetaLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);

    WritePair( rOStm, maStartPt );
    WritePair( rOStm, maEndPt );  // Version 1
    WriteLineInfo( rOStm, maLineInfo );            // Version 2
}

void MetaLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);

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
    MetaAction(MetaActionType::RECT)
{}

MetaRectAction::~MetaRectAction()
{}

MetaRectAction::MetaRectAction( const Rectangle& rRect ) :
    MetaAction  ( MetaActionType::RECT ),
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
    return maRect == static_cast<const MetaRectAction&>(rMetaAction).maRect;
}

void MetaRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteRectangle( rOStm, maRect );
}

void MetaRectAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRectangle( rIStm, maRect );
}

MetaRoundRectAction::MetaRoundRectAction() :
    MetaAction  ( MetaActionType::ROUNDRECT ),
    mnHorzRound ( 0 ),
    mnVertRound ( 0 )
{}

MetaRoundRectAction::~MetaRoundRectAction()
{}

MetaRoundRectAction::MetaRoundRectAction( const Rectangle& rRect,
                                          sal_uInt32 nHorzRound, sal_uInt32 nVertRound ) :
    MetaAction  ( MetaActionType::ROUNDRECT ),
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
    return ( maRect == static_cast<const MetaRoundRectAction&>(rMetaAction).maRect ) &&
           ( mnHorzRound == static_cast<const MetaRoundRectAction&>(rMetaAction).mnHorzRound ) &&
           ( mnVertRound == static_cast<const MetaRoundRectAction&>(rMetaAction).mnVertRound );
}

void MetaRoundRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteRectangle( rOStm, maRect );
    rOStm.WriteUInt32( mnHorzRound ).WriteUInt32( mnVertRound );
}

void MetaRoundRectAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRectangle( rIStm, maRect ).ReadUInt32( mnHorzRound ).ReadUInt32( mnVertRound );
}

MetaEllipseAction::MetaEllipseAction() :
    MetaAction(MetaActionType::ELLIPSE)
{}

MetaEllipseAction::~MetaEllipseAction()
{}

MetaEllipseAction::MetaEllipseAction( const Rectangle& rRect ) :
    MetaAction  ( MetaActionType::ELLIPSE ),
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
    return maRect == static_cast<const MetaEllipseAction&>(rMetaAction).maRect;
}

void MetaEllipseAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteRectangle( rOStm, maRect );
}

void MetaEllipseAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRectangle( rIStm, maRect );
}

MetaArcAction::MetaArcAction() :
    MetaAction(MetaActionType::ARC)
{}

MetaArcAction::~MetaArcAction()
{}

MetaArcAction::MetaArcAction( const Rectangle& rRect,
                              const Point& rStart, const Point& rEnd ) :
    MetaAction  ( MetaActionType::ARC ),
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
    return ( maRect == static_cast<const MetaArcAction&>(rMetaAction).maRect ) &&
           ( maStartPt == static_cast<const MetaArcAction&>(rMetaAction).maStartPt ) &&
           ( maEndPt == static_cast<const MetaArcAction&>(rMetaAction).maEndPt );
}

void MetaArcAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteRectangle( rOStm, maRect );
    WritePair( rOStm, maStartPt );
    WritePair( rOStm, maEndPt );
}

void MetaArcAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRectangle( rIStm, maRect );
    ReadPair( rIStm, maStartPt );
    ReadPair( rIStm, maEndPt );
}

MetaPieAction::MetaPieAction() :
    MetaAction(MetaActionType::PIE)
{}

MetaPieAction::~MetaPieAction()
{}

MetaPieAction::MetaPieAction( const Rectangle& rRect,
                              const Point& rStart, const Point& rEnd ) :
    MetaAction  ( MetaActionType::PIE ),
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
    return ( maRect == static_cast<const MetaPieAction&>(rMetaAction).maRect ) &&
           ( maStartPt == static_cast<const MetaPieAction&>(rMetaAction).maStartPt ) &&
           ( maEndPt == static_cast<const MetaPieAction&>(rMetaAction).maEndPt );
}

void MetaPieAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteRectangle( rOStm, maRect );
    WritePair( rOStm, maStartPt );
    WritePair( rOStm, maEndPt );
}

void MetaPieAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRectangle( rIStm, maRect );
    ReadPair( rIStm, maStartPt );
    ReadPair( rIStm, maEndPt );
}

MetaChordAction::MetaChordAction() :
    MetaAction(MetaActionType::CHORD)
{}

MetaChordAction::~MetaChordAction()
{}

MetaChordAction::MetaChordAction( const Rectangle& rRect,
                                  const Point& rStart, const Point& rEnd ) :
    MetaAction  ( MetaActionType::CHORD ),
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
    return ( maRect == static_cast<const MetaChordAction&>(rMetaAction).maRect ) &&
           ( maStartPt == static_cast<const MetaChordAction&>(rMetaAction).maStartPt ) &&
           ( maEndPt == static_cast<const MetaChordAction&>(rMetaAction).maEndPt );
}

void MetaChordAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteRectangle( rOStm, maRect );
    WritePair( rOStm, maStartPt );
    WritePair( rOStm, maEndPt );
}

void MetaChordAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRectangle( rIStm, maRect );
    ReadPair( rIStm, maStartPt );
    ReadPair( rIStm, maEndPt );
}

MetaPolyLineAction::MetaPolyLineAction() :
    MetaAction(MetaActionType::POLYLINE)
{}

MetaPolyLineAction::~MetaPolyLineAction()
{}

MetaPolyLineAction::MetaPolyLineAction( const Polygon& rPoly ) :
    MetaAction  ( MetaActionType::POLYLINE ),
    maPoly      ( rPoly )
{}

MetaPolyLineAction::MetaPolyLineAction( const Polygon& rPoly, const LineInfo& rLineInfo ) :
    MetaAction  ( MetaActionType::POLYLINE ),
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
    if ( maLineInfo != static_cast<const MetaPolyLineAction&>(rMetaAction).maLineInfo )
        bIsEqual = false;
    else
        bIsEqual = maPoly.IsEqual(static_cast<const MetaPolyLineAction&>(rMetaAction).maPoly );
    return bIsEqual;

}

void MetaPolyLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 3);

    Polygon aSimplePoly;
    maPoly.AdaptiveSubdivide( aSimplePoly );

    WritePolygon( rOStm, aSimplePoly );                               // Version 1
    WriteLineInfo( rOStm, maLineInfo );                                // Version 2

    bool bHasPolyFlags = maPoly.HasFlags();        // Version 3
    rOStm.WriteBool( bHasPolyFlags );
    if ( bHasPolyFlags )
        maPoly.Write( rOStm );
}

void MetaPolyLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);

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
    MetaAction(MetaActionType::POLYGON)
{}

MetaPolygonAction::~MetaPolygonAction()
{}

MetaPolygonAction::MetaPolygonAction( const Polygon& rPoly ) :
    MetaAction  ( MetaActionType::POLYGON ),
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
    return maPoly.IsEqual(static_cast<const MetaPolygonAction&>(rMetaAction).maPoly );
}

void MetaPolygonAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);

    Polygon aSimplePoly;                            // Version 1
    maPoly.AdaptiveSubdivide( aSimplePoly );
    WritePolygon( rOStm, aSimplePoly );

    bool bHasPolyFlags = maPoly.HasFlags();    // Version 2
    rOStm.WriteBool( bHasPolyFlags );
    if ( bHasPolyFlags )
        maPoly.Write( rOStm );
}

void MetaPolygonAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);

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
    MetaAction(MetaActionType::POLYPOLYGON)
{}

MetaPolyPolygonAction::~MetaPolyPolygonAction()
{}

MetaPolyPolygonAction::MetaPolyPolygonAction( const tools::PolyPolygon& rPolyPoly ) :
    MetaAction  ( MetaActionType::POLYPOLYGON ),
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
    return maPolyPoly.IsEqual(static_cast<const MetaPolyPolygonAction&>(rMetaAction).maPolyPoly );
}

void MetaPolyPolygonAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);

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
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadPolyPolygon( rIStm, maPolyPoly );                // Version 1

    if ( aCompat.GetVersion() >= 2 )    // Version 2
    {
        sal_uInt16 nNumberOfComplexPolygons(0);
        rIStm.ReadUInt16( nNumberOfComplexPolygons );
        const size_t nMinRecordSize = sizeof(sal_uInt16);
        const size_t nMaxRecords = rIStm.remainingSize() / nMinRecordSize;
        if (nNumberOfComplexPolygons > nMaxRecords)
        {
            SAL_WARN("vcl.gdi", "Parsing error: " << nMaxRecords <<
                     " max possible entries, but " << nNumberOfComplexPolygons << " claimed, truncating");
            nNumberOfComplexPolygons = nMaxRecords;
        }
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
    MetaAction  ( MetaActionType::TEXT ),
    mnIndex     ( 0 ),
    mnLen       ( 0 )
{}

MetaTextAction::~MetaTextAction()
{}

MetaTextAction::MetaTextAction( const Point& rPt, const OUString& rStr,
                                sal_Int32 nIndex, sal_Int32 nLen ) :
    MetaAction  ( MetaActionType::TEXT ),
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
    return ( maPt == static_cast<const MetaTextAction&>(rMetaAction).maPt ) &&
           ( maStr == static_cast<const MetaTextAction&>(rMetaAction).maStr ) &&
           ( mnIndex == static_cast<const MetaTextAction&>(rMetaAction).mnIndex ) &&
           ( mnLen == static_cast<const MetaTextAction&>(rMetaAction).mnLen );
}

void MetaTextAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);
    WritePair( rOStm, maPt );
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm.WriteUInt16(mnIndex);
    rOStm.WriteUInt16(mnLen);

    write_uInt16_lenPrefixed_uInt16s_FromOUString(rOStm, maStr); // version 2
}

void MetaTextAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
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
    MetaAction  ( MetaActionType::TEXTARRAY ),
    mpDXAry     ( NULL ),
    mnIndex     ( 0 ),
    mnLen       ( 0 )
{}

MetaTextArrayAction::MetaTextArrayAction( const MetaTextArrayAction& rAction ) :
    MetaAction  ( MetaActionType::TEXTARRAY ),
    maStartPt   ( rAction.maStartPt ),
    maStr       ( rAction.maStr ),
    mnIndex     ( rAction.mnIndex ),
    mnLen       ( rAction.mnLen )
{
    if( rAction.mpDXAry )
    {
        const sal_Int32 nAryLen = mnLen;

        mpDXAry = new long[ nAryLen ];
        memcpy( mpDXAry, rAction.mpDXAry, nAryLen * sizeof( long ) );
    }
    else
        mpDXAry = NULL;
}

MetaTextArrayAction::MetaTextArrayAction( const Point& rStartPt,
                                          const OUString& rStr,
                                          const long* pDXAry,
                                          sal_Int32 nIndex,
                                          sal_Int32 nLen ) :
    MetaAction  ( MetaActionType::TEXTARRAY ),
    maStartPt   ( rStartPt ),
    maStr       ( rStr ),
    mnIndex     ( nIndex ),
    mnLen       ( nLen )
{
    const sal_Int32 nAryLen = pDXAry ? mnLen : 0;

    if( nAryLen )
    {
        mpDXAry = new long[ nAryLen ];
        memcpy( mpDXAry, pDXAry, nAryLen * sizeof(long) );
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
    return ( maStartPt == static_cast<const MetaTextArrayAction&>(rMetaAction).maStartPt ) &&
           ( maStr == static_cast<const MetaTextArrayAction&>(rMetaAction).maStr ) &&
           ( mnIndex == static_cast<const MetaTextArrayAction&>(rMetaAction).mnIndex ) &&
           ( mnLen == static_cast<const MetaTextArrayAction&>(rMetaAction).mnLen ) &&
           ( memcmp( mpDXAry, static_cast<const MetaTextArrayAction&>(rMetaAction).mpDXAry, mnLen ) == 0 );
}

void MetaTextArrayAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    const sal_Int32 nAryLen = mpDXAry ? mnLen : 0;

    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);
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

    VersionCompat aCompat(rIStm, StreamMode::READ);
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

    if (mnLen > maStr.getLength() - mnIndex)
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
            mpDXAry = new (std::nothrow)long[ mnLen ];
            if ( mpDXAry )
            {
                sal_Int32 i;
                sal_Int32 val;
                for( i = 0; i < nAryLen; i++ )
                {
                    rIStm.ReadInt32( val);
                    mpDXAry[ i ] = val;
                }
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
    MetaAction  ( MetaActionType::STRETCHTEXT ),
    mnWidth     ( 0 ),
    mnIndex     ( 0 ),
    mnLen       ( 0 )
{}

MetaStretchTextAction::~MetaStretchTextAction()
{}

MetaStretchTextAction::MetaStretchTextAction( const Point& rPt, sal_uInt32 nWidth,
                                              const OUString& rStr,
                                              sal_Int32 nIndex, sal_Int32 nLen ) :
    MetaAction  ( MetaActionType::STRETCHTEXT ),
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
    return ( maPt == static_cast<const MetaStretchTextAction&>(rMetaAction).maPt ) &&
           ( maStr == static_cast<const MetaStretchTextAction&>(rMetaAction).maStr ) &&
           ( mnWidth == static_cast<const MetaStretchTextAction&>(rMetaAction).mnWidth ) &&
           ( mnIndex == static_cast<const MetaStretchTextAction&>(rMetaAction).mnIndex ) &&
           ( mnLen == static_cast<const MetaStretchTextAction&>(rMetaAction).mnLen );
}

void MetaStretchTextAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);
    WritePair( rOStm, maPt );
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm.WriteUInt32( mnWidth );
    rOStm.WriteUInt16( mnIndex );
    rOStm.WriteUInt16( mnLen );

    write_uInt16_lenPrefixed_uInt16s_FromOUString(rOStm, maStr); // version 2
}

void MetaStretchTextAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
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
    MetaAction  ( MetaActionType::TEXTRECT ),
    mnStyle     ( 0 )
{}

MetaTextRectAction::~MetaTextRectAction()
{}

MetaTextRectAction::MetaTextRectAction( const Rectangle& rRect,
                                        const OUString& rStr, sal_uInt16 nStyle ) :
    MetaAction  ( MetaActionType::TEXTRECT ),
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
    return ( maRect == static_cast<const MetaTextRectAction&>(rMetaAction).maRect ) &&
           ( maStr == static_cast<const MetaTextRectAction&>(rMetaAction).maStr ) &&
           ( mnStyle == static_cast<const MetaTextRectAction&>(rMetaAction).mnStyle );
}

void MetaTextRectAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);
    WriteRectangle( rOStm, maRect );
    rOStm.WriteUniOrByteString( maStr, pData->meActualCharSet );
    rOStm.WriteUInt16( mnStyle );

    write_uInt16_lenPrefixed_uInt16s_FromOUString(rOStm, maStr); // version 2
}

void MetaTextRectAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRectangle( rIStm, maRect );
    maStr = rIStm.ReadUniOrByteString(pData->meActualCharSet);
    rIStm  .ReadUInt16( mnStyle );

    if ( aCompat.GetVersion() >= 2 )                            // Version 2
        maStr = read_uInt16_lenPrefixed_uInt16s_ToOUString(rIStm);
}

MetaTextLineAction::MetaTextLineAction() :
    MetaAction  ( MetaActionType::TEXTLINE ),
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
    MetaAction  ( MetaActionType::TEXTLINE ),
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
    return ( maPos == static_cast<const MetaTextLineAction&>(rMetaAction).maPos ) &&
           ( mnWidth == static_cast<const MetaTextLineAction&>(rMetaAction).mnWidth ) &&
           ( meStrikeout == static_cast<const MetaTextLineAction&>(rMetaAction).meStrikeout ) &&
           ( meUnderline == static_cast<const MetaTextLineAction&>(rMetaAction).meUnderline ) &&
           ( meOverline  == static_cast<const MetaTextLineAction&>(rMetaAction).meOverline );
}

void MetaTextLineAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 2);

    WritePair( rOStm, maPos );
    rOStm.WriteInt32( mnWidth );
    rOStm.WriteUInt32( meStrikeout );
    rOStm.WriteUInt32( meUnderline );
    // new in version 2
    rOStm.WriteUInt32( meOverline );
}

void MetaTextLineAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);

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
    MetaAction(MetaActionType::BMP)
{}

MetaBmpAction::~MetaBmpAction()
{}

MetaBmpAction::MetaBmpAction( const Point& rPt, const Bitmap& rBmp ) :
    MetaAction  ( MetaActionType::BMP ),
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
    return maBmp.IsEqual(static_cast<const MetaBmpAction&>(rMetaAction).maBmp ) &&
           ( maPt == static_cast<const MetaBmpAction&>(rMetaAction).maPt );
}

void MetaBmpAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maPt );
    }
}

void MetaBmpAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maPt );
}

MetaBmpScaleAction::MetaBmpScaleAction() :
    MetaAction(MetaActionType::BMPSCALE)
{}

MetaBmpScaleAction::~MetaBmpScaleAction()
{}

MetaBmpScaleAction::MetaBmpScaleAction( const Point& rPt, const Size& rSz,
                                        const Bitmap& rBmp ) :
    MetaAction  ( MetaActionType::BMPSCALE ),
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
    return ( maBmp.IsEqual(static_cast<const MetaBmpScaleAction&>(rMetaAction).maBmp )) &&
           ( maPt == static_cast<const MetaBmpScaleAction&>(rMetaAction).maPt ) &&
           ( maSz == static_cast<const MetaBmpScaleAction&>(rMetaAction).maSz );
}

void MetaBmpScaleAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maPt );
        WritePair( rOStm, maSz );
    }
}

void MetaBmpScaleAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maPt );
    ReadPair( rIStm, maSz );
}

MetaBmpScalePartAction::MetaBmpScalePartAction() :
    MetaAction(MetaActionType::BMPSCALEPART)
{}

MetaBmpScalePartAction::~MetaBmpScalePartAction()
{}

MetaBmpScalePartAction::MetaBmpScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                const Point& rSrcPt, const Size& rSrcSz,
                                                const Bitmap& rBmp ) :
    MetaAction  ( MetaActionType::BMPSCALEPART ),
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
    return ( maBmp.IsEqual(static_cast<const MetaBmpScalePartAction&>(rMetaAction).maBmp )) &&
           ( maDstPt == static_cast<const MetaBmpScalePartAction&>(rMetaAction).maDstPt ) &&
           ( maDstSz == static_cast<const MetaBmpScalePartAction&>(rMetaAction).maDstSz ) &&
           ( maSrcPt == static_cast<const MetaBmpScalePartAction&>(rMetaAction).maSrcPt ) &&
           ( maSrcSz == static_cast<const MetaBmpScalePartAction&>(rMetaAction).maSrcSz );
}

void MetaBmpScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maDstPt );
        WritePair( rOStm, maDstSz );
        WritePair( rOStm, maSrcPt );
        WritePair( rOStm, maSrcSz );
    }
}

void MetaBmpScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maDstPt );
    ReadPair( rIStm, maDstSz );
    ReadPair( rIStm, maSrcPt );
    ReadPair( rIStm, maSrcSz );
}

MetaBmpExAction::MetaBmpExAction() :
    MetaAction(MetaActionType::BMPEX)
{}

MetaBmpExAction::~MetaBmpExAction()
{}

MetaBmpExAction::MetaBmpExAction( const Point& rPt, const BitmapEx& rBmpEx ) :
    MetaAction  ( MetaActionType::BMPEX ),
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
    return ( maBmpEx.IsEqual(static_cast<const MetaBmpExAction&>(rMetaAction).maBmpEx )) &&
           ( maPt == static_cast<const MetaBmpExAction&>(rMetaAction).maPt );
}

void MetaBmpExAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
        WriteDIBBitmapEx(maBmpEx, rOStm);
        WritePair( rOStm, maPt );
    }
}

void MetaBmpExAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIBBitmapEx(maBmpEx, rIStm);
    ReadPair( rIStm, maPt );
}

MetaBmpExScaleAction::MetaBmpExScaleAction() :
    MetaAction(MetaActionType::BMPEXSCALE)
{}

MetaBmpExScaleAction::~MetaBmpExScaleAction()
{}

MetaBmpExScaleAction::MetaBmpExScaleAction( const Point& rPt, const Size& rSz,
                                            const BitmapEx& rBmpEx ) :
    MetaAction  ( MetaActionType::BMPEXSCALE ),
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
    return ( maBmpEx.IsEqual(static_cast<const MetaBmpExScaleAction&>(rMetaAction).maBmpEx )) &&
           ( maPt == static_cast<const MetaBmpExScaleAction&>(rMetaAction).maPt ) &&
           ( maSz == static_cast<const MetaBmpExScaleAction&>(rMetaAction).maSz );
}

void MetaBmpExScaleAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
        WriteDIBBitmapEx(maBmpEx, rOStm);
        WritePair( rOStm, maPt );
        WritePair( rOStm, maSz );
    }
}

void MetaBmpExScaleAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIBBitmapEx(maBmpEx, rIStm);
    ReadPair( rIStm, maPt );
    ReadPair( rIStm, maSz );
}

MetaBmpExScalePartAction::MetaBmpExScalePartAction() :
    MetaAction(MetaActionType::BMPEXSCALEPART)
{}

MetaBmpExScalePartAction::~MetaBmpExScalePartAction()
{}

MetaBmpExScalePartAction::MetaBmpExScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                    const Point& rSrcPt, const Size& rSrcSz,
                                                    const BitmapEx& rBmpEx ) :
    MetaAction  ( MetaActionType::BMPEXSCALEPART ),
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
    return ( maBmpEx.IsEqual(static_cast<const MetaBmpExScalePartAction&>(rMetaAction).maBmpEx )) &&
           ( maDstPt == static_cast<const MetaBmpExScalePartAction&>(rMetaAction).maDstPt ) &&
           ( maDstSz == static_cast<const MetaBmpExScalePartAction&>(rMetaAction).maDstSz ) &&
           ( maSrcPt == static_cast<const MetaBmpExScalePartAction&>(rMetaAction).maSrcPt ) &&
           ( maSrcSz == static_cast<const MetaBmpExScalePartAction&>(rMetaAction).maSrcSz );
}

void MetaBmpExScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmpEx.GetBitmap() )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
        WriteDIBBitmapEx(maBmpEx, rOStm);
        WritePair( rOStm, maDstPt );
        WritePair( rOStm, maDstSz );
        WritePair( rOStm, maSrcPt );
        WritePair( rOStm, maSrcSz );
    }
}

void MetaBmpExScalePartAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIBBitmapEx(maBmpEx, rIStm);
    ReadPair( rIStm, maDstPt );
    ReadPair( rIStm, maDstSz );
    ReadPair( rIStm, maSrcPt );
    ReadPair( rIStm, maSrcSz );
}

MetaMaskAction::MetaMaskAction() :
    MetaAction(MetaActionType::MASK)
{}

MetaMaskAction::~MetaMaskAction()
{}

MetaMaskAction::MetaMaskAction( const Point& rPt,
                                const Bitmap& rBmp,
                                const Color& rColor ) :
    MetaAction  ( MetaActionType::MASK ),
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
    return ( maBmp.IsEqual(static_cast<const MetaMaskAction&>(rMetaAction).maBmp )) &&
           ( maColor == static_cast<const MetaMaskAction&>(rMetaAction).maColor ) &&
           ( maPt == static_cast<const MetaMaskAction&>(rMetaAction).maPt );
}

void MetaMaskAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maPt );
    }
}

void MetaMaskAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maPt );
}

MetaMaskScaleAction::MetaMaskScaleAction() :
    MetaAction(MetaActionType::MASKSCALE)
{}

MetaMaskScaleAction::~MetaMaskScaleAction()
{}

MetaMaskScaleAction::MetaMaskScaleAction( const Point& rPt, const Size& rSz,
                                          const Bitmap& rBmp,
                                          const Color& rColor ) :
    MetaAction  ( MetaActionType::MASKSCALE ),
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
    return ( maBmp.IsEqual(static_cast<const MetaMaskScaleAction&>(rMetaAction).maBmp )) &&
           ( maColor == static_cast<const MetaMaskScaleAction&>(rMetaAction).maColor ) &&
           ( maPt == static_cast<const MetaMaskScaleAction&>(rMetaAction).maPt ) &&
           ( maSz == static_cast<const MetaMaskScaleAction&>(rMetaAction).maSz );
}

void MetaMaskScaleAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
        WriteDIB(maBmp, rOStm, false, true);
        WritePair( rOStm, maPt );
        WritePair( rOStm, maSz );
    }
}

void MetaMaskScaleAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIB(maBmp, rIStm, true);
    ReadPair( rIStm, maPt );
    ReadPair( rIStm, maSz );
}

MetaMaskScalePartAction::MetaMaskScalePartAction() :
    MetaAction(MetaActionType::MASKSCALEPART)
{}

MetaMaskScalePartAction::~MetaMaskScalePartAction()
{}

MetaMaskScalePartAction::MetaMaskScalePartAction( const Point& rDstPt, const Size& rDstSz,
                                                  const Point& rSrcPt, const Size& rSrcSz,
                                                  const Bitmap& rBmp,
                                                  const Color& rColor ) :
    MetaAction  ( MetaActionType::MASKSCALEPART ),
    maBmp       ( rBmp ),
    maColor     ( rColor ),
    maDstPt     ( rDstPt ),
    maDstSz     ( rDstSz ),
    maSrcPt     ( rSrcPt ),
    maSrcSz     ( rSrcSz )
{}

void MetaMaskScalePartAction::Execute( OutputDevice* pOut )
{
    pOut->DrawMask( maDstPt, maDstSz, maSrcPt, maSrcSz, maBmp, maColor, MetaActionType::MASKSCALE );
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
    return ( maBmp.IsEqual(static_cast<const MetaMaskScalePartAction&>(rMetaAction).maBmp )) &&
           ( maColor == static_cast<const MetaMaskScalePartAction&>(rMetaAction).maColor ) &&
           ( maDstPt == static_cast<const MetaMaskScalePartAction&>(rMetaAction).maDstPt ) &&
           ( maDstSz == static_cast<const MetaMaskScalePartAction&>(rMetaAction).maDstSz ) &&
           ( maSrcPt == static_cast<const MetaMaskScalePartAction&>(rMetaAction).maSrcPt ) &&
           ( maSrcSz == static_cast<const MetaMaskScalePartAction&>(rMetaAction).maSrcSz );
}

void MetaMaskScalePartAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    if( !!maBmp )
    {
        MetaAction::Write(rOStm, pData);
        VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
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
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadDIB(maBmp, rIStm, true);
    maColor.Read( rIStm, true );
    ReadPair( rIStm, maDstPt );
    ReadPair( rIStm, maDstSz );
    ReadPair( rIStm, maSrcPt );
    ReadPair( rIStm, maSrcSz );
}

MetaGradientAction::MetaGradientAction() :
    MetaAction(MetaActionType::GRADIENT)
{}

MetaGradientAction::~MetaGradientAction()
{}

MetaGradientAction::MetaGradientAction( const Rectangle& rRect, const Gradient& rGradient ) :
    MetaAction  ( MetaActionType::GRADIENT ),
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
    return ( maRect == static_cast<const MetaGradientAction&>(rMetaAction).maRect ) &&
           ( maGradient == static_cast<const MetaGradientAction&>(rMetaAction).maGradient );
}

void MetaGradientAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteRectangle( rOStm, maRect );
    WriteGradient( rOStm, maGradient );
}

void MetaGradientAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRectangle( rIStm, maRect );
    ReadGradient( rIStm, maGradient );
}

MetaGradientExAction::MetaGradientExAction() :
    MetaAction  ( MetaActionType::GRADIENTEX )
{}

MetaGradientExAction::MetaGradientExAction( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient ) :
    MetaAction  ( MetaActionType::GRADIENTEX ),
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
    return ( maPolyPoly == static_cast<const MetaGradientExAction&>(rMetaAction).maPolyPoly ) &&
           ( maGradient == static_cast<const MetaGradientExAction&>(rMetaAction).maGradient );
}

void MetaGradientExAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    // #i105373# see comment at MetaTransparentAction::Write
    tools::PolyPolygon aNoCurvePolyPolygon;
    maPolyPoly.AdaptiveSubdivide(aNoCurvePolyPolygon);

    WritePolyPolygon( rOStm, aNoCurvePolyPolygon );
    WriteGradient( rOStm, maGradient );
}

void MetaGradientExAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadPolyPolygon( rIStm, maPolyPoly );
    ReadGradient( rIStm, maGradient );
}

MetaHatchAction::MetaHatchAction() :
    MetaAction(MetaActionType::HATCH)
{}

MetaHatchAction::~MetaHatchAction()
{}

MetaHatchAction::MetaHatchAction( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch ) :
    MetaAction  ( MetaActionType::HATCH ),
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
    return ( maPolyPoly == static_cast<const MetaHatchAction&>(rMetaAction).maPolyPoly ) &&
           ( maHatch == static_cast<const MetaHatchAction&>(rMetaAction).maHatch );
}

void MetaHatchAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    // #i105373# see comment at MetaTransparentAction::Write
    tools::PolyPolygon aNoCurvePolyPolygon;
    maPolyPoly.AdaptiveSubdivide(aNoCurvePolyPolygon);

    WritePolyPolygon( rOStm, aNoCurvePolyPolygon );
    WriteHatch( rOStm, maHatch );
}

void MetaHatchAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadPolyPolygon( rIStm, maPolyPoly );
    ReadHatch( rIStm, maHatch );
}

MetaWallpaperAction::MetaWallpaperAction() :
    MetaAction(MetaActionType::WALLPAPER)
{}

MetaWallpaperAction::~MetaWallpaperAction()
{}

MetaWallpaperAction::MetaWallpaperAction( const Rectangle& rRect,
                                          const Wallpaper& rPaper ) :
    MetaAction  ( MetaActionType::WALLPAPER ),
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
    return ( maRect == static_cast<const MetaWallpaperAction&>(rMetaAction).maRect ) &&
           ( maWallpaper == static_cast<const MetaWallpaperAction&>(rMetaAction).maWallpaper );
}

void MetaWallpaperAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    WriteWallpaper( rOStm, maWallpaper );
}

void MetaWallpaperAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadWallpaper( rIStm, maWallpaper );
}

MetaClipRegionAction::MetaClipRegionAction() :
    MetaAction  ( MetaActionType::CLIPREGION ),
    mbClip      ( false )
{}

MetaClipRegionAction::~MetaClipRegionAction()
{}

MetaClipRegionAction::MetaClipRegionAction( const vcl::Region& rRegion, bool bClip ) :
    MetaAction  ( MetaActionType::CLIPREGION ),
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
    return ( maRegion == static_cast<const MetaClipRegionAction&>(rMetaAction).maRegion ) &&
           ( mbClip == static_cast<const MetaClipRegionAction&>(rMetaAction).mbClip );
}

void MetaClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    WriteRegion( rOStm, maRegion );
    rOStm.WriteBool( mbClip );
}

void MetaClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRegion( rIStm, maRegion );
    rIStm.ReadCharAsBool( mbClip );
}

MetaISectRectClipRegionAction::MetaISectRectClipRegionAction() :
    MetaAction(MetaActionType::ISECTRECTCLIPREGION)
{}

MetaISectRectClipRegionAction::~MetaISectRectClipRegionAction()
{}

MetaISectRectClipRegionAction::MetaISectRectClipRegionAction( const Rectangle& rRect ) :
    MetaAction  ( MetaActionType::ISECTRECTCLIPREGION ),
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
    return maRect == static_cast<const MetaISectRectClipRegionAction&>(rMetaAction).maRect;
}

void MetaISectRectClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteRectangle( rOStm, maRect );
}

void MetaISectRectClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRectangle( rIStm, maRect );
}

MetaISectRegionClipRegionAction::MetaISectRegionClipRegionAction() :
    MetaAction(MetaActionType::ISECTREGIONCLIPREGION)
{}

MetaISectRegionClipRegionAction::~MetaISectRegionClipRegionAction()
{}

MetaISectRegionClipRegionAction::MetaISectRegionClipRegionAction( const vcl::Region& rRegion ) :
    MetaAction  ( MetaActionType::ISECTREGIONCLIPREGION ),
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
    return maRegion == static_cast<const MetaISectRegionClipRegionAction&>(rMetaAction).maRegion;
}

void MetaISectRegionClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteRegion( rOStm, maRegion );
}

void MetaISectRegionClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadRegion( rIStm, maRegion );
}

MetaMoveClipRegionAction::MetaMoveClipRegionAction() :
    MetaAction  ( MetaActionType::MOVECLIPREGION ),
    mnHorzMove  ( 0 ),
    mnVertMove  ( 0 )
{}

MetaMoveClipRegionAction::~MetaMoveClipRegionAction()
{}

MetaMoveClipRegionAction::MetaMoveClipRegionAction( long nHorzMove, long nVertMove ) :
    MetaAction  ( MetaActionType::MOVECLIPREGION ),
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
    return ( mnHorzMove == static_cast<const MetaMoveClipRegionAction&>(rMetaAction).mnHorzMove ) &&
           ( mnVertMove == static_cast<const MetaMoveClipRegionAction&>(rMetaAction).mnVertMove );
}

void MetaMoveClipRegionAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    rOStm.WriteInt32( mnHorzMove ).WriteInt32( mnVertMove );
}

void MetaMoveClipRegionAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    sal_Int32 nTmpHM(0), nTmpVM(0);
    rIStm.ReadInt32( nTmpHM ).ReadInt32( nTmpVM );
    mnHorzMove = nTmpHM;
    mnVertMove = nTmpVM;
}

MetaLineColorAction::MetaLineColorAction() :
    MetaAction  ( MetaActionType::LINECOLOR ),
    mbSet       ( false )
{}

MetaLineColorAction::~MetaLineColorAction()
{}

MetaLineColorAction::MetaLineColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( MetaActionType::LINECOLOR ),
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
    return ( maColor == static_cast<const MetaLineColorAction&>(rMetaAction).maColor ) &&
           ( mbSet == static_cast<const MetaLineColorAction&>(rMetaAction).mbSet );
}

void MetaLineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteBool( mbSet );
}

void MetaLineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaFillColorAction::MetaFillColorAction() :
    MetaAction  ( MetaActionType::FILLCOLOR ),
    mbSet       ( false )
{}

MetaFillColorAction::~MetaFillColorAction()
{}

MetaFillColorAction::MetaFillColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( MetaActionType::FILLCOLOR ),
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
    return ( maColor == static_cast<const MetaFillColorAction&>(rMetaAction).maColor ) &&
           ( mbSet == static_cast<const MetaFillColorAction&>(rMetaAction).mbSet );
}

void MetaFillColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteBool( mbSet );
}

void MetaFillColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaTextColorAction::MetaTextColorAction() :
    MetaAction(MetaActionType::TEXTCOLOR)
{}

MetaTextColorAction::~MetaTextColorAction()
{}

MetaTextColorAction::MetaTextColorAction( const Color& rColor ) :
    MetaAction  ( MetaActionType::TEXTCOLOR ),
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
    return maColor == static_cast<const MetaTextColorAction&>(rMetaAction).maColor;
}

void MetaTextColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    maColor.Write( rOStm, true );
}

void MetaTextColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    maColor.Read( rIStm, true );
}

MetaTextFillColorAction::MetaTextFillColorAction() :
    MetaAction  ( MetaActionType::TEXTFILLCOLOR ),
    mbSet       ( false )
{}

MetaTextFillColorAction::~MetaTextFillColorAction()
{}

MetaTextFillColorAction::MetaTextFillColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( MetaActionType::TEXTFILLCOLOR ),
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
    return ( maColor == static_cast<const MetaTextFillColorAction&>(rMetaAction).maColor ) &&
           ( mbSet == static_cast<const MetaTextFillColorAction&>(rMetaAction).mbSet );
}

void MetaTextFillColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteBool( mbSet );
}

void MetaTextFillColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaTextLineColorAction::MetaTextLineColorAction() :
    MetaAction  ( MetaActionType::TEXTLINECOLOR ),
    mbSet       ( false )
{}

MetaTextLineColorAction::~MetaTextLineColorAction()
{}

MetaTextLineColorAction::MetaTextLineColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( MetaActionType::TEXTLINECOLOR ),
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
    return ( maColor == static_cast<const MetaTextLineColorAction&>(rMetaAction).maColor ) &&
           ( mbSet == static_cast<const MetaTextLineColorAction&>(rMetaAction).mbSet );
}

void MetaTextLineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteBool( mbSet );
}

void MetaTextLineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaOverlineColorAction::MetaOverlineColorAction() :
    MetaAction  ( MetaActionType::OVERLINECOLOR ),
    mbSet       ( false )
{}

MetaOverlineColorAction::~MetaOverlineColorAction()
{}

MetaOverlineColorAction::MetaOverlineColorAction( const Color& rColor, bool bSet ) :
    MetaAction  ( MetaActionType::OVERLINECOLOR ),
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
    return ( maColor == static_cast<const MetaOverlineColorAction&>(rMetaAction).maColor ) &&
           ( mbSet == static_cast<const MetaOverlineColorAction&>(rMetaAction).mbSet );
}

void MetaOverlineColorAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    maColor.Write( rOStm, true );
    rOStm.WriteBool( mbSet );
}

void MetaOverlineColorAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    maColor.Read( rIStm, true );
    rIStm.ReadCharAsBool( mbSet );
}

MetaTextAlignAction::MetaTextAlignAction() :
    MetaAction  ( MetaActionType::TEXTALIGN ),
    maAlign     ( ALIGN_TOP )
{}

MetaTextAlignAction::~MetaTextAlignAction()
{}

MetaTextAlignAction::MetaTextAlignAction( TextAlign aAlign ) :
    MetaAction  ( MetaActionType::TEXTALIGN ),
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
    return maAlign == static_cast<const MetaTextAlignAction&>(rMetaAction).maAlign;
}

void MetaTextAlignAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    rOStm.WriteUInt16( maAlign );
}

void MetaTextAlignAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    sal_uInt16 nTmp16(0);

    VersionCompat aCompat(rIStm, StreamMode::READ);
    rIStm.ReadUInt16( nTmp16 ); maAlign = (TextAlign) nTmp16;
}

MetaMapModeAction::MetaMapModeAction() :
    MetaAction(MetaActionType::MAPMODE)
{}

MetaMapModeAction::~MetaMapModeAction()
{}

MetaMapModeAction::MetaMapModeAction( const MapMode& rMapMode ) :
    MetaAction  ( MetaActionType::MAPMODE ),
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
    return maMapMode == static_cast<const MetaMapModeAction&>(rMetaAction).maMapMode;
}

void MetaMapModeAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteMapMode( rOStm, maMapMode );
}

void MetaMapModeAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadMapMode( rIStm, maMapMode );
}

MetaFontAction::MetaFontAction() :
    MetaAction(MetaActionType::FONT)
{}

MetaFontAction::~MetaFontAction()
{}

MetaFontAction::MetaFontAction( const vcl::Font& rFont ) :
    MetaAction  ( MetaActionType::FONT ),
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
    return maFont == static_cast<const MetaFontAction&>(rMetaAction).maFont;
}

void MetaFontAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    WriteFont( rOStm, maFont );
    pData->meActualCharSet = maFont.GetCharSet();
    if ( pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW )
        pData->meActualCharSet = osl_getThreadTextEncoding();
}

void MetaFontAction::Read( SvStream& rIStm, ImplMetaReadData* pData )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadFont( rIStm, maFont );
    pData->meActualCharSet = maFont.GetCharSet();
    if ( pData->meActualCharSet == RTL_TEXTENCODING_DONTKNOW )
        pData->meActualCharSet = osl_getThreadTextEncoding();
}

MetaPushAction::MetaPushAction() :
    MetaAction  ( MetaActionType::PUSH ),
    mnFlags     ( PushFlags::NONE )
{}

MetaPushAction::~MetaPushAction()
{}

MetaPushAction::MetaPushAction( PushFlags nFlags ) :
    MetaAction  ( MetaActionType::PUSH ),
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
    return mnFlags == static_cast<const MetaPushAction&>(rMetaAction).mnFlags;
}

void MetaPushAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    rOStm.WriteUInt16( static_cast<sal_uInt16>(mnFlags) );
}

void MetaPushAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    sal_uInt16 tmp;
    rIStm.ReadUInt16( tmp );
    mnFlags = static_cast<PushFlags>(tmp);
}

MetaPopAction::MetaPopAction() :
    MetaAction(MetaActionType::POP)
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
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
}

void MetaPopAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
}

MetaRasterOpAction::MetaRasterOpAction() :
    MetaAction  ( MetaActionType::RASTEROP ),
    meRasterOp  ( ROP_OVERPAINT )
{}

MetaRasterOpAction::~MetaRasterOpAction()
{}

MetaRasterOpAction::MetaRasterOpAction( RasterOp eRasterOp ) :
    MetaAction  ( MetaActionType::RASTEROP ),
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
    return meRasterOp == static_cast<const MetaRasterOpAction&>(rMetaAction).meRasterOp;
}

void MetaRasterOpAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    rOStm.WriteUInt16( meRasterOp );
}

void MetaRasterOpAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    sal_uInt16 nTmp16(0);

    VersionCompat aCompat(rIStm, StreamMode::READ);
    rIStm.ReadUInt16( nTmp16 ); meRasterOp = (RasterOp) nTmp16;
}

MetaTransparentAction::MetaTransparentAction() :
    MetaAction      ( MetaActionType::TRANSPARENT ),
    mnTransPercent  ( 0 )
{}

MetaTransparentAction::~MetaTransparentAction()
{}

MetaTransparentAction::MetaTransparentAction( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransPercent ) :
    MetaAction      ( MetaActionType::TRANSPARENT ),
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
    return ( maPolyPoly == static_cast<const MetaTransparentAction&>(rMetaAction).maPolyPoly ) &&
           ( mnTransPercent == static_cast<const MetaTransparentAction&>(rMetaAction).mnTransPercent );
}

void MetaTransparentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    // #i105373# The tools::PolyPolygon in this action may be a curve; this
    // was ignored until now what is an error. To make older office
    // versions work with MetaFiles, i opt for applying AdaptiveSubdivide
    // to the PolyPoylgon.
    // The alternative would be to really write the curve information
    // like in MetaPolyPolygonAction::Write (where someone extended it
    // correctly, but not here :-( ).
    // The golden solution would be to combine both, but i think it's
    // not necessary; a good subdivision will be sufficient.
    tools::PolyPolygon aNoCurvePolyPolygon;
    maPolyPoly.AdaptiveSubdivide(aNoCurvePolyPolygon);

    WritePolyPolygon( rOStm, aNoCurvePolyPolygon );
    rOStm.WriteUInt16( mnTransPercent );
}

void MetaTransparentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadPolyPolygon( rIStm, maPolyPoly );
    rIStm.ReadUInt16( mnTransPercent );
}

MetaFloatTransparentAction::MetaFloatTransparentAction() :
    MetaAction(MetaActionType::FLOATTRANSPARENT)
{}

MetaFloatTransparentAction::~MetaFloatTransparentAction()
{}

MetaFloatTransparentAction::MetaFloatTransparentAction( const GDIMetaFile& rMtf, const Point& rPos,
                                                        const Size& rSize, const Gradient& rGradient ) :
    MetaAction      ( MetaActionType::FLOATTRANSPARENT ),
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
    return ( maMtf == static_cast<const MetaFloatTransparentAction&>(rMetaAction).maMtf ) &&
           ( maPoint == static_cast<const MetaFloatTransparentAction&>(rMetaAction).maPoint ) &&
           ( maSize == static_cast<const MetaFloatTransparentAction&>(rMetaAction).maSize ) &&
           ( maGradient == static_cast<const MetaFloatTransparentAction&>(rMetaAction).maGradient );
}

void MetaFloatTransparentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    maMtf.Write( rOStm );
    WritePair( rOStm,  maPoint );
    WritePair( rOStm, maSize );
    WriteGradient( rOStm, maGradient );
}

void MetaFloatTransparentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadGDIMetaFile( rIStm, maMtf );
    ReadPair( rIStm, maPoint );
    ReadPair( rIStm, maSize );
    ReadGradient( rIStm, maGradient );
}

MetaEPSAction::MetaEPSAction() :
    MetaAction(MetaActionType::EPS)
{}

MetaEPSAction::~MetaEPSAction()
{}

MetaEPSAction::MetaEPSAction( const Point& rPoint, const Size& rSize,
                              const GfxLink& rGfxLink, const GDIMetaFile& rSubst ) :
    MetaAction  ( MetaActionType::EPS ),
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
    return ( maGfxLink.IsEqual(static_cast<const MetaEPSAction&>(rMetaAction).maGfxLink )) &&
           ( maSubst == static_cast<const MetaEPSAction&>(rMetaAction).maSubst ) &&
           ( maPoint == static_cast<const MetaEPSAction&>(rMetaAction).maPoint ) &&
           ( maSize == static_cast<const MetaEPSAction&>(rMetaAction).maSize );
}

void MetaEPSAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    WriteGfxLink( rOStm, maGfxLink );
    WritePair( rOStm, maPoint );
    WritePair( rOStm, maSize );
    maSubst.Write( rOStm );
}

void MetaEPSAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadGfxLink( rIStm, maGfxLink );
    ReadPair( rIStm, maPoint );
    ReadPair( rIStm, maSize );
    ReadGDIMetaFile( rIStm, maSubst );
}

MetaRefPointAction::MetaRefPointAction() :
    MetaAction  ( MetaActionType::REFPOINT ),
    mbSet       ( false )
{}

MetaRefPointAction::~MetaRefPointAction()
{}

MetaRefPointAction::MetaRefPointAction( const Point& rRefPoint, bool bSet ) :
    MetaAction  ( MetaActionType::REFPOINT ),
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
    return ( maRefPoint == static_cast<const MetaRefPointAction&>(rMetaAction).maRefPoint ) &&
           ( mbSet == static_cast<const MetaRefPointAction&>(rMetaAction).mbSet );
}

void MetaRefPointAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);

    WritePair( rOStm, maRefPoint );
    rOStm.WriteBool( mbSet );
}

void MetaRefPointAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    ReadPair( rIStm, maRefPoint ).ReadCharAsBool( mbSet );
}

MetaCommentAction::MetaCommentAction( sal_Int32 nValue ) :
    MetaAction  ( MetaActionType::COMMENT ),
    mnValue     ( nValue )
{
    ImplInitDynamicData( NULL, 0UL );
}

MetaCommentAction::MetaCommentAction( const MetaCommentAction& rAct ) :
    MetaAction  ( MetaActionType::COMMENT ),
    maComment   ( rAct.maComment ),
    mnValue     ( rAct.mnValue )
{
    ImplInitDynamicData( rAct.mpData, rAct.mnDataSize );
}

MetaCommentAction::MetaCommentAction( const OString& rComment, sal_Int32 nValue, const sal_uInt8* pData, sal_uInt32 nDataSize ) :
    MetaAction  ( MetaActionType::COMMENT ),
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
                SvMemoryStream  aMemStm( (void*)mpData, mnDataSize, StreamMode::READ );
                SvMemoryStream  aDest;
                if ( bPathStroke )
                {
                    SvtGraphicStroke aStroke;
                    ReadSvtGraphicStroke( aMemStm, aStroke );

                    Polygon aPath;
                    aStroke.getPath( aPath );
                    aPath.Move( nXMove, nYMove );
                    aStroke.setPath( aPath );

                    tools::PolyPolygon aStartArrow;
                    aStroke.getStartArrow(aStartArrow);
                    aStartArrow.Move(nXMove, nYMove);
                    aStroke.setStartArrow(aStartArrow);

                    tools::PolyPolygon aEndArrow;
                    aStroke.getEndArrow(aEndArrow);
                    aEndArrow.Move(nXMove, nYMove);
                    aStroke.setEndArrow(aEndArrow);

                    WriteSvtGraphicStroke( aDest, aStroke );
                }
                else
                {
                    SvtGraphicFill aFill;
                    ReadSvtGraphicFill( aMemStm, aFill );

                    tools::PolyPolygon aPath;
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
                SvMemoryStream  aMemStm( (void*)mpData, mnDataSize, StreamMode::READ );
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
                    tools::PolyPolygon aPath;
                    aFill.getPath( aPath );
                    aPath.Scale( fXScale, fYScale );
                    aFill.setPath( aPath );
                    WriteSvtGraphicFill( aDest, aFill );
                }
                delete[] mpData;
                ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
            } else if( maComment == "EMF_PLUS_HEADER_INFO" ){
                SvMemoryStream  aMemStm( (void*)mpData, mnDataSize, StreamMode::READ );
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
    return ( maComment == static_cast<const MetaCommentAction&>(rMetaAction).maComment ) &&
           ( mnValue == static_cast<const MetaCommentAction&>(rMetaAction).mnValue ) &&
           ( mnDataSize == static_cast<const MetaCommentAction&>(rMetaAction).mnDataSize ) &&
           ( memcmp( mpData, static_cast<const MetaCommentAction&>(rMetaAction).mpData, mnDataSize ) == 0 );
}

void MetaCommentAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    write_uInt16_lenPrefixed_uInt8s_FromOString(rOStm, maComment);
    rOStm.WriteInt32( mnValue ).WriteUInt32( mnDataSize );

    if ( mnDataSize )
        rOStm.Write( mpData, mnDataSize );
}

void MetaCommentAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
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
    MetaAction  ( MetaActionType::LAYOUTMODE ),
    mnLayoutMode( TEXT_LAYOUT_DEFAULT )
{}

MetaLayoutModeAction::~MetaLayoutModeAction()
{}

MetaLayoutModeAction::MetaLayoutModeAction( ComplexTextLayoutMode nLayoutMode ) :
    MetaAction  ( MetaActionType::LAYOUTMODE ),
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
    return mnLayoutMode == static_cast<const MetaLayoutModeAction&>(rMetaAction).mnLayoutMode;
}

void MetaLayoutModeAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    rOStm.WriteUInt32( mnLayoutMode );
}

void MetaLayoutModeAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    sal_uInt32 tmp;
    rIStm.ReadUInt32( tmp );
    mnLayoutMode = static_cast<ComplexTextLayoutMode>(tmp);
}

MetaTextLanguageAction::MetaTextLanguageAction() :
    MetaAction  ( MetaActionType::TEXTLANGUAGE ),
    meTextLanguage( LANGUAGE_DONTKNOW )
{}

MetaTextLanguageAction::~MetaTextLanguageAction()
{}

MetaTextLanguageAction::MetaTextLanguageAction( LanguageType eTextLanguage ) :
    MetaAction  ( MetaActionType::TEXTLANGUAGE ),
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
    return meTextLanguage == static_cast<const MetaTextLanguageAction&>(rMetaAction).meTextLanguage;
}

void MetaTextLanguageAction::Write( SvStream& rOStm, ImplMetaWriteData* pData )
{
    MetaAction::Write(rOStm, pData);
    VersionCompat aCompat(rOStm, StreamMode::WRITE, 1);
    rOStm.WriteUInt16( meTextLanguage );
}

void MetaTextLanguageAction::Read( SvStream& rIStm, ImplMetaReadData* )
{
    VersionCompat aCompat(rIStm, StreamMode::READ);
    rIStm.ReadUInt16( meTextLanguage );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
