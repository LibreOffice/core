/*************************************************************************
 *
 *  $RCSfile: pdfwriter.cxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-06-17 12:19:19 $
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

#include <pdfwriter_impl.hxx>
#include <bitmapex.hxx>
#include <image.hxx>

using namespace vcl;

PDFWriter::PDFWriter( const rtl::OUString& rFilename, PDFVersion eVersion, Compression eCompression )
        :
        pImplementation( new PDFWriterImpl( rFilename, eVersion, eCompression ) )
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

void PDFWriter::SetTextAlign( TextAlign eAlign )
{
    ((PDFWriterImpl*)pImplementation)->setTextAlign( eAlign );
}

void PDFWriter::DrawJPGBitmap( SvStream& rStreamData, const Size& rSrcSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask )
{
    ((PDFWriterImpl*)pImplementation)->drawJPGBitmap( rStreamData, rSrcSizePixel, rTargetArea, rMask );
}
