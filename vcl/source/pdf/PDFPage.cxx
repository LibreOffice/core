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

#include <sal/types.h>
#include <sal/log.hxx>
#include <osl/file.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/string.hxx>
#include <tools/gen.hxx>
#include <tools/long.hxx>
#include <tools/poly.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/range/b2drange.hxx>
#include <basegfx/vector/b2enums.hxx>

#include <vcl/lineinfo.hxx>
#include <vcl/pdfwriter.hxx>

#include <pdf/Matrix3.hxx>
#include <pdf/pdfwriter_impl.hxx>

#include "pdfwriter_utils.hxx"

#include <vector>

using namespace::com::sun::star;

static bool g_bDebugDisableCompression = getenv("VCL_DEBUG_DISABLE_PDFCOMPRESSION");

namespace vcl
{

PDFPage::PDFPage( PDFWriterImpl* pWriter, double nPageWidth, double nPageHeight, PDFWriter::Orientation eOrientation )
        :
        m_pWriter( pWriter ),
        m_nPageWidth( nPageWidth ),
        m_nPageHeight( nPageHeight ),
        m_nUserUnit( 1 ),
        m_eOrientation( eOrientation ),
        m_nPageObject( 0 ),  // invalid object number
        m_nStreamLengthObject( 0 ),
        m_nBeginStreamPos( 0 ),
        m_eTransition( PDFWriter::PageTransition::Regular ),
        m_nTransTime( 0 )
{
    // object ref must be only ever updated in emit()
    m_nPageObject = m_pWriter->createObject();

    switch (m_pWriter->m_aContext.Version)
    {
        // 1.6 or later
        default:
            m_nUserUnit = std::ceil(std::max(nPageWidth, nPageHeight) / 14400.0);
            break;
        case PDFWriter::PDFVersion::PDF_1_4:
        case PDFWriter::PDFVersion::PDF_1_5:
        case PDFWriter::PDFVersion::PDF_A_1:
            break;
    }
}

void PDFPage::beginStream()
{
    if (g_bDebugDisableCompression)
    {
        m_pWriter->emitComment("PDFWriterImpl::PDFPage::beginStream, +");
    }
    m_aStreamObjects.push_back(m_pWriter->createObject());
    if( ! m_pWriter->updateObject( m_aStreamObjects.back() ) )
        return;

    m_nStreamLengthObject = m_pWriter->createObject();
    // write content stream header
    OStringBuffer aLine(
        OString::number(m_aStreamObjects.back())
        + " 0 obj\n<</Length "
        + OString::number( m_nStreamLengthObject )
        + " 0 R" );
    if (!g_bDebugDisableCompression)
        aLine.append( "/Filter/FlateDecode" );
    aLine.append( ">>\nstream\n" );
    if( ! m_pWriter->writeBuffer( aLine ) )
        return;
    if (osl::File::E_None != m_pWriter->m_aFile.getPos(m_nBeginStreamPos))
    {
        m_pWriter->m_aFile.close();
        m_pWriter->m_bOpen = false;
    }
    if (!g_bDebugDisableCompression)
        m_pWriter->beginCompression();
    m_pWriter->checkAndEnableStreamEncryption( m_aStreamObjects.back() );
}

void PDFPage::endStream()
{
    if (!g_bDebugDisableCompression)
        m_pWriter->endCompression();
    sal_uInt64 nEndStreamPos;
    if (osl::File::E_None != m_pWriter->m_aFile.getPos(nEndStreamPos))
    {
        m_pWriter->m_aFile.close();
        m_pWriter->m_bOpen = false;
        return;
    }
    m_pWriter->disableStreamEncryption();
    if( ! m_pWriter->writeBuffer( "\nendstream\nendobj\n\n" ) )
        return;
    // emit stream length object
    if( ! m_pWriter->updateObject( m_nStreamLengthObject ) )
        return;
    OString aLine =
        OString::number( m_nStreamLengthObject ) +
        " 0 obj\n"  +
        OString::number( static_cast<sal_Int64>(nEndStreamPos-m_nBeginStreamPos) ) +
        "\nendobj\n\n";
    m_pWriter->writeBuffer( aLine );
}

bool PDFPage::emit(sal_Int32 nParentObject )
{
    m_pWriter->MARK("PDFPage::emit");
    // emit page object
    if( ! m_pWriter->updateObject( m_nPageObject ) )
        return false;
    OStringBuffer aLine(
        OString::number(m_nPageObject)
        + " 0 obj\n"
          "<</Type/Page/Parent "
        + OString::number(nParentObject)
        + " 0 R"
        "/Resources "
        + OString::number(m_pWriter->getResourceDictObj())
        + " 0 R" );
    if( m_nPageWidth && m_nPageHeight )
    {
        aLine.append( "/MediaBox[0 0 "
            + OString::number(m_nPageWidth / m_nUserUnit)
            + " "
            + OString::number(m_nPageHeight / m_nUserUnit)
            + "]" );
        if (m_nUserUnit > 1)
        {
            aLine.append("\n/UserUnit " + OString::number(m_nUserUnit));
        }
    }
    switch( m_eOrientation )
    {
        case PDFWriter::Orientation::Portrait: aLine.append( "/Rotate 0\n" );break;
        case PDFWriter::Orientation::Inherit:  break;
    }
    int nAnnots = m_aAnnotations.size();
    if( nAnnots > 0 )
    {
        aLine.append( "/Annots[\n" );
        for( int i = 0; i < nAnnots; i++ )
        {
            aLine.append( OString::number(m_aAnnotations[i])
                + " 0 R" );
            aLine.append( ((i+1)%15) ? " " : "\n" );
        }
        aLine.append( "]\n" );
    }
    if (PDFWriter::PDFVersion::PDF_1_5 <= m_pWriter->m_aContext.Version)
    {
        // ISO 14289-1:2014, Clause: 7.18.3 requires it if there are annotations
        // but Adobe Acrobat Pro complains if it is ever missing so just
        // write it always.
        aLine.append( "/Tabs/S\n" );
    }
    if( !m_aMCIDParents.empty() )
    {
        OStringBuffer aStructParents( 1024 );
        aStructParents.append( "[ " );
        int nParents = m_aMCIDParents.size();
        for( int i = 0; i < nParents; i++ )
        {
            aStructParents.append( OString::number(m_aMCIDParents[i])
                + " 0 R" );
            aStructParents.append( ((i%10) == 9) ? "\n" : " " );
        }
        aStructParents.append( "]" );
        m_pWriter->m_aStructParentTree.push_back( aStructParents.makeStringAndClear() );

        aLine.append( "/StructParents "
            + OString::number( sal_Int32(m_pWriter->m_aStructParentTree.size()-1) )
            + "\n" );
    }
    if( m_eTransition != PDFWriter::PageTransition::Regular && m_nTransTime > 0 )
    {
        // transition duration
        aLine.append( "/Trans<</D " );
        appendDouble( static_cast<double>(m_nTransTime)/1000.0, aLine, 3 );
        aLine.append( "\n" );
        const char *pStyle = nullptr, *pDm = nullptr, *pM = nullptr, *pDi = nullptr;
        switch( m_eTransition )
        {
            case PDFWriter::PageTransition::SplitHorizontalInward:
                pStyle = "Split"; pDm = "H"; pM = "I"; break;
            case PDFWriter::PageTransition::SplitHorizontalOutward:
                pStyle = "Split"; pDm = "H"; pM = "O"; break;
            case PDFWriter::PageTransition::SplitVerticalInward:
                pStyle = "Split"; pDm = "V"; pM = "I"; break;
            case PDFWriter::PageTransition::SplitVerticalOutward:
                pStyle = "Split"; pDm = "V"; pM = "O"; break;
            case PDFWriter::PageTransition::BlindsHorizontal:
                pStyle = "Blinds"; pDm = "H"; break;
            case PDFWriter::PageTransition::BlindsVertical:
                pStyle = "Blinds"; pDm = "V"; break;
            case PDFWriter::PageTransition::BoxInward:
                pStyle = "Box"; pM = "I"; break;
            case PDFWriter::PageTransition::BoxOutward:
                pStyle = "Box"; pM = "O"; break;
            case PDFWriter::PageTransition::WipeLeftToRight:
                pStyle = "Wipe"; pDi = "0"; break;
            case PDFWriter::PageTransition::WipeBottomToTop:
                pStyle = "Wipe"; pDi = "90"; break;
            case PDFWriter::PageTransition::WipeRightToLeft:
                pStyle = "Wipe"; pDi = "180"; break;
            case PDFWriter::PageTransition::WipeTopToBottom:
                pStyle = "Wipe"; pDi = "270"; break;
            case PDFWriter::PageTransition::Dissolve:
                pStyle = "Dissolve"; break;
            case PDFWriter::PageTransition::Regular:
                break;
        }
        // transition style
        if( pStyle )
        {
            aLine.append( OString::Concat("/S/") + pStyle + "\n" );
        }
        if( pDm )
        {
            aLine.append( OString::Concat("/Dm/") + pDm + "\n" );
        }
        if( pM )
        {
            aLine.append( OString::Concat("/M/") + pM + "\n" );
        }
        if( pDi  )
        {
            aLine.append( OString::Concat("/Di ") + pDi + "\n" );
        }
        aLine.append( ">>\n" );
    }

    aLine.append( "/Contents" );
    unsigned int nStreamObjects = m_aStreamObjects.size();
    if( nStreamObjects > 1 )
        aLine.append( '[' );
    for(sal_Int32 i : m_aStreamObjects)
    {
        aLine.append( " " + OString::number( i ) + " 0 R" );
    }
    if( nStreamObjects > 1 )
        aLine.append( ']' );
    aLine.append( ">>\nendobj\n\n" );
    return m_pWriter->writeBuffer( aLine );
}

void PDFPage::appendPoint( const Point& rPoint, OStringBuffer& rBuffer ) const
{
    Point aPoint( convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                               m_pWriter->m_aMapMode,
                               m_pWriter,
                               rPoint ) );

    sal_Int32 nValue    = aPoint.X();

    appendFixedInt( nValue, rBuffer );

    rBuffer.append( ' ' );

    nValue      = pointToPixel(getHeight()) - aPoint.Y();

    appendFixedInt( nValue, rBuffer );
}

void PDFPage::appendPixelPoint( const basegfx::B2DPoint& rPoint, OStringBuffer& rBuffer ) const
{
    double fValue   = pixelToPoint(rPoint.getX());

    appendDouble( fValue, rBuffer, nLog10Divisor );
    rBuffer.append( ' ' );
    fValue      = getHeight() - pixelToPoint(rPoint.getY());
    appendDouble( fValue, rBuffer, nLog10Divisor );
}

void PDFPage::appendRect( const tools::Rectangle& rRect, OStringBuffer& rBuffer ) const
{
    appendPoint( rRect.BottomLeft() + Point( 0, 1 ), rBuffer );
    rBuffer.append( ' ' );
    appendMappedLength( static_cast<sal_Int32>(rRect.GetWidth()), rBuffer, false );
    rBuffer.append( ' ' );
    appendMappedLength( static_cast<sal_Int32>(rRect.GetHeight()), rBuffer );
    rBuffer.append( " re" );
}

void PDFPage::convertRect( tools::Rectangle& rRect ) const
{
    const Point aLL = convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter,
                             rRect.BottomLeft() + Point( 0, 1 )
                             );
    const Size aSize = convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                              m_pWriter->m_aMapMode,
                              m_pWriter,
                              rRect.GetSize() );
    rRect.SetLeft( aLL.X() );
    rRect.SetRight( aLL.X() + aSize.Width() );
    rRect.SetTop( pointToPixel(getHeight()) - aLL.Y() );
    rRect.SetBottom( rRect.Top() + aSize.Height() );
}

void PDFPage::appendPolygon( const tools::Polygon& rPoly, OStringBuffer& rBuffer, bool bClose ) const
{
    const sal_uInt16 nPoints = rPoly.GetSize();
    /*
     *  #108582# applications do weird things
     */
    sal_uInt32 nBufLen = rBuffer.getLength();
    if( nPoints <= 0 )
        return;

    const PolyFlags* pFlagArray = rPoly.GetConstFlagAry();
    appendPoint( rPoly[0], rBuffer );
    rBuffer.append( " m\n" );
    for( sal_uInt16 i = 1; i < nPoints; i++ )
    {
        if( pFlagArray && pFlagArray[i] == PolyFlags::Control && nPoints-i > 2 )
        {
            // bezier
            SAL_WARN_IF( pFlagArray[i+1] != PolyFlags::Control || pFlagArray[i+2] == PolyFlags::Control, "vcl.pdfwriter", "unexpected sequence of control points" );
            appendPoint( rPoly[i], rBuffer );
            rBuffer.append( " " );
            appendPoint( rPoly[i+1], rBuffer );
            rBuffer.append( " " );
            appendPoint( rPoly[i+2], rBuffer );
            rBuffer.append( " c" );
            i += 2; // add additionally consumed points
        }
        else
        {
            // line
            appendPoint( rPoly[i], rBuffer );
            rBuffer.append( " l" );
        }
        if( (rBuffer.getLength() - nBufLen) > 65 )
        {
            rBuffer.append( "\n" );
            nBufLen = rBuffer.getLength();
        }
        else
            rBuffer.append( " " );
    }
    if( bClose )
        rBuffer.append( "h\n" );
}

void PDFPage::appendPolygon( const basegfx::B2DPolygon& rPoly, OStringBuffer& rBuffer ) const
{
    basegfx::B2DPolygon aPoly( convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                                            m_pWriter->m_aMapMode,
                                            m_pWriter,
                                            rPoly ) );

    if( basegfx::utils::isRectangle( aPoly ) )
    {
        basegfx::B2DRange aRange( aPoly.getB2DRange() );
        basegfx::B2DPoint aBL( aRange.getMinX(), aRange.getMaxY() );
        appendPixelPoint( aBL, rBuffer );
        rBuffer.append( ' ' );
        appendMappedLength( aRange.getWidth(), rBuffer, false, nLog10Divisor );
        rBuffer.append( ' ' );
        appendMappedLength( aRange.getHeight(), rBuffer, true, nLog10Divisor );
        rBuffer.append( " re\n" );
        return;
    }
    sal_uInt32 nPoints = aPoly.count();
    if( nPoints <= 0 )
        return;

    sal_uInt32 nBufLen = rBuffer.getLength();
    basegfx::B2DPoint aLastPoint( aPoly.getB2DPoint( 0 ) );
    appendPixelPoint( aLastPoint, rBuffer );
    rBuffer.append( " m\n" );
    for( sal_uInt32 i = 1; i <= nPoints; i++ )
    {
        if( i != nPoints || aPoly.isClosed() )
        {
            sal_uInt32 nCurPoint  = i % nPoints;
            sal_uInt32 nLastPoint = i-1;
            basegfx::B2DPoint aPoint( aPoly.getB2DPoint( nCurPoint ) );
            if( aPoly.isNextControlPointUsed( nLastPoint ) &&
                aPoly.isPrevControlPointUsed( nCurPoint ) )
            {
                appendPixelPoint( aPoly.getNextControlPoint( nLastPoint ), rBuffer );
                rBuffer.append( ' ' );
                appendPixelPoint( aPoly.getPrevControlPoint( nCurPoint ), rBuffer );
                rBuffer.append( ' ' );
                appendPixelPoint( aPoint, rBuffer );
                rBuffer.append( " c" );
            }
            else if( aPoly.isNextControlPointUsed( nLastPoint ) )
            {
                appendPixelPoint( aPoly.getNextControlPoint( nLastPoint ), rBuffer );
                rBuffer.append( ' ' );
                appendPixelPoint( aPoint, rBuffer );
                rBuffer.append( " y" );
            }
            else if( aPoly.isPrevControlPointUsed( nCurPoint ) )
            {
                appendPixelPoint( aPoly.getPrevControlPoint( nCurPoint ), rBuffer );
                rBuffer.append( ' ' );
                appendPixelPoint( aPoint, rBuffer );
                rBuffer.append( " v" );
            }
            else
            {
                appendPixelPoint( aPoint, rBuffer );
                rBuffer.append( " l" );
            }
            if( (rBuffer.getLength() - nBufLen) > 65 )
            {
                rBuffer.append( "\n" );
                nBufLen = rBuffer.getLength();
            }
            else
                rBuffer.append( " " );
        }
    }
    rBuffer.append( "h\n" );
}

void PDFPage::appendPolyPolygon( const tools::PolyPolygon& rPolyPoly, OStringBuffer& rBuffer ) const
{
    sal_uInt16 nPolygons = rPolyPoly.Count();
    for( sal_uInt16 n = 0; n < nPolygons; n++ )
        appendPolygon( rPolyPoly[n], rBuffer );
}

void PDFPage::appendPolyPolygon( const basegfx::B2DPolyPolygon& rPolyPoly, OStringBuffer& rBuffer ) const
{
    for(auto const& rPolygon : rPolyPoly)
        appendPolygon( rPolygon, rBuffer );
}

void PDFPage::appendMappedLength( sal_Int32 nLength, OStringBuffer& rBuffer, bool bVertical, sal_Int32* pOutLength ) const
{
    sal_Int32 nValue = nLength;
    if ( nLength < 0 )
    {
        rBuffer.append( '-' );
        nValue = -nLength;
    }
    Size aSize( convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter,
                             Size( nValue, nValue ) ) );
    nValue = bVertical ? aSize.Height() : aSize.Width();
    if( pOutLength )
        *pOutLength = ((nLength < 0 ) ? -nValue : nValue);

    appendFixedInt( nValue, rBuffer );
}

void PDFPage::appendMappedLength( double fLength, OStringBuffer& rBuffer, bool bVertical, sal_Int32 nPrecision ) const
{
    Size aSize( convert( m_pWriter->m_aGraphicsStack.front().m_aMapMode,
                             m_pWriter->m_aMapMode,
                             m_pWriter,
                             Size( 1000, 1000 ) ) );
    fLength *= pixelToPoint(static_cast<double>(bVertical ? aSize.Height() : aSize.Width()) / 1000.0);
    appendDouble( fLength, rBuffer, nPrecision );
}

bool PDFPage::appendLineInfo( const LineInfo& rInfo, OStringBuffer& rBuffer ) const
{
    if(LineStyle::Dash == rInfo.GetStyle() && rInfo.GetDashLen() != rInfo.GetDotLen())
    {
        // dashed and non-degraded case, check for implementation limits of dash array
        // in PDF reader apps (e.g. acroread)
        if(2 * (rInfo.GetDashCount() + rInfo.GetDotCount()) > 10)
        {
            return false;
        }
    }

    if(basegfx::B2DLineJoin::NONE != rInfo.GetLineJoin())
    {
        // LineJoin used, ExtLineInfo required
        return false;
    }

    if(css::drawing::LineCap_BUTT != rInfo.GetLineCap())
    {
        // LineCap used, ExtLineInfo required
        return false;
    }

    if( rInfo.GetStyle() == LineStyle::Dash )
    {
        rBuffer.append( "[ " );
        if( rInfo.GetDashLen() == rInfo.GetDotLen() ) // degraded case
        {
            appendMappedLength( rInfo.GetDashLen(), rBuffer );
            rBuffer.append( ' ' );
            appendMappedLength( rInfo.GetDistance(), rBuffer );
            rBuffer.append( ' ' );
        }
        else
        {
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
        }
        rBuffer.append( "] 0 d\n" );
    }

    if( rInfo.GetWidth() > 1 )
    {
        appendMappedLength( rInfo.GetWidth(), rBuffer );
        rBuffer.append( " w\n" );
    }
    else if( rInfo.GetWidth() == 0 )
    {
        // "pixel" line
        appendDouble( 72.0/double(m_pWriter->GetDPIX()), rBuffer );
        rBuffer.append( " w\n" );
    }

    return true;
}

void PDFPage::appendWaveLine( sal_Int32 nWidth, sal_Int32 nY, sal_Int32 nDelta, OStringBuffer& rBuffer ) const
{
    if( nWidth <= 0 )
        return;
    if( nDelta < 1 )
        nDelta = 1;

    rBuffer.append( "0 " );
    appendMappedLength( nY, rBuffer );
    rBuffer.append( " m\n" );
    for( sal_Int32 n = 0; n < nWidth; )
    {
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nDelta+nY, rBuffer );
        rBuffer.append( ' ' );
        n += nDelta;
        appendMappedLength( n, rBuffer, false );
        rBuffer.append( ' ' );
        appendMappedLength( nY, rBuffer );
        rBuffer.append( " v " );
        if( n < nWidth )
        {
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY-nDelta, rBuffer );
            rBuffer.append( ' ' );
            n += nDelta;
            appendMappedLength( n, rBuffer, false );
            rBuffer.append( ' ' );
            appendMappedLength( nY, rBuffer );
            rBuffer.append( " v\n" );
        }
    }
    rBuffer.append( "S\n" );
}

void PDFPage::appendMatrix3(Matrix3 const & rMatrix, OStringBuffer& rBuffer)
{
    appendDouble(rMatrix.get(0), rBuffer);
    rBuffer.append(' ');
    appendDouble(rMatrix.get(1), rBuffer);
    rBuffer.append(' ');
    appendDouble(rMatrix.get(2), rBuffer);
    rBuffer.append(' ');
    appendDouble(rMatrix.get(3), rBuffer);
    rBuffer.append(' ');
    appendPoint(Point(tools::Long(rMatrix.get(4)), tools::Long(rMatrix.get(5))), rBuffer);
}

double PDFPage::getHeight() const
{
    double fRet = m_nPageHeight ? m_nPageHeight : 842; // default A4 height in inch/72, OK to use hardcoded value here?

    if (m_nUserUnit > 1)
    {
        fRet /= m_nUserUnit;
    }

    return fRet;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
