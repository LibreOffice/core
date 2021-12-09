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

#include <stdio.h>
#include <string.h>
#include <osl/thread.h>
#include <sal/log.hxx>
#include <tools/stream.hxx>
#include <tools/vcompat.hxx>
#include <tools/helpers.hxx>
#include <vcl/dibtools.hxx>
#include <vcl/filter/SvmReader.hxx>
#include <vcl/filter/SvmWriter.hxx>
#include <vcl/outdev.hxx>
#include <vcl/metaact.hxx>
#include <vcl/graphictools.hxx>
#include <unotools/fontdefs.hxx>
#include <vcl/TypeSerializer.hxx>

namespace
{

void ImplScalePoint( Point& rPt, double fScaleX, double fScaleY )
{
    rPt.setX( FRound( fScaleX * rPt.X() ) );
    rPt.setY( FRound( fScaleY * rPt.Y() ) );
}

void ImplScaleRect( tools::Rectangle& rRect, double fScaleX, double fScaleY )
{
    Point aTL( rRect.TopLeft() );
    Point aBR( rRect.BottomRight() );

    ImplScalePoint( aTL, fScaleX, fScaleY );
    ImplScalePoint( aBR, fScaleX, fScaleY );

    rRect = tools::Rectangle( aTL, aBR );
    rRect.Justify();
}

void ImplScalePoly( tools::Polygon& rPoly, double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = rPoly.GetSize(); i < nCount; i++ )
        ImplScalePoint( rPoly[ i ], fScaleX, fScaleY );
}

void ImplScaleLineInfo( LineInfo& rLineInfo, double fScaleX, double fScaleY )
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
    mnType( MetaActionType::NONE )
{
}

MetaAction::MetaAction( MetaActionType nType ) :
    mnType( nType )
{
}

MetaAction::MetaAction( MetaAction const & rOther ) :
    SimpleReferenceObject(), mnType( rOther.mnType )
{
}

MetaAction::~MetaAction()
{
}

void MetaAction::Execute( OutputDevice* )
{
}

rtl::Reference<MetaAction> MetaAction::Clone() const
{
    return new MetaAction;
}

void MetaAction::Move( tools::Long, tools::Long )
{
}

void MetaAction::Scale( double, double )
{
}

void MetaAction::ReadFillMode(SvStream& rIStm, PolyFillMode &rFillMode)
{
    sal_uInt32 nTmp;
    rIStm.ReadUInt32(nTmp);
    rFillMode = PolyFillMode(nTmp);
}

void MetaAction::WriteFillMode(SvStream& rIStm, PolyFillMode eFillMode)
{
    rIStm.WriteUInt32(static_cast<sal_uInt32>(eFillMode));
}

void MetaAction::ReadColor(SvStream& rIStm, ::Color& rColor)
{
    sal_uInt32 nTmp;
    rIStm.ReadUInt32(nTmp);
    rColor = ::Color(ColorTransparency, nTmp);
}

void MetaAction::WriteColor(SvStream& rIStm, ::Color aColor)
{
    rIStm.WriteUInt32(static_cast<sal_uInt32>(aColor));
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

rtl::Reference<MetaAction> MetaPixelAction::Clone() const
{
    return new MetaPixelAction( *this );
}

void MetaPixelAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaPixelAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaPointAction::Clone() const
{
    return new MetaPointAction( *this );
}

void MetaPointAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaPointAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaLineAction::Clone() const
{
    return new MetaLineAction( *this );
}

void MetaLineAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
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

MetaRectAction::MetaRectAction() :
    MetaAction(MetaActionType::RECT)
{}

MetaRectAction::~MetaRectAction()
{}

MetaRectAction::MetaRectAction( const tools::Rectangle& rRect ) :
    MetaAction  ( MetaActionType::RECT ),
    maRect      ( rRect )
{}

void MetaRectAction::Execute( OutputDevice* pOut )
{
    pOut->DrawRect( maRect );
}

rtl::Reference<MetaAction> MetaRectAction::Clone() const
{
    return new MetaRectAction( *this );
}

void MetaRectAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaRectAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

MetaRoundRectAction::MetaRoundRectAction() :
    MetaAction  ( MetaActionType::ROUNDRECT ),
    mnHorzRound ( 0 ),
    mnVertRound ( 0 )
{}

MetaRoundRectAction::~MetaRoundRectAction()
{}

MetaRoundRectAction::MetaRoundRectAction( const tools::Rectangle& rRect,
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

rtl::Reference<MetaAction> MetaRoundRectAction::Clone() const
{
    return new MetaRoundRectAction( *this );
}

void MetaRoundRectAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaRoundRectAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
    mnHorzRound = FRound( mnHorzRound * fabs(fScaleX) );
    mnVertRound = FRound( mnVertRound * fabs(fScaleY) );
}

MetaEllipseAction::MetaEllipseAction() :
    MetaAction(MetaActionType::ELLIPSE)
{}

MetaEllipseAction::~MetaEllipseAction()
{}

MetaEllipseAction::MetaEllipseAction( const tools::Rectangle& rRect ) :
    MetaAction  ( MetaActionType::ELLIPSE ),
    maRect      ( rRect )
{}

void MetaEllipseAction::Execute( OutputDevice* pOut )
{
    pOut->DrawEllipse( maRect );
}

rtl::Reference<MetaAction> MetaEllipseAction::Clone() const
{
    return new MetaEllipseAction( *this );
}

void MetaEllipseAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaEllipseAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

MetaArcAction::MetaArcAction() :
    MetaAction(MetaActionType::ARC)
{}

MetaArcAction::~MetaArcAction()
{}

MetaArcAction::MetaArcAction( const tools::Rectangle& rRect,
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

rtl::Reference<MetaAction> MetaArcAction::Clone() const
{
    return new MetaArcAction( *this );
}

void MetaArcAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
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

MetaPieAction::MetaPieAction() :
    MetaAction(MetaActionType::PIE)
{}

MetaPieAction::~MetaPieAction()
{}

MetaPieAction::MetaPieAction( const tools::Rectangle& rRect,
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

rtl::Reference<MetaAction> MetaPieAction::Clone() const
{
    return new MetaPieAction( *this );
}

void MetaPieAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
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

MetaChordAction::MetaChordAction() :
    MetaAction(MetaActionType::CHORD)
{}

MetaChordAction::~MetaChordAction()
{}

MetaChordAction::MetaChordAction( const tools::Rectangle& rRect,
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

rtl::Reference<MetaAction> MetaChordAction::Clone() const
{
    return new MetaChordAction( *this );
}

void MetaChordAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
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

MetaPolyLineAction::MetaPolyLineAction() :
    MetaAction(MetaActionType::POLYLINE)
{}

MetaPolyLineAction::~MetaPolyLineAction()
{}

MetaPolyLineAction::MetaPolyLineAction( const tools::Polygon& rPoly ) :
    MetaAction  ( MetaActionType::POLYLINE ),
    maPoly      ( rPoly )
{}

MetaPolyLineAction::MetaPolyLineAction( const tools::Polygon& rPoly, const LineInfo& rLineInfo ) :
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

rtl::Reference<MetaAction> MetaPolyLineAction::Clone() const
{
    return new MetaPolyLineAction( *this );
}

void MetaPolyLineAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPoly.Move( nHorzMove, nVertMove );
}

void MetaPolyLineAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoly( maPoly, fScaleX, fScaleY );
    ImplScaleLineInfo( maLineInfo, fScaleX, fScaleY );
}

MetaPolygonAction::MetaPolygonAction() :
    MetaAction(MetaActionType::POLYGON)
{}

MetaPolygonAction::~MetaPolygonAction()
{}

MetaPolygonAction::MetaPolygonAction( const tools::Polygon& rPoly ) :
    MetaAction  ( MetaActionType::POLYGON ),
    maPoly      ( rPoly )
{}

void MetaPolygonAction::Execute( OutputDevice* pOut )
{
    pOut->DrawPolygon( maPoly );
}

rtl::Reference<MetaAction> MetaPolygonAction::Clone() const
{
    return new MetaPolygonAction( *this );
}

void MetaPolygonAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPoly.Move( nHorzMove, nVertMove );
}

void MetaPolygonAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoly( maPoly, fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaPolyPolygonAction::Clone() const
{
    return new MetaPolyPolygonAction( *this );
}

void MetaPolyPolygonAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaPolyPolygonAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaTextAction::Clone() const
{
    return new MetaTextAction( *this );
}

void MetaTextAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaTextAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
}

MetaTextArrayAction::MetaTextArrayAction() :
    MetaAction  ( MetaActionType::TEXTARRAY ),
    mnIndex     ( 0 ),
    mnLen       ( 0 )
{}

MetaTextArrayAction::MetaTextArrayAction( const MetaTextArrayAction& rAction ) :
    MetaAction  ( MetaActionType::TEXTARRAY ),
    maStartPt   ( rAction.maStartPt ),
    maStr       ( rAction.maStr ),
    maDXAry     ( rAction.maDXAry ),
    mnIndex     ( rAction.mnIndex ),
    mnLen       ( rAction.mnLen )
{
}

MetaTextArrayAction::MetaTextArrayAction( const Point& rStartPt,
                                          const OUString& rStr,
                                          const std::vector<sal_Int32>& rDXAry,
                                          sal_Int32 nIndex,
                                          sal_Int32 nLen ) :
    MetaAction  ( MetaActionType::TEXTARRAY ),
    maStartPt   ( rStartPt ),
    maStr       ( rStr ),
    maDXAry     ( rDXAry ),
    mnIndex     ( nIndex ),
    mnLen       ( nLen )
{
}

MetaTextArrayAction::MetaTextArrayAction( const Point& rStartPt,
                                          const OUString& rStr,
                                          o3tl::span<const sal_Int32> pDXAry,
                                          sal_Int32 nIndex,
                                          sal_Int32 nLen ) :
    MetaAction  ( MetaActionType::TEXTARRAY ),
    maStartPt   ( rStartPt ),
    maStr       ( rStr ),
    maDXAry     ( pDXAry.begin(), pDXAry.end() ),
    mnIndex     ( nIndex ),
    mnLen       ( nLen )
{
}

MetaTextArrayAction::~MetaTextArrayAction()
{
}

void MetaTextArrayAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTextArray( maStartPt, maStr, maDXAry, mnIndex, mnLen );
}

rtl::Reference<MetaAction> MetaTextArrayAction::Clone() const
{
    return new MetaTextArrayAction( *this );
}

void MetaTextArrayAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maStartPt.Move( nHorzMove, nVertMove );
}

void MetaTextArrayAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maStartPt, fScaleX, fScaleY );

    if ( !maDXAry.empty() && mnLen )
    {
        for ( sal_uInt16 i = 0, nCount = mnLen; i < nCount; i++ )
            maDXAry[ i ] = FRound( maDXAry[ i ] * fabs(fScaleX) );
    }
}

void MetaTextArrayAction::SetDXArray(std::vector<sal_Int32> aArray)
{
    maDXAry = std::move(aArray);
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

rtl::Reference<MetaAction> MetaStretchTextAction::Clone() const
{
    return new MetaStretchTextAction( *this );
}

void MetaStretchTextAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaStretchTextAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
    mnWidth = static_cast<sal_uLong>(FRound( mnWidth * fabs(fScaleX) ));
}
MetaTextRectAction::MetaTextRectAction() :
    MetaAction  ( MetaActionType::TEXTRECT ),
    mnStyle     ( DrawTextFlags::NONE )
{}

MetaTextRectAction::~MetaTextRectAction()
{}

MetaTextRectAction::MetaTextRectAction( const tools::Rectangle& rRect,
                                        const OUString& rStr, DrawTextFlags nStyle ) :
    MetaAction  ( MetaActionType::TEXTRECT ),
    maRect      ( rRect ),
    maStr       ( rStr ),
    mnStyle     ( nStyle )
{}

void MetaTextRectAction::Execute( OutputDevice* pOut )
{
    pOut->DrawText( maRect, maStr, mnStyle );
}

rtl::Reference<MetaAction> MetaTextRectAction::Clone() const
{
    return new MetaTextRectAction( *this );
}

void MetaTextRectAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaTextRectAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
}

MetaTextLineAction::MetaTextLineAction() :
    MetaAction  ( MetaActionType::TEXTLINE ),
    mnWidth     ( 0 ),
    meStrikeout ( STRIKEOUT_NONE ),
    meUnderline ( LINESTYLE_NONE ),
    meOverline  ( LINESTYLE_NONE )
{}

MetaTextLineAction::~MetaTextLineAction()
{}

MetaTextLineAction::MetaTextLineAction( const Point& rPos, tools::Long nWidth,
                                        FontStrikeout eStrikeout,
                                        FontLineStyle eUnderline,
                                        FontLineStyle eOverline ) :
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

rtl::Reference<MetaAction> MetaTextLineAction::Clone() const
{
    return new MetaTextLineAction( *this );
}

void MetaTextLineAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPos.Move( nHorzMove, nVertMove );
}

void MetaTextLineAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPos, fScaleX, fScaleY );
    mnWidth = FRound( mnWidth * fabs(fScaleX) );
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

rtl::Reference<MetaAction> MetaBmpAction::Clone() const
{
    return new MetaBmpAction( *this );
}

void MetaBmpAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaBmpAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaBmpScaleAction::Clone() const
{
    return new MetaBmpScaleAction( *this );
}

void MetaBmpScaleAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaBmpScaleAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maPt, maSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPt = aRectangle.TopLeft();
    maSz = aRectangle.GetSize();
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

rtl::Reference<MetaAction> MetaBmpScalePartAction::Clone() const
{
    return new MetaBmpScalePartAction( *this );
}

void MetaBmpScalePartAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

void MetaBmpScalePartAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maDstPt, maDstSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
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

rtl::Reference<MetaAction> MetaBmpExAction::Clone() const
{
    return new MetaBmpExAction( *this );
}

void MetaBmpExAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaBmpExAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaBmpExScaleAction::Clone() const
{
    return new MetaBmpExScaleAction( *this );
}

void MetaBmpExScaleAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaBmpExScaleAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maPt, maSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPt = aRectangle.TopLeft();
    maSz = aRectangle.GetSize();
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

rtl::Reference<MetaAction> MetaBmpExScalePartAction::Clone() const
{
    return new MetaBmpExScalePartAction( *this );
}

void MetaBmpExScalePartAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

void MetaBmpExScalePartAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maDstPt, maDstSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
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

rtl::Reference<MetaAction> MetaMaskAction::Clone() const
{
    return new MetaMaskAction( *this );
}

void MetaMaskAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaMaskAction::Scale( double fScaleX, double fScaleY )
{
    ImplScalePoint( maPt, fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaMaskScaleAction::Clone() const
{
    return new MetaMaskScaleAction( *this );
}

void MetaMaskScaleAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPt.Move( nHorzMove, nVertMove );
}

void MetaMaskScaleAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maPt, maSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPt = aRectangle.TopLeft();
    maSz = aRectangle.GetSize();
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

rtl::Reference<MetaAction> MetaMaskScalePartAction::Clone() const
{
    return new MetaMaskScalePartAction( *this );
}

void MetaMaskScalePartAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maDstPt.Move( nHorzMove, nVertMove );
}

void MetaMaskScalePartAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maDstPt, maDstSz);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maDstPt = aRectangle.TopLeft();
    maDstSz = aRectangle.GetSize();
}

MetaGradientAction::MetaGradientAction() :
    MetaAction(MetaActionType::GRADIENT)
{}

MetaGradientAction::~MetaGradientAction()
{}

MetaGradientAction::MetaGradientAction( const tools::Rectangle& rRect, const Gradient& rGradient ) :
    MetaAction  ( MetaActionType::GRADIENT ),
    maRect      ( rRect ),
    maGradient  ( rGradient )
{}

void MetaGradientAction::Execute( OutputDevice* pOut )
{
    pOut->DrawGradient( maRect, maGradient );
}

rtl::Reference<MetaAction> MetaGradientAction::Clone() const
{
    return new MetaGradientAction( *this );
}

void MetaGradientAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaGradientAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
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
        pOut->GetConnectMetaFile()->AddAction( this );
    }
}

rtl::Reference<MetaAction> MetaGradientExAction::Clone() const
{
    return new MetaGradientExAction( *this );
}

void MetaGradientExAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaGradientExAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaHatchAction::Clone() const
{
    return new MetaHatchAction( *this );
}

void MetaHatchAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaHatchAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
}

MetaWallpaperAction::MetaWallpaperAction() :
    MetaAction(MetaActionType::WALLPAPER)
{}

MetaWallpaperAction::~MetaWallpaperAction()
{}

MetaWallpaperAction::MetaWallpaperAction( const tools::Rectangle& rRect,
                                          const Wallpaper& rPaper ) :
    MetaAction  ( MetaActionType::WALLPAPER ),
    maRect      ( rRect ),
    maWallpaper ( rPaper )
{}

void MetaWallpaperAction::Execute( OutputDevice* pOut )
{
    pOut->DrawWallpaper( maRect, maWallpaper );
}

rtl::Reference<MetaAction> MetaWallpaperAction::Clone() const
{
    return new MetaWallpaperAction( *this );
}

void MetaWallpaperAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaWallpaperAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaClipRegionAction::Clone() const
{
    return new MetaClipRegionAction( *this );
}

void MetaClipRegionAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maRegion.Move( nHorzMove, nVertMove );
}

void MetaClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    maRegion.Scale( fScaleX, fScaleY );
}

MetaISectRectClipRegionAction::MetaISectRectClipRegionAction() :
    MetaAction(MetaActionType::ISECTRECTCLIPREGION)
{}

MetaISectRectClipRegionAction::~MetaISectRectClipRegionAction()
{}

MetaISectRectClipRegionAction::MetaISectRectClipRegionAction( const tools::Rectangle& rRect ) :
    MetaAction  ( MetaActionType::ISECTRECTCLIPREGION ),
    maRect      ( rRect )
{}

void MetaISectRectClipRegionAction::Execute( OutputDevice* pOut )
{
    pOut->IntersectClipRegion( maRect );
}

rtl::Reference<MetaAction> MetaISectRectClipRegionAction::Clone() const
{
    return new MetaISectRectClipRegionAction( *this );
}

void MetaISectRectClipRegionAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maRect.Move( nHorzMove, nVertMove );
}

void MetaISectRectClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    ImplScaleRect( maRect, fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaISectRegionClipRegionAction::Clone() const
{
    return new MetaISectRegionClipRegionAction( *this );
}

void MetaISectRegionClipRegionAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maRegion.Move( nHorzMove, nVertMove );
}

void MetaISectRegionClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    maRegion.Scale( fScaleX, fScaleY );
}

MetaMoveClipRegionAction::MetaMoveClipRegionAction() :
    MetaAction  ( MetaActionType::MOVECLIPREGION ),
    mnHorzMove  ( 0 ),
    mnVertMove  ( 0 )
{}

MetaMoveClipRegionAction::~MetaMoveClipRegionAction()
{}

MetaMoveClipRegionAction::MetaMoveClipRegionAction( tools::Long nHorzMove, tools::Long nVertMove ) :
    MetaAction  ( MetaActionType::MOVECLIPREGION ),
    mnHorzMove  ( nHorzMove ),
    mnVertMove  ( nVertMove )
{}

void MetaMoveClipRegionAction::Execute( OutputDevice* pOut )
{
    pOut->MoveClipRegion( mnHorzMove, mnVertMove );
}

rtl::Reference<MetaAction> MetaMoveClipRegionAction::Clone() const
{
    return new MetaMoveClipRegionAction( *this );
}

void MetaMoveClipRegionAction::Scale( double fScaleX, double fScaleY )
{
    mnHorzMove = FRound( mnHorzMove * fScaleX );
    mnVertMove = FRound( mnVertMove * fScaleY );
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

rtl::Reference<MetaAction> MetaLineColorAction::Clone() const
{
    return new MetaLineColorAction( *this );
}

MetaFillModeAction::MetaFillModeAction() :
    MetaAction  ( MetaActionType::FILLMODE ),
    mbSet       ( false )
{}

MetaFillModeAction::~MetaFillModeAction()
{}

void MetaFillModeAction::Execute( OutputDevice* pOut )
{
    if( mbSet )
        pOut->SetFillMode( mePolyFillMode );
    else
        pOut->SetFillMode();
}

rtl::Reference<MetaAction> MetaFillModeAction::Clone() const
{
    return new MetaFillModeAction( *this );
}

MetaFillModeAction::MetaFillModeAction( const PolyFillMode& rPolyFillMode, bool bSet )
    :    MetaAction  ( MetaActionType::FILLMODE),
      mePolyFillMode     ( rPolyFillMode ),
      mbSet       ( bSet )
{}

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

rtl::Reference<MetaAction> MetaFillColorAction::Clone() const
{
    return new MetaFillColorAction( *this );
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

rtl::Reference<MetaAction> MetaTextColorAction::Clone() const
{
    return new MetaTextColorAction( *this );
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

rtl::Reference<MetaAction> MetaTextFillColorAction::Clone() const
{
    return new MetaTextFillColorAction( *this );
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

rtl::Reference<MetaAction> MetaTextLineColorAction::Clone() const
{
    return new MetaTextLineColorAction( *this );
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

rtl::Reference<MetaAction> MetaOverlineColorAction::Clone() const
{
    return new MetaOverlineColorAction( *this );
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

rtl::Reference<MetaAction> MetaTextAlignAction::Clone() const
{
    return new MetaTextAlignAction( *this );
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

rtl::Reference<MetaAction> MetaMapModeAction::Clone() const
{
    return new MetaMapModeAction( *this );
}

void MetaMapModeAction::Scale( double fScaleX, double fScaleY )
{
    Point aPoint( maMapMode.GetOrigin() );

    ImplScalePoint( aPoint, fScaleX, fScaleY );
    maMapMode.SetOrigin( aPoint );
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
    if ( IsStarSymbol( maFont.GetFamilyName() )
        && ( maFont.GetCharSet() != RTL_TEXTENCODING_UNICODE ) )
    {
        maFont.SetCharSet( RTL_TEXTENCODING_UNICODE );
    }
}

void MetaFontAction::Execute( OutputDevice* pOut )
{
    pOut->SetFont( maFont );
}

rtl::Reference<MetaAction> MetaFontAction::Clone() const
{
    return new MetaFontAction( *this );
}

void MetaFontAction::Scale( double fScaleX, double fScaleY )
{
    const Size aSize(
        FRound(maFont.GetFontSize().Width() * fabs(fScaleX)),
        FRound(maFont.GetFontSize().Height() * fabs(fScaleY)));
    maFont.SetFontSize( aSize );
}

MetaPushAction::MetaPushAction() :
    MetaAction  ( MetaActionType::PUSH ),
    mnFlags     ( vcl::PushFlags::NONE )
{}

MetaPushAction::~MetaPushAction()
{}

MetaPushAction::MetaPushAction( vcl::PushFlags nFlags ) :
    MetaAction  ( MetaActionType::PUSH ),
    mnFlags     ( nFlags )
{}

void MetaPushAction::Execute( OutputDevice* pOut )
{
    pOut->Push( mnFlags );
}

rtl::Reference<MetaAction> MetaPushAction::Clone() const
{
    return new MetaPushAction( *this );
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

rtl::Reference<MetaAction> MetaPopAction::Clone() const
{
    return new MetaPopAction( *this );
}

MetaRasterOpAction::MetaRasterOpAction() :
    MetaAction  ( MetaActionType::RASTEROP ),
    meRasterOp  ( RasterOp::OverPaint )
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

rtl::Reference<MetaAction> MetaRasterOpAction::Clone() const
{
    return new MetaRasterOpAction( *this );
}

MetaTransparentAction::MetaTransparentAction() :
    MetaAction      ( MetaActionType::Transparent ),
    mnTransPercent  ( 0 )
{}

MetaTransparentAction::~MetaTransparentAction()
{}

MetaTransparentAction::MetaTransparentAction( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransPercent ) :
    MetaAction      ( MetaActionType::Transparent ),
    maPolyPoly      ( rPolyPoly ),
    mnTransPercent  ( nTransPercent )
{}

void MetaTransparentAction::Execute( OutputDevice* pOut )
{
    pOut->DrawTransparent( maPolyPoly, mnTransPercent );
}

rtl::Reference<MetaAction> MetaTransparentAction::Clone() const
{
    return new MetaTransparentAction( *this );
}

void MetaTransparentAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPolyPoly.Move( nHorzMove, nVertMove );
}

void MetaTransparentAction::Scale( double fScaleX, double fScaleY )
{
    for( sal_uInt16 i = 0, nCount = maPolyPoly.Count(); i < nCount; i++ )
        ImplScalePoly( maPolyPoly[ i ], fScaleX, fScaleY );
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

rtl::Reference<MetaAction> MetaFloatTransparentAction::Clone() const
{
    return new MetaFloatTransparentAction( *this );
}

void MetaFloatTransparentAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPoint.Move( nHorzMove, nVertMove );
}

void MetaFloatTransparentAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maPoint, maSize);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPoint = aRectangle.TopLeft();
    maSize = aRectangle.GetSize();
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

rtl::Reference<MetaAction> MetaEPSAction::Clone() const
{
    return new MetaEPSAction( *this );
}

void MetaEPSAction::Move( tools::Long nHorzMove, tools::Long nVertMove )
{
    maPoint.Move( nHorzMove, nVertMove );
}

void MetaEPSAction::Scale( double fScaleX, double fScaleY )
{
    tools::Rectangle aRectangle(maPoint, maSize);
    ImplScaleRect( aRectangle, fScaleX, fScaleY );
    maPoint = aRectangle.TopLeft();
    maSize = aRectangle.GetSize();
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

rtl::Reference<MetaAction> MetaRefPointAction::Clone() const
{
    return new MetaRefPointAction( *this );
}

MetaCommentAction::MetaCommentAction() :
    MetaAction  ( MetaActionType::COMMENT ),
    mnValue     ( 0 )
{
    ImplInitDynamicData( nullptr, 0UL );
}

MetaCommentAction::MetaCommentAction( const MetaCommentAction& rAct ) :
    MetaAction  ( MetaActionType::COMMENT ),
    maComment   ( rAct.maComment ),
    mnValue     ( rAct.mnValue )
{
    ImplInitDynamicData( rAct.mpData.get(), rAct.mnDataSize );
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
}

void MetaCommentAction::ImplInitDynamicData( const sal_uInt8* pData, sal_uInt32 nDataSize )
{
    if ( nDataSize && pData )
    {
        mnDataSize = nDataSize;
        mpData.reset( new sal_uInt8[ mnDataSize ] );
        memcpy( mpData.get(), pData, mnDataSize );
    }
    else
    {
        mnDataSize = 0;
        mpData = nullptr;
    }
}

void MetaCommentAction::Execute( OutputDevice* pOut )
{
    if ( pOut->GetConnectMetaFile() )
    {
        pOut->GetConnectMetaFile()->AddAction( this );
    }
}

rtl::Reference<MetaAction> MetaCommentAction::Clone() const
{
    return new MetaCommentAction( *this );
}

void MetaCommentAction::Move( tools::Long nXMove, tools::Long nYMove )
{
    if ( !(nXMove || nYMove) )
        return;

    if ( !(mnDataSize && mpData) )
        return;

    bool bPathStroke = (maComment == "XPATHSTROKE_SEQ_BEGIN");
    if ( !(bPathStroke || maComment == "XPATHFILL_SEQ_BEGIN") )
        return;

    SvMemoryStream  aMemStm( static_cast<void*>(mpData.get()), mnDataSize, StreamMode::READ );
    SvMemoryStream  aDest;
    if ( bPathStroke )
    {
        SvtGraphicStroke aStroke;
        ReadSvtGraphicStroke( aMemStm, aStroke );

        tools::Polygon aPath;
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
    mpData.reset();
    ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
}

// SJ: 25.07.06 #i56656# we are not able to mirror certain kind of
// comments properly, especially the XPATHSTROKE and XPATHFILL lead to
// problems, so it is better to remove these comments when mirroring
// FIXME: fake comment to apply the next hunk in the right location
void MetaCommentAction::Scale( double fXScale, double fYScale )
{
    if (( fXScale == 1.0 ) && ( fYScale == 1.0 ))
        return;

    if ( !(mnDataSize && mpData) )
        return;

    bool bPathStroke = (maComment == "XPATHSTROKE_SEQ_BEGIN");
    if ( bPathStroke || maComment == "XPATHFILL_SEQ_BEGIN" )
    {
        SvMemoryStream  aMemStm( static_cast<void*>(mpData.get()), mnDataSize, StreamMode::READ );
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
        mpData.reset();
        ImplInitDynamicData( static_cast<const sal_uInt8*>( aDest.GetData() ), aDest.Tell() );
    } else if( maComment == "EMF_PLUS_HEADER_INFO" ){
        SvMemoryStream  aMemStm( static_cast<void*>(mpData.get()), mnDataSize, StreamMode::READ );
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

MetaLayoutModeAction::MetaLayoutModeAction() :
    MetaAction  ( MetaActionType::LAYOUTMODE ),
    mnLayoutMode( vcl::text::ComplexTextLayoutFlags::Default )
{}

MetaLayoutModeAction::~MetaLayoutModeAction()
{}

MetaLayoutModeAction::MetaLayoutModeAction( vcl::text::ComplexTextLayoutFlags nLayoutMode ) :
    MetaAction  ( MetaActionType::LAYOUTMODE ),
    mnLayoutMode( nLayoutMode )
{}

void MetaLayoutModeAction::Execute( OutputDevice* pOut )
{
    pOut->SetLayoutMode( mnLayoutMode );
}

rtl::Reference<MetaAction> MetaLayoutModeAction::Clone() const
{
    return new MetaLayoutModeAction( *this );
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

rtl::Reference<MetaAction> MetaTextLanguageAction::Clone() const
{
    return new MetaTextLanguageAction( *this );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
