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


#include "idlc/idlc.hxx"
#include "sal/main.h"

#include <string.h>

using namespace ::rtl;

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    std::vector< std::string > args;
    for (int i = 1; i < argc; i++)
    {
        if (!Options::checkArgument (args, argv[i], strlen(argv[i])))
            return (1);
    }

    Options options(argv[0]);
    try
    {
        if (!options.initOptions(args))
           return (0);
    }
    catch(const IllegalArgument& e)
    {
        fprintf(stderr, "Illegal argument: %s\n%s",
            e.m_message.getStr(),
            options.prepareVersion().getStr());
        return (99);
    }

    setIdlc(&options);

    sal_Int32 nErrors = 0;
    if (options.readStdin()) {
        if ( !options.quiet() )
            fprintf(
                stdout, "%s: Compiling stdin\n",
                options.getProgramName().getStr());
        nErrors = compileFile(0);
        if ( ( idlc()->getWarningCount() > 0 ) && !options.quiet() ) {
            fprintf(
                stdout, "%s: detected %lu warnings compiling stdin\n",
                options.getProgramName().getStr(),
                sal::static_int_cast< unsigned long >(
                    idlc()->getWarningCount()));
        }
        OString outputUrl;
        if (options.isValid("-O")) {
            outputUrl = convertToFileUrl(options.getOption("-O"));
            if (outputUrl[outputUrl.getLength() - 1] != '/') {
                outputUrl += "/";
            }
            outputUrl += "stdin.urd";
        } else {
            outputUrl = convertToFileUrl("stdin.urd");
        }
        if (nErrors > 0) {
            removeIfExists(outputUrl);
        } else {
            nErrors = produceFile(outputUrl, 0);
        }
        idlc()->reset();
    }
    StringVector const & files = options.getInputFiles();
    if ( options.verbose() )
    {
        fprintf( stdout, "%s: compiling %i source files ... \n",
            options.getProgramName().getStr(), (int)files.size() );
        fflush( stdout );
    }
    for (StringVector::const_iterator i(files.begin());
         i != files.end() && nErrors == 0; ++i)
    {
        OString sysFileName( convertToAbsoluteSystemPath(*i) );

        if ( !options.quiet() )
            fprintf(stdout, "Compiling: %s\n",
                (*i).getStr());
        nErrors = compileFile(&sysFileName);

        if ( idlc()->getWarningCount() && !options.quiet() )
            fprintf(stdout, "%s: detected %lu warnings compiling file '%s'\n",
                    options.getProgramName().getStr(),
                    sal::static_int_cast< unsigned long >(
                        idlc()->getWarningCount()),
                    (*i).getStr());

        // prepare output file name
        OString const strippedFileName(
                sysFileName.copy(sysFileName.lastIndexOf(SEPARATOR) + 1));
        OString outputFile;
        if ( options.isValid("-O") )
        {
            outputFile = (options.getOption("-O"));
            if ('/' != outputFile.getStr()[outputFile.getLength()-1]) {
                outputFile += OString('/');
            }
            outputFile += strippedFileName.replaceAt(
                    strippedFileName.getLength() -3 , 3, "urd");
        } else {
            outputFile =
                sysFileName.replaceAt(sysFileName.getLength() -3 , 3, "urd");
        }
        OString const outputFileUrl = convertToFileUrl(outputFile);

        OString depFileUrl;
        if (options.isValid("-M")) {
            depFileUrl = convertToFileUrl(options.getOption("-M"));
            if ('/' != depFileUrl.getStr()[depFileUrl.getLength()-1]) {
                depFileUrl += OString('/');
            }
            depFileUrl += strippedFileName.replaceAt(
                    strippedFileName.getLength() -3 , 3, "d");
        }

        if ( nErrors ) {
            if (options.isValid("-M")) {
                removeIfExists(depFileUrl);
            }
            removeIfExists(outputFileUrl);
        } else {
            sPair_t const pair(depFileUrl, outputFile);
            nErrors = produceFile(outputFileUrl,
                        (options.isValid("-M")) ? &pair : 0);
        }

        idlc()->reset();
    }

    if ( nErrors > 0 )
    {
        fprintf(stderr, "%s: detected %ld errors%s",
            options.getProgramName().getStr(),
            sal::static_int_cast< long >(nErrors),
            options.prepareVersion().getStr());
    } else
    {
        if ( options.verbose() )
            fprintf(stdout, "%s: returned successful%s",
                options.getProgramName().getStr(),
                options.prepareVersion().getStr());
    }
    return nErrors;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
