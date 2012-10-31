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

#include "pdfioutdev_gpl.hxx"
#include "pnghelper.hxx"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <vector>

#include <boost/shared_array.hpp>

#if defined __SUNPRO_CC
#pragma disable_warn
#elif defined _MSC_VER
#pragma warning(push, 1)
#endif

#if POPPLER_CHECK_VERSION(0, 21, 0)
#include "UTF.h"
#else
#include "UTF8.h"
#endif

#if defined __SUNPRO_CC
#pragma enable_warn
#elif defined _MSC_VER
#pragma warning(pop)
#endif

#ifdef WNT
# define snprintf _snprintf

#if defined GCC
#pragma GCC diagnostic warning "-Wformat"
#pragma GCC diagnostic warning "-Wformat-extra-args"
#endif
#endif

/* SYNC STREAMS
   ============

   We stream human-readble tokens to stdout, and binary data (fonts,
   bitmaps) to g_binary_out. Another process reads from those pipes, and
   there lies the rub: things can deadlock, if the two involved
   processes access the pipes in different order. At any point in
   time, both processes must access the same pipe. To ensure this,
   data must be flushed to the OS before writing to a different pipe,
   otherwise not-yet-written data will leave the reading process
   waiting on the wrong pipe.
 */

namespace pdfi
{

/// cut off very small numbers & clamp value to zero
inline double normalize( double val )
{
    return fabs(val) < 0.0000001 ? 0.0 : val;
}

namespace
{

/** Escapes line-ending characters (\n and \r) in input string.
  */
boost::shared_array<char> lcl_escapeLineFeeds(const char* const i_pStr)
{
    size_t nLength(strlen(i_pStr));
    char* pBuffer = new char[2*nLength+1];

    const char* pRead = i_pStr;
    char* pWrite = pBuffer;
    while( nLength-- )
    {
        if( *pRead == '\r' )
        {
            *pWrite++ = '\\';
            *pWrite++ = 'r';
        }
        else if( *pRead == '\n' )
        {
            *pWrite++ = '\\';
            *pWrite++ = 'n';
        }
        else if( *pRead == '\\' )
        {
            *pWrite++ = '\\';
            *pWrite++ = '\\';
        }
        else
            *pWrite++ = *pRead;
        pRead++;
    }
    *pWrite++ = 0;

    return boost::shared_array<char>(pBuffer);
}

}

/// for the temp char buffer the header gets snprintfed in
#define WRITE_BUFFER_SIZE 1024

/// for the initial std::vector capacity when copying stream from xpdf
#define WRITE_BUFFER_INITIAL_CAPACITY (1024*100)

void initBuf(OutputBuffer& io_rBuffer)
{
    io_rBuffer.reserve(WRITE_BUFFER_INITIAL_CAPACITY);
}

void writeBinaryBuffer( const OutputBuffer& rBuffer )
{
    // ---sync point--- see SYNC STREAMS above
    fflush(stdout);

    // put buffer to stderr
    if( !rBuffer.empty() )
        if( fwrite(&rBuffer[0], sizeof(char),
                   rBuffer.size(), g_binary_out) != (size_t)rBuffer.size() )
            exit(1); // error

    // ---sync point--- see SYNC STREAMS above
    fflush(g_binary_out);
}

void writeJpeg_( OutputBuffer& o_rOutputBuf, Stream* str, bool bWithLinefeed )
{
    // dump JPEG file as-is
#if POPPLER_CHECK_VERSION(0, 17, 3)
    str = str->getBaseStream();
#else
    str = ((DCTStream *)str)->getRawStream();
#endif
    str->reset();

    int c;
    o_rOutputBuf.clear();
    while((c=str->getChar()) != EOF)
        o_rOutputBuf.push_back(static_cast<char>(c));

    printf( " JPEG %d", (int)o_rOutputBuf.size() );
    if( bWithLinefeed )
        printf("\n");

    str->close();
}

void writePbm_(OutputBuffer& o_rOutputBuf, Stream* str, int width, int height, bool bWithLinefeed, bool bInvert )
{
    // write as PBM (char by char, to avoid stdlib lineend messing)
    o_rOutputBuf.clear();
    o_rOutputBuf.resize(WRITE_BUFFER_SIZE);
    o_rOutputBuf[0] = 'P';
    o_rOutputBuf[1] = '4';
    o_rOutputBuf[2] = 0x0A;
    char *pAsCharPtr = reinterpret_cast<char *>(&o_rOutputBuf[3]);
    int nOutLen = snprintf(pAsCharPtr, WRITE_BUFFER_SIZE-10, "%d %d", width, height);
    if( nOutLen < 0 )
        nOutLen = WRITE_BUFFER_SIZE-10;
    o_rOutputBuf[3+nOutLen]  =0x0A;
    o_rOutputBuf[3+nOutLen+1]=0;

    const int header_size = 3+nOutLen+1;
    const int size = height * ((width + 7) / 8);

    printf( " PBM %d", size + header_size );
    if( bWithLinefeed )
        printf("\n");

    // trim buffer to exact header length
    o_rOutputBuf.resize(header_size);

    // initialize stream
    str->reset();

    // copy the raw stream
    if( bInvert )
    {
        for( int i=0; i<size; ++i)
            o_rOutputBuf.push_back(static_cast<char>(str->getChar() ^ 0xff));
    }
    else
    {
        for( int i=0; i<size; ++i)
            o_rOutputBuf.push_back(static_cast<char>(str->getChar()));
    }

    str->close();
}

void writePpm_( OutputBuffer&     o_rOutputBuf,
                Stream*           str,
                int               width,
                int               height,
                GfxImageColorMap* colorMap,
                bool              bWithLinefeed )
{
    // write as PPM (char by char, to avoid stdlib lineend messing)
    o_rOutputBuf.clear();
    o_rOutputBuf.resize(WRITE_BUFFER_SIZE);
    o_rOutputBuf[0] = 'P';
    o_rOutputBuf[1] = '6';
    o_rOutputBuf[2] = '\n';
    char *pAsCharPtr = reinterpret_cast<char *>(&o_rOutputBuf[3]);
    int nOutLen = snprintf(pAsCharPtr, WRITE_BUFFER_SIZE-10, "%d %d", width, height);
    if( nOutLen < 0 )
        nOutLen = WRITE_BUFFER_SIZE-10;
    o_rOutputBuf[3+nOutLen]  ='\n';
    o_rOutputBuf[3+nOutLen+1]='2';
    o_rOutputBuf[3+nOutLen+2]='5';
    o_rOutputBuf[3+nOutLen+3]='5';
    o_rOutputBuf[3+nOutLen+4]='\n';
    o_rOutputBuf[3+nOutLen+5]=0;

    const int header_size = 3+nOutLen+5;
    const int size = width*height*3 + header_size;

    printf( " PPM %d", size );
    if( bWithLinefeed )
        printf("\n");

    // trim buffer to exact header size
    o_rOutputBuf.resize(header_size);

    // initialize stream
    Guchar *p;
    GfxRGB rgb;
    ImageStream* imgStr =
        new ImageStream(str,
                        width,
                        colorMap->getNumPixelComps(),
                        colorMap->getBits());
    imgStr->reset();

    for( int y=0; y<height; ++y)
    {
        p = imgStr->getLine();
        for( int x=0; x<width; ++x)
        {
            colorMap->getRGB(p, &rgb);
            o_rOutputBuf.push_back(colToByte(rgb.r));
            o_rOutputBuf.push_back(colToByte(rgb.g));
            o_rOutputBuf.push_back(colToByte(rgb.b));

            p +=colorMap->getNumPixelComps();
        }
    }

    delete imgStr;

}

// call this only for 1 bit image streams !
void writePng_( OutputBuffer&     o_rOutputBuf,
                Stream*           str,
                int               width,
                int               height,
                GfxRGB&           zeroColor,
                GfxRGB&           oneColor,
                bool              bIsMask,
                bool              bWithLinefeed )
{
    o_rOutputBuf.clear();

    // get png image
    PngHelper::createPng( o_rOutputBuf, str, width, height, zeroColor, oneColor, bIsMask );

    printf( " PNG %d", (int)o_rOutputBuf.size() );
    if( bWithLinefeed )
        printf("\n");
}

void writePng_( OutputBuffer& o_rOutputBuf,
                Stream* str,
                int width, int height, GfxImageColorMap* colorMap,
                Stream* maskStr,
                int maskWidth, int maskHeight, GfxImageColorMap* maskColorMap,
                bool bWithLinefeed )
{
    o_rOutputBuf.clear();

    // get png image
    PngHelper::createPng( o_rOutputBuf, str, width, height, colorMap, maskStr, maskWidth, maskHeight, maskColorMap );

    printf( " PNG %d", (int)o_rOutputBuf.size() );
    if( bWithLinefeed )
        printf("\n");
}

void writePng_( OutputBuffer& o_rOutputBuf,
                Stream* str,
                int width, int height, GfxImageColorMap* colorMap,
                Stream* maskStr,
                int maskWidth, int maskHeight, bool maskInvert,
                bool bWithLinefeed )
{
    o_rOutputBuf.clear();

    // get png image
    PngHelper::createPng( o_rOutputBuf, str, width, height, colorMap, maskStr, maskWidth, maskHeight, maskInvert );

    printf( " PNG %d", (int)o_rOutputBuf.size() );
    if( bWithLinefeed )
        printf("\n");
}

// stolen from ImageOutputDev.cc
void writeMask_( OutputBuffer& o_rOutputBuf, Stream* str, int width, int height, bool bWithLinefeed, bool bInvert )
{
    if( str->getKind() == strDCT )
        writeJpeg_(o_rOutputBuf, str, bWithLinefeed);
    else
        writePbm_(o_rOutputBuf, str, width, height, bWithLinefeed, bInvert );
}

void writeImage_( OutputBuffer&     o_rOutputBuf,
                  Stream*           str,
                  int               width,
                  int               height,
                  GfxImageColorMap* colorMap,
                  bool              bWithLinefeed )
{
    // dump JPEG file
    if( str->getKind() == strDCT &&
        (colorMap->getNumPixelComps() == 1 ||
         colorMap->getNumPixelComps() == 3) )
    {
        writeJpeg_(o_rOutputBuf, str, bWithLinefeed);
    }
    else if (colorMap->getNumPixelComps() == 1 &&
             colorMap->getBits() == 1)
    {
        // this is a two color bitmap, write a png
        // provide default colors
        GfxRGB zeroColor = { 0, 0, 0 },
                oneColor = { byteToCol( 0xff ), byteToCol( 0xff ), byteToCol( 0xff ) };
        if( colorMap->getColorSpace()->getMode() == csIndexed || colorMap->getColorSpace()->getMode() == csDeviceGray )
        {
            Guchar nIndex = 0;
            colorMap->getRGB( &nIndex, &zeroColor );
            nIndex = 1;
            colorMap->getRGB( &nIndex, &oneColor );
        }
        writePng_( o_rOutputBuf, str, width, height, zeroColor, oneColor, false, bWithLinefeed );
    }
    else
        writePpm_( o_rOutputBuf, str, width, height, colorMap, bWithLinefeed );
}

// forwarders
// ------------------------------------------------------------------

inline void writeImage( OutputBuffer&     o_rOutputBuf,
                        Stream*           str,
                        int               width,
                        int               height,
                        GfxImageColorMap* colorMap ) { writeImage_(o_rOutputBuf,str,width,height,colorMap,false); }
inline void writeImageLF( OutputBuffer&     o_rOutputBuf,
                          Stream*           str,
                          int               width,
                          int               height,
                          GfxImageColorMap* colorMap ) { writeImage_(o_rOutputBuf,str,width,height,colorMap,true); }
inline void writeMask( OutputBuffer&     o_rOutputBuf,
                       Stream*           str,
                       int               width,
                       int               height,
                       bool              bInvert ) { writeMask_(o_rOutputBuf,str,width,height,false,bInvert); }
inline void writeMaskLF( OutputBuffer&     o_rOutputBuf,
                         Stream*           str,
                         int               width,
                         int               height,
                         bool              bInvert ) { writeMask_(o_rOutputBuf,str,width,height,true,bInvert); }

// ------------------------------------------------------------------


int PDFOutDev::parseFont( long long nNewId, GfxFont* gfxFont, GfxState* state ) const
{
    FontAttributes aNewFont;
    int nSize = 0;

    GooString* pFamily = gfxFont->getName();
    if( pFamily )
    {
        aNewFont.familyName.clear();
        aNewFont.familyName.append( gfxFont->getName() );
    }
    else
    {
        aNewFont.familyName.clear();
        aNewFont.familyName.append( "Arial" );
    }

    aNewFont.isBold        = gfxFont->isBold();
    aNewFont.isItalic      = gfxFont->isItalic();
    aNewFont.size          = state->getTransformedFontSize();
    aNewFont.isUnderline   = false;

    if( gfxFont->getType() == fontTrueType || gfxFont->getType() == fontType1 )
    {
        // TODO(P3): Unfortunately, need to read stream twice, since
        // we must write byte count to stdout before
        char* pBuf = gfxFont->readEmbFontFile( m_pDoc->getXRef(), &nSize );
        if( pBuf )
            aNewFont.isEmbedded = true;
    }

    m_aFontMap[ nNewId ] = aNewFont;
    return nSize;
}

void PDFOutDev::writeFontFile( GfxFont* gfxFont ) const
{
    if( gfxFont->getType() != fontTrueType && gfxFont->getType() != fontType1 )
        return;

    int nSize = 0;
    char* pBuf = gfxFont->readEmbFontFile( m_pDoc->getXRef(), &nSize );
    if( !pBuf )
        return;

    // ---sync point--- see SYNC STREAMS above
    fflush(stdout);

    if( fwrite(pBuf, sizeof(char), nSize, g_binary_out) != (size_t)nSize )
        exit(1); // error

    // ---sync point--- see SYNC STREAMS above
    fflush(g_binary_out);
}

void PDFOutDev::printPath( GfxPath* pPath ) const
{
    int nSubPaths = pPath ? pPath->getNumSubpaths() : 0;
    for( int i=0; i<nSubPaths; i++ )
    {
        GfxSubpath* pSub  = pPath->getSubpath( i );
        const int nPoints = pSub->getNumPoints();

        printf( " subpath %d", pSub->isClosed() );

        for( int n=0; n<nPoints; ++n )
        {
            printf( " %f %f %d",
                    normalize(pSub->getX(n)),
                    normalize(pSub->getY(n)),
                    pSub->getCurve(n) );
        }
    }
}

PDFOutDev::PDFOutDev( PDFDoc* pDoc ) :
    m_pDoc( pDoc ),
    m_aFontMap(),
    m_pUtf8Map( new UnicodeMap((char*)"UTF-8", gTrue, &mapUTF8) )
{
}

void PDFOutDev::startPage(int /*pageNum*/, GfxState* state)
{
    assert(state);
    printf("startPage %f %f\n",
           normalize(state->getPageWidth()),
           normalize(state->getPageHeight()));
}

void PDFOutDev::endPage()
{
    printf("endPage\n");
}

#if POPPLER_CHECK_VERSION(0, 19, 0)
void PDFOutDev::processLink(AnnotLink *link)
#elif POPPLER_CHECK_VERSION(0, 17, 0)
void PDFOutDev::processLink(AnnotLink *link, Catalog *)
#else
void PDFOutDev::processLink(Link* link, Catalog*)
#endif
{
    assert(link);

    double x1,x2,y1,y2;
    link->getRect( &x1, &y1, &x2, &y2 );

    LinkAction* pAction = link->getAction();
    if( pAction->getKind() == actionURI )
    {
        const char* pURI = static_cast<LinkURI*>(pAction)->getURI()->getCString();

        boost::shared_array<char> pEsc( lcl_escapeLineFeeds(pURI) );

        printf( "drawLink %f %f %f %f %s\n",
                normalize(x1),
                normalize(y1),
                normalize(x2),
                normalize(y2),
                pEsc.get() );
    }
}

void PDFOutDev::saveState(GfxState*)
{
    printf( "saveState\n" );
}

void PDFOutDev::restoreState(GfxState*)
{
    printf( "restoreState\n" );
}

void PDFOutDev::setDefaultCTM(double *pMat)
{
    assert(pMat);

    OutputDev::setDefaultCTM(pMat);

    printf( "updateCtm %f %f %f %f %f %f\n",
            normalize(pMat[0]),
            normalize(pMat[2]),
            normalize(pMat[1]),
            normalize(pMat[3]),
            normalize(pMat[4]),
            normalize(pMat[5]) );
}

void PDFOutDev::updateCTM(GfxState* state,
                          double, double,
                          double, double,
                          double, double)
{
    assert(state);

    const double* const pMat = state->getCTM();
    assert(pMat);

    printf( "updateCtm %f %f %f %f %f %f\n",
            normalize(pMat[0]),
            normalize(pMat[2]),
            normalize(pMat[1]),
            normalize(pMat[3]),
            normalize(pMat[4]),
            normalize(pMat[5]) );
}

void PDFOutDev::updateLineDash(GfxState *state)
{
    assert(state);

    double* dashArray; int arrayLen; double startOffset;
    state->getLineDash(&dashArray, &arrayLen, &startOffset);

    printf( "updateLineDash" );
    if( arrayLen && dashArray )
    {
        printf( " %f %d", normalize(startOffset), arrayLen );
        for( int i=0; i<arrayLen; ++i )
            printf( " %f", normalize(*dashArray++) );
    }
    printf( "\n" );
}

void PDFOutDev::updateFlatness(GfxState *state)
{
    assert(state);
    printf( "updateFlatness %d\n", state->getFlatness() );
}

void PDFOutDev::updateLineJoin(GfxState *state)
{
    assert(state);
    printf( "updateLineJoin %d\n", state->getLineJoin() );
}

void PDFOutDev::updateLineCap(GfxState *state)
{
    assert(state);
    printf( "updateLineCap %d\n", state->getLineCap() );
}

void PDFOutDev::updateMiterLimit(GfxState *state)
{
    assert(state);
    printf( "updateMiterLimit %f\n", normalize(state->getMiterLimit()) );
}

void PDFOutDev::updateLineWidth(GfxState *state)
{
    assert(state);
    printf( "updateLineWidth %f\n", normalize(state->getLineWidth()) );
}

void PDFOutDev::updateFillColor(GfxState *state)
{
    assert(state);

    GfxRGB aRGB;
    state->getFillRGB( &aRGB );

    printf( "updateFillColor %f %f %f %f\n",
            normalize(colToDbl(aRGB.r)),
            normalize(colToDbl(aRGB.g)),
            normalize(colToDbl(aRGB.b)),
            normalize(state->getFillOpacity()) );
}

void PDFOutDev::updateStrokeColor(GfxState *state)
{
    assert(state);

    GfxRGB aRGB;
    state->getStrokeRGB( &aRGB );

    printf( "updateStrokeColor %f %f %f %f\n",
            normalize(colToDbl(aRGB.r)),
            normalize(colToDbl(aRGB.g)),
            normalize(colToDbl(aRGB.b)),
            normalize(state->getFillOpacity()) );
}

void PDFOutDev::updateFillOpacity(GfxState *state)
{
    updateFillColor(state);
}

void PDFOutDev::updateStrokeOpacity(GfxState *state)
{
    updateStrokeColor(state);
}

void PDFOutDev::updateBlendMode(GfxState*)
{
}

void PDFOutDev::updateFont(GfxState *state)
{
    assert(state);

    GfxFont *gfxFont = state->getFont();
    if( gfxFont )
    {
        FontAttributes aFont;
        int nEmbedSize=0;

        Ref* pID = gfxFont->getID();
        // TODO(Q3): Portability problem
        long long fontID = (long long)pID->gen << 32 | (long long)pID->num;
        boost::unordered_map< long long, FontAttributes >::const_iterator it =
            m_aFontMap.find( fontID );
        if( it == m_aFontMap.end() )
        {
            nEmbedSize = parseFont( fontID, gfxFont, state );
            it = m_aFontMap.find( fontID );
        }

        printf( "updateFont" );
        if( it != m_aFontMap.end() )
        {
            // conflating this with printf below crashes under Windoze
            printf( " %lld", fontID );

            aFont = it->second;

            boost::shared_array<char> pEsc( lcl_escapeLineFeeds(aFont.familyName.getCString()) );
            printf( " %d %d %d %d %f %d %s",
                    aFont.isEmbedded,
                    aFont.isBold,
                    aFont.isItalic,
                    aFont.isUnderline,
                    normalize(state->getTransformedFontSize()),
                    nEmbedSize,
                    pEsc.get() );
        }
        printf( "\n" );

        if( nEmbedSize )
            writeFontFile(gfxFont);
    }
}

void PDFOutDev::updateRender(GfxState *state)
{
    assert(state);

    printf( "setTextRenderMode %d\n", state->getRender() );
}

void PDFOutDev::stroke(GfxState *state)
{
    assert(state);

    printf( "strokePath" );
    printPath( state->getPath() );
    printf( "\n" );
}

void PDFOutDev::fill(GfxState *state)
{
    assert(state);

    printf( "fillPath" );
    printPath( state->getPath() );
    printf( "\n" );
}

void PDFOutDev::eoFill(GfxState *state)
{
    assert(state);

    printf( "eoFillPath" );
    printPath( state->getPath() );
    printf( "\n" );
}

void PDFOutDev::clip(GfxState *state)
{
    assert(state);

    printf( "clipPath" );
    printPath( state->getPath() );
    printf( "\n" );
}

void PDFOutDev::eoClip(GfxState *state)
{
    assert(state);

    printf( "eoClipPath" );
    printPath( state->getPath() );
    printf( "\n" );
}

/** Output one glyph


    @param dx
    horizontal skip for character (already scaled with font size) +
    inter-char space: cursor is shifted by this amount for next char

    @param dy
    vertical skip for character (zero for horizontal writing mode):
    cursor is shifted by this amount for next char

    @param originX
    local offset of character (zero for horizontal writing mode). not
    taken into account for output pos updates. Used for vertical writing.

    @param originY
    local offset of character (zero for horizontal writing mode). not
    taken into account for output pos updates. Used for vertical writing.
 */
void PDFOutDev::drawChar(GfxState *state, double x, double y,
                         double dx, double dy,
                         double originX, double originY,
                         CharCode, int /*nBytes*/, Unicode *u, int uLen)
{
    assert(state);

    if( u == NULL )
        return;

    // normalize coordinates: correct from baseline-relative to upper
    // left corner of glyphs
    double x2(0.0), y2(0.0);
    state->textTransformDelta( 0.0,
                               state->getFont()->getAscent(),
                               &x2, &y2 );
    const double fFontSize(state->getFontSize());
    x += x2*fFontSize;
    y += y2*fFontSize;

    const double aPositionX(x-originX);
    const double aPositionY(y-originY);
    // TODO(F2): use leading here, when set
    const double nWidth(dx != 0.0 ? dx : fFontSize);
    const double nHeight(dy != 0.0 ? dy : fFontSize);

    const double* pTextMat=state->getTextMat();
    printf( "drawChar %f %f %f %f %f %f %f %f ",
            normalize(aPositionX),
            normalize(aPositionY),
            normalize(aPositionX+nWidth),
            normalize(aPositionY-nHeight),
            normalize(pTextMat[0]),
            normalize(pTextMat[2]),
            normalize(pTextMat[1]),
            normalize(pTextMat[3]) );

    // silence spurious warning
    (void)&mapUCS2;

    char buf[9];
    for( int i=0; i<uLen; ++i )
    {
        buf[ m_pUtf8Map->mapUnicode(u[i], buf, sizeof(buf)-1) ] = 0;
        boost::shared_array<char> pEsc( lcl_escapeLineFeeds(buf) );
        printf( "%s", pEsc.get() );
    }

    printf( "\n" );
}

void PDFOutDev::drawString(GfxState*, GooString* /*s*/)
{
    // TODO(F3): NYI
}

void PDFOutDev::endTextObject(GfxState*)
{
    printf( "endTextObject\n" );
}

void PDFOutDev::drawImageMask(GfxState* pState, Object*, Stream* str,
                              int width, int height, GBool invert,
                              GBool /*inlineImg*/ )
{
    OutputBuffer aBuf; initBuf(aBuf);

    printf( "drawMask %d %d %d", width, height, invert );

    int bitsPerComponent = 1;
    StreamColorSpaceMode csMode = streamCSNone;
    str->getImageParams( &bitsPerComponent, &csMode );
    if( bitsPerComponent == 1 && (csMode == streamCSNone || csMode == streamCSDeviceGray) )
    {
        GfxRGB oneColor = { dblToCol( 1.0 ), dblToCol( 1.0 ), dblToCol( 1.0 ) };
        GfxRGB zeroColor = { dblToCol( 0.0 ), dblToCol( 0.0 ), dblToCol( 0.0 ) };
        pState->getFillColorSpace()->getRGB( pState->getFillColor(), &zeroColor );
        if( invert )
            writePng_( aBuf, str, width, height, oneColor, zeroColor, true, true );
        else
            writePng_( aBuf, str, width, height, zeroColor, oneColor, true, true );
    }
    else
        writeMaskLF(aBuf, str, width, height, invert != 0);
    writeBinaryBuffer(aBuf);
}

void PDFOutDev::drawImage(GfxState*, Object*, Stream* str,
                          int width, int height, GfxImageColorMap* colorMap,
                          int* maskColors, GBool /*inlineImg*/ )
{
    OutputBuffer aBuf; initBuf(aBuf);
    OutputBuffer aMaskBuf;

    printf( "drawImage %d %d", width, height );

    if( maskColors )
    {
        // write mask colors. nBytes must be even - first half is
        // lower bound values, second half upper bound values
        if( colorMap->getColorSpace()->getMode() == csIndexed )
        {
            aMaskBuf.push_back( (char)maskColors[0] );
            aMaskBuf.push_back( (char)maskColors[gfxColorMaxComps] );
        }
        else
        {
            GfxRGB aMinRGB;
            colorMap->getColorSpace()->getRGB(
                (GfxColor*)maskColors,
                &aMinRGB );

            GfxRGB aMaxRGB;
            colorMap->getColorSpace()->getRGB(
                (GfxColor*)maskColors+gfxColorMaxComps,
                &aMaxRGB );

            aMaskBuf.push_back( colToByte(aMinRGB.r) );
            aMaskBuf.push_back( colToByte(aMinRGB.g) );
            aMaskBuf.push_back( colToByte(aMinRGB.b) );
            aMaskBuf.push_back( colToByte(aMaxRGB.r) );
            aMaskBuf.push_back( colToByte(aMaxRGB.g) );
            aMaskBuf.push_back( colToByte(aMaxRGB.b) );
        }
    }

    printf( " %d", (int)aMaskBuf.size() );
    writeImageLF( aBuf, str, width, height, colorMap );
    writeBinaryBuffer(aBuf);
    writeBinaryBuffer(aMaskBuf);
}

void PDFOutDev::drawMaskedImage(GfxState*, Object*, Stream* str,
                                int width, int height,
                                GfxImageColorMap* colorMap,
                                Stream* maskStr,
                                int maskWidth, int maskHeight,
                                GBool maskInvert)
{
    OutputBuffer aBuf;     initBuf(aBuf);
    printf( "drawImage %d %d 0", width, height );
    writePng_( aBuf, str, width, height, colorMap, maskStr, maskWidth, maskHeight, maskInvert, true );
    writeBinaryBuffer( aBuf );
}

void PDFOutDev::drawSoftMaskedImage(GfxState*, Object*, Stream* str,
                                    int width, int height,
                                    GfxImageColorMap* colorMap,
                                    Stream* maskStr,
                                    int maskWidth, int maskHeight,
                                    GfxImageColorMap* maskColorMap )
{
    OutputBuffer aBuf;     initBuf(aBuf);
    printf( "drawImage %d %d 0", width, height );
    writePng_( aBuf, str, width, height, colorMap, maskStr, maskWidth, maskHeight, maskColorMap, true );
    writeBinaryBuffer( aBuf );
}

void PDFOutDev::setPageNum( int nNumPages )
{
    // TODO(F3): printf might format int locale-dependent!
    printf("setPageNum %d\n", nNumPages);
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
