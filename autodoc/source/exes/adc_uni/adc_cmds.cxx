/*************************************************************************
 *
 *  $RCSfile: adc_cmds.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:34:37 $
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


#include <precomp.h>
#include "adc_cmds.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/ary.hxx>
#include <autodoc/displaying.hxx>
#include <autodoc/dsp_html_std.hxx>
#include <display/corframe.hxx>
#include <display/uidldisp.hxx>


namespace autodoc
{
namespace command
{

extern const String C_opt_Include("-I:");

extern const String C_opt_Verbose("-v");

extern const String C_opt_Parse("-parse");
extern const String C_opt_Name("-name");
extern const String C_opt_LangAll("-lg");
extern const String C_opt_ExtensionsAll("-extg");
extern const String C_opt_DevmanFile("-dvgfile");
extern const String C_opt_SinceFile("-sincefile");

extern const String C_arg_Cplusplus("c++");
extern const String C_arg_Idl("idl");
extern const String C_arg_Java("java");

extern const String C_opt_Project("-p");
//extern const String C_opt_Lang;
//extern const String C_opt_Extensions;
extern const String C_opt_SourceDir("-d");
extern const String C_opt_SourceTree("-t");
extern const String C_opt_SourceFile("-f");

extern const String C_opt_CreateHtml("-html");
extern const String C_opt_DevmanRoot("-dvgroot");

//extern const String C_opt_CreateXml("-xml");
//extern const String C_opt_Load("-load");
//extern const String C_opt_Save("-save");




//**************************        CreateHTML    ***********************//

CreateHtml::CreateHtml()
    :   sOutputRootDirectory(),
        sDevelopersManual_HtmlRoot()
{
}

CreateHtml::~CreateHtml()
{
}

void
CreateHtml::do_Init( opt_iter &          it,
                     opt_iter            itEnd )
{
    ++it;
    CHECKOPT( it != itEnd && (*it).char_at(0) != '-',
              "output directory", C_opt_CreateHtml );
    sOutputRootDirectory = *it;

    for ( ++it;
          it != itEnd AND (*it == C_opt_DevmanRoot);
          ++it )
    {
        if (*it == C_opt_DevmanRoot)
        {
            ++it;
            CHECKOPT( it != itEnd AND (*it).char_at(0) != '-',
                      "HTML root directory of Developers Guide",
                      C_opt_DevmanRoot );
            sDevelopersManual_HtmlRoot = *it;
        }
    }   // end for
}

bool
CreateHtml::do_Run() const
{
    if ( ::ary::n22::Repository::The_().HasIdl() )
        run_Idl();
    if ( ::ary::n22::Repository::The_().HasCpp() )
        run_Cpp();
    return true;
}

int
CreateHtml::inq_RunningRank() const
{
    return static_cast<int>(rank_CreateHtml);
}

void
CreateHtml::run_Idl() const
{
    const ary::idl::Gate &
        rGate = ary::n22::Repository::The_().Gate_Idl();

    Cout() << "Creating HTML-output into the directory "
              << sOutputRootDirectory
              << "."
              << Endl();

    const DisplayToolsFactory_Ifc &
        rToolsFactory = DisplayToolsFactory_Ifc::GetIt_();
    Dyn<autodoc::HtmlDisplay_Idl_Ifc>
        pDisplay( rToolsFactory.Create_HtmlDisplay_Idl() );

    DYN display::CorporateFrame &   // KORR: Remove the need for const_cast in future.
        drFrame = const_cast< display::CorporateFrame& >(rToolsFactory.Create_StdFrame());
    if (NOT DevelopersManual_HtmlRoot().empty())
        drFrame.Set_DevelopersGuideHtmlRoot( DevelopersManual_HtmlRoot() );

    pDisplay->Run( sOutputRootDirectory,
                   rGate,
                   drFrame );
}

void
CreateHtml::run_Cpp() const
{
    const ary::n22::Repository &
        rReposy = ary::n22::Repository::The_();
    const ary::cpp::DisplayGate &
        rGate = rReposy.Gate_Cpp();

    const DisplayToolsFactory_Ifc &
        rToolsFactory = DisplayToolsFactory_Ifc::GetIt_();
    Dyn< autodoc::HtmlDisplay_UdkStd >
        pDisplay( rToolsFactory.Create_HtmlDisplay_UdkStd() );

    pDisplay->Run( sOutputRootDirectory,
                   rGate,
                   rToolsFactory.Create_StdFrame() );
}


}   // namespace command
}   // namespace autodoc
