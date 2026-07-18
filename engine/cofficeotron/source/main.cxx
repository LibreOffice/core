/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * cofficeotron - a C++ port of the Java(tm) Office-o-tron
 *
 * Copyright (c) 2009-2010 Griffin Brown Digital Publishing Ltd.
 * Copyright (c) 2010 Novell Inc.
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>
#include <cstring>
#include <string>
#include <vector>

#include "odfsession.hxx"
#include "ooxmlsession.hxx"
#include "report.hxx"
#include "schemacache.hxx"
#include "xmlutil.hxx"
#include "zipfile.hxx"

int main(int argc, char** argv)
{
    bool onlyErrors = false;
    OdfSessionOptions odfOptions;
    std::string schemaDirOverride;
    std::vector<std::string> files;

    for (int i = 1; i < argc; ++i)
    {
        std::string argument = argv[i];
        if (argument == "--errors-only")
        {
            onlyErrors = true;
        }
        else if (argument == "--check-ids")
        {
            odfOptions.checkIds = true;
        }
        else if (argument == "--force-is")
        {
            odfOptions.forceIs = true;
        }
        else if (argument == "--schema-dir" && i + 1 < argc)
        {
            schemaDirOverride = argv[++i];
        }
        else if (argument == "--help")
        {
            printf("arguments: [--errors-only] [--check-ids] [--force-is]"
                        " [--schema-dir DIR] file1 ...\n");
            return 0;
        }
        else if (argument == "--version")
        {
            // A plain dotted version, so configure scripts can compare it
            // numerically. The Java officeotron ended at 0.8.8.
            printf("1.0.0\n");
            return 0;
        }
        else
        {
            files.push_back(argument);
        }
    }

    initValidation();

    std::string schemaDirectory = locateSchemaDirectory(schemaDirOverride);
    if (schemaDirectory.empty())
    {
        fprintf(stderr, "cofficeotron: cannot find the schema directory; use --schema-dir"
                             " or set COFFICEOTRON_SCHEMA_DIR\n");
        return 2;
    }
    SchemaCache schemas(schemaDirectory);

    const bool showInfos = !onlyErrors;
    int exitCode = 0;

    for (const std::string& file : files)
    {
        StdioValidationReport report(showInfos);

        ZipArchive zip;
        std::string zipError;
        if (!zip.open(file, zipError))
        {
            fprintf(stderr, "cofficeotron: can't access file: %s (%s)\n", file.c_str(),
                         zipError.c_str());
            exitCode = 2;
            continue;
        }

        report.addComment("Inspecting ZIP ...");
        report.incIndent();
        if (zip.localHeaderCount() != zip.centralRecordCount())
        {
            report.addComment("WARN", "Mismatch between local header and central record"
                                      " (weakened ZIP resilience)");
        }
        else
        {
            report.addComment(std::to_string(zip.centralRecordCount())
                              + " central records found");
        }
        report.decIndent();

        if (zip.find("META-INF/manifest.xml"))
        {
            runOdfValidation(zip, report, schemas, odfOptions);
        }
        else if (zip.find("_rels/.rels"))
        {
            runOOXMLValidation(zip, report, schemas);
        }
        else
        {
            report.addComment("FATAL",
                              "The file is neither an ODF nor an OOXML package");
            exitCode = 2;
        }
    }

    return exitCode;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
