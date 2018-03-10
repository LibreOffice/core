/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include <precomp.h>
#include "adc_cmds.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/ary.hxx>
#include <autodoc/displaying.hxx>
#include <autodoc/dsp_html_std.hxx>
#include <display/corframe.hxx>
#include <adc_cl.hxx>


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

extern const String C_opt_IgnoreDefine("-ignoredefine");
extern const String C_opt_ExternNamespace("-extnsp");
extern const String C_opt_ExternRoot("-extroot");



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
    if ( CommandLine::Get_().IdlUsed() )
        run_Idl();
    if ( CommandLine::Get_().CppUsed() )
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
        rGate = CommandLine::Get_().TheRepository().Gate_Idl();

    Cout() << "Creating HTML-output into the directory "
              << sOutputRootDirectory
              << "."
              << Endl();

    const DisplayToolsFactory_Ifc &
        rToolsFactory = DisplayToolsFactory_Ifc::GetIt_();
    Dyn<autodoc::HtmlDisplay_Idl_Ifc>
        pDisplay( rToolsFactory.Create_HtmlDisplay_Idl() );

    DYN display::CorporateFrame &   // KORR_FUTURE: Remove the need for const_cast
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
    const ary::Repository &
        rReposy = CommandLine::Get_().TheRepository();
    const ary::cpp::Gate &
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
