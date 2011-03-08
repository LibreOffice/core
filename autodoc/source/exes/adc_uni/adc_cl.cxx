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

#include <precomp.h>
#include <adc_cl.hxx>


// NOT FULLY DEFINED SERVICES
#include <algorithm>
#include <cosv/x.hxx>
#include <cosv/file.hxx>
#include <cosv/tpl/tpltools.hxx>
#include <ary/ary.hxx>
#include <tools/tkpchars.hxx>
#include <adc_msg.hxx>
#include "adc_cmds.hxx"
#include "adc_cmd_parse.hxx"
#include "cmd_sincedata.hxx"


namespace autodoc
{

CommandLine * CommandLine::pTheInstance_ = 0;

const char * const C_sUserGuide =
"\n\n\n"
"               General Use of Autodoc\n"
"               ----------------------\n"
"\n"
"   Example for C++:\n"
"\n"
"   -html <OutputDirectory> -name \"UDK 3.x anything\" -lg c++\n"
"        -p <ProjName> <ProjectRootDirectory>\n"
"            -t <SourceDir_relativeToProjectRoot>\n"
"\n"
"   There may be several projects specified by -p.\n"
"\n"
"\n"
"   Example for IDL:\n"
"\n"
"   -html <OutputDirectory> -name \"UDK 3.x anything\" -lg idl\n"
"         -t <SourceDir1> <SourceDir2>\n"
"\n"
"   For both languages, instead of or in addition to -t may be\n"
"   used -d (no subdirectories) or -f (just one file). There can\n"
"   be multiple arguments after each of these options (-t -d -f).\n"
"\n"
"\n"
"           Replacing @since Tag Content\n"
"           ----------------------------\n"
"\n"
"   In both languages you can give a transformation file to replace\n"
"   entries in @since tags by different entries.\n"
"   This file is given by the option\n"
"       -sincefile <TransformationFilePath>\n"
"   This option has to appear between the -html and the -lg option.\n"
"   Example:\n"
"   -html <OutputDirectory> -sincefile replacesince.txt\n"
"       -name \"UDK 3.x anything\" -lg idl -t <SourceDir>\n"
"\n"
"\n";

CommandLine::CommandLine()
    :   nDebugStyle(0),
        pSinceTransformator(new command::SinceTagTransformationData),
        aCommands(),
        bInitOk(false),
        pCommand_CreateHtml(0),
        pReposy( & ary::Repository::Create_() ),
        bCpp(false),
        bIdl(false)
{
    csv_assert(pTheInstance_ == 0);
    pTheInstance_ = this;
}

CommandLine::~CommandLine()
{
    csv::erase_container_of_heap_ptrs(aCommands);
    pTheInstance_ = 0;
}

int
CommandLine::Run() const
{
    Cout() << "\nAutodoc version 2.2.5"
           << "\n---------------------"
           << "\n" << Endl();

    bool
        ok = true;
    for ( CommandList::const_iterator it = aCommands.begin();
          ok AND it != aCommands.end();
          ++it )
    {
        ok = (*it)->Run();
    }

    if (pCommand_CreateHtml != 0)
    {
        StreamStr aDiagnosticMessagesFile(700);
        aDiagnosticMessagesFile
            << pCommand_CreateHtml->OutputDir()
            << csv::ploc::Delimiter()
            << "Autodoc_DiagnosticMessages.txt";
        TheMessages().WriteFile(aDiagnosticMessagesFile.c_str());
    }

    return ok ? 0 : 1;
}

CommandLine &
CommandLine::Get_()
{
    csv_assert(pTheInstance_ != 0);
    return *pTheInstance_;
}

bool
CommandLine::DoesTransform_SinceTag() const
{
    return pSinceTransformator->DoesTransform();
}

//bool
//CommandLine::Strip_SinceTagText( String & io_sSinceTagValue ) const
//{
//    return pSinceTransformator->StripSinceTagText(io_sSinceTagValue);
//}

const String &
CommandLine::DisplayOf_SinceTagValue( const String & i_sVersionNumber ) const
{
    return pSinceTransformator->DisplayOf(i_sVersionNumber);
}

void
CommandLine::do_Init( int                 argc,
                      char *              argv[] )
{
  try
  {
    bInitOk = false;
    StringVector    aParameters;

    char * * itpEnd = &argv[0] + argc;
    for ( char * * itp = &argv[1]; itp != itpEnd; ++itp )
    {
         if ( strncmp(*itp, "-I:", 3) != 0 )
            aParameters.push_back(String(*itp));
        else
            load_IncludedCommands(aParameters, (*itp)+3);
    }

    StringVector::const_iterator itEnd = aParameters.end();
    for ( StringVector::const_iterator it = aParameters.begin();
          it != itEnd;
        )
    {
        if ( *it == command::C_opt_Verbose )
            do_clVerbose(it,itEnd);
        else if ( *it == command::C_opt_LangAll
                  OR *it == command::C_opt_Name
                  OR *it == command::C_opt_DevmanFile )
            do_clParse(it,itEnd);
        else if (*it == command::C_opt_CreateHtml)
            do_clCreateHtml(it,itEnd);
        else if (*it == command::C_opt_SinceFile)
            do_clSinceFile(it,itEnd);
        else if (*it == command::C_opt_ExternNamespace)
        {
            sExternNamespace = *(++it);
            ++it;
            if ( strncmp(sExternNamespace.c_str(), "::", 2) != 0)
            {
                 throw command::X_CommandLine(
                        "-extnsp needs an absolute qualified namespace, starting with \"::\"."
                        );
            }
        }
        else if (*it == command::C_opt_ExternRoot)
        {
            ++it;
            StreamLock sl(1000);
            if ( csv::compare(*it, 0, "http://", 7) != 0 )
            {
                sl() << "http://" << *it;
            }
            if ( *(sl().end()-1) != '/')
                sl() << '/';
            sExternRoot = sl().c_str();

            ++it;
        }
//        else if (*it == command::C_opt_CreateXml)
//            do_clCreateXml(it,itEnd);
//        else if (command::C_opt_Load)
//            do_clLoad(it,itEnd);
//        else if (*it == command::C_opt_Save)
//            do_clSave(it,itEnd);
        else if (*it == "-h" OR *it == "-?" OR *it == "?")
            // Leads to displaying help, because bInitOk stays on false.
             return;
        else if ( *it == command::C_opt_Parse )
            // Only for backwards compatibility.
            //   Just ignore "-parse".
            ++it;
        else
        {
            StreamLock sl(200);
             throw command::X_CommandLine(
                            sl() << "Unknown commandline option \""
                                 << *it
                                 << "\"."
                                 << c_str );
        }
    }   // end for
    sort_Commands();

    bInitOk = true;

  }   // end try
  catch ( command::X_CommandLine & xxx )
  {
    xxx.Report( Cerr() );
  }
  catch ( csv::Exception & xxx )
  {
    xxx.GetInfo( Cerr() );
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
    if (NOT bInitOk OR aCommands.size() == 0)
        return false;
    return true;
}

void
CommandLine::load_IncludedCommands( StringVector &      out,
                                    const char *        i_filePath )
{
    CharacterSource
        aIncludedCommands;
    csv::File
        aFile(i_filePath, csv::CFM_READ);
    if (NOT aFile.open())
    {
         Cerr() << "Command include file \""
               << i_filePath
               << "\" not found."
               << Endl();
        throw command::X_CommandLine("Invalid file in option -I:<command-file>.");
    }
    aIncludedCommands.LoadText(aFile);
    aFile.close();

    bool bInToken = false;
    StreamLock aTransmit(200);
    for ( ; NOT aIncludedCommands.IsFinished(); aIncludedCommands.MoveOn() )
    {
        if (bInToken)
        {
            if (aIncludedCommands.CurChar() <= 32)
            {
                const char *
                    pToken = aIncludedCommands.CutToken();
                bInToken = false;

                 if ( strncmp(pToken, "-I:", 3) != 0 )
                 {
                     aTransmit().seekp(0);
                     aTransmit() << pToken;
                     aTransmit().replace_all('\\', *csv::ploc::Delimiter());
                     aTransmit().replace_all('/', *csv::ploc::Delimiter());
                    out.push_back(String(aTransmit().c_str()));
                 }
                else
                    load_IncludedCommands(out, pToken+3);
            }
        }
        else
        {
            if (aIncludedCommands.CurChar() > 32)
            {
                aIncludedCommands.CutToken();
                bInToken = true;
            }
        }   // endif (bInToken) else

    }   // end while()
}

namespace
{
inline int
v_nr(StringVector::const_iterator it)
{
     return int( *(*it).c_str() ) - int('0');
}
}   // anonymous namespace

void
CommandLine::do_clVerbose(  opt_iter &          it,
                            opt_iter            itEnd )
{
    ++it;
    if ( it == itEnd ? true : v_nr(it) < 0 OR v_nr(it) > 7 )
        throw command::X_CommandLine( "Missing or invalid number in -v option." );
    nDebugStyle = v_nr(it);
    ++it;
}

void
CommandLine::do_clParse( opt_iter &          it,
                         opt_iter            itEnd )
{
    command::Command *
        pCmd_Parse = new command::Parse;
    pCmd_Parse->Init(it, itEnd);
    aCommands.push_back(pCmd_Parse);
}

void
CommandLine::do_clCreateHtml( opt_iter &          it,
                              opt_iter            itEnd )
{
    pCommand_CreateHtml = new command::CreateHtml;
    pCommand_CreateHtml->Init(it, itEnd);
    aCommands.push_back(pCommand_CreateHtml);
}

void
CommandLine::do_clSinceFile( opt_iter &          it,
                             opt_iter            itEnd )
{
    pSinceTransformator->Init(it, itEnd);
}


namespace
{

struct Less_RunningRank
{
    bool                operator()(
                            const command::Command * const &
                                                i1,
                            const command::Command * const &
                                                i2 ) const
                        { return i1->RunningRank() < i2->RunningRank(); }
};

}   // anonymous namespace



void
CommandLine::sort_Commands()
{
    std::sort( aCommands.begin(),
               aCommands.end(),
               Less_RunningRank() );
}

}   // namespace autodoc

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
