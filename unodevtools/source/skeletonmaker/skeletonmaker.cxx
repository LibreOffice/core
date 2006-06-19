/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: skeletonmaker.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-20 00:51:11 $
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
#include <iostream>

#include "sal/main.h"
#include "rtl/process.h"
#include "rtl/ustrbuf.hxx"
#include "unodevtools/typemanager.hxx"
#include "unodevtools/options.hxx"
#include "skeletonjava.hxx"
#include "skeletoncpp.hxx"

#include "com/sun/star/uno/Reference.hxx"

using namespace ::rtl;
using namespace ::skeletonmaker;
using namespace ::unodevtools;
using namespace ::com::sun::star::uno;

namespace {

static const char usageText[] =
"\n sub-commands:\n"
"    dump        dump declarations on stdout (e.g. constructors, methods, type\n"
"                mapping for properties) or complete method bodies with\n"
"                method forwarding.\n"
"    component   generates language specific code skeleton files using the\n"
"                implementation name as the file and class name\n"
"    calc-add-in generates a language specific code skeleton for a calc add-in\n"
"                using the implementation name as the file and class name. A \n"
"                service type is necessary referencing an interface which defines\n"
"                the new add-in functions."
"\n options:\n"
"    -env:INIFILENAME=<url> url specifies a URL to an UNO ini|rc file of an\n"
"                           existing UNO environment (URE, office installation).\n"
"    -env:UNO_TYPES=<url>   url specifies a binary type library file. It can be\n"
"                           a space separated list of urls.\n"
"    -a, --all              list all interface methods, not only the direct\n"
"                           ones\n"
"    --(java4|java5|cpp)    select the target language\n"
"                           --java4 generate output for Java 1.4 or earlier\n"
"                           --java5 generate output for Java 1.5 or later (is \n"
"                                   currently the default)\n"
"                           --cpp   generate output for C++\n"
"    -sn, --shortnames      using namespace abbreviation 'css:': for\n"
"                           '::com::sun::star::', only valid for sub-command\n"
"                           'dump' and target language 'cpp'. It's default for\n"
"                           sub-command 'component'.\n"
"    --propertysetmixin     means that the generated skeleton implements the\n"
"                           cppu::PropertySetMixin helper if a referenced new\n"
"                           style service specifies an interface which provides\n"
"                           attributes (directly or inherited).\n"
"    -lh --licenseheader    generates a default OpenOffice.org LGPL license\n"
"                           header at the beginning of a component source file.\n"
"                           This option is taken into account in 'component' mode\n"
"                           only and if -o is unequal 'stdout'.\n"
"    -o <path>              path specifies an existing directory where the\n"
"                           output files are generated to, only valid for\n"
"                           sub-command 'component'. If path=stdout the generated\n"
"                           code is generated on standard out instead of a file.\n"
"    -l <file>              specifies a binary type library (can be used more\n"
"                           than once). The type library is integrated as an\n"
"                           additional type provider in the bootstrapped type\n"
"                           system.\n"
"    -n <name>              specifies an implementation name for the component\n"
"                           (used as classname, filename and package|namespace\n"
"                           name). In 'dump' mode it is used as classname (e.g.\n"
"                           \"MyBase::\", C++ only) to generate method bodies not\n"
"                           inline.\n"
"    -d <name>              specifies a base classname or a delegator.\n"
"                           In 'dump' mode it is used as a delegator to forward\n"
"                           methods. It can be used as '<name>::' for base\n"
"                           forwarding, or '<name>->|.' for composition.\n"
"                           Using \"_\" means that a default bodies with default\n"
"                           return values are dumped.\n"
"    -t <name>              specifies an UNOIDL type name, e.g.\n"
"                           com.sun.star.text.XText (can be used more than once)\n"
"    -V, --version          print version number and exit\n"
"    -h, --help             print this help and exit\n\n"
" Sun Microsystems (R) ";

void printUsageAndExit(const char* programname, const char* version)
{
    std::cerr
        << "\n using: " << programname
        << " (-env:INIFILENAME=<url> | -env:UNO_TYPES=<url>)\n"
        << "                          dump [<options>] -t <type> ...\n"
        << "        " << programname
        << " (-env:INIFILENAME=<url> | -env:UNO_TYPES=<url>)\n"
        << "                          component [<options>] -n <name> -t "
        << "<type> ...\n"
        << "        " << programname
        << " (-env:INIFILENAME=<url> | -env:UNO_TYPES=<url>)\n"
        << "                          clc-add-in [<options>] -n <name> -t "
        << "<add-in_service>\n"
        << "        " << programname << " -V, --version\n"
        << "        " << programname << " -h, --help\n"
        << usageText
        << programname << " Version " << version << "\n\n";
}

}

SAL_IMPLEMENT_MAIN_WITH_ARGS(argc, /*argv*/)
{
    const char* version = "0.3";
    const char* programname = "uno-skeletonmaker";

    if ( argc <= 1 ) {
        printUsageAndExit(programname, version);
        exit(EXIT_FAILURE);
    }

    ProgramOptions options;
    std::vector< OUString > registries;
    std::vector< OString > types;
    OString delegate;

    try {

    sal_Int32 nPos = 0;
    sal_Int32 nCount = (sal_Int32)rtl_getAppCommandArgCount();
    OUString arg, sOption;
    sal_Bool bOption=sal_False;

    // check command
    rtl_getAppCommandArg(nPos++, &arg.pData);
    if ( arg.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("dump"))) ) {
        options.dump = true;
    } else if ( arg.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("component"))) ) {
        options.dump = false;
        options.shortnames = true;
    } else if ( arg.equals(OUString(RTL_CONSTASCII_USTRINGPARAM("calc-add-in"))) ) {
        options.dump = false;
        options.shortnames = true;
        options.componenttype = 2;
    } else if ( readOption( &bOption, "h", &nPos, arg) ||
                readOption( &bOption, "help", &nPos, arg) ) {
        printUsageAndExit(programname, version);
        exit(EXIT_SUCCESS);
    } else if ( readOption( &bOption, "V", &nPos, arg) ||
                readOption( &bOption, "version", &nPos, arg) ) {
        std::cerr << "\n Sun Microsystems (R) " << programname
                  << " Version " << version << "\n\n";
        exit(EXIT_SUCCESS);
    } else {
        std::cerr
            << "ERROR: unexpected command \""
            << OUStringToOString(arg, RTL_TEXTENCODING_UTF8).getStr()
            << "\"!\n";
        printUsageAndExit(programname, version);
        exit(EXIT_FAILURE);
    }

    // read up to arguments
    while ( nPos < nCount )
    {
        rtl_getAppCommandArg(nPos, &arg.pData);

        if ( readOption( &bOption, "a", &nPos, arg) ||
             readOption( &bOption, "all", &nPos, arg) ) {
            options.all = true;
            continue;
        }
        if ( readOption( &bOption, "java4", &nPos, arg) ) {
            options.java5 = false;
            options.language = 1;
            continue;
        }
        if ( readOption( &bOption, "java5", &nPos, arg) ) {
            options.java5 = true;
            options.language = 1;
            continue;
        }
        if ( readOption( &bOption, "cpp", &nPos, arg) ) {
            options.java5 = false;
            options.language = 2;
            continue;
        }
        if ( readOption( &bOption, "sn", &nPos, arg) ||
             readOption( &bOption, "shortnames", &nPos, arg) ) {
            options.shortnames = true;
            continue;
        }
        if ( readOption( &bOption, "lh", &nPos, arg) ||
             readOption( &bOption, "licenseheader", &nPos, arg) ) {
            options.license = true;
            continue;
        }
        if ( readOption( &bOption, "propertysetmixin", &nPos, arg) ) {
            options.supportpropertysetmixin = true;
            continue;
        }
        if ( readOption( &sOption, "d", &nPos, arg) ) {
            delegate = OUStringToOString(sOption, RTL_TEXTENCODING_UTF8);
            continue;
        }
        if ( readOption( &sOption, "n", &nPos, arg) ) {
            options.implname = OUStringToOString(sOption, RTL_TEXTENCODING_UTF8);
            continue;
        }
        if ( readOption( &sOption, "o", &nPos, arg) ) {
            options.outputpath = OUStringToOString(sOption, RTL_TEXTENCODING_UTF8);
            continue;
        }
        if ( readOption( &sOption, "l", &nPos, arg) ) {
            registries.push_back(sOption);
            continue;
        }
        if ( readOption( &sOption, "t", &nPos, arg) ) {
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
        exit(EXIT_FAILURE);
    }

    UnoTypeManager manager;
    if ( !manager.init(registries) ) {
        std::cerr
            << ("\nError: Using the binary type libraries failed, check the -L"
                " options\n");
        exit(EXIT_FAILURE);
    }

    if ( options.dump ) {
        std::vector< OString >::const_iterator iter = types.begin();
        while (iter != types.end()) {
            std::cout << "\n/***************************************************"
                "*****************************/\n";
            switch (options.language )
            {
            case 1: //Java
                java::generateDocumentation(std::cout, options, manager,
                                            *iter, delegate);
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
        switch ( options.language )
        {
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
