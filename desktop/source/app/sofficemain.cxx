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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_desktop.hxx"

#include "app.hxx"
#include "cmdlineargs.hxx"
#include "cmdlinehelp.hxx"

#include <rtl/logfile.hxx>
#include <tools/extendapplicationenvironment.hxx>

int SVMain();

// -=-= main() -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

extern "C" int soffice_main()
{
    tools::extendApplicationEnvironment();

    RTL_LOGFILE_PRODUCT_TRACE( "PERFORMANCE - enter Main()" );

    desktop::Desktop aDesktop;
    // This string is used during initialization of the Gtk+ VCL module
    aDesktop.SetAppName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("soffice")) );
    aDesktop.CreateProcessServiceFactory();
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
#endif
    return SVMain();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
