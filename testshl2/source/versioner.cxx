/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: versioner.cxx,v $
 * $Revision: 1.10 $
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
#include "precompiled_testshl2.hxx"

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "testshl/versionhelper.hxx"

#include <rtl/ustring.hxx>
#include <iostream>

// ----------------------------------- Main -----------------------------------
#if (defined UNX) || (defined OS2)
int main( int, char* argv[] )
#else
int _cdecl main( int, char* argv[] )
#endif
{
    static char const * optionSet[] = {
        "-time,      display time only",
        "-date,      display date only",
        "-upd,       display UPD only",
        "-minor,     display minor only",
        "-build,     display build only",
        "-inpath,    display inpath only",
        "-verbose,   be verbose",
        "-h:s,       display help or help on option",
        "-help:s,    see -h",
        NULL
    };

    GetOpt opt( argv, optionSet );

    // someone indicates that he needs help
    if ( opt.hasOpt( "-h" ) || opt.hasOpt( "-help" ) )
    {
        opt.showUsage();
        exit(0);
    }

    if (opt.getParams().empty())
    {
        // std::cerr << "error: At least a library should given." << std::endl;
        fprintf(stderr, "error: At least a library should given.\n");
        opt.showUsage();
        exit(0);
    }

    rtl::OUString suLibraryName = rtl::OStringToOUString(opt.getFirstParam(), RTL_TEXTENCODING_ASCII_US );
    VersionHelper aHelper(suLibraryName, opt);

    if (! aHelper.isOk() )
    {
        fprintf(stderr, "error: No version info found.\n");
        exit(1);
    }

    if (opt.hasOpt("-time"))
    {
        fprintf(stdout, "%s\n", aHelper.getTime().getStr());
    }
    else if (opt.hasOpt("-date"))
    {
        fprintf(stdout, "%s\n", aHelper.getDate().getStr());
    }
    else if (opt.hasOpt("-upd"))
    {
        fprintf(stdout, "%s\n", aHelper.getUpd().getStr());
    }
    else if (opt.hasOpt("-minor"))
    {
        fprintf(stdout, "%s\n", aHelper.getMinor().getStr());
    }
    else if (opt.hasOpt("-build"))
    {
        fprintf(stdout, "%s\n", aHelper.getBuild().getStr());
    }
    else if (opt.hasOpt("-inpath"))
    {
        fprintf(stdout, "%s\n", aHelper.getInpath().getStr());
    }
    else
    {
        // std::cout << aHelper << std::endl;
        aHelper.printall(stdout);
    }

    return 0;
}
