/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: pdfwriter.cxx,v $
 * $Revision: 1.24 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_vcl.hxx"

#include <pdfwriter_impl.hxx>
#include <vcl/bitmapex.hxx>
#include <vcl/image.hxx>

using namespace vcl;

PDFWriter::AnyWidget::~AnyWidget()
{
}

PDFWriter::PDFWriter( const PDFWriter::PDFWriterContext& rContext )
        :
        pImplementation( new PDFWriterImpl( rContext ) )
{
}

PDFWriter::~PDFWriter()
{
    delete (PDFWriterImpl*)pImplementation;
}

OutputDevice* PDFWriter::GetReferenceDevice()
{
    return ((PDFWriterImpl*)pImplementation)->getReferenceDevice();
}

sal_Int32 PDFWriter::NewPage( sal_Int32 nPageWidth, sal_Int32 nPageHeight, Orientation eOrientation )
{
    return ((PDFWriterImpl*)pImplementation)->newPage( nPageWidth, nPageHeight, eOrientation );
}

bool PDFWriter::Emit()
{
    return ((PDFWriterImpl*)pImplementation)->emit();
}

PDFWriter::PDFVersion PDFWriter::GetVersion() const
{
    return ((PDFWriterImpl*)pImplementation)->getVersion();
}

void PDFWriter::SetDocInfo( const PDFDocInfo& rInfo )
{
    ((PDFWriterImpl*)pImplementation)->setDocInfo( rInfo );
}

const PDFDocInfo& PDFWriter::GetDocInfo() const
{
    return ((PDFWriterImpl*)pImplementation)->getDocInfo();
}

void PDFWriter::SetDocumentLocale( const com::sun::star::lang::Locale& rLoc )
{
    ((PDFWriterImpl*)pImplementation)->setDocumentLocale( rLoc );
}

void PDFWriter::SetFont( const Font& rFont )
{
    ((PDFWriterImpl*)pImplementation)->setFont( rFont );
}

void PDFWriter::DrawText( const Point& rPos, const String& rText )
{
    ((PDFWriterImpl*)pImplementation)->drawText( rPos, rText );
}

void PDFWriter::DrawTextLine(
                             const Point& rPos,
                             long nWidth,
                             FontStrikeout eStrikeout,
                             FontUnderline eUnderline,
                             BOOL bUnderlineAbove )
{
    ((PDFWriterImpl*)pImplementation)->drawTextLine( rPos, nWidth, eStrikeout, eUnderline, bUnderlineAbove );
}

void PDFWriter::DrawTextArray(
                              const Point& rStartPt,
                              const XubString& rStr,
                              const sal_Int32* pDXAry,
                              xub_StrLen nIndex,
                              xub_StrLen nLen )
{
    ((PDFWriterImpl*)pImplementation)->drawTextArray( rStartPt, rStr, pDXAry, nIndex, nLen );
}

void PDFWriter::DrawStretchText(
                                const Point& rStartPt,
                                ULONG nWidth,
                                const XubString& rStr,
                                xub_StrLen nIndex,
                                xub_StrLen nLen )
{
    ((PDFWriterImpl*)pImplementation)->drawStretchText( rStartPt, nWidth, rStr, nIndex, nLen );
}

void PDFWriter::DrawText(
                         const Rectangle& rRect,
                         const XubString& rStr,
                         USHORT nStyle )
{
    ((PDFWriterImpl*)pImplementation)->drawText( rRect, rStr, nStyle );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop )
{
    ((PDFWriterImpl*)pImplementation)->drawLine( rStart, rStop );
}

void PDFWriter::DrawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
    ((PDFWriterImpl*)pImplementation)->drawLine( rStart, rStop, rInfo );
}

void PDFWriter::DrawPolygon( const Polygon& rPoly )
{
    ((PDFWriterImpl*)pImplementation)->drawPolygon( rPoly );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly )
{
    ((PDFWriterImpl*)pImplementation)->drawPolyLine( rPoly );
}

void PDFWriter::DrawRect( const Rectangle& rRect )
{
    ((PDFWriterImpl*)pImplementation)->drawRectangle( rRect );
}

void PDFWriter::DrawRect( const Rectangle& rRect, ULONG nHorzRound, ULONG nVertRound )
{
    ((PDFWriterImpl*)pImplementation)->drawRectangle( rRect, nHorzRound, nVertRound );
}

void PDFWriter::DrawEllipse( const Rectangle& rRect )
{
    ((PDFWriterImpl*)pImplementation)->drawEllipse( rRect );
}

void PDFWriter::DrawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
    ((PDFWriterImpl*)pImplementation)->drawArc( rRect, rStart, rStop, false, false );
}

void PDFWriter::DrawPie( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
    ((PDFWriterImpl*)pImplementation)->drawArc( rRect, rStart, rStop, true, false );
}

void PDFWriter::DrawChord( const Rectangle& rRect, const Point& rStart, const Point& rStop )
{
    ((PDFWriterImpl*)pImplementation)->drawArc( rRect, rStart, rStop, false, true );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly, const LineInfo& rInfo )
{
    ((PDFWriterImpl*)pImplementation)->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyLine( const Polygon& rPoly, const ExtLineInfo& rInfo )
{
    ((PDFWriterImpl*)pImplementation)->drawPolyLine( rPoly, rInfo );
}

void PDFWriter::DrawPolyPolygon( const PolyPolygon& rPolyPoly )
{
    ((PDFWriterImpl*)pImplementation)->drawPolyPolygon( rPolyPoly );
}

void PDFWriter::DrawPixel( const Point& rPos, const Color& rColor )
{
    ((PDFWriterImpl*)pImplementation)->drawPixel( rPos, rColor );
}

void PDFWriter::DrawPixel( const Polygon& rPts, const Color* pColors )
{
    ((PDFWriterImpl*)pImplementation)->drawPixel( rPts, pColors );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Bitmap& rBitmap )
{
    Size aSize = OutputDevice::LogicToLogic( rBitmap.GetPrefSize(),
                                             rBitmap.GetPrefMapMode(),
                                             ((PDFWriterImpl*)pImplementation)->getMapMode() );
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, aSize, rBitmap );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap )
{
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawBitmap( const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel, const Size& rSrcSizePixel, const Bitmap& rBitmap )
{
    Bitmap aBitmap( rBitmap );
    aBitmap.Crop( Rectangle( rSrcPtPixel, rSrcSizePixel ) );
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, aBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const BitmapEx& rBitmap )
{
    Size aSize = OutputDevice::LogicToLogic( rBitmap.GetPrefSize(),
                                             rBitmap.GetPrefMapMode(),
                                             ((PDFWriterImpl*)pImplementation)->getMapMode() );
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, aSize, rBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize, const BitmapEx& rBitmap )
{
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, rBitmap );
}

void PDFWriter::DrawBitmapEx( const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel, const Size& rSrcSizePixel, const BitmapEx& rBitmap )
{
    BitmapEx aBitmap( rBitmap );
    aBitmap.Crop( Rectangle( rSrcPtPixel, rSrcSizePixel ) );
    ((PDFWriterImpl*)pImplementation)->drawBitmap( rDestPt, rDestSize, aBitmap );
}

void PDFWriter::DrawMask( const Point& rDestPt, const Bitmap& rBitmap, const Color& rMaskColor )
{
    Size aSize = OutputDevice::LogicToLogic( rBitmap.GetPrefSize(),
                                             rBitmap.GetPrefMapMode(),
                                             ((PDFWriterImpl*)pImplementation)->getMapMode() );
    ((PDFWriterImpl*)pImplementation)->drawMask( rDestPt, aSize, rBitmap, rMaskColor );
}

void PDFWriter::DrawMask( const Point& rDestPt, const Size& rDestSize, const Bitmap& rBitmap, const Color& rMaskColor )
{
    ((PDFWriterImpl*)pImplementation)->drawMask( rDestPt, rDestSize, rBitmap, rMaskColor );
}

void PDFWriter::DrawMask( const Point& rDestPt, const Size& rDestSize, const Point& rSrcPtPixel, const Size& rSrcSizePixel, const Bitmap& rBitmap, const Color& rMaskColor )
{
    Bitmap aBitmap( rBitmap );
    aBitmap.Crop( Rectangle( rSrcPtPixel, rSrcSizePixel ) );
    ((PDFWriterImpl*)pImplementation)->drawMask( rDestPt, rDestSize, aBitmap, rMaskColor );
}

void PDFWriter::DrawGradient( const Rectangle& rRect, const Gradient& rGradient )
{
    ((PDFWriterImpl*)pImplementation)->drawGradient( rRect, rGradient );
}

void PDFWriter::DrawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    ((PDFWriterImpl*)pImplementation)->drawGradient( rPolyPoly, rGradient );
}

void PDFWriter::DrawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    ((PDFWriterImpl*)pImplementation)->drawHatch( rPolyPoly, rHatch );
}

void PDFWriter::DrawWallpaper( const Rectangle& rRect, const Wallpaper& rWallpaper )
{
    ((PDFWriterImpl*)pImplementation)->drawWallpaper( rRect, rWallpaper );
}

void PDFWriter::DrawTransparent( const PolyPolygon& rPolyPoly, USHORT nTransparencePercent )
{
    ((PDFWriterImpl*)pImplementation)->drawTransparent( rPolyPoly, nTransparencePercent );
}

void PDFWriter::BeginTransparencyGroup()
{
    ((PDFWriterImpl*)pImplementation)->beginTransparencyGroup();
}

void PDFWriter::EndTransparencyGroup( const Rectangle& rRect, USHORT nTransparentPercent )
{
    ((PDFWriterImpl*)pImplementation)->endTransparencyGroup( rRect, nTransparentPercent );
}

void PDFWriter::EndTransparencyGroup( const Rectangle& rRect, const Bitmap& rAlphaMask )
{
    ((PDFWriterImpl*)pImplementation)->endTransparencyGroup( rRect, rAlphaMask );
}

void PDFWriter::Push( USHORT nFlags )
{
    ((PDFWriterImpl*)pImplementation)->push( nFlags );
}

void PDFWriter::Pop()
{
    ((PDFWriterImpl*)pImplementation)->pop();
}

void PDFWriter::SetMapMode( const MapMode& rMapMode )
{
    ((PDFWriterImpl*)pImplementation)->setMapMode( rMapMode );
}

void PDFWriter::SetMapMode()
{
    ((PDFWriterImpl*)pImplementation)->setMapMode();
}

void PDFWriter::SetLineColor( const Color& rColor )
{
    ((PDFWriterImpl*)pImplementation)->setLineColor( rColor );
}

void PDFWriter::SetFillColor( const Color& rColor )
{
    ((PDFWriterImpl*)pImplementation)->setFillColor( rColor );
}

void PDFWriter::SetClipRegion()
{
    ((PDFWriterImpl*)pImplementation)->clearClipRegion();
}

void PDFWriter::SetClipRegion( const Region& rRegion )
{
    ((PDFWriterImpl*)pImplementation)->setClipRegion( rRegion );
}

void PDFWriter::MoveClipRegion( long nHorzMove, long nVertMove )
{
    ((PDFWriterImpl*)pImplementation)->moveClipRegion( nHorzMove, nVertMove );
}

void PDFWriter::IntersectClipRegion( const Region& rRegion )
{
    ((PDFWriterImpl*)pImplementation)->intersectClipRegion( rRegion );
}

void PDFWriter::IntersectClipRegion( const Rectangle& rRect )
{
    ((PDFWriterImpl*)pImplementation)->intersectClipRegion( rRect );
}

void PDFWriter::SetAntialiasing( USHORT nMode )
{
    ((PDFWriterImpl*)pImplementation)->setAntiAlias( (sal_Int32)nMode );
}

void PDFWriter::SetLayoutMode( ULONG nMode )
{
    ((PDFWriterImpl*)pImplementation)->setLayoutMode( (sal_Int32)nMode );
}

void PDFWriter::SetDigitLanguage( LanguageType eLang )
{
    ((PDFWriterImpl*)pImplementation)->setDigitLanguage( eLang );
}

void PDFWriter::SetTextColor( const Color& rColor )
{
    ((PDFWriterImpl*)pImplementation)->setTextColor( rColor );
}

void PDFWriter::SetTextFillColor()
{
    ((PDFWriterImpl*)pImplementation)->setTextFillColor();
}

void PDFWriter::SetTextFillColor( const Color& rColor )
{
    ((PDFWriterImpl*)pImplementation)->setTextFillColor( rColor );
}

void PDFWriter::SetTextLineColor()
{
    ((PDFWriterImpl*)pImplementation)->setTextLineColor();
}

void PDFWriter::SetTextLineColor( const Color& rColor )
{
    ((PDFWriterImpl*)pImplementation)->setTextLineColor( rColor );
}

void PDFWriter::SetTextAlign( ::TextAlign eAlign )
{
    ((PDFWriterImpl*)pImplementation)->setTextAlign( eAlign );
}

void PDFWriter::DrawJPGBitmap( SvStream& rStreamData, bool bIsTrueColor, const Size& rSrcSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask )
{
    ((PDFWriterImpl*)pImplementation)->drawJPGBitmap( rStreamData, bIsTrueColor, rSrcSizePixel, rTargetArea, rMask );
}

sal_Int32 PDFWriter::CreateLink( const Rectangle& rRect, sal_Int32 nPageNr )
{
    return ((PDFWriterImpl*)pImplementation)->createLink( rRect, nPageNr );
}
//--->i56629
sal_Int32 PDFWriter::CreateNamedDest( const rtl::OUString& sDestName, const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    return ((PDFWriterImpl*)pImplementation)->createNamedDest( sDestName, rRect, nPageNr, eType );
}
//<---
sal_Int32 PDFWriter::CreateDest( const Rectangle& rRect, sal_Int32 nPageNr, PDFWriter::DestAreaType eType )
{
    return ((PDFWriterImpl*)pImplementation)->createDest( rRect, nPageNr, eType );
}

sal_Int32 PDFWriter::SetLinkDest( sal_Int32 nLinkId, sal_Int32 nDestId )
{
    return ((PDFWriterImpl*)pImplementation)->setLinkDest( nLinkId, nDestId );
}

sal_Int32 PDFWriter::SetLinkURL( sal_Int32 nLinkId, const rtl::OUString& rURL )
{
    return ((PDFWriterImpl*)pImplementation)->setLinkURL( nLinkId, rURL );
}

void PDFWriter::SetLinkPropertyID( sal_Int32 nLinkId, sal_Int32 nPropertyId )
{
    ((PDFWriterImpl*)pImplementation)->setLinkPropertyId( nLinkId, nPropertyId );
}

sal_Int32 PDFWriter::CreateOutlineItem( sal_Int32 nParent, const rtl::OUString& rText, sal_Int32 nDestID )
{
    return ((PDFWriterImpl*)pImplementation)->createOutlineItem( nParent, rText, nDestID );
}

sal_Int32 PDFWriter::SetOutlineItemParent( sal_Int32 nItem, sal_Int32 nNewParent )
{
    return ((PDFWriterImpl*)pImplementation)->setOutlineItemParent( nItem, nNewParent );
}

sal_Int32 PDFWriter::SetOutlineItemText( sal_Int32 nItem, const rtl::OUString& rText )
{
    return  ((PDFWriterImpl*)pImplementation)->setOutlineItemText( nItem, rText );
}

sal_Int32 PDFWriter::SetOutlineItemDest( sal_Int32 nItem, sal_Int32 nDest )
{
    return ((PDFWriterImpl*)pImplementation)->setOutlineItemDest( nItem, nDest );
}

void PDFWriter::CreateNote( const Rectangle& rRect, const PDFNote& rNote, sal_Int32 nPageNr )
{
    ((PDFWriterImpl*)pImplementation)->createNote( rRect, rNote, nPageNr );
}

sal_Int32 PDFWriter::BeginStructureElement( PDFWriter::StructElement eType, const rtl::OUString& rAlias )
{
    return ((PDFWriterImpl*)pImplementation)->beginStructureElement( eType, rAlias );
}

void PDFWriter::EndStructureElement()
{
    ((PDFWriterImpl*)pImplementation)->endStructureElement();
}

bool PDFWriter::SetCurrentStructureElement( sal_Int32 nID )
{
    return ((PDFWriterImpl*)pImplementation)->setCurrentStructureElement( nID );
}

sal_Int32 PDFWriter::GetCurrentStructureElement()
{
    return ((PDFWriterImpl*)pImplementation)->getCurrentStructureElement();
}

bool PDFWriter::SetStructureAttribute( enum StructAttribute eAttr, enum StructAttributeValue eVal )
{
    return ((PDFWriterImpl*)pImplementation)->setStructureAttribute( eAttr, eVal );
}

bool PDFWriter::SetStructureAttributeNumerical( enum StructAttribute eAttr, sal_Int32 nValue )
{
    return ((PDFWriterImpl*)pImplementation)->setStructureAttributeNumerical( eAttr, nValue );
}

void PDFWriter::SetStructureBoundingBox( const Rectangle& rRect )
{
    ((PDFWriterImpl*)pImplementation)->setStructureBoundingBox( rRect );
}

void PDFWriter::SetActualText( const String& rText )
{
    ((PDFWriterImpl*)pImplementation)->setActualText( rText );
}

void PDFWriter::SetAlternateText( const String& rText )
{
    ((PDFWriterImpl*)pImplementation)->setAlternateText( rText );
}

void PDFWriter::SetAutoAdvanceTime( sal_uInt32 nSeconds, sal_Int32 nPageNr )
{
    ((PDFWriterImpl*)pImplementation)->setAutoAdvanceTime( nSeconds, nPageNr );
}

void PDFWriter::SetPageTransition( PDFWriter::PageTransition eType, sal_uInt32 nMilliSec, sal_Int32 nPageNr )
{
    ((PDFWriterImpl*)pImplementation)->setPageTransition( eType, nMilliSec, nPageNr );
}

sal_Int32 PDFWriter::CreateControl( const PDFWriter::AnyWidget& rControl, sal_Int32 nPageNr )
{
    return ((PDFWriterImpl*)pImplementation)->createControl( rControl, nPageNr );
}

PDFOutputStream::~PDFOutputStream()
{
}

void PDFWriter::AddStream( const String& rMimeType, PDFOutputStream* pStream, bool bCompress )
{
    ((PDFWriterImpl*)pImplementation)->addStream( rMimeType, pStream, bCompress );
}

void PDFWriter::BeginPattern( const Rectangle& rCellRect )
{
    ((PDFWriterImpl*)pImplementation)->beginPattern( rCellRect );
}

sal_Int32 PDFWriter::EndPattern( const SvtGraphicFill::Transform& rTransform )
{
    return ((PDFWriterImpl*)pImplementation)->endPattern( rTransform );
}

void PDFWriter::DrawPolyPolygon( const PolyPolygon& rPolyPoly, sal_Int32 nPattern, bool bEOFill )
{
    ((PDFWriterImpl*)pImplementation)->drawPolyPolygon( rPolyPoly, nPattern, bEOFill );
}

std::set< PDFWriter::ErrorCode > PDFWriter::GetErrors()
{
    return ((PDFWriterImpl*)pImplementation)->getErrors();
}
