/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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


#include "contentsink.hxx"
#include "pdfparse.hxx"
#include "pdfihelper.hxx"

#include "osl/file.h"
#include "osl/thread.h"
#include "osl/process.h"
#include "osl/diagnose.h"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/byteseq.hxx"

#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/awt/FontDescriptor.hpp"
#include "com/sun/star/deployment/PackageInformationProvider.hpp"
#include "com/sun/star/beans/XMaterialHolder.hpp"
#include "com/sun/star/rendering/PathCapType.hpp"
#include "com/sun/star/rendering/PathJoinType.hpp"
#include "com/sun/star/rendering/XColorSpace.hpp"
#include "com/sun/star/rendering/XPolyPolygon2D.hpp"
#include "com/sun/star/rendering/XBitmap.hpp"
#include "com/sun/star/geometry/Matrix2D.hpp"
#include "com/sun/star/geometry/AffineMatrix2D.hpp"
#include "com/sun/star/geometry/RealRectangle2D.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"

#include "basegfx/point/b2dpoint.hxx"
#include "basegfx/polygon/b2dpolypolygon.hxx"
#include "basegfx/polygon/b2dpolygon.hxx"
#include "basegfx/tools/canvastools.hxx"
#include "basegfx/tools/unopolypolygon.hxx"

#include <boost/scoped_ptr.hpp>
#include <boost/unordered_map.hpp>
#include <string.h>
#ifdef WNT
#include <stdlib.h>
#include <ctype.h>
#endif

#include "rtl/bootstrap.h"

#include <string.h> // memcmp

#ifndef PDFI_IMPL_IDENTIFIER
# error define implementation name for pdfi extension, please!
#endif

using namespace com::sun::star;

namespace pdfi
{

namespace
{

// identifier of the strings coming from the out-of-process xpdf
// converter
enum parseKey {
    CLIPPATH,
    DRAWCHAR,
    DRAWIMAGE,
    DRAWLINK,
    DRAWMASK,
    DRAWMASKEDIMAGE,
    DRAWSOFTMASKEDIMAGE,
    ENDPAGE,
    ENDTEXTOBJECT,
    EOCLIPPATH,
    EOFILLPATH,
    FILLPATH,
    HYPERLINK,
    INTERSECTCLIP,
    INTERSECTEOCLIP,
    POPSTATE,
    PUSHSTATE,
    RESTORESTATE,
    SAVESTATE,
    SETBLENDMODE,
    SETFILLCOLOR,
    SETFONT,
    SETLINECAP,
    SETLINEDASH,
    SETLINEJOIN,
    SETLINEWIDTH,
    SETMITERLIMIT,
    SETPAGENUM,
    SETSTROKECOLOR,
    SETTEXTRENDERMODE,
    SETTRANSFORMATION,
    STARTPAGE,
    STROKEPATH,
    UPDATEBLENDMODE,
    UPDATECTM,
    UPDATEFILLCOLOR,
    UPDATEFILLOPACITY,
    UPDATEFLATNESS,
    UPDATEFONT,
    UPDATELINECAP,
    UPDATELINEDASH,
    UPDATELINEJOIN,
    UPDATELINEWIDTH,
    UPDATEMITERLIMIT,
    UPDATESTROKECOLOR,
    UPDATESTROKEOPACITY,
    NONE
};

#include "hash.cxx"

class Parser
{
    typedef boost::unordered_map< sal_Int64,
                           FontAttributes > FontMapType;

    const uno::Reference<uno::XComponentContext> m_xContext;
    const ContentSinkSharedPtr                   m_pSink;
    const oslFileHandle                          m_pErr;
    ::rtl::OString                               m_aLine;
    FontMapType                                  m_aFontMap;
    sal_Int32                                    m_nNextToken;
    sal_Int32                                    m_nCharIndex;

    const double                                 minAreaThreshold;
    const double                                 minLineWidth;

    ::rtl::OString readNextToken();
    void           readInt32( sal_Int32& o_Value );
    sal_Int32      readInt32();
    void           readInt64( sal_Int64& o_Value );
    void           readDouble( double& o_Value );
    double         readDouble();
    void           readBinaryData( uno::Sequence<sal_Int8>& rBuf );

    uno::Reference<rendering::XPolyPolygon2D> readPath( double* );

    void                 readChar();
    void                 readLineCap();
    void                 readLineDash();
    void                 readLineJoin();
    void                 readTransformation();
    rendering::ARGBColor readColor();
    void                 parseFontFamilyName( FontAttributes& aResult );
    void                 readFont();
    uno::Sequence<beans::PropertyValue> readImageImpl();

    void                 readImage();
    void                 readMask();
    void                 readLink();
    void                 readMaskedImage();
    void                 readSoftMaskedImage();
    int          parseFontCheckForString( const sal_Unicode* pCopy, const char* str, sal_Int32& nLen,
                    FontAttributes& aResult, bool bItalic, bool bBold);
    int          parseFontRemoveSuffix( const sal_Unicode* pCopy, const char* s, sal_Int32& nLen);


public:
    Parser( const ContentSinkSharedPtr&                   rSink,
            oslFileHandle                                 pErr,
            const uno::Reference<uno::XComponentContext>& xContext ) :
        m_xContext(xContext),
        m_pSink(rSink),
        m_pErr(pErr),
        m_aLine(),
        m_aFontMap(101),
        m_nNextToken(-1),
        m_nCharIndex(-1),
        minAreaThreshold( 300.0 ),
        minLineWidth( 12 )
    {}

    void parseLine( const ::rtl::OString& rLine );
};


namespace
{

    /** Unescapes line-ending characters in input string. These
        characters are encoded as pairs of characters: '\\' 'n', resp.
        '\\' 'r'. This function converts them back to '\n', resp. '\r'.
      */
    rtl::OString lcl_unescapeLineFeeds(const rtl::OString& i_rStr)
    {
        const size_t nOrigLen(sal::static_int_cast<size_t>(i_rStr.getLength()));
        const sal_Char* const pOrig(i_rStr.getStr());
        sal_Char* const pBuffer(new sal_Char[nOrigLen + 1]);

        const sal_Char* pRead(pOrig);
        sal_Char* pWrite(pBuffer);
        const sal_Char* pCur(pOrig);
        while ((pCur = strchr(pCur, '\\')) != 0)
        {
            const sal_Char cNext(pCur[1]);
            if (cNext == 'n' || cNext == 'r' || cNext == '\\')
            {
                const size_t nLen(pCur - pRead);
                strncpy(pWrite, pRead, nLen);
                pWrite += nLen;
                *pWrite = cNext == 'n' ? '\n' : (cNext == 'r' ? '\r' : '\\');
                ++pWrite;
                pCur = pRead = pCur + 2;
            }
            else
            {
                // Just continue on the next character. The current
                // block will be copied the next time it goes through the
                // 'if' branch.
                ++pCur;
            }
        }
        // maybe there are some data to copy yet
        if (sal::static_int_cast<size_t>(pRead - pOrig) < nOrigLen)
        {
            const size_t nLen(nOrigLen - (pRead - pOrig));
            strncpy(pWrite, pRead, nLen);
            pWrite += nLen;
        }
        *pWrite = '\0';

        rtl::OString aResult(pBuffer);
        delete[] pBuffer;
        return aResult;
    }

}


::rtl::OString Parser::readNextToken()
{
    OSL_PRECOND(m_nCharIndex!=-1,"insufficient input");
    return m_aLine.getToken(m_nNextToken,' ',m_nCharIndex);
}

void Parser::readInt32( sal_Int32& o_Value )
{
    o_Value = readNextToken().toInt32();
}

sal_Int32 Parser::readInt32()
{
    return readNextToken().toInt32();
}

void Parser::readInt64( sal_Int64& o_Value )
{
    o_Value = readNextToken().toInt64();
}

void Parser::readDouble( double& o_Value )
{
    o_Value = readNextToken().toDouble();
}

double Parser::readDouble()
{
    return readNextToken().toDouble();
}

void Parser::readBinaryData( uno::Sequence<sal_Int8>& rBuf )
{
    sal_Int32 nFileLen( rBuf.getLength() );
    sal_Int8*           pBuf( rBuf.getArray() );
    sal_uInt64          nBytesRead(0);
    oslFileError        nRes=osl_File_E_None;
    while( nFileLen &&
           osl_File_E_None == (nRes=osl_readFile( m_pErr, pBuf, nFileLen, &nBytesRead )) )
    {
        pBuf += nBytesRead;
        nFileLen -= sal::static_int_cast<sal_Int32>(nBytesRead);
    }

    OSL_PRECOND(nRes==osl_File_E_None, "inconsistent data");
}

uno::Reference<rendering::XPolyPolygon2D> Parser::readPath( double* pArea = NULL )
{
    const rtl::OString aSubPathMarker( "subpath" );

    if( 0 != readNextToken().compareTo( aSubPathMarker ) )
        OSL_PRECOND(false, "broken path");

    basegfx::B2DPolyPolygon aResult;
    while( m_nCharIndex != -1 )
    {
        basegfx::B2DPolygon aSubPath;

        sal_Int32 nClosedFlag;
        readInt32( nClosedFlag );
        aSubPath.setClosed( nClosedFlag != 0 );

        sal_Int32 nContiguousControlPoints(0);
        sal_Int32 nDummy=m_nCharIndex;
        rtl::OString aCurrToken( m_aLine.getToken(m_nNextToken,' ',nDummy) );

        while( m_nCharIndex != -1 && 0 != aCurrToken.compareTo(aSubPathMarker) )
        {
            sal_Int32 nCurveFlag;
            double    nX, nY;
            readDouble( nX );
            readDouble( nY );
            readInt32(  nCurveFlag );

            aSubPath.append(basegfx::B2DPoint(nX,nY));
            if( nCurveFlag )
            {
                ++nContiguousControlPoints;
            }
            else if( nContiguousControlPoints )
            {
                OSL_PRECOND(nContiguousControlPoints==2,"broken bezier path");

                // have two control points before us. the current one
                // is a normal point - thus, convert previous points
                // into bezier segment
                const sal_uInt32 nPoints( aSubPath.count() );
                const basegfx::B2DPoint aCtrlA( aSubPath.getB2DPoint(nPoints-3) );
                const basegfx::B2DPoint aCtrlB( aSubPath.getB2DPoint(nPoints-2) );
                const basegfx::B2DPoint aEnd( aSubPath.getB2DPoint(nPoints-1) );
                aSubPath.remove(nPoints-3, 3);
                aSubPath.appendBezierSegment(aCtrlA, aCtrlB, aEnd);

                nContiguousControlPoints=0;
            }

            // one token look-ahead (new subpath or more points?
            nDummy=m_nCharIndex;
            aCurrToken = m_aLine.getToken(m_nNextToken,' ',nDummy);
        }

        aResult.append( aSubPath );
        if( m_nCharIndex != -1 )
            readNextToken();
    }

    if( pArea )
    {
        basegfx::B2DRange aRange( aResult.getB2DRange() );
        if( aRange.getWidth() <= minLineWidth || aRange.getHeight() <= minLineWidth)
            *pArea = 0.0;
        else
            *pArea = aRange.getWidth() * aRange.getHeight();
    }

    return static_cast<rendering::XLinePolyPolygon2D*>(
        new basegfx::unotools::UnoPolyPolygon(aResult));
}

void Parser::readChar()
{
    geometry::Matrix2D aUnoMatrix;
    geometry::RealRectangle2D aRect;

    readDouble(aRect.X1);
    readDouble(aRect.Y1);
    readDouble(aRect.X2);
    readDouble(aRect.Y2);
    readDouble(aUnoMatrix.m00);
    readDouble(aUnoMatrix.m01);
    readDouble(aUnoMatrix.m10);
    readDouble(aUnoMatrix.m11);

    rtl::OString aChars = lcl_unescapeLineFeeds( m_aLine.copy( m_nCharIndex ) );

    // chars gobble up rest of line
    m_nCharIndex = -1;

    m_pSink->drawGlyphs( rtl::OStringToOUString( aChars,
                                                 RTL_TEXTENCODING_UTF8 ),
                         aRect, aUnoMatrix );
}

void Parser::readLineCap()
{
    sal_Int8 nCap(rendering::PathCapType::BUTT);
    switch( readInt32() )
    {
        default:
            // FALLTHROUGH intended
        case 0: nCap = rendering::PathCapType::BUTT; break;
        case 1: nCap = rendering::PathCapType::ROUND; break;
        case 2: nCap = rendering::PathCapType::SQUARE; break;
    }
    m_pSink->setLineCap(nCap);
}

void Parser::readLineDash()
{
    if( m_nCharIndex == -1 )
    {
        m_pSink->setLineDash( uno::Sequence<double>(), 0.0 );
        return;
    }

    const double nOffset(readDouble());
    const sal_Int32 nLen(readInt32());

    uno::Sequence<double> aDashArray(nLen);
    double* pArray=aDashArray.getArray();
    for( sal_Int32 i=0; i<nLen; ++i )
        *pArray++ = readDouble();

    m_pSink->setLineDash( aDashArray, nOffset );
}

void Parser::readLineJoin()
{
    sal_Int8 nJoin(rendering::PathJoinType::MITER);
    switch( readInt32() )
    {
        default:
            // FALLTHROUGH intended
        case 0: nJoin = rendering::PathJoinType::MITER; break;
        case 1: nJoin = rendering::PathJoinType::ROUND; break;
        case 2: nJoin = rendering::PathJoinType::BEVEL; break;
    }
    m_pSink->setLineJoin(nJoin);
}

void Parser::readTransformation()
{
    geometry::AffineMatrix2D aMat;
    readDouble(aMat.m00);
    readDouble(aMat.m10);
    readDouble(aMat.m01);
    readDouble(aMat.m11);
    readDouble(aMat.m02);
    readDouble(aMat.m12);
    m_pSink->setTransformation( aMat );
}

rendering::ARGBColor Parser::readColor()
{
    rendering::ARGBColor aRes;
    readDouble(aRes.Red);
    readDouble(aRes.Green);
    readDouble(aRes.Blue);
    readDouble(aRes.Alpha);
    return aRes;
}

int Parser::parseFontCheckForString( const sal_Unicode* pCopy, const char* s, sal_Int32& nLen,
        FontAttributes& aResult, bool bItalic, bool bBold)
{
    int l = strlen(s);
    if (nLen < l)
        return 0;
    for (int i = 0; i < l; i++)
        if (tolower(pCopy[i]) != s[i]
            && toupper(pCopy[i]) != s[i])
            return 0;
    aResult.isItalic = bItalic;
    aResult.isBold = bBold;
        nLen -= l;
        pCopy += l;
    return l;
}

int Parser::parseFontRemoveSuffix( const sal_Unicode* pCopy, const char* s, sal_Int32& nLen)
{
    int l = strlen(s);
    if (nLen < l)
        return 0;
    for (int i = 0; i < l; i++)
        if ( pCopy[nLen - l + i] != s[i] )
            return 0;
        nLen -= l;
    return l;
}

void Parser::parseFontFamilyName( FontAttributes& aResult )
{
    rtl::OUStringBuffer aNewFamilyName( aResult.familyName.getLength() );

    const sal_Unicode* pCopy = aResult.familyName.getStr();
    sal_Int32 nLen = aResult.familyName.getLength();
    // parse out truetype subsets (e.g. BAAAAA+Thorndale)
    if( nLen > 8 && pCopy[6] == sal_Unicode('+') )
    {
        pCopy += 7;
        nLen -= 7;
    }

    while( nLen )
    {
    if (parseFontRemoveSuffix( pCopy, "PSMT", nLen)) {}
    else if (parseFontRemoveSuffix( pCopy, "MT", nLen)) {}

    if (parseFontCheckForString( pCopy, "Italic", nLen, aResult, true, false)) {}
    else if (parseFontCheckForString( pCopy, "-Bold", nLen, aResult, false, true)) {}
    else if (parseFontCheckForString( pCopy, "Bold", nLen, aResult, false, true)) {}
    else if (parseFontCheckForString( pCopy, "-Roman", nLen, aResult, false, false)) {}
    else if (parseFontCheckForString( pCopy, "-LightOblique", nLen, aResult, true, false)) {}
    else if (parseFontCheckForString( pCopy, "-BoldOblique", nLen, aResult, true, true)) {}
    else if (parseFontCheckForString( pCopy, "-Light", nLen, aResult, false, false)) {}
    else if (parseFontCheckForString( pCopy, "-Reg", nLen, aResult, false, false)) {}
        else
        {
            if( *pCopy != '-' )
                aNewFamilyName.append( *pCopy );
            pCopy++;
            nLen--;
        }
    }
    aResult.familyName = aNewFamilyName.makeStringAndClear();
}

void Parser::readFont()
{
    ::rtl::OString aFontName;
    sal_Int64      nFontID;
    sal_Int32      nIsEmbedded, nIsBold, nIsItalic, nIsUnderline, nFileLen;
    double         nSize;

    readInt64(nFontID);
    readInt32(nIsEmbedded);
    readInt32(nIsBold);
    readInt32(nIsItalic);
    readInt32(nIsUnderline);
    readDouble(nSize);
    readInt32(nFileLen);

    nSize = nSize < 0.0 ? -nSize : nSize;
    aFontName = lcl_unescapeLineFeeds( m_aLine.copy( m_nCharIndex ) );

    // name gobbles up rest of line
    m_nCharIndex = -1;

    FontMapType::const_iterator pFont( m_aFontMap.find(nFontID) );
    if( pFont != m_aFontMap.end() )
    {
        OSL_PRECOND(nFileLen==0,"font data for known font");
        FontAttributes aRes(pFont->second);
        aRes.size = nSize;
        m_pSink->setFont( aRes );

        return;
    }

    // yet unknown font - get info and add to map
    FontAttributes aResult( rtl::OStringToOUString( aFontName,
                                                    RTL_TEXTENCODING_UTF8 ),
                            nIsBold != 0,
                            nIsItalic != 0,
                            nIsUnderline != 0,
                            false,
                            nSize );

    // extract textual attributes (bold, italic in the name, etc.)
    parseFontFamilyName(aResult);
    // need to read font file?
    if( nFileLen )
    {
        uno::Sequence<sal_Int8> aFontFile(nFileLen);
        readBinaryData( aFontFile );

        awt::FontDescriptor aFD;
        uno::Sequence< uno::Any > aArgs(1);
        aArgs[0] <<= aFontFile;

        try
        {
            uno::Reference< beans::XMaterialHolder > xMat(
                m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    rtl::OUString( "com.sun.star.awt.FontIdentificator"  ),
                    aArgs,
                    m_xContext ),
                uno::UNO_QUERY );
            if( xMat.is() )
            {
                uno::Any aRes( xMat->getMaterial() );
                if( aRes >>= aFD )
                {
                    aResult.familyName  = aFD.Name;
                parseFontFamilyName(aResult);
                    aResult.isBold      = (aFD.Weight > 100.0);
                    aResult.isItalic    = (aFD.Slant == awt::FontSlant_OBLIQUE ||
                                           aFD.Slant == awt::FontSlant_ITALIC );
                    aResult.isUnderline = false;
                    aResult.size        = 0;
                }
            }
        }
        catch( uno::Exception& )
        {
        }

        if( aResult.familyName.isEmpty() )
        {
            // last fallback
            aResult.familyName  = ::rtl::OUString( "Arial"  );
            aResult.isUnderline = false;
        }

    }
    m_aFontMap[nFontID] = aResult;

    aResult.size = nSize;
    m_pSink->setFont(aResult);
}

uno::Sequence<beans::PropertyValue> Parser::readImageImpl()
{
    static const rtl::OString aJpegMarker( "JPEG" );
    static const rtl::OString aPbmMarker( "PBM" );
    static const rtl::OString aPpmMarker( "PPM" );
    static const rtl::OString aPngMarker( "PNG" );
    static const rtl::OUString aJpegFile( "DUMMY.JPEG" );
    static const rtl::OUString aPbmFile( "DUMMY.PBM" );
    static const rtl::OUString aPpmFile( "DUMMY.PPM" );
    static const rtl::OUString aPngFile( "DUMMY.PNG" );

    rtl::OString aToken = readNextToken();
    const sal_Int32 nImageSize( readInt32() );

    rtl::OUString           aFileName;
    if( aToken.compareTo( aPngMarker ) == 0 )
        aFileName = aPngFile;
    else if( aToken.compareTo( aJpegMarker ) == 0 )
        aFileName = aJpegFile;
    else if( aToken.compareTo( aPbmMarker ) == 0 )
        aFileName = aPbmFile;
    else
    {
        OSL_PRECOND( aToken.compareTo( aPpmMarker ) == 0,
                     "Invalid bitmap format" );
        aFileName = aPpmFile;
    }

    uno::Sequence<sal_Int8> aDataSequence(nImageSize);
    readBinaryData( aDataSequence );

    uno::Sequence< uno::Any > aStreamCreationArgs(1);
    aStreamCreationArgs[0] <<= aDataSequence;

    uno::Reference< uno::XComponentContext > xContext( m_xContext, uno::UNO_SET_THROW );
    uno::Reference< lang::XMultiComponentFactory > xFactory( xContext->getServiceManager(), uno::UNO_SET_THROW );
    uno::Reference< io::XInputStream > xDataStream( xFactory->createInstanceWithArgumentsAndContext(
        ::rtl::OUString( "com.sun.star.io.SequenceInputStream"  ),
        aStreamCreationArgs, m_xContext ), uno::UNO_QUERY_THROW );

    uno::Sequence<beans::PropertyValue> aSequence(3);
    aSequence[0] = beans::PropertyValue( ::rtl::OUString("URL"),
                                         0,
                                         uno::makeAny(aFileName),
                                         beans::PropertyState_DIRECT_VALUE );
    aSequence[1] = beans::PropertyValue( ::rtl::OUString("InputStream"),
                                         0,
                                         uno::makeAny( xDataStream ),
                                         beans::PropertyState_DIRECT_VALUE );
    aSequence[2] = beans::PropertyValue( ::rtl::OUString("InputSequence"),
                                         0,
                                         uno::makeAny(aDataSequence),
                                         beans::PropertyState_DIRECT_VALUE );

    return aSequence;
}

void Parser::readImage()
{
    sal_Int32 nWidth, nHeight,nMaskColors;
    readInt32(nWidth);
    readInt32(nHeight);
    readInt32(nMaskColors);

    uno::Sequence<beans::PropertyValue> aImg( readImageImpl() );

    if( nMaskColors )
    {
        uno::Sequence<sal_Int8> aDataSequence(nMaskColors);
        readBinaryData( aDataSequence );

        uno::Sequence<uno::Any> aMaskRanges(2);

        uno::Sequence<double> aMinRange(nMaskColors/2);
        uno::Sequence<double> aMaxRange(nMaskColors/2);
        for( sal_Int32 i=0; i<nMaskColors/2; ++i )
        {
            aMinRange[i] = aDataSequence[i] / 255.0;
            aMaxRange[i] = aDataSequence[i+nMaskColors/2] / 255.0;
        }

        aMaskRanges[0] = uno::makeAny(aMinRange);
        aMaskRanges[1] = uno::makeAny(aMaxRange);

        m_pSink->drawColorMaskedImage( aImg, aMaskRanges );
    }
    else
        m_pSink->drawImage( aImg );
}

void Parser::readMask()
{
    sal_Int32 nWidth, nHeight, nInvert;
    readInt32(nWidth);
    readInt32(nHeight);
    readInt32(nInvert);

    m_pSink->drawMask( readImageImpl(), nInvert );
}

void Parser::readLink()
{
    geometry::RealRectangle2D aBounds;
    readDouble(aBounds.X1);
    readDouble(aBounds.Y1);
    readDouble(aBounds.X2);
    readDouble(aBounds.Y2);

    m_pSink->hyperLink( aBounds,
                        rtl::OStringToOUString( lcl_unescapeLineFeeds(
                                m_aLine.copy(m_nCharIndex) ),
                                RTL_TEXTENCODING_UTF8 ) );
    // name gobbles up rest of line
    m_nCharIndex = -1;
}

void Parser::readMaskedImage()
{
    sal_Int32 nWidth, nHeight, nMaskWidth, nMaskHeight, nMaskInvert;
    readInt32(nWidth);
    readInt32(nHeight);
    readInt32(nMaskWidth);
    readInt32(nMaskHeight);
    readInt32(nMaskInvert);

    const uno::Sequence<beans::PropertyValue> aImage( readImageImpl() );
    const uno::Sequence<beans::PropertyValue> aMask ( readImageImpl() );
    m_pSink->drawMaskedImage( aImage, aMask, nMaskInvert != 0 );
}

void Parser::readSoftMaskedImage()
{
    sal_Int32 nWidth, nHeight, nMaskWidth, nMaskHeight;
    readInt32(nWidth);
    readInt32(nHeight);
    readInt32(nMaskWidth);
    readInt32(nMaskHeight);

    const uno::Sequence<beans::PropertyValue> aImage( readImageImpl() );
    const uno::Sequence<beans::PropertyValue> aMask ( readImageImpl() );
    m_pSink->drawAlphaMaskedImage( aImage, aMask );
}

void Parser::parseLine( const ::rtl::OString& rLine )
{
    OSL_PRECOND( m_pSink,         "Invalid sink" );
    OSL_PRECOND( m_pErr,          "Invalid filehandle" );
    OSL_PRECOND( m_xContext.is(), "Invalid service factory" );

    m_nNextToken = 0; m_nCharIndex = 0; m_aLine = rLine;
    uno::Reference<rendering::XPolyPolygon2D> xPoly;
    const ::rtl::OString& rCmd = readNextToken();
    const hash_entry* pEntry = PdfKeywordHash::in_word_set( rCmd.getStr(),
                                                            rCmd.getLength() );
    OSL_ASSERT(pEntry);
    switch( pEntry->eKey )
    {
        case CLIPPATH:
            m_pSink->intersectClip(readPath()); break;
        case DRAWCHAR:
            readChar(); break;
        case DRAWIMAGE:
            readImage(); break;
        case DRAWLINK:
            readLink(); break;
        case DRAWMASK:
            readMask(); break;
        case DRAWMASKEDIMAGE:
            readMaskedImage(); break;
        case DRAWSOFTMASKEDIMAGE:
            readSoftMaskedImage(); break;
        case ENDPAGE:
            m_pSink->endPage(); break;
        case ENDTEXTOBJECT:
            m_pSink->endText(); break;
        case EOCLIPPATH:
            m_pSink->intersectEoClip(readPath()); break;
        case EOFILLPATH:
        {
            double area = 0.0;
            uno::Reference<rendering::XPolyPolygon2D> path = readPath( &area );
            m_pSink->eoFillPath(path);
            // if area is smaller than required, add borders.
            if(area < minAreaThreshold)
                m_pSink->strokePath(path);
        }
        break;
        case FILLPATH:
        {
            double area = 0.0;
            uno::Reference<rendering::XPolyPolygon2D> path = readPath( &area );
            m_pSink->fillPath(path);
            // if area is smaller than required, add borders.
            if(area < minAreaThreshold)
                m_pSink->strokePath(path);
        }
        break;
        case RESTORESTATE:
            m_pSink->popState(); break;
        case SAVESTATE:
            m_pSink->pushState(); break;
        case SETPAGENUM:
            m_pSink->setPageNum( readInt32() ); break;
        case STARTPAGE:
        {
            const double nWidth ( readDouble() );
            const double nHeight( readDouble() );
            m_pSink->startPage( geometry::RealSize2D( nWidth, nHeight ) );
            break;
        }
        case STROKEPATH:
            m_pSink->strokePath(readPath()); break;
        case UPDATECTM:
            readTransformation(); break;
        case UPDATEFILLCOLOR:
            m_pSink->setFillColor( readColor() ); break;
        case UPDATEFLATNESS:
            m_pSink->setFlatness( readDouble( ) ); break;
        case UPDATEFONT:
            readFont(); break;
        case UPDATELINECAP:
            readLineCap(); break;
        case UPDATELINEDASH:
            readLineDash(); break;
        case UPDATELINEJOIN:
            readLineJoin(); break;
        case UPDATELINEWIDTH:
            m_pSink->setLineWidth( readDouble() );break;
        case UPDATEMITERLIMIT:
            m_pSink->setMiterLimit( readDouble() ); break;
        case UPDATESTROKECOLOR:
            m_pSink->setStrokeColor( readColor() ); break;
        case UPDATESTROKEOPACITY:
            break;
        case SETTEXTRENDERMODE:
            m_pSink->setTextRenderMode( readInt32() ); break;

        case NONE:
        default:
            OSL_PRECOND(false,"Unknown input");
            break;
    }

    // all consumed?
    OSL_POSTCOND(m_nCharIndex==-1,"leftover scanner input");
}

oslFileError readLine( oslFileHandle pFile, ::rtl::OStringBuffer& line )
{
    OSL_PRECOND( line.getLength() == 0, "line buf not empty" );

    // TODO(P3): read larger chunks
    sal_Char aChar('\n');
    sal_uInt64 nBytesRead;
    oslFileError nRes;

    // skip garbage \r \n at start of line
    while( osl_File_E_None == (nRes=osl_readFile(pFile, &aChar, 1, &nBytesRead)) &&
           nBytesRead == 1 &&
           (aChar == '\n' || aChar == '\r') ) ;

    if( aChar != '\n' && aChar != '\r' )
        line.append( aChar );

    while( osl_File_E_None == (nRes=osl_readFile(pFile, &aChar, 1, &nBytesRead)) &&
           nBytesRead == 1 && aChar != '\n' && aChar != '\r' )
    {
        line.append( aChar );
    }

    return nRes;
}

} // namespace

static bool checkEncryption( const rtl::OUString&                               i_rPath,
                             const uno::Reference< task::XInteractionHandler >& i_xIHdl,
                             rtl::OUString&                                     io_rPwd,
                             bool&                                              o_rIsEncrypted,
                             const rtl::OUString&                               i_rDocName
                             )
{
    bool bSuccess = false;
    rtl::OString aPDFFile;
    aPDFFile = rtl::OUStringToOString( i_rPath, osl_getThreadTextEncoding() );

    pdfparse::PDFReader aParser;
    boost::scoped_ptr<pdfparse::PDFEntry> pEntry( aParser.read( aPDFFile.getStr() ));
    if( pEntry )
    {
        pdfparse::PDFFile* pPDFFile = dynamic_cast<pdfparse::PDFFile*>(pEntry.get());
        if( pPDFFile )
        {
            o_rIsEncrypted = pPDFFile->isEncrypted();
            if( o_rIsEncrypted )
            {
                if( pPDFFile->usesSupportedEncryptionFormat() )
                {
                    bool bAuthenticated = false;
                    if( !io_rPwd.isEmpty() )
                    {
                        rtl::OString aIsoPwd = rtl::OUStringToOString( io_rPwd,
                                                                       RTL_TEXTENCODING_ISO_8859_1 );
                        bAuthenticated = pPDFFile->setupDecryptionData( aIsoPwd.getStr() );
                    }
                    if( bAuthenticated )
                        bSuccess = true;
                    else
                    {
                        if( i_xIHdl.is() )
                        {
                            bool bEntered = false;
                            do
                            {
                                bEntered = getPassword( i_xIHdl, io_rPwd, ! bEntered, i_rDocName );
                                rtl::OString aIsoPwd = rtl::OUStringToOString( io_rPwd,
                                                                               RTL_TEXTENCODING_ISO_8859_1 );
                                bAuthenticated = pPDFFile->setupDecryptionData( aIsoPwd.getStr() );
                            } while( bEntered && ! bAuthenticated );
                        }

                        OSL_TRACE( "password: %s", bAuthenticated ? "matches" : "does not match" );
                        bSuccess = bAuthenticated;
                    }
                    if( bAuthenticated )
                    {
                        rtl::OUStringBuffer aBuf( 128 );
                        aBuf.appendAscii( "_OOO_pdfi_Credentials_" );
                        aBuf.append( pPDFFile->getDecryptionKey() );
                        io_rPwd = aBuf.makeStringAndClear();
                    }
                }
                else if( i_xIHdl.is() )
                {
                    reportUnsupportedEncryptionFormat( i_xIHdl );
                        //TODO: this should either be handled further down the
                        // call stack, or else information that this has already
                        // been handled should be passed down the call stack, so
                        // that SfxBaseModel::load does not show an additional
                        // "General Error" message box
                }
            }
            else
                bSuccess = true;
        }
    }
    return bSuccess;
}

bool xpdf_ImportFromFile( const ::rtl::OUString&                             rURL,
                          const ContentSinkSharedPtr&                        rSink,
                          const uno::Reference< task::XInteractionHandler >& xIHdl,
                          const rtl::OUString&                               rPwd,
                          const uno::Reference< uno::XComponentContext >&    xContext )
{
    OSL_ASSERT(rSink);

    ::rtl::OUString aSysUPath;
    if( osl_getSystemPathFromFileURL( rURL.pData, &aSysUPath.pData ) != osl_File_E_None )
        return false;
    rtl::OUString aDocName( rURL.copy( rURL.lastIndexOf( sal_Unicode('/') )+1 ) );

    // check for encryption, if necessary get password
    rtl::OUString aPwd( rPwd );
    bool bIsEncrypted = false;
    if( checkEncryption( aSysUPath, xIHdl, aPwd, bIsEncrypted, aDocName ) == false )
        return false;

    // Retrieve package location URL, xpdfimport executable is located there:
    OUString location(
        deployment::PackageInformationProvider::get(xContext)->
        getPackageLocation(PDFI_IMPL_IDENTIFIER));
    if (location.isEmpty()) {
        SAL_WARN(
            "sdext.pdfimport",
            "getPackageLocation(" PDFI_IMPL_IDENTIFIER ") failed");
        return false;
    }
    rtl::OUStringBuffer converterURL(location);
    if (!location.endsWith("/")) {
        converterURL.append('/');
    }
    converterURL.append("xpdfimport");

    // spawn separate process to keep LGPL/GPL code apart.
    // ---------------------------------------------------
    rtl_uString** ppEnv = NULL;
    sal_uInt32 nEnv = 0;

    #if defined UNX && ! defined MACOSX
    rtl::OUString aStr( "$URE_LIB_DIR"  );
    rtl_bootstrap_expandMacros( &aStr.pData );
    rtl::OUString aSysPath;
    osl_getSystemPathFromFileURL( aStr.pData, &aSysPath.pData );
    rtl::OUStringBuffer aEnvBuf( aStr.getLength() + 20 );
    aEnvBuf.appendAscii( "LD_LIBRARY_PATH=" );
    aEnvBuf.append( aSysPath );
    aStr = aEnvBuf.makeStringAndClear();
    ppEnv = &aStr.pData;
    nEnv = 1;
    #endif

    rtl_uString*  args[] = { aSysUPath.pData };
    sal_Int32 nArgs = 1;

    oslProcess    aProcess;
    oslFileHandle pIn  = NULL;
    oslFileHandle pOut = NULL;
    oslFileHandle pErr = NULL;
    const oslProcessError eErr =
        osl_executeProcess_WithRedirectedIO(converterURL.makeStringAndClear().pData,
                                            args,
                                            nArgs,
                                            osl_Process_SEARCHPATH|osl_Process_HIDDEN,
                                            osl_getCurrentSecurity(),
                                            0, ppEnv, nEnv,
                                            &aProcess, &pIn, &pOut, &pErr);

    bool bRet=true;
    try
    {
        if( eErr!=osl_Process_E_None )
            return false;

        if( pIn )
        {
            rtl::OStringBuffer aBuf(256);
            if( bIsEncrypted )
                aBuf.append( rtl::OUStringToOString( aPwd, RTL_TEXTENCODING_ISO_8859_1 ) );
            aBuf.append( '\n' );

            sal_uInt64 nWritten = 0;
            osl_writeFile( pIn, aBuf.getStr(), sal_uInt64(aBuf.getLength()), &nWritten );
        }

        if( pOut && pErr )
        {
            // read results of PDF parser. One line - one call to
            // OutputDev. stderr is used for alternate streams, like
            // embedded fonts and bitmaps
            Parser aParser(rSink,pErr,xContext);
            ::rtl::OStringBuffer line;
            while( osl_File_E_None == readLine(pOut, line) && line.getLength() )
                aParser.parseLine(line.makeStringAndClear());
        }
    }
    catch( uno::Exception& )
    {
        // crappy C file interface. need manual resource dealloc
        bRet = false;
    }

    if( pIn )
        osl_closeFile(pIn);
    if( pOut )
        osl_closeFile(pOut);
    if( pErr )
        osl_closeFile(pErr);
    osl_freeProcessHandle(aProcess);
    return bRet;
}


bool xpdf_ImportFromStream( const uno::Reference< io::XInputStream >&         xInput,
                            const ContentSinkSharedPtr&                       rSink,
                            const uno::Reference<task::XInteractionHandler >& xIHdl,
                            const rtl::OUString&                              rPwd,
                            const uno::Reference< uno::XComponentContext >&   xContext )
{
    OSL_ASSERT(xInput.is());
    OSL_ASSERT(rSink);

    // convert XInputStream to local temp file
    oslFileHandle aFile = NULL;
    rtl::OUString aURL;
    if( osl_createTempFile( NULL, &aFile, &aURL.pData ) != osl_File_E_None )
        return false;

    // copy content, buffered...
    const sal_uInt32 nBufSize = 4096;
    uno::Sequence<sal_Int8> aBuf( nBufSize );
    sal_uInt64 nBytes = 0;
    sal_uInt64 nWritten = 0;
    bool bSuccess = true;
    do
    {
        try
        {
            nBytes = xInput->readBytes( aBuf, nBufSize );
        }
        catch( com::sun::star::uno::Exception& )
        {
            osl_closeFile( aFile );
            throw;
        }
        if( nBytes > 0 )
        {
            osl_writeFile( aFile, aBuf.getConstArray(), nBytes, &nWritten );
            if( nWritten != nBytes )
            {
                bSuccess = false;
                break;
            }
        }
    }
    while( nBytes == nBufSize );

    osl_closeFile( aFile );

    if ( bSuccess )
        bSuccess = xpdf_ImportFromFile( aURL, rSink, xIHdl, rPwd, xContext );
    osl_removeFile( aURL.pData );

    return bSuccess;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
