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

PDFWriter::PDFWriter( const PDFWriter::PDFWriterContext& rContext, const com::sun::star::uno::Reference< com::sun::star::beans::XMaterialHolder >& xEnc )
        :
        pImplementation( new PDFWriterImpl( rContext, xEnc, *this ) )
{
}

PDFWriter::~PDFWriter()
{
}

OutputDevice* PDFWriter::GetReferenceDevice()
{
    return pImplementation->getReferenceDevice();
}

sal_Int32 PDFWriter::NewPage( sal_Int32 nPageWidth, sal_Int32 nPageHeight, Orientation eOrientation )
{
    return pImplementation->newPage( nPageWidth, nPageHeight, eOrientation );
}

bool PDFWriter::Emit()
{
    return pImplementation->emit();
}

void PDFWriter::SetDocumentLocale( const com::sun::star::lang::Locale& rLoc )
{
    pImplementation->setDocumentLocale( rLoc );
}

void PDFWriter::SetFont( const Font& rFont )
{
    pImplementation->setFont( rFont );
}

void PDFWriter::DrawText( const Point& rPos, const OUString& rText )
{
    pImplementation->drawText( rPos, rText );
}

void PDFWriter::DrawTextLine(
                             const Point& rPos,
                             long nWidth,
                             FontStrikeout eStrikeout,
                             FontUnderline eUnderline,
                             FontUnderline eOverline,
                             sal_Bool bUnderlineAbove )
{
    pImplementation->drawTextLine( rPos, nWidth, eStrikeout, eUnderline, eOverline, bUnderlineAbove );
}

void PDFWriter::DrawTextArray(
                              const Point& rStartPt,
                              const OUString& rStr,
                              const sal_Int32* pDXAry,
                              xub_StrLen nIndex,
                              xub_StrLen nLen )
{
    pImplementation->drawTextArray( rStartPt, rStr, pDXAry, nIndex, nLen );
}

void PDFWriter::DrawStretchText(
                                const Point& rStartPt,
                                sal_uLong nWidth,
                                const OUString& rStr,
                                xub_StrLen nIndex,
                                xub_StrLen nLen )
{
    pImplementation->drawStretchText( rStartPt, nWidth, rStr, nIndex, nLen );
}

void PDFWriter::DrawText(
                         const Rectangle& rRect,
                         const OUString& rStr,
                         sal_uInt16 nStyle )
{
    pImplementation->drawText( rRect, rStr, nStyle );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop )
{
    pImplementation->drawLine( rStart, rStop );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
    pImplementation->drawLine( rStart, rStop, rInfo );
}

void PDFWriter::DrawPolygon( const Polygon& rPoly )
{
    pImplementation->drawPolygon( rPoly );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly )
{
    pImplementation->drawPolyLine( rPoly );
}

void PDFWriter::DrawRect( const Rectangle& rRect )
{
    pImplementation->drawRectangle( rRect );
}

void PDFWriter::DrawRect( const Rectangle& rRect, sal_uLong nHorzRound, sal_uLong nVertRound )
{
    pImplementation->drawRectangle( rRect, nHorzRound, nVertRound );
}

void PDFWriter::DrawEllipse( const Rectangle& rRect )
{
    pImplementation->drawEllipse( rRect );
}

void PDFWriter::DrawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
    pImplementation->drawArc( rRect, rStart, rStop, false, false );
}

void PDFWriter::DrawPie( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
    pImplementation->drawArc( rRect, rStart, rStop, true, false );
}

void PDFWriter::DrawChord( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
    pImplementation->drawArc( rRect, rStart, rStop, false, true );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly, const LineInfo& rInfo )
{
    pImplementation->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly, const ExtLineInfo& rInfo )
{
    pImplementation->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyPolygon( const PolyPolygon& rPolyPoly )
{
    pImplementation->drawPolyPolygon( rPolyPoly );
}

void PDFWriter::DrawPixel( const Point& rPos, const Color& rColor )
{
    pImplementation->drawPixel( rPos, rColor );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
    pImplementation->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize, const BitmapEx& rBitmap )
{
    pImplementation->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    pImplementation->drawHatch( rPolyPoly, rHatch );
}

void PDFWriter::DrawWallpaper( const Rectangle& rRect, const Wallpaper& rWallpaper )
{
    pImplementation->drawWallpaper( rRect, rWallpaper );
}

void PDFWriter::DrawTransparent( const PolyPolygon& rPolyPoly, sal_uInt16 nTransparencePercent )
{
    pImplementation->drawTransparent( rPolyPoly, nTransparencePercent );
}

void PDFWriter::BeginTransparencyGroup()
{
    pImplementation->beginTransparencyGroup();
}

void PDFWriter::EndTransparencyGroup( const Rectangle& rRect, sal_uInt16 nTransparentPercent )
{
    pImplementation->endTransparencyGroup( rRect, nTransparentPercent );
}

void PDFWriter::Push( sal_uInt16 nFlags )
{
    pImplementation->push( nFlags );
}

void PDFWriter::Pop()
{
    pImplementation->pop();
}

void PDFWriter::SetMapMode( const MapMode& rMapMode )
{
    pImplementation->setMapMode( rMapMode );
}

void PDFWriter::SetLineColor( const Color& rColor )
{
    pImplementation->setLineColor( rColor );
}

void PDFWriter::SetFillColor( const Color& rColor )
{
    pImplementation->setFillColor( rColor );
}

void PDFWriter::SetClipRegion()
{
    pImplementation->clearClipRegion();
}

void PDFWriter::SetClipRegion( const basegfx::B2DPolyPolygon& rRegion )
{
    pImplementation->setClipRegion( rRegion );
}

void PDFWriter::MoveClipRegion( long nHorzMove, long nVertMove )
{
    pImplementation->moveClipRegion( nHorzMove, nVertMove );
}

void PDFWriter::IntersectClipRegion( const basegfx::B2DPolyPolygon& rRegion )
{
    pImplementation->intersectClipRegion( rRegion );
}

void PDFWriter::IntersectClipRegion( const Rectangle& rRect )
{
    pImplementation->intersectClipRegion( rRect );
}

void PDFWriter::SetLayoutMode( sal_uLong nMode )
{
    pImplementation->setLayoutMode( (sal_Int32)nMode );
}

void PDFWriter::SetDigitLanguage( LanguageType eLang )
{
    pImplementation->setDigitLanguage( eLang );
}

void PDFWriter::SetTextColor( const Color& rColor )
{
    pImplementation->setTextColor( rColor );
}

void PDFWriter::SetTextFillColor()
{
    pImplementation->setTextFillColor();
}

void PDFWriter::SetTextFillColor( const Color& rColor )
{
    pImplementation->setTextFillColor( rColor );
}

void PDFWriter::SetTextLineColor()
{
    pImplementation->setTextLineColor();
}

void PDFWriter::SetTextLineColor( const Color& rColor )
{
    pImplementation->setTextLineColor( rColor );
}

void PDFWriter::SetOverlineColor()
{
    pImplementation->setOverlineColor();
}

void PDFWriter::SetOverlineColor( const Color& rColor )
{
    pImplementation->setOverlineColor( rColor );
}

void PDFWriter::SetTextAlign( ::TextAlign eAlign )
{
    pImplementation->setTextAlign( eAlign );
}

void PDFWriter::DrawJPGBitmap( SvStream& rStreamData, bool bIsTrueColor, const Size& rSrcSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask )
{
    pImplementation->drawJPGBitmap( rStreamData, bIsTrueColor, rSrcSizePixel, rTargetArea, rMask );
}

sal_Int32 PDFWriter::CreateLink( const Rectangle& rRect, sal_Int32 nPageNr )
{
    return pImplementation->createLink( rRect, nPageNr );
}
sal_Int32 PDFWriter::RegisterDestReference( sal_Int32 nDestId, const Rectangle& rRect, sal_Int32 nPageNr, DestAreaType eType )
{
    return pImplementation->registerDestReference( nDestId, rRect, nPageNr, eType );
}
//--->i56629
sal_Int32 PDFWriter::CreateNamedDest( const OUString& sDestName, const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    return pImplementation->createNamedDest( sDestName, rRect, nPageNr, eType );
}
sal_Int32 PDFWriter::CreateDest( const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    return pImplementation->createDest( rRect, nPageNr, eType );
}

sal_Int32 PDFWriter::SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
    return pImplementation->setLinkDest( nLinkId, nDestId );
}

sal_Int32 PDFWriter::SetLinkURL( sal_Int32 nLinkId, const OUString& rURL )
{
    return pImplementation->setLinkURL( nLinkId, rURL );
}

void PDFWriter::SetLinkPropertyID( sal_Int32 nLinkId, sal_Int32 nPropertyId )
{
    pImplementation->setLinkPropertyId( nLinkId, nPropertyId );
}

sal_Int32 PDFWriter::CreateOutlineItem( sal_Int32 nParent, const OUString& rText, sal_Int32 nDestID )
{
    return pImplementation->createOutlineItem( nParent, rText, nDestID );
}

sal_Int32 PDFWriter::SetOutlineItemParent( sal_Int32 nItem, sal_Int32 nNewParent )
{
    return pImplementation->setOutlineItemParent( nItem, nNewParent );
}

sal_Int32 PDFWriter::SetOutlineItemText( sal_Int32 nItem, const OUString& rText )
{
    return  pImplementation->setOutlineItemText( nItem, rText );
}

sal_Int32 PDFWriter::SetOutlineItemDest( sal_Int32 nItem, sal_Int32 nDest )
{
    return pImplementation->setOutlineItemDest( nItem, nDest );
}

void PDFWriter::CreateNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
    pImplementation->createNote( rRect, rNote, nPageNr );
}

sal_Int32 PDFWriter::BeginStructureElement( PDFWriter::StructElement eType, const OUString& rAlias )
{
    return pImplementation->beginStructureElement( eType, rAlias );
}

void PDFWriter::EndStructureElement()
{
    pImplementation->endStructureElement();
}

bool PDFWriter::SetCurrentStructureElement( sal_Int32 nID )
{
    return pImplementation->setCurrentStructureElement( nID );
}

bool PDFWriter::SetStructureAttribute( enum StructAttribute eAttr, enum StructAttributeValue eVal )
{
    return pImplementation->setStructureAttribute( eAttr, eVal );
}

bool PDFWriter::SetStructureAttributeNumerical( enum StructAttribute eAttr, sal_Int32 nValue )
{
    return pImplementation->setStructureAttributeNumerical( eAttr, nValue );
}

void PDFWriter::SetStructureBoundingBox( const Rectangle& rRect )
{
    pImplementation->setStructureBoundingBox( rRect );
}

void PDFWriter::SetActualText( const OUString& rText )
{
    pImplementation->setActualText( rText );
}

void PDFWriter::SetAlternateText( const OUString& rText )
{
    pImplementation->setAlternateText( rText );
}

void PDFWriter::SetAutoAdvanceTime( sal_uInt32 nSeconds, sal_Int32 nPageNr )
{
    pImplementation->setAutoAdvanceTime( nSeconds, nPageNr );
}

void PDFWriter::SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
    pImplementation->setPageTransition( eType, nMilliSec, nPageNr );
}

sal_Int32 PDFWriter::CreateControl( const PDFWriter::AnyWidget& rControl, sal_Int32 nPageNr )
{
    return pImplementation->createControl( rControl, nPageNr );
}

PDFOutputStream::~PDFOutputStream()
{
}

void PDFWriter::AddStream( const OUString& rMimeType, PDFOutputStream* pStream, bool bCompress )
{
    pImplementation->addStream( rMimeType, pStream, bCompress );
}

std::set< PDFWriter::ErrorCode > PDFWriter::GetErrors()
{
    return pImplementation->getErrors();
}

com::sun::star::uno::Reference< com::sun::star::beans::XMaterialHolder >
PDFWriter::InitEncryption( const OUString& i_rOwnerPassword,
                           const OUString& i_rUserPassword,
                           bool b128Bit
                          )
{
    return PDFWriterImpl::initEncryption( i_rOwnerPassword, i_rUserPassword, b128Bit );
}

void PDFWriter::PlayMetafile( const GDIMetaFile& i_rMTF, const vcl::PDFWriter::PlayMetafileContext& i_rPlayContext, PDFExtOutDevData* i_pData )
{
    pImplementation->playMetafile( i_rMTF, i_pData, i_rPlayContext, NULL);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
