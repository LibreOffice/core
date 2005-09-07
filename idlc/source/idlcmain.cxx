/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: idlcmain.cxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:11:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _IDLC_IDLC_HXX_
#include <idlc/idlc.hxx>
#endif
#ifndef _SAL_MAIN_H_
#include "sal/main.h"
#endif

using namespace ::rtl;

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    Options options;

    try
    {
        if (!options.initOptions(argc, argv))
           exit(1);
    }
    catch( IllegalArgument& e)
    {
        fprintf(stderr, "Illegal argument: %s\n%s",
            e.m_message.getStr(),
            options.prepareVersion().getStr());
        exit(99);
    }

    setIdlc(&options);

    sal_Int32 nErrors = 0;
    if (options.readStdin()) {
        fprintf(
            stdout, "%s: compile stdin...\n",
            options.getProgramName().getStr());
        nErrors = compileFile(0);
        if (idlc()->getWarningCount() > 0) {
            fprintf(
                stdout, "%s: detected %d warnings compiling stdin\n",
                options.getProgramName().getStr(), idlc()->getWarningCount());
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
    for (StringVector::const_iterator i(files.begin());
         i != files.end() && nErrors == 0; ++i)
    {
        OString sysFileName( convertToAbsoluteSystemPath(*i) );

        fprintf(stdout, "%s: compile '%s' ... \n",
            options.getProgramName().getStr(), (*i).getStr());
        nErrors = compileFile(&sysFileName);

        if ( idlc()->getWarningCount() )
            fprintf(stdout, "%s: detected %d warnings compiling file '%s'\n",
                    options.getProgramName().getStr(), idlc()->getWarningCount(),
                    (*i).getStr(), options.prepareVersion().getStr());

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
        fprintf(stdout, "%s: detected %d errors%s",
            options.getProgramName().getStr(), nErrors,
            options.prepareVersion().getStr());
    } else
    {
        fprintf(stdout, "%s: returned successful%s",
            options.getProgramName().getStr(),
            options.prepareVersion().getStr());
    }
    exit(nErrors);
    return 0;
}
