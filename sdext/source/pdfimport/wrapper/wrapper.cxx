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

#include <config_folders.h>

#include <contentsink.hxx>
#include <pdfparse.hxx>
#include <pdfihelper.hxx>
#include <wrapper.hxx>

#include <o3tl/string_view.hxx>
#include <osl/file.h>
#include <osl/file.hxx>
#include <osl/thread.h>
#include <osl/process.h>
#include <osl/diagnose.h>
#include <rtl/bootstrap.hxx>
#include <rtl/ustring.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/strbuf.hxx>
#include <sal/log.hxx>

#include <comphelper/propertysequence.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/awt/FontDescriptor.hpp>
#include <com/sun/star/beans/XMaterialHolder.hpp>
#include <com/sun/star/rendering/PathCapType.hpp>
#include <com/sun/star/rendering/PathJoinType.hpp>
#include <com/sun/star/rendering/XPolyPolygon2D.hpp>
#include <com/sun/star/geometry/Matrix2D.hpp>
#include <com/sun/star/geometry/AffineMatrix2D.hpp>
#include <com/sun/star/geometry/RealRectangle2D.hpp>
#include <com/sun/star/geometry/RealSize2D.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <tools/diagnose_ex.h>

#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/utils/unopolypolygon.hxx>

#include <vcl/metric.hxx>
#include <vcl/font.hxx>
#include <vcl/virdev.hxx>

#include <cstddef>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <string.h>
#include <stdlib.h>

#include <rtl/character.hxx>

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

#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-register"
#pragma clang diagnostic ignored "-Wextra-tokens"
#endif
#include <hash.cxx>
#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic pop
#endif

class Parser
{
    friend class LineParser;

    typedef std::unordered_map< sal_Int64,
                           FontAttributes > FontMapType;

    ScopedVclPtr<VirtualDevice> m_xDev;
    const uno::Reference<uno::XComponentContext> m_xContext;
    const ContentSinkSharedPtr                   m_pSink;
    const oslFileHandle                          m_pErr;
    FontMapType                                  m_aFontMap;

public:
    Parser( const ContentSinkSharedPtr&                   rSink,
            oslFileHandle                                 pErr,
            const uno::Reference<uno::XComponentContext>& xContext ) :
        m_xContext(xContext),
        m_pSink(rSink),
        m_pErr(pErr),
        m_aFontMap(101)
    {}

    void parseLine( const OString& rLine );
};

class LineParser {
    Parser  & m_parser;
    OString m_aLine;

    static void parseFontFamilyName( FontAttributes& aResult );
    void    readInt32( sal_Int32& o_Value );
    void    readInt64( sal_Int64& o_Value );
    void    readDouble( double& o_Value );
    void    readBinaryData( uno::Sequence<sal_Int8>& rBuf );

    uno::Sequence<beans::PropertyValue> readImageImpl();

public:
    std::size_t m_nCharIndex = 0;

    LineParser(Parser & parser, OString const & line): m_parser(parser), m_aLine(line) {}

    std::string_view readNextToken();
    sal_Int32      readInt32();
    double         readDouble();

    uno::Reference<rendering::XPolyPolygon2D> readPath();

    void                 readChar();
    void                 readLineCap();
    void                 readLineDash();
    void                 readLineJoin();
    void                 readTransformation();
    rendering::ARGBColor readColor();
    void                 readFont();

    void                 readImage();
    void                 readMask();
    void                 readLink();
    void                 readMaskedImage();
    void                 readSoftMaskedImage();
};

/** Unescapes line-ending characters in input string. These
    characters are encoded as pairs of characters: '\\' 'n', resp.
    '\\' 'r'. This function converts them back to '\n', resp. '\r'.
  */
OString lcl_unescapeLineFeeds(std::string_view i_rStr)
{
    const size_t nOrigLen(i_rStr.size());
    const char* const pOrig(i_rStr.data());
    std::unique_ptr<char[]> pBuffer(new char[nOrigLen + 1]);

    const char* pRead(pOrig);
    char* pWrite(pBuffer.get());
    const char* pCur(pOrig);
    while ((pCur = strchr(pCur, '\\')) != nullptr)
    {
        const char cNext(pCur[1]);
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

    OString aResult(pBuffer.get());
    return aResult;
}

std::string_view LineParser::readNextToken()
{
    if (m_nCharIndex == std::string_view::npos) {
        SAL_WARN("sdext.pdfimport", "insufficient input");
        return {};
    }
    return o3tl::getToken(m_aLine,' ',m_nCharIndex);
}

void LineParser::readInt32( sal_Int32& o_Value )
{
    std::string_view tok = readNextToken();
    sal_Int64 n = rtl_str_toInt64_WithLength(tok.data(), 10, tok.size());
    if (n < SAL_MIN_INT32 || n > SAL_MAX_INT32)
        n = 0;
    o_Value = n;
}

sal_Int32 LineParser::readInt32()
{
    std::string_view tok = readNextToken();
    sal_Int64 n =rtl_str_toInt64_WithLength(tok.data(), 10, tok.size());
    if (n < SAL_MIN_INT32 || n > SAL_MAX_INT32)
        n = 0;
    return n;
}

void LineParser::readInt64( sal_Int64& o_Value )
{
    std::string_view tok = readNextToken();
    o_Value = rtl_str_toInt64_WithLength(tok.data(), 10, tok.size());
}

void LineParser::readDouble( double& o_Value )
{
    std::string_view tok = readNextToken();
    o_Value = rtl_math_stringToDouble(tok.data(), tok.data() + tok.size(), '.', 0,
                                   nullptr, nullptr);
}

double LineParser::readDouble()
{
    std::string_view tok = readNextToken();
    return rtl_math_stringToDouble(tok.data(), tok.data() + tok.size(), '.', 0,
                                   nullptr, nullptr);
}

void LineParser::readBinaryData( uno::Sequence<sal_Int8>& rBuf )
{
    sal_Int32 nFileLen( rBuf.getLength() );
    sal_Int8*           pBuf( rBuf.getArray() );
    sal_uInt64          nBytesRead(0);
    oslFileError        nRes=osl_File_E_None;
    while( nFileLen )
    {
        nRes = osl_readFile( m_parser.m_pErr, pBuf, nFileLen, &nBytesRead );
        if (osl_File_E_None != nRes )
            break;
        pBuf += nBytesRead;
        nFileLen -= sal::static_int_cast<sal_Int32>(nBytesRead);
    }

    OSL_PRECOND(nRes==osl_File_E_None, "inconsistent data");
}

uno::Reference<rendering::XPolyPolygon2D> LineParser::readPath()
{
    static const std::string_view aSubPathMarker( "subpath" );

    if( readNextToken() != aSubPathMarker )
        OSL_PRECOND(false, "broken path");

    basegfx::B2DPolyPolygon aResult;
    while( m_nCharIndex != std::string_view::npos )
    {
        basegfx::B2DPolygon aSubPath;

        sal_Int32 nClosedFlag;
        readInt32( nClosedFlag );
        aSubPath.setClosed( nClosedFlag != 0 );

        sal_Int32 nContiguousControlPoints(0);

        while( m_nCharIndex != std::string_view::npos )
        {
            std::size_t nDummy=m_nCharIndex;
            if (o3tl::getToken(m_aLine,' ',nDummy) == aSubPathMarker) {
                break;
            }

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
        }

        aResult.append( aSubPath );
        if( m_nCharIndex != std::string_view::npos )
            readNextToken();
    }

    return static_cast<rendering::XLinePolyPolygon2D*>(
        new basegfx::unotools::UnoPolyPolygon(aResult));
}

void LineParser::readChar()
{
    double fontSize;
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
    readDouble(fontSize);

    OString aChars;

    if (m_nCharIndex != std::string_view::npos)
        aChars = lcl_unescapeLineFeeds( m_aLine.subView( m_nCharIndex ) );

    // chars gobble up rest of line
    m_nCharIndex = std::string_view::npos;

    m_parser.m_pSink->drawGlyphs(OStringToOUString(aChars, RTL_TEXTENCODING_UTF8),
        aRect, aUnoMatrix, fontSize);
}

void LineParser::readLineCap()
{
    sal_Int8 nCap(rendering::PathCapType::BUTT);
    switch( readInt32() )
    {
        default:
        case 0: nCap = rendering::PathCapType::BUTT; break;
        case 1: nCap = rendering::PathCapType::ROUND; break;
        case 2: nCap = rendering::PathCapType::SQUARE; break;
    }
    m_parser.m_pSink->setLineCap(nCap);
}

void LineParser::readLineDash()
{
    if( m_nCharIndex == std::string_view::npos )
    {
        m_parser.m_pSink->setLineDash( uno::Sequence<double>(), 0.0 );
        return;
    }

    const double nOffset(readDouble());
    const sal_Int32 nLen(readInt32());

    uno::Sequence<double> aDashArray(nLen);
    double* pArray=aDashArray.getArray();
    for( sal_Int32 i=0; i<nLen; ++i )
        *pArray++ = readDouble();

    m_parser.m_pSink->setLineDash( aDashArray, nOffset );
}

void LineParser::readLineJoin()
{
    sal_Int8 nJoin(rendering::PathJoinType::MITER);
    switch( readInt32() )
    {
        default:
        case 0: nJoin = rendering::PathJoinType::MITER; break;
        case 1: nJoin = rendering::PathJoinType::ROUND; break;
        case 2: nJoin = rendering::PathJoinType::BEVEL; break;
    }
    m_parser.m_pSink->setLineJoin(nJoin);
}

void LineParser::readTransformation()
{
    geometry::AffineMatrix2D aMat;
    readDouble(aMat.m00);
    readDouble(aMat.m10);
    readDouble(aMat.m01);
    readDouble(aMat.m11);
    readDouble(aMat.m02);
    readDouble(aMat.m12);
    m_parser.m_pSink->setTransformation( aMat );
}

rendering::ARGBColor LineParser::readColor()
{
    rendering::ARGBColor aRes;
    readDouble(aRes.Red);
    readDouble(aRes.Green);
    readDouble(aRes.Blue);
    readDouble(aRes.Alpha);
    return aRes;
}

/* Parse and convert the font family name (passed from xpdfimport) to correct font names
e.g. TimesNewRomanPSMT            -> TimesNewRoman
      TimesNewRomanPS-BoldMT       -> TimesNewRoman
      TimesNewRomanPS-BoldItalicMT -> TimesNewRoman
During the conversion, also apply the font features (bold italic etc) to the result.

TODO: Further convert the font names to real font names in the system rather than the PS names.
e.g., TimesNewRoman -> Times New Roman
*/
void LineParser::parseFontFamilyName( FontAttributes& rResult )
{
    SAL_INFO("sdext.pdfimport", "Processing " << rResult.familyName << " ---");
    rResult.familyName = rResult.familyName.trim();
    for (const OUString& fontAttributesSuffix: fontAttributesSuffixes)
    {
        if ( rResult.familyName.endsWith(fontAttributesSuffix) )
        {
            rResult.familyName = rResult.familyName.replaceAll(fontAttributesSuffix, "");
            SAL_INFO("sdext.pdfimport", rResult.familyName);
            if (fontAttributesSuffix == "Bold")
            {
                rResult.isBold = true;
            } else if ( (fontAttributesSuffix == "Italic") or (fontAttributesSuffix == "Oblique") )
            {
                rResult.isItalic = true;
            }
        }
    }
}

void LineParser::readFont()
{
    /*
    xpdf line is like (separated by space):
    updateFont <FontID> <isEmbedded> <isBold> <isItalic> <isUnderline> <TransformedFontSize> <nEmbedSize> <FontName>
    updateFont 14       1            0        0          0             1200.000000           23068        TimesNewRomanPSMT

    If nEmbedSize > 0, then a fontFile is followed as a stream.
    */

    OString        aFontName;
    sal_Int64      nFontID;
    sal_Int32      nIsEmbedded, nIsBold, nIsItalic, nIsUnderline, nFileLen;
    double         nSize;

    readInt64(nFontID);     // read FontID
    readInt32(nIsEmbedded); // read isEmbedded
    readInt32(nIsBold);     // read isBold
    readInt32(nIsItalic);   // read isItalic
    readInt32(nIsUnderline);// read isUnderline
    readDouble(nSize);      // read TransformedFontSize
    readInt32(nFileLen);    // read nEmbedSize

    nSize = nSize < 0.0 ? -nSize : nSize;
    // Read FontName. From the current position to the end (any white spaces will be included).
    aFontName = lcl_unescapeLineFeeds(m_aLine.subView(m_nCharIndex));

    // name gobbles up rest of line
    m_nCharIndex = std::string_view::npos;

    // Check if this font is already in our font map list.
    // If yes, update the font size and skip.
    Parser::FontMapType::const_iterator pFont( m_parser.m_aFontMap.find(nFontID) );
    if( pFont != m_parser.m_aFontMap.end() )
    {
        OSL_PRECOND(nFileLen==0,"font data for known font");
        FontAttributes aRes(pFont->second);
        aRes.size = nSize;
        m_parser.m_pSink->setFont( aRes );

        return;
    }

    // yet unknown font - get info and add to map
    FontAttributes aResult( OStringToOUString( aFontName, RTL_TEXTENCODING_UTF8 ),
                            nIsBold != 0,
                            nIsItalic != 0,
                            nIsUnderline != 0,
                            nSize,
                            1.0);

    /* The above font attributes (fontName, bold, italic) are based on
       xpdf line output and may not be reliable. To get correct attributes,
       we do the following:
    1. Read the embedded font file and determine the attributes based on the
       font file.
    2. If we failed to read the font file, or empty result is returned, then
       determine the font attributes from the font name.
    3. If all these attempts have failed, then use a fallback font.
    */
    if (nFileLen > 0)
    {
        uno::Sequence<sal_Int8> aFontFile(nFileLen);
        readBinaryData(aFontFile);  // Read fontFile.

        uno::Sequence<uno::Any> aArgs(1);
        awt::FontDescriptor aFontDescriptor;
        aArgs[0] <<= aFontFile;

        try
        {
            uno::Reference<beans::XMaterialHolder> xHolder(
                m_parser.m_xContext->getServiceManager()->createInstanceWithArgumentsAndContext(
                    "com.sun.star.awt.FontIdentificator", aArgs, m_parser.m_xContext),
                uno::UNO_QUERY);
            if (xHolder.is())
            {
                uno::Any aFontReadResult(xHolder->getMaterial());
                aFontReadResult >>= aFontDescriptor;
                if (!aFontDescriptor.Name.isEmpty())
                {
                    aResult.familyName = aFontDescriptor.Name;
                    // tdf#143959: there are cases when the family name returned by font descriptor
                    // is like "AAAAAA+TimesNewRoman,Bold". In this case, use the font name
                    // determined by parseFontFamilyName instead, but still determine the font
                    // attributes (bold italic etc) from the font descriptor.
                    if (aResult.familyName.getLength() > 7 and aResult.familyName.indexOf(u"+", 6) == 6)
                    {
                        aResult.familyName = aResult.familyName.copy(7, aResult.familyName.getLength() - 7);
                        parseFontFamilyName(aResult);
                    }
                    aResult.isBold = (aFontDescriptor.Weight > 100.0);
                    aResult.isItalic = (aFontDescriptor.Slant == awt::FontSlant_OBLIQUE ||
                                        aFontDescriptor.Slant == awt::FontSlant_ITALIC);
                } else
                {
                    SAL_WARN("sdext.pdfimport",
                        "Font detection from fontFile returned empty result.\
                        Guessing font info from font name.");
                    parseFontFamilyName(aResult);
                }
            } else
            {
                SAL_WARN("sdext.pdfimport",
                    "Failed to run FontIdentificator service.\
                    Guessing font info from font name.");
                parseFontFamilyName(aResult);
            }
        } catch (uno::Exception&)
        {
            TOOLS_WARN_EXCEPTION("sdext.pdfimport", "Exception when trying to read font file.");
            parseFontFamilyName(aResult);
        }
    } else
        parseFontFamilyName(aResult);

    // last fallback
    if (aResult.familyName.isEmpty())
    {
        SAL_WARN("sdext.pdfimport", "Failed to determine the font, using a fallback font Arial.");
        aResult.familyName = "Arial";
    }

    if (!m_parser.m_xDev)
        m_parser.m_xDev.disposeAndReset(VclPtr<VirtualDevice>::Create());

    vcl::Font font(aResult.familyName, Size(0, 1000));
    m_parser.m_xDev->SetFont(font);
    FontMetric metric(m_parser.m_xDev->GetFontMetric());
    aResult.ascent = metric.GetAscent() / 1000.0;

    m_parser.m_aFontMap[nFontID] = aResult;

    aResult.size = nSize;
    m_parser.m_pSink->setFont(aResult);
}

uno::Sequence<beans::PropertyValue> LineParser::readImageImpl()
{
    std::string_view aToken = readNextToken();
    const sal_Int32 nImageSize( readInt32() );

    OUString           aFileName;
    if( aToken == "PNG" )
        aFileName = "DUMMY.PNG";
    else if( aToken == "JPEG" )
        aFileName = "DUMMY.JPEG";
    else if( aToken == "PBM" )
        aFileName = "DUMMY.PBM";
    else
    {
        SAL_WARN_IF(aToken != "PPM","sdext.pdfimport","Invalid bitmap format");
        aFileName = "DUMMY.PPM";
    }

    uno::Sequence<sal_Int8> aDataSequence(nImageSize);
    readBinaryData( aDataSequence );

    uno::Sequence< uno::Any > aStreamCreationArgs(1);
    aStreamCreationArgs[0] <<= aDataSequence;

    uno::Reference< uno::XComponentContext > xContext( m_parser.m_xContext, uno::UNO_SET_THROW );
    uno::Reference< lang::XMultiComponentFactory > xFactory( xContext->getServiceManager(), uno::UNO_SET_THROW );
    uno::Reference< io::XInputStream > xDataStream(
        xFactory->createInstanceWithArgumentsAndContext( "com.sun.star.io.SequenceInputStream", aStreamCreationArgs, m_parser.m_xContext ),
        uno::UNO_QUERY_THROW );

    uno::Sequence<beans::PropertyValue> aSequence( comphelper::InitPropertySequence({
            { "URL", uno::makeAny(aFileName) },
            { "InputStream", uno::makeAny( xDataStream ) },
            { "InputSequence", uno::makeAny(aDataSequence) }
        }));

    return aSequence;
}

void LineParser::readImage()
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

        aMaskRanges[0] <<= aMinRange;
        aMaskRanges[1] <<= aMaxRange;

        m_parser.m_pSink->drawColorMaskedImage( aImg, aMaskRanges );
    }
    else
        m_parser.m_pSink->drawImage( aImg );
}

void LineParser::readMask()
{
    sal_Int32 nWidth, nHeight, nInvert;
    readInt32(nWidth);
    readInt32(nHeight);
    readInt32(nInvert);

    m_parser.m_pSink->drawMask( readImageImpl(), nInvert != 0);
}

void LineParser::readLink()
{
    geometry::RealRectangle2D aBounds;
    readDouble(aBounds.X1);
    readDouble(aBounds.Y1);
    readDouble(aBounds.X2);
    readDouble(aBounds.Y2);

    m_parser.m_pSink->hyperLink( aBounds,
                        OStringToOUString( lcl_unescapeLineFeeds(
                                m_aLine.subView(m_nCharIndex) ),
                                RTL_TEXTENCODING_UTF8 ) );
    // name gobbles up rest of line
    m_nCharIndex = std::string_view::npos;
}

void LineParser::readMaskedImage()
{
    sal_Int32 nWidth, nHeight, nMaskWidth, nMaskHeight, nMaskInvert;
    readInt32(nWidth);
    readInt32(nHeight);
    readInt32(nMaskWidth);
    readInt32(nMaskHeight);
    readInt32(nMaskInvert);

    const uno::Sequence<beans::PropertyValue> aImage( readImageImpl() );
    const uno::Sequence<beans::PropertyValue> aMask ( readImageImpl() );
    m_parser.m_pSink->drawMaskedImage( aImage, aMask, nMaskInvert != 0 );
}

void LineParser::readSoftMaskedImage()
{
    sal_Int32 nWidth, nHeight, nMaskWidth, nMaskHeight;
    readInt32(nWidth);
    readInt32(nHeight);
    readInt32(nMaskWidth);
    readInt32(nMaskHeight);

    const uno::Sequence<beans::PropertyValue> aImage( readImageImpl() );
    const uno::Sequence<beans::PropertyValue> aMask ( readImageImpl() );
    m_parser.m_pSink->drawAlphaMaskedImage( aImage, aMask );
}

void Parser::parseLine( const OString& rLine )
{
    OSL_PRECOND( m_pSink,         "Invalid sink" );
    OSL_PRECOND( m_pErr,          "Invalid filehandle" );
    OSL_PRECOND( m_xContext.is(), "Invalid service factory" );

    LineParser lp(*this, rLine);
    const std::string_view rCmd = lp.readNextToken();
    const hash_entry* pEntry = PdfKeywordHash::in_word_set( rCmd.data(),
                                                            rCmd.size() );
    OSL_ASSERT(pEntry);
    switch( pEntry->eKey )
    {
        case CLIPPATH:
            m_pSink->intersectClip(lp.readPath()); break;
        case DRAWCHAR:
            lp.readChar(); break;
        case DRAWIMAGE:
            lp.readImage(); break;
        case DRAWLINK:
            lp.readLink(); break;
        case DRAWMASK:
            lp.readMask(); break;
        case DRAWMASKEDIMAGE:
            lp.readMaskedImage(); break;
        case DRAWSOFTMASKEDIMAGE:
            lp.readSoftMaskedImage(); break;
        case ENDPAGE:
            m_pSink->endPage(); break;
        case ENDTEXTOBJECT:
            m_pSink->endText(); break;
        case EOCLIPPATH:
            m_pSink->intersectEoClip(lp.readPath()); break;
        case EOFILLPATH:
            m_pSink->eoFillPath(lp.readPath()); break;
        case FILLPATH:
            m_pSink->fillPath(lp.readPath()); break;
        case RESTORESTATE:
            m_pSink->popState(); break;
        case SAVESTATE:
            m_pSink->pushState(); break;
        case SETPAGENUM:
            m_pSink->setPageNum( lp.readInt32() ); break;
        case STARTPAGE:
        {
            const double nWidth ( lp.readDouble() );
            const double nHeight( lp.readDouble() );
            m_pSink->startPage( geometry::RealSize2D( nWidth, nHeight ) );
            break;
        }
        case STROKEPATH:
            m_pSink->strokePath(lp.readPath()); break;
        case UPDATECTM:
            lp.readTransformation(); break;
        case UPDATEFILLCOLOR:
            m_pSink->setFillColor( lp.readColor() ); break;
        case UPDATEFLATNESS:
            m_pSink->setFlatness( lp.readDouble( ) ); break;
        case UPDATEFONT:
            lp.readFont(); break;
        case UPDATELINECAP:
            lp.readLineCap(); break;
        case UPDATELINEDASH:
            lp.readLineDash(); break;
        case UPDATELINEJOIN:
            lp.readLineJoin(); break;
        case UPDATELINEWIDTH:
            m_pSink->setLineWidth( lp.readDouble() );break;
        case UPDATEMITERLIMIT:
            m_pSink->setMiterLimit( lp.readDouble() ); break;
        case UPDATESTROKECOLOR:
            m_pSink->setStrokeColor( lp.readColor() ); break;
        case UPDATESTROKEOPACITY:
            break;
        case SETTEXTRENDERMODE:
            m_pSink->setTextRenderMode( lp.readInt32() ); break;

        case NONE:
        default:
            OSL_PRECOND(false,"Unknown input");
            break;
    }

    // all consumed?
    SAL_WARN_IF(
        lp.m_nCharIndex!=std::string_view::npos, "sdext.pdfimport", "leftover scanner input");
}

} // namespace

static bool checkEncryption( std::u16string_view                           i_rPath,
                             const uno::Reference< task::XInteractionHandler >& i_xIHdl,
                             OUString&                                     io_rPwd,
                             bool&                                              o_rIsEncrypted,
                             const OUString&                               i_rDocName
                             )
{
    bool bSuccess = false;
    OString aPDFFile = OUStringToOString( i_rPath, osl_getThreadTextEncoding() );

    std::unique_ptr<pdfparse::PDFEntry> pEntry( pdfparse::PDFReader::read( aPDFFile.getStr() ));
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
                        OString aIsoPwd = OUStringToOString( io_rPwd,
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
                                OString aIsoPwd = OUStringToOString( io_rPwd,
                                                                               RTL_TEXTENCODING_ISO_8859_1 );
                                bAuthenticated = pPDFFile->setupDecryptionData( aIsoPwd.getStr() );
                            } while( bEntered && ! bAuthenticated );
                        }

                        bSuccess = bAuthenticated;
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

namespace {

class Buffering
{
    static const int SIZE = 64*1024;
    std::unique_ptr<char[]> aBuffer;
    oslFileHandle& pOut;
    size_t pos;
    sal_uInt64 left;

public:
    explicit Buffering(oslFileHandle& out) : aBuffer(new char[SIZE]), pOut(out), pos(0), left(0) {}

    oslFileError read(char *pChar, short count, sal_uInt64* pBytesRead)
    {
        oslFileError nRes = osl_File_E_None;
        sal_uInt64 nBytesRead = 0;
        while (count > 0)
        {
            if (left == 0)
            {
                nRes = osl_readFile(pOut, aBuffer.get(), SIZE, &left);
                if (nRes != osl_File_E_None || left == 0)
                {
                    *pBytesRead = nBytesRead;
                    return nRes;
                }
                pos = 0;
            }
            *pChar = aBuffer.get()[pos];
            --count;
            ++pos;
            --left;
            ++pChar;
            ++nBytesRead;
        }
        *pBytesRead = nBytesRead;
        return osl_File_E_None;
    }
};

}

bool xpdf_ImportFromFile(const OUString& rURL,
                         const ContentSinkSharedPtr& rSink,
                         const uno::Reference<task::XInteractionHandler>& xIHdl,
                         const OUString& rPwd,
                         const uno::Reference<uno::XComponentContext>& xContext,
                         const OUString& rFilterOptions)
{
    OSL_ASSERT(rSink);

    OUString aSysUPath;
    if( osl_getSystemPathFromFileURL( rURL.pData, &aSysUPath.pData ) != osl_File_E_None )
    {
        SAL_WARN(
            "sdext.pdfimport",
            "getSystemPathFromFileURL(" << rURL << ") failed");
        return false;
    }
    OUString aDocName( rURL.copy( rURL.lastIndexOf( '/' )+1 ) );

    // check for encryption, if necessary get password
    OUString aPwd( rPwd );
    bool bIsEncrypted = false;
    if( !checkEncryption( aSysUPath, xIHdl, aPwd, bIsEncrypted, aDocName ) )
    {
        SAL_INFO(
            "sdext.pdfimport",
            "checkEncryption(" << aSysUPath << ") failed");
        return false;
    }

    // Determine xpdfimport executable URL:
    OUString converterURL("$BRAND_BASE_DIR/" LIBO_BIN_FOLDER "/xpdfimport");
    rtl::Bootstrap::expandMacros(converterURL); //TODO: detect failure

    // Determine pathname of xpdfimport_err.pdf:
    OUString errPathname("$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/xpdfimport/xpdfimport_err.pdf");
    rtl::Bootstrap::expandMacros(errPathname); //TODO: detect failure
    if (osl::FileBase::getSystemPathFromFileURL(errPathname, errPathname)
        != osl::FileBase::E_None)
    {
        SAL_WARN(
            "sdext.pdfimport",
            "getSystemPathFromFileURL(" << errPathname << ") failed");
        return false;
    }

    // spawn separate process to keep LGPL/GPL code apart.

    OUString aOptFlag("-o");
    rtl_uString*  args[] = { aSysUPath.pData, errPathname.pData,
                             aOptFlag.pData, rFilterOptions.pData };
    sal_Int32 nArgs = rFilterOptions.isEmpty() ? 2 : 4;

    oslProcess    aProcess;
    oslFileHandle pIn  = nullptr;
    oslFileHandle pOut = nullptr;
    oslFileHandle pErr = nullptr;
    oslSecurity pSecurity = osl_getCurrentSecurity ();
    oslProcessError eErr =
        osl_executeProcess_WithRedirectedIO(converterURL.pData,
                                            args,
                                            nArgs,
                                            osl_Process_SEARCHPATH|osl_Process_HIDDEN,
                                            pSecurity,
                                            nullptr, nullptr, 0,
                                            &aProcess, &pIn, &pOut, &pErr);
    osl_freeSecurityHandle(pSecurity);

    bool bRet=true;
    try
    {
        if( eErr!=osl_Process_E_None )
        {
            SAL_WARN(
                "sdext.pdfimport",
                "executeProcess of " << converterURL << " failed with "
                    << +eErr);
            return false;
        }

        if( pIn )
        {
            OStringBuffer aBuf(256);
            if( bIsEncrypted )
                aBuf.append( OUStringToOString( aPwd, RTL_TEXTENCODING_ISO_8859_1 ) );
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
            Buffering aBuffering(pOut);
            OStringBuffer line;
            for( ;; )
            {
                char aChar('\n');
                sal_uInt64 nBytesRead;
                oslFileError nRes;

                // skip garbage \r \n at start of line
                for (;;)
                {
                    nRes = aBuffering.read(&aChar, 1, &nBytesRead);
                    if (osl_File_E_None != nRes || nBytesRead != 1 || (aChar != '\n' && aChar != '\r') )
                        break;
                }
                if ( osl_File_E_None != nRes )
                    break;

                if( aChar != '\n' && aChar != '\r' )
                    line.append( aChar );

                for (;;)
                {
                    nRes = aBuffering.read(&aChar, 1, &nBytesRead);
                    if ( osl_File_E_None != nRes || nBytesRead != 1 || aChar == '\n' || aChar == '\r' )
                        break;
                    line.append( aChar );
                }
                if ( osl_File_E_None != nRes )
                    break;
                if ( line.isEmpty() )
                    break;

                aParser.parseLine(line.makeStringAndClear());
            }
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
    eErr = osl_joinProcess(aProcess);
    if (eErr == osl_Process_E_None)
    {
        oslProcessInfo info;
        info.Size = sizeof info;
        eErr = osl_getProcessInfo(aProcess, osl_Process_EXITCODE, &info);
        if (eErr == osl_Process_E_None)
        {
            if (info.Code != 0)
            {
                SAL_WARN(
                    "sdext.pdfimport",
                    "getProcessInfo of " << converterURL
                        << " failed with exit code " << info.Code);
                bRet = false;
            }
        }
        else
        {
            SAL_WARN(
                "sdext.pdfimport",
                "getProcessInfo of " << converterURL << " failed with "
                    << +eErr);
            bRet = false;
        }
    }
    else
    {
        SAL_WARN(
            "sdext.pdfimport",
            "joinProcess of " << converterURL << " failed with " << +eErr);
        bRet = false;
    }
    osl_freeProcessHandle(aProcess);
    return bRet;
}


bool xpdf_ImportFromStream( const uno::Reference< io::XInputStream >&         xInput,
                            const ContentSinkSharedPtr&                       rSink,
                            const uno::Reference<task::XInteractionHandler >& xIHdl,
                            const OUString&                              rPwd,
                            const uno::Reference< uno::XComponentContext >&   xContext,
                            const OUString&                                   rFilterOptions )
{
    OSL_ASSERT(xInput.is());
    OSL_ASSERT(rSink);

    // convert XInputStream to local temp file
    oslFileHandle aFile = nullptr;
    OUString aURL;
    if( osl_createTempFile( nullptr, &aFile, &aURL.pData ) != osl_File_E_None )
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
        catch( css::uno::Exception& )
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
        bSuccess = xpdf_ImportFromFile( aURL, rSink, xIHdl, rPwd, xContext, rFilterOptions );
    osl_removeFile( aURL.pData );

    return bSuccess;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
