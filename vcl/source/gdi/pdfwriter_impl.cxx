/*************************************************************************
 *
 *  $RCSfile: pdfwriter_impl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: pl $ $Date: 2002-07-15 12:02:20 $
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
#include <rtl/strbuf.hxx>
#include <tools/debug.hxx>
#include <virdev.hxx>
#include <bmpacc.hxx>
#include <bitmapex.hxx>
#include <image.hxx>
#include <outdev.h>
#include <sallayout.hxx>

#include "implncvt.hxx"

#include <math.h>
#ifdef WNT
// Aaarrgh
#define M_PI 3.14159265
#endif

using namespace vcl;
using namespace rtl;

#if defined DEBUG || defined DBG_UTIL
#define MARK( x ) emitComment( x )
#else
#define MARK( x )
#endif

static void appendHex( sal_Int8 nInt, OStringBuffer& rBuffer )
{
    static const sal_Char pHexDigits[] = { '0', '1', '2', '3', '4', '5', '6', '7',
                                           '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
    rBuffer.append( pHexDigits[ (nInt >> 4) & 15 ] );
    rBuffer.append( pHexDigits[ nInt & 15 ] );
}

// appends a double. PDF does not accept exponential format, only fixed point
static void appendDouble( double fValue, OStringBuffer& rBuffer, int nPrecision = 5 )
{
    if( fValue < 0 )
    {
        rBuffer.append( '-' );
        fValue=-fValue;
    }

    sal_Int64 nInt = (sal_Int64)fValue;
    fValue -= (double)nInt;
    // optimizing hardware may lead to a value of 1.0 after the subtraction
    if( fValue == 1.0 || log10( 1.0-fValue ) <= -nPrecision )
    {
        nInt++;
        fValue = 0.0;
    }
    rBuffer.append( nInt );
    if( fValue )
    {
        fValue *= pow( 10.0, (double)nPrecision );
        nInt = (sal_Int64)fValue;
        while( nInt && ! (nInt % 10 ) )
            nInt /= 10;
        if( nInt )
        {
            rBuffer.append( '.' );
            rBuffer.append( nInt );
        }
    }
}


static void appendColor( const Color& rColor, OStringBuffer& rBuffer )
{

    if( rColor != Color( COL_TRANSPARENT ) )
    {
        appendDouble( (double)rColor.GetRed() / 255.0, rBuffer );
        rBuffer.append( ' ' );
        appendDouble( (double)rColor.GetGreen() / 255.0, rBuffer );
        rBuffer.append( ' ' );
        appendDouble( (double)rColor.GetBlue() / 255.0, rBuffer );
    }
}

static void appendStrokingColor( const Color& rColor, OStringBuffer& rBuffer )
{
    if( rColor != Color( COL_TRANSPARENT ) )
    {
        appendColor( rColor, rBuffer );
        rBuffer.append( " RG" );
    }
}

static void appendNonStrokingColor( const Color& rColor, OStringBuffer& rBuffer )
{
    if( rColor != Color( COL_TRANSPARENT ) )
    {
        appendColor( rColor, rBuffer );
        rBuffer.append( " rg" );
    }
}


PDFWriterImpl::PDFPage::PDFPage( PDFWriterImpl* pWriter, sal_Int32 nPageWidth, sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation )
        :
        m_pWriter( pWriter ),
        m_nPageWidth( nPageWidth ),
        m_nPageHeight( nPageHeight ),
        m_eOrientation( eOrientation ),
        m_nPageObject( 0 ),  // invalid object number
        m_nStreamObject( 0 ),
        m_nStreamLengthObject( 0 ),
        m_nBeginStreamPos( 0 )
{
}

PDFWriterImpl::PDFPage::~PDFPage()
{
}

void PDFWriterImpl::PDFPage::beginStream()
{
    m_nStreamObject = m_pWriter->createObject();
    if( ! m_pWriter->updateObject( m_nStreamObject ) )
        return;

    m_nStreamLengthObject = m_pWriter->createObject();
    // write content stream header
    OStringBuffer aLine;
    aLine.append( m_nStreamObject );
    aLine.append( " 0 obj\r\n  << /Length " );
    aLine.append( m_nStreamLengthObject );
    aLine.append( " 0 R >>\r\nstream\r\n" );
    if( ! m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() ) )
        return;
    if( osl_File_E_None != osl_getFilePos( m_pWriter->m_aFile, &m_nBeginStreamPos ) )
    {
        osl_closeFile( m_pWriter->m_aFile );
        m_pWriter->m_bOpen = false;
    }
}

void PDFWriterImpl::PDFPage::endStream()
{
    sal_uInt64 nEndStreamPos;
    if( osl_File_E_None != osl_getFilePos( m_pWriter->m_aFile, &nEndStreamPos ) )
    {
        osl_closeFile( m_pWriter->m_aFile );
        m_pWriter->m_bOpen = false;
        return;
    }
    if( ! m_pWriter->writeBuffer( "endstream\r\nendobj\r\n\r\n", 21 ) )
        return;
    // emit stream length object
    if( ! m_pWriter->updateObject( m_nStreamLengthObject ) )
        return;
    OStringBuffer aLine;
    aLine.append( m_nStreamLengthObject );
    aLine.append( " 0 obj\r\n  " );
    aLine.append( (sal_Int64)(nEndStreamPos-m_nBeginStreamPos) );
    aLine.append( "\r\nendobj\r\n\r\n" );
    m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() );
}

bool PDFWriterImpl::PDFPage::emit(sal_Int32 nParentObject )
{
    // emit page object
    m_nPageObject = m_pWriter->createObject();
    if( ! m_pWriter->updateObject( m_nPageObject ) )
        return false;
    OStringBuffer aLine;

    aLine.append( m_nPageObject );
    aLine.append( " 0 obj\r\n  << /Type /Page\r\n     /Parent " );
    aLine.append( nParentObject );
    aLine.append( " 0 R\r\n" );
    if( m_nPageWidth && m_nPageHeight )
    {
        aLine.append( "     /MediaBox " );
        aLine.append( "[ 0 0 " );
        aLine.append( m_nPageWidth );
        aLine.append( ' ' );
        aLine.append( m_nPageHeight );
        aLine.append( " ]\r\n" );
    }
    switch( m_eOrientation )
    {
        case PDFWriter::Landscape: aLine.append( "     /Rotate 90\r\n" );break;
        case PDFWriter::Seascape: aLine.append( "     /Rotate -90\r\n" );break;
        case PDFWriter::Portrait: aLine.append( "     /Rotate 0\r\n" );break;

        case PDFWriter::Inherit:
        default:
            break;
    }
    aLine.append( "     /Contents " );
    aLine.append( m_nStreamObject );
    aLine.append( " 0 R\r\n  >>\r\nendobj\r\n\r\n" );
    return m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::PDFPage::appendPoint( const Point& rPoint, OStringBuffer& rBuffer, bool bNeg )
{
    Point aPoint = OutputDevice::LogicToLogic( Point( rPoint.X(), rPoint.Y() ),
                                               m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                                               m_pWriter->m_aMapMode );
    sal_Int32 nValue    = aPoint.X();
    if( bNeg )
        nValue = -nValue;

    if( nValue < 0 )
    {
        rBuffer.append( '-' );
        nValue = -nValue;
    }
    sal_Int32 nInt      = nValue / 10;
    sal_Int32 nDecimal  = nValue % 10;
    rBuffer.append( nInt );
    if( nDecimal )
    {
        rBuffer.append( '.' );
        rBuffer.append( nDecimal );
    }

    rBuffer.append( ' ' );

    nValue      = 10*(m_nPageHeight ? m_nPageHeight : m_pWriter->m_nInheritedPageHeight) - aPoint.Y();
    if( bNeg )
        nValue = -nValue;

    if( nValue < 0 )
    {
        rBuffer.append( '-' );
        nValue = -nValue;
    }
    nInt        = nValue / 10;
    nDecimal    = nValue % 10;
    rBuffer.append( nInt );
    if( nDecimal )
    {
        rBuffer.append( '.' );
        rBuffer.append( nDecimal );
    }
}

void PDFWriterImpl::PDFPage::appendRect( const Rectangle& rRect, OStringBuffer& rBuffer )
{
    appendPoint( rRect.BottomLeft() + Point( 0, 1 ), rBuffer );
    rBuffer.append( ' ' );
    appendMappedLength( rRect.GetWidth(), rBuffer, false );
    rBuffer.append( ' ' );
    appendMappedLength( rRect.GetHeight(), rBuffer, true );
    rBuffer.append( " re" );
}

void PDFWriterImpl::PDFPage::appendPolygon( const Polygon& rPoly, OStringBuffer& rBuffer, bool bClose )
{
    int nPoints = rPoly.GetSize();
    appendPoint( rPoly[0], rBuffer );
    rBuffer.append( " m\r\n" );
    for( int i = 1; i < nPoints; i++ )
    {
        appendPoint( rPoly[i], rBuffer );
        rBuffer.append( " l" );
        rBuffer.append( (i & 3) ? " " : "\r\n" );
    }
    if( bClose )
        rBuffer.append( "h\r\n" );
}

void PDFWriterImpl::PDFPage::appendPolyPolygon( const PolyPolygon& rPolyPoly, OStringBuffer& rBuffer, bool bClose )
{
    int nPolygons = rPolyPoly.Count();
    for( int n = 0; n < nPolygons; n++ )
        appendPolygon( rPolyPoly[n], rBuffer, bClose );
}

void PDFWriterImpl::PDFPage::appendMappedLength( sal_Int32 nLength, OStringBuffer& rBuffer, bool bVertical )
{
    Size aSize = OutputDevice::LogicToLogic( Size( nLength, nLength ),
                                             m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                                             m_pWriter->m_aMapMode );
    sal_Int32 nInt      = ( bVertical ? aSize.Height() : aSize.Width() ) / 10;
    sal_Int32 nDecimal  = ( bVertical ? aSize.Height() : aSize.Width() ) % 10;
    rBuffer.append( nInt );
    if( nDecimal )
    {
        rBuffer.append( '.' );
        rBuffer.append( nDecimal );
    }
}

void PDFWriterImpl::PDFPage::appendLineInfo( const LineInfo& rInfo, OStringBuffer& rBuffer )
{
    if( rInfo.GetStyle() == LINE_DASH )
    {
        rBuffer.append( "[ " );
        for( int n = 0; n < rInfo.GetDashCount(); n++ )
        {
            appendMappedLength( rInfo.GetDashLen(), rBuffer );
            rBuffer.append( ' ' );
            appendMappedLength( rInfo.GetDistance(), rBuffer );
            rBuffer.append( ' ' );
        }
        for( int m = 0; m < rInfo.GetDotCount(); m++ )
        {
            appendMappedLength( rInfo.GetDotLen(), rBuffer );
            rBuffer.append( ' ' );
            appendMappedLength( rInfo.GetDistance(), rBuffer );
            rBuffer.append( ' ' );
        }
        rBuffer.append( "] 0 d\r\n" );
    }
    if( rInfo.GetWidth() > 1 )
    {
        appendMappedLength( rInfo.GetWidth(), rBuffer );
        rBuffer.append( " w\r\n" );
    }
}

/*
 *  class PDFWriterImpl
 */

PDFWriterImpl::PDFWriterImpl( const OUString& rFilename, PDFWriter::PDFVersion eVersion )
        :
        m_aFileName( rFilename ),
        m_pReferenceDevice( NULL ),
        m_nInheritedPageWidth( 595 ),  // default A4
        m_nInheritedPageHeight( 842 ), // default A4
        m_eInheritedOrientation( PDFWriter::Portrait ),
        m_nCurrentPage( -1 ),
        m_eVersion( eVersion ),
        m_nNextFID( 1 ),
        m_aMapMode( MAP_POINT, Point(), Fraction( 1L, 10L ), Fraction( 1L, 10L ) )
{
    Font aFont;
    aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ) );
    aFont.SetSize( Size( 0, 12 ) );

    GraphicsState aState;
    aState.m_aMapMode       = m_aMapMode;
    aState.m_aFont          = aFont;
    m_aGraphicsStack.push_front( aState );

    oslFileError  aError = osl_openFile( m_aFileName.pData, &m_aFile, osl_File_OpenFlag_Write | osl_File_OpenFlag_Create );
    if( aError != osl_File_E_None )
    {
        if( aError == osl_File_E_EXIST )
        {
            aError = osl_openFile( m_aFileName.pData, &m_aFile, osl_File_OpenFlag_Write );
            if( aError == osl_File_E_None )
                aError = osl_setFileSize( m_aFile, 0 );
        }
    }
    if( aError != osl_File_E_None )
        return;

    // write header
    OStringBuffer aBuffer( 20 );
    aBuffer.append( "%PDF-" );
    switch( m_eVersion )
    {
        case PDFWriter::PDF_1_2: aBuffer.append( "1.2" );break;
        case PDFWriter::PDF_1_3: aBuffer.append( "1.3" );break;
        default:
        case PDFWriter::PDF_1_4: aBuffer.append( "1.4" );break;
    }
    // append something binary as comment (suggested in PDF Reference)
    aBuffer.append( "\r\n%дц№я\r\n" );
    if( !writeBuffer( aBuffer.getStr(), aBuffer.getLength() ) )
    {
        osl_closeFile( m_aFile );
        return;
    }
    m_bOpen = true;
}

PDFWriterImpl::~PDFWriterImpl()
{
    delete static_cast<VirtualDevice*>(m_pReferenceDevice);
}

void PDFWriterImpl::emitComment( const OString& rComment )
{
    OStringBuffer aLine( rComment.getLength()+5 );
    aLine.append( "% " );
    aLine.append( rComment );
    aLine.append( "\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

bool PDFWriterImpl::writeBuffer( const void* pBuffer, sal_uInt64 nBytes )
{
    if( ! m_bOpen ) // we are already down the drain
        return false;

    if( ! nBytes ) // huh ?
        return true;

    sal_uInt64 nWritten;
    if( osl_writeFile( m_aFile, pBuffer, nBytes, &nWritten ) != osl_File_E_None )
        nWritten = 0;

    if( nWritten != nBytes )
    {
        osl_closeFile( m_aFile );
        m_bOpen = false;
    }

    return nWritten == nBytes;
}

OutputDevice* PDFWriterImpl::getReferenceDevice()
{
    if( ! m_pReferenceDevice )
    {
        VirtualDevice* pVDev = new VirtualDevice( 0 );
        m_pReferenceDevice = pVDev;
        pVDev->SetOutputSizePixel( Size( 640, 480 ) );
        m_pReferenceDevice->mpPDFWriter = this;
    }
    return m_pReferenceDevice;
}

ImplDevFontList* PDFWriterImpl::filterDevFontList( ImplDevFontList* pFontList )
{
    DBG_ASSERT( m_aPages.begin() == m_aPages.end(), "Fonts changing during PDF generation, document will be invalid" );

    ImplDevFontList* pFiltered = new ImplDevFontList();

    ImplDevFontListData* pData = pFontList->First();
    while( pData )
    {
        ImplFontData* pEntry = pData->mpFirst;
        while( pEntry )
        {
            if( pEntry->mbSubsettable || pEntry->mbEmbeddable )
            {
                ImplFontData* pNewData = new ImplFontData();
                *pNewData = *pEntry;
                pFiltered->Add( pNewData );
            }
            pData = pFontList->Next();
        }
    }
    return pFiltered;
}

sal_Int32 PDFWriterImpl::newPage( sal_Int32 nPageWidth, sal_Int32 nPageHeight, PDFWriter::Orientation eOrientation )
{
    endPage();
    m_aPages.push_back( PDFPage(this, nPageWidth, nPageHeight, eOrientation ) );
    m_aPages.back().beginStream();
    return ++m_nCurrentPage;
}

void PDFWriterImpl::endPage()
{
    if( m_aPages.begin() != m_aPages.end() )
    {
        m_aGraphicsStack.clear();
        m_aGraphicsStack.push_back( GraphicsState() );

        // this should pop the PDF graphics stack if necessary
        updateGraphicsState();

        m_aPages.back().endStream();

        // reset the default font
        Font aFont;
        aFont.SetName( String( RTL_CONSTASCII_USTRINGPARAM( "Times" ) ) );
        aFont.SetSize( Size( 0, 12 ) );

        m_aCurrentPDFState = m_aGraphicsStack.front();
        m_aGraphicsStack.front().m_aFont =  aFont;

        for( std::list<BitmapEmit>::iterator it = m_aBitmaps.begin();
             it != m_aBitmaps.end(); ++it )
        {
            if( ! it->m_aBitmap.IsEmpty() )
            {
                writeBitmapObject( *it );
                it->m_aBitmap = BitmapEx();
            }
        }
    }
}

sal_Int32 PDFWriterImpl::createObject()
{
    m_aObjects.push_back( ~0 );
    return m_aObjects.size();
}

bool PDFWriterImpl::updateObject( sal_Int32 n )
{
    if( ! m_bOpen )
        return false;

    sal_uInt64 nOffset = ~0;
    oslFileError aError = osl_getFilePos( m_aFile, &nOffset );
    DBG_ASSERT( aError == osl_File_E_None, "could not register object" );
    if( aError != osl_File_E_None )
    {
        osl_closeFile( m_aFile );
        m_bOpen = false;
    }
    m_aObjects[ n-1 ] = nOffset;
    return aError == osl_File_E_None;
}

#define CHECK_RETURN( x ) if( !x ) return 0

bool PDFWriterImpl::emitGradients()
{
    for( std::list<GradientEmit>::iterator it = m_aGradients.begin();
         it != m_aGradients.end(); ++it )
    {
        CHECK_RETURN( writeGradientFunction( *it ) );
    }
    return true;
}

bool PDFWriterImpl::emitTilings()
{
    OStringBuffer aTilingStream( 1024 );
    OStringBuffer aTilingObj( 1024 );

    for( std::list<BitmapPatternEmit>::const_iterator it = m_aTilings.begin(); it != m_aTilings.end(); ++it )
    {
        aTilingStream.setLength( 0 );
        aTilingObj.setLength( 0 );

        sal_Int32 nX = (sal_Int32)it->m_aRectangle.BottomLeft().X();
        sal_Int32 nY = (sal_Int32)it->m_aRectangle.BottomLeft().Y()+1;
        sal_Int32 nW = (sal_Int32)it->m_aRectangle.GetWidth();
        sal_Int32 nH = (sal_Int32)it->m_aRectangle.GetHeight();

        appendDouble( (double)nW/10.0, aTilingStream, 1 );
        aTilingStream.append( " 0 0 " );
        appendDouble( (double)nH/10.0, aTilingStream, 1 );
        aTilingStream.append( ' ' );
        appendDouble( (double)nX/10.0, aTilingStream, 1 );
        aTilingStream.append( ' ' );
        appendDouble( (double)nY/10.0, aTilingStream, 1 );
        aTilingStream.append( " cm\r\n  /Im" );
        aTilingStream.append( it->m_nBitmapObject );
        aTilingStream.append( " Do\r\n" );

        // write pattern object
        aTilingObj.append( it->m_nObject );
        aTilingObj.append( " 0 obj\r\n" );
        aTilingObj.append( "  << /Type /Pattern\r\n     /PatternType 1\r\n     /PaintType 1\r\n     /TilingType 1\r\n     /BBox [ " );
        appendDouble( (double)nX/10.0, aTilingObj, 1 );
        aTilingObj.append( ' ' );
        appendDouble( (double)nY/10.0, aTilingObj, 1 );
        aTilingObj.append( ' ' );
        appendDouble( (double)(nX+nW)/10.0, aTilingObj, 1 );
        aTilingObj.append( ' ' );
        appendDouble( (double)(nY+nH)/10.0, aTilingObj, 1 );
        aTilingObj.append( " ]\r\n     /XStep " );
        appendDouble( (double)nW/10.0, aTilingObj, 1 );
        aTilingObj.append( "\r\n     /YStep " );
        appendDouble( (double)nH/10.0, aTilingObj, 1 );
        aTilingObj.append( "\r\n     /Resources <<\r\n" );
        aTilingObj.append( "   /XObject << /Im" );
        aTilingObj.append( it->m_nBitmapObject );
        aTilingObj.append( ' ' );
        aTilingObj.append( it->m_nBitmapObject );
        aTilingObj.append( " 0 R >> >>\r\n     /Length " );
        aTilingObj.append( (sal_Int32)aTilingStream.getLength() );
        aTilingObj.append( "\r\n  >>\r\nstream\r\n" );
        CHECK_RETURN( updateObject( it->m_nObject ) );
        CHECK_RETURN( writeBuffer( aTilingObj.getStr(), aTilingObj.getLength() ) );
        CHECK_RETURN( writeBuffer( aTilingStream.getStr(), aTilingStream.getLength() ) );
        aTilingObj.setLength( 0 );
        aTilingObj.append( "\r\nendstream\r\nendobj\r\n\r\n" );
        CHECK_RETURN( writeBuffer( aTilingObj.getStr(), aTilingObj.getLength() ) );
    }
    return true;
}

bool PDFWriterImpl::emitHatches()
{
    OStringBuffer aHatchStream( 1024 );
    OStringBuffer aHatchObj( 1024 );

    // get maximum page rectangle
    sal_Int32 nWidth, nHeight, nMax = 0;
    for( std::list<PDFPage>::const_iterator pg = m_aPages.begin(); pg != m_aPages.end(); ++pg )
    {
        if( pg->m_nPageWidth && pg->m_nPageHeight )
        {
            nWidth  = pg->m_nPageWidth;
            nHeight = pg->m_nPageHeight;
        }
        else
        {
            nWidth  = m_nInheritedPageWidth;
            nHeight = m_nInheritedPageHeight;
        }
        nMax = (nMax > nWidth ? nMax : nWidth);
        nMax = (nMax > nHeight ? nMax : nHeight);
    }

    nMax /= 2;

    sal_Int32 nSingleHatch = 0, nDoubleHatch = 0, nTripleHatch = 0;

    for( std::list<HatchEmit>::iterator it = m_aHatches.begin(); it != m_aHatches.end(); ++it )
    {
        aHatchStream.setLength( 0 );
        aHatchObj.setLength( 0 );

        const Hatch& rHatch = it->m_aHatch;

        // draw hatch
        sal_Int32 nBaseHatch = 0;
        switch( rHatch.GetStyle() )
        {
            case HATCH_SINGLE:
                if( ! nSingleHatch )
                    nSingleHatch = createObject();
                nBaseHatch = nSingleHatch;
                break;
            case HATCH_DOUBLE:
                if( ! nDoubleHatch )
                    nDoubleHatch = createObject();
                nBaseHatch = nDoubleHatch;
                break;
            case HATCH_TRIPLE:
                if( ! nTripleHatch )
                    nTripleHatch = createObject();
                nBaseHatch = nTripleHatch;
                break;
        }
        aHatchStream.append( "/HCS cs " );
        aHatchStream.append( (double)rHatch.GetColor().GetRed()/255.0 );
        aHatchStream.append( ' ' );
        aHatchStream.append( (double)rHatch.GetColor().GetGreen()/255.0 );
        aHatchStream.append( ' ' );
        aHatchStream.append( (double)rHatch.GetColor().GetBlue()/255.0 );
        aHatchStream.append( " /P" );
        aHatchStream.append( nBaseHatch );
        aHatchStream.append( " scn\r\n" );
        aHatchStream.append( -nMax );
        aHatchStream.append( ' ' );
        aHatchStream.append( -nMax );
        aHatchStream.append( ' ' );
        aHatchStream.append( 2*nMax );
        aHatchStream.append( ' ' );
        aHatchStream.append( 2*nMax );
        aHatchStream.append( " re f\r\n" );

        // write pattern object
        aHatchObj.append( it->m_nObject );
        aHatchObj.append( " 0 obj\r\n" );
        aHatchObj.append( "  << /Type /Pattern\r\n     /PatternType 1\r\n     /PaintType 1\r\n     /TilingType 1\r\n     /BBox [ " );
        aHatchObj.append( -nMax );
        aHatchObj.append( ' ' );
        aHatchObj.append( -nMax );
        aHatchObj.append( ' ' );
        aHatchObj.append( nMax );
        aHatchObj.append( ' ' );
        aHatchObj.append( nMax );
        aHatchObj.append( " ]\r\n     /XStep " );
        aHatchObj.append( 2*nMax );
        aHatchObj.append( "\r\n     /YStep " );
        aHatchObj.append( 2*nMax );
        aHatchObj.append( "\r\n     /Matrix [ " );
        // prepare matrix
        const double theta = (double)rHatch.GetAngle() * M_PI / 1800.0;
        /* the constant factor in scale is arbitrary */
        const double scale = 2.0*(double)rHatch.GetDistance();
        appendDouble( scale*cos( theta ), aHatchObj );
        aHatchObj.append( ' ' );
        appendDouble( scale*sin( theta ), aHatchObj );
        aHatchObj.append( ' ' );
        appendDouble( scale*(-sin( theta )), aHatchObj );
        aHatchObj.append( ' ' );
        appendDouble( scale*cos( theta ), aHatchObj );
        aHatchObj.append( " 0 0 ]\r\n" );

        aHatchObj.append( "     /Resources <<\r\n" );
        aHatchObj.append( "  /ColorSpace << /HCS [ /Pattern /DeviceRGB ] >>\r\n" );
        aHatchObj.append( "  /Pattern << /P" );
        aHatchObj.append( nBaseHatch );
        aHatchObj.append( ' ' );
        aHatchObj.append( nBaseHatch );
        aHatchObj.append( " 0 R >>\r\n" );
        aHatchObj.append( " >>\r\n     /Length " );
        aHatchObj.append( (sal_Int32)aHatchStream.getLength() );
        aHatchObj.append( "\r\n  >>\r\nstream\r\n" );
        CHECK_RETURN( updateObject( it->m_nObject ) );
        CHECK_RETURN( writeBuffer( aHatchObj.getStr(), aHatchObj.getLength() ) );
        CHECK_RETURN( writeBuffer( aHatchStream.getStr(), aHatchStream.getLength() ) );
        aHatchObj.setLength( 0 );
        aHatchObj.append( "\r\nendstream\r\nendobj\r\n\r\n" );
        CHECK_RETURN( writeBuffer( aHatchObj.getStr(), aHatchObj.getLength() ) );
    }

    // emit needed base hatches
    for( int i = 0; i < 3; i++ )
    {
        if( ( i == 0 && nSingleHatch ) ||
            ( i == 1 && nDoubleHatch ) ||
            ( i == 2 && nTripleHatch ) )
        {
            sal_Int32 nObject = 0;
            aHatchStream.setLength( 0 );
            switch( i )
            {
                case 0:
                    nObject = nSingleHatch;
                    aHatchStream.append( "0 0 m 1 0 l S\r\n" );
                    break;
                case 1:
                    nObject = nDoubleHatch;
                    aHatchStream.append( "0 0 m 1 0 l S\r\n" );
                    aHatchStream.append( "0 0 m 0 1 l S\r\n" );
                    break;
                case 2:
                    nObject = nTripleHatch;
                    aHatchStream.append( "0 0 m 1 0 l S\r\n" );
                    aHatchStream.append( "0 0 m 0 1 l S\r\n" );
                    aHatchStream.append( "0 1 m 1 0 l S\r\n" );
                    break;
            }
            CHECK_RETURN( updateObject( nObject ) );
            aHatchObj.setLength( 0 );

            aHatchObj.append( nObject );
            aHatchObj.append( " 0 obj\r\n" );
            aHatchObj.append( "  << /Type /Pattern\r\n     /PatternType 1\r\n     /PaintType 2\r\n     /TilingType 1\r\n     /BBox [ 0 0 1 1 ]\r\n" );
            aHatchObj.append( "     /XStep 1\r\n" );
            aHatchObj.append( "     /YStep 1\r\n" );
            aHatchObj.append( "     /Resources << >>\r\n     /Length " );
            aHatchObj.append( (sal_Int32)aHatchStream.getLength() );
            aHatchObj.append( "\r\n  >>\r\nstream\r\n" );
            CHECK_RETURN( writeBuffer( aHatchObj.getStr(), aHatchObj.getLength() ) );
            CHECK_RETURN( writeBuffer( aHatchStream.getStr(), aHatchStream.getLength() ) );
            aHatchObj.setLength( 0 );
            aHatchObj.append( "\r\nendstream\r\nendobj\r\n\r\n" );
            CHECK_RETURN( writeBuffer( aHatchObj.getStr(), aHatchObj.getLength() ) );
        }
    }
    return true;
}

sal_Int32 PDFWriterImpl::emitFonts()
{
    // TODO
    return 0;
}

sal_Int32 PDFWriterImpl::emitResources()
{
    OStringBuffer aLine( 512 );

    // emit shadings
    sal_Int32 nShadingDict = 0;
    if( m_aGradients.begin() != m_aGradients.end() )
    {
        CHECK_RETURN( emitGradients() );
        aLine.setLength( 0 );
        aLine.append( nShadingDict = createObject() );
        aLine.append( " 0 obj\r\n  << " );
        for( std::list<GradientEmit>::iterator it = m_aGradients.begin();
             it != m_aGradients.end(); ++it )
        {
            aLine.append( "/P" );
            aLine.append( it->m_nObject );
            aLine.append( ' ' );
            aLine.append( it->m_nObject );
            aLine.append( " 0 R\r\n     " );
        }
        aLine.append( ">>\r\nendobj\r\n\r\n" );
        CHECK_RETURN( updateObject( nShadingDict ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit patterns
    sal_Int32 nPatternDict = 0;
    if( m_aHatches.begin() != m_aHatches.end() || m_aTilings.begin() != m_aTilings.end() )
    {
        if( m_aHatches.begin() != m_aHatches.end() )
            CHECK_RETURN( emitHatches() );
        if( m_aTilings.begin() != m_aTilings.end() )
            CHECK_RETURN( emitTilings() );
        aLine.setLength( 0 );
        aLine.append( nPatternDict = createObject() );
        aLine.append( " 0 obj\r\n  << " );
        for( std::list<HatchEmit>::const_iterator hatch = m_aHatches.begin();
             hatch != m_aHatches.end(); ++hatch )
        {
            aLine.append( "/P" );
            aLine.append( hatch->m_nObject );
            aLine.append( ' ' );
            aLine.append( hatch->m_nObject );
            aLine.append( " 0 R\r\n     " );
        }
        for( std::list<BitmapPatternEmit>::const_iterator tile = m_aTilings.begin();
             tile != m_aTilings.end(); ++tile )
        {
            aLine.append( "/P" );
            aLine.append( tile->m_nObject );
            aLine.append( ' ' );
            aLine.append( tile->m_nObject );
            aLine.append( " 0 R\r\n     " );
        }
        aLine.append( ">>\r\nendobj\r\n\r\n" );
        CHECK_RETURN( updateObject( nPatternDict ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit font dict
    sal_Int32 nFontDict = 0;
    CHECK_RETURN( (nFontDict = emitFonts()) );

    // emit xobject dict
    sal_Int32 nXObjectDict = 0;
    if( m_aBitmaps.begin() != m_aBitmaps.end() )
    {
        aLine.setLength( 0 );
        nXObjectDict = createObject();
        aLine.append( nXObjectDict );
        aLine.append( " 0 obj\r\n  << " );
        for( std::list<BitmapEmit>::const_iterator it = m_aBitmaps.begin();
             it != m_aBitmaps.end(); ++it )
        {
            aLine.append( "/Im" );
            aLine.append( it->m_nObject );
            aLine.append( ' ' );
            aLine.append( it->m_nObject );
            aLine.append( " 0 R\r\n     " );
        }
        aLine.append( ">>\r\nendobj\r\n\r\n" );
        CHECK_RETURN( updateObject( nXObjectDict ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit Resource dict
    sal_Int32 nResourceDict = createObject();
    CHECK_RETURN( updateObject( nResourceDict ) );
    aLine.setLength( 0 );
    aLine.append( nResourceDict );
    aLine.append( " 0 obj\r\n  << /Font " );
    aLine.append( nFontDict );
    aLine.append( " 0 R\r\n" );
    if( nXObjectDict )
    {
        aLine.append( "     /XObject " );
        aLine.append( nXObjectDict );
        aLine.append( " 0 R\r\n" );
    }
    if( nShadingDict )
    {
        aLine.append( "     /Shading " );
        aLine.append( nShadingDict );
        aLine.append( " 0 R\r\n" );
    }
    if( nPatternDict )
    {
        aLine.append( "     /Pattern " );
        aLine.append( nPatternDict );
        aLine.append( " 0 R\r\n" );
    }
    aLine.append( "     /ProcSet [ /PDF " );
    if( nXObjectDict )
        aLine.append( "/ImageC /ImageI " );
    aLine.append( "]\r\n" );
    aLine.append( "  >>\r\nendobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nResourceDict;
}

#undef CHECK_RETURN
#define CHECK_RETURN( x ) if( !x ) return false

bool PDFWriterImpl::emitCatalog()
{
    // build page tree
    // currently there is only one node that contains all leaves

    // first create a page tree node id
    sal_Int32 nTreeNode = createObject();

    // emit all pages
    for( std::list<PDFPage>::iterator it = m_aPages.begin(); it != m_aPages.end(); ++it )
        if( ! it->emit( nTreeNode ) )
            return false;

    sal_Int32 nResourceDict = emitResources();

    // adjust tree node file offset
    if( ! updateObject( nTreeNode ) )
        return false;

    // emit tree node
    OStringBuffer aLine( 1024 );
    aLine.append( nTreeNode );
    aLine.append( " 0 obj\r\n" );
    aLine.append( "  << /Type /Pages\r\n" );
    aLine.append( "     /Resources " );
    aLine.append( nResourceDict );
    aLine.append( " 0 R\r\n" );
    switch( m_eInheritedOrientation )
    {
        case PDFWriter::Landscape: aLine.append( "     /Rotate 90\r\n" );break;
        case PDFWriter::Seascape: aLine.append( "     /Rotate -90\r\n" );break;

        case PDFWriter::Inherit: // actually Inherit would be a bug, but insignificant
        case PDFWriter::Portrait:
        default:
            break;
    }
    aLine.append( "     /MediaBox [ 0 0 " );
    aLine.append( m_nInheritedPageWidth );
    aLine.append( ' ' );
    aLine.append( m_nInheritedPageHeight );
    aLine.append( " ]\r\n     /Kids [ " );
    for( std::list<PDFPage>::const_iterator iter = m_aPages.begin(); iter != m_aPages.end(); ++iter )
    {
        aLine.append( iter->m_nPageObject );
        aLine.append( " 0 R\r\n             " );
    }
    aLine.append( "]\r\n     /Count " );
    aLine.append( (sal_Int32)m_aPages.size() );
    aLine.append( "\r\n  >>\r\nendobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // emit Catalog
    m_nCatalogObject = createObject();
    if( ! updateObject( m_nCatalogObject ) )
        return false;
    aLine.setLength( 0 );
    aLine.append( m_nCatalogObject );
    aLine.append( " 0 obj\r\n  << /Type /Catalog\r\n     /Pages " );
    aLine.append( nTreeNode );
    aLine.append( " 0 R\r\n  >>\r\nendobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

bool PDFWriterImpl::emitTrailer()
{
    // emit xref table

    // remember start
    sal_uInt64 nXRefOffset = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nXRefOffset )) );
    nXRefOffset;
    CHECK_RETURN( writeBuffer( "xref\r\n", 6 ) );

    sal_Int32 nObjects = m_aObjects.size();
    OStringBuffer aLine;
    aLine.append( "0 " );
    aLine.append( (sal_Int32)(nObjects+1) );
    aLine.append( "\r\n" );
    aLine.append( "0000000000 65535 f\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    for( sal_Int32 i = 0; i < nObjects; i++ )
    {
        aLine.setLength( 0 );
        OString aOffset = OString::valueOf( (sal_Int64)m_aObjects[i] );
        for( sal_Int32 j = 0; j < (10-aOffset.getLength()); j++ )
            aLine.append( '0' );
        aLine.append( aOffset );
        aLine.append( " 00000 n\r\n" );
        DBG_ASSERT( aLine.getLength() == 20, "invalid xref entry" );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit trailer
    aLine.setLength( 0 );
    aLine.append( "trailer\r\n<< /Size " );
    aLine.append( (sal_Int32)(nObjects+1) );
    aLine.append( "\r\n   /Root " );
    aLine.append( m_nCatalogObject );
    aLine.append( " 0 R\r\n>>\r\nstartxref\r\n" );
    aLine.append( (sal_Int64)nXRefOffset );
    aLine.append( "\r\n%%EOF\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

bool PDFWriterImpl::emit()
{
    endPage();

    // emit catalog
    CHECK_RETURN( emitCatalog() );

    // emit trailer
    CHECK_RETURN( emitTrailer() );

    osl_closeFile( m_aFile );
    m_bOpen = false;

    return true;
}

void PDFWriterImpl::registerGlyphs( int nGlyphs, long* pGlyphs, sal_Unicode* pUnicodes, sal_uInt8* pMappedGlyphs, sal_Int32* pMappedFontObjects )
{
    ImplFontData* pCurrentFont = m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData;
    if( pCurrentFont->mbSubsettable )
    {
        FontSubset& rSubset = m_aSubsets[ pCurrentFont ];
        for( int i = 0; i < nGlyphs; i++ )
        {
            // search for glyphID
            FontMapping::iterator it = rSubset.m_aMapping.find( pGlyphs[i] );
            if( it != rSubset.m_aMapping.end() )
            {
                pMappedFontObjects[i] = it->second.m_nFontID;
                pMappedGlyphs[i] = it->second.m_nSubsetGlyphID;
            }
            else
            {
                // create new subset if necessary
                if( rSubset.m_aSubsets.begin() == rSubset.m_aSubsets.end() ||
                    rSubset.m_aSubsets.back().m_aMapping.size() > 253 )
                {
                    rSubset.m_aSubsets.push_back( FontEmit( m_nNextFID++ ) );
                }

                // copy font id
                pMappedFontObjects[i] = rSubset.m_aSubsets.back().m_nFontID;
                // create new glyph in subset
                sal_uInt8 nNewId = rSubset.m_aSubsets.back().m_aMapping.size()+1;
                pMappedGlyphs[i] = nNewId;

                // add new glyph to emitted font subset
                GlyphEmit& rNewGlyphEmit = rSubset.m_aSubsets.back().m_aMapping[ pGlyphs[i] ];
                rNewGlyphEmit.m_nSubsetGlyphID = nNewId;
                rNewGlyphEmit.m_aUnicode = (pUnicodes ? pUnicodes[i] : 0);

                // add new glyph to font mapping
                Glyph& rNewGlyph = rSubset.m_aMapping[ pGlyphs[i] ];
                rNewGlyph.m_nFontID = pMappedFontObjects[i];
                rNewGlyph.m_nSubsetGlyphID = nNewId;
            }
        }
    }
    else
    {
        // TODO: downloadable font
    }
}

void PDFWriterImpl::drawLayout( const SalLayout& rSalLayout )
{
    // TODO: the needed methods must be moved to the base class
    // then rename the input paramter and scratch the next line
    const GenericSalLayout& rLayout = reinterpret_cast<const GenericSalLayout&>(rSalLayout);

    OStringBuffer aLine( 512 );
    // begin text object
    aLine.append( "BT\r\n" );

    const int nMaxGlyphs = 100;

    long pGlyphs[nMaxGlyphs];
    sal_Int32 pXOffsets[nMaxGlyphs];
    sal_uInt8 pMappedGlyphs[nMaxGlyphs];
    sal_Int32 pMappedFontObjects[nMaxGlyphs];
    int nGlyphs;
    int nIndex = 0;
    Point aPos;
    while( nGlyphs = rLayout.GetNextGlyphs( nMaxGlyphs, pGlyphs, aPos, nIndex, pXOffsets ) )
    {
        registerGlyphs( nGlyphs, pGlyphs, NULL, pMappedGlyphs, pMappedFontObjects );
        int nLast = 0;
        while( nLast < nGlyphs )
        {
            int nNext = nLast+1;
            while( nNext < nGlyphs && pMappedFontObjects[ nNext ] == pMappedFontObjects[nLast] )
                nNext++;
            aLine.append( "/F" );
            aLine.append( pMappedFontObjects[nLast] );
            aLine.append( ' ' );
            m_aPages.back().appendMappedLength( m_aCurrentPDFState.m_aFont.GetHeight(), aLine, true );
            aLine.append( " Tf [ <" );
            for( int i = nLast; i < nNext; i++ )
            {
                appendHex( (sal_Int8)pMappedGlyphs[i], aLine );
                if( (i % 70) == 0 )
                    aLine.append( "\r\n" );
            }
            aLine.append( "> ] Tj\r\n" );

            nLast = nNext;
        }
    }

    // end textobject
    aLine.append( "ET\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawText( const Point& rPos, const String& rText )
{
    MARK( "drawText" );

    updateGraphicsState();

    // get a sal layout
    getReferenceDevice();
    // set font on reference device
    m_pReferenceDevice->SetFont( m_aCurrentPDFState.m_aFont );
    m_pReferenceDevice->SetLayoutMode( m_aCurrentPDFState.m_nLayoutMode );
    // get the layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, 0, rText.Len(), rPos );
    pLayout->Reference();

    drawLayout( *pLayout );
    pLayout->Release();
}

void PDFWriterImpl::drawLine( const Point& rStart, const Point& rStop )
{
    MARK( "drawLine" );

    updateGraphicsState();

    OStringBuffer aLine;
    m_aPages.back().appendPoint( rStart, aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( rStop, aLine );
    aLine.append( " l S\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
    MARK( "drawLine with LineInfo" );

    updateGraphicsState();

#if 1
    if( rInfo.GetStyle() == LINE_SOLID && rInfo.GetWidth() < 2 )
    {
        drawLine( rStart, rStop );
        return;
    }

    OStringBuffer aLine;

    aLine.append( "q " );
    m_aPages.back().appendLineInfo( rInfo, aLine );
    m_aPages.back().appendPoint( rStart, aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( rStop, aLine );
    aLine.append( " l S Q\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
#else
    Polygon             aPoly( 2 ); aPoly[ 0 ] = rStart; aPoly[ 1 ] = rStop;
    ImplLineConverter   aLineCvt( aPoly, rInfo, NULL );

    if( rInfo.GetWidth() > 1 )
    {
        Color aOldLineColor = m_aGraphicsStack.front().m_aLineColor;
        Color aOldFillColor = m_aGraphicsStack.front().m_aFillColor;
        setFillColor( m_aGraphicsStack.front().m_aLineColor );
        setLineColor( Color( COL_TRANSPARENT ) );
        for( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
            drawPolygon( *pPoly );
        setLineColor( aOldLineColor );
        setFillColor( aOldFillColor );
    }
    else
    {
        for( const Polygon* pPoly = aLineCvt.ImplGetFirst(); pPoly; pPoly = aLineCvt.ImplGetNext() )
            drawLine( (*pPoly)[0], (*pPoly)[1] );
    }
#endif
}

void PDFWriterImpl::drawPolygon( const Polygon& rPoly )
{
    MARK( "drawPolygon" );

    updateGraphicsState();

    int nPoints = rPoly.GetSize();
    OStringBuffer aLine( 20 * nPoints );
    m_aPages.back().appendPolygon( rPoly, aLine );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "B*\r\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "S\r\n" );
    else
        aLine.append( "f*\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyPolygon( const PolyPolygon& rPolyPoly )
{
    MARK( "drawPolyPolygon" );

    updateGraphicsState();

    int nPolygons = rPolyPoly.Count();

    OStringBuffer aLine( 40 * nPolygons );
    m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "B*\r\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "S\r\n" );
    else
        aLine.append( "f*\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawTransparent( const PolyPolygon& rPolyPoly, sal_uInt32 nTransparentPercent )
{
    MARK( "drawTransparent" );

    updateGraphicsState();

    if( m_eVersion < PDFWriter::PDF_1_4 )
    {
        drawPolyPolygon( rPolyPoly );
        return;
    }
    // TODO: build in transparency
    drawPolyPolygon( rPolyPoly );
}

void PDFWriterImpl::drawRectangle( const Rectangle& rRect )
{
    MARK( "drawRectangle" );

    updateGraphicsState();

    OStringBuffer aLine( 40 );
    m_aPages.back().appendRect( rRect, aLine );

    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( " B*\r\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( " S\r\n" );
    else
        aLine.append( " f*\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawRectangle( const Rectangle& rRect, sal_uInt32 nHorzRound, sal_uInt32 nVertRound )
{
    MARK( "drawRectangle with rounded edges" );

    if( !nHorzRound && !nVertRound )
        drawRectangle( rRect );

    updateGraphicsState();

    if( nHorzRound > (sal_uInt32)rRect.GetWidth()/2 )
        nHorzRound = rRect.GetWidth()/2;
    if( nVertRound > (sal_uInt32)rRect.GetWidth()/2 )
        nVertRound = rRect.GetWidth()/2;

    Point aPoints[16];
    const double kappa = 0.5522847498;
    const sal_uInt32 kx = (sal_uInt32)((kappa*(double)nHorzRound)+0.5);
    const sal_uInt32 ky = (sal_uInt32)((kappa*(double)nVertRound)+0.5);

    aPoints[1]  = Point( rRect.TopLeft().X() + nHorzRound, rRect.TopLeft().Y() );
    aPoints[0]  = Point( aPoints[1].X() - kx, aPoints[1].Y() );
    aPoints[2]  = Point( rRect.TopRight().X()+1 - nHorzRound, aPoints[1].Y() );
    aPoints[3]  = Point( aPoints[2].X()+kx, aPoints[2].Y() );

    aPoints[5]  = Point( rRect.TopRight().X()+1, rRect.TopRight().Y()+nVertRound );
    aPoints[4]  = Point( aPoints[5].X(), aPoints[5].Y()-ky );
    aPoints[6]  = Point( aPoints[5].X(), rRect.BottomRight().Y()+1 - nVertRound );
    aPoints[7]  = Point( aPoints[6].X(), aPoints[6].Y()+ky );

    aPoints[9]  = Point( rRect.BottomRight().X()+1-nHorzRound, rRect.BottomRight().Y()+1 );
    aPoints[8]  = Point( aPoints[9].X()+kx, aPoints[9].Y() );
    aPoints[10] = Point( rRect.BottomLeft().X() + nHorzRound, aPoints[9].Y() );
    aPoints[11] = Point( aPoints[10].X()-kx, aPoints[10].Y() );

    aPoints[13] = Point( rRect.BottomLeft().X(), rRect.BottomLeft().Y()+1-nVertRound );
    aPoints[12] = Point( aPoints[13].X(), aPoints[13].Y()+ky );
    aPoints[14] = Point( rRect.TopLeft().X(), rRect.TopLeft().Y()+nVertRound );
    aPoints[15] = Point( aPoints[14].X(), aPoints[14].Y()-ky );


    OStringBuffer aLine( 80 );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( aPoints[2], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[3], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[4], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[5], aLine );
    aLine.append( " c\r\n" );
    m_aPages.back().appendPoint( aPoints[6], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[7], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[8], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[9], aLine );
    aLine.append( " c\r\n" );
    m_aPages.back().appendPoint( aPoints[10], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[11], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[12], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[13], aLine );
    aLine.append( " c\r\n" );
    m_aPages.back().appendPoint( aPoints[14], aLine );
    aLine.append( " l " );
    m_aPages.back().appendPoint( aPoints[15], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[0], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " c " );

    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "b*\r\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "s\r\n" );
    else
        aLine.append( "f*\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawEllipse( const Rectangle& rRect )
{
    MARK( "drawEllipse" );

    updateGraphicsState();

    Point aPoints[12];
    const double kappa = 0.5522847498;
    const sal_uInt32 kx = (sal_uInt32)((kappa*(double)rRect.GetWidth()/2.0)+0.5);
    const sal_uInt32 ky = (sal_uInt32)((kappa*(double)rRect.GetHeight()/2.0)+0.5);

    aPoints[1]  = Point( rRect.TopLeft().X() + rRect.GetWidth()/2, rRect.TopLeft().Y() );
    aPoints[0]  = Point( aPoints[1].X() - kx, aPoints[1].Y() );
    aPoints[2]  = Point( aPoints[1].X() + kx, aPoints[1].Y() );

    aPoints[4]  = Point( rRect.TopRight().X()+1, rRect.TopRight().Y() + rRect.GetHeight()/2 );
    aPoints[3]  = Point( aPoints[4].X(), aPoints[4].Y() - ky );
    aPoints[5]  = Point( aPoints[4].X(), aPoints[4].Y() + ky );

    aPoints[7]  = Point( rRect.BottomLeft().X() + rRect.GetWidth()/2, rRect.BottomLeft().Y()+1 );
    aPoints[6]  = Point( aPoints[7].X() + kx, aPoints[7].Y() );
    aPoints[8]  = Point( aPoints[7].X() - kx, aPoints[7].Y() );

    aPoints[10] = Point( rRect.TopLeft().X(), rRect.TopLeft().Y() + rRect.GetHeight()/2 );
    aPoints[9]  = Point( aPoints[10].X(), aPoints[10].Y() + ky );
    aPoints[11] = Point( aPoints[10].X(), aPoints[10].Y() - ky );

    OStringBuffer aLine( 80 );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( aPoints[2], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[3], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[4], aLine );
    aLine.append( " c\r\n" );
    m_aPages.back().appendPoint( aPoints[5], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[6], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[7], aLine );
    aLine.append( " c\r\n" );
    m_aPages.back().appendPoint( aPoints[8], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[9], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[10], aLine );
    aLine.append( " c\r\n" );
    m_aPages.back().appendPoint( aPoints[11], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[0], aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPoints[1], aLine );
    aLine.append( " c " );

    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "b*\r\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "s\r\n" );
    else
        aLine.append( "f*\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

static double calcAngle( const Rectangle& rRect, const Point& rPoint )
{
    Point aOrigin((rRect.TopLeft().X()+rRect.BottomRight().X()+1)/2,
                  (rRect.TopLeft().Y()+rRect.BottomRight().Y()+1)/2);
    Point aPoint = rPoint - aOrigin;
    aPoint.Y() = -aPoint.Y();
    if( rRect.GetWidth() > rRect.GetHeight() )
        aPoint.Y() = aPoint.Y()*rRect.GetWidth()/rRect.GetHeight();
    else if( rRect.GetHeight() > rRect.GetWidth() )
        aPoint.X() = aPoint.X()*rRect.GetHeight()/rRect.GetWidth();
    return atan2( (double)aPoint.Y(), (double)aPoint.X() );
}

void PDFWriterImpl::drawArc( const Rectangle& rRect, const Point& rStart, const Point& rStop, bool bWithPie, bool bWithChord )
{
    MARK( "drawArc" );

    updateGraphicsState();

    // calculate start and stop angles
    double fStartAngle = calcAngle( rRect, rStart );
    double fStopAngle  = calcAngle( rRect, rStop );
    while( fStopAngle < fStartAngle )
        fStopAngle += 2.0*M_PI;
    int nFragments = (int)((fStopAngle-fStartAngle)/(M_PI/2.0))+1;
    double fFragmentDelta = (fStopAngle-fStartAngle)/(double)nFragments;
    double kappa = fabs( 4.0 * (1.0-cos(fFragmentDelta/2.0))/sin(fFragmentDelta/2.0) / 3.0);
    double halfWidth = (double)rRect.GetWidth()/2.0;
    double halfHeight = (double)rRect.GetHeight()/2.0;

    Point aCenter( (rRect.TopLeft().X()+rRect.BottomRight().X()+1)/2,
                   (rRect.TopLeft().Y()+rRect.BottomRight().Y()+1)/2 );

    OStringBuffer aLine( 30*nFragments );
    Point aPoint( (int)(halfWidth * cos(fStartAngle) ),
                  -(int)(halfHeight * sin(fStartAngle) ) );
    aPoint += aCenter;
    Point aStart = aPoint;
    m_aPages.back().appendPoint( aPoint, aLine );
    aLine.append( " m " );
    for( int i = 0; i < nFragments; i++ )
    {
        double fStartFragment = fStartAngle + (double)i*fFragmentDelta;
        double fStopFragment = fStartFragment + fFragmentDelta;
        aPoint = Point( (int)(halfWidth * (cos(fStartFragment) - kappa*sin(fStartFragment) ) ),
                        -(int)(halfHeight * (sin(fStartFragment) + kappa*cos(fStartFragment) ) ) );
        aPoint += aCenter;
        m_aPages.back().appendPoint( aPoint, aLine );
        aLine.append( ' ' );

        aPoint = Point( (int)(halfWidth * (cos(fStopFragment) + kappa*sin(fStopFragment) ) ),
                        -(int)(halfHeight * (sin(fStopFragment) - kappa*cos(fStopFragment) ) ) );
        aPoint += aCenter;
        m_aPages.back().appendPoint( aPoint, aLine );
        aLine.append( ' ' );

        aPoint = Point( (int)(halfWidth * cos(fStopFragment) ),
                        -(int)(halfHeight * sin(fStopFragment) ) );
        aPoint += aCenter;
        m_aPages.back().appendPoint( aPoint, aLine );
        aLine.append( " c\r\n" );
    }
    if( bWithPie )
    {
        m_aPages.back().appendPoint( aCenter, aLine );
        aLine.append( " l " );
    }
    if( ! bWithChord && ! bWithPie )
        aLine.append( "S\r\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor != Color( COL_TRANSPARENT ) )
        aLine.append( "b*\r\n" );
    else if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "s\r\n" );
    else
        aLine.append( "f*\r\n" );

#ifdef DEBUG_ARC
    aLine.append( "0 1 1 RG " );
    m_aPages.back().appendPoint( rStart, aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( aCenter, aLine );
    aLine.append( " l S 1 0 0 RG " );
    m_aPages.back().appendPoint( aCenter, aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( rStop, aLine );
    aLine.append( " l S\r\n" );
    aLine.append( "0 0 0 RG " );
    m_aPages.back().appendRect( rRect );
    aLine.append( " S\r\n" );
#endif

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyLine( const Polygon& rPoly )
{
    MARK( "drawPolyLine" );

    updateGraphicsState();

    int nPoints = rPoly.GetSize();
    OStringBuffer aLine( 20 * nPoints );
    m_aPages.back().appendPolygon( rPoly, aLine, false );
    aLine.append( "S\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyLine( const Polygon& rPoly, const LineInfo& rInfo )
{
    MARK( "drawPolyLine with LineInfo" );

    updateGraphicsState();

    OStringBuffer aLine;
    aLine.append( "q " );
    m_aPages.back().appendLineInfo( rInfo,aLine );
    writeBuffer( aLine.getStr(), aLine.getLength() );
    drawPolyLine( rPoly );
    writeBuffer( "Q\r\n", 3 );
}

void PDFWriterImpl::drawPixel( const Point& rPoint, const Color& rColor )
{
    MARK( "drawPixel" );

    Color aColor = ( rColor == Color( COL_TRANSPARENT ) ? m_aGraphicsStack.front().m_aLineColor : rColor );

    // pixels are drawn in line color, so have to set
    // the nonstroking color to line color
    Color aOldFillColor = m_aGraphicsStack.front().m_aFillColor;
    setFillColor( aColor );

    updateGraphicsState();

    OStringBuffer aLine( 20 );
    m_aPages.back().appendPoint( rPoint, aLine );
    aLine.append( " 1 1 re f\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    setFillColor( aOldFillColor );
}

void PDFWriterImpl::drawPixel( const Polygon& rPoints, const Color* pColors )
{
    MARK( "drawPixel with Polygon" );

    updateGraphicsState();

    int nPoints = rPoints.GetSize();
    OStringBuffer aLine( nPoints*40 );
    aLine.append( "q " );
    if( ! pColors )
    {
        appendNonStrokingColor( m_aGraphicsStack.front().m_aLineColor, aLine );
        aLine.append( ' ' );
    }

    for( int i = 0; i < nPoints; i++ )
    {
        if( pColors )
        {
            appendNonStrokingColor( pColors[i], aLine );
            aLine.append( ' ' );
        }
        m_aPages.back().appendPoint( rPoints[i], aLine );
        aLine.append( " 1 1 re f\r\n" );
    }
    aLine.append( "Q\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

class AccessReleaser
{
    BitmapReadAccess* m_pAccess;
public:
    AccessReleaser( BitmapReadAccess* pAccess ) : m_pAccess( pAccess ){}
    ~AccessReleaser() { delete m_pAccess; }
};

bool PDFWriterImpl::writeGradientFunction( GradientEmit& rObject )
{
    sal_Int32 nFunctionObject = createObject();
    CHECK_RETURN( updateObject( nFunctionObject ) );

    OutputDevice* pRefDevice = getReferenceDevice();
    if( rObject.m_aSize.Width() > pRefDevice->GetOutputSizePixel().Width() )
        rObject.m_aSize.Width() = pRefDevice->GetOutputSizePixel().Width();
    if( rObject.m_aSize.Height() > pRefDevice->GetOutputSizePixel().Height() )
        rObject.m_aSize.Height() = pRefDevice->GetOutputSizePixel().Height();
    pRefDevice->SetMapMode( MapMode( MAP_PIXEL ) );
    pRefDevice->DrawGradient( Rectangle( Point( 0, 0 ), rObject.m_aSize ), rObject.m_aGradient );

    Bitmap aSample = pRefDevice->GetBitmap( Point( 0, 0 ), rObject.m_aSize );
    BitmapReadAccess* pAccess = aSample.AcquireReadAccess();
    AccessReleaser aReleaser( pAccess );

    Size aSize = aSample.GetSizePixel();

    OStringBuffer aLine( 120 );
    aLine.append( nFunctionObject );
    aLine.append( " 0 obj\r\n  << /FunctionType 0\r\n     /Domain [ 0 1 0 1 ]\r\n" );
    aLine.append( "     /Size [ " );
    aLine.append( (sal_Int32)aSize.Width() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)aSize.Height() );
    aLine.append( " ]\r\n     /BitsPerSample 8\r\n     /Range [ 0 1 0 1 0 1 ]\r\n" );
    aLine.append( "     /Length " );
    aLine.append( (sal_Int32)(aSize.Width()*aSize.Height()*3) );
    aLine.append( "\r\n  >>\r\nstream\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    for( int y = 0; y < aSize.Height(); y++ )
    {
        for( int x = 0; x < aSize.Width(); x++ )
        {
            sal_uInt8 aCol[3];
            BitmapColor aColor = pAccess->GetColor( y, x );
            aCol[0] = aColor.GetRed();
            aCol[1] = aColor.GetGreen();
            aCol[2] = aColor.GetBlue();
            CHECK_RETURN( writeBuffer( aCol, 3 ) );
        }
    }
    aLine.setLength( 0 );
    aLine.append( "\r\nendstream\r\nendobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    CHECK_RETURN( updateObject( rObject.m_nObject ) );
    aLine.setLength( 0 );
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\r\n  << /ShadingType 1\r\n     /ColorSpace /DeviceRGB\r\n" );
    aLine.append( "     /AntiAlias true\r\n     /Domain [ 0 1 0 1 ]\r\n" );
    aLine.append( "     /Matrix [ " );
    aLine.append( (sal_Int32)aSize.Width() );
    aLine.append( " 0 0 " );
    aLine.append( (sal_Int32)aSize.Height() );
    aLine.append( " 0 0 ]\r\n     /Function " );
    aLine.append( nFunctionObject );
    aLine.append( " 0 R\r\n  >>\r\nendobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return true;
}

bool PDFWriterImpl::writeBitmapObject( BitmapEmit& rObject, bool bMask )
{
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    Bitmap  aBitmap = ( bMask ? rObject.m_aBitmap.GetMask() : rObject.m_aBitmap.GetBitmap() );
    Color   aTransparentColor( COL_TRANSPARENT );
    bool    bDrawMask = false;
    if( ! bMask )
    {
        switch( rObject.m_aBitmap.GetTransparentType() )
        {
            case TRANSPARENT_NONE:
                // comes from drawMask
                if( aBitmap.GetBitCount() == 1 &&
                    rObject.m_aBitmap.GetTransparentColor() == Color(COL_BLACK) )
                    bMask = true;
                break;
            case TRANSPARENT_COLOR:
                aTransparentColor = rObject.m_aBitmap.GetTransparentColor();
                break;
            case TRANSPARENT_BITMAP:
                bDrawMask = true;
                break;
        }
    }
    else
    {
            if( aBitmap.GetBitCount() > 1 )
                aBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            DBG_ASSERT( aBitmap.GetBitCount() == 1, "mask conversion failed" );
    }

    BitmapReadAccess* pAccess = aBitmap.AcquireReadAccess();
    AccessReleaser aReleaser( pAccess );

    bool bTrueColor;
    sal_Int32 nBitsPerComponent;
    switch( aBitmap.GetBitCount() )
    {
        case 1:
        case 2:
        case 4:
        case 8:
            bTrueColor = false;
            nBitsPerComponent = aBitmap.GetBitCount();
            DBG_ASSERT( pAccess->GetScanlineSize() == pAccess->GetBitCount()*pAccess->Width()/8, "wrong scanline size" );
            break;
        default:
            bTrueColor = true;
            nBitsPerComponent = 8;
            break;
    }

    sal_Int32 nStreamLengthObject   = createObject();
    sal_Int32 nMaskObject           = 0;

    OStringBuffer aLine(80);
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\r\n  << /Type /XObject\r\n     /Subtype /Image\r\n     /Width " );
    aLine.append( (sal_Int32)aBitmap.GetSizePixel().Width() );
    aLine.append( "\r\n     /Height " );
    aLine.append( (sal_Int32)aBitmap.GetSizePixel().Height() );
    aLine.append( "\r\n     /BitsPerComponent " );
    aLine.append( nBitsPerComponent );
    aLine.append( "\r\n     /Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R\r\n" );

    if( ! bMask )
    {
        aLine.append( "     /ColorSpace " );
        if( bTrueColor )
            aLine.append( "/DeviceRGB\r\n" );
        else
        {
            aLine.append( "[  /Indexed /DeviceRGB " );
            aLine.append( (sal_Int32)(pAccess->GetPaletteEntryCount()-1) );
            aLine.append( " <\r\n" );
            for( int i = 0; i < pAccess->GetPaletteEntryCount(); i++ )
            {
                const BitmapColor& rColor = pAccess->GetPaletteColor( i );
                appendHex( rColor.GetRed(), aLine );
                appendHex( rColor.GetGreen(), aLine );
                appendHex( rColor.GetBlue(), aLine );
                if( (i+1) & 7 )
                    aLine.append( ' ' );
                else
                    aLine.append( "\r\n" );
            }
            aLine.append( "> ]\r\n" );
        }
    }
    else
    {
        aLine.append( "     /ImageMask true\r\n" );
        sal_Int32 nBlackIndex = pAccess->GetBestPaletteIndex( BitmapColor( Color( COL_BLACK ) ) );
        DBG_ASSERT( nBlackIndex == 0 || nBlackIndex == 1, "wrong black index" );
        if( nBlackIndex )
            aLine.append( "     /Decode [ 1 0 ]\r\n" );
        else
            aLine.append( "     /Decode [ 0 1 ]\r\n" );
    }

    if( ! bMask && m_eVersion > PDFWriter::PDF_1_2 )
    {
        if( bDrawMask )
        {
            nMaskObject = createObject();
            aLine.append( "     /Mask " );
            aLine.append( nMaskObject );
            aLine.append( " 0 R\r\n" );
        }
        else if( aTransparentColor != Color( COL_TRANSPARENT ) )
        {
            aLine.append( "    /Mask [ " );
            if( bTrueColor )
            {
                aLine.append( (sal_Int32)aTransparentColor.GetRed() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetRed() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetGreen() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetGreen() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetBlue() );
                aLine.append( ' ' );
                aLine.append( (sal_Int32)aTransparentColor.GetBlue() );
            }
            else
            {
                sal_Int32 nIndex = pAccess->GetBestPaletteIndex( BitmapColor( aTransparentColor ) );
                aLine.append( nIndex );
            }
            aLine.append( " ]\r\n" );
        }
    }
    aLine.append( "  >>\r\nstream\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    sal_uInt64 nStartPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartPos )) );

    if( ! bTrueColor || pAccess->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB )
    {
        for( int i = 0; i < pAccess->Height(); i++ )
        {
            CHECK_RETURN( writeBuffer( pAccess->GetScanline( i ), pAccess->GetScanlineSize() ) );
        }
    }
    else
    {
        const int nScanLineBytes = pAccess->Width()*3;
        sal_uInt8 *pCol = (sal_uInt8*)rtl_allocateMemory( nScanLineBytes );
        for( int y = 0; y < pAccess->Height(); y++ )
        {
            for( int x = 0; x < pAccess->Width(); x++ )
            {
                BitmapColor aColor = pAccess->GetColor( y, x );
                pCol[3*x+0] = aColor.GetRed();
                pCol[3*x+1] = aColor.GetGreen();
                pCol[3*x+2] = aColor.GetBlue();
            }
            CHECK_RETURN( writeBuffer( pCol, nScanLineBytes ) );
        }
        rtl_freeMemory( pCol );
    }

    sal_uInt64 nEndPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nEndPos )) );
    aLine.setLength( 0 );
    aLine.append( "\r\nendstream\r\nendobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    CHECK_RETURN( updateObject( nStreamLengthObject ) );
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\r\n" );
    aLine.append( (sal_Int64)(nEndPos-nStartPos) );
    aLine.append( "\r\nendobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    if( nMaskObject )
    {
        BitmapEmit aEmit;
        aEmit.m_nObject             = nMaskObject;
        aEmit.m_aBitmap             = rObject.m_aBitmap;
        return writeBitmapObject( aEmit, true );
    }

    return true;
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEmit& rBitmap, const Color& rFillColor )
{
    OStringBuffer aLine( 80 );
    updateGraphicsState();

    aLine.append( "q " );
    if( rFillColor != Color( COL_TRANSPARENT ) )
    {
        appendNonStrokingColor( rFillColor, aLine );
        aLine.append( ' ' );
    }
    m_aPages.back().appendMappedLength( rDestSize.Width(), aLine, false );
    aLine.append( " 0 0 " );
    m_aPages.back().appendMappedLength( rDestSize.Height(), aLine, true );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rDestPoint + Point( 0, rDestSize.Height()-1 ), aLine );
    aLine.append( " cm\r\n  /Im" );
    aLine.append( rBitmap.m_nObject );
    aLine.append( " Do Q\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap )
{
    MARK( "drawBitmap (Bitmap)" );

    m_aBitmaps.push_back( BitmapEmit() );
    m_aBitmaps.back().m_nObject             = createObject();
    m_aBitmaps.back().m_aBitmap             = BitmapEx( rBitmap );

    drawBitmap( rDestPoint, rDestSize, m_aBitmaps.back(), Color( COL_TRANSPARENT ) );
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEx& rBitmap )
{
    MARK( "drawBitmap (BitmapEx)" );

    m_aBitmaps.push_back( BitmapEmit() );
    m_aBitmaps.back().m_nObject             = createObject();
    m_aBitmaps.back().m_aBitmap             = rBitmap;
    drawBitmap( rDestPoint, rDestSize, m_aBitmaps.back(), Color( COL_TRANSPARENT ) );
}

void PDFWriterImpl::drawMask( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap, const Color& rFillColor )
{
    MARK( "drawMask" );

    Bitmap aBitmap( rBitmap );
    if( aBitmap.GetBitCount() > 1 )
        aBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
    DBG_ASSERT( aBitmap.GetBitCount() == 1, "mask conversion failed" );

    m_aBitmaps.push_back( BitmapEmit() );
    m_aBitmaps.back().m_nObject             = createObject();
    m_aBitmaps.back().m_aBitmap             = aBitmap;
    m_aBitmaps.back().m_aBitmap.SetTransparentColor( Color( COL_BLACK ) );

    drawBitmap( rDestPoint, rDestSize, m_aBitmaps.back(), rFillColor );
}

sal_Int32 PDFWriterImpl::createGradient( const Gradient& rGradient, const Size& rSize )
{
    Size aPtSize = OutputDevice::LogicToLogic( rSize, m_aGraphicsStack.front().m_aMapMode, MapMode( MAP_POINT ) );
    // check if we already have this gradient
    for( std::list<GradientEmit>::iterator it = m_aGradients.begin(); it != m_aGradients.end(); ++it )
    {
        if( it->m_aGradient == rGradient )
        {
            if( it->m_aSize.Width() < aPtSize.Width() )
                it->m_aSize.Width() = aPtSize.Width();
            if( it->m_aSize.Height() <= aPtSize.Height() )
                it->m_aSize.Height() = aPtSize.Height();
            return it->m_nObject;
        }
    }

    m_aGradients.push_back( GradientEmit() );
    m_aGradients.back().m_aGradient = rGradient;
    m_aGradients.back().m_nObject   = createObject();
    m_aGradients.back().m_aSize     = aPtSize;
    return m_aGradients.back().m_nObject;
}

void PDFWriterImpl::drawGradient( const Rectangle& rRect, const Gradient& rGradient )
{
    MARK( "drawGradient (Rectangle)" );

    if( m_eVersion == PDFWriter::PDF_1_2 )
    {
        drawRectangle( rRect );
        return;
    }

    sal_Int32 nGradient = createGradient( rGradient, rRect.GetSize() );

    Point aTranslate( rRect.BottomLeft() );
    aTranslate += Point( 0, 1 );

    updateGraphicsState();

    OStringBuffer aLine( 80 );
    aLine.append( "q 1 0 0 1 " );
    m_aPages.back().appendPoint( aTranslate, aLine );
    aLine.append( " cm " );
    // if a stroke is appended reset the clip region before stroke
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( "q " );
    aLine.append( "0 0 " );
    m_aPages.back().appendMappedLength( rRect.GetWidth(), aLine, false );
    aLine.append( ' ' );
    m_aPages.back().appendMappedLength( rRect.GetHeight(), aLine, true );
    aLine.append( " re W n\r\n" );

    aLine.append( "/P" );
    aLine.append( nGradient );
    aLine.append( " sh " );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
    {
        aLine.append( "Q 0 0 " );
        m_aPages.back().appendMappedLength( rRect.GetWidth(), aLine, false );
        aLine.append( ' ' );
        m_aPages.back().appendMappedLength( rRect.GetHeight(), aLine, true );
        aLine.append( " re S " );
    }
    aLine.append( "Q\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawGradient( const PolyPolygon& rPolyPoly, const Gradient& rGradient )
{
    MARK( "drawGradient (PolyPolygon)" );

    if( m_eVersion == PDFWriter::PDF_1_2 )
    {
        drawPolyPolygon( rPolyPoly );
        return;
    }

    sal_Int32 nGradient = createGradient( rGradient, rPolyPoly.GetBoundRect().GetSize() );

    updateGraphicsState();

    Rectangle aBoundRect = rPolyPoly.GetBoundRect();
    Point aTranslate = aBoundRect.BottomLeft() + Point( 0, 1 );
    int nPolygons = rPolyPoly.Count();

    OStringBuffer aLine( 80*nPolygons );
    aLine.append( "q " );
    // set PolyPolygon as clip path
    m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
    aLine.append( "W* n\r\n" );
    aLine.append( "1 0 0 1 " );
    m_aPages.back().appendPoint( aTranslate, aLine );
    aLine.append( " cm\r\n" );
    aLine.append( "/P" );
    aLine.append( nGradient );
    aLine.append( " sh Q\r\n" );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
    {
        // and draw the surrounding path
        m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
        aLine.append( "S\r\n" );
    }
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

sal_Int32 PDFWriterImpl::createHatch( const Hatch& rHatch )
{
    // check if we already have this gradient
    for( std::list<HatchEmit>::iterator it = m_aHatches.begin(); it != m_aHatches.end(); ++it )
    {
        if( it->m_aHatch == rHatch )
            return it->m_nObject;
    }

    m_aHatches.push_back( HatchEmit() );
    m_aHatches.back().m_aHatch  = rHatch;
    m_aHatches.back().m_nObject = createObject();
    return m_aHatches.back().m_nObject;
}

void PDFWriterImpl::drawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    MARK( "drawHatch" );

    sal_Int32 nHatch = createHatch( rHatch );

    updateGraphicsState();

    OStringBuffer aLine( 256 );
    aLine.append( "q /Pattern cs /P" );
    aLine.append( nHatch );
    aLine.append( " scn\r\n" );
    m_aPages.back().appendPolyPolygon( rPolyPoly, aLine );
    if( m_aGraphicsStack.front().m_aLineColor != Color( COL_TRANSPARENT ) )
        aLine.append( " B* " );
    else
        aLine.append( " f* " );
    aLine.append( "Q\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawWallpaper( const Rectangle& rRect, const Wallpaper& rWall )
{
    MARK( "drawWallpaper" );

    bool bDrawColor         = false;
    bool bDrawGradient      = false;
    bool bDrawBitmap        = false;

    BitmapEx aBitmap;
    Point aBmpPos = rRect.TopLeft();
    Size aBmpSize;
    if( rWall.IsBitmap() )
    {
        aBitmap = rWall.GetBitmap();
        aBmpSize = OutputDevice::LogicToLogic( aBitmap.GetPrefSize(),
                                               aBitmap.GetPrefMapMode(),
                                               getMapMode() );
        Rectangle aRect( rRect );
        if( rWall.IsRect() )
        {
            aRect = rWall.GetRect();
            aBmpPos = aRect.TopLeft();
            aBmpSize = aRect.GetSize();
        }
        if( rWall.GetStyle() != WALLPAPER_SCALE )
        {
            if( rWall.GetStyle() != WALLPAPER_TILE )
            {
                bDrawBitmap     = true;
                if( rWall.IsGradient() )
                    bDrawGradient = true;
                else
                    bDrawColor = true;
                switch( rWall.GetStyle() )
                {
                    case WALLPAPER_TOPLEFT:
                        break;
                    case WALLPAPER_TOP:
                        aBmpPos.X() += (aRect.GetWidth()-aBmpSize.Width())/2;
                        break;
                    case WALLPAPER_LEFT:
                        aBmpPos.Y() += (aRect.GetHeight()-aBmpSize.Height())/2;
                        break;
                    case WALLPAPER_TOPRIGHT:
                        aBmpPos.X() += aRect.GetWidth()-aBmpSize.Width();
                        break;
                    case WALLPAPER_CENTER:
                        aBmpPos.X() += (aRect.GetWidth()-aBmpSize.Width())/2;
                        aBmpPos.Y() += (aRect.GetHeight()-aBmpSize.Height())/2;
                        break;
                    case WALLPAPER_RIGHT:
                        aBmpPos.X() += aRect.GetWidth()-aBmpSize.Width();
                        aBmpPos.Y() += (aRect.GetHeight()-aBmpSize.Height())/2;
                        break;
                    case WALLPAPER_BOTTOMLEFT:
                        aBmpPos.Y() += aRect.GetHeight()-aBmpSize.Height();
                        break;
                    case WALLPAPER_BOTTOM:
                        aBmpPos.X() += (aRect.GetWidth()-aBmpSize.Width())/2;
                        aBmpPos.Y() += aRect.GetHeight()-aBmpSize.Height();
                        break;
                    case WALLPAPER_BOTTOMRIGHT:
                        aBmpPos.X() += aRect.GetWidth()-aBmpSize.Width();
                        aBmpPos.Y() += aRect.GetHeight()-aBmpSize.Height();
                        break;
                }
            }
            else
            {
                // push the bitmap
                m_aBitmaps.push_back( BitmapEmit() );
                m_aBitmaps.back().m_nObject = createObject();
                m_aBitmaps.back().m_aBitmap = aBitmap;

                // convert to internal mapmode (10th point)
                Rectangle aConvertRect = OutputDevice::LogicToLogic( Rectangle( aBmpPos, aBmpSize ),
                                                                     getMapMode(),
                                                                     m_aMapMode );
                // convert to page coordinates; this needs to be done here
                // since the emit does not know the page anymore
                sal_Int32 nMirror = m_aPages.back().m_nPageHeight ? m_aPages.back().m_nPageHeight : m_nInheritedPageHeight;
                aConvertRect = Rectangle( Point( aConvertRect.BottomLeft().X(), 10*nMirror-aConvertRect.BottomLeft().Y() ),
                                          aConvertRect.GetSize() );

                // push the pattern
                m_aTilings.push_back( BitmapPatternEmit() );
                m_aTilings.back().m_nObject         = createObject();
                m_aTilings.back().m_nBitmapObject   = m_aBitmaps.back().m_nObject;
                m_aTilings.back().m_aRectangle      = aConvertRect;

                updateGraphicsState();

                // fill a rRect with the pattern
                OStringBuffer aLine( 100 );
                aLine.append( "q /Pattern cs /P" );
                aLine.append( m_aTilings.back().m_nObject );
                aLine.append( " scn " );
                m_aPages.back().appendRect( rRect, aLine );
                aLine.append( " f Q\r\n" );
                writeBuffer( aLine.getStr(), aLine.getLength() );
            }
        }
        else
        {
            aBmpPos     = aRect.TopLeft();
            aBmpSize    = aRect.GetSize();
            bDrawBitmap = true;
        }

        if( aBitmap.IsTransparent() )
        {
            if( rWall.IsGradient() )
                bDrawGradient = true;
            else
                bDrawColor = true;
        }
    }
    else if( rWall.IsGradient() )
        bDrawGradient = true;
    else
        bDrawColor = true;

    if( bDrawGradient )
    {
        drawGradient( rRect, rWall.GetGradient() );
    }
    if( bDrawColor )
    {
        Color aOldLineColor = m_aGraphicsStack.front().m_aLineColor;
        Color aOldFillColor = m_aGraphicsStack.front().m_aFillColor;
        setLineColor( Color( COL_TRANSPARENT ) );
        setFillColor( rWall.GetColor() );
        drawRectangle( rRect );
        setLineColor( aOldLineColor );
        setFillColor( aOldFillColor );
    }
    if( bDrawBitmap )
    {
        // set temporary clip region since aBmpPos and aBmpSize
        // may be outside rRect
        OStringBuffer aLine( 20 );
        aLine.append( "q " );
        m_aPages.back().appendRect( rRect, aLine );
        aLine.append( " W n\r\n" );
        writeBuffer( aLine.getStr(), aLine.getLength() );
        drawBitmap( aBmpPos, aBmpSize, aBitmap );
        writeBuffer( "Q\r\n", 3 );
    }
}

void PDFWriterImpl::updateGraphicsState()
{
    OStringBuffer aLine( 256 );
    GraphicsState& rNewState = m_aGraphicsStack.front();
    // first set clip region since it might invalidate everything else
    Region& rNewClip = rNewState.m_aClipRegion;
    if( m_aCurrentPDFState.m_aClipRegion != rNewClip )
    {
        if( ! m_aCurrentPDFState.m_aClipRegion.IsEmpty() &&
            ! m_aCurrentPDFState.m_aClipRegion.IsNull() )
        {
            aLine.append( "Q " );
            // invalidate everything
            m_aCurrentPDFState = GraphicsState();
        }
        if( ! rNewClip.IsEmpty() && ! rNewClip.IsNull() )
        {
            aLine.append( "q " );
            if( rNewClip.HasPolyPolygon() )
            {
                m_aPages.back().appendPolyPolygon( rNewClip.GetPolyPolygon(), aLine );
                aLine.append( "W* n\r\n" );
            }
            else
            {
                // need to clip all rectangles
                RegionHandle aHandle = rNewClip.BeginEnumRects();
                Rectangle aRect;
                while( rNewClip.GetNextEnumRect( aHandle, aRect ) )
                {
                    m_aPages.back().appendRect( aRect, aLine );
                    if( aLine.getLength() > 80 )
                    {
                        aLine.append( "\r\n" );
                        writeBuffer( aLine.getStr(), aLine.getLength() );
                        aLine.setLength( 0 );
                    }
                    else
                        aLine.append( ' ' );
            }
                rNewClip.EndEnumRects( aHandle );
                aLine.append( "W* n\r\n" );
            }
        }
    }

    if( m_aCurrentPDFState.m_aLineColor != rNewState.m_aLineColor &&
        rNewState.m_aLineColor != Color( COL_TRANSPARENT ) )
    {
        appendStrokingColor( rNewState.m_aLineColor, aLine );
        aLine.append( "\r\n" );
    }

    if( m_aCurrentPDFState.m_aFillColor != rNewState.m_aFillColor &&
        rNewState.m_aFillColor != Color( COL_TRANSPARENT ) )
    {
        appendNonStrokingColor( rNewState.m_aFillColor, aLine );
        aLine.append( "\r\n" );
    }

    if( m_eVersion >= PDFWriter::PDF_1_4 && m_aCurrentPDFState.m_nTransparentPercent != rNewState.m_nTransparentPercent )
    {
        // TODO: switch extended graphicsstate
    }

    // everything is up to date now
    m_aCurrentPDFState = m_aGraphicsStack.front();
    if( aLine.getLength() )
        writeBuffer( aLine.getStr(), aLine.getLength() );
}
