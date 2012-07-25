/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "sal/config.h"

#include "desktopdllapi.h"

#include "app.hxx"
#include "cmdlineargs.hxx"
#include "cmdlinehelp.hxx"

#include <rtl/logfile.hxx>
#include <rtl/bootstrap.hxx>
#include <tools/extendapplicationenvironment.hxx>

#if defined WNT
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

int SVMain();

// -=-= main() -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

extern "C" int DESKTOP_DLLPUBLIC soffice_main()
{
#if defined ANDROID || defined WNT
    try {
#endif
#if defined(ANDROID)
        rtl::Bootstrap::setIniFilename(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file:///assets/program/lofficerc")));
#endif
    tools::extendApplicationEnvironment();

    RTL_LOGFILE_PRODUCT_TRACE( "PERFORMANCE - enter Main()" );

    desktop::Desktop aDesktop;
    // This string is used during initialization of the Gtk+ VCL module
    aDesktop.SetAppName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("soffice")) );
#ifdef UNX
    // handle --version and --help already here, otherwise they would be handled
    // after VCL initialization that might fail if $DISPLAY is not set
    const desktop::CommandLineArgs& rCmdLineArgs = aDesktop.GetCommandLineArgs();
    if ( rCmdLineArgs.IsHelp() )
    {
        desktop::displayCmdlineHelp();
        return EXIT_SUCCESS;
    }
    else if ( rCmdLineArgs.IsVersion() )
    {
        desktop::displayVersion();
        return EXIT_SUCCESS;
    }
    else if ( rCmdLineArgs.HasUnknown() )
    {
        return EXIT_FAILURE;
    }
#endif
    return SVMain();
#if defined ANDROID || defined WNT
    } catch (const ::com::sun::star::uno::Exception &e) {
#if defined ANDROID
        fprintf (stderr, "Not handled UNO exception at main: '%s'\n",
                 rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
#elif defined WNT
        MessageBoxW(
            0,
            reinterpret_cast< LPCWSTR >(
                rtl::OUString("Unhandled exception:\n" + e.Message).getStr()),
            reinterpret_cast< LPCWSTR >(rtl::OUString("Fatal Error").getStr()),
            (MB_OK | MB_ICONERROR | MB_DEFBUTTON1 | MB_TASKMODAL
             | MB_SETFOREGROUND | MB_TOPMOST));
#endif
        throw; // to get exception type printed
    }
#endif
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
