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

#include <pdfwriter_impl.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>

using namespace vcl;

PDFWriter::AnyWidget::~AnyWidget()
{
}

PDFWriter::PDFWriter( const PDFWriter::PDFWriterContext& rContext, const css::uno::Reference< css::beans::XMaterialHolder >& xEnc )
        :
        xImplementation( new PDFWriterImpl( rContext, xEnc, *this ) )
{
}

PDFWriter::~PDFWriter()
{
}

OutputDevice* PDFWriter::GetReferenceDevice()
{
    return xImplementation->getReferenceDevice();
}

sal_Int32 PDFWriter::NewPage( sal_Int32 nPageWidth, sal_Int32 nPageHeight, Orientation eOrientation )
{
    return xImplementation->newPage( nPageWidth, nPageHeight, eOrientation );
}

bool PDFWriter::Emit()
{
    return xImplementation->emit();
}

void PDFWriter::SetDocumentLocale( const css::lang::Locale& rLoc )
{
    xImplementation->setDocumentLocale( rLoc );
}

void PDFWriter::SetFont( const vcl::Font& rFont )
{
    xImplementation->setFont( rFont );
}

void PDFWriter::DrawText( const Point& rPos, const OUString& rText )
{
    xImplementation->drawText( rPos, rText, 0, rText.getLength() );
}

void PDFWriter::DrawTextLine(
                             const Point& rPos,
                             long nWidth,
                             FontStrikeout eStrikeout,
                             FontUnderline eUnderline,
                             FontUnderline eOverline,
                             bool bUnderlineAbove )
{
    xImplementation->drawTextLine( rPos, nWidth, eStrikeout, eUnderline, eOverline, bUnderlineAbove );
}

void PDFWriter::DrawTextArray(
                              const Point& rStartPt,
                              const OUString& rStr,
                              const long* pDXAry,
                              sal_Int32 nIndex,
                              sal_Int32 nLen )
{
    xImplementation->drawTextArray( rStartPt, rStr, pDXAry, nIndex, nLen );
}

void PDFWriter::DrawStretchText(
                                const Point& rStartPt,
                                sal_uLong nWidth,
                                const OUString& rStr,
                                sal_Int32 nIndex,
                                sal_Int32 nLen )
{
    xImplementation->drawStretchText( rStartPt, nWidth, rStr, nIndex, nLen );
}

void PDFWriter::DrawText(
                         const Rectangle& rRect,
                         const OUString& rStr,
                         DrawTextFlags nStyle )
{
    xImplementation->drawText( rRect, rStr, nStyle );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop )
{
    xImplementation->drawLine( rStart, rStop );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
    xImplementation->drawLine( rStart, rStop, rInfo );
}

void PDFWriter::DrawPolygon( const tools::Polygon& rPoly )
{
    xImplementation->drawPolygon( rPoly );
}

void PDFWriter::DrawPolyLine( const tools::Polygon& rPoly )
{
    xImplementation->drawPolyLine( rPoly );
}

void PDFWriter::DrawRect( const Rectangle& rRect )
{
    xImplementation->drawRectangle( rRect );
}

void PDFWriter::DrawRect( const Rectangle& rRect, sal_uLong nHorzRound, sal_uLong nVertRound )
{
    xImplementation->drawRectangle( rRect, nHorzRound, nVertRound );
}

void PDFWriter::DrawEllipse( const Rectangle& rRect )
{
    xImplementation->drawEllipse( rRect );
}

void PDFWriter::DrawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
    xImplementation->drawArc( rRect, rStart, rStop, false, false );
}

void PDFWriter::DrawPie( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
    xImplementation->drawArc( rRect, rStart, rStop, true, false );
}

void PDFWriter::DrawChord( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
    xImplementation->drawArc( rRect, rStart, rStop, false, true );
}

void PDFWriter::DrawPolyLine( const tools::Polygon& rPoly, const LineInfo& rInfo )
{
    xImplementation->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyLine( const tools::Polygon& rPoly, const ExtLineInfo& rInfo )
{
    xImplementation->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyPolygon( const tools::PolyPolygon& rPolyPoly )
{
    xImplementation->drawPolyPolygon( rPolyPoly );
}

void PDFWriter::DrawPixel( const Point& rPos, const Color& rColor )
{
    xImplementation->drawPixel( rPos, rColor );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
    xImplementation->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize, const BitmapEx& rBitmap )
{
    xImplementation->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawHatch( const tools::PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    xImplementation->drawHatch( rPolyPoly, rHatch );
}

void PDFWriter::DrawGradient( const Rectangle& rRect, const Gradient& rGradient )
{
    xImplementation->drawGradient( rRect, rGradient );
}

void PDFWriter::DrawGradient( const tools::PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    xImplementation->push(PushFlags::CLIPREGION);
    xImplementation->setClipRegion( rPolyPoly.getB2DPolyPolygon() );
    xImplementation->drawGradient( rPolyPoly.GetBoundRect(), rGradient );
    xImplementation->pop();
}

void PDFWriter::DrawWallpaper( const Rectangle& rRect, const Wallpaper& rWallpaper )
{
    xImplementation->drawWallpaper( rRect, rWallpaper );
}

void PDFWriter::DrawTransparent( const tools::PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent )
{
    xImplementation->drawTransparent( rPolyPoly, nTransparencePercent );
}

void PDFWriter::BeginTransparencyGroup()
{
    xImplementation->beginTransparencyGroup();
}

void PDFWriter::EndTransparencyGroup( const Rectangle& rRect, sal_uInt16 nTransparentPercent )
{
    xImplementation->endTransparencyGroup( rRect, nTransparentPercent );
}

void PDFWriter::Push( PushFlags nFlags )
{
    xImplementation->push( nFlags );
}

void PDFWriter::Pop()
{
    xImplementation->pop();
}

void PDFWriter::SetMapMode( const MapMode& rMapMode )
{
    xImplementation->setMapMode( rMapMode );
}

void PDFWriter::SetLineColor( const Color& rColor )
{
    xImplementation->setLineColor( rColor );
}

void PDFWriter::SetFillColor( const Color& rColor )
{
    xImplementation->setFillColor( rColor );
}

void PDFWriter::SetClipRegion()
{
    xImplementation->clearClipRegion();
}

void PDFWriter::SetClipRegion( const basegfx::B2DPolyPolygon& rRegion )
{
    xImplementation->setClipRegion( rRegion );
}

void PDFWriter::MoveClipRegion( long nHorzMove, long nVertMove )
{
    xImplementation->moveClipRegion( nHorzMove, nVertMove );
}

void PDFWriter::IntersectClipRegion( const basegfx::B2DPolyPolygon& rRegion )
{
    xImplementation->intersectClipRegion( rRegion );
}

void PDFWriter::IntersectClipRegion( const Rectangle& rRect )
{
    xImplementation->intersectClipRegion( rRect );
}

void PDFWriter::SetLayoutMode( ComplexTextLayoutMode nMode )
{
    xImplementation->setLayoutMode( nMode );
}

void PDFWriter::SetDigitLanguage( LanguageType eLang )
{
    xImplementation->setDigitLanguage( eLang );
}

void PDFWriter::SetTextColor( const Color& rColor )
{
    xImplementation->setTextColor( rColor );
}

void PDFWriter::SetTextFillColor()
{
    xImplementation->setTextFillColor();
}

void PDFWriter::SetTextFillColor( const Color& rColor )
{
    xImplementation->setTextFillColor( rColor );
}

void PDFWriter::SetTextLineColor()
{
    xImplementation->setTextLineColor();
}

void PDFWriter::SetTextLineColor( const Color& rColor )
{
    xImplementation->setTextLineColor( rColor );
}

void PDFWriter::SetOverlineColor()
{
    xImplementation->setOverlineColor();
}

void PDFWriter::SetOverlineColor( const Color& rColor )
{
    xImplementation->setOverlineColor( rColor );
}

void PDFWriter::SetTextAlign( ::TextAlign eAlign )
{
    xImplementation->setTextAlign( eAlign );
}

void PDFWriter::DrawJPGBitmap( SvStream& rStreamData, bool bIsTrueColor, const Size& rSrcSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask )
{
    xImplementation->drawJPGBitmap( rStreamData, bIsTrueColor, rSrcSizePixel, rTargetArea, rMask );
}

sal_Int32 PDFWriter::CreateLink( const Rectangle& rRect, sal_Int32 nPageNr )
{
    return xImplementation->createLink( rRect, nPageNr );
}
sal_Int32 PDFWriter::RegisterDestReference( sal_Int32 nDestId, const Rectangle& rRect, sal_Int32 nPageNr, DestAreaType eType )
{
    return xImplementation->registerDestReference( nDestId, rRect, nPageNr, eType );
}
//--->i56629
sal_Int32 PDFWriter::CreateNamedDest( const OUString& sDestName, const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    return xImplementation->createNamedDest( sDestName, rRect, nPageNr, eType );
}
sal_Int32 PDFWriter::CreateDest( const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    return xImplementation->createDest( rRect, nPageNr, eType );
}

sal_Int32 PDFWriter::SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
    return xImplementation->setLinkDest( nLinkId, nDestId );
}

sal_Int32 PDFWriter::SetLinkURL( sal_Int32 nLinkId, const OUString& rURL )
{
    return xImplementation->setLinkURL( nLinkId, rURL );
}

void PDFWriter::SetLinkPropertyID( sal_Int32 nLinkId, sal_Int32 nPropertyId )
{
    xImplementation->setLinkPropertyId( nLinkId, nPropertyId );
}

sal_Int32 PDFWriter::CreateOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID )
{
    return xImplementation->createOutlineItem( nParent, rText, nDestID );
}

sal_Int32 PDFWriter::SetOutlineItemParent( sal_Int32 nItem, sal_Int32 nNewParent )
{
    return xImplementation->setOutlineItemParent( nItem, nNewParent );
}

sal_Int32 PDFWriter::SetOutlineItemText( sal_Int32 nItem, const OUString& rText )
{
    return  xImplementation->setOutlineItemText( nItem, rText );
}

sal_Int32 PDFWriter::SetOutlineItemDest( sal_Int32 nItem, sal_Int32 nDest )
{
    return xImplementation->setOutlineItemDest( nItem, nDest );
}

void PDFWriter::CreateNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
    xImplementation->createNote( rRect, rNote, nPageNr );
}

sal_Int32 PDFWriter::BeginStructureElement( PDFWriter::StructElement eType, const OUString& rAlias )
{
    return xImplementation->beginStructureElement( eType, rAlias );
}

void PDFWriter::EndStructureElement()
{
    xImplementation->endStructureElement();
}

bool PDFWriter::SetCurrentStructureElement( sal_Int32 nID )
{
    return xImplementation->setCurrentStructureElement( nID );
}

bool PDFWriter::SetStructureAttribute( enum StructAttribute eAttr, enum StructAttributeValue eVal )
{
    return xImplementation->setStructureAttribute( eAttr, eVal );
}

bool PDFWriter::SetStructureAttributeNumerical( enum StructAttribute eAttr, sal_Int32 nValue )
{
    return xImplementation->setStructureAttributeNumerical( eAttr, nValue );
}

void PDFWriter::SetStructureBoundingBox( const Rectangle& rRect )
{
    xImplementation->setStructureBoundingBox( rRect );
}

void PDFWriter::SetActualText( const OUString& rText )
{
    xImplementation->setActualText( rText );
}

void PDFWriter::SetAlternateText( const OUString& rText )
{
    xImplementation->setAlternateText( rText );
}

void PDFWriter::SetAutoAdvanceTime( sal_uInt32 nSeconds, sal_Int32 nPageNr )
{
    xImplementation->setAutoAdvanceTime( nSeconds, nPageNr );
}

void PDFWriter::SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
    xImplementation->setPageTransition( eType, nMilliSec, nPageNr );
}

sal_Int32 PDFWriter::CreateControl( const PDFWriter::AnyWidget& rControl, sal_Int32 nPageNr )
{
    return xImplementation->createControl( rControl, nPageNr );
}

PDFOutputStream::~PDFOutputStream()
{
}

void PDFWriter::AddStream( const OUString& rMimeType, PDFOutputStream* pStream, bool bCompress )
{
    xImplementation->addStream( rMimeType, pStream, bCompress );
}

std::set< PDFWriter::ErrorCode > PDFWriter::GetErrors()
{
    return xImplementation->getErrors();
}

css::uno::Reference< css::beans::XMaterialHolder >
PDFWriter::InitEncryption( const OUString& i_rOwnerPassword,
                           const OUString& i_rUserPassword,
                           bool b128Bit
                          )
{
    return PDFWriterImpl::initEncryption( i_rOwnerPassword, i_rUserPassword, b128Bit );
}

void PDFWriter::PlayMetafile( const GDIMetaFile& i_rMTF, const vcl::PDFWriter::PlayMetafileContext& i_rPlayContext, PDFExtOutDevData* i_pData )
{
    xImplementation->playMetafile( i_rMTF, i_pData, i_rPlayContext );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
