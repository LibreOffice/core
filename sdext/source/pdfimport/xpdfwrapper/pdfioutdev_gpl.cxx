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

#if defined __GNUC__ || defined __clang__
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100) // unreferenced formal parameter
#pragma warning(disable : 4121) // alignment of a member was sensitive to packing in Gfx.h/Operator
#endif

#include <Gfx.h>
#include <splash/SplashBitmap.h>
#include <SplashOutputDev.h>
#if defined __GNUC__ || defined __clang__
# pragma GCC diagnostic pop
#elif defined _MSC_VER
#pragma warning(pop)
#endif


#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include <memory>
#include <vector>

// sigh, UTF8.h was removed in poppler-0.21.0 and put back in 0.21.1, then renamed to UnicodeMapFuncs.h in 0.62.0
// FIXME: we can't use #if POPPLER_CHECK_VERSION(0, 21, 0) && !POPPLER_CHECK_VERSION(0, 21, 1)
//        because the internal poppler does not provide poppler-version.h and the macro always returns 0
#if POPPLER_CHECK_VERSION(0, 62, 0)
#include <UnicodeMapFuncs.h>
#elif POPPLER_CHECK_VERSION(0, 21, 1)
#include <UTF8.h>
#elif POPPLER_CHECK_VERSION(0, 21, 0)
#include "UTF.h"
#else
#include "UTF8.h"
#endif

#ifdef _WIN32
# define snprintf _snprintf

#if defined __GNUC__
#pragma GCC diagnostic warning "-Wformat"
#pragma GCC diagnostic warning "-Wformat-extra-args"
#endif
#endif

/* SYNC STREAMS
   ============

   We stream human-readable tokens to stdout, and binary data (fonts,
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
static double normalize( double val )
{
    return fabs(val) < 0.0000001 ? 0.0 : val;
}

namespace
{

/** Escapes line-ending characters (\n and \r) in input string.
  */
std::vector<char> lcl_escapeLineFeeds(const char* const i_pStr)
{
    size_t nLength(strlen(i_pStr));
    std::vector<char> aBuffer;
    aBuffer.reserve(2*nLength+1);

    const char* pRead = i_pStr;
    while( nLength-- )
    {
        if( *pRead == '\r' )
        {
            aBuffer.push_back('\\');
            aBuffer.push_back('r');
        }
        else if( *pRead == '\n' )
        {
            aBuffer.push_back('\\');
            aBuffer.push_back('n');
        }
        else if( *pRead == '\\' )
        {
            aBuffer.push_back('\\');
            aBuffer.push_back('\\');
        }
        else
            aBuffer.push_back(*pRead);
        pRead++;
    }
    aBuffer.push_back(0);

    return aBuffer;
}

}

/// for the temp char buffer the header gets snprintfed in
#define WRITE_BUFFER_SIZE 1024

/// for the initial std::vector capacity when copying stream from xpdf
#define WRITE_BUFFER_INITIAL_CAPACITY (1024*100)

static void initBuf(OutputBuffer& io_rBuffer)
{
    io_rBuffer.reserve(WRITE_BUFFER_INITIAL_CAPACITY);
}

static void writeBinaryBuffer( const OutputBuffer& rBuffer )
{
    // ---sync point--- see SYNC STREAMS above
    fflush(stdout);

    // put buffer to stderr
    if( !rBuffer.empty() )
        if( fwrite(rBuffer.data(), sizeof(char),
                   rBuffer.size(), g_binary_out) != static_cast<size_t>(rBuffer.size()) )
            exit(1); // error

    // ---sync point--- see SYNC STREAMS above
    fflush(g_binary_out);
}

static bool ExtractJpegData(Stream* str, OutputBuffer& outBuf)
{
    int bytesToMarker = 0;
    int bytesToLen = -1;
    bool collectBytes = false;
    int startOfScan = 0;
    int b1 = -1;
    for (; ; )
    {
        const int b2 = b1;
        b1 = str->getChar();

        if (b1 == -1)
            return false;

        if (collectBytes)
        {
            outBuf.push_back(static_cast<Output_t>(b1));

            bytesToMarker--;
            bytesToLen--;
        }

        if (bytesToMarker == 0)
        {
            if (startOfScan == 1)
            {
                bytesToMarker = -1;
                startOfScan = 2;
            }
            else if (b2 == 0xFF)
            {
                if (b1 == 0xD8)
                {
                    collectBytes = true;
                    bytesToMarker = 2;

                    outBuf.push_back(Output_t(0xFF));
                    outBuf.push_back(Output_t(0xD8));
                }
                else
                {
                    bytesToLen = 2;
                }
                if (b1 == 0xDA)
                {
                    startOfScan = 1;
                }
            }
            else if (collectBytes)
            {
                return false;
            }
        }

        if (bytesToLen == 0)
        {
            bytesToMarker = b2 * 256 + b1;
        }

        if (startOfScan == 2)
            if ((b2 == 0xFF) && (b1 == 0xD9))
                return true;
    }
}

static void writeJpeg_( OutputBuffer& o_rOutputBuf, Stream* str )
{
    // dump JPEG file as-is
#if POPPLER_CHECK_VERSION(0, 17, 3)
    str = str->getNextStream();
#else
    str = ((DCTStream *)str)->getRawStream();
#endif
    str->reset();

    o_rOutputBuf.clear();
    ExtractJpegData(str, o_rOutputBuf);

    printf( " JPEG %d", static_cast<int>(o_rOutputBuf.size()) );
    printf("\n");

    str->close();
}

static void writePbm_(OutputBuffer& o_rOutputBuf, Stream* str, int width, int height, bool bInvert )
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

static void writePpm_( OutputBuffer&     o_rOutputBuf,
                Stream*           str,
                int               width,
                int               height,
                GfxImageColorMap* colorMap )
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
    printf("\n");

    // trim buffer to exact header size
    o_rOutputBuf.resize(header_size);

    // initialize stream
    unsigned char *p;
    GfxRGB rgb;
    std::unique_ptr<ImageStream> imgStr(
        new ImageStream(str,
                        width,
                        colorMap->getNumPixelComps(),
                        colorMap->getBits()));
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
}

// call this only for 1 bit image streams !
static void writePng_( OutputBuffer&     o_rOutputBuf,
                Stream*           str,
                int               width,
                int               height,
                GfxRGB const &    zeroColor,
                GfxRGB const &    oneColor,
                bool              bIsMask )
{
    o_rOutputBuf.clear();

    // get png image
    PngHelper::createPng( o_rOutputBuf, str, width, height, zeroColor, oneColor, bIsMask );

    printf( " PNG %d", static_cast<int>(o_rOutputBuf.size()) );
    printf("\n");
}

static void writePng_( OutputBuffer& o_rOutputBuf,
                Stream* str,
                int width, int height, GfxImageColorMap* colorMap,
                Stream* maskStr,
                int maskWidth, int maskHeight, GfxImageColorMap* maskColorMap )
{
    o_rOutputBuf.clear();

    // get png image
    PngHelper::createPng( o_rOutputBuf, str, width, height, colorMap, maskStr, maskWidth, maskHeight, maskColorMap );

    printf( " PNG %d", static_cast<int>(o_rOutputBuf.size()) );
    printf("\n");
}

static void writePng_( OutputBuffer& o_rOutputBuf,
                Stream* str,
                int width, int height, GfxImageColorMap* colorMap,
                Stream* maskStr,
                int maskWidth, int maskHeight, bool maskInvert )
{
    o_rOutputBuf.clear();

    // get png image
    PngHelper::createPng( o_rOutputBuf, str, width, height, colorMap, maskStr, maskWidth, maskHeight, maskInvert );

    printf( " PNG %d", static_cast<int>(o_rOutputBuf.size()) );
    printf("\n");
}

// stolen from ImageOutputDev.cc
static void writeMask_( OutputBuffer& o_rOutputBuf, Stream* str, int width, int height, bool bInvert )
{
    if( str->getKind() == strDCT )
        writeJpeg_(o_rOutputBuf, str);
    else
        writePbm_(o_rOutputBuf, str, width, height, bInvert );
}

static void writeImage_( OutputBuffer&     o_rOutputBuf,
                  Stream*           str,
                  int               width,
                  int               height,
                  GfxImageColorMap* colorMap )
{
    // dump JPEG file
    if( str->getKind() == strDCT &&
        (colorMap->getNumPixelComps() == 1 ||
         colorMap->getNumPixelComps() == 3) )
    {
        writeJpeg_(o_rOutputBuf, str);
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
            unsigned char nIndex = 0;
            colorMap->getRGB( &nIndex, &zeroColor );
            nIndex = 1;
            colorMap->getRGB( &nIndex, &oneColor );
        }
        writePng_( o_rOutputBuf, str, width, height, zeroColor, oneColor, false);
    }
    else
        writePpm_( o_rOutputBuf, str, width, height, colorMap );
}

// forwarders
static void writeImageLF( OutputBuffer&     o_rOutputBuf,
                          Stream*           str,
                          int               width,
                          int               height,
                          GfxImageColorMap* colorMap ) { writeImage_(o_rOutputBuf,str,width,height,colorMap); }
static void writeMaskLF( OutputBuffer&     o_rOutputBuf,
                         Stream*           str,
                         int               width,
                         int               height,
                         bool              bInvert ) { writeMask_(o_rOutputBuf,str,width,height,bInvert); }

// Vertically flip the bitmap
static void flipSplashBitmap(SplashBitmap *pBitmap)
{
    if (pBitmap->getRowSize() <= 0)
        return;

    auto nBitmapHeight = static_cast<size_t>(pBitmap->getHeight());
    auto nRowSize = static_cast<size_t>(pBitmap->getRowSize());
    auto nAlphaRowSize = static_cast<size_t>(pBitmap->getAlphaRowSize());

    auto aTmpRow = new unsigned char[nRowSize];
    auto aTmpAlphaRow = new unsigned char[nAlphaRowSize];

    auto pBitmapData = pBitmap->getDataPtr();
    auto pAlphaData = pBitmap->getAlphaPtr();

    // Set up pairs of pointers working from each end of the bitmap
    auto pCurRowA = pBitmapData;
    auto pCurAlphaA = pAlphaData;
    auto pCurRowB = pBitmapData+nRowSize*(nBitmapHeight-1);
    auto pCurAlphaB = pAlphaData+nAlphaRowSize*(nBitmapHeight-1);

    for (size_t nCur = 0;
         nCur < nBitmapHeight/2;
         nCur++, pCurRowA+=nRowSize, pCurRowB-=nRowSize,
         pCurAlphaA+=nAlphaRowSize, pCurAlphaB-=nAlphaRowSize)
    {
        memcpy(aTmpRow, pCurRowA, nRowSize);
        memcpy(pCurRowA, pCurRowB, nRowSize);
        memcpy(pCurRowB, aTmpRow, nRowSize);

        memcpy(aTmpAlphaRow, pCurAlphaA, nAlphaRowSize);
        memcpy(pCurAlphaA, pCurAlphaB, nAlphaRowSize);
        memcpy(pCurAlphaB, aTmpAlphaRow, nAlphaRowSize);
    }
    delete[] aTmpRow;
    delete[] aTmpAlphaRow;
}

int PDFOutDev::parseFont( long long nNewId, GfxFont* gfxFont, const GfxState* state ) const
{
    FontAttributes aNewFont;
    int nSize = 0;

#if POPPLER_CHECK_VERSION(20, 12, 0)
    std::string familyName = gfxFont->getNameWithoutSubsetTag();
#else
#if POPPLER_CHECK_VERSION(0, 71, 0) // GooString::toStr()
    std::string familyName = gfxFont->getName()->toStr();
#else
    const GooString* gooString = gfxFont->getName();
    std::string familyName = std::string(gooString->getCString(), gooString->getLength());
#endif
    if (familyName.length() > 7 && familyName.at(6) == '+')
    {
        familyName = familyName.substr(7);
    }
#endif
    if( familyName != "" )
    {
        aNewFont.familyName.clear();
#if POPPLER_CHECK_VERSION(0, 83, 0) // GooString::append(const std::string&)
        aNewFont.familyName.append( familyName );
#else
        aNewFont.familyName.append( familyName.c_str() );
#endif
    }
    else
    {
        aNewFont.familyName.clear();
        aNewFont.familyName.append( "Arial" );
    }

    aNewFont.maFontWeight  = gfxFont->getWeight();
    aNewFont.isItalic      = gfxFont->isItalic();
#if POPPLER_CHECK_VERSION(0, 83, 0) // const added to getTransformedFontSize
    aNewFont.size          = state->getTransformedFontSize();
#else
    aNewFont.size          = const_cast<GfxState*>(state)->getTransformedFontSize();
#endif
    aNewFont.isUnderline   = false;

    if( gfxFont->getType() == fontTrueType || gfxFont->getType() == fontType1 )
    {
        // TODO(P3): Unfortunately, need to read stream twice, since
        // we must write byte count to stdout before
#if POPPLER_CHECK_VERSION(22, 6, 0)
        std::optional<std::vector<unsigned char>> pBuf = gfxFont->readEmbFontFile( m_pDoc->getXRef() );
        if ( pBuf )
        {
            aNewFont.isEmbedded = true;
            nSize = pBuf->size();
        }
#else
        char* pBuf = gfxFont->readEmbFontFile( m_pDoc->getXRef(), &nSize );
        if( pBuf )
        {
            aNewFont.isEmbedded = true;
            gfree(pBuf);
        }
#endif
    }

    m_aFontMap[ nNewId ] = aNewFont;
    return nSize;
}

void PDFOutDev::writeFontFile( GfxFont* gfxFont ) const
{
    if( gfxFont->getType() != fontTrueType && gfxFont->getType() != fontType1 )
        return;

    int nSize = 0;
#if POPPLER_CHECK_VERSION(22, 6, 0)
    std::optional<std::vector<unsigned char>> pBuf = gfxFont->readEmbFontFile( m_pDoc->getXRef() );
    if ( pBuf )
        nSize = pBuf->size();
    if ( nSize == 0 )
        return;
#else
    char* pBuf = gfxFont->readEmbFontFile( m_pDoc->getXRef(), &nSize );
    if( !pBuf )
        return;
#endif

    // ---sync point--- see SYNC STREAMS above
    fflush(stdout);

#if POPPLER_CHECK_VERSION(22, 6, 0)
    if( fwrite(pBuf->data(), sizeof(*pBuf->data()), nSize, g_binary_out) != static_cast<size_t>(nSize) )
    {
        exit(1); // error
    }
    // ---sync point--- see SYNC STREAMS above
    fflush(g_binary_out);
#else
    if( fwrite(pBuf, sizeof(char), nSize, g_binary_out) != static_cast<size_t>(nSize) )
    {
        gfree(pBuf);
        exit(1); // error
    }
    // ---sync point--- see SYNC STREAMS above
    fflush(g_binary_out);
    gfree(pBuf);
#endif
}

#if POPPLER_CHECK_VERSION(0, 83, 0)
void PDFOutDev::printPath( const GfxPath* pPath )
#else
void PDFOutDev::printPath( GfxPath* pPath )
#endif
{
    int nSubPaths = pPath ? pPath->getNumSubpaths() : 0;
    for( int i=0; i<nSubPaths; i++ )
    {
#if POPPLER_CHECK_VERSION(0, 83, 0)
        const
#endif
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
    m_pUtf8Map( new UnicodeMap("UTF-8", true, &mapUTF8) ),
    m_bSkipImages(false)
{
}
PDFOutDev::~PDFOutDev()
{
}

void PDFOutDev::startPage(int /*pageNum*/, GfxState* state
#if POPPLER_CHECK_VERSION(0, 23, 0) ||  POPPLER_CHECK_VERSION(0, 24, 0)
                          , XRef* /*xref*/
#endif
)
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
    if (!(pAction && pAction->getKind() == actionURI))
        return;

#if POPPLER_CHECK_VERSION(0, 86, 0)
    const char* pURI = static_cast<LinkURI*>(pAction)->getURI().c_str();
#elif POPPLER_CHECK_VERSION(0, 72, 0)
    const char* pURI = static_cast<LinkURI*>(pAction)->getURI()->c_str();
#else
    const char* pURI = static_cast<LinkURI*>(pAction)->getURI()->getCString();
#endif

    std::vector<char> aEsc( lcl_escapeLineFeeds(pURI) );

    printf( "drawLink %f %f %f %f %s\n",
            normalize(x1),
            normalize(y1),
            normalize(x2),
            normalize(y2),
            aEsc.data() );
}

void PDFOutDev::saveState(GfxState*)
{
    printf( "saveState\n" );
}

void PDFOutDev::restoreState(GfxState*)
{
    printf( "restoreState\n" );
}

#if POPPLER_CHECK_VERSION(0, 71, 0)
void PDFOutDev::setDefaultCTM(const double *pMat)
#else
void PDFOutDev::setDefaultCTM(double *pMat)
#endif
{
    assert(pMat);

    OutputDev::setDefaultCTM(pMat);

    printf( "updateCtm %f %f %f %f %f %f\n",
            normalize(pMat[0]),
            normalize(pMat[1]),
            normalize(pMat[2]),
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
            normalize(pMat[1]),
            normalize(pMat[2]),
            normalize(pMat[3]),
            normalize(pMat[4]),
            normalize(pMat[5]) );
}

void PDFOutDev::updateLineDash(GfxState *state)
{
    if (m_bSkipImages)
        return;
    assert(state);

    int arrayLen; double startOffset;
#if POPPLER_CHECK_VERSION(22, 9, 0)
    const std::vector<double> &dash = state->getLineDash(&startOffset);
    const double* dashArray = dash.data();
    arrayLen = dash.size();
#else
    double* dashArray;
    state->getLineDash(&dashArray, &arrayLen, &startOffset);
#endif

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
    if (m_bSkipImages)
        return;
    assert(state);
    printf( "updateFlatness %d\n", state->getFlatness() );
}

void PDFOutDev::updateLineJoin(GfxState *state)
{
    if (m_bSkipImages)
        return;
    assert(state);
    printf( "updateLineJoin %d\n", state->getLineJoin() );
}

void PDFOutDev::updateLineCap(GfxState *state)
{
    if (m_bSkipImages)
        return;
    assert(state);
    printf( "updateLineCap %d\n", state->getLineCap() );
}

void PDFOutDev::updateMiterLimit(GfxState *state)
{
    if (m_bSkipImages)
        return;
    assert(state);
    printf( "updateMiterLimit %f\n", normalize(state->getMiterLimit()) );
}

void PDFOutDev::updateLineWidth(GfxState *state)
{
    if (m_bSkipImages)
        return;
    assert(state);
    printf( "updateLineWidth %f\n", normalize(state->getLineWidth()) );
}

void PDFOutDev::updateFillColor(GfxState *state)
{
    if (m_bSkipImages)
        return;
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
    if (m_bSkipImages)
        return;
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
    if (m_bSkipImages)
        return;
    updateFillColor(state);
}

void PDFOutDev::updateStrokeOpacity(GfxState *state)
{
    if (m_bSkipImages)
        return;
    updateStrokeColor(state);
}

void PDFOutDev::updateBlendMode(GfxState*)
{
}

void PDFOutDev::updateFont(GfxState *state)
{
    assert(state);

#if POPPLER_CHECK_VERSION(22, 6, 0)
    GfxFont *gfxFont = state->getFont().get();
#else
    GfxFont *gfxFont = state->getFont();
#endif
    if( !gfxFont )
        return;

    FontAttributes aFont;
    int nEmbedSize=0;

#if POPPLER_CHECK_VERSION(0, 64, 0)
    const
#endif
    Ref* pID = gfxFont->getID();
    // TODO(Q3): Portability problem
    long long fontID = static_cast<long long>(pID->gen) << 32 | static_cast<long long>(pID->num);
    std::unordered_map< long long, FontAttributes >::const_iterator it =
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

#if POPPLER_CHECK_VERSION(0, 72, 0)
        std::vector<char> aEsc( lcl_escapeLineFeeds(aFont.familyName.c_str()) );
#else
        std::vector<char> aEsc( lcl_escapeLineFeeds(aFont.familyName.getCString()) );
#endif
        printf( " %d %d %d %d %f %d %s",
                aFont.isEmbedded,
                aFont.maFontWeight,
                aFont.isItalic,
                aFont.isUnderline,
                normalize(state->getTransformedFontSize()),
                nEmbedSize,
                aEsc.data() );
    }
    printf( "\n" );

    if (nEmbedSize)
    {
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
    if (m_bSkipImages)
        return;
    assert(state);

    printf( "strokePath" );
    printPath( state->getPath() );
    printf( "\n" );
}

void PDFOutDev::fill(GfxState *state)
{
    if (m_bSkipImages)
        return;
    assert(state);

    printf( "fillPath" );
    printPath( state->getPath() );
    printf( "\n" );
}

void PDFOutDev::eoFill(GfxState *state)
{
    if (m_bSkipImages)
        return;
    assert(state);

    printf( "eoFillPath" );
    printPath( state->getPath() );
    printf( "\n" );
}

void PDFOutDev::clip(GfxState *state)
{
    if (m_bSkipImages)
        return;
    assert(state);

    printf( "clipPath" );
    printPath( state->getPath() );
    printf( "\n" );
}

void PDFOutDev::eoClip(GfxState *state)
{
    if (m_bSkipImages)
        return;
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

#if POPPLER_CHECK_VERSION(0, 82, 0)
void PDFOutDev::drawChar(GfxState *state, double x, double y,
                         double dx, double dy,
                         double originX, double originY,
                         CharCode, int /*nBytes*/, const Unicode *u, int uLen)
{
#else
void PDFOutDev::drawChar(GfxState *state, double x, double y,
                         double dx, double dy,
                         double originX, double originY,
                         CharCode, int /*nBytes*/, Unicode *u, int uLen)
{
#endif
    assert(state);

    if( u == nullptr )
        return;

    // Fix for tdf#96080
    if (uLen == 4 && u[0] == '\t' && u[1] == '\r' && u[2] == ' ' && u[3] == 0xA0)
    {
        u += 2;
        uLen = 1;
    }

    double csdx = 0.0;
    double csdy = 0.0;
    if (!state->getFont() || !state->getFont()->getWMode())
    {
        csdx = state->getCharSpace();
        if (*u == ' ')
            csdx += state->getWordSpace();
        csdx *= state->getHorizScaling();
    }
    else
    {
        csdy = state->getCharSpace();
        if (*u == ' ')
            csdy += state->getWordSpace();
    }

    double cstdx = 0.0;
    double cstdy = 0.0;
    state->textTransformDelta(csdx, csdy, &cstdx, &cstdy);

    const double fontSize = state->getFontSize();

    const double aPositionX(x-originX);
    const double aPositionY(y-originY);

    const double* pTextMat=state->getTextMat();
    printf( "drawChar %f %f %f %f %f %f %f %f %f ",
            normalize(aPositionX),
            normalize(aPositionY),
            normalize(aPositionX + dx - cstdx),
            normalize(aPositionY + dy - cstdy),
            normalize(pTextMat[0]),
            normalize(pTextMat[2]),
            normalize(pTextMat[1]),
            normalize(pTextMat[3]),
            normalize(fontSize)
            );

    // silence spurious warning
#if POPPLER_CHECK_VERSION(0, 62, 0)
    (void)&mapUTF16;
#else
    (void)&mapUCS2;
#endif

    char buf[9];
    for( int i=0; i<uLen; ++i )
    {
        buf[ m_pUtf8Map->mapUnicode(u[i], buf, sizeof(buf)-1) ] = 0;
        std::vector<char> aEsc( lcl_escapeLineFeeds(buf) );
        printf( "%s", aEsc.data() );
    }

    printf( "\n" );
}

#if POPPLER_CHECK_VERSION(0, 64, 0)
void PDFOutDev::drawString(GfxState*, const GooString* /*s*/)
#else
void PDFOutDev::drawString(GfxState*, GooString* /*s*/)
#endif
{
    // TODO(F3): NYI
}

void PDFOutDev::endTextObject(GfxState*)
{
    printf( "endTextObject\n" );
}

void PDFOutDev::drawImageMask(GfxState* pState, Object*, Stream* str,
                              int width, int height, poppler_bool invert,
                              poppler_bool /*interpolate*/,
                              poppler_bool /*inlineImg*/ )
{
    if (m_bSkipImages)
        return;
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
            writePng_( aBuf, str, width, height, oneColor, zeroColor, true );
        else
            writePng_( aBuf, str, width, height, zeroColor, oneColor, true );
    }
    else
        writeMaskLF(aBuf, str, width, height, invert);
    writeBinaryBuffer(aBuf);
}

#if POPPLER_CHECK_VERSION(0, 82, 0)
void PDFOutDev::drawImage(GfxState*, Object*, Stream* str,
                          int width, int height, GfxImageColorMap* colorMap,
                          poppler_bool /*interpolate*/,
                          const int* maskColors, poppler_bool /*inlineImg*/ )
{
#else
void PDFOutDev::drawImage(GfxState*, Object*, Stream* str,
                          int width, int height, GfxImageColorMap* colorMap,
                          poppler_bool /*interpolate*/,
                          int* maskColors, poppler_bool /*inlineImg*/ )
{
#endif
    if (m_bSkipImages)
        return;
    OutputBuffer aBuf; initBuf(aBuf);
    OutputBuffer aMaskBuf;

    printf( "drawImage %d %d", width, height );

    if( maskColors )
    {
        // write mask colors. nBytes must be even - first half is
        // lower bound values, second half upper bound values
        if( colorMap->getColorSpace()->getMode() == csIndexed )
        {
            aMaskBuf.push_back( static_cast<char>(maskColors[0]) );
            aMaskBuf.push_back( static_cast<char>(maskColors[gfxColorMaxComps]) );
        }
        else
        {
            GfxRGB aMinRGB;
            colorMap->getColorSpace()->getRGB(
#if POPPLER_CHECK_VERSION(0, 82, 0)
                reinterpret_cast<const GfxColor*>(maskColors),
#else
                reinterpret_cast<GfxColor*>(maskColors),
#endif
                &aMinRGB );

            GfxRGB aMaxRGB;
            colorMap->getColorSpace()->getRGB(
#if POPPLER_CHECK_VERSION(0, 82, 0)
                reinterpret_cast<const GfxColor*>(maskColors)+gfxColorMaxComps,
#else
                reinterpret_cast<GfxColor*>(maskColors)+gfxColorMaxComps,
#endif
                &aMaxRGB );

            aMaskBuf.push_back( colToByte(aMinRGB.r) );
            aMaskBuf.push_back( colToByte(aMinRGB.g) );
            aMaskBuf.push_back( colToByte(aMinRGB.b) );
            aMaskBuf.push_back( colToByte(aMaxRGB.r) );
            aMaskBuf.push_back( colToByte(aMaxRGB.g) );
            aMaskBuf.push_back( colToByte(aMaxRGB.b) );
        }
    }

    printf( " %d", static_cast<int>(aMaskBuf.size()) );
    writeImageLF( aBuf, str, width, height, colorMap );
    writeBinaryBuffer(aBuf);
    writeBinaryBuffer(aMaskBuf);
}

void PDFOutDev::drawMaskedImage(GfxState*, Object*, Stream* str,
                                int width, int height,
                                GfxImageColorMap* colorMap,
                                poppler_bool /*interpolate*/,
                                Stream* maskStr,
                                int maskWidth, int maskHeight,
                                poppler_bool maskInvert, poppler_bool /*maskInterpolate*/
                               )
{
    if (m_bSkipImages)
        return;
    OutputBuffer aBuf;     initBuf(aBuf);
    printf( "drawImage %d %d 0", width, height );
    writePng_( aBuf, str, width, height, colorMap, maskStr, maskWidth, maskHeight, maskInvert );
    writeBinaryBuffer( aBuf );
}

void PDFOutDev::drawSoftMaskedImage(GfxState*, Object*, Stream* str,
                                    int width, int height,
                                    GfxImageColorMap* colorMap,
                                    poppler_bool /*interpolate*/,
                                    Stream* maskStr,
                                    int maskWidth, int maskHeight,
                                    GfxImageColorMap* maskColorMap
                                    , poppler_bool /*maskInterpolate*/
                                   )
{
    if (m_bSkipImages)
        return;
    OutputBuffer aBuf;     initBuf(aBuf);
    printf( "drawImage %d %d 0", width, height );
    writePng_( aBuf, str, width, height, colorMap, maskStr, maskWidth, maskHeight, maskColorMap );
    writeBinaryBuffer( aBuf );
}

void PDFOutDev::setPageNum( int nNumPages )
{
    // TODO(F3): printf might format int locale-dependent!
    printf("setPageNum %d\n", nNumPages);
}

void PDFOutDev::setSkipImages( bool bSkipImages )
{
    m_bSkipImages = bSkipImages;
}

#if POPPLER_CHECK_VERSION(21, 3, 0)
poppler_bool PDFOutDev::tilingPatternFill(GfxState *state, Gfx *, Catalog *,
                                          GfxTilingPattern *tPat, const double *mat,
                                          int x0, int y0, int x1, int y1,
                                          double xStep, double yStep)
{
    const double *pBbox = tPat->getBBox();
    const int nPaintType = tPat->getPaintType();
    Dict *pResDict = tPat->getResDict();
    Object *aStr = tPat->getContentStream();
    double nWidth = pBbox[2] - pBbox[0];
    double nHeight = pBbox[3] - pBbox[1];

    // If our wrapper is skipping images then we don't need to do anything
    // but return 'true' so that Poppler doesn't do the slow method
    if (m_bSkipImages)
        return true;

    // Copied from the Cairo output dev; I think this is patterns
    // with gaps, let poppler do the slow method for now.
    if (xStep != nWidth || yStep != nHeight)
        return false;

    printf( "tilingPatternFill %d %d %d %d %f %f "
            "%d "
            "%f %f %f %f %f %f", // No ending space!

            x0, y0, x1, y1, normalize(xStep), normalize(yStep),

            nPaintType,

            normalize(mat[0]), normalize(mat[1]),
            normalize(mat[2]), normalize(mat[3]),
            normalize(mat[4]), normalize(mat[5])
            );

    PDFRectangle aBox;
    aBox.x1 = pBbox[0];
    aBox.y1 = pBbox[1];
    aBox.x2 = pBbox[2];
    aBox.y2 = pBbox[3];

    const int nDPI = 72; // GfxState seems to have 72.0 as magic for some reason
    auto pSplashGfxState = new GfxState(nDPI, nDPI, &aBox, 0, false);
    auto pSplashOut = new SplashOutputDev(splashModeRGB8, 1, false, nullptr);
    pSplashOut->setEnableFreeType(false);
    pSplashOut->startPage(0 /* pageNum */, pSplashGfxState, nullptr /* xref */);

    auto pSplashGfx = new Gfx(m_pDoc, pSplashOut, pResDict, &aBox, nullptr);
    pSplashGfx->display(aStr);
    auto pSplashBitmap = pSplashOut->takeBitmap();
    delete pSplashGfxState;
    delete pSplashGfx;
    delete pSplashOut;

    // Add a vertical flip, we can't do this in LO for an image filled poly
    flipSplashBitmap(pSplashBitmap);

    auto nBitmapWidth = static_cast<size_t>(pSplashBitmap->getWidth());
    auto nBitmapHeight = static_cast<size_t>(pSplashBitmap->getHeight());

    char *pBitmapData = reinterpret_cast<char *>(pSplashBitmap->getDataPtr());
    if (nPaintType == 2)
    {
        // My understanding is Type 2 fills are just bitmaps of *what* to fill
        // in the current fill colour.
        // sending it to LO as a flat colour image with the alpha map is easiest
        GfxRGB aCurFill;
        unsigned char r,g,b;
        state->getFillColorSpace()->getRGB(state->getFillColor(), &aCurFill);
        r = colToByte(aCurFill.r);
        g = colToByte(aCurFill.g);
        b = colToByte(aCurFill.b);

        for(size_t i=0; i < (nBitmapWidth * nBitmapHeight * 3); i+=3)
        {
            pBitmapData[i  ] = r;
            pBitmapData[i+1] = g;
            pBitmapData[i+2] = b;
        }
    }

    auto pRgbStr = new MemStream(pBitmapData, 0,
        nBitmapWidth * nBitmapHeight * 3, Object(objNull));
    auto pAlphaStr = new MemStream(reinterpret_cast<char *>(pSplashBitmap->getAlphaPtr()), 0,
        nBitmapWidth * nBitmapHeight, Object(objNull));
    auto aDecode = Object(objNull);
    auto pRgbIdentityColorMap = new GfxImageColorMap(8, &aDecode, new GfxDeviceRGBColorSpace());
    auto pGrayIdentityColorMap = new GfxImageColorMap(8, &aDecode, new GfxDeviceGrayColorSpace());

    OutputBuffer aBuf; initBuf(aBuf);
    writePng_(aBuf, pRgbStr, nBitmapWidth, nBitmapHeight, pRgbIdentityColorMap,
        pAlphaStr, nBitmapWidth, nBitmapHeight, pGrayIdentityColorMap);
    writeBinaryBuffer(aBuf);

    delete pAlphaStr;
    delete pRgbStr;
    delete pSplashBitmap;

    // If we return false here we can fall back to the slow path
    return true;
}

// This could be implemented for earlier versions, but the interface keeps
// changing a little; not having it is only a problem for inputs with
// large patterns.
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
