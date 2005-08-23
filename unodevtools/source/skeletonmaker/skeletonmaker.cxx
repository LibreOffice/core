/*************************************************************************
 *
 *  $RCSfile: skeletonmaker.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: jsc $ $Date: 2005-08-23 08:31:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "sal/main.h"
#include "rtl/process.h"
#include "rtl/ustrbuf.hxx"
#include "unodevtools/typemanager.hxx"
#include "unodevtools/options.hxx"
#include "skeletonjava.hxx"
#include "skeletoncpp.hxx"

#include <com/sun/star/uno/Reference.hxx>

using namespace ::rtl;
using namespace ::skeletonmaker;
using namespace ::unodevtools;
using namespace ::com::sun::star::uno;

namespace {

static const char usageText[] =
"\n sub-commands:\n"
"    dump        dump declarations on stdout (e.g. constructors, methods, type\n"
"                mapping for properties)\n"
"                or complete method bodies with method forwarding.\n"
"    component   generates language specific code skeleton files using the\n"
"                implementation name as the file and class name\n"
"\n options:\n"
"    -env:INIFILENAME=<url> url specifies a URL to an UNO ini|rc file of an\n"
"                           existing UNO environment (URE, office installation).\n"
"    -env:UNO_TYPES=<url>   url specifies a binary type library file. It can be\n"
"                           a space separated list of urls.\n"
"    --a                    list all interface methods, not only the direct\n"
"                           ones\n"
"    --(java4|java5|cpp)    select the target language\n"
"                           --java4 generate output for Java 1.4 or earlier\n"
"                           --java5 generate output for Java 1.5 or later (is \n"
"                                   currently the default)\n"
"                           --cpp   generate output for C++\n"
"    -o <path>              path specifies an existing directory where the\n"
"                           output files are generated, only valid for\n"
"                           sub-command 'component'.\n"
"    -l <file>              specifies a binary type library (can be used more\n"
"                           than once). The type library is integrated as an\n"
"                           additional type provider in the bootstrapped type\n"
"                           system.\n"
"    -n <name>              specifies an implementation name for the component\n"
"                           (used as classname, filename and package|namespace\n"
"                           name). In 'dump' mode it is used as classname (e.g.\n"
"                           \"MyBase::\") to generate method bodies not inline.\n"
"    -d <name>              specifies a base classname or a delegator.\n"
"                           In 'dump' mode it is used as a delegator to forward\n"
"                           methods. It can be used as '<name>::' for base\n"
"                           forwarding, or '<name>->|.' for composition.\n"
"    -t <name>              specifies an UNOIDL type name, e.g.\n"
"                           com.sun.star.text.XText (can be used more than once)\n"
"    --v                    print version number and exit\n"
"    --h                    print this help and exit\n\n"
" Sun Microsystems (R) ";

void printUsageAndExit(char* programname, char* version) {
    std::cerr
        << "\n using: " << programname
        << " dump [<options>] (-env:INIFILENAME=<url> | \n"
        << "                          -env:UNO_TYPES=<url>) -t <type> ...\n"
        << "        " << programname
        << " component [<options>] (-env:INIFILENAME=<path>\n"
        << "                          | -env:UNO_TYPES=<url>) -n <name> -t <type> ...\n"
        << "        " << programname << " --v\n"
        << "        " << programname << " --h\n"
        << usageText
        << programname << " Version " << version << "\n\n";
    exit(EXIT_FAILURE);
}

}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, argv)
{
    char* version = "0.1";
    char* programname = "uno-skeletonmaker";

    if (argc <= 1)
        printUsageAndExit(programname, version);

    ProgramOptions options;
    std::vector< rtl::OUString > registries;
    std::vector< rtl::OString > types;
    rtl::OString delegate;

    try {

    sal_Int32 nPos = 0;
    sal_Int32 nCount = (sal_Int32)rtl_getAppCommandArgCount();
    OUString arg, sOption;
    sal_Bool bOption=sal_False;

    // check command
    rtl_getAppCommandArg(nPos++, &arg.pData);
    if (arg.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("dump")))) {
        options.dump = true;
    } else if (arg.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("component")))) {
        options.dump = false;
    } else if (readOption( &bOption, "h", &nPos, arg)) {
        printUsageAndExit(programname, version);
    } else if (readOption( &bOption, "v", &nPos, arg)) {
        std::cerr << "\n Sun Microsystems (R) " << programname
                  << " Version " << version << "\n\n";
        exit(EXIT_FAILURE);
    } else {
        std::cerr
            << "ERROR: unexpected command \""
            << OUStringToOString(arg, RTL_TEXTENCODING_UTF8).getStr()
            << "\"!\n";
        printUsageAndExit(programname, version);
    }

    // read up to arguments
    while (nPos < nCount)
    {
        rtl_getAppCommandArg(nPos, &arg.pData);

        if (readOption( &bOption, "a", &nPos, arg)) {
            options.all = true;
            continue;
        }
        if (readOption( &bOption, "java4", &nPos, arg)) {
            options.java5 = false;
            options.language = 1;
            continue;
        }
        if (readOption( &bOption, "java5", &nPos, arg)) {
            options.java5 = true;
            options.language = 1;
            continue;
        }
        if (readOption( &bOption, "cpp", &nPos, arg)) {
            options.java5 = false;
            options.language = 2;
            continue;
        }
        if (readOption( &sOption, "d", &nPos, arg)) {
            delegate = OUStringToOString(sOption, RTL_TEXTENCODING_UTF8);
            continue;
        }
        if (readOption( &sOption, "n", &nPos, arg)) {
            options.implname = OUStringToOString(sOption, RTL_TEXTENCODING_UTF8);
            continue;
        }
        if (readOption( &sOption, "o", &nPos, arg)) {
            options.outputpath = OUStringToOString(sOption, RTL_TEXTENCODING_UTF8);
            continue;
        }
        if (readOption( &sOption, "l", &nPos, arg)) {
            registries.push_back(sOption);
            continue;
        }
        if (readOption( &sOption, "t", &nPos, arg)) {
            types.push_back(OUStringToOString(sOption, RTL_TEXTENCODING_UTF8));
            continue;
        }

        // else illegal argument
        OUStringBuffer buf( 64 );
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("unexpected parameter \""));
        buf.append(arg);
        buf.appendAscii(RTL_CONSTASCII_STRINGPARAM("\"!"));
        throw RuntimeException(buf.makeStringAndClear(),
                               Reference< XInterface >());
    }

    if ( types.empty() ) {
        std::cerr
            << ("\nError: no type is specified, use the -T option at least once\n");
        printUsageAndExit(programname, version);
    }

    UnoTypeManager manager;
    if (!manager.init(registries)) {
        std::cerr
            << ("\nError: Using the binary type libraries failed, check the -L"
                " options\n");
        exit(EXIT_FAILURE);
    }

    if (options.dump) {
        std::vector< rtl::OString >::const_iterator iter = types.begin();
        while (iter != types.end()) {
            std::cout << "\n/********************************************************************************/\n";
            switch (options.language ) {
            case 1: //Java
                java::generateDocumentation(std::cout, options, manager,
                                            *iter);
                break;
            case 2: //C++
                cpp::generateDocumentation(std::cout, options, manager,
                                           *iter, delegate);
                break;
            default:
                OSL_ASSERT(false);
                break;
            }
            ++iter;
        }
    } else {
        switch (options.language ) {
        case 1: //Java
            java::generateSkeleton(options, manager, types, delegate);
            break;
        case 2: //C++
            cpp::generateSkeleton(options, manager, types, delegate);
            break;
        default:
            OSL_ASSERT(false);
            break;
        }
    }

    } catch (CannotDumpException & e) {
        std::cout.flush();
        std::cerr << "\nError: " << e.m_message << std::endl;
    } catch(Exception& e) {
        std::cout.flush();
        std::cerr
            << "\nError: "
            << OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr()
            << std::endl;
    }

    return 0;
}
