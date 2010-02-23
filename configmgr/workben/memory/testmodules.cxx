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
#include "precompiled_configmgr.hxx"

#include "testmodules.hxx"


namespace configtest
{
    #if 0 // more modules
            +"org.openoffice.Office.Common",
            +"org.openoffice.Office.Linguistic",
            x"org.openoffice.Office.TypeDetection",
            x"org.openoffice.Setup",
            -"org.openoffice.UserProfile",
            +"org.openoffice.Inet",
            -"org.openoffice.Office.Calc",
            -"org.openoffice.Office.Chart",
            -"org.openoffice.Office.DataAccess",
            -"org.openoffice.Office.Draw",
            -"org.openoffice.Office.Impress",
            -"org.openoffice.Office.Java",
            -"org.openoffice.Office.Labels",
            -"org.openoffice.Office.Math",
            +"org.openoffice.Office.Views",
            +"org.openoffice.Office.Writer",
            +"org.openoffice.Office.WriterWeb",
            +"org.openoffice.ucb.Configuration",
            -"org.openoffice.ucb.Hierarchy",
            x"org.openoffice.ucb.Store",
    #endif

    char const * const s_aTestModules[] =
        {
            "org.openoffice.Office.Common",
            "org.openoffice.Setup/CJK/Enable",
            "org.openoffice.Setup/Office/Modules",
            "org.openoffice.Inet",
            "org.openoffice.Office.Views",
            "org.openoffice.Setup/Product",
            "org.openoffice.ucb.Configuration",
            "org.openoffice.ucb.Store/ContentProperties",
            "org.openoffice.Office.TypeDetection",
            "org.openoffice.Office.Writer",
            "org.openoffice.Office.WriterWeb",
            "org.openoffice.Office.Linguistic",
            NULL // end marker
        };
    unsigned int const s_nTestModules = sizeof s_aTestModules/sizeof 0[s_aTestModules];

    std::vector<char const *> listTestModules(int argc, char* argv[])
    {
        std::vector<char const *> aModuleList( s_aTestModules, s_aTestModules + s_nTestModules);
        return aModuleList;
    }
}


