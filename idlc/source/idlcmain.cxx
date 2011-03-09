/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_idlc.hxx"

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
    catch( IllegalArgument& e)
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
            nErrors = produceFile(outputUrl);
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
        OString outputFileUrl;
        if ( options.isValid("-O") )
        {
            OString strippedFileName(sysFileName.copy(sysFileName.lastIndexOf(SEPARATOR) + 1));
            outputFileUrl = convertToFileUrl(options.getOption("-O"));
            sal_Char c = outputFileUrl.getStr()[outputFileUrl.getLength()-1];

            if ( c != '/' )
                outputFileUrl += OString::valueOf('/');

            outputFileUrl += strippedFileName.replaceAt(strippedFileName.getLength() -3 , 3, "urd");
        } else
        {
            outputFileUrl = convertToFileUrl(sysFileName.replaceAt(sysFileName.getLength() -3 , 3, "urd"));
        }

        if ( nErrors )
            removeIfExists(outputFileUrl);
        else
            nErrors = produceFile(outputFileUrl);

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
