/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <string.h>
#include <cmath>

#include <vector>
#include <atomic>
#include <iostream>
#include <osl/time.h>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKitInit.h>
#include <LibreOfficeKit/LibreOfficeKit.hxx>

#ifdef IOS
#include <vcl/svapp.hxx>
#endif

#include <boost/property_tree/json_parser.hpp>

using namespace lok;

static int help( const char *error = nullptr )
{
    if (error)
        fprintf (stderr, "Error: %s\n\n", error);
    fprintf( stderr, "Usage: tilebench <absolute-path-to-libreoffice-install> [path to document] [--preinit] <options>\n");
    fprintf( stderr, "\trenders a selection of small tiles from the document, checksums them and times the process based on options:\n" );
    fprintf( stderr, "\t--tile\t[max parts|-1] [max tiles|-1]\n" );
    fprintf( stderr, "\t--dialog\t<.uno:Command>\n" );
    fprintf( stderr, "\t--join\trun tile joining tests\n" );
    return 1;
}

static double getTimeNow()
{
    TimeValue aValue;
    osl_getSystemTime(&aValue);
    return static_cast<double>(aValue.Seconds) +
           static_cast<double>(aValue.Nanosec) / (1000*1000*1000);
}

static double origin;

namespace {

struct TimeRecord {
    const char *mpName;
    double mfTime;

    TimeRecord() : mpName(nullptr), mfTime(getTimeNow()) { }
    explicit TimeRecord(const char *pName) :
        mpName(pName), mfTime(getTimeNow())
    {
        fprintf(stderr, "%3.3fs - %s\n", (mfTime - origin), mpName);
    }
};

}

static std::vector< TimeRecord > aTimes;

/// Dump an array (or sub-array) of RGBA or BGRA to an RGB PPM file.
static void dumpTile(const char *pNameStem,
                     const int nWidth, const int nHeight,
                     const int mode, const unsigned char* pBufferU,
                     const int nOffX = 0, const int nOffY = 0,
                     int nTotalWidth = -1)
{
    if (nTotalWidth < 0)
        nTotalWidth = nWidth;

    auto pBuffer = reinterpret_cast<const char *>(pBufferU);
    static int counter = 0;
    std::string aName = "/tmp/dump_tile";
    aName += pNameStem;
    aName += "_" + std::to_string(counter);
    aName += ".ppm";
#ifndef IOS
    std::ofstream ofs(aName);
#else
    NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    NSString *documentsDirectory = [paths objectAtIndex:0];
    NSString *path = [NSString stringWithFormat:@"%@/dump_tile_%d.ppm", documentsDirectory, counter];
    std::ofstream ofs([path UTF8String]);
    std::cerr << "---> Dumping tile\n";
#endif
    counter++;
    ofs << "P6\n"
        << nWidth << " "
        << nHeight << "\n"
        << 255 << "\n" ;

    const bool dumpText = false;

    if (dumpText)
        fprintf(stderr, "Stream %s - %dx%d:\n", pNameStem, nWidth, nHeight);

    for (int y = 0; y < nHeight; ++y)
    {
        const char* row = pBuffer + (y + nOffY) * nTotalWidth * 4 + nOffX * 4;
        for (int x = 0; x < nWidth; ++x)
        {
            const char* pixel = row + x * 4;
            if (mode == LOK_TILEMODE_RGBA)
            {
                ofs.write(pixel, 3); // Skip alpha
            }
            else if (mode == LOK_TILEMODE_BGRA)
            {
                const int alpha = *(pixel + 3);
                char buf[3];
                if (alpha == 0)
                {
                    buf[0] = 0;
                    buf[1] = 0;
                    buf[2] = 0;
                }
                else
                {
                    buf[0] = (*(pixel + 2) * 255 + alpha / 2) / alpha;
                    buf[1] = (*(pixel + 1) * 255 + alpha / 2) / alpha;
                    buf[2] = (*(pixel + 0) * 255 + alpha / 2) / alpha;
                }

                ofs.write(buf, 3);
            }
            if (dumpText)
            {
                int lowResI = (pixel[0] + pixel[1] + pixel[2])/(3*16);
                fprintf(stderr,"%1x", lowResI);
            }
        }
        if (dumpText)
            fprintf(stderr,"\n");
    }
    ofs.close();
}

static void testTile( Document *pDocument, int max_parts,
                      int max_tiles, bool dump )
{
    const int mode = pDocument->getTileMode();

    aTimes.emplace_back("getparts");
    const int nOriginalPart = (pDocument->getDocumentType() == LOK_DOCTYPE_TEXT ? 1 : pDocument->getPart());
    // Writer really has 1 part (the full doc).
    const int nTotalParts = (pDocument->getDocumentType() == LOK_DOCTYPE_TEXT ? 1 : pDocument->getParts());
    const int nParts = (max_parts < 0 ? nTotalParts : std::min(max_parts, nTotalParts));
    aTimes.emplace_back();

    aTimes.emplace_back("get size of parts");
    long nWidth = 0;
    long nHeight = 0;
    for (int n = 0; n < nParts; ++n)
    {
        const int nPart = (nOriginalPart + n) % nTotalParts;
        char* pName = pDocument->getPartName(nPart);
        pDocument->setPart(nPart);
        pDocument->getDocumentSize(&nWidth, &nHeight);
        fprintf (stderr, "  '%s' -> %ld, %ld\n", pName, nWidth, nHeight);
        free (pName);
    }
    aTimes.emplace_back();

    // Use realistic dimensions, similar to the Online client.
    long const nTilePixelWidth = 512;
    long const nTilePixelHeight = 512;
    long const nTileTwipWidth = 3840;
    long const nTileTwipHeight = 3840;

    // Estimate the maximum tiles based on the number of parts requested, if Writer.
    if (pDocument->getDocumentType() == LOK_DOCTYPE_TEXT)
        max_tiles = static_cast<int>(ceil(max_parts * 16128. / nTilePixelHeight) * ceil(static_cast<double>(nWidth) / nTilePixelWidth));
    fprintf(stderr, "Parts to render: %d, Total Parts: %d, Max parts: %d, Max tiles: %d\n", nParts, nTotalParts, max_parts, max_tiles);

    std::vector<unsigned char> vBuffer(nTilePixelWidth * nTilePixelHeight * 4);
    unsigned char* pPixels = vBuffer.data();

    for (int n = 0; n < nParts; ++n)
    {
        const int nPart = (nOriginalPart + n) % nTotalParts;
        char* pName = pDocument->getPartName(nPart);
        pDocument->setPart(nPart);
        pDocument->getDocumentSize(&nWidth, &nHeight);
        fprintf (stderr, "render '%s' -> %ld, %ld\n", pName, nWidth, nHeight);
        free (pName);

        if (dump || pDocument->getDocumentType() != LOK_DOCTYPE_TEXT)
        {
            // whole part; meaningful only for non-writer documents.
            aTimes.emplace_back("render whole part");
            pDocument->paintTile(pPixels, nTilePixelWidth, nTilePixelHeight,
                                 nWidth/2, 2000, 1000, 1000);
            aTimes.emplace_back();
            if (dump)
                dumpTile("tile", nTilePixelWidth, nTilePixelHeight, mode, pPixels);
        }

        { // 1:1
            aTimes.emplace_back("render sub-region at 1:1");
            // Estimate the maximum tiles based on the number of parts requested, if Writer.
            int nMaxTiles = max_tiles;
            int nTiles = 0;
            for (long nY = 0; nY < nHeight - 1; nY += nTilePixelHeight)
            {
                for (long nX = 0; nX < nWidth - 1; nX += nTilePixelWidth)
                {
                    if (nMaxTiles >= 0 && nTiles >= nMaxTiles)
                    {
                        nY = nHeight;
                        break;
                    }
                    pDocument->paintTile(pPixels, nTilePixelWidth, nTilePixelHeight,
                                         nX, nY, nTilePixelWidth, nTilePixelHeight);
                    nTiles++;
                    fprintf (stderr, "   rendered 1:1 tile %d at %ld, %ld\n",
                             nTiles, nX, nY);
                }
            }
            aTimes.emplace_back();
        }

        { // scaled
            aTimes.emplace_back("render sub-regions at scale");
            int nMaxTiles = max_tiles;
            if (pDocument->getDocumentType() == LOK_DOCTYPE_TEXT)
                nMaxTiles = static_cast<int>(ceil(max_parts * 16128. / nTileTwipHeight) * ceil(static_cast<double>(nWidth) / nTileTwipWidth));
            int nTiles = 0;
            for (long nY = 0; nY < nHeight - 1; nY += nTileTwipHeight)
            {
                for (long nX = 0; nX < nWidth - 1; nX += nTileTwipWidth)
                {
                    if (nMaxTiles >= 0 && nTiles >= nMaxTiles)
                    {
                        nY = nHeight;
                        break;
                    }
                    pDocument->paintTile(pPixels, nTilePixelWidth, nTilePixelHeight,
                                         nX, nY, nTileTwipWidth, nTileTwipHeight);
                    nTiles++;
                    fprintf (stderr, "   rendered scaled tile %d at %ld, %ld\n",
                             nTiles, nX, nY);
                }
            }
            aTimes.emplace_back();
        }
    }
}

static uint32_t fade(uint32_t col)
{
    uint8_t a = (col >> 24) & 0xff;
    uint8_t b = (col >> 16) & 0xff;
    uint8_t g = (col >>  8) & 0xff;
    uint8_t r = (col >>  0) & 0xff;
    uint8_t grey = (r+g+b)/6;
    return (a<<24) + (grey<<16) + (grey<<8) + grey;
}

static bool sloppyEqual(uint32_t pixA, uint32_t pixB)
{
    uint8_t a[4], b[4];

    a[0] = (pixA >> 24) & 0xff;
    a[1] = (pixA >> 16) & 0xff;
    a[2] = (pixA >>  8) & 0xff;
    a[3] = (pixA >>  0) & 0xff;

    b[0] = (pixB >> 24) & 0xff;
    b[1] = (pixB >> 16) & 0xff;
    b[2] = (pixB >>  8) & 0xff;
    b[3] = (pixB >>  0) & 0xff;

    for (int i = 0; i < 4; ++i)
    {
        int delta = a[i];
        delta -= b[i];
        // tolerate small differences
        if (delta < -4 || delta > 4)
            return false;
    }
    return true;
}

// Count and build a picture of any differences into rDiff
static int diffTiles( const std::vector<unsigned char> &vBase,
                       long nBaseRowPixelWidth,
                       const std::vector<unsigned char> &vCompare,
                       long nCompareRowPixelWidth,
                       long nTilePixelHeight,
                       long nPosX, long nPosY,
                       std::vector<unsigned char> &rDiff )
{
    int nDifferent = 0;
    const uint32_t *pBase = reinterpret_cast<const uint32_t *>(vBase.data());
    const uint32_t *pCompare = reinterpret_cast<const uint32_t *>(vCompare.data());
    uint32_t *pDiff = reinterpret_cast<uint32_t *>(rDiff.data());
    long left = 0, mid = nCompareRowPixelWidth, right = nCompareRowPixelWidth*2;
    for (long y = 0; y < nTilePixelHeight; ++y)
    {
        long nBaseOffset = nBaseRowPixelWidth * (y + nPosY) + nPosX * nCompareRowPixelWidth;
        long nCompareOffset = nCompareRowPixelWidth * y;
        long nDiffRowStart = nCompareOffset * 3;
        for (long x = 0; x < nCompareRowPixelWidth; ++x)
        {
            pDiff[nDiffRowStart + left  + x] = pBase[nBaseOffset + x];
            pDiff[nDiffRowStart + mid   + x] = pCompare[nCompareOffset + x];
            pDiff[nDiffRowStart + right + x] = fade(pBase[nBaseOffset + x]);
            if (!sloppyEqual(pBase[nBaseOffset + x], pCompare[nCompareOffset + x]))
            {
                pDiff[nDiffRowStart + right + x] = 0xffff00ff;
                if (!nDifferent)
                    fprintf (stderr, "First mismatching pixel at %ld (pixels) into row %ld\n", x, y);
                nDifferent++;
            }
        }
    }
    return nDifferent;
}

static std::vector<unsigned char> paintTile( Document *pDocument,
                                             long nX, long nY,
                                             long const nTilePixelWidth,
                                             long const nTilePixelHeight,
                                             long const nTileTwipWidth,
                                             long const nTileTwipHeight )
{
//    long e = 0; // tweak if we suspect an overlap / visibility issue.
//    pDocument->setClientVisibleArea( nX - e, nY - e, nTileTwipWidth + e, nTileTwipHeight + e );
    std::vector<unsigned char> vData( nTilePixelWidth * nTilePixelHeight * 4 );
    pDocument->paintTile( vData.data(), nTilePixelWidth, nTilePixelHeight,
                          nX, nY, nTileTwipWidth, nTileTwipHeight );
    return vData;
}

static int testJoinsAt( Document *pDocument, long nX, long nY,
                        long const nTilePixelSize,
                        long const nTileTwipSize )
{
    const int mode = pDocument->getTileMode();

    long const nTilePixelWidth = nTilePixelSize;
    long const nTilePixelHeight = nTilePixelSize;
    long const nTileTwipWidth = nTileTwipSize;
    long const nTileTwipHeight = nTileTwipSize;

    long initPosX = nX * nTileTwipWidth, initPosY = nY * nTileTwipHeight;

    // Calc has to do significant work on changing zoom ...
    pDocument->setClientZoom( nTilePixelWidth, nTilePixelHeight,
                              nTileTwipWidth, nTileTwipHeight );

    // Unfortunately without getting this nothing renders ...
    std::stringstream aForceHeaders;
    aForceHeaders << ".uno:ViewRowColumnHeaders?x=" << initPosX << "&y=" << initPosY <<
        "&width=" << (nTileTwipWidth * 2) << "&height=" << (nTileTwipHeight * 2);
    std::string cmd = aForceHeaders.str();
    char* pJSON = pDocument->getCommandValues(cmd.c_str());
    fprintf(stderr, "command: '%s' values '%s'\n", cmd.c_str(), pJSON);
    free(pJSON);

    // Get a base image 4x the size
    std::vector<unsigned char> vBase(
        paintTile(pDocument, initPosX, initPosY,
                  nTilePixelWidth * 2, nTilePixelHeight * 2,
                  nTileTwipWidth * 2, nTileTwipHeight * 2));

    const struct {
        long X;
        long Y;
    } aCompare[] = {
        { 0, 0 },
        { 1, 0 },
        { 0, 1 },
        { 1, 1 }
    };

    int nDifferences = 0;
    // Compare each of the 4x tiles with a sub-tile of the larger image
    for( auto &rPos : aCompare )
    {
        std::vector<unsigned char> vCompare(
            paintTile(pDocument,
                      initPosX + rPos.X * nTileTwipWidth,
                      initPosY + rPos.Y * nTileTwipHeight,
                      nTilePixelWidth, nTilePixelHeight,
                      nTileTwipWidth, nTileTwipHeight));

        std::vector<unsigned char> vDiff( nTilePixelWidth * 3 * nTilePixelHeight * 4 );
        int nDiffs = diffTiles( vBase, nTilePixelWidth * 2,
                                vCompare, nTilePixelWidth,
                                nTilePixelHeight,
                                rPos.X, rPos.Y * nTilePixelHeight,
                                vDiff );
        if ( nDiffs > 0 )
        {
            fprintf( stderr, "  %d differences in sub-tile pixel mismatch at %ld, %ld at offset %ld, %ld (twips) size %ld\n",
                     nDiffs, rPos.X, rPos.Y, initPosX, initPosY,
                     nTileTwipWidth);
            dumpTile("_base", nTilePixelWidth * 2, nTilePixelHeight * 2,
                     mode, vBase.data());
/*            dumpTile("_sub", nTilePixelWidth, nTilePixelHeight,
                     mode, vBase.data(),
                     rPos.X*nTilePixelWidth, rPos.Y*nTilePixelHeight,
                     nTilePixelWidth * 2);
            dumpTile("_compare", nTilePixelWidth, nTilePixelHeight,
            mode, vCompare.data());*/
            dumpTile("_diff", nTilePixelWidth * 3, nTilePixelHeight, mode, vDiff.data());
        }
        nDifferences += nDiffs;
    }

    return nDifferences;
}

// Check that our tiles join nicely ...
static int testJoin( Document *pDocument)
{
    // Ignore parts - just the first for now ...
    long nWidth = 0, nHeight = 0;
    pDocument->getDocumentSize(&nWidth, &nHeight);
    fprintf (stderr, "Width is %ld, %ld (twips)\n", nWidth, nHeight);

    // Use realistic dimensions, similar to the Online client.
    long const nTilePixelSize = 256;
    long const nTileTwipSize = 3840;
    double fZooms[] = {
        0.5,
        0.6, 0.7, 0.85,
        1.0,
        1.2, 1.5, 1.75,
        2.0
    };
    long nFails = 0;
    std::stringstream results;

    for( auto z : fZooms )
    {
        long nBad = 0;
        long nDifferences = 0;
        for( long y = 0; y < 8; ++y )
        {
            for( long x = 0; x < 8; ++x )
            {
                int nDiffs = testJoinsAt( pDocument, x, y, nTilePixelSize, nTileTwipSize * z );
                if (nDiffs)
                    nBad++;
                nDifferences += nDiffs;
            }
        }
        if (nBad > 0)
            results << "\tZoom " << z << " bad tiles: " << nBad << " with " << nDifferences << " mismatching pixels\n";
        nFails += nBad;
    }

    if (nFails > 0)
        fprintf( stderr, "Failed %ld joins\n", nFails );
    else
        fprintf( stderr, "All joins compared correctly\n" );

    fprintf(stderr, "%s\n", results.str().c_str());

    return nFails;
}

static std::atomic<bool> bDialogRendered(false);
static std::atomic<int> nDialogId(-1);

static void kitCallback(int nType, const char* pPayload, void* pData)
{
    Document *pDocument = static_cast<Document *>(pData);

    if (nType != LOK_CALLBACK_WINDOW)
        return;

    std::stringstream aStream(pPayload);
    boost::property_tree::ptree aRoot;
    boost::property_tree::read_json(aStream, aRoot);
    nDialogId = aRoot.get<unsigned>("id");
    const std::string aAction = aRoot.get<std::string>("action");

    if (aAction != "created")
        return;

    const std::string aType = aRoot.get<std::string>("type");
    const std::string aSize = aRoot.get<std::string>("size");
    int nWidth = atoi(aSize.c_str());
    int nHeight = 400;
    const char *pComma = strstr(aSize.c_str(), ", ");
    if (pComma)
        nHeight = atoi(pComma + 2);
    std::cerr << "Size " << aSize << " is " << nWidth << ", " << nHeight << "\n";

    if (aType != "dialog")
        return;

    aTimes.emplace_back(); // complete wait for dialog

    unsigned char *pBuffer = new unsigned char[nWidth * nHeight * 4];

    aTimes.emplace_back("render dialog");
    pDocument->paintWindow(nDialogId, pBuffer, 0, 0, nWidth, nHeight);
    dumpTile("dialog", nWidth, nHeight, pDocument->getTileMode(), pBuffer);
    aTimes.emplace_back();

    delete[] pBuffer;

    bDialogRendered = true;
}

static void testDialog( Document *pDocument, const char *uno_cmd )
{
    int view = pDocument->createView();
    pDocument->setView(view);
    pDocument->registerCallback(kitCallback, pDocument);

    aTimes.emplace_back("open dialog");
    pDocument->postUnoCommand(uno_cmd, nullptr, true);
    aTimes.emplace_back();

    aTimes.emplace_back("wait for dialog");
    while (!bDialogRendered)
    {
        usleep (1000);
    }

    aTimes.emplace_back("post close dialog");
    pDocument->postWindow(nDialogId, LOK_WINDOW_CLOSE);
    aTimes.emplace_back();

    pDocument->destroyView(view);
}

static void documentCallback(const int type, const char* p, void*)
{
    std::cerr << "Document callback " << type << ": " << (p ? p : "(null)") << "\n";
}

// Avoid excessive dbgutil churn.
static void ignoreCallback(const int /*type*/, const char* /*p*/, void* /*data*/)
{
}

int main( int argc, char* argv[] )
{
    int arg = 2;
    origin = getTimeNow();

#ifndef IOS
    // avoid X oddness etc.
    unsetenv("DISPLAY");

    if( argc < 4 ||
        ( argc > 1 && ( !strcmp( argv[1], "--help" ) || !strcmp( argv[1], "-h" ) ) ) )
        return help();

    if ( argv[1][0] != '/' )
    {
        fprintf(stderr, "Absolute path required to libreoffice install\n");
        return 1;
    }

    const char *doc_url = argv[arg++];
    const char *mode = argv[arg++];

    bool pre_init = false;
    if (!strcmp(mode, "--preinit"))
    {
        pre_init = true;
        mode = argv[arg++];
    }

    std::string user_url("file:///");
    user_url.append(argv[1]);
    user_url.append("../user");

    if (pre_init)
    {
        aTimes.emplace_back("pre-initialization");
        setenv("LOK_ALLOWLIST_LANGUAGES", "en_US", 0);
        // coverity[tainted_string] - build time test tool
        lok_preinit(argv[1], user_url.c_str());
        aTimes.emplace_back();
    }
    const char *install_path = argv[1];
    const char *user_profile = user_url.c_str();
#else
    const char *install_path = nullptr;
    const char *user_profile = nullptr;
    const char *doc_url = strdup([[[[[NSBundle mainBundle] bundleURL] absoluteString] stringByAppendingString:@"/test.odt"] UTF8String]);
    const char *mode = "--tile";
#endif

    aTimes.emplace_back("initialization");
    // coverity[tainted_string] - build time test tool
    std::unique_ptr<Office> pOffice( lok_cpp_init(install_path, user_profile) );
    if (pOffice == nullptr)
    {
        fprintf(stderr, "Failed to initialize Office from %s\n", argv[1]);
        return 1;
    }
    aTimes.emplace_back();
    pOffice->registerCallback(ignoreCallback, nullptr);

    std::unique_ptr<Document> pDocument;

    pOffice->setOptionalFeatures(LOK_FEATURE_NO_TILED_ANNOTATIONS);

    aTimes.emplace_back("load document");
    if (doc_url != nullptr)
        pDocument.reset(pOffice->documentLoad(doc_url));
    aTimes.emplace_back();

    if (pDocument)
    {
        pDocument->initializeForRendering("{\".uno:Author\":{\"type\":\"string\",\"value\":\"Local Host #0\"}}");
        pDocument->registerCallback(documentCallback, nullptr);
        if (!strcmp(mode, "--tile"))
        {
            const int max_parts = (argc > arg ? atoi(argv[arg++]) : -1);
            int max_tiles = (argc > arg ? atoi(argv[arg++]) : -1);
            const bool dump = true;

            testTile (pDocument.get(), max_parts, max_tiles, dump);
        }
        else if (!strcmp(mode, "--join"))
        {
            return testJoin (pDocument.get());
        }
        else if (!strcmp (mode, "--dialog"))
        {
            const char *uno_cmd = argc > arg ? argv[arg++] : nullptr;
            if (!uno_cmd)
            {
                switch (pDocument->getDocumentType())
                {
                case LOK_DOCTYPE_SPREADSHEET:
                    uno_cmd = ".uno:FormatCellDialog";
                    break;
                case LOK_DOCTYPE_TEXT:
                case LOK_DOCTYPE_PRESENTATION:
                case LOK_DOCTYPE_DRAWING:
                case LOK_DOCTYPE_OTHER:
                    return help("missing argument to --dialog and no default");
                }
            }
            testDialog (pDocument.get(), uno_cmd);
        } else
            return help ("unknown parameter");
    }

#ifdef IOS
    Application::Quit();
#endif
    aTimes.emplace_back("destroy document");
    pDocument.reset();
    aTimes.emplace_back();

    pOffice.reset();

    double nTotal = 0.0;
    fprintf (stderr, "profile run:\n");
    for (size_t i = 0; i < aTimes.size() - 1; i++)
    {
        const double nDelta = aTimes[i+1].mfTime - aTimes[i].mfTime;
        fprintf (stderr, "  %s - %2.4f(ms)\n", aTimes[i].mpName, nDelta * 1000.0);
        if (aTimes[i+1].mpName == nullptr)
            i++; // skip it.
        nTotal += nDelta;
    }
    fprintf (stderr, "Total: %2.4f(s)\n", nTotal);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
