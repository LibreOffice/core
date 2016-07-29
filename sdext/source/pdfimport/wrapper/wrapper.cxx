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

#include "contentsink.hxx"
#include "pdfparse.hxx"
#include "pdfihelper.hxx"
#include "wrapper.hxx"

#include "osl/file.h"
#include "osl/file.hxx"
#include "osl/thread.h"
#include "osl/process.h"
#include "osl/diagnose.h"
#include "rtl/bootstrap.hxx"
#include "rtl/ustring.hxx"
#include "rtl/ustrbuf.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/byteseq.hxx"

#include "cppuhelper/exc_hlp.hxx"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/uno/XComponentContext.hpp"
#include "com/sun/star/awt/FontDescriptor.hpp"
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

#include <vcl/metric.hxx>
#include <vcl/font.hxx>
#include <vcl/virdev.hxx>
#include <vcl/lazydelete.hxx>

#include <memory>
#include <unordered_map>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "rtl/bootstrap.h"

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
#include "hash.cxx"
#if defined _MSC_VER && defined __clang__
#pragma clang diagnostic pop
#endif

class Parser
{
    typedef std::unordered_map< sal_Int64,
                           FontAttributes > FontMapType;

    const uno::Reference<uno::XComponentContext> m_xContext;
    const ContentSinkSharedPtr                   m_pSink;
    const oslFileHandle                          m_pErr;
    OString                               m_aLine;
    FontMapType                                  m_aFontMap;
    sal_Int32                                    m_nNextToken;
    sal_Int32                                    m_nCharIndex;


    OString readNextToken();
    void           readInt32( sal_Int32& o_Value );
    sal_Int32      readInt32();
    void           readInt64( sal_Int64& o_Value );
    void           readDouble( double& o_Value );
    double         readDouble();
    void           readBinaryData( uno::Sequence<sal_Int8>& rBuf );

    uno::Reference<rendering::XPolyPolygon2D> readPath();

    void                 readChar();
    void                 readLineCap();
    void                 readLineDash();
    void                 readLineJoin();
    void                 readTransformation();
    rendering::ARGBColor readColor();
    static void          parseFontFamilyName( FontAttributes& aResult );
    void                 readFont();
    uno::Sequence<beans::PropertyValue> readImageImpl();

    void                 readImage();
    void                 readMask();
    void                 readLink();
    void                 readMaskedImage();
    void                 readSoftMaskedImage();
    static sal_Int32 parseFontCheckForString(const sal_Unicode* pCopy, sal_Int32 nCopyLen,
                                      const char* pAttrib, sal_Int32 nAttribLen,
                                      FontAttributes& rResult, bool bItalic, bool bBold);
    static sal_Int32 parseFontRemoveSuffix(const sal_Unicode* pCopy, sal_Int32 nCopyLen,
                              const char* pAttrib, sal_Int32 nAttribLen);

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
        m_nCharIndex(-1)
    {}

    void parseLine( const OString& rLine );
};


namespace
{

    /** Unescapes line-ending characters in input string. These
        characters are encoded as pairs of characters: '\\' 'n', resp.
        '\\' 'r'. This function converts them back to '\n', resp. '\r'.
      */
    OString lcl_unescapeLineFeeds(const OString& i_rStr)
    {
        const size_t nOrigLen(sal::static_int_cast<size_t>(i_rStr.getLength()));
        const sal_Char* const pOrig(i_rStr.getStr());
        std::unique_ptr<sal_Char[]> pBuffer(new sal_Char[nOrigLen + 1]);

        const sal_Char* pRead(pOrig);
        sal_Char* pWrite(pBuffer.get());
        const sal_Char* pCur(pOrig);
        while ((pCur = strchr(pCur, '\\')) != nullptr)
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

        OString aResult(pBuffer.get());
        return aResult;
    }

}


OString Parser::readNextToken()
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

uno::Reference<rendering::XPolyPolygon2D> Parser::readPath()
{
    const OString aSubPathMarker( "subpath" );

    if( readNextToken() != aSubPathMarker )
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
        OString aCurrToken( m_aLine.getToken(m_nNextToken,' ',nDummy) );

        while( m_nCharIndex != -1 && aCurrToken != aSubPathMarker )
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

    return static_cast<rendering::XLinePolyPolygon2D*>(
        new basegfx::unotools::UnoPolyPolygon(aResult));
}

void Parser::readChar()
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

    if (m_nCharIndex != -1)
        aChars = lcl_unescapeLineFeeds( m_aLine.copy( m_nCharIndex ) );

    // chars gobble up rest of line
    m_nCharIndex = -1;

    m_pSink->drawGlyphs(OStringToOUString(aChars, RTL_TEXTENCODING_UTF8),
        aRect, aUnoMatrix, fontSize);
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

sal_Int32 Parser::parseFontCheckForString(
    const sal_Unicode* pCopy, sal_Int32 nCopyLen,
    const char* pAttrib, sal_Int32 nAttribLen,
    FontAttributes& rResult, bool bItalic, bool bBold)
{
    if (nCopyLen < nAttribLen)
        return 0;
    for (sal_Int32 i = 0; i < nAttribLen; ++i)
        if (rtl::toAsciiLowerCase(pCopy[i]) != rtl::toAsciiLowerCase(pAttrib[i])
            && rtl::toAsciiUpperCase(pCopy[i]) != rtl::toAsciiLowerCase(pAttrib[i]))
            return 0;
    rResult.isItalic |= bItalic;
    rResult.isBold |= bBold;
    return nAttribLen;
}

sal_Int32 Parser::parseFontRemoveSuffix(
    const sal_Unicode* pCopy, sal_Int32 nCopyLen,
    const char* pAttrib, sal_Int32 nAttribLen)
{
    if (nCopyLen < nAttribLen)
        return 0;
    for (sal_Int32 i = 0; i < nAttribLen; ++i)
        if ( pCopy[nCopyLen - nAttribLen + i] != pAttrib[i] )
            return 0;
    return nAttribLen;
}

void Parser::parseFontFamilyName( FontAttributes& rResult )
{
    OUStringBuffer aNewFamilyName( rResult.familyName.getLength() );

    const sal_Unicode* pCopy = rResult.familyName.getStr();
    sal_Int32 nLen = rResult.familyName.getLength();
    // parse out truetype subsets (e.g. BAAAAA+Thorndale)
    if( nLen > 8 && pCopy[6] == '+' )
    {
        pCopy += 7;
        nLen -= 7;
    }

    // TODO: Looks like this block needs to be refactored
    while( nLen )
    {
        if (parseFontRemoveSuffix(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("PSMT")))
        {
            nLen -= RTL_CONSTASCII_LENGTH("PSMT");
        }
        else if (parseFontRemoveSuffix(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("MT")))
        {
            nLen -= RTL_CONSTASCII_LENGTH("MT");
        }

        if (parseFontCheckForString(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("Italic"), rResult, true, false))
        {
            sal_Int32 nAttribLen = RTL_CONSTASCII_LENGTH("Italic");
            nLen -= nAttribLen;
            pCopy += nAttribLen;
        }
        else if (parseFontCheckForString(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("-Bold"), rResult, false, true))
        {
            sal_Int32 nAttribLen = RTL_CONSTASCII_LENGTH("-Bold");
            nLen -= nAttribLen;
            pCopy += nAttribLen;
        }
        else if (parseFontCheckForString(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("Bold"), rResult, false, true))
        {
            sal_Int32 nAttribLen = RTL_CONSTASCII_LENGTH("Bold");
            nLen -= nAttribLen;
            pCopy += nAttribLen;
        }
        else if (parseFontCheckForString(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("-Roman"), rResult, false, false))
        {
            sal_Int32 nAttribLen = RTL_CONSTASCII_LENGTH("-Roman");
            nLen -= nAttribLen;
            pCopy += nAttribLen;
        }
        else if (parseFontCheckForString(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("-LightOblique"), rResult, true, false))
        {
            sal_Int32 nAttribLen = RTL_CONSTASCII_LENGTH("-LightOblique");
            nLen -= nAttribLen;
            pCopy += nAttribLen;
        }
        else if (parseFontCheckForString(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("-BoldOblique"), rResult, true, true))
        {
            sal_Int32 nAttribLen = RTL_CONSTASCII_LENGTH("-BoldOblique");
            nLen -= nAttribLen;
            pCopy += nAttribLen;
        }
        else if (parseFontCheckForString(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("-Light"), rResult, false, false))
        {
            sal_Int32 nAttribLen = RTL_CONSTASCII_LENGTH("-Light");
            nLen -= nAttribLen;
            pCopy += nAttribLen;
        }
        else if (parseFontCheckForString(pCopy, nLen, RTL_CONSTASCII_STRINGPARAM("-Reg"), rResult, false, false))
        {
            sal_Int32 nAttribLen = RTL_CONSTASCII_LENGTH("-Reg");
            nLen -= nAttribLen;
            pCopy += nAttribLen;
        }
        else if(nLen > 0)
        {
            if( *pCopy != '-' )
                aNewFamilyName.append( *pCopy );
            pCopy++;
            nLen--;
        }
    }
    rResult.familyName = aNewFamilyName.makeStringAndClear();
}

void Parser::readFont()
{
    OString aFontName;
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
    FontAttributes aResult( OStringToOUString( aFontName,
                                                    RTL_TEXTENCODING_UTF8 ),
                            nIsBold != 0,
                            nIsItalic != 0,
                            nIsUnderline != 0,
                            nSize,
                            1.0);

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
                    "com.sun.star.awt.FontIdentificator", aArgs, m_xContext ),
                uno::UNO_QUERY );
            if( xMat.is() )
            {
                uno::Any aRes( xMat->getMaterial() );
                if( aRes >>= aFD )
                {
                    if (!aFD.Name.isEmpty())
                    {
                        aResult.familyName = aFD.Name;
                        parseFontFamilyName(aResult);
                    }
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
            aResult.familyName  = "Arial";
            aResult.isUnderline = false;
        }

    }

    static vcl::DeleteOnDeinit< VclPtr<VirtualDevice> > vDev( new VclPtr<VirtualDevice> );
    if (!vDev.get()->get())
        (*vDev.get()) = VclPtr<VirtualDevice>::Create();

    vcl::Font font(aResult.familyName, Size(0, 1000));
    (*vDev.get())->SetFont(font);
    FontMetric metric((*vDev.get())->GetFontMetric());
    aResult.ascent = metric.GetAscent() / 1000.0;

    m_aFontMap[nFontID] = aResult;

    aResult.size = nSize;
    m_pSink->setFont(aResult);
}

uno::Sequence<beans::PropertyValue> Parser::readImageImpl()
{
    static const char aJpegMarker[] = "JPEG";
    static const char aPbmMarker[]  = "PBM";
    static const char aPpmMarker[]  = "PPM";
    static const char aPngMarker[]  = "PNG";
    static const char aJpegFile[]   = "DUMMY.JPEG";
    static const char aPbmFile[]    = "DUMMY.PBM";
    static const char aPpmFile[]    = "DUMMY.PPM";
    static const char aPngFile[]    = "DUMMY.PNG";

    OString aToken = readNextToken();
    const sal_Int32 nImageSize( readInt32() );

    OUString           aFileName;
    if( aToken == aPngMarker )
        aFileName = aPngFile;
    else if( aToken == aJpegMarker )
        aFileName = aJpegFile;
    else if( aToken == aPbmMarker )
        aFileName = aPbmFile;
    else
    {
        SAL_WARN_IF(aToken != aPpmMarker,"sdext.pdfimport","Invalid bitmap format");
        aFileName = aPpmFile;
    }

    uno::Sequence<sal_Int8> aDataSequence(nImageSize);
    readBinaryData( aDataSequence );

    uno::Sequence< uno::Any > aStreamCreationArgs(1);
    aStreamCreationArgs[0] <<= aDataSequence;

    uno::Reference< uno::XComponentContext > xContext( m_xContext, uno::UNO_SET_THROW );
    uno::Reference< lang::XMultiComponentFactory > xFactory( xContext->getServiceManager(), uno::UNO_SET_THROW );
    uno::Reference< io::XInputStream > xDataStream(
        xFactory->createInstanceWithArgumentsAndContext( "com.sun.star.io.SequenceInputStream", aStreamCreationArgs, m_xContext ),
        uno::UNO_QUERY_THROW );

    uno::Sequence<beans::PropertyValue> aSequence(3);
    aSequence[0] = beans::PropertyValue( OUString("URL"),
                                         0,
                                         uno::makeAny(aFileName),
                                         beans::PropertyState_DIRECT_VALUE );
    aSequence[1] = beans::PropertyValue( OUString("InputStream"),
                                         0,
                                         uno::makeAny( xDataStream ),
                                         beans::PropertyState_DIRECT_VALUE );
    aSequence[2] = beans::PropertyValue( OUString("InputSequence"),
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
                        OStringToOUString( lcl_unescapeLineFeeds(
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

void Parser::parseLine( const OString& rLine )
{
    OSL_PRECOND( m_pSink,         "Invalid sink" );
    OSL_PRECOND( m_pErr,          "Invalid filehandle" );
    OSL_PRECOND( m_xContext.is(), "Invalid service factory" );

    m_nNextToken = 0; m_nCharIndex = 0; m_aLine = rLine;
    uno::Reference<rendering::XPolyPolygon2D> xPoly;
    const OString& rCmd = readNextToken();
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
            m_pSink->eoFillPath(readPath()); break;
        case FILLPATH:
            m_pSink->fillPath(readPath()); break;
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
    SAL_WARN_IF(m_nCharIndex!=-1, "sdext.pdfimport", "leftover scanner input");
}

} // namespace

static bool checkEncryption( const OUString&                               i_rPath,
                             const uno::Reference< task::XInteractionHandler >& i_xIHdl,
                             OUString&                                     io_rPwd,
                             bool&                                              o_rIsEncrypted,
                             const OUString&                               i_rDocName
                             )
{
    bool bSuccess = false;
    OString aPDFFile;
    aPDFFile = OUStringToOString( i_rPath, osl_getThreadTextEncoding() );

    pdfparse::PDFReader aParser;
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

                        OSL_TRACE( "password: %s", bAuthenticated ? "matches" : "does not match" );
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

bool xpdf_ImportFromFile( const OUString&                             rURL,
                          const ContentSinkSharedPtr&                        rSink,
                          const uno::Reference< task::XInteractionHandler >& xIHdl,
                          const OUString&                               rPwd,
                          const uno::Reference< uno::XComponentContext >&    xContext,
                          const OUString&                                    rFilterOptions )
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
                while( osl_File_E_None == (nRes = aBuffering.read(&aChar, 1, &nBytesRead)) &&
                       nBytesRead == 1 &&
                       (aChar == '\n' || aChar == '\r') ) ;
                if ( osl_File_E_None != nRes )
                    break;

                if( aChar != '\n' && aChar != '\r' )
                    line.append( aChar );

                while( osl_File_E_None == (nRes = aBuffering.read(&aChar, 1, &nBytesRead)) &&
                       nBytesRead == 1 && aChar != '\n' && aChar != '\r' )
                {
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
