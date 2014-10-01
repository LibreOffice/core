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

        TimeRecord() : mpName(NULL), mfTime(getTimeNow()) { }
        explicit TimeRecord(const char *pName) :
                       mpName(pName ), mfTime(getTimeNow()) { }
        explicit TimeRecord(const TimeRecord *pSrc) :
                       mpName(pSrc->mpName), mfTime(pSrc->mfTime) { }
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
    Office *pOffice = lok_cpp_init(argv[1]);
    aTimes.push_back(TimeRecord());

    if (argv[2] != NULL)
    {
        aTimes.push_back(TimeRecord("load document"));
        Document *pDocument(pOffice->documentLoad(argv[2]));
        aTimes.push_back(TimeRecord());

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
        if (aTimes[i+1].mpName == NULL)
            i++; // skip it.
        nTotal += nDelta;
    }
    fprintf (stderr, "Total: %2.4f(s)\n", nTotal);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
