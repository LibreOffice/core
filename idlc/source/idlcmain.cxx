/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
