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

#include <vector>
#include <osl/time.h>

#define LOK_USE_UNSTABLE_API

#include <LibreOfficeKit/LibreOfficeKitInit.h>
#include <LibreOfficeKit/LibreOfficeKit.hxx>

using namespace lok;

static int help()
{
    fprintf( stderr, "Usage: tilebench <absolute-path-to-libreoffice-install> [path to document]\n" );
    fprintf( stderr, "renders a selection of small tiles from the document, checksums them and times the process\n" );
    return 1;
}

static double getTimeNow()
{
    TimeValue aValue;
    osl_getSystemTime(&aValue);
    return (double)aValue.Seconds +
           (double)aValue.Nanosec / (1000*1000*1000);
}

int main( int argc, char* argv[] )
{
    struct TimeRecord {
        const char *mpName;
        double mfTime;

        TimeRecord() : mpName(nullptr), mfTime(getTimeNow()) { }
        explicit TimeRecord(const char *pName) :
                       mpName(pName ), mfTime(getTimeNow()) { }
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

    aTimes.push_back(TimeRecord("initialization"));
    // coverity[tainted_string] - build time test tool
    Office *pOffice = lok_cpp_init(argv[1]);
    aTimes.push_back(TimeRecord());

    if (argv[2] != nullptr)
    {
        aTimes.push_back(TimeRecord("load document"));
        Document *pDocument(pOffice->documentLoad(argv[2]));
        aTimes.push_back(TimeRecord());

        aTimes.push_back(TimeRecord("getparts"));
        int nParts = pDocument->getParts();
        aTimes.push_back(TimeRecord());

        aTimes.push_back(TimeRecord("get size of parts"));
        for (int nPart = 0; nPart < nParts; nPart++)
        {
            char* pName = pDocument->getPartName(nPart);
            pDocument->setPart(nPart);
            long nWidth = 0, nHeight = 0;
            pDocument->getDocumentSize(&nWidth, &nHeight);
            fprintf (stderr, "  '%s' -> %ld, %ld\n", pName, nWidth, nHeight);
            free (pName);
        }
        aTimes.push_back(TimeRecord());

        unsigned char pPixels[256*256*4];
        for (int nPart = 0; nPart < nParts; nPart++)
        {
            {
                char* pName = pDocument->getPartName(nPart);
                fprintf (stderr, "render '%s'\n", pName);
                free (pName);
            }
            pDocument->setPart(nPart);
            long nWidth = 0, nHeight = 0;
            pDocument->getDocumentSize(&nWidth, &nHeight);

            { // whole document
                aTimes.push_back(TimeRecord("render whole document"));
                pDocument->paintTile(pPixels, 256, 256,
                                     0, 0, nWidth, nHeight); // not square
                aTimes.push_back(TimeRecord());
            }

            { // 1:1
                aTimes.push_back(TimeRecord("render sub-region at 1:1"));
                int nTiles = 0;
                int nSplit = nWidth / 4;
                for (int nX = 0; nX < 4; nX++)
                {
                    for (int nY = 0; nY < nHeight / nSplit; nY++)
                    {
                        int nTilePosX = nX * nSplit;
                        int nTilePosY = nY * nSplit;
                        pDocument->paintTile(pPixels, 256, 256,
                                             nTilePosX, nTilePosY, 256, 256);
                        nTiles++;
                        fprintf (stderr, "   rendered tile %d at %d, %d\n",
                                 nTiles, nTilePosX, nTilePosY);
                    }
                }
                aTimes.push_back(TimeRecord());
            }

            { // scaled
                aTimes.push_back(TimeRecord("render sub-regions at scale"));
                int nTiles = 0;
                int nSplit = nWidth / 4;
                for (int nX = 0; nX < 4; nX++)
                {
                    for (int nY = 0; nY < nHeight / nSplit; nY++)
                    {
                        int nTilePosX = nX * nSplit;
                        int nTilePosY = nY * nSplit;
                        pDocument->paintTile(pPixels, 256, 256,
                                             nTilePosX, nTilePosY, nSplit, nSplit);
                        nTiles++;
                        fprintf (stderr, "   rendered tile %d at %d, %d\n",
                                 nTiles, nTilePosX, nTilePosY);
                    }
                }
                aTimes.push_back(TimeRecord());
            }
        }

        aTimes.push_back(TimeRecord("destroy document"));
        delete pDocument;
        aTimes.push_back(TimeRecord());
    }

    delete pOffice;

    double nTotal = 0.0;
    fprintf (stderr, "profile run:\n");
    for (size_t i = 0; i < aTimes.size() - 1; i++)
    {
        double nDelta = aTimes[i+1].mfTime - aTimes[i].mfTime;
        fprintf (stderr, "  %s - %2.4f(ms)\n", aTimes[i].mpName, nDelta * 1000.0);
        if (aTimes[i+1].mpName == nullptr)
            i++; // skip it.
        nTotal += nDelta;
    }
    fprintf (stderr, "Total: %2.4f(s)\n", nTotal);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
