/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testmodules.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 04:52:25 $
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


