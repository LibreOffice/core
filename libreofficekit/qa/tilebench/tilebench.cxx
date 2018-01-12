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
#include <fstream>
#include <osl/time.h>

#include <LibreOfficeKit/LibreOfficeKitEnums.h>
#include <LibreOfficeKit/LibreOfficeKitInit.h>
#include <LibreOfficeKit/LibreOfficeKit.hxx>

using namespace lok;

static int help()
{
    fprintf( stderr, "Usage: tilebench <absolute-path-to-libreoffice-install> [path to document] [max parts|-1] [max tiles|-1]\n" );
    fprintf( stderr, "renders a selection of small tiles from the document, checksums them and times the process\n" );
    return 1;
}

static double getTimeNow()
{
    TimeValue aValue;
    osl_getSystemTime(&aValue);
    return static_cast<double>(aValue.Seconds) +
           static_cast<double>(aValue.Nanosec) / (1000*1000*1000);
}

/// Dump an array of RGBA or BGRA to an RGB PPM file.
static void dumpTile(const int nWidth, const int nHeight, const int mode, const char* pBuffer)
{
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

int main( int argc, char* argv[] )
{
    static const double origin = getTimeNow();
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
    std::vector< TimeRecord > aTimes;
    if( argc < 2 ||
        ( argc > 1 && ( !strcmp( argv[1], "--help" ) || !strcmp( argv[1], "-h" ) ) ) )
        return help();

    if ( argv[1][0] != '/' )
    {
        fprintf(stderr, "Absolute path required to libreoffice install\n");
        return 1;
    }

    // Use realistic dimensions, similar to the Online client.
    long nTilePixelWidth = 512;
    long nTilePixelHeight = 512;
    long nTileTwipWidth = 3840;
    long nTileTwipHeight = 3840;

    aTimes.emplace_back("initialization");
    // coverity[tainted_string] - build time test tool
    Office *pOffice = lok_cpp_init(argv[1]);
    aTimes.emplace_back();

    const int max_parts = (argc > 3 ? atoi(argv[3]) : -1);
    int max_tiles = (argc > 4 ? atoi(argv[4]) : -1);
    const bool dump = true;

    if (argv[2] != nullptr)
    {
        aTimes.emplace_back("load document");
        Document *pDocument(pOffice->documentLoad(argv[2]));
        aTimes.emplace_back();
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

        // Estimate the maximum tiles based on the number of parts requested, if Writer.
        if (pDocument->getDocumentType() == LOK_DOCTYPE_TEXT)
            max_tiles = static_cast<int>(ceil(max_parts * 16128. / nTilePixelHeight)) * ceil(static_cast<double>(nWidth) / nTilePixelWidth);
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
                    dumpTile(nTilePixelWidth, nTilePixelHeight, mode, reinterpret_cast<char*>(pPixels));
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
                    nMaxTiles = static_cast<int>(ceil(max_parts * 16128. / nTileTwipHeight)) * ceil(static_cast<double>(nWidth) / nTileTwipWidth);
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

        aTimes.emplace_back("destroy document");
        delete pDocument;
        aTimes.emplace_back();
    }

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
