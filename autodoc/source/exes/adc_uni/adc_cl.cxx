/*************************************************************************
 *
 *  $RCSfile: adc_cl.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: np $ $Date: 2002-03-08 14:45:26 $
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
#include <adc_cl.hxx>


// NOT FULLY DEFINED SERVICES
#include "adc_cmds.hxx"


namespace autodoc
{

CommandLine * CommandLine::pTheInstance_ = 0;


const char * const C_sUserGuide =

"\n\n\n"
"               Use of Autodoc\n"
"               --------------\n"
"\n"
"   Example for C++:\n"
"\n"
"   -html <OutputDirectory> -name \"UDK 3.x anything\" -lg c++\n"
"        -p <ProjName> <ProjectRootDirectory>\n"
"            -t <SourceDir_relativeToProjectRoot>\n"
"\n"
"   There may be several Projects specified by -p.\n"
"\n"
"\n"
"   Example for IDL:\n"
"\n"
"   -html <OutputDirectory> -name \"UDK 3.x anything\" -lg idl\n"
"         -t <SourceDir1> <SourceDir2>\n"
"\n"
"\n"
"   For both languages, instead of or in addition to -t may be\n"
"   used -d (no subdirectories) or -f (just one file). There can\n"
"   be multiple arguments after each of these options (-t -d -f).\n";


#if 0   // FUTURE
"\n\n\n"
"               Use of Autodoc\n"
"               --------------\n"
"\n"
"   Basics:\n"
"\n"
"   Autodoc may perform different tasks.\n"
"\n"
"   Possible tasks are\n"
"       - parsing source code\n"
"       - creating HTML-output.\n"
"   On the command line each task starts with a specific\n"
"   option:\n"
"       '-parse' for parsing source code,\n"
"       '-html' for creating HTML.\n"
"   All command line options, related to one task, have to follow before\n"
"   the starting option of the next task.\n"
"\n"
"   Within the task '-parse', there may be defined different projects.\n"
"   A project definition is started with '-p'.\n"
"   All not project specific options within the task '-parse' have to\n"
"   appear in front of the first '-p'.\n"
"   There can be no project at all. Then all options, available for\n"
"   projects, can be used like for one nameless default project, without using\n"
"   '-p', but these options still have to appear behind all other\n"
"   options of the task '-parse'.\n"
"\n"
"\n"
"   Legend:\n"
"\n"
"       <SomeText>      Describes an argument.\n"
"       'SomeText'      Within '...' is the literal value of an argument.\n"
"       +               There can be multiple arguments.\n"
"       |               Separator for alternative literal values of an argument.\n"
"\n"
"\n"
"   Syntax:\n"
"\n"
"   -parse\n"
"       -name <RepositoryName>]\n"
"       -lg 'c++'|'idl'\n"
"       -extg <AdditonalExtensions>+\n"
"       -docg 'usehtml'\n"
"       -p <ProjectName> <ProjectRootDir>\n"
"           -l 'c++'|'idl'\n"
"           -ext <AdditonalExtensions>+\n"
"           -doc 'usehtml'\n"
"           -d <SourceDir_relative2ProjectRootDir_nosubdirs>+\n"
"           -t <SourceTree_relative2ProjectRootDir>+\n"
"           -f <SourceFile_relative2ProjectRootDir>+\n"
"   -html <OutputDir>\n"
"       -xlinks <Namespace> <ExternLinksRootDir>\n"
"   -i <CommandFilePath>\n"
"   -v <VerboseNr>\n"
"\n"
"\n"
"   Detailed Options Description:\n"
"\n"
"   Option      Arguments\n"
"   ----------------------------------------------------------\n"
"\n"
"   -parse      \n\n"
"               Starts the task \"Parse source code\".\n"
"               May be omitted, if it would be the first option on the\n"
"               command line.\n"
"\n"
"   -name       <RepositoryName>\n\n"
"               This name is used for naming the repository in\n"
"               human readable output. In future it may be used also for\n"
"               identifiing a repository in searches.\n"
"\n"
"   -lg         'c++|'idl'\n\n"
"               Identifies the programming language to be parsed.\n"
"                   'c++':  C++\n"
"                           Files with extensions '.h', '.hxx' are parsed.\n"
"                   'idl':  UNO-IDL\n"
"                           Files with extensions '.idl' are parsed.\n"
"               Here the language is set globally for all projects.\n"
"               A project can override this by using '-l'.\n"
"\n"
"   -extg       <.AdditionalExtension>+\n\n"
"               Has to follow immediately behind '-lg'.\n"
"               Specifies additional extensions, that will be recognised as\n"
"               source code files of the previously specified programming\n"
"               language.  Each extension has to start with '.'.\n"
"               It is possible to include extensionless files, too,\n"
"               by the argument '.'\n"
"               Here these extensions are set globally for all projects.\n"
"               A project can override this by using '-l' and '-ext'.\n"
"\n"
"   -docg       'html'|'nohtml'\n\n"
"               Specifies the default for all comments in source code, so \n"
"               that HTML-tags are interpreted as such or else treated as\n"
"               regular text.\n"
"               Without this option, the default is 'nohtml'.\n"
"               Here the default is set globally for all projects.\n"
"               A project can override this by using '-doc'.\n"
"\n"
"   -p          <ProjectName> <ProjectRootDirectory>\n\n"
"               ProjectName is used in output as human readable identifier\n"
"               for the project. ProjectRootDirectory is the path,\n"
"               where the arguments of '-d', '-t' and '-f' are relative to.\n"
"               This option can be omitted, then there is no project name\n"
"               and all paths are relative to the current working directory.\n"
"\n"
"   -l          'c++|'idl'\n\n"
"               Overrides -lg and -extg for the current project, which is\n"
"               specified by the last previous '-p'.\n"
"               For details see at option '-lg'.\n"
"\n"
"   -ext        <.AdditionalExtension>+\n\n"
"               Can be used only immediately behind '-l'.\n"
"               Overrides -extg for the current project, which is\n"
"               specified by the last previous '-p'.\n"
"               For details see at option '-extg'.\n"
"\n"
"   -doc        'html'|'nohtml'\n\n"
"               Overrides -docg for the current project, which is\n"
"               specified by the last previous '-p'.\n"
"               For details see at option '-docg'.\n"
"\n"
"   -d          <SourceDir_relative2ProjectRootDir_nosubdirs>+\n\n"
"               For the current project all files in the given\n"
"               directories are parsed, which have valid extensions.\n"
"               Subdirectories are NOT parsed.\n"
"\n"
"   -t          <SourceTree_relative2ProjectRootDir>+\n\n"
"               For the current project all files in the given\n"
"               directories AND its subdirectories are parsed, which\n"
"               have valid extensions.\n"
"\n"
"   -f          <SourceFile_relative2ProjectRootDir>+\n\n"
"               For the current project and language the given files\n"
"               are parsed. It doesn't matter, if their extensions match\n"
"               the valid extensions.\n"
"\n"
"   -html       <OutputRootDir>\n\n"
"               Starts the task \"Create HTML output\".\n"
"\n"
"   -xlinks     <Namespace> <ExternLinksRootDir>\n\n"
"               This option allows, to create links to external\n"
"               HTML-documents.\n"
"               For all source code objects (like classes or functions)\n"
"               which belong in the given namespace, the given root\n"
"               directory is used as a base for links to them.\n"
"               Presently, this works only for C++-mappings of IDL-items.\n"
"               The given namespace has to be absolute.\n"
"\n"
"   -i          <CommandFilePath>\n\n"
"               This option is replaced by the contents of the given\n"
"               file. The file has to be ASCII and each option\n"
"               has to start in the first column of a new line.\n"
"               So each valid line starts with a '-'.\n"
"               Empty lines are allowed.\n"
"               Comment lines have to start with '#'\n"
"\n"
"   -v          <VerboseNumber>\n\n"
"               Show details during parsing:\n"
"                   2    shows each parsed letter,\n"
"                   4    shows stored objects.\n"
"                   1    shows recognised tokens.\n"
"               These bit-values can be combined.\n"
"               This option suppresses errors, because of\n"
"               missing output options (no '-html').\n";
#endif // 0, FUTURE


CommandLine::CommandLine()
    :   nDebugStyle(0),
        // pCmd_Parse,
        // pCmd_Load,
        // pCmd_CreateHtml,
        // pCmd_CreateXml,
        // pCmd_Save,
        pCurProject(0),
        bInitOk(false)
{
    csv_assert(pTheInstance_ == 0);
    pTheInstance_ = this;
}

CommandLine::~CommandLine()
{
    pTheInstance_ = 0;
}

void
CommandLine::SetUpdate( const char * i_sRepositoryDir )
{
    if ( pCmd_Load )
    {
        if ( strcmp(pCmd_Load->ReposyDir(), i_sRepositoryDir) != 0 )
        {
            StreamLock slMsg(200);
            throw command::X_CommandLine(
                slMsg() << "Different repository directories with options "
                        << command::C_opt_Load
                        << " and "
                        << command::C_opt_Update
                        << "."
                        << c_str );
        }
    }
    else
    {
         pCmd_Load = new command::Load(i_sRepositoryDir);
    }
}

void
CommandLine::SetCurProject( command::S_ProjectData & io_rProject )
{
    pCurProject = &io_rProject;
}

command::S_ProjectData &
CommandLine::CurProject()
{
    if (pCurProject == 0)
    {
        if ( NOT pCmd_Parse )
        {
            pCmd_Parse = new command::Parse(*this);
        }
        pCurProject = &pCmd_Parse->CreateDefaultProject();
    }

    return *pCurProject;
}

const CommandLine &
CommandLine::Get_()
{
    csv_assert(pTheInstance_ != 0);
    return *pTheInstance_;
}

void
CommandLine::do_Init( int                 argc,
                      char *              argv[] )
{
    bInitOk = false;

    try
    {

    char * * itEnd = &argv[0] + argc;
    for ( char * * it = &argv[1]; it != itEnd; )
    {
         if ( strcmp(*it, command::C_opt_Verbose) == 0 )
        {
            ++it;
            if ( NOT(it != itEnd && **it >= '0' && **it <= '9') )
            {
                StreamLock slMsg(200);
                throw command::X_CommandLine(
                    slMsg() << "Missing number after "
                            << command::C_opt_Verbose
                            << "."
                            << c_str );
            }

            nDebugStyle = int(**it - '0');
            ++it;
        }
        else if ( strcmp(*it, command::C_opt_Load) == 0 )
        {
            pCmd_Load = new command::Load;
            it = pCmd_Load->Init(it, itEnd);
        }
        else if ( strcmp(*it, command::C_opt_CreateHtml) == 0 )
        {
            pCmd_CreateHtml = new command::CreateHtml;
            it = pCmd_CreateHtml->Init(it, itEnd);
        }
        else if ( strcmp(*it, command::C_opt_CreateXml) == 0 )
        {
            pCmd_CreateXml = new command::CreateXml;
            it = pCmd_CreateXml->Init(it, itEnd);
        }
        else if ( strcmp(*it, command::C_opt_Save) == 0 )
        {
            pCmd_Save = new command::Save;
            it = pCmd_Save->Init(it, itEnd);
        }
        else
        {
            pCmd_Parse = new command::Parse(*this);
            char * * result = pCmd_Parse->Init(it, itEnd);
            if (result == it)
            {
                StreamLock slMsg(200);
                throw command::X_CommandLine(
                    slMsg() << "Unknown comand line option: '"
                            << *it
                            << "' ."
                            << c_str );
            }
            else
                it = result;
        }
    }   // end for

    bInitOk = true;

    }   // end try
    catch ( command::X_CommandLine & xxx )
    {
        xxx.Report( Cerr() );
    }
    catch ( ... )
    {
        Cerr() << "Unknown error during command line parsing." << Endl();
    }

}

void
CommandLine::do_PrintUse() const
{
    Cout() << C_sUserGuide << Endl();
}

bool
CommandLine::inq_CheckParameters() const
{
    if ( NOT pCmd_Parse AND NOT pCmd_Load)
        return false;
    if ( pCmd_Parse )
    {
         if ( pCmd_Parse->GlobalLanguageInfo() == 0 )
        {
             Cerr() << "Missing language option ("
                 << command::C_opt_LangAll
                 << ") in command line"
                 << Endl();
            return false;
        }
    }
    if ( NOT pCmd_Save AND NOT pCmd_CreateHtml AND NOT pCmd_CreateXml
         AND nDebugStyle == 0 )
        return false;
    return bInitOk;
}


}   // namespace autodoc

