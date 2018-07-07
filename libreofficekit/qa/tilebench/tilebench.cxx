/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <cmath>

#include <vector>
#include <atomic>
#include <iostream>
#include <fstream>
#include <osl/time.h>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKitInit.h>
#include <LibreOfficeKit/LibreOfficeKit.hxx>

#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>

using namespace lok;

static int help( const char *error = nullptr )
{
    if (error)
        fprintf (stderr, "Error: %s\n\n", error);
    fprintf( stderr, "Usage: tilebench <absolute-path-to-libreoffice-install> [path to document] [--preinit] <options>\n");
    fprintf( stderr, "\trenders a selection of small tiles from the document, checksums them and times the process based on options:\n" );
    fprintf( stderr, "\t--tile\t[max parts|-1] [max tiles|-1]\n" );
    fprintf( stderr, "\t--dialog\t<.uno:Command>\n" );
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
static std::vector< TimeRecord > aTimes;

/// Dump an array of RGBA or BGRA to an RGB PPM file.
static void dumpTile(const int nWidth, const int nHeight, const int mode, const unsigned char* pBufferU)
{
    auto pBuffer = reinterpret_cast<const char *>(pBufferU);
    std::ofstream ofs("/tmp/dump_tile.ppm");
    ofs << "P6\n"
        << nWidth << " "
        << nHeight << "\n"
        << 255 << "\n" ;

    for (int y = 0; y < nHeight; ++y)
    {
        const char* row = pBuffer + y * nWidth * 4;
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
        }
    }
}

void testTile( Document *pDocument, int max_parts,
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
    long nTilePixelWidth = 512;
    long nTilePixelHeight = 512;
    long nTileTwipWidth = 3840;
    long nTileTwipHeight = 3840;

    // Estimate the maximum tiles based on the number of parts requested, if Writer.
    if (pDocument->getDocumentType() == LOK_DOCTYPE_TEXT)
        max_tiles = static_cast<int>(ceil(max_parts * 16128. / nTilePixelHeight) * ceil(static_cast<double>(nWidth) / nTilePixelWidth));
    fprintf(stderr, "Parts to render: %d, Total Parts: %d, Max parts: %d, Max tiles: %d\n", nParts, nTotalParts, max_parts, max_tiles);

    std::vector<unsigned char> vBuffer(nTilePixelWidth * nTilePixelHeight * 4);
    unsigned char* pPixels = &vBuffer[0];

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
                                 nWidth/2, 2000, 1000, 1000); // not square
            aTimes.emplace_back();
            if (dump)
                dumpTile(nTilePixelWidth, nTilePixelHeight, mode, pPixels);
        }

        { // 1:1
            aTimes.emplace_back("render sub-region at 1:1");
            // Estimate the maximum tiles based on the number of parts requested, if Writer.
            int nMaxTiles = max_tiles;
            int nTiles = 0;
            for (int nY = 0; nY < nHeight - 1; nY += nTilePixelHeight)
            {
                for (int nX = 0; nX < nWidth - 1; nX += nTilePixelWidth)
                {
                    if (nMaxTiles >= 0 && nTiles >= nMaxTiles)
                    {
                        nY = nHeight;
                        break;
                    }

                    pDocument->paintTile(pPixels, nTilePixelWidth, nTilePixelHeight,
                                         nX, nY, nTilePixelWidth, nTilePixelHeight);
                    nTiles++;
                    fprintf (stderr, "   rendered 1:1 tile %d at %d, %d\n",
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
            for (int nY = 0; nY < nHeight - 1; nY += nTileTwipHeight)
            {
                for (int nX = 0; nX < nWidth - 1; nX += nTileTwipWidth)
                {
                    if (nMaxTiles >= 0 && nTiles >= nMaxTiles)
                    {
                        nY = nHeight;
                        break;
                    }

                    pDocument->paintTile(pPixels, nTilePixelWidth, nTilePixelHeight,
                                         nX, nY, nTileTwipWidth, nTileTwipHeight);
                    nTiles++;
                    fprintf (stderr, "   rendered scaled tile %d at %d, %d\n",
                             nTiles, nX, nY);
                }
            }
            aTimes.emplace_back();
        }
    }
}

static std::atomic<bool> bDialogRendered(false);
static std::atomic<int> nDialogId(-1);

void kitCallback(int nType, const char* pPayload, void* pData)
{
    Document *pDocument = static_cast<Document *>(pData);

    if (nType != LOK_CALLBACK_WINDOW)
        return;

    std::stringstream aStream(pPayload);
    boost::property_tree::ptree aRoot;
    boost::property_tree::read_json(aStream, aRoot);
    nDialogId = aRoot.get<unsigned>("id");
    const std::string aAction = aRoot.get<std::string>("action");

    if (aAction == "created")
    {
        const std::string aType = aRoot.get<std::string>("type");
        const std::string aSize = aRoot.get<std::string>("size");
        int nWidth = atoi(aSize.c_str());
        int nHeight = 400;
        const char *pComma = strstr(aSize.c_str(), ", ");
        if (pComma)
            nHeight = atoi(pComma + 2);
        std::cerr << "Size " << aSize << " is " << nWidth << ", " << nHeight << "\n";

        if (aType == "dialog")
        {
            aTimes.emplace_back(); // complete wait for dialog

            unsigned char *pBuffer = new unsigned char[nWidth * nHeight * 4];

            aTimes.emplace_back("render dialog");
            pDocument->paintWindow(nDialogId, pBuffer, 0, 0, nWidth, nHeight);
            dumpTile(nWidth, nHeight, pDocument->getTileMode(), pBuffer);
            aTimes.emplace_back();

            delete[] pBuffer;

            bDialogRendered = true;
        }
    }
}

void testDialog( Document *pDocument, const char *uno_cmd )
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

int main( int argc, char* argv[] )
{
    int arg;

    // avoid X oddness etc.
    unsetenv("DISPLAY");

    origin = getTimeNow();
    if( argc < 4 ||
        ( argc > 1 && ( !strcmp( argv[1], "--help" ) || !strcmp( argv[1], "-h" ) ) ) )
        return help();

    if ( argv[1][0] != '/' )
    {
        fprintf(stderr, "Absolute path required to libreoffice install\n");
        return 1;
    }

    arg = 2;
    const char *doc_url = argv[arg++];
    const char *mode = argv[arg++];

    bool pre_init = false;
    if (!strcmp(mode, "--preinit"))
    {
        pre_init = true;
        mode = argv[arg++];
    }

    char user_url[8046];;
    strcpy(user_url, "file:///");
    strcat(user_url, argv[1]);
    strcat(user_url, "../user");

    if (pre_init)
    {
        aTimes.emplace_back("pre-initialization");
        setenv("LOK_WHITELIST_LANGUAGES", "en_US", 0);
        // coverity[tainted_string] - build time test tool
        lok_preinit(argv[1], user_url);
        aTimes.emplace_back();
    }

    aTimes.emplace_back("initialization");
    // coverity[tainted_string] - build time test tool
    Office *pOffice = lok_cpp_init(argv[1], user_url);
    if (pOffice == nullptr)
    {
        fprintf(stderr, "Failed to initialize Office from %s\n", argv[1]);
        return 1;
    }
    aTimes.emplace_back();

    Document *pDocument = nullptr;

    aTimes.emplace_back("load document");
    if (doc_url != nullptr)
        pDocument = pOffice->documentLoad(doc_url);
    aTimes.emplace_back();

    if (pDocument)
    {
        if (!strcmp(mode, "--tile"))
        {
            const int max_parts = (argc > arg ? atoi(argv[arg++]) : -1);
            int max_tiles = (argc > arg ? atoi(argv[arg++]) : -1);
            const bool dump = true;

            testTile (pDocument, max_parts, max_tiles, dump);
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
            testDialog (pDocument, uno_cmd);
        } else
            return help ("unknown parameter");
    }

    aTimes.emplace_back("destroy document");
    delete pDocument;
    aTimes.emplace_back();

    delete pOffice;

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
