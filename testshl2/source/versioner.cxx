/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: versioner.cxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 02:28:45 $
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

#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "versionhelper.hxx"

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
