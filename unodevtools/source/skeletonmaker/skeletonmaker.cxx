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
#include <iostream>

#include <codemaker/global.hxx>
#include <codemaker/typemanager.hxx>
#include <sal/main.h>
#include <rtl/process.h>
#include <options.hxx>
#include <unoidl/unoidl.hxx>

#include "skeletonjava.hxx"
#include "skeletoncpp.hxx"

using namespace ::skeletonmaker;
using namespace ::unodevtools;

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
"                service type is necessary, referencing an interface which defines\n"
"                the new add-in functions.\n"
"    add-on      generates a language specific code skeleton for an add-on component\n"
"                using the implementation name as the file and class name. The protocol\n"
"                name(s) and the corresponding command(s) have to be specified with the\n"
"                '-p' option.\n"
"\n options:\n"
"    -a, --all              list all interface methods, not only the direct\n"
"                           ones\n"
"    --(java5|cpp)          select the target language\n"
"                           --java5 generate output for Java 1.5 or later (is \n"
"                                   currently the default)\n"
"                           --cpp   generate output for C++\n"
"    -sn, --shortnames      using namespace abbreviation 'css:': for\n"
"                           '::com::sun::star::', only valid for sub-command\n"
"                           'dump' and target language 'cpp'. It is default for the\n"
"                           sub-command 'component'.\n"
"    --propertysetmixin     the generated skeleton implements the cppu::PropertySetMixin\n"
"                           helper if a referenced new style service specifies an\n"
"                           interface which provides attributes (directly or inherited).\n"
"    -lh --licenseheader    generates a default LibreOffice MPL license\n"
"                           header at the beginning of a component source file.\n"
"                           This option is taken into account in 'component' mode\n"
"                           only and if -o is unequal 'stdout'.\n"
"    -bc                    specifies that the generated calc add-in is backward\n"
"    --backward-compatible  compatible to older office versions and implement the\n"
"                           former required add-in interfaces where the implementation\n"
"                           is mapped on the new add-in configuration. In this case\n"
"                           the config schema needs to be bundled with the extension\n"
"                           add-in as well. Default is a minimal add-in component\n"
"                           skeleton based on the configuration coming with the\n"
"                           office since OO.org 2.0.4.\n"
"    -o <path>              path specifies an existing directory where the\n"
"                           output files are generated to, only valid for\n"
"                           sub-command 'component'. If path=stdout the generated\n"
"                           code is generated on standard out instead of a file.\n"
"    -l <file>              specifies a binary type library (can be used more\n"
"                           than once).\n"
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
"    -p <protocol:cmd(s)>   specifies an add-on protocol name and the corresponding\n"
"                           command names, where the commands are a ',' separated list\n"
"                           of unique commands. This option is only valid for add-ons.\n"
"    -V, --version          print version number and exit\n"
"    -h, --help             print this help and exit\n\n";

void printUsageAndExit(const char* programname, const char* version)
{
    std::cerr
        << "\n using: " << programname << "\n"
           "            dump [<options>] -t <type> ...\n"
           "        " << programname << "\n"
           "            component [<options>] -n <name> -t <type> ...\n"
           "        " << programname << "\n"
           "            calc-add-in [<options>] -n <name> -t <add-in_service>\n"
           "        " << programname << "\n"
           "            add-on [<options>] -n <name> -p <protocol_name:command,...>\n"
           "        " << programname << " -V, --version\n"
           "        " << programname << " -h, --help\n"
        << usageText
        << programname << " Version " << version << "\n\n";
}

}

SAL_IMPLEMENT_MAIN()
{
    const char* const version = "0.4";
    const char* const programname = "uno-skeletonmaker";

    sal_uInt32 nCount = rtl_getAppCommandArgCount();
    if ( nCount == 0 ) {
        printUsageAndExit(programname, version);
        exit(EXIT_FAILURE);
    }

    ProgramOptions options;
    std::vector< OString > registries;
    std::vector< OString > types;
    OString delegate;

    try {

    sal_uInt32 nPos = 0;
    OUString arg, sOption;

    // check command
    rtl_getAppCommandArg(nPos++, &arg.pData);
    if ( arg == "dump" ) {
        options.dump = true;
    } else if ( arg == "component" ) {
        options.dump = false;
        options.shortnames = true;
    } else if ( arg == "calc-add-in" ) {
        options.dump = false;
        options.shortnames = true;
        options.componenttype = 2;
    } else if ( arg == "add-on" ) {
        options.dump = false;
        options.shortnames = true;
        options.componenttype = 3;
    } else if ( readOption( "h", &nPos, arg) ||
                readOption( "help", &nPos, arg) ) {
        printUsageAndExit(programname, version);
        exit(EXIT_SUCCESS);
    } else if ( readOption( "V", &nPos, arg) ||
                readOption( "version", &nPos, arg) ) {
        std::cerr << "\n Sun Microsystems (R) " << programname
                  << " Version " << version << "\n\n";
        exit(EXIT_SUCCESS);
    } else {
        std::cerr << "ERROR: unexpected command \"" << arg << "\"!\n";
        printUsageAndExit(programname, version);
        exit(EXIT_FAILURE);
    }

    // read up to arguments
    while ( nPos < nCount )
    {
        rtl_getAppCommandArg(nPos, &arg.pData);

        if ( readOption( "a", &nPos, arg) ||
             readOption( "all", &nPos, arg) ) {
            options.all = true;
            continue;
        }
        if ( readOption( "java4", &nPos, arg) ) {
            std::cerr <<
              "\nError: Java 1.4 is no longer supported, use --java5 instead\n";
        }
        if ( readOption( "java5", &nPos, arg) ) {
            options.language = 1;
            continue;
        }
        if ( readOption( "cpp", &nPos, arg) ) {
            options.language = 2;
            continue;
        }
        if ( readOption( "sn", &nPos, arg) ||
             readOption( "shortnames", &nPos, arg) ) {
            options.shortnames = true;
            continue;
        }
        if ( readOption( "lh", &nPos, arg) ||
             readOption( "licenseheader", &nPos, arg) ) {
            options.license = true;
            continue;
        }
        if ( readOption( "bc", &nPos, arg) ||
             readOption( "backward-compatible", &nPos, arg) ) {
            options.backwardcompatible = true;
            continue;
        }
        if ( readOption( "propertysetmixin", &nPos, arg) ) {
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
            registries.push_back(OUStringToOString(sOption, RTL_TEXTENCODING_UTF8));
            continue;
        }
        if ( readOption( &sOption, "t", &nPos, arg) ) {
            types.push_back(OUStringToOString(sOption, RTL_TEXTENCODING_UTF8));
            continue;
        }
        if ( readOption( &sOption, "p", &nPos, arg) ) {
            OString sTmp(OUStringToOString(sOption, RTL_TEXTENCODING_UTF8));
            sal_Int32 nIndex{ sTmp.indexOf(':')+1 };
            const OString sPrt = sTmp.copy(0, nIndex);
            std::vector< OString > vCmds;
            while (nIndex>=0)
                vCmds.push_back(sTmp.getToken( 0, ',', nIndex ));
            options.protocolCmdMap.emplace(sPrt, vCmds);
            continue;
        }


        // else illegal argument
        throw CannotDumpException("unexpected parameter \"" + arg + "\"!");
    }

    if ( types.empty() && options.componenttype != 3) {
        std::cerr
            << "\nError: no type is specified, use the -T option at least once\n";
        printUsageAndExit(programname, version);
        exit(EXIT_FAILURE);
    }

    rtl::Reference< TypeManager > manager(new TypeManager);
    for (const auto& rRegistry : registries)
    {
        manager->loadProvider(convertToFileUrl(rRegistry), true);
    }

    if ( options.dump ) {
        for (const auto& rType : types) {
            std::cout << "\n/***************************************************"
                "*****************************/\n";
            switch (options.language )
            {
            case 1: //Java
                java::generateDocumentation(std::cout, options, manager,
                                            rType, delegate);
                break;
            case 2: //C++
                cpp::generateDocumentation(std::cout, options, manager,
                                           rType, delegate);
                break;
            default:
                OSL_ASSERT(false);
                break;
            }
        }
    } else {
        switch ( options.language )
        {
        case 1: //Java
            java::generateSkeleton(options, manager, types);
            break;
        case 2: //C++
            cpp::generateSkeleton(options, manager, types);
            break;
        default:
            OSL_ASSERT(false);
            break;
        }
    }

    } catch (CannotDumpException & e) {
        std::cerr << "ERROR: " << e.getMessage() << '\n';
        return EXIT_FAILURE;
    } catch (unoidl::NoSuchFileException & e) {
        std::cerr << "ERROR: No such file <" << e.getUri() << ">\n";
        return EXIT_FAILURE;
    } catch (unoidl::FileFormatException & e) {
        std::cerr
            << "ERROR: Bad format of <" << e.getUri() << ">, \""
            << e.getDetail() << "\"\n";
        return EXIT_FAILURE;
    } catch (std::exception & e) {
        std::cerr << "ERROR: " << e.what() << "\n";
        return EXIT_FAILURE;
    }

    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
