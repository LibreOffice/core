/*************************************************************************
 *
 *  $RCSfile: pdfwriter_impl.cxx,v $
 *
 *  $Revision: 1.55 $
 *
 *  last change: $Author: vg $ $Date: 2003-07-21 11:21:49 $
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
#include <tools/zcodec.hxx>
#include <tools/stream.hxx>
#include <virdev.hxx>
#include <bmpacc.hxx>
#include <bitmapex.hxx>
#include <image.hxx>
#include <outdev.h>
#include <sallayout.hxx>
#include <metric.hxx>
#ifndef REMOTE_APPSERVER
#include <svsys.h>
#include <salgdi.hxx>
#else
#include <rmoutdev.hxx>
#endif
#include <osl/thread.h>
#include <osl/file.h>
#include <rtl/crc.h>

#include "implncvt.hxx"

#include <math.h>
#ifdef WNT
// Aaarrgh
#define M_PI 3.14159265
#endif

#define ENABLE_COMPRESSION
#if OSL_DEBUG_LEVEL < 2
#define COMPRESS_PAGES
#endif

using namespace vcl;
using namespace rtl;

#if (OSL_DEBUG_LEVEL > 1) || defined DBG_UTIL
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

static void appendName( const OUString& rStr, OStringBuffer& rBuffer )
{
    OString aStr( OUStringToOString( rStr, RTL_TEXTENCODING_UTF8 ) );
    const sal_Char* pStr = aStr.getStr();
    int nLen = aStr.getLength();
    for( int i = 0; i < nLen; i++ )
    {
        /*  #i16920# PDF recommendation: output UTF8, any byte
         *  outside the interval [33(=ASCII'!');126(=ASCII'~')]
         *  should be escaped hexadecimal
         *  for the sake of ghostscript which also reads PDF
         *  but has a narrower acceptance rate we only pass
         *  alphanumerics and '-' literally.
         */
        if( (pStr[i] >= 'A' && pStr[i] <= 'Z' ) ||
            (pStr[i] >= 'a' && pStr[i] <= 'z' ) ||
            (pStr[i] >= '0' && pStr[i] <= '9' ) ||
            pStr[i] == '-' )
        {
            rBuffer.append( pStr[i] );
        }
        else
        {
            rBuffer.append( '#' );
            appendHex( (sal_Int8)pStr[i], rBuffer );
        }
    }
}

static void appendName( const sal_Char* pStr, OStringBuffer& rBuffer )
{
    while( pStr && *pStr )
    {
        if( (*pStr >= 'A' && *pStr <= 'Z' ) ||
            (*pStr >= 'a' && *pStr <= 'z' ) ||
            (*pStr >= '0' && *pStr <= '9' ) ||
            *pStr == '-' )
        {
            rBuffer.append( *pStr );
        }
        else
        {
            rBuffer.append( '#' );
            appendHex( (sal_Int8)*pStr, rBuffer );
        }
        pStr++;
    }
}

static void appendUnicodeTextString( const String& rString, OStringBuffer& rBuffer )
{
    rBuffer.append( "<FEFF" );
    for( int i = 0; i < rString.Len(); i++ )
    {
        sal_Unicode aChar = rString.GetChar(i);
        appendHex( (sal_Int8)(aChar >> 8), rBuffer );
        appendHex( (sal_Int8)(aChar & 255 ), rBuffer );
    }
    rBuffer.append( ">" );
}

// appends a double. PDF does not accept exponential format, only fixed point
static void appendDouble( double fValue, OStringBuffer& rBuffer, int nPrecision = 5 )
{
    bool bNeg = false;
    if( fValue < 0.0 )
    {
        bNeg = true;
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
    sal_Int64 nFrac = 0;
    if( fValue )
    {
        fValue *= pow( 10.0, (double)nPrecision );
        nFrac = (sal_Int64)fValue;
    }
    if( bNeg && ( nInt || nFrac ) )
        rBuffer.append( '-' );
    rBuffer.append( nInt );
    if( nFrac )
    {
        int i;
        rBuffer.append( '.' );
        sal_Int64 nBound = (sal_Int64)(pow( 10, nPrecision - 1 )+0.5);
        for ( i = 0; ( i < nPrecision ) && nFrac; i++ )
        {
            sal_Int64 nNumb = nFrac / nBound;
            nFrac -= nNumb * nBound;
            rBuffer.append( nNumb );
            nBound /= 10;
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
    aLine.append( " 0 obj\r\n<< /Length " );
    aLine.append( m_nStreamLengthObject );
    aLine.append( " 0 R\r\n" );
#if defined COMPRESS_PAGES && defined ENABLE_COMPRESSION
    aLine.append( "   /Filter /FlateDecode\r\n" );
#endif
    aLine.append( ">>\r\nstream\r\n" );
    if( ! m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() ) )
        return;
    if( osl_File_E_None != osl_getFilePos( m_pWriter->m_aFile, &m_nBeginStreamPos ) )
    {
        osl_closeFile( m_pWriter->m_aFile );
        m_pWriter->m_bOpen = false;
    }
#if defined COMPRESS_PAGES && defined ENABLE_COMPRESSION
    m_pWriter->beginCompression();
#endif
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
    aLine.append( " 0 obj\r\n"
                  "<< /Type /Page\r\n"
                  "   /Parent " );
    aLine.append( nParentObject );
    aLine.append( " 0 R\r\n" );
    if( m_nPageWidth && m_nPageHeight )
    {
        aLine.append( "   /MediaBox [ 0 0 " );
        aLine.append( m_nPageWidth );
        aLine.append( ' ' );
        aLine.append( m_nPageHeight );
        aLine.append( " ]\r\n" );
    }
    switch( m_eOrientation )
    {
        case PDFWriter::Landscape: aLine.append( "   /Rotate 90\r\n" );break;
        case PDFWriter::Seascape: aLine.append( "   /Rotate -90\r\n" );break;
        case PDFWriter::Portrait: aLine.append( "   /Rotate 0\r\n" );break;

        case PDFWriter::Inherit:
        default:
            break;
    }
    aLine.append( "   /Contents " );
    aLine.append( m_nStreamObject );
    aLine.append( " 0 R\r\n>>\r\nendobj\r\n\r\n" );
    return m_pWriter->writeBuffer( aLine.getStr(), aLine.getLength() );
}

namespace vcl
{
template < class GEOMETRY >
GEOMETRY lcl_convert( const MapMode& _rSource, const MapMode& _rDest, OutputDevice* _pPixelConversion, const GEOMETRY& _rObject )
{
    GEOMETRY aPoint;
    if ( MAP_PIXEL == _rSource.GetMapUnit() )
    {
        aPoint = _pPixelConversion->PixelToLogic( _rObject, _rDest );
    }
    else
    {
        aPoint = OutputDevice::LogicToLogic( _rObject, _rSource, _rDest );
    }
    return aPoint;
}
}

void PDFWriterImpl::PDFPage::appendPoint( const Point& rPoint, OStringBuffer& rBuffer, bool bNeg )
{
    Point aPoint( lcl_convert(
                        m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                        m_pWriter->m_aMapMode,
                        m_pWriter->getReferenceDevice(),
                        rPoint
                    ) );

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

void PDFWriterImpl::PDFPage::convertRect( Rectangle& rRect )
{
    Rectangle aConvertRect( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                                         m_pWriter->m_aMapMode,
                                         m_pWriter->getReferenceDevice(),
                                         rRect
                                         ) );
    sal_Int32 nMirror = m_nPageHeight ? m_nPageHeight : m_pWriter->m_nInheritedPageHeight;
    rRect = Rectangle( Point( aConvertRect.BottomLeft().X(), 10*nMirror-aConvertRect.BottomLeft().Y() ),
                       aConvertRect.GetSize() );
}

void PDFWriterImpl::PDFPage::appendPolygon( const Polygon& rPoly, OStringBuffer& rBuffer, bool bClose )
{
    int nPoints = rPoly.GetSize();
    /*
     *  #108582# applications do weird things
     */
    if( nPoints > 0 )
    {
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
}

void PDFWriterImpl::PDFPage::appendPolyPolygon( const PolyPolygon& rPolyPoly, OStringBuffer& rBuffer, bool bClose )
{
    int nPolygons = rPolyPoly.Count();
    for( int n = 0; n < nPolygons; n++ )
        appendPolygon( rPolyPoly[n], rBuffer, bClose );
}

void PDFWriterImpl::PDFPage::appendMappedLength( sal_Int32 nLength, OStringBuffer& rBuffer, bool bVertical )
{
    if ( nLength < 0 )
    {
        rBuffer.append( '-' );
        nLength = -nLength;
    }

    Size aSize( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter->getReferenceDevice(),
                             Size( nLength, nLength ) ) );
    sal_Int32 nInt      = ( bVertical ? aSize.Height() : aSize.Width() ) / 10;
    sal_Int32 nDecimal  = ( bVertical ? aSize.Height() : aSize.Width() ) % 10;

    rBuffer.append( nInt );
    if( nDecimal )
    {
        rBuffer.append( '.' );
        rBuffer.append( nDecimal );
    }
}

void PDFWriterImpl::PDFPage::appendMappedLength( double fLength, OStringBuffer& rBuffer, bool bVertical )
{
    Size aSize( lcl_convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter->getReferenceDevice(),
                             Size( 1000, 1000 ) ) );
    fLength *= (double)(bVertical ? aSize.Height() : aSize.Width()) / 10000.0;
    appendDouble( fLength, rBuffer );
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
    else if( rInfo.GetWidth() == 0 )
        rBuffer.append( "0 w\r\n" );
}

void PDFWriterImpl::PDFPage::appendWaveLine( sal_Int32 nWidth, sal_Int32 nY, sal_Int32 nDelta, OStringBuffer& rBuffer )
{
    if( nWidth <= 0 )
        return;
    if( nDelta < 1 )
        nDelta = 1;

    rBuffer.append( "0 " );
    appendMappedLength( nY, rBuffer, true );
    rBuffer.append( " m\r\n" );
    for( sal_Int32 n = 0; n < nWidth; )
    {
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nDelta+nY, rBuffer, true );
        rBuffer.append( ' ' );
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nY, rBuffer, true );
        rBuffer.append( " v " );
        if( n < nWidth )
        {
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY-nDelta, rBuffer, true );
            rBuffer.append( ' ' );
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY, rBuffer, true );
            rBuffer.append( " v\r\n" );
        }
    }
    rBuffer.append( "S\r\n" );
}

/*
 *  class PDFWriterImpl
 */

PDFWriterImpl::PDFWriterImpl( const OUString& rFilename, PDFWriter::PDFVersion eVersion, PDFWriter::Compression eCompression )
        :
        m_pReferenceDevice( NULL ),
        m_aMapMode( MAP_POINT, Point(), Fraction( 1L, 10L ), Fraction( 1L, 10L ) ),
        m_nNextFID( 1 ),
        m_nInheritedPageWidth( 595 ),  // default A4
        m_nInheritedPageHeight( 842 ), // default A4
        m_eInheritedOrientation( PDFWriter::Portrait ),
        m_nCurrentPage( -1 ),
        m_eVersion( eVersion ),
        m_eCompression( eCompression ),
        m_aFileName( rFilename ),
        m_pCodec( NULL )
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

    m_bOpen = true;
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
        m_bOpen = false;
        return;
    }
}

PDFWriterImpl::~PDFWriterImpl()
{
    delete static_cast<VirtualDevice*>(m_pReferenceDevice);
}

void PDFWriterImpl::setDocInfo( const PDFDocInfo& rInfo )
{
    m_aDocInfo.Title                = rInfo.Title;
    m_aDocInfo.Author               = rInfo.Author;
    m_aDocInfo.Subject              = rInfo.Subject;
    m_aDocInfo.Keywords             = rInfo.Keywords;
    m_aDocInfo.Creator              = rInfo.Creator;
    m_aDocInfo.Producer             = rInfo.Producer;
}

void PDFWriterImpl::emitComment( const OString& rComment )
{
    OStringBuffer aLine( rComment.getLength()+5 );
    aLine.append( "% " );
    aLine.append( rComment );
    aLine.append( "\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::beginCompression()
{
#ifdef ENABLE_COMPRESSION
    m_pCodec = new ZCodec( 0x4000, 0x4000 );
    m_pMemStream = new SvMemoryStream();
    m_pCodec->BeginCompression();
#endif
}

void PDFWriterImpl::endCompression()
{
#ifdef ENABLE_COMPRESSION
    if( m_pCodec )
    {
        m_pCodec->EndCompression();
        delete m_pCodec;
        m_pCodec = NULL;
        sal_uInt64 nLen = m_pMemStream->Tell();
        m_pMemStream->Seek( 0 );
        writeBuffer( m_pMemStream->GetData(), nLen );
        delete m_pMemStream;
        m_pMemStream = NULL;
    }
#endif
}

bool PDFWriterImpl::writeBuffer( const void* pBuffer, sal_uInt64 nBytes )
{
    if( ! m_bOpen ) // we are already down the drain
        return false;

    if( ! nBytes ) // huh ?
        return true;

    sal_uInt64 nWritten;
    if( m_pCodec )
    {
        m_pCodec->Write( *m_pMemStream, static_cast<const BYTE*>(pBuffer), (ULONG)nBytes );
        nWritten = nBytes;
    }
    else
    {
        if( osl_writeFile( m_aFile, pBuffer, nBytes, &nWritten ) != osl_File_E_None )
            nWritten = 0;

        if( nWritten != nBytes )
        {
            osl_closeFile( m_aFile );
            m_bOpen = false;
        }
    }

    return nWritten == nBytes;
}

OutputDevice* PDFWriterImpl::getReferenceDevice()
{
    if( ! m_pReferenceDevice )
    {
        VirtualDevice*  pVDev = new VirtualDevice( 0 );
        sal_Int32       nDPI;

        m_pReferenceDevice = pVDev;

        switch( m_eCompression )
        {
            case( PDFWriter::Print ): nDPI = 1200; break;
            case( PDFWriter::Press ): nDPI = 2400; break;

            default:
                nDPI = 600;
            break;
        }

        pVDev->SetOutputSizePixel( Size( 640, 480 ) );
        pVDev->SetMapMode( MAP_MM );
        pVDev->mnDPIX = pVDev->mnDPIY = nDPI;

        m_pReferenceDevice->mpPDFWriter = this;
        m_pReferenceDevice->ImplUpdateFontData( TRUE );
    }
    return m_pReferenceDevice;
}

ImplDevFontList* PDFWriterImpl::filterDevFontList( ImplDevFontList* pFontList )
{
    DBG_ASSERT( m_aSubsets.size() == 0, "Fonts changing during PDF generation, document will be invalid" );

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
                pNewData->mbDevice = FALSE; // obviously
                pFiltered->Add( pNewData );
            }
            pEntry = pEntry->mpNext;
        }
        pData = pFontList->Next();
    }
    // append the 14 PDF builtin fonts
    for( unsigned int i = 0; i < sizeof(m_aBuiltinFonts)/sizeof(m_aBuiltinFonts[0]); i++ )
    {
        ImplFontData* pNewData = new ImplFontData();
        pNewData->mpSysData     = (void*)&m_aBuiltinFonts[i];
        pNewData->maName        = String::CreateFromAscii( m_aBuiltinFonts[i].m_pName );
        pNewData->maStyleName   = String::CreateFromAscii( m_aBuiltinFonts[i].m_pStyleName );
        pNewData->mnWidth       = 0;
        pNewData->mnHeight      = 0;
        pNewData->meFamily      = m_aBuiltinFonts[i].m_eFamily;
        pNewData->meCharSet     = m_aBuiltinFonts[i].m_eCharSet;
        pNewData->mePitch       = m_aBuiltinFonts[i].m_ePitch;
        pNewData->meWeight      = m_aBuiltinFonts[i].m_eWeight;
        pNewData->meItalic      = m_aBuiltinFonts[i].m_eItalic;
        pNewData->meWidthType   = m_aBuiltinFonts[i].m_eWidthType;
        pNewData->meType        = TYPE_SCALABLE;
        pNewData->mnVerticalOrientation = 0;
        pNewData->mbOrientation = TRUE;
        pNewData->mbDevice      = TRUE;
        pNewData->mnQuality     = 50000;
        pNewData->mbSubsettable = FALSE;
        pNewData->mbEmbeddable  = FALSE;

        pFiltered->Add( pNewData );
    }
    return pFiltered;
}

bool PDFWriterImpl::isBuiltinFont( ImplFontData* pFont ) const
{
    for( unsigned int i = 0; i < sizeof(m_aBuiltinFonts)/sizeof(m_aBuiltinFonts[0]); i++ )
    {
        if( pFont->mpSysData == (void*)&m_aBuiltinFonts[i] )
            return true;
    }
    return false;
}

void PDFWriterImpl::getFontMetric( ImplFontSelectData* pSelect, ImplFontMetricData* pMetric ) const
{
    for( unsigned int i = 0; i < sizeof(m_aBuiltinFonts)/sizeof(m_aBuiltinFonts[0]); i++ )
    {
        if( pSelect->mpFontData->mpSysData == (void*)&m_aBuiltinFonts[i] )
        {
            pMetric->mnWidth        = pSelect->mnHeight;
            pMetric->mnAscent       = ( pSelect->mnHeight * m_aBuiltinFonts[i].m_nAscent + 500 ) / 1000;
            pMetric->mnDescent      = ( pSelect->mnHeight * (-m_aBuiltinFonts[i].m_nDescent) + 500 ) / 1000;
            pMetric->mnLeading      = 0;
            pMetric->mnSlant        = 0;
            pMetric->mnFirstChar    = 32;
            pMetric->mnLastChar     = 255;
            pMetric->meFamily       = m_aBuiltinFonts[i].m_eFamily;
            pMetric->meCharSet      = m_aBuiltinFonts[i].m_eCharSet;
            pMetric->mePitch        = m_aBuiltinFonts[i].m_ePitch;
            pMetric->meWeight       = m_aBuiltinFonts[i].m_eWeight;
            pMetric->meItalic       = m_aBuiltinFonts[i].m_eItalic;
            pMetric->meType         = TYPE_SCALABLE;
            pMetric->mbDevice       = TRUE;
            break;
        }
    }
}

// -----------------------------------------------------------------------

namespace vcl {

class PDFSalLayout : public GenericSalLayout
{
    PDFWriterImpl&  mrPDFWriterImpl;
    const PDFWriterImpl::BuiltinFont& mrBuiltinFont;
    bool            mbIsSymbolFont;
    long            mnPixelPerEM;
    String          maOrigText;

public:
                    PDFSalLayout( PDFWriterImpl&,
                                  const PDFWriterImpl::BuiltinFont&,
                                  long nPixelPerEM, int nOrientation );

    void            SetText( const String& rText )  { maOrigText = rText; }
    virtual bool    LayoutText( ImplLayoutArgs& );
    virtual void    InitFont() const;
    virtual void    DrawText( SalGraphics& ) const;
};

}

// -----------------------------------------------------------------------

PDFSalLayout::PDFSalLayout( PDFWriterImpl& rPDFWriterImpl,
    const PDFWriterImpl::BuiltinFont& rBuiltinFont,
    long nPixelPerEM, int nOrientation )
:   mrPDFWriterImpl( rPDFWriterImpl ),
    mrBuiltinFont( rBuiltinFont ),
    mnPixelPerEM( nPixelPerEM )
{
    mbIsSymbolFont = (rBuiltinFont.m_eCharSet == RTL_TEXTENCODING_SYMBOL);
    SetOrientation( nOrientation );
}

// -----------------------------------------------------------------------

bool PDFSalLayout::LayoutText( ImplLayoutArgs& rArgs )
{
    const String aText( rArgs.mpStr+rArgs.mnMinCharPos, rArgs.mnEndCharPos-rArgs.mnMinCharPos );
    SetText( aText );
    SetUnitsPerPixel( 1000 );

    Point aNewPos( 0, 0 );
    bool bRightToLeft;
    for( int nCharPos = -1; rArgs.GetNextPos( &nCharPos, &bRightToLeft ); )
    {
        sal_Unicode cChar = rArgs.mpStr[ nCharPos ];
        if( cChar & 0xff00 )
        {
            // some characters can be used by conversion
            if( (cChar >= 0xf000) && mbIsSymbolFont  )
                cChar -= 0xf000;
            else
            {
                String aString( cChar);
                ByteString aChar( aString, RTL_TEXTENCODING_MS_1252 );
                cChar = ((sal_Unicode)aChar.GetChar( 0 )) & 0x00ff;
            }
        }
        DBG_ASSERT( cChar < 256, "invalid character index requested for builtin font" );
        if( cChar & 0xff00 )
        {
            cChar = 0;   // NotDef glyph
            rArgs.NeedFallback( nCharPos, bRightToLeft );
        }

        long nGlyphWidth = (long)mrBuiltinFont.m_aWidths[cChar] * mnPixelPerEM;
        long nGlyphFlags = (nGlyphWidth > 0) ? 0 : GlyphItem::IS_IN_CLUSTER;
        if( bRightToLeft )
            nGlyphFlags |= GlyphItem::IS_RTL_GLYPH;
        // TODO: get kerning from builtin fonts
        GlyphItem aGI( nCharPos, cChar, aNewPos, nGlyphFlags, nGlyphWidth );
        AppendGlyph( aGI );

        aNewPos.X() += nGlyphWidth;
    }

    return true;
}

// -----------------------------------------------------------------------

void PDFSalLayout::InitFont() const
{
    // TODO: recreate font with all its attributes
}

// -----------------------------------------------------------------------

void PDFSalLayout::DrawText( SalGraphics& rSalGraphics ) const
{
    mrPDFWriterImpl.drawLayout( *const_cast<PDFSalLayout*>(this), maOrigText, true );
}

// -----------------------------------------------------------------------

SalLayout* PDFWriterImpl::GetTextLayout( ImplLayoutArgs& rArgs, ImplFontSelectData* pSelect )
{
    DBG_ASSERT( (pSelect->mpFontData != NULL),
        "PDFWriterImpl::GetTextLayout mpFontData is NULL" );

    for( unsigned int n = 0; n < sizeof(m_aBuiltinFonts)/sizeof(m_aBuiltinFonts[0]); n++ )
    {
        if( pSelect->mpFontData->mpSysData != (void*)&m_aBuiltinFonts[n] )
            continue;

        long nPixelPerEM = pSelect->mnWidth ? pSelect->mnWidth : pSelect->mnHeight;
        int nOrientation = pSelect->mnOrientation;
        PDFSalLayout* pLayout = new PDFSalLayout( *this, m_aBuiltinFonts[n],
            nPixelPerEM, nOrientation );
        pLayout->SetText( rArgs.mpStr );
        return pLayout;
    }

    return NULL;
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

        if( m_pCodec )
            endCompression();

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
        for( std::list<JPGEmit>::iterator jpeg = m_aJPGs.begin(); jpeg != m_aJPGs.end(); ++jpeg )
        {
            if( jpeg->m_pStream )
            {
                writeJPG( *jpeg );
                delete jpeg->m_pStream;
                jpeg->m_pStream = NULL;
                jpeg->m_aMask = Bitmap();
            }
        }
        for( std::list<TransparencyEmit>::iterator t = m_aTransparentObjects.begin();
             t != m_aTransparentObjects.end(); ++t )
        {
            if( t->m_aContentStream.getLength() )
            {
                writeTransparentObject( *t );
                t->m_aContentStream = OStringBuffer();
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

#define CHECK_RETURN( x ) if( !(x) ) return 0

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
        aTilingObj.append( "<< /Type /Pattern\r\n"
                           "   /PatternType 1\r\n"
                           "   /PaintType 1\r\n"
                           "   /TilingType 1\r\n"
                           "   /BBox [ " );
        appendDouble( (double)nX/10.0, aTilingObj, 1 );
        aTilingObj.append( ' ' );
        appendDouble( (double)nY/10.0, aTilingObj, 1 );
        aTilingObj.append( ' ' );
        appendDouble( (double)(nX+nW)/10.0, aTilingObj, 1 );
        aTilingObj.append( ' ' );
        appendDouble( (double)(nY+nH)/10.0, aTilingObj, 1 );
        aTilingObj.append( " ]\r\n"
                           "   /XStep " );
        appendDouble( (double)nW/10.0, aTilingObj, 1 );
        aTilingObj.append( "\r\n"
                           "   /YStep " );
        appendDouble( (double)nH/10.0, aTilingObj, 1 );
        aTilingObj.append( "\r\n"
                           "   /Resources <<\r\n"
                           "   /XObject << /Im" );
        aTilingObj.append( it->m_nBitmapObject );
        aTilingObj.append( ' ' );
        aTilingObj.append( it->m_nBitmapObject );
        aTilingObj.append( " 0 R >> >>\r\n"
                           "  /Length " );
        aTilingObj.append( (sal_Int32)aTilingStream.getLength() );
        aTilingObj.append( "\r\n"
                           ">>\r\nstream\r\n" );
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
        aHatchObj.append( "<< /Type /Pattern\r\n"
                          "   /PatternType 1\r\n"
                          "   /PaintType 1\r\n"
                          "   /TilingType 1\r\n"
                          "   /BBox [ " );
        aHatchObj.append( -nMax );
        aHatchObj.append( ' ' );
        aHatchObj.append( -nMax );
        aHatchObj.append( ' ' );
        aHatchObj.append( nMax );
        aHatchObj.append( ' ' );
        aHatchObj.append( nMax );
        aHatchObj.append( " ]\r\n"
                          "   /XStep " );
        aHatchObj.append( 2*nMax );
        aHatchObj.append( "\r\n"
                          "   /YStep " );
        aHatchObj.append( 2*nMax );
        aHatchObj.append( "\r\n"
                          "   /Matrix [ " );
        // prepare matrix
        const double theta = (double)rHatch.GetAngle() * M_PI / 1800.0;
        Size aSize( rHatch.GetDistance(), 0 );
        aSize = lcl_convert( it->m_aMapMode,
                             MapMode( MAP_POINT ),
                             getReferenceDevice(),
                             aSize );
        const double scale = (double)aSize.Width();
        appendDouble( scale*cos( theta ), aHatchObj );
        aHatchObj.append( ' ' );
        appendDouble( scale*sin( theta ), aHatchObj );
        aHatchObj.append( ' ' );
        appendDouble( scale*(-sin( theta )), aHatchObj );
        aHatchObj.append( ' ' );
        appendDouble( scale*cos( theta ), aHatchObj );
        aHatchObj.append( " 0 0 ]\r\n" );

        aHatchObj.append( "   /Resources <<\r\n"
                          "     /ColorSpace << /HCS [ /Pattern /DeviceRGB ] >>\r\n"
                          "     /Pattern << /P" );
        aHatchObj.append( nBaseHatch );
        aHatchObj.append( ' ' );
        aHatchObj.append( nBaseHatch );
        aHatchObj.append( " 0 R >>\r\n"
                          "     >>\r\n"
                          "   /Length " );
        aHatchObj.append( (sal_Int32)aHatchStream.getLength() );
        aHatchObj.append( "\r\n"
                          ">>\r\n"
                          "stream\r\n" );
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
                    aHatchStream.append( "0 0.5 m 1 0.5 l S\r\n" );
                    break;
                case 1:
                    nObject = nDoubleHatch;
                    aHatchStream.append( "0 0.5 m 1 0.5 l S\r\n" );
                    aHatchStream.append( "0.5 0 m 0.5 1 l S\r\n" );
                    break;
                case 2:
                    nObject = nTripleHatch;
                    aHatchStream.append( "0 0.5 m 1 0.5 l S\r\n" );
                    aHatchStream.append( "0.5 0 m 0.5 1 l S\r\n" );
                    aHatchStream.append( "0 1 m 1 0 l S\r\n" );
                    break;
            }
            CHECK_RETURN( updateObject( nObject ) );
            aHatchObj.setLength( 0 );

            aHatchObj.append( nObject );
            aHatchObj.append( " 0 obj\r\n" );
            aHatchObj.append( "<< /Type /Pattern\r\n"
                              "   /PatternType 1\r\n"
                              "   /PaintType 2\r\n"
                              "   /TilingType 1\r\n"
                              "   /BBox [ 0 0 1 1 ]\r\n"
                              "   /XStep 1\r\n"
                              "   /YStep 1\r\n"
                              "   /Resources << >>\r\n"
                              "   /Length " );
            aHatchObj.append( (sal_Int32)aHatchStream.getLength() );
            aHatchObj.append( "\r\n"
                              ">>\r\n"
                              "stream\r\n" );
            CHECK_RETURN( writeBuffer( aHatchObj.getStr(), aHatchObj.getLength() ) );
            CHECK_RETURN( writeBuffer( aHatchStream.getStr(), aHatchStream.getLength() ) );
            aHatchObj.setLength( 0 );
            aHatchObj.append( "\r\nendstream\r\nendobj\r\n\r\n" );
            CHECK_RETURN( writeBuffer( aHatchObj.getStr(), aHatchObj.getLength() ) );
        }
    }
    return true;
}

sal_Int32 PDFWriterImpl::emitBuiltinFont( ImplFontData* pFont )
{
    sal_Int32 nFontObject = 0;
    for( unsigned int i = 0; i < sizeof(m_aBuiltinFonts)/sizeof(m_aBuiltinFonts[0]); i++ )
    {
        if( pFont->mpSysData == (void*)&m_aBuiltinFonts[i] )
        {
            OStringBuffer aLine( 1024 );

            nFontObject = createObject();
            CHECK_RETURN( updateObject( nFontObject ) );
            aLine.append( nFontObject );
            aLine.append( " 0 obj\r\n"
                          "<< /Type /Font\r\n"
                          "   /Subtype /Type1\r\n"
                          "   /BaseFont /" );
            appendName( m_aBuiltinFonts[i].m_pPSName, aLine );
            aLine.append( "\r\n" );
            if( m_aBuiltinFonts[i].m_eCharSet != RTL_TEXTENCODING_SYMBOL )
                aLine.append( "   /Encoding /WinAnsiEncoding\r\n" );
            aLine.append( ">>\r\nendobj\r\n\r\n" );
            CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

            break;
        }
    }
    return nFontObject;
}

std::map< sal_Int32, sal_Int32 > PDFWriterImpl::emitEmbeddedFont( ImplFontData* pFont, EmbedFont& rEmbed )
{
    std::map< sal_Int32, sal_Int32 > aRet;
    if( isBuiltinFont( pFont ) )
    {
        aRet[ rEmbed.m_nNormalFontID ] = emitBuiltinFont( pFont );
        return aRet;
    }

    sal_Int32 nFontObject = 0;
    sal_Int32 nStreamObject = 0;
    sal_Int32 nFontDescriptor = 0;

    FontSubsetInfo aInfo;
    sal_Int32 pWidths[256];
    const unsigned char* pFontData = NULL;
    long nFontLen = 0;
    sal_Int32 nLength1, nLength2;
    if( (pFontData = (const unsigned char*)m_pReferenceDevice->mpGraphics->GetEmbedFontData( pFont, pWidths, aInfo, &nFontLen )) )
    {
        if( aInfo.m_nFontType != SAL_FONTSUBSETINFO_TYPE_TYPE1 )
            goto streamend;
        // see whether it is pfb or pfa; if it is a pfb, fill ranges
        // of 6 bytes that are not part of the font program
        std::list< int > aSections;
        std::list< int >::const_iterator it;
        int nIndex = 0;
        while( pFontData[nIndex] == 0x80 )
        {
            aSections.push_back( nIndex );
            if( pFontData[nIndex+1] == 0x03 )
                break;
            sal_Int32 nBytes =
                ((sal_Int32)pFontData[nIndex+2])            |
                ((sal_Int32)pFontData[nIndex+3]) << 8       |
                ((sal_Int32)pFontData[nIndex+4]) << 16      |
                ((sal_Int32)pFontData[nIndex+5]) << 24;
            nIndex += nBytes+6;
        }

        // search for eexec
        nIndex = 0;
        int nEndAsciiIndex;
        int nBeginBinaryIndex;
        int nEndBinaryIndex;
        do
        {
            while( nIndex < nFontLen-4 &&
                   ( pFontData[nIndex] != 'e'   ||
                     pFontData[nIndex+1] != 'e' ||
                     pFontData[nIndex+2] != 'x' ||
                     pFontData[nIndex+3] != 'e' ||
                     pFontData[nIndex+4] != 'c'
                     )
                   )
                nIndex++;
            // check whether we are in a excluded section
            for( it = aSections.begin(); it != aSections.end() && (nIndex < *it || nIndex > ((*it) + 5) ); ++it )
                ;
        } while( it != aSections.end() && nIndex < nFontLen-4 );
        // this should end the ascii part
        if( nIndex > nFontLen-5 )
            goto streamend;

        nEndAsciiIndex = nIndex+4;
        // now count backwards until we can account for 512 '0'
        // which is the endmarker of the (hopefully) binary data
        // do not count the pfb header sections
        int nFound = 0;
        nIndex =  nFontLen-1;
        while( nIndex > 0 && nFound < 512 )
        {
            for( it = aSections.begin(); it != aSections.end() && (nIndex < *it || nIndex > ((*it) + 5) ); ++it )
                ;
            if( it == aSections.end() )
            {
                // inside the 512 '0' block there may only be whitespace
                // according to T1 spec; probably it would be to simple
                // if all fonts complied
                if( pFontData[nIndex] == '0' )
                    nFound++;
                else if( nFound > 0                 &&
                         pFontData[nIndex] != '\r'      &&
                         pFontData[nIndex] != '\t'      &&
                         pFontData[nIndex] != '\n'      &&
                         pFontData[nIndex] != ' ' )
                    break;
            }
            nIndex--;
        }
        if( nIndex < 1 || nIndex <= nEndAsciiIndex )
            goto streamend;
        // there may be whitespace to ignore before the 512 '0'
        while( pFontData[nIndex] == '\r' || pFontData[nIndex] == '\n' )
        {
            nIndex--;
            for( it = aSections.begin(); it != aSections.end() && (nIndex < *it || nIndex > ((*it) + 5) ); ++it )
                ;
            if( it != aSections.end() )
            {
                nIndex = (*it)-1;
                break; // this is surely a binary boundary, in ascii case it wouldn't matter
            }
        }
        nEndBinaryIndex = nIndex;

        // search for beginning of binary section
        nBeginBinaryIndex = nEndAsciiIndex;
        do
        {
            nBeginBinaryIndex++;
            for( it = aSections.begin(); it != aSections.end() && (nBeginBinaryIndex < *it || nBeginBinaryIndex > ((*it) + 5) ); ++it )
                ;
        } while( nBeginBinaryIndex < nEndBinaryIndex &&
                 ( pFontData[nBeginBinaryIndex] == '\r' ||
                   pFontData[nBeginBinaryIndex] == '\n' ||
                   it != aSections.end() ) );

        // it seems to be vital to copy the exact whitespace between binary data
        // and eexec, else a invalid font results. so make nEndAsciiIndex
        // always immediate in front of nBeginBinaryIndex
        nEndAsciiIndex = nBeginBinaryIndex-1;
        for( it = aSections.begin(); it != aSections.end() && (nEndAsciiIndex < *it || nEndAsciiIndex > ((*it)+5)); ++it )
            ;
        if( it != aSections.end() )
            nEndAsciiIndex = (*it)-1;

        nLength1 = nEndAsciiIndex+1; // including the last character
        for( it = aSections.begin(); it != aSections.end() && *it < nEndAsciiIndex; ++it )
            nLength1 -= 6; // decrease by pfb section size

        // if the first four bytes are all ascii hex characters, then binary data
        // has to be converted to real binary data
        for( nIndex = 0; nIndex < 4 &&
                 ( ( pFontData[ nBeginBinaryIndex+nIndex ] >= '0' && pFontData[ nBeginBinaryIndex+nIndex ] <= '9' ) ||
                   ( pFontData[ nBeginBinaryIndex+nIndex ] >= 'a' && pFontData[ nBeginBinaryIndex+nIndex ] <= 'f' ) ||
                   ( pFontData[ nBeginBinaryIndex+nIndex ] >= 'A' && pFontData[ nBeginBinaryIndex+nIndex ] <= 'F' )
                   ); ++nIndex )
            ;
        bool bConvertHexData = true;
        if( nIndex < 4 )
        {
            bConvertHexData = false;
            nLength2 = nEndBinaryIndex - nBeginBinaryIndex + 1; // include the last byte
            for( it = aSections.begin(); it != aSections.end(); ++it )
                if( *it > nBeginBinaryIndex && *it < nEndBinaryIndex )
                    nLength2 -= 6;
        }
        else
        {
            // count the hex ascii characters to get nLength2
            nLength2 = 0;
            int nNextSectionIndex = 0;
            for( it = aSections.begin(); it != aSections.end() && *it < nBeginBinaryIndex; ++it )
                ;
            if( it != aSections.end() )
                nNextSectionIndex = *it;
            for( nIndex = nBeginBinaryIndex; nIndex <= nEndBinaryIndex; nIndex++ )
            {
                if( nIndex == nNextSectionIndex )
                {
                    nIndex += 6;
                    ++it;
                    nNextSectionIndex = (it == aSections.end() ? 0 : *it );
                }
                if( ( pFontData[ nIndex ] >= '0' && pFontData[ nIndex ] <= '9' ) ||
                    ( pFontData[ nIndex ] >= 'a' && pFontData[ nIndex ] <= 'f' ) ||
                    ( pFontData[ nIndex ] >= 'A' && pFontData[ nIndex ] <= 'F' ) )
                    nLength2++;
            }
            DBG_ASSERT( !(nLength2 & 1), "uneven number of hex chars in binary pfa section" );
            nLength2 /= 2;
        }

        // now we can actually write the font stream !
        OStringBuffer aLine( 512 );
        nStreamObject = createObject();
        if( !updateObject(nStreamObject))
            goto streamend;
        sal_Int32 nStreamLengthObject = createObject();
        aLine.append( nStreamObject );
        aLine.append( " 0 obj\r\n"
                      "<< /Length " );
        aLine.append( nStreamLengthObject );
        aLine.append( " 0 R\r\n"
#ifdef ENABLE_COMPRESSION
                      "   /Filter /FlateDecode\r\n"
#endif
                      "   /Length1 " );
        aLine.append( nLength1 );
        aLine.append( "\r\n"
                      "   /Length2 " );
        aLine.append( nLength2 );
        aLine.append( "\r\n"
                      "   /Length3 0\r\n"
                      ">>\r\n"
                      "stream\r\n" );
        if( !writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;

        sal_uInt64 nBeginStreamPos = 0;
        osl_getFilePos( m_aFile, &nBeginStreamPos );

        beginCompression();

        // write ascii section
        if( aSections.begin() == aSections.end() )
        {
            if( ! writeBuffer( pFontData, nEndAsciiIndex+1 ) )
                goto streamend;
        }
        else
        {
            // first section always starts at 0
            it = aSections.begin();
            nIndex = (*it)+6;
            ++it;
            while( *it < nEndAsciiIndex )
            {
                if( ! writeBuffer( pFontData+nIndex, (*it)-nIndex ) )
                    goto streamend;
                nIndex = (*it)+6;
                ++it;
            }
            // write partial last section
            if( ! writeBuffer( pFontData+nIndex, nEndAsciiIndex-nIndex+1 ) )
                goto streamend;
        }

        // write binary section
        if( ! bConvertHexData )
        {
            if( aSections.begin() == aSections.end() )
            {
                if( ! writeBuffer( pFontData+nBeginBinaryIndex, nEndBinaryIndex-nBeginBinaryIndex+1 ) )
                    goto streamend;
            }
            else
            {
                for( it = aSections.begin(); *it < nBeginBinaryIndex; ++it )
                    ;
                if( *it > nEndBinaryIndex )
                {
                    if( ! writeBuffer( pFontData+nBeginBinaryIndex, nEndBinaryIndex-nBeginBinaryIndex+1 ) )
                        goto streamend;
                }
                else
                {
                    // write first partial section
                    if( ! writeBuffer( pFontData+nBeginBinaryIndex, (*it) - nBeginBinaryIndex ) )
                        goto streamend;
                    nIndex = (*it)+6;
                    ++it;
                    while( *it < nEndBinaryIndex )
                    {
                        if( ! writeBuffer( pFontData+nIndex, (*it)-nIndex ) )
                            goto streamend;
                        nIndex = (*it)+6;
                        ++it;
                    }
                    // write partial last section
                    if( ! writeBuffer( pFontData+nIndex, nEndBinaryIndex-nIndex+1 ) )
                        goto streamend;
                }
            }
        }
        else
        {
            unsigned char* pWriteBuffer = (unsigned char*)rtl_allocateMemory( nLength2 );
            memset( pWriteBuffer, 0, nLength2 );
            int nWriteIndex = 0;

            int nNextSectionIndex = 0;
            for( it = aSections.begin(); it != aSections.end() && *it < nBeginBinaryIndex; ++it )
                ;
            if( it != aSections.end() )
                nNextSectionIndex = *it;
            for( nIndex = nBeginBinaryIndex; nIndex <= nEndBinaryIndex; nIndex++ )
            {
                if( nIndex == nNextSectionIndex )
                {
                    nIndex += 6;
                    ++it;
                    nNextSectionIndex = (it == aSections.end() ? 0 : *it );
                }
                unsigned char cNibble = 0x80;
                if( pFontData[ nIndex ] >= '0' && pFontData[ nIndex ] <= '9' )
                    cNibble = pFontData[nIndex] - '0';
                else if( pFontData[ nIndex ] >= 'a' && pFontData[ nIndex ] <= 'f' )
                    cNibble = pFontData[nIndex] - 'a' + 10;
                else if( pFontData[ nIndex ] >= 'A' && pFontData[ nIndex ] <= 'F' )
                    cNibble = pFontData[nIndex] - 'A' + 10;
                if( cNibble != 0x80 )
                {
                    if( !(nWriteIndex & 1 ) )
                        cNibble <<= 4;
                    pWriteBuffer[ nWriteIndex/2 ] |= cNibble;
                    nWriteIndex++;
                }
            }
            if( ! writeBuffer( pWriteBuffer, nLength2 ) )
                goto streamend;

            rtl_freeMemory( pWriteBuffer );
        }
        endCompression();


        sal_uInt64 nEndStreamPos = 0;
        osl_getFilePos( m_aFile, &nEndStreamPos );

        // and finally close the stream
        aLine.setLength( 0 );
        aLine.append( "\r\nendstream\r\nendobj\r\n\r\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;

        // write stream length object
        aLine.setLength( 0 );
        if( ! updateObject( nStreamLengthObject ) )
            goto streamend;
        aLine.append( nStreamLengthObject );
        aLine.append( " 0 obj\r\n" );
        aLine.append( (sal_Int64)(nEndStreamPos-nBeginStreamPos ) );
        aLine.append( "\r\nendobj\r\n\r\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;
    }

    if( nStreamObject )
        // write font descriptor
        nFontDescriptor = emitFontDescriptor( pFont, aInfo, 0, nStreamObject );

    if( nFontDescriptor )
    {
        const std::map< sal_Unicode, sal_Int32 >* pEncoding = m_pReferenceDevice->mpGraphics->GetFontEncodingVector( pFont, NULL );
        sal_Int32 nToUnicodeStream = 0;
        sal_uInt8 nEncoding[256];
        sal_Unicode nEncodedCodes[256];
        if( pEncoding )
        {
            memset( nEncodedCodes, 0, sizeof(nEncodedCodes) );
            memset( nEncoding, 0, sizeof(nEncoding) );
            for( std::map< sal_Unicode, sal_Int32 >::const_iterator it = pEncoding->begin(); it != pEncoding->end(); ++it )
            {
                if( it->second != -1 )
                {
                    sal_Int32 nCode = (sal_Int32)(it->second & 0x000000ff);
                    nEncoding[ nCode ] = static_cast<sal_uInt8>( nCode );
                    nEncodedCodes[ nCode ] = it->first;
                }
            }
            nToUnicodeStream = createToUnicodeCMap( nEncoding, nEncodedCodes, sizeof(nEncoding)/sizeof(nEncoding[0]) );
        }

        // write font object
        sal_Int32 nObject = createObject();
        if( ! updateObject( nObject ) )
            goto streamend;

        OStringBuffer aLine( 1024 );
        aLine.append( nObject );
        aLine.append( " 0 obj\r\n"
                      "<< /Type /Font\r\n"
                      "   /Subtype /Type1\r\n"
                      "   /BaseFont /" );
        appendName( aInfo.m_aPSName, aLine );
        aLine.append( "\r\n" );
        if( pFont->meCharSet != RTL_TEXTENCODING_SYMBOL &&  pEncoding == 0 )
            aLine.append( "   /Encoding /WinAnsiEncoding\r\n" );
        if( nToUnicodeStream )
        {
            aLine.append( "   /ToUnicode " );
            aLine.append( nToUnicodeStream );
            aLine.append( " 0 R\r\n" );
        }
        aLine.append( "   /FirstChar 0\r\n"
                      "   /LastChar 255\r\n"
                      "   /Widths [ " );
        for( int i = 0; i < 256; i++ )
        {
            aLine.append( pWidths[i] );
            aLine.append( ((i&7) == 7) ? "\r\n     " : " " );
        }
        aLine.append( " ]\r\n"
                      "   /FontDescriptor " );
        aLine.append( nFontDescriptor );
        aLine.append( " 0 R\r\n"
                      ">>\r\n"
                      "endobj\r\n\r\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            goto streamend;

        nFontObject = nObject;

        aRet[ rEmbed.m_nNormalFontID ] = nObject;

        // write additional encodings
        for( std::list< EmbedEncoding >::iterator enc_it = rEmbed.m_aExtendedEncodings.begin(); enc_it != rEmbed.m_aExtendedEncodings.end(); ++enc_it )
        {
            sal_Int32 aEncWidths[ 256 ];
            // emit encoding dict
            sal_Int32 nEncObject = createObject();
            if( ! updateObject( nEncObject ) )
                goto streamend;

            OutputDevice* pRef = getReferenceDevice();
            pRef->Push( PUSH_FONT | PUSH_MAPMODE );
            pRef->SetMapMode( MapMode( MAP_POINT ) );
            Font aFont( pFont->maName, pFont->maStyleName, Size( 0, 100 ) );
            pRef->SetFont( aFont );
            pRef->ImplNewFont();

            aLine.setLength( 0 );
            aLine.append( nEncObject );
            aLine.append( " 0 obj\r\n"
                          "<< /Type /Encoding\r\n"
                          "   /Differences [ 0\r\n" );
            int nEncoded = 0;
            for( std::vector< EmbedCode >::iterator str_it = enc_it->m_aEncVector.begin(); str_it != enc_it->m_aEncVector.end(); ++str_it )
            {
                String aStr( str_it->m_aUnicode );
                aEncWidths[nEncoded] = pRef->GetTextWidth( aStr ) * 10;
                nEncodedCodes[nEncoded] = str_it->m_aUnicode;
                nEncoding[nEncoded] = nEncoded;

                aLine.append( " /" );
                aLine.append( str_it->m_aName );
                if( !((++nEncoded) & 7) )
                    aLine.append( "\r\n" );
            }
            aLine.append( " ]\r\n"
                          ">>\r\n"
                          "endobj\r\n\r\n" );

            pRef->Pop();

            if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
                goto streamend;

            nToUnicodeStream = createToUnicodeCMap( nEncoding, nEncodedCodes, nEncoded );

            nObject = createObject();
            if( ! updateObject( nObject ) )
                goto streamend;

            aLine.setLength( 0 );
            aLine.append( nObject );
            aLine.append( " 0 obj\r\n"
                          "<< /Type /Font\r\n"
                          "   /Subtype /Type1\r\n"
                          "   /BaseFont /" );
            appendName( aInfo.m_aPSName, aLine );
            aLine.append( "\r\n" );
            aLine.append( "   /Encoding " );
            aLine.append( nEncObject );
            aLine.append( " 0 R\r\n" );
            if( nToUnicodeStream )
            {
                aLine.append( "   /ToUnicode " );
                aLine.append( nToUnicodeStream );
                aLine.append( " 0 R\r\n" );
            }
            aLine.append( "   /FirstChar 0\r\n"
                          "   /LastChar " );
            aLine.append( (sal_Int32)(nEncoded-1) );
            aLine.append( "\r\n"
                          "   /Widths [ " );
            for( int i = 0; i < nEncoded; i++ )
            {
                aLine.append( aEncWidths[i] );
                aLine.append( ((i&7) == 7) ? "\r\n     " : " " );
            }
            aLine.append( " ]\r\n"
                          "   /FontDescriptor " );
            aLine.append( nFontDescriptor );
            aLine.append( " 0 R\r\n"
                          ">>\r\n"
                          "endobj\r\n\r\n" );
            if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
                goto streamend;

            aRet[ enc_it->m_nFontID ] = nObject;
        }
    }

  streamend:
    if( pFontData )
        m_pReferenceDevice->mpGraphics->FreeEmbedFontData( pFontData, nFontLen );

    return aRet;
}

static void appendSubsetName( int nSubsetID, const OUString& rPSName, OStringBuffer& rBuffer )
{
    if( nSubsetID )
    {
        for( int i = 0; i < 6; i++ )
        {
            int nOffset = (nSubsetID % 26);
            nSubsetID /= 26;
            rBuffer.append( (sal_Char)('A'+nOffset) );
        }
        rBuffer.append( '+' );
    }
    appendName( rPSName, rBuffer );
}

sal_Int32 PDFWriterImpl::createToUnicodeCMap( sal_uInt8* pEncoding, sal_Unicode* pUnicodes, int nGlyphs )
{
    int nMapped = 0, n = 0;
    for( n = 0; n < nGlyphs; n++ )
        if( pUnicodes[n] )
            nMapped++;

    if( nMapped == 0 )
        return 0;

    sal_Int32 nStream = createObject();
    CHECK_RETURN( updateObject( nStream ) );

    OStringBuffer aContents( 1024 );
    aContents.append(
                     "/CIDInit /ProcSet findresource begin\r\n"
                     "12 dict begin\r\n"
                     "begincmap\r\n"
                     "/CIDSystemInfo <<\r\n"
                     "  /Registry (Adobe)\r\n"
                     "  /Ordering (UCS)\r\n"
                     "  /Supplement 0\r\n"
                     ">> def\r\n"
                     "/CMapName /Adobe-Identity-UCS def\r\n"
                     "/CMapType 2 def\r\n"
                     "1 begincodespacerange\r\n"
                     "<00> <FF>\r\n"
                     "endcodespacerange\r\n"
                     );
    int nCount = 0;
    for( n = 0; n < nGlyphs; n++ )
    {
        if( pUnicodes[n] )
        {
            if( (nCount % 100) == 0 )
            {
                if( nCount )
                    aContents.append( "endbfchar\r\n" );
                aContents.append( (sal_Int32)((nMapped-nCount > 100) ? 100 : nMapped-nCount ) );
                aContents.append( " beginbfchar\r\n" );
            }
            aContents.append( '<' );
            appendHex( (sal_Int8)pEncoding[n], aContents );
            aContents.append( "> <" );
            appendHex( (sal_Int8)(pUnicodes[n] / 256), aContents );
            appendHex( (sal_Int8)(pUnicodes[n] & 255), aContents );
            aContents.append( ">\r\n" );
            nCount++;
        }
    }
    aContents.append( "endbfchar\r\n"
                      "endcmap\r\n"
                      "CMapName currentdict /CMap defineresource pop\r\n"
                      "end\r\n"
                      "end\r\n" );
#if defined COMPRESS_PAGES && defined ENABLE_COMPRESSION
    ZCodec* pCodec = new ZCodec( 0x4000, 0x4000 );
    SvMemoryStream aStream;
    pCodec->BeginCompression();
    pCodec->Write( aStream, (const BYTE*)aContents.getStr(), aContents.getLength() );
    pCodec->EndCompression();
    delete pCodec;
#endif

    OStringBuffer aLine( 40 );

    aLine.append( nStream );
    aLine.append( " 0 obj\r\n<< /Length " );
#if defined COMPRESS_PAGES && defined ENABLE_COMPRESSION
    sal_Int32 nLen = (sal_Int32)aStream.Tell();
    aStream.Seek( 0 );
    aLine.append( nLen );
    aLine.append( "\r\n   /Filter /FlateDecode" );
#else
    aLine.append( aContents.getLength() );
#endif
    aLine.append( " >>\r\nstream\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
#if defined COMPRESS_PAGES && defined ENABLE_COMPRESSION
    CHECK_RETURN( writeBuffer( aStream.GetData(), nLen ) );
#else
    CHECK_RETURN( writeBuffer( aContents.getStr(), aContents.getLength() ) );
#endif
    aLine.setLength( 0 );
    aLine.append( "endstream\r\n"
                  "endobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    return nStream;
}

sal_Int32 PDFWriterImpl::emitFontDescriptor( ImplFontData* pFont, FontSubsetInfo& rInfo, sal_Int32 nSubsetID, sal_Int32 nFontStream )
{
    OStringBuffer aLine( 1024 );
    // get font flags, see PDF reference 1.4 p. 358
    // possibly characters outside Adobe standard encoding
    // so set Symbolic flag
    sal_Int32 nFontFlags = (1<<2);
    if( pFont->meItalic == ITALIC_NORMAL || pFont->meItalic == ITALIC_OBLIQUE )
        nFontFlags |= (1 << 6);
    if( pFont->mePitch == PITCH_FIXED )
        nFontFlags |= 1;
    if( pFont->meFamily == FAMILY_SCRIPT )
        nFontFlags |= (1 << 3);
    if( pFont->meFamily == FAMILY_ROMAN )
        nFontFlags |= (1 << 1);

    sal_Int32 nFontDescriptor = createObject();
    CHECK_RETURN( updateObject( nFontDescriptor ) );
    aLine.setLength( 0 );
    aLine.append( nFontDescriptor );
    aLine.append( " 0 obj\r\n"
                  "<< /Type /FontDescriptor\r\n"
                  "   /FontName /" );
    appendSubsetName( nSubsetID, rInfo.m_aPSName, aLine );
    aLine.append( "\r\n"
                  "   /Flags " );
    aLine.append( nFontFlags );
    aLine.append( "\r\n"
                  "   /FontBBox [ " );
    // note: Top and Bottom are reversed in VCL and PDF rectangles
    aLine.append( (sal_Int32)rInfo.m_aFontBBox.TopLeft().X() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)rInfo.m_aFontBBox.TopLeft().Y() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)rInfo.m_aFontBBox.BottomRight().X() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)(rInfo.m_aFontBBox.BottomRight().Y()+1) );
    aLine.append( " ]\r\n"
                  "   /ItalicAngle " );
    if( pFont->meItalic == ITALIC_OBLIQUE || pFont->meItalic == ITALIC_NORMAL )
        aLine.append( "-30" );
    else
        aLine.append( "0" );
    aLine.append( "\r\n"
                  "   /Ascent " );
    aLine.append( (sal_Int32)rInfo.m_nAscent );
    aLine.append( "\r\n"
                  "   /Descent " );
    aLine.append( (sal_Int32)-rInfo.m_nDescent );
    aLine.append( "\r\n"
                  "   /CapHeight " );
    aLine.append( (sal_Int32)rInfo.m_nCapHeight );
    // According to PDF reference 1.4 StemV is required
    // seems a tad strange to me, but well ...
    aLine.append( "\r\n"
                  "   /StemV 80\r\n"
                  "   /FontFile" );
    switch( rInfo.m_nFontType )
    {
        case SAL_FONTSUBSETINFO_TYPE_TRUETYPE:
            aLine.append( '2' );
            break;
        case SAL_FONTSUBSETINFO_TYPE_TYPE1:
            break;
        default:
            DBG_ERROR( "unknown fonttype in PDF font descriptor" );
            return 0;
    }
    aLine.append( ' ' );
    aLine.append( nFontStream );
    aLine.append( " 0 R\r\n"
                  ">>\r\n"
                  "endobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return nFontDescriptor;
}

sal_Int32 PDFWriterImpl::emitFonts()
{
    sal_Int32 nFontDict = 0;

    if( ! m_aSubsets.size() && ! m_aEmbeddedFonts.size() ) // no fonts
        return 0;

#ifndef REMOTE_APPSERVER
    if( ! m_pReferenceDevice->ImplGetGraphics() )
#else
    if( ! m_pReferenceDevice->ImplGetServerGraphics() )
#endif
        return 0;

    OStringBuffer aLine( 1024 );
    char buf[8192];

    std::map< sal_Int32, sal_Int32 > aFontIDToObject;

    OUString aTmpName;
    osl_createTempFile( NULL, NULL, &aTmpName.pData );
    for( FontSubsetData::iterator it = m_aSubsets.begin(); it != m_aSubsets.end(); ++it )
    {
        for( FontEmitList::iterator lit = it->second.m_aSubsets.begin(); lit != it->second.m_aSubsets.end(); ++lit )
        {
            long pGlyphIDs[ 256 ];
            sal_Int32 pWidths[ 256 ];
            sal_uInt8 pEncoding[ 256 ];
            sal_Unicode pUnicodes[ 256 ];
            int nGlyphs = 1;
            // fill arrays and prepare encoding index map
            sal_Int32 nToUnicodeStream = 0;

            memset( pGlyphIDs, 0, sizeof( pGlyphIDs ) );
            memset( pEncoding, 0, sizeof( pEncoding ) );
            memset( pUnicodes, 0, sizeof( pUnicodes ) );
            for( FontEmitMapping::iterator fit = lit->m_aMapping.begin(); fit != lit->m_aMapping.end();++fit )
            {
                sal_uInt8 nEnc = fit->second.m_nSubsetGlyphID;

                DBG_ASSERT( pGlyphIDs[nEnc] == 0 && pEncoding[nEnc] == 0, "duplicate glyph" );
                DBG_ASSERT( nEnc <= lit->m_aMapping.size(), "invalid glyph encoding" );

                pGlyphIDs[ nEnc ] = fit->first;
                pEncoding[ nEnc ] = nEnc;
                pUnicodes[ nEnc ] = fit->second.m_aUnicode;
                if( pUnicodes[ nEnc ] )
                    nToUnicodeStream = 1;
                if( nGlyphs < 256 )
                    nGlyphs++;
                else
                {
                    DBG_ERROR( "too many glyphs for subset" );
                }
            }
            FontSubsetInfo aSubsetInfo;
            if( m_pReferenceDevice->mpGraphics->CreateFontSubset( aTmpName, it->first, pGlyphIDs, pEncoding, pWidths, nGlyphs, aSubsetInfo ) )
            {
                DBG_ASSERT( aSubsetInfo.m_nFontType == SAL_FONTSUBSETINFO_TYPE_TRUETYPE, "wrong font type in font subset" );
                // create font stream
                oslFileHandle aFontFile;
                CHECK_RETURN( (osl_File_E_None == osl_openFile( aTmpName.pData, &aFontFile, osl_File_OpenFlag_Read ) ) );
                // get file size
                sal_uInt64 nLength;
                CHECK_RETURN( (osl_File_E_None == osl_setFilePos( aFontFile, osl_Pos_End, 0 ) ) );
                CHECK_RETURN( (osl_File_E_None == osl_getFilePos( aFontFile, &nLength ) ) );
                CHECK_RETURN( (osl_File_E_None == osl_setFilePos( aFontFile, osl_Pos_Absolut, 0 ) ) );

                sal_Int32 nFontStream = createObject();
                sal_Int32 nStreamLengthObject = createObject();
                CHECK_RETURN( updateObject( nFontStream ) );
                aLine.setLength( 0 );
                aLine.append( nFontStream );
                aLine.append( " 0 obj\r\n"
                              "<< /Length " );
                aLine.append( (sal_Int32)nStreamLengthObject );
                aLine.append( " 0 R\r\n"
#ifdef ENABLE_COMPRESSION
                              "   /Filter /FlateDecode\r\n"
#endif
                              "   /Length1 " );
                aLine.append( (sal_Int32)nLength );
                aLine.append( "\r\n"
                              ">>\r\n"
                              "stream\r\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                sal_uInt64 nStartPos = 0;
                CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartPos ) ) );

                // copy font file
                beginCompression();
                sal_uInt64 nRead;
                sal_Bool bEOF = sal_False;
                do
                {
                    CHECK_RETURN( (osl_File_E_None == osl_readFile( aFontFile, buf, sizeof( buf ), &nRead ) ) );
                    CHECK_RETURN( writeBuffer( buf, nRead ) );
                    CHECK_RETURN( (osl_File_E_None == osl_isEndOfFile( aFontFile, &bEOF ) ) );
                } while( ! bEOF );
                endCompression();
                // close the file
                osl_closeFile( aFontFile );

                sal_uInt64 nEndPos = 0;
                CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nEndPos ) ) );
                // end the stream
                aLine.setLength( 0 );
                aLine.append( "\r\nendstream\r\nendobj\r\n\r\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                // emit stream length object
                CHECK_RETURN( updateObject( nStreamLengthObject ) );
                aLine.setLength( 0 );
                aLine.append( nStreamLengthObject );
                aLine.append( " 0 obj\r\n" );
                aLine.append( (sal_Int64)(nEndPos-nStartPos) );
                aLine.append( "\r\nendobj\r\n\r\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                // write font descriptor
                sal_Int32 nFontDescriptor = emitFontDescriptor( it->first, aSubsetInfo, lit->m_nFontID, nFontStream );

                if( nToUnicodeStream )
                    nToUnicodeStream = createToUnicodeCMap( pEncoding, pUnicodes, nGlyphs );

                sal_Int32 nFontObject = createObject();
                CHECK_RETURN( updateObject( nFontObject ) );
                aLine.setLength( 0 );
                aLine.append( nFontObject );
                aLine.append( " 0 obj\r\n"
                              "<< /Type /Font\r\n"
                              "   /Subtype /TrueType\r\n"
                              "   /BaseFont /" );
                appendSubsetName( lit->m_nFontID, aSubsetInfo.m_aPSName, aLine );
                aLine.append( "\r\n"
                              "   /FirstChar 0\r\n"
                              "   /LastChar " );
                aLine.append( (sal_Int32)(nGlyphs-1) );
                aLine.append( "\r\n"
                              "   /Widths [ " );
                for( int i = 0; i < nGlyphs; i++ )
                {
                    aLine.append( pWidths[ i ] );
                    aLine.append( ((i & 7) == 7) ? "\r\n     " : " " );
                }
                aLine.append( "]\r\n"
                              "   /FontDescriptor " );
                aLine.append( nFontDescriptor );
                aLine.append( " 0 R\r\n" );
                if( nToUnicodeStream )
                {
                    aLine.append( "   /ToUnicode " );
                    aLine.append( nToUnicodeStream );
                    aLine.append( " 0 R\r\n" );
                }
                aLine.append( ">>\r\n"
                              "endobj\r\n\r\n" );
                CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

                aFontIDToObject[ lit->m_nFontID ] = nFontObject;
            }
        }
    }
    osl_removeFile( aTmpName.pData );

    // emit embedded fonts
    for( FontEmbedData::iterator eit = m_aEmbeddedFonts.begin(); eit != m_aEmbeddedFonts.end(); ++eit )
    {
        std::map< sal_Int32, sal_Int32 > aObjects = emitEmbeddedFont( eit->first, eit->second );
        for( std::map< sal_Int32, sal_Int32 >::iterator fit = aObjects.begin(); fit != aObjects.end(); ++fit )
        {
            CHECK_RETURN( fit->second );
            aFontIDToObject[ fit->first ] = fit->second;
        }
    }

    nFontDict = createObject();
    CHECK_RETURN( updateObject( nFontDict ) );
    aLine.setLength( 0 );
    aLine.append( nFontDict );
    aLine.append( " 0 obj\r\n"
                  "<< " );
    for( std::map< sal_Int32, sal_Int32 >::iterator mit = aFontIDToObject.begin(); mit != aFontIDToObject.end(); ++mit )
    {
        aLine.append( "/F" );
        aLine.append( mit->first );
        aLine.append( ' ' );
        aLine.append( mit->second );
        aLine.append( " 0 R\r\n"
                      "   " );
    }
    aLine.append( ">>\r\n"
                  "endobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return nFontDict;
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
        aLine.append( " 0 obj\r\n"
                      "<< " );
        for( std::list<GradientEmit>::iterator it = m_aGradients.begin();
             it != m_aGradients.end(); ++it )
        {
            aLine.append( "/P" );
            aLine.append( it->m_nObject );
            aLine.append( ' ' );
            aLine.append( it->m_nObject );
            aLine.append( " 0 R\r\n   " );
        }
        aLine.append( ">>\r\n"
                      "endobj\r\n\r\n" );
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
        aLine.append( " 0 obj\r\n<< " );
        for( std::list<HatchEmit>::const_iterator hatch = m_aHatches.begin();
             hatch != m_aHatches.end(); ++hatch )
        {
            aLine.append( "/P" );
            aLine.append( hatch->m_nObject );
            aLine.append( ' ' );
            aLine.append( hatch->m_nObject );
            aLine.append( " 0 R\r\n   " );
        }
        for( std::list<BitmapPatternEmit>::const_iterator tile = m_aTilings.begin();
             tile != m_aTilings.end(); ++tile )
        {
            aLine.append( "/P" );
            aLine.append( tile->m_nObject );
            aLine.append( ' ' );
            aLine.append( tile->m_nObject );
            aLine.append( " 0 R\r\n   " );
        }
        aLine.append( ">>\r\n"
                      "endobj\r\n\r\n" );
        CHECK_RETURN( updateObject( nPatternDict ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit font dict
    sal_Int32 nFontDict = emitFonts();

    // emit xobject dict
    sal_Int32 nXObjectDict = 0;
    if( m_aBitmaps.begin() != m_aBitmaps.end()      ||
        m_aJPGs.begin() != m_aJPGs.end()            ||
        m_aTransparentObjects.begin() != m_aTransparentObjects.end()
        )
    {
        aLine.setLength( 0 );
        nXObjectDict = createObject();
        aLine.append( nXObjectDict );
        aLine.append( " 0 obj\r\n"
                      "<< " );
        for( std::list<BitmapEmit>::const_iterator it = m_aBitmaps.begin();
             it != m_aBitmaps.end(); ++it )
        {
            aLine.append( "/Im" );
            aLine.append( it->m_nObject );
            aLine.append( ' ' );
            aLine.append( it->m_nObject );
            aLine.append( " 0 R\r\n   " );
        }
        for( std::list<JPGEmit>::const_iterator jpeg = m_aJPGs.begin(); jpeg != m_aJPGs.end(); ++jpeg )
        {
            aLine.append( "/Im" );
            aLine.append( jpeg->m_nObject );
            aLine.append( ' ' );
            aLine.append( jpeg->m_nObject );
            aLine.append( " 0 R\r\n   " );
        }
        for( std::list<TransparencyEmit>::const_iterator t = m_aTransparentObjects.begin();
             t != m_aTransparentObjects.end(); ++t )
        {
            aLine.append( "/Tr" );
            aLine.append( t->m_nObject );
            aLine.append( ' ' );
            aLine.append( t->m_nObject );
            aLine.append( " 0 R\r\n   " );
        }

        aLine.append( ">>\r\n"
                      "endobj\r\n\r\n" );
        CHECK_RETURN( updateObject( nXObjectDict ) );
        CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    }

    // emit Resource dict
    sal_Int32 nResourceDict = createObject();
    CHECK_RETURN( updateObject( nResourceDict ) );
    aLine.setLength( 0 );
    aLine.append( nResourceDict );
    aLine.append( " 0 obj\r\n<<\r\n" );
    if( nFontDict )
    {
        aLine.append( "   /Font " );
        aLine.append( nFontDict );
        aLine.append( " 0 R\r\n" );
    }
    if( nXObjectDict )
    {
        aLine.append( "   /XObject " );
        aLine.append( nXObjectDict );
        aLine.append( " 0 R\r\n" );
    }
    if( nShadingDict )
    {
        aLine.append( "   /Shading " );
        aLine.append( nShadingDict );
        aLine.append( " 0 R\r\n" );
    }
    if( nPatternDict )
    {
        aLine.append( "   /Pattern " );
        aLine.append( nPatternDict );
        aLine.append( " 0 R\r\n" );
    }
    aLine.append( "   /ProcSet [ /PDF " );
    if( nXObjectDict )
        aLine.append( "/ImageC /ImageI " );
    aLine.append( "]\r\n" );
    aLine.append( ">>\r\n"
                  "endobj\r\n\r\n" );
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
    aLine.append( "<< /Type /Pages\r\n" );
    aLine.append( "   /Resources " );
    aLine.append( nResourceDict );
    aLine.append( " 0 R\r\n" );
    switch( m_eInheritedOrientation )
    {
        case PDFWriter::Landscape: aLine.append( "   /Rotate 90\r\n" );break;
        case PDFWriter::Seascape: aLine.append( "   /Rotate -90\r\n" );break;

        case PDFWriter::Inherit: // actually Inherit would be a bug, but insignificant
        case PDFWriter::Portrait:
        default:
            break;
    }
    aLine.append( "   /MediaBox [ 0 0 " );
    aLine.append( m_nInheritedPageWidth );
    aLine.append( ' ' );
    aLine.append( m_nInheritedPageHeight );
    aLine.append( " ]\r\n"
                  "   /Kids [ " );
    for( std::list<PDFPage>::const_iterator iter = m_aPages.begin(); iter != m_aPages.end(); ++iter )
    {
        aLine.append( iter->m_nPageObject );
        aLine.append( " 0 R\r\n"
                      "           " );
    }
    aLine.append( "]\r\n"
                  "   /Count " );
    aLine.append( (sal_Int32)m_aPages.size() );
    aLine.append( "\r\n"
                  ">>\r\n"
                  "endobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // emit Catalog
    m_nCatalogObject = createObject();
    if( ! updateObject( m_nCatalogObject ) )
        return false;
    aLine.setLength( 0 );
    aLine.append( m_nCatalogObject );
    aLine.append( " 0 obj\r\n"
                  "<< /Type /Catalog\r\n"
                  "   /Pages " );
    aLine.append( nTreeNode );
    aLine.append( " 0 R\r\n"
                  ">>\r\n"
                  "endobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

sal_Int32 PDFWriterImpl::emitInfoDict()
{
    sal_Int32 nObject = createObject();

    if( updateObject( nObject ) )
    {
        OStringBuffer aLine( 1024 );
        aLine.append( nObject );
        aLine.append( " 0 obj\r\n"
                      "<< " );
        if( m_aDocInfo.Title.Len() )
        {
            aLine.append( "/Title " );
            appendUnicodeTextString( m_aDocInfo.Title, aLine );
            aLine.append( "\r\n" );
        }
        if( m_aDocInfo.Author.Len() )
        {
            aLine.append( "/Author " );
            appendUnicodeTextString( m_aDocInfo.Author, aLine );
            aLine.append( "\r\n" );
        }
        if( m_aDocInfo.Subject.Len() )
        {
            aLine.append( "/Subject " );
            appendUnicodeTextString( m_aDocInfo.Subject, aLine );
            aLine.append( "\r\n" );
        }
        if( m_aDocInfo.Keywords.Len() )
        {
            aLine.append( "/Keywords " );
            appendUnicodeTextString( m_aDocInfo.Keywords, aLine );
            aLine.append( "\r\n" );
        }
        if( m_aDocInfo.Creator.Len() )
        {
            aLine.append( "/Creator " );
            appendUnicodeTextString( m_aDocInfo.Creator, aLine );
            aLine.append( "\r\n" );
        }
        if( m_aDocInfo.Producer.Len() )
        {
            aLine.append( "/Producer " );
            appendUnicodeTextString( m_aDocInfo.Producer, aLine );
            aLine.append( "\r\n" );
        }
        TimeValue aTVal, aGMT;
        oslDateTime aDT;
        osl_getSystemTime( &aGMT );
        osl_getLocalTimeFromSystemTime( &aGMT, &aTVal );
        osl_getDateTimeFromTimeValue( &aTVal, &aDT );
        aLine.append( "/CreationDate (D:" );
        aLine.append( (sal_Char)('0' + ((aDT.Year/1000)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Year/100)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Year/10)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Year)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Month/10)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Month)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Day/10)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Day)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Hours/10)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Hours)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Minutes/10)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Minutes)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Seconds/10)%10)) );
        aLine.append( (sal_Char)('0' + ((aDT.Seconds)%10)) );
        sal_uInt32 nDelta = 0;
        if( aGMT.Seconds > aTVal.Seconds )
        {
            aLine.append( "-" );
            nDelta = aGMT.Seconds-aTVal.Seconds;
        }
        else if( aGMT.Seconds < aTVal.Seconds )
        {
            aLine.append( "+" );
            nDelta = aTVal.Seconds-aGMT.Seconds;
        }
        else
            aLine.append( "Z" );
        if( nDelta )
        {
            aLine.append( (sal_Char)('0' + ((nDelta/36000)%10)) );
            aLine.append( (sal_Char)('0' + ((nDelta/3600)%10)) );
            aLine.append( "'" );
            aLine.append( (sal_Char)('0' + ((nDelta/600)%6)) );
            aLine.append( (sal_Char)('0' + ((nDelta/60)%10)) );
        }
        aLine.append( "')\r\n" );

        aLine.append( ">>\r\nendobj\r\n\r\n" );
        if( ! writeBuffer( aLine.getStr(), aLine.getLength() ) )
            nObject = 0;
    }
    else
        nObject = 0;

    return nObject;
}

bool PDFWriterImpl::emitTrailer()
{
    // emit doc info
    sal_Int32 nDocInfoObject = emitInfoDict();

    // emit xref table

    // remember start
    sal_uInt64 nXRefOffset = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nXRefOffset )) );
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
    aLine.append( "trailer\r\n"
                  "<< /Size " );
    aLine.append( (sal_Int32)(nObjects+1) );
    aLine.append( "\r\n"
                  "   /Root " );
    aLine.append( m_nCatalogObject );
    aLine.append( " 0 R\r\n" );
    if( nDocInfoObject )
    {
        aLine.append( "   /Info " );
        aLine.append( nDocInfoObject );
        aLine.append( " 0 R\r\n" );
    }
    aLine.append( ">>\r\n"
                  "startxref\r\n" );
    aLine.append( (sal_Int64)nXRefOffset );
    aLine.append( "\r\n"
                  "%%EOF\r\n" );
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

void PDFWriterImpl::registerGlyphs(
                                   int nGlyphs,
                                   long* pGlyphs,
                                   sal_Unicode* pUnicodes,
                                   sal_uInt8* pMappedGlyphs,
                                   sal_Int32* pMappedFontObjects,
                                   ImplFontData* pFallbackFonts[] )
{
    ImplFontData* pCurrentFont = m_pReferenceDevice->mpFontEntry->maFontSelData.mpFontData;
    FontSubset* pCurrentSubset = &m_aSubsets[ pCurrentFont ];
    if( pCurrentFont->mbSubsettable )
    {
        for( int i = 0; i < nGlyphs; i++ )
        {
            if( ! pGlyphs[i] )
                continue;

            FontSubset& rSubset = pFallbackFonts[i] ? m_aSubsets[pFallbackFonts[i]] : *pCurrentSubset;
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
                    rSubset.m_aSubsets.back().m_aMapping.size() > 254 )
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
        sal_Int32 nFontID = 0;
        FontEmbedData::iterator it = m_aEmbeddedFonts.find( pCurrentFont );
        if( it != m_aEmbeddedFonts.end() )
            nFontID = it->second.m_nNormalFontID;
        else
        {
            nFontID = m_nNextFID++;
            m_aEmbeddedFonts[ pCurrentFont ] = EmbedFont();
            m_aEmbeddedFonts[ pCurrentFont ].m_nNormalFontID = nFontID;
        }
        EmbedFont& rEmbedFont = m_aEmbeddedFonts[pCurrentFont];

        const std::map< sal_Unicode, sal_Int32 >* pEncoding = NULL;
        const std::map< sal_Unicode, rtl::OString >* pNonEncoded = NULL;
#ifndef REMOTE_APPSERVER
        getReferenceDevice()->ImplGetGraphics();
        pEncoding = m_pReferenceDevice->mpGraphics->GetFontEncodingVector( pCurrentFont, &pNonEncoded );
#endif

        std::map< sal_Unicode, sal_Int32 >::const_iterator enc_it;
        std::map< sal_Unicode, rtl::OString >::const_iterator nonenc_it;
        for( int i = 0; i < nGlyphs; i++ )
        {
            sal_Int32 nCurFontID = nFontID;
            sal_Unicode cChar = pUnicodes[i];
            if( pEncoding )
            {
                enc_it = pEncoding->find( cChar );
                if( enc_it != pEncoding->end() && enc_it->second > 0 )
                {
                    DBG_ASSERT( (enc_it->second & 0xffffff00) == 0, "Invalid character code" );
                    cChar = (sal_Unicode)enc_it->second;
                }
                else if( (enc_it == pEncoding->end() || enc_it->second == -1) &&
                         pNonEncoded &&
                         (nonenc_it = pNonEncoded->find( cChar )) != pNonEncoded->end() )
                {
                    nCurFontID = 0;
                    // find non encoded glyph
                    for( std::list< EmbedEncoding >::iterator nec_it = rEmbedFont.m_aExtendedEncodings.begin(); nec_it != rEmbedFont.m_aExtendedEncodings.end(); ++nec_it )
                    {
                        if( nec_it->m_aCMap.find( cChar ) != nec_it->m_aCMap.end() )
                        {
                            nCurFontID = nec_it->m_nFontID;
                            cChar = (sal_Unicode)nec_it->m_aCMap[ cChar ];
                            break;
                        }
                    }
                    if( nCurFontID == 0 ) // new nonencoded glyph
                    {
                        if( rEmbedFont.m_aExtendedEncodings.empty() || rEmbedFont.m_aExtendedEncodings.back().m_aEncVector.size() == 255 )
                        {
                            rEmbedFont.m_aExtendedEncodings.push_back( EmbedEncoding() );
                            rEmbedFont.m_aExtendedEncodings.back().m_nFontID = m_nNextFID++;
                        }
                        EmbedEncoding& rEncoding = rEmbedFont.m_aExtendedEncodings.back();
                        rEncoding.m_aEncVector.push_back( EmbedCode() );
                        rEncoding.m_aEncVector.back().m_aUnicode = cChar;
                        rEncoding.m_aEncVector.back().m_aName = nonenc_it->second;
                        rEncoding.m_aCMap[ cChar ] = (sal_Int8)(rEncoding.m_aEncVector.size()-1);
                        nCurFontID = rEncoding.m_nFontID;
                        cChar = (sal_Unicode)rEncoding.m_aCMap[ cChar ];
                    }
                }
                else
                    cChar = 0;
            }
            else
            {
                if( cChar & 0xff00 )
                {
                    // some characters can be used by conversion
                    if( cChar >= 0xf000 && cChar <= 0xf0ff ) // symbol encoding in private use area
                        cChar -= 0xf000;
                    else
                    {
                        String aString( cChar);
                        ByteString aChar( aString, RTL_TEXTENCODING_MS_1252 );
                        cChar = ((sal_Unicode)aChar.GetChar( 0 )) & 0x00ff;
                    }
                }
            }
            pMappedGlyphs[ i ] = (sal_Int8)cChar;
            pMappedFontObjects[ i ] = nCurFontID;
        }
    }
}

void PDFWriterImpl::drawLayout( SalLayout& rLayout, const String& rText, bool bTextLines )
{
    FontRelief eRelief = m_aCurrentPDFState.m_aFont.GetRelief();
    // relief takes precedence over shadow (see outdev3.cxx)
    if(  eRelief != RELIEF_NONE )
    {
        push( PUSH_ALL );

        Color aTextColor = m_aCurrentPDFState.m_aFont.GetColor();
        Color aTextLineColor = m_aCurrentPDFState.m_aTextLineColor;
        Color aReliefColor( COL_LIGHTGRAY );
        if( aTextColor == COL_BLACK )
            aTextColor = Color( COL_WHITE );
        if( aTextLineColor == COL_BLACK )
            aTextLineColor = Color( COL_WHITE );
        if( aTextColor == COL_WHITE )
            aReliefColor = Color( COL_BLACK );

        Font aSetFont = m_aCurrentPDFState.m_aFont;
        aSetFont.SetRelief( RELIEF_NONE );
        aSetFont.SetShadow( FALSE );

        aSetFont.SetColor( aReliefColor );
        setTextLineColor( aTextLineColor );
        setFont( aSetFont );
        long nOff = 1 + getReferenceDevice()->mnDPIX/300;
        if( eRelief == RELIEF_ENGRAVED )
            nOff = -nOff;

        rLayout.DrawOffset() += Point( nOff, nOff );
        updateGraphicsState();
        drawLayout( rLayout, rText, bTextLines );

        rLayout.DrawOffset() -= Point( nOff, nOff );
        setTextLineColor( aTextLineColor );
        aSetFont.SetColor( aTextColor );
        setFont( aSetFont );
        updateGraphicsState();
        drawLayout( rLayout, rText, bTextLines );

        // clean up the mess
        pop();
        return;
    }
    else if( m_aCurrentPDFState.m_aFont.IsShadow() )
    {
        Font aSaveFont = m_aCurrentPDFState.m_aFont;
        Color aSaveTextLineColor = m_aCurrentPDFState.m_aTextLineColor;

        Font& rFont = m_aCurrentPDFState.m_aFont;
        if( rFont.GetColor() == Color( COL_BLACK ) || rFont.GetColor().GetLuminance() < 8 )
            rFont.SetColor( Color( COL_LIGHTGRAY ) );
        else
            rFont.SetColor( Color( COL_BLACK ) );
        rFont.SetShadow( FALSE );
        setFont( rFont );
        setTextLineColor( rFont.GetColor() );
        updateGraphicsState();

        long nOff = 1 + ((m_pReferenceDevice->mpFontEntry->mnLineHeight-24)/24);
        if( rFont.IsOutline() )
            nOff++;
        rLayout.DrawBase() += Point( nOff, nOff );
        drawLayout( rLayout, rText, bTextLines );
        rLayout.DrawBase() -= Point( nOff, nOff );

        setFont( aSaveFont );
        setTextLineColor( aSaveTextLineColor );
        updateGraphicsState();
    }

    OStringBuffer aLine( 512 );

    // setup text colors (if necessary)
    bool bPop = false;
    if( m_aCurrentPDFState.m_aFont.IsOutline() &&
        m_aCurrentPDFState.m_aLineColor != m_aCurrentPDFState.m_aFont.GetColor() )
    {
        bPop = true;
        aLine.append( "q " );
        appendStrokingColor( m_aCurrentPDFState.m_aFont.GetColor(), aLine );
        aLine.append( "\r\n" );
    }
    else if( m_aCurrentPDFState.m_aFillColor != m_aCurrentPDFState.m_aFont.GetColor() )
    {
        bPop = true;
        aLine.append( "q " );
        appendNonStrokingColor( m_aCurrentPDFState.m_aFont.GetColor(), aLine );
        aLine.append( "\r\n" );
    }

    // begin text object
    aLine.append( "BT\r\n" );
    // outline attribute ?
    if( m_aCurrentPDFState.m_aFont.IsOutline() )
    {
        aLine.append( "1 Tr " );
        double fW = (double)m_aCurrentPDFState.m_aFont.GetHeight() / 30.0;
        m_aPages.back().appendMappedLength( fW, aLine );
        aLine.append ( " w\r\n" );
    }

    const int nMaxGlyphs = 256;

    // note: the layout calculates in outdevs device pixel !!

    long pGlyphs[nMaxGlyphs];
    sal_uInt8 pMappedGlyphs[nMaxGlyphs];
    sal_Int32 pMappedFontObjects[nMaxGlyphs];
    sal_Unicode pUnicodes[nMaxGlyphs];
    int pCharPosAry[nMaxGlyphs];
    long nAdvanceWidths[nMaxGlyphs];
    ImplFontData* pFallbackFonts[nMaxGlyphs];
    long *pAdvanceWidths = m_aCurrentPDFState.m_aFont.IsVertical() ? nAdvanceWidths : NULL;
    long nGlyphFlags[nMaxGlyphs];
    int nGlyphs;
    int nIndex = 0;
    Point aPos, aLastPos(0, 0);
    bool bFirst = true;
    int nMinCharPos = 0, nMaxCharPos = rText.Len()-1;
    double fXScale = 1.0;
    sal_Int32 nFontHeight = m_pReferenceDevice->mpFontEntry->maFontSelData.mnHeight;
    TextAlign eAlign = m_aCurrentPDFState.m_aFont.GetAlign();

    // transform font height back to current units
    nFontHeight = m_pReferenceDevice->ImplDevicePixelToLogicHeight( nFontHeight );
    if( m_aCurrentPDFState.m_aFont.GetWidth() )
    {
        Font aFont( m_aCurrentPDFState.m_aFont );
        aFont.SetWidth( 0 );
        FontMetric aMetric = m_pReferenceDevice->GetFontMetric( aFont );
        if( aMetric.GetWidth() != m_aCurrentPDFState.m_aFont.GetWidth() )
        {
            fXScale =
                (double)m_aCurrentPDFState.m_aFont.GetWidth() /
                (double)aMetric.GetWidth();
        }
        // force state before GetFontMetric
        m_pReferenceDevice->ImplNewFont();
    }
    // if the mapmode is distorted we need to adjust for that also
    if( m_aCurrentPDFState.m_aMapMode.GetScaleX() != m_aCurrentPDFState.m_aMapMode.GetScaleY() )
    {
        fXScale *= (double)(m_aCurrentPDFState.m_aMapMode.GetScaleX() / m_aCurrentPDFState.m_aMapMode.GetScaleY());
    }

    double fAngle = (double)m_aCurrentPDFState.m_aFont.GetOrientation() * M_PI / 1800.0;
    double fSin = sin( fAngle );
    double fCos = cos( fAngle );

    sal_Int32 nLastMappedFont = -1;
    while( (nGlyphs = rLayout.GetNextGlyphs( nMaxGlyphs, pGlyphs, aPos, nIndex, pAdvanceWidths, pCharPosAry )) )
    {
        // back transformation to current coordinate system
        aPos = m_pReferenceDevice->PixelToLogic( aPos );

        Point aDiff;
        if ( eAlign == ALIGN_BOTTOM )
            aDiff.Y() -= m_pReferenceDevice->GetFontMetric().GetDescent();
        else if ( eAlign == ALIGN_TOP )
            aDiff.Y() += m_pReferenceDevice->GetFontMetric().GetAscent();

        if( aDiff.X() || aDiff.Y() )
        {
            aDiff = Point( (int)(fXScale * fCos * (double)aDiff.X() + fSin * (double)aDiff.Y()),
                               -(int)(fXScale * fSin * (double)aDiff.X() - fCos * (double)aDiff.Y()) );
            aPos += aDiff;
        }

        for( int i = 0; i < nGlyphs; i++ )
        {
            if( pGlyphs[i] & GF_FONTMASK )
                pFallbackFonts[i] = ((MultiSalLayout&)rLayout).GetFallbackFontData((pGlyphs[i] & GF_FONTMASK) >> GF_FONTSHIFT);
            else
                pFallbackFonts[i] = NULL;

            nGlyphFlags[i] = (pGlyphs[i] & GF_FLAGMASK);
#ifndef WNT
            // #104930# workaround for Win32 bug: the glyph ids are actually
            // Unicodes for vertical fonts because Win32 does not return
            // the correct glyph ids; this is indicated by GF_ISCHAR which is
            // needed in SalGraphics::CreateFontSubset to convert the Unicodes
            // to vertical glyph ids. Doing this here on a per character
            // basis would be a major performance hit.
            pGlyphs[i] &= GF_IDXMASK;
#endif
            if( pCharPosAry[i] >= nMinCharPos && pCharPosAry[i] <= nMaxCharPos )
                pUnicodes[i] = rText.GetChar( pCharPosAry[i] );
            else
                pUnicodes[i] = 0;
            // note: in case of ctl one character may result
            // in multiple glyphs. The current SalLayout
            // implementations set -1 then to indicate that no direct
            // mapping is possible
        }
        registerGlyphs( nGlyphs, pGlyphs, pUnicodes, pMappedGlyphs, pMappedFontObjects, pFallbackFonts );

        if( pAdvanceWidths )
        {
            // have to emit each glyph on its own
            long nXOffset = 0;
            for( int n = 0; n < nGlyphs; n++ )
            {
                double fDeltaAngle = 0.0;
                double fYScale = 1.0;
                double fTempXScale = fXScale;

                Point aDeltaPos;
                if( ( nGlyphFlags[n] & GF_ROTMASK ) == GF_ROTL )
                {
                    fDeltaAngle = M_PI/2.0;
                    aDeltaPos.X() = m_pReferenceDevice->GetFontMetric().GetAscent() - m_pReferenceDevice->GetFontMetric().GetDescent();
                    aDeltaPos.Y() = (int)((double)m_pReferenceDevice->GetFontMetric().GetDescent() * fXScale);
                    fYScale = fXScale;
                    fTempXScale = 1.0;
                }
                else if( ( nGlyphFlags[n] & GF_ROTMASK ) == GF_ROTR )
                {
                    fDeltaAngle = -M_PI/2.0;
                    aDeltaPos.X() = (int)((double)m_pReferenceDevice->GetFontMetric().GetDescent()*fXScale);
                    aDeltaPos.Y() = -m_pReferenceDevice->GetFontMetric().GetAscent() + m_pReferenceDevice->GetFontMetric().GetDescent();
                    fYScale = fXScale;
                    fTempXScale = 1.0;
                }
                aDeltaPos += (m_pReferenceDevice->PixelToLogic( Point( (int)((double)nXOffset/fXScale)/rLayout.GetUnitsPerPixel(), 0 ) ) - m_pReferenceDevice->PixelToLogic( Point() ) );
                nXOffset += pAdvanceWidths[n];
                if( ! pGlyphs[n] )
                    continue;


                aDeltaPos = Point( (int)(fXScale * fCos * (double)aDeltaPos.X() + fSin * (double)aDeltaPos.Y()),
                                   -(int)(fXScale * fSin * (double)aDeltaPos.X() - fCos * (double)aDeltaPos.Y()) );

                double fDSin = sin( fAngle+fDeltaAngle );
                double fDCos = cos( fAngle+fDeltaAngle );

                appendDouble( fTempXScale*fDCos, aLine );
                aLine.append( ' ' );
                appendDouble( fDSin*fTempXScale, aLine );
                aLine.append( ' ' );
                appendDouble( -fDSin*fYScale, aLine );
                aLine.append( ' ' );
                appendDouble( fDCos*fYScale, aLine );
                aLine.append( ' ' );
                m_aPages.back().appendPoint( aPos+aDeltaPos, aLine );
                aLine.append( " Tm" );
                if( nLastMappedFont != pMappedFontObjects[n] )
                {
                    nLastMappedFont = pMappedFontObjects[n];
                    aLine.append( " /F" );
                    aLine.append( pMappedFontObjects[n] );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nFontHeight, aLine, true );
                    aLine.append( " Tf" );
                }
                aLine.append( " <" );
                appendHex( (sal_Int8)pMappedGlyphs[n], aLine );
                aLine.append( "> Tj\r\n" );
            }
        }
        else // normal case
        {
            // optimize use of Td vs. Tm
            if( fXScale == 1.0 && fCos == 1.0 && fSin == 0.0 )
            {
                if( bFirst )
                {
                    m_aPages.back().appendPoint( aPos, aLine );
                    bFirst = false;
                }
                else
                {
                    Point aDiff = aPos - aLastPos;
                    m_aPages.back().appendMappedLength( aDiff.X(), aLine, false );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( aDiff.Y(), aLine, true );
                }
                aLine.append( " Td " );
                aLastPos = aPos;
            }
            else
            {
                appendDouble( fXScale*fCos, aLine );
                aLine.append( ' ' );
                appendDouble( fSin*fXScale, aLine );
                aLine.append( ' ' );
                appendDouble( -fSin, aLine );
                aLine.append( ' ' );
                appendDouble( fCos, aLine );
                aLine.append( ' ' );
                m_aPages.back().appendPoint( aPos, aLine );
                aLine.append( " Tm\r\n" );
            }
            int nLast = 0;
            while( nLast < nGlyphs )
            {
                while( ! pGlyphs[nLast] && nLast < nGlyphs )
                    nLast++;
                if( nLast >= nGlyphs )
                    break;

                int nNext = nLast+1;
                while( nNext < nGlyphs && pMappedFontObjects[ nNext ] == pMappedFontObjects[nLast] && pGlyphs[nNext] )
                    nNext++;
                if( nLastMappedFont != pMappedFontObjects[nLast] )
                {
                    aLine.append( "/F" );
                    aLine.append( pMappedFontObjects[nLast] );
                    aLine.append( ' ' );
                    m_aPages.back().appendMappedLength( nFontHeight, aLine, true );
                    aLine.append( " Tf " );
                    nLastMappedFont = pMappedFontObjects[nLast];
                }
                aLine.append( "<" );
                for( int i = nLast; i < nNext; i++ )
                {
                    appendHex( (sal_Int8)pMappedGlyphs[i], aLine );
                    if( i && (i % 35) == 0 )
                        aLine.append( "\r\n" );
                }
                aLine.append( "> Tj\r\n" );

                nLast = nNext;
            }
        }
    }

    // end textobject
    aLine.append( "ET\r\n" );
    if( bPop )
        aLine.append( "Q\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );

    // draw eventual textlines
    FontStrikeout eStrikeout = m_aCurrentPDFState.m_aFont.GetStrikeout();
    FontUnderline eUnderline = m_aCurrentPDFState.m_aFont.GetUnderline();
    if( bTextLines &&
        (
         ( eUnderline != UNDERLINE_NONE && eUnderline != UNDERLINE_DONTKNOW ) ||
         ( eStrikeout != STRIKEOUT_NONE && eStrikeout != STRIKEOUT_DONTKNOW )
         )
        )
    {
        BOOL bUnderlineAbove = OutputDevice::ImplIsUnderlineAbove( m_aCurrentPDFState.m_aFont );
        if( m_aCurrentPDFState.m_aFont.IsWordLineMode() )
        {
            Point aPos, aStartPt;
            long nWidth = 0, nAdvance=0;
            for( int nStart = 0;;)
            {
                long nGlyphIndex;
                if( !rLayout.GetNextGlyphs( 1, &nGlyphIndex, aPos, nStart, &nAdvance ) )
                    break;

                if( !rLayout.IsSpacingGlyph( nGlyphIndex ) )
                {
                    if( !nWidth )
                        aStartPt = aPos;

                    nWidth += nAdvance;
                }
                else if( nWidth > 0 )
                {
                    drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                                  m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                                  eStrikeout, eUnderline, bUnderlineAbove );
                    nWidth = 0;
                }
            }

            if( nWidth > 0 )
            {
                drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                              m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                              eStrikeout, eUnderline, bUnderlineAbove );
            }
        }
        else
        {
            Point aStartPt = rLayout.GetDrawPosition();
            int nWidth = rLayout.GetTextWidth() / rLayout.GetUnitsPerPixel();
            drawTextLine( m_pReferenceDevice->PixelToLogic( aStartPt ),
                          m_pReferenceDevice->ImplDevicePixelToLogicWidth( nWidth ),
                          eStrikeout, eUnderline, bUnderlineAbove );
        }
    }

    // write eventual emphasis marks
    if( m_aCurrentPDFState.m_aFont.GetEmphasisMark() & EMPHASISMARK_STYLE )
    {
        PolyPolygon             aEmphPoly;
        Rectangle               aEmphRect1;
        Rectangle               aEmphRect2;
        long                    nEmphYOff;
        long                    nEmphWidth;
        long                    nEmphHeight;
        BOOL                    bEmphPolyLine;
        FontEmphasisMark        nEmphMark;

        push( PUSH_ALL );

        aLine.setLength( 0 );
        aLine.append( "q\r\n" );

        nEmphMark = m_pReferenceDevice->ImplGetEmphasisMarkStyle( m_aCurrentPDFState.m_aFont );
        if ( nEmphMark & EMPHASISMARK_POS_BELOW )
            nEmphHeight = m_pReferenceDevice->mnEmphasisDescent;
        else
            nEmphHeight = m_pReferenceDevice->mnEmphasisAscent;
        m_pReferenceDevice->ImplGetEmphasisMark( aEmphPoly,
                                                 bEmphPolyLine,
                                                 aEmphRect1,
                                                 aEmphRect2,
                                                 nEmphYOff,
                                                 nEmphWidth,
                                                 nEmphMark,
                                                 nEmphHeight,
                                                 m_pReferenceDevice->mpFontEntry->mnOrientation );
        if ( bEmphPolyLine )
        {
            setLineColor( m_aCurrentPDFState.m_aFont.GetColor() );
            setFillColor( Color( COL_TRANSPARENT ) );
        }
        else
        {
            setFillColor( m_aCurrentPDFState.m_aFont.GetColor() );
            setLineColor( Color( COL_TRANSPARENT ) );
        }
        writeBuffer( aLine.getStr(), aLine.getLength() );

        Point aOffset = Point(0,0);

        if ( nEmphMark & EMPHASISMARK_POS_BELOW )
            aOffset.Y() += m_pReferenceDevice->mpFontEntry->maMetric.mnDescent + nEmphYOff;
        else
            aOffset.Y() -= m_pReferenceDevice->mpFontEntry->maMetric.mnAscent + nEmphYOff;

        long nEmphWidth2     = nEmphWidth / 2;
        long nEmphHeight2    = nEmphHeight / 2;
        aOffset += Point( nEmphWidth2, nEmphHeight2 );

        if ( eAlign == ALIGN_BOTTOM )
            aOffset.Y() -= m_pReferenceDevice->mpFontEntry->maMetric.mnDescent;
        else if ( eAlign == ALIGN_TOP )
            aOffset.Y() += m_pReferenceDevice->mpFontEntry->maMetric.mnAscent;

        for( int nStart = 0;;)
        {
            Point aPos;
            long nGlyphIndex, nAdvance;
            if( !rLayout.GetNextGlyphs( 1, &nGlyphIndex, aPos, nStart, &nAdvance ) )
                break;

            if( !rLayout.IsSpacingGlyph( nGlyphIndex ) )
            {
                Point aAdjOffset = aOffset;
                aAdjOffset.X() += (nAdvance - nEmphWidth) / 2;
                aAdjOffset = Point( (int)(fXScale * fCos * (double)aAdjOffset.X() + fSin * (double)aAdjOffset.Y()),
                                 -(int)(fXScale * fSin * (double)aAdjOffset.X() - fCos * (double)aAdjOffset.Y()) );

                aAdjOffset -= Point( nEmphWidth2, nEmphHeight2 );

                aPos += aAdjOffset;
                aPos = m_pReferenceDevice->PixelToLogic( aPos );
                drawEmphasisMark( aPos.X(), aPos.Y(),
                                  aEmphPoly, bEmphPolyLine,
                                  aEmphRect1, aEmphRect2 );
            }
        }

        writeBuffer( "Q\r\n", 3 );
        pop();
    }

}

void PDFWriterImpl::drawEmphasisMark( long nX, long nY,
                                      const PolyPolygon& rPolyPoly, BOOL bPolyLine,
                                      const Rectangle& rRect1, const Rectangle& rRect2 )
{
    // TODO: pass nWidth as width of this mark
    long nWidth = 0;

    if ( rPolyPoly.Count() )
    {
        if ( bPolyLine )
        {
            Polygon aPoly = rPolyPoly.GetObject( 0 );
            aPoly.Move( nX, nY );
            drawPolyLine( aPoly );
        }
        else
        {
            PolyPolygon aPolyPoly = rPolyPoly;
            aPolyPoly.Move( nX, nY );
            drawPolyPolygon( aPolyPoly );
        }
    }

    if ( !rRect1.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect1.Left(),
                                nY+rRect1.Top() ), rRect1.GetSize() );
        drawRectangle( aRect );
    }

    if ( !rRect2.IsEmpty() )
    {
        Rectangle aRect( Point( nX+rRect2.Left(),
                                nY+rRect2.Top() ), rRect2.GetSize() );

        drawRectangle( aRect );
    }
}

void PDFWriterImpl::drawText( const Point& rPos, const String& rText, xub_StrLen nIndex, xub_StrLen nLen, bool bTextLines )
{
    MARK( "drawText" );

    updateGraphicsState();

    // get a layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
        pLayout->Release();
    }
}

void PDFWriterImpl::drawTextArray( const Point& rPos, const String& rText, const long* pDXArray, xub_StrLen nIndex, xub_StrLen nLen, bool bTextLines )
{
    MARK( "drawText with array" );

    updateGraphicsState();

    // get a layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos, 0, pDXArray );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
        pLayout->Release();
    }
}

void PDFWriterImpl::drawStretchText( const Point& rPos, ULONG nWidth, const String& rText, xub_StrLen nIndex, xub_StrLen nLen, bool bTextLines )
{
    MARK( "drawStretchText" );

    updateGraphicsState();

    // get a layout from the OuputDevice's SalGraphics
    // this also enforces font substitution and sets the font on SalGraphics
    SalLayout* pLayout = m_pReferenceDevice->ImplLayout( rText, nIndex, nLen, rPos, nWidth );
    if( pLayout )
    {
        drawLayout( *pLayout, rText, bTextLines );
        pLayout->Release();
    }
}

void PDFWriterImpl::drawText( const Rectangle& rRect, const String& rOrigStr, USHORT nStyle, bool bTextLines )
{
    long        nWidth          = rRect.GetWidth();
    long        nHeight         = rRect.GetHeight();

    if ( nWidth <= 0 || nHeight <= 0 )
        return;

    MARK( "drawText with rectangle" );

    updateGraphicsState();

    // clip with rectangle
    OStringBuffer aLine;
    aLine.append( "q " );
    m_aPages.back().appendRect( rRect, aLine );
    aLine.append( " W* n\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );

    // if disabled text is needed, put in here

    Point       aPos            = rRect.TopLeft();

    long        nTextHeight     = m_pReferenceDevice->GetTextHeight();
    TextAlign   eAlign          = m_aCurrentPDFState.m_aFont.GetAlign();
    xub_StrLen  nMnemonicPos    = STRING_NOTFOUND;

    String aStr = rOrigStr;
    if ( nStyle & TEXT_DRAW_MNEMONIC )
        aStr = m_pReferenceDevice->GetNonMnemonicString( aStr, nMnemonicPos );

    // multiline text
    if ( nStyle & TEXT_DRAW_MULTILINE )
    {
        XubString               aLastLine;
        ImplMultiTextLineInfo   aMultiLineInfo;
        ImplTextLineInfo*       pLineInfo;
        long                    nMaxTextWidth;
        xub_StrLen              i;
        xub_StrLen              nLines;
        xub_StrLen              nFormatLines;

        if ( nTextHeight )
        {
            nMaxTextWidth = m_pReferenceDevice->ImplGetTextLines( aMultiLineInfo, nWidth, aStr, nStyle );
            nLines = (xub_StrLen)(nHeight/nTextHeight);
            nFormatLines = aMultiLineInfo.Count();
            if ( !nLines )
                nLines = 1;
            if ( nFormatLines > nLines )
            {
                if ( nStyle & TEXT_DRAW_ENDELLIPSIS )
                {
                    // handle last line
                    nFormatLines = nLines-1;

                    pLineInfo = aMultiLineInfo.GetLine( nFormatLines );
                    aLastLine = aStr.Copy( pLineInfo->GetIndex() );
                    aLastLine.ConvertLineEnd( LINEEND_LF );
                    // replace line feed by space
                    xub_StrLen nLastLineLen = aLastLine.Len();
                    for ( i = 0; i < nLastLineLen; i++ )
                    {
                        if ( aLastLine.GetChar( i ) == _LF )
                            aLastLine.SetChar( i, ' ' );
                    }
                    aLastLine = m_pReferenceDevice->GetEllipsisString( aLastLine, nWidth, nStyle );
                    nStyle &= ~(TEXT_DRAW_VCENTER | TEXT_DRAW_BOTTOM);
                    nStyle |= TEXT_DRAW_TOP;
                }
            }

            // vertical alignment
            if ( nStyle & TEXT_DRAW_BOTTOM )
                aPos.Y() += nHeight-(nFormatLines*nTextHeight);
            else if ( nStyle & TEXT_DRAW_VCENTER )
                aPos.Y() += (nHeight-(nFormatLines*nTextHeight))/2;

            // draw all lines excluding the last
            for ( i = 0; i < nFormatLines; i++ )
            {
                pLineInfo = aMultiLineInfo.GetLine( i );
                if ( nStyle & TEXT_DRAW_RIGHT )
                    aPos.X() += nWidth-pLineInfo->GetWidth();
                else if ( nStyle & TEXT_DRAW_CENTER )
                    aPos.X() += (nWidth-pLineInfo->GetWidth())/2;
                xub_StrLen nIndex   = pLineInfo->GetIndex();
                xub_StrLen nLineLen = pLineInfo->GetLen();
                drawText( aPos, aStr, nIndex, nLineLen, bTextLines );
                // mnemonics should not appear in documents,
                // if the need arises, put them in here
                aPos.Y() += nTextHeight;
                aPos.X() = rRect.Left();
            }


            // output last line left adjusted since it was shortened
            if ( aLastLine.Len() )
                drawText( aPos, aLastLine, 0, STRING_LEN, bTextLines );
        }
    }
    else
    {
        long nTextWidth = m_pReferenceDevice->GetTextWidth( aStr );

        // Evt. Text kuerzen
        if ( nTextWidth > nWidth )
        {
            if ( nStyle & (TEXT_DRAW_ENDELLIPSIS | TEXT_DRAW_PATHELLIPSIS | TEXT_DRAW_NEWSELLIPSIS) )
            {
                aStr = m_pReferenceDevice->GetEllipsisString( aStr, nWidth, nStyle );
                nStyle &= ~(TEXT_DRAW_CENTER | TEXT_DRAW_RIGHT);
                nStyle |= TEXT_DRAW_LEFT;
                nTextWidth = m_pReferenceDevice->GetTextWidth( aStr );
            }
        }

        // vertical alignment
        if ( nStyle & TEXT_DRAW_RIGHT )
            aPos.X() += nWidth-nTextWidth;
        else if ( nStyle & TEXT_DRAW_CENTER )
            aPos.X() += (nWidth-nTextWidth)/2;

        if ( nStyle & TEXT_DRAW_BOTTOM )
            aPos.Y() += nHeight-nTextHeight;
        else if ( nStyle & TEXT_DRAW_VCENTER )
            aPos.Y() += (nHeight-nTextHeight)/2;

        // mnemonics should be inserted here if the need arises

        // draw the actual text
        drawText( aPos, aStr, 0, STRING_LEN, bTextLines );
    }

    // reset clip region to original value
    aLine.setLength( 0 );
    aLine.append( "Q\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawLine( const Point& rStart, const Point& rStop )
{
    MARK( "drawLine" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine;
    aLine.append( "q 0 w " );
    m_aPages.back().appendPoint( rStart, aLine );
    aLine.append( " m " );
    m_aPages.back().appendPoint( rStop, aLine );
    aLine.append( " l S Q\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawLine( const Point& rStart, const Point& rStop, const LineInfo& rInfo )
{
    MARK( "drawLine with LineInfo" );
    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

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

void PDFWriterImpl::drawWaveLine( const Point& rStart, const Point& rStop, sal_Int32 nDelta, sal_Int32 nLineWidth )
{
    Point aDiff( rStop-rStart );
    double fLen = sqrt( (double)(aDiff.X()*aDiff.X() + aDiff.Y()*aDiff.Y()) );
    if( fLen < 1.0 )
        return;

    MARK( "drawWaveLine" );
    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine( 512 );
    aLine.append( "q " );
    m_aPages.back().appendMappedLength( nLineWidth, aLine, true );
    aLine.append( " w " );

    appendDouble( (double)aDiff.X()/fLen, aLine );
    aLine.append( ' ' );
    appendDouble( -(double)aDiff.Y()/fLen, aLine );
    aLine.append( ' ' );
    appendDouble( (double)aDiff.Y()/fLen, aLine );
    aLine.append( ' ' );
    appendDouble( (double)aDiff.X()/fLen, aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rStart, aLine );
    aLine.append( " cm " );
    m_aPages.back().appendWaveLine( (sal_Int32)fLen, 0, nDelta, aLine );
    aLine.append( "Q\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

#define WCONV( x ) m_pReferenceDevice->ImplDevicePixelToLogicWidth( x )
#define HCONV( x ) m_pReferenceDevice->ImplDevicePixelToLogicHeight( x )

void PDFWriterImpl::drawTextLine( const Point& rPos, long nWidth, FontStrikeout eStrikeout, FontUnderline eUnderline, bool bUnderlineAbove )
{
    if ( !nWidth ||
         ( ((eStrikeout == STRIKEOUT_NONE)||(eStrikeout == STRIKEOUT_DONTKNOW)) &&
           ((eUnderline == UNDERLINE_NONE)||(eUnderline == UNDERLINE_DONTKNOW)) ) )
        return;

    MARK( "drawTextLine" );
    updateGraphicsState();

    // note: units in pFontEntry are ref device pixel
    ImplFontEntry*  pFontEntry = m_pReferenceDevice->mpFontEntry;
    Color           aUnderlineColor = m_aCurrentPDFState.m_aTextLineColor;
    Color           aStrikeoutColor = m_aCurrentPDFState.m_aFont.GetColor();
    long            nLineHeight = 0;
    long            nLinePos = 0;
    long            nLinePos2 = 0;
    bool            bNormalLines = true;

    if ( bNormalLines &&
         ((eStrikeout == STRIKEOUT_SLASH) || (eStrikeout == STRIKEOUT_X)) )
    {
        String aStrikeoutChar = String::CreateFromAscii( eStrikeout == STRIKEOUT_SLASH ? "/" : "X" );
        String aStrikeout = aStrikeoutChar;
        while( m_pReferenceDevice->GetTextWidth( aStrikeout ) < nWidth )
            aStrikeout.Append( aStrikeout );

        // do not get broader than nWidth modulo 1 character
        while( m_pReferenceDevice->GetTextWidth( aStrikeout ) >= nWidth )
            aStrikeout.Erase( 0, 1 );
        aStrikeout.Append( aStrikeoutChar );
        BOOL bShadow = m_aCurrentPDFState.m_aFont.IsShadow();
        if( bShadow )
        {
            Font aFont = m_aCurrentPDFState.m_aFont;
            aFont.SetShadow( FALSE );
            setFont( aFont );
            updateGraphicsState();
        }
        drawText( rPos, aStrikeout, 0, aStrikeout.Len(), false );
        if( bShadow )
        {
            Font aFont = m_aCurrentPDFState.m_aFont;
            aFont.SetShadow( TRUE );
            setFont( aFont );
            updateGraphicsState();
        }

        switch( eUnderline )
        {
            case UNDERLINE_NONE:
            case UNDERLINE_DONTKNOW:
            case UNDERLINE_SMALLWAVE:
            case UNDERLINE_WAVE:
            case UNDERLINE_DOUBLEWAVE:
            case UNDERLINE_BOLDWAVE:
                bNormalLines = false;
        }
    }

    Point aPos( rPos );
    TextAlign eAlign = m_aCurrentPDFState.m_aFont.GetAlign();
    if( eAlign == ALIGN_TOP )
        aPos.Y() += HCONV( pFontEntry->maMetric.mnAscent );
    else if( eAlign == ALIGN_BOTTOM )
        aPos.Y() -= HCONV( pFontEntry->maMetric.mnDescent );

    OStringBuffer aLine( 512 );
    // save GS
    aLine.append( "q " );

    // rotate and translate matrix
    double fAngle = (double)m_aCurrentPDFState.m_aFont.GetOrientation() * M_PI / 1800.0;
    double fSin = sin( fAngle );
    double fCos = cos( fAngle );
    appendDouble( fCos, aLine );
    aLine.append( ' ' );
    appendDouble( fSin, aLine );
    aLine.append( ' ' );
    appendDouble( -fSin, aLine );
    aLine.append( ' ' );
    appendDouble( fCos, aLine );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( aPos, aLine );
    aLine.append( " cm\r\n" );

    if ( aUnderlineColor.GetTransparency() != 0 )
        aUnderlineColor = aStrikeoutColor;

    if ( (eUnderline == UNDERLINE_SMALLWAVE) ||
         (eUnderline == UNDERLINE_WAVE) ||
         (eUnderline == UNDERLINE_DOUBLEWAVE) ||
         (eUnderline == UNDERLINE_BOLDWAVE) )
    {
        appendStrokingColor( aUnderlineColor, aLine );
        aLine.append( "\r\n" );

        if ( bUnderlineAbove )
        {
            if ( !pFontEntry->maMetric.mnAboveWUnderlineSize )
                m_pReferenceDevice->ImplInitAboveTextLineSize();
            nLinePos = HCONV( pFontEntry->maMetric.mnAboveWUnderlineOffset );
            nLineHeight = HCONV( pFontEntry->maMetric.mnAboveWUnderlineSize );
        }
        else
        {
            if ( !pFontEntry->maMetric.mnWUnderlineSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLinePos = HCONV( pFontEntry->maMetric.mnWUnderlineOffset );
            nLineHeight = HCONV( pFontEntry->maMetric.mnWUnderlineSize );

        }
        if ( (eUnderline == UNDERLINE_SMALLWAVE) &&
             (nLineHeight > 3) )
            nLineHeight = 3;

        long nLineWidth = nLineHeight;

        if ( eUnderline == UNDERLINE_BOLDWAVE )
            nLineWidth = 3*nLineWidth/2;

        m_aPages.back().appendMappedLength( nLineWidth, aLine );
        aLine.append( " w " );

        if ( eUnderline == UNDERLINE_DOUBLEWAVE )
        {
            long nOrgLineHeight = nLineHeight;
            nLineHeight /= 3;
            if ( nLineHeight < 2 )
            {
                if ( nOrgLineHeight > 1 )
                    nLineHeight = 2;
                else
                    nLineHeight = 1;
            }
            long nLineDY = nOrgLineHeight-(nLineHeight*2);
            if ( nLineDY < nLineWidth )
                nLineDY = nLineWidth;
            long nLineDY2 = nLineDY/2;
            if ( !nLineDY2 )
                nLineDY2 = 1;

            nLinePos -= nLineWidth-nLineDY2;

            m_aPages.back().appendWaveLine( nWidth, -nLinePos, 2*nLineHeight, aLine );

            nLinePos += nLineWidth+nLineDY;
            m_aPages.back().appendWaveLine( nWidth, -nLinePos, 2*nLineHeight, aLine );
        }
        else
        {
            if( eUnderline != UNDERLINE_BOLDWAVE )
                nLinePos -= nLineWidth/2;
            m_aPages.back().appendWaveLine( nWidth, -nLinePos, nLineHeight, aLine );
        }

        if ( (eStrikeout == STRIKEOUT_NONE) ||
             (eStrikeout == STRIKEOUT_DONTKNOW) )
            bNormalLines = false;
    }

    if ( bNormalLines )
    {
        if ( eUnderline > UNDERLINE_BOLDWAVE )
            eUnderline = UNDERLINE_SINGLE;

        if ( (eUnderline == UNDERLINE_SINGLE) ||
             (eUnderline == UNDERLINE_DOTTED) ||
             (eUnderline == UNDERLINE_DASH) ||
             (eUnderline == UNDERLINE_LONGDASH) ||
             (eUnderline == UNDERLINE_DASHDOT) ||
             (eUnderline == UNDERLINE_DASHDOTDOT) )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveUnderlineSize )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnAboveUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnAboveUnderlineOffset );
            }
            else
            {
                if ( !pFontEntry->maMetric.mnUnderlineSize )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnUnderlineOffset );
            }
        }
        else if ( (eUnderline == UNDERLINE_BOLD) ||
                  (eUnderline == UNDERLINE_BOLDDOTTED) ||
                  (eUnderline == UNDERLINE_BOLDDASH) ||

                  (eUnderline == UNDERLINE_BOLDLONGDASH) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOT) ||
                  (eUnderline == UNDERLINE_BOLDDASHDOTDOT) )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveBUnderlineSize )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnAboveBUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnAboveBUnderlineOffset );
            }
            else
            {
                if ( !pFontEntry->maMetric.mnBUnderlineSize )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnBUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnBUnderlineOffset );
                nLinePos += nLineHeight/2;
            }
        }
        else if ( eUnderline == UNDERLINE_DOUBLE )
        {
            if ( bUnderlineAbove )
            {
                if ( !pFontEntry->maMetric.mnAboveDUnderlineSize )
                    m_pReferenceDevice->ImplInitAboveTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnAboveDUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnAboveDUnderlineOffset1 );
                nLinePos2   = HCONV( pFontEntry->maMetric.mnAboveDUnderlineOffset2 );
            }
            else
            {
                if ( !pFontEntry->maMetric.mnDUnderlineSize )
                    m_pReferenceDevice->ImplInitTextLineSize();
                nLineHeight = HCONV( pFontEntry->maMetric.mnDUnderlineSize );
                nLinePos    = HCONV( pFontEntry->maMetric.mnDUnderlineOffset1 );
                nLinePos2   = HCONV( pFontEntry->maMetric.mnDUnderlineOffset2 );
            }
        }
        else
            nLineHeight = 0;

        if ( nLineHeight )
        {
            m_aPages.back().appendMappedLength( nLineHeight, aLine, true );
            aLine.append( " w " );
            appendStrokingColor( aUnderlineColor, aLine );
            aLine.append( "\r\n" );

            if ( (eUnderline == UNDERLINE_DOTTED) ||
                 (eUnderline == UNDERLINE_BOLDDOTTED) )
            {
                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nLineHeight, aLine, false );
                aLine.append( " ] 0 d\r\n" );
            }
            else if ( (eUnderline == UNDERLINE_DASH) ||
                      (eUnderline == UNDERLINE_LONGDASH) ||
                      (eUnderline == UNDERLINE_BOLDDASH) ||
                      (eUnderline == UNDERLINE_BOLDLONGDASH) )
            {
                sal_Int32 nDashLength = 4*nLineHeight;
                sal_Int32 nVoidLength = 2*nLineHeight;
                switch( eUnderline )
                {
                    case UNDERLINE_LONGDASH:
                    case UNDERLINE_BOLDLONGDASH:
                        nDashLength = 8*nLineHeight;
                }

                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( " ] 0 d\r\n" );
            }
            else if ( (eUnderline == UNDERLINE_DASHDOT) ||
                      (eUnderline == UNDERLINE_BOLDDASHDOT) )
            {
                sal_Int32 nDashLength = 4*nLineHeight;
                sal_Int32 nVoidLength = 2*nLineHeight;
                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nLineHeight, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( " ] 0 d\r\n" );
            }
            else if ( (eUnderline == UNDERLINE_DASHDOTDOT) ||

                      (eUnderline == UNDERLINE_BOLDDASHDOTDOT) )
            {
                sal_Int32 nDashLength = 4*nLineHeight;
                sal_Int32 nVoidLength = 2*nLineHeight;
                aLine.append( "[ " );
                m_aPages.back().appendMappedLength( nDashLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nLineHeight, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nLineHeight, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( nVoidLength, aLine, false );
                aLine.append( " ] 0 d\r\n" );
            }

            aLine.append( "0 " );
            m_aPages.back().appendMappedLength( -nLinePos, aLine, true );
            aLine.append( " m " );
            m_aPages.back().appendMappedLength( nWidth, aLine, false );
            aLine.append( ' ' );
            m_aPages.back().appendMappedLength( -nLinePos, aLine, true );
            aLine.append( " l S\r\n" );
            if ( eUnderline == UNDERLINE_DOUBLE )
            {
                aLine.append( "0 " );
                m_aPages.back().appendMappedLength( -nLinePos2-nLineHeight, aLine, true );
                aLine.append( " m " );
                m_aPages.back().appendMappedLength( nWidth, aLine, false );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( -nLinePos2-nLineHeight, aLine, true );
                aLine.append( " l S\r\n" );
            }
        }

        if ( eStrikeout > STRIKEOUT_X )
            eStrikeout = STRIKEOUT_SINGLE;

        if ( eStrikeout == STRIKEOUT_SINGLE )
        {
            if ( !pFontEntry->maMetric.mnStrikeoutSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontEntry->maMetric.mnStrikeoutSize );
            nLinePos    = HCONV( pFontEntry->maMetric.mnStrikeoutOffset );
        }
        else if ( eStrikeout == STRIKEOUT_BOLD )
        {
            if ( !pFontEntry->maMetric.mnBStrikeoutSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontEntry->maMetric.mnBStrikeoutSize );
            nLinePos    = HCONV( pFontEntry->maMetric.mnBStrikeoutOffset );

        }
        else if ( eStrikeout == STRIKEOUT_DOUBLE )
        {
            if ( !pFontEntry->maMetric.mnDStrikeoutSize )
                m_pReferenceDevice->ImplInitTextLineSize();
            nLineHeight = HCONV( pFontEntry->maMetric.mnDStrikeoutSize );
            nLinePos    = HCONV( pFontEntry->maMetric.mnDStrikeoutOffset1 );
            nLinePos2   = HCONV( pFontEntry->maMetric.mnDStrikeoutOffset2 );
        }
        else
            nLineHeight = 0;

        if ( nLineHeight )
        {
            m_aPages.back().appendMappedLength( nLineHeight, aLine, true );
            aLine.append( " w " );
            appendStrokingColor( aStrikeoutColor, aLine );
            aLine.append( "\r\n" );

            aLine.append( "0 " );
            m_aPages.back().appendMappedLength( -nLinePos, aLine, true );
            aLine.append( " m " );
            m_aPages.back().appendMappedLength( nWidth, aLine, true );
            aLine.append( ' ' );
            m_aPages.back().appendMappedLength( -nLinePos, aLine, true );
            aLine.append( " l S\r\n" );

            if ( eStrikeout == STRIKEOUT_DOUBLE )
            {
                aLine.append( "0 " );
                m_aPages.back().appendMappedLength( -nLinePos2-nLineHeight, aLine, true );
                aLine.append( " m " );
                m_aPages.back().appendMappedLength( nWidth, aLine, true );
                aLine.append( ' ' );
                m_aPages.back().appendMappedLength( -nLinePos2-nLineHeight, aLine, true );
                aLine.append( " l S\r\n" );

            }
        }
    }
    aLine.append( "Q\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolygon( const Polygon& rPoly )
{
    MARK( "drawPolygon" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

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

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

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

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    if( m_eVersion < PDFWriter::PDF_1_4 )
    {
        drawPolyPolygon( rPolyPoly );
        return;
    }

    // create XObject
    m_aTransparentObjects.push_back( TransparencyEmit() );
    m_aTransparentObjects.back().m_aBoundRect   = rPolyPoly.GetBoundRect();
    // convert rectangle to default user space
    m_aPages.back().convertRect( m_aTransparentObjects.back().m_aBoundRect );
    m_aTransparentObjects.back().m_nObject      = createObject();
    m_aTransparentObjects.back().m_fAlpha       = (double)(100-nTransparentPercent) / 100.0;
    // create XObject's content stream
    m_aPages.back().appendPolyPolygon( rPolyPoly, m_aTransparentObjects.back().m_aContentStream );
    if( m_aCurrentPDFState.m_aLineColor != Color( COL_TRANSPARENT ) &&
        m_aCurrentPDFState.m_aFillColor != Color( COL_TRANSPARENT ) )
        m_aTransparentObjects.back().m_aContentStream.append( " B*\r\n" );
    else if( m_aCurrentPDFState.m_aLineColor != Color( COL_TRANSPARENT ) )
        m_aTransparentObjects.back().m_aContentStream.append( " S\r\n" );
    else
        m_aTransparentObjects.back().m_aContentStream.append( " f*\r\n" );

    OStringBuffer aLine( 80 );
    // insert XObject
    aLine.append( "/Tr" );
    aLine.append( m_aTransparentObjects.back().m_nObject );
    aLine.append( " Do\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawRectangle( const Rectangle& rRect )
{
    MARK( "drawRectangle" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

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

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

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

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

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

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

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

    int nPoints = rPoly.GetSize();
    if( nPoints < 2 )
        return;

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

    OStringBuffer aLine( 20 * nPoints );
    aLine.append( "q 0 w " );
    m_aPages.back().appendPolygon( rPoly, aLine, rPoly[0] == rPoly[nPoints-1] );
    aLine.append( "S Q\r\n" );

    writeBuffer( aLine.getStr(), aLine.getLength() );
}

void PDFWriterImpl::drawPolyLine( const Polygon& rPoly, const LineInfo& rInfo )
{
    MARK( "drawPolyLine with LineInfo" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) )
        return;

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

    if( aColor == Color( COL_TRANSPARENT ) )
        return;

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

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) && ! pColors )
        return;

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
            if( pColors[i] == Color( COL_TRANSPARENT ) )
                continue;

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

bool PDFWriterImpl::writeTransparentObject( TransparencyEmit& rObject )
{
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    OStringBuffer aProlog;
    sal_Int32 nStateObject = createObject();
    aProlog.append( "/EGS" );
    aProlog.append( nStateObject );
    aProlog.append( " gs\r\n" );

    OStringBuffer aLine( 512 );
    CHECK_RETURN( updateObject( rObject.m_nObject ) );
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\r\n"
                  "<< /Type /XObject\r\n"
                  "   /Subtype /Form\r\n"
                  "   /BBox [ " );
    appendDouble( ((double)rObject.m_aBoundRect.TopLeft().X())/10.0, aLine );
    aLine.append( ' ' );
    appendDouble( ((double)rObject.m_aBoundRect.TopLeft().Y())/10.0, aLine );
    aLine.append( ' ' );
    appendDouble( ((double)rObject.m_aBoundRect.BottomRight().X())/10.0, aLine );
    aLine.append( ' ' );
    appendDouble( ((double)rObject.m_aBoundRect.BottomRight().Y()+1)/10.0, aLine );
    aLine.append( " ]\r\n"
                  "   /Resources << /ExtGState << /EGS" );
    aLine.append( nStateObject );
    aLine.append( ' ' );
    aLine.append( nStateObject );
    aLine.append( " 0 R >> >>\r\n" );
    aLine.append( "   /Group << /S /Transparency /CS /DeviceRGB >>\r\n" );
    aLine.append( "   /Length " );
    aLine.append( (sal_Int32)(rObject.m_aContentStream.getLength()+aProlog.getLength()) );
    aLine.append( "\r\n"
                  ">>\r\n"
                  "stream\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    CHECK_RETURN( writeBuffer( aProlog.getStr(), aProlog.getLength() ) );
    CHECK_RETURN( writeBuffer( rObject.m_aContentStream.getStr(), rObject.m_aContentStream.getLength() ) );
    aLine.setLength( 0 );
    aLine.append( "endstream\r\n"
                  "endobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // write ExtGState dict for this XObject
    aLine.setLength( 0 );
    CHECK_RETURN( updateObject( nStateObject ) );
    aLine.append( nStateObject );
    aLine.append( " 0 obj\r\n"
                  "<< /CA " );
    appendDouble( rObject.m_fAlpha, aLine );
    aLine.append( "\r\n"
                  "   /ca " );
    appendDouble( rObject.m_fAlpha, aLine );
    aLine.append( "\r\n"
                  ">>\r\n"
                  "endobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    return true;
}

bool PDFWriterImpl::writeGradientFunction( GradientEmit& rObject )
{
    sal_Int32 nFunctionObject = createObject();
    CHECK_RETURN( updateObject( nFunctionObject ) );

    OutputDevice* pRefDevice = getReferenceDevice();
    pRefDevice->Push( PUSH_ALL );
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

    sal_Int32 nStreamLengthObject = createObject();
    OStringBuffer aLine( 120 );
    aLine.append( nFunctionObject );
    aLine.append( " 0 obj\r\n"
                  "<< /FunctionType 0\r\n"
                  "   /Domain [ 0 1 0 1 ]\r\n"
                  "   /Size [ " );
    aLine.append( (sal_Int32)aSize.Width() );
    aLine.append( ' ' );
    aLine.append( (sal_Int32)aSize.Height() );
    aLine.append( " ]\r\n"
                  "   /BitsPerSample 8\r\n"
                  "   /Range [ 0 1 0 1 0 1 ]\r\n"
                  "   /Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R\r\n"
#ifdef ENABLE_COMPRESSION
                  "   /Filter /FlateDecode\r\n"
#endif
                  ">>\r\n"
                  "stream\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    sal_uInt64 nStartStreamPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartStreamPos )) );

    beginCompression();
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
    endCompression();

    sal_uInt64 nEndStreamPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nEndStreamPos )) );

    aLine.setLength( 0 );
    aLine.append( "\r\nendstream\r\nendobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    // write stream length
    CHECK_RETURN( updateObject( nStreamLengthObject ) );
    aLine.setLength( 0 );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 obj\r\n" );
    aLine.append( (sal_Int64)(nEndStreamPos-nStartStreamPos) );
    aLine.append( "\r\nendobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    CHECK_RETURN( updateObject( rObject.m_nObject ) );
    aLine.setLength( 0 );
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\r\n"
                  "<< /ShadingType 1\r\n"
                  "   /ColorSpace /DeviceRGB\r\n"
                  "   /AntiAlias true\r\n"
                  "   /Domain [ 0 1 0 1 ]\r\n"
                  "   /Matrix [ " );
    aLine.append( (sal_Int32)aSize.Width() );
    aLine.append( " 0 0 " );
    aLine.append( (sal_Int32)aSize.Height() );
    aLine.append( " 0 0 ]\r\n"
                  "   /Function " );
    aLine.append( nFunctionObject );
    aLine.append( " 0 R\r\n"
                  ">>\r\n"
                  "endobj\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    pRefDevice->Pop();

    return true;
}

bool PDFWriterImpl::writeJPG( JPGEmit& rObject )
{
    CHECK_RETURN( rObject.m_pStream );
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    sal_Int32 nLength = 0;
    rObject.m_pStream->Seek( STREAM_SEEK_TO_END );
    nLength = rObject.m_pStream->Tell();
    rObject.m_pStream->Seek( STREAM_SEEK_TO_BEGIN );

    sal_Int32 nMaskObject = 0;
    if( !!rObject.m_aMask )
    {
        if( rObject.m_aMask.GetBitCount() == 1 ||
            ( rObject.m_aMask.GetBitCount() == 8 && m_eVersion >= PDFWriter::PDF_1_4 )
            )
            nMaskObject = createObject();
    }

    OStringBuffer aLine(80);
    aLine.append( rObject.m_nObject );
    aLine.append( " 0 obj\r\n"
                  "<< /Type /XObject\r\n"
                  "   /Subtype /Image\r\n"
                  "   /Width " );
    aLine.append( (sal_Int32)rObject.m_aID.m_aPixelSize.Width() );
    aLine.append( "\r\n"
                  "   /Height " );
    aLine.append( (sal_Int32)rObject.m_aID.m_aPixelSize.Height() );
    aLine.append( "\r\n"
                  "   /BitsPerComponent 8\r\n"
                  "   /ColorSpace /DeviceRGB\r\n"
                  "   /Filter /DCTDecode\r\n"
                  "   /Length "
                  );
    aLine.append( nLength );
    if( nMaskObject )
    {
        aLine.append( rObject.m_aMask.GetBitCount() == 1 ? "\r\n   /Mask " : "\r\n   /SMask " );
        aLine.append( nMaskObject );
        aLine.append( " 0 R" );
    }
    aLine.append( "\r\n>>\r\nstream\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    CHECK_RETURN( writeBuffer( rObject.m_pStream->GetData(), nLength ) );

    aLine.setLength( 0 );
    aLine.append( "\r\nendstream\r\n\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );

    if( nMaskObject )
    {
        BitmapEmit aEmit;
        aEmit.m_nObject = nMaskObject;
        if( rObject.m_aMask.GetBitCount() == 1 )
            aEmit.m_aBitmap = BitmapEx( rObject.m_aMask, rObject.m_aMask );
        else if( rObject.m_aMask.GetBitCount() == 8 )
            aEmit.m_aBitmap = BitmapEx( rObject.m_aMask, AlphaMask( rObject.m_aMask ) );
        writeBitmapObject( aEmit, true );
    }

    return true;
}

bool PDFWriterImpl::writeBitmapObject( BitmapEmit& rObject, bool bMask )
{
    CHECK_RETURN( updateObject( rObject.m_nObject ) );

    Bitmap  aBitmap;
    Color   aTransparentColor( COL_TRANSPARENT );
    bool    bWriteMask = false;
    if( ! bMask )
    {
        aBitmap = rObject.m_aBitmap.GetBitmap();
        if( rObject.m_aBitmap.IsAlpha() )
        {
            if( m_eVersion >= PDFWriter::PDF_1_4 )
                bWriteMask = true;
            // else draw without alpha channel
        }
        else
        {
            switch( rObject.m_aBitmap.GetTransparentType() )
            {
                case TRANSPARENT_NONE:
                    // comes from drawMask function
                    if( aBitmap.GetBitCount() == 1 && rObject.m_bDrawMask )
                        bMask = true;
                    break;
                case TRANSPARENT_COLOR:
                    aTransparentColor = rObject.m_aBitmap.GetTransparentColor();
                    break;
                case TRANSPARENT_BITMAP:
                    bWriteMask = true;
                    break;
            }
        }
    }
    else
    {
        if( m_eVersion < PDFWriter::PDF_1_4 || ! rObject.m_aBitmap.IsAlpha() )
        {
            aBitmap = rObject.m_aBitmap.GetMask();
            aBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
            DBG_ASSERT( aBitmap.GetBitCount() == 1, "mask conversion failed" );
        }
        else if( aBitmap.GetBitCount() != 8 )
        {
            aBitmap = rObject.m_aBitmap.GetAlpha().GetBitmap();
            aBitmap.Convert( BMP_CONVERSION_8BIT_GREYS );
            DBG_ASSERT( aBitmap.GetBitCount() == 8, "alpha mask conversion failed" );
        }
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
    aLine.append( " 0 obj\r\n"
                  "<< /Type /XObject\r\n"
                  "   /Subtype /Image\r\n"
                  "   /Width " );
    aLine.append( (sal_Int32)aBitmap.GetSizePixel().Width() );
    aLine.append( "\r\n"
                  "   /Height " );
    aLine.append( (sal_Int32)aBitmap.GetSizePixel().Height() );
    aLine.append( "\r\n"
                  "   /BitsPerComponent " );
    aLine.append( nBitsPerComponent );
    aLine.append( "\r\n"
                  "   /Length " );
    aLine.append( nStreamLengthObject );
    aLine.append( " 0 R\r\n" );
#ifdef ENABLE_COMPRESSION
    aLine.append( "   /Filter /FlateDecode\r\n" );
#endif
    if( ! bMask )
    {
        aLine.append( "   /ColorSpace " );
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
        if( aBitmap.GetBitCount() == 1 )
        {
            aLine.append( "   /ImageMask true\r\n" );
            sal_Int32 nBlackIndex = pAccess->GetBestPaletteIndex( BitmapColor( Color( COL_BLACK ) ) );
            DBG_ASSERT( nBlackIndex == 0 || nBlackIndex == 1, "wrong black index" );
            if( nBlackIndex )
                aLine.append( "   /Decode [ 1 0 ]\r\n" );
            else
                aLine.append( "   /Decode [ 0 1 ]\r\n" );
        }
        else if( aBitmap.GetBitCount() == 8 )
        {
            aLine.append( "   /ColorSpace /DeviceGray\r\n"
                          "   /Decode [ 1 0 ]\r\n" );
        }
    }

    if( ! bMask && m_eVersion > PDFWriter::PDF_1_2 )
    {
        if( bWriteMask )
        {
            nMaskObject = createObject();
            if( rObject.m_aBitmap.IsAlpha() && m_eVersion > PDFWriter::PDF_1_3 )
                aLine.append( "   /SMask " );
            else
                aLine.append( "   /Mask " );
            aLine.append( nMaskObject );
            aLine.append( " 0 R\r\n" );
        }
        else if( aTransparentColor != Color( COL_TRANSPARENT ) )
        {
            aLine.append( "  /Mask [ " );
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
    aLine.append( ">>\r\n"
                  "stream\r\n" );
    CHECK_RETURN( writeBuffer( aLine.getStr(), aLine.getLength() ) );
    sal_uInt64 nStartPos = 0;
    CHECK_RETURN( (osl_File_E_None == osl_getFilePos( m_aFile, &nStartPos )) );

    beginCompression();
    if( ! bTrueColor || pAccess->GetScanlineFormat() == BMP_FORMAT_24BIT_TC_RGB )
    {
        const int nScanLineBytes = 1 + ( pAccess->GetBitCount() * ( pAccess->Width() - 1 ) / 8U );

        for( int i = 0; i < pAccess->Height(); i++ )
        {
            CHECK_RETURN( writeBuffer( pAccess->GetScanline( i ), nScanLineBytes ) );
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
    endCompression();

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

void PDFWriterImpl::drawJPGBitmap( SvStream& rDCTData, const Size& rSizePixel, const Rectangle& rTargetArea, const Bitmap& rMask )
{
    MARK( "drawJPGBitmap" );

    OStringBuffer aLine( 80 );
    updateGraphicsState();

    SvMemoryStream* pStream = new SvMemoryStream;
    rDCTData.Seek( 0 );
    *pStream << rDCTData;
    pStream->Seek( STREAM_SEEK_TO_END );

    BitmapID aID;
    aID.m_aPixelSize    = rSizePixel;
    aID.m_nSize         = pStream->Tell();
    pStream->Seek( STREAM_SEEK_TO_BEGIN );
    aID.m_nChecksum     = rtl_crc32( 0, pStream->GetData(), aID.m_nSize );
    if( ! rMask.IsEmpty() )
        aID.m_nMaskChecksum = rMask.GetChecksum();

    std::list< JPGEmit >::const_iterator it;
    for( it = m_aJPGs.begin(); it != m_aJPGs.end() && ! (aID == it->m_aID); ++it )
        ;
    if( it == m_aJPGs.end() )
    {
        m_aJPGs.push_front( JPGEmit() );
        JPGEmit& rEmit = m_aJPGs.front();
        rEmit.m_nObject     = createObject();
        rEmit.m_aID         = aID;
        rEmit.m_pStream     = pStream;
        if( !! rMask && rMask.GetSizePixel() == rSizePixel )
            rEmit.m_aMask   = rMask;

        it = m_aJPGs.begin();
    }
    else
        delete pStream;

    aLine.append( "q " );
    m_aPages.back().appendMappedLength( rTargetArea.GetWidth(), aLine, false );
    aLine.append( " 0 0 " );
    m_aPages.back().appendMappedLength( rTargetArea.GetHeight(), aLine, true );
    aLine.append( ' ' );
    m_aPages.back().appendPoint( rTargetArea.BottomLeft(), aLine );
    aLine.append( " cm\r\n  /Im" );
    aLine.append( it->m_nObject );
    aLine.append( " Do Q\r\n" );
    writeBuffer( aLine.getStr(), aLine.getLength() );
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

const PDFWriterImpl::BitmapEmit& PDFWriterImpl::createBitmapEmit( const BitmapEx& rBitmap, bool bDrawMask )
{
    BitmapID aID;
    aID.m_aPixelSize        = rBitmap.GetSizePixel();
    aID.m_nSize             = rBitmap.GetBitCount();
    aID.m_nChecksum         = rBitmap.GetBitmap().GetChecksum();
    aID.m_nMaskChecksum     = 0;
    if( rBitmap.IsAlpha() )
        aID.m_nMaskChecksum = rBitmap.GetAlpha().GetChecksum();
    else
    {
        Bitmap aMask = rBitmap.GetMask();
        if( ! aMask.IsEmpty() )
            aID.m_nMaskChecksum = aMask.GetChecksum();
    }
    for( std::list< BitmapEmit >::const_iterator it = m_aBitmaps.begin(); it != m_aBitmaps.end(); ++it )
    {
        if( aID == it->m_aID )
            return *it;
    }
    m_aBitmaps.push_front( BitmapEmit() );
    m_aBitmaps.front().m_aID        = aID;
    m_aBitmaps.front().m_aBitmap    = rBitmap;
    m_aBitmaps.front().m_nObject    = createObject();
    m_aBitmaps.front().m_bDrawMask  = bDrawMask;

    return m_aBitmaps.front();
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap )
{
    MARK( "drawBitmap (Bitmap)" );

    const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( rBitmap ) );
    drawBitmap( rDestPoint, rDestSize, rEmit, Color( COL_TRANSPARENT ) );
}

void PDFWriterImpl::drawBitmap( const Point& rDestPoint, const Size& rDestSize, const BitmapEx& rBitmap )
{
    MARK( "drawBitmap (BitmapEx)" );

    const BitmapEmit& rEmit = createBitmapEmit( rBitmap );
    drawBitmap( rDestPoint, rDestSize, rEmit, Color( COL_TRANSPARENT ) );
}

void PDFWriterImpl::drawMask( const Point& rDestPoint, const Size& rDestSize, const Bitmap& rBitmap, const Color& rFillColor )
{
    MARK( "drawMask" );

    Bitmap aBitmap( rBitmap );
    if( aBitmap.GetBitCount() > 1 )
        aBitmap.Convert( BMP_CONVERSION_1BIT_THRESHOLD );
    DBG_ASSERT( aBitmap.GetBitCount() == 1, "mask conversion failed" );

    const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( aBitmap ), true );
    drawBitmap( rDestPoint, rDestSize, rEmit, rFillColor );
}

sal_Int32 PDFWriterImpl::createGradient( const Gradient& rGradient, const Size& rSize )
{
    Size aPtSize( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                               MapMode( MAP_POINT ),
                               getReferenceDevice(),
                               rSize ) );
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
        if( it->m_aHatch == rHatch && it->m_aMapMode == m_aCurrentPDFState.m_aMapMode )
            return it->m_nObject;
    }

    m_aHatches.push_back( HatchEmit() );
    m_aHatches.back().m_aHatch  = rHatch;
    m_aHatches.back().m_nObject = createObject();
    m_aHatches.back().m_aMapMode = m_aCurrentPDFState.m_aMapMode;
    return m_aHatches.back().m_nObject;
}

void PDFWriterImpl::drawHatch( const PolyPolygon& rPolyPoly, const Hatch& rHatch )
{
    MARK( "drawHatch" );

    updateGraphicsState();

    if( m_aGraphicsStack.front().m_aLineColor == Color( COL_TRANSPARENT ) &&
        m_aGraphicsStack.front().m_aFillColor == Color( COL_TRANSPARENT ) )
        return;

    sal_Int32 nHatch = createHatch( rHatch );

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
        aBmpSize = lcl_convert( aBitmap.GetPrefMapMode(),
                                getMapMode(),
                                getReferenceDevice(),
                                aBitmap.GetPrefSize() );
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
                const BitmapEmit& rEmit = createBitmapEmit( BitmapEx( aBitmap ) );

                // convert to page coordinates; this needs to be done here
                // since the emit does not know the page anymore
                Rectangle aConvertRect( aBmpPos, aBmpSize );
                m_aPages.back().convertRect( aConvertRect );

                // push the pattern
                m_aTilings.push_back( BitmapPatternEmit() );
                m_aTilings.back().m_nObject         = createObject();
                m_aTilings.back().m_nBitmapObject   = rEmit.m_nObject;
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

    /*  #103137# equality operator is not implemented
     *  const as API promises but may change Region
     *  from Polygon to rectangles. Arrrgghh !!!!
     */
    Region aLeft = m_aCurrentPDFState.m_aClipRegion;
    Region aRight = rNewClip;
    if( aLeft != aRight )
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
            // clip region is always stored in private PDF mapmode
            MapMode aNewMapMode = rNewState.m_aMapMode;
            rNewState.m_aMapMode = m_aMapMode;
            getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
            m_aCurrentPDFState.m_aMapMode = rNewState.m_aMapMode;

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

            rNewState.m_aMapMode = aNewMapMode;
            getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
            m_aCurrentPDFState.m_aMapMode = rNewState.m_aMapMode;
        }
    }

    if( m_aCurrentPDFState.m_aMapMode != rNewState.m_aMapMode )
    {
        getReferenceDevice()->SetMapMode( rNewState.m_aMapMode );
    }

    if( m_aCurrentPDFState.m_aFont != rNewState.m_aFont )
    {
        getReferenceDevice()->SetFont( rNewState.m_aFont );
        getReferenceDevice()->ImplNewFont();
    }

    if( m_aCurrentPDFState.m_nLayoutMode != rNewState.m_nLayoutMode )
    {
        getReferenceDevice()->SetLayoutMode( rNewState.m_nLayoutMode );
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

void PDFWriterImpl::push( sal_uInt16 nFlags )
{
    m_aGraphicsStack.push_front( m_aGraphicsStack.front() );
    m_aGraphicsStack.front().m_nFlags = nFlags;
}

void PDFWriterImpl::pop()
{
    GraphicsState aState = m_aGraphicsStack.front();
    m_aGraphicsStack.pop_front();

    // move those parameters back that were not pushed
    // in the first place
    if( ! (aState.m_nFlags & PUSH_LINECOLOR) )
        m_aGraphicsStack.front().m_aLineColor = aState.m_aLineColor;
    if( ! (aState.m_nFlags & PUSH_FILLCOLOR) )
        m_aGraphicsStack.front().m_aFillColor = aState.m_aFillColor;
    if( ! (aState.m_nFlags & PUSH_FONT) )
        m_aGraphicsStack.front().m_aFont = aState.m_aFont;
    if( ! (aState.m_nFlags & PUSH_MAPMODE) )
        setMapMode( aState.m_aMapMode );
    if( ! (aState.m_nFlags & PUSH_CLIPREGION) )
        m_aGraphicsStack.front().m_aClipRegion = aState.m_aClipRegion;
    if( ! (aState.m_nFlags & PUSH_TEXTLINECOLOR ) )
        m_aGraphicsStack.front().m_aTextLineColor = aState.m_aTextLineColor;
    if( ! (aState.m_nFlags & PUSH_TEXTALIGN ) )
        m_aGraphicsStack.front().m_aFont.SetAlign( aState.m_aFont.GetAlign() );
    if( ! (aState.m_nFlags & PUSH_TEXTFILLCOLOR) )
        m_aGraphicsStack.front().m_aFont.SetFillColor( aState.m_aFont.GetFillColor() );
    if( ! (aState.m_nFlags & PUSH_REFPOINT) )
    {
        // what ?
    }
}

void PDFWriterImpl::setMapMode( const MapMode& rMapMode )
{
    m_aGraphicsStack.front().m_aMapMode = rMapMode;
    getReferenceDevice()->SetMapMode( rMapMode );
    m_aCurrentPDFState.m_aMapMode = rMapMode;
}

void PDFWriterImpl::setClipRegion( const Region& rRegion )
{
    Region aRegion = getReferenceDevice()->LogicToPixel( rRegion, m_aGraphicsStack.front().m_aMapMode );
    aRegion = getReferenceDevice()->PixelToLogic( aRegion, m_aMapMode );
    m_aGraphicsStack.front().m_aClipRegion = aRegion;
}

void PDFWriterImpl::moveClipRegion( sal_Int32 nX, sal_Int32 nY )
{
    Point aPoint( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                               m_aMapMode,
                               getReferenceDevice(),
                               Point( nX, nY ) ) );
    aPoint -= lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                           m_aMapMode,
                           getReferenceDevice(),
                           Point() );
    m_aGraphicsStack.front().m_aClipRegion.Move( nX, nY );
}

bool PDFWriterImpl::intersectClipRegion( const Rectangle& rRect )
{
    Rectangle aRect( lcl_convert( m_aGraphicsStack.front().m_aMapMode,
                                  m_aMapMode,
                                  getReferenceDevice(),
                                  rRect ) );

    return m_aGraphicsStack.front().m_aClipRegion.Intersect( aRect );
}


bool PDFWriterImpl::intersectClipRegion( const Region& rRegion )
{
    Region aRegion = getReferenceDevice()->LogicToPixel( rRegion, m_aGraphicsStack.front().m_aMapMode );
    aRegion = getReferenceDevice()->PixelToLogic( aRegion, m_aMapMode );
    return m_aGraphicsStack.front().m_aClipRegion.Intersect( aRegion );
}
