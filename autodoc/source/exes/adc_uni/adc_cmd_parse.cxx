/*************************************************************************
 *
 *  $RCSfile: adc_cmd_parse.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:34:00 $
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
#include "adc_cmd_parse.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/template/tpltools.hxx>
#include "adc_cmds.hxx"
#include "cmd_run.hxx"



namespace autodoc
{
namespace command
{

namespace
{

const String C_FileEnding_hxx("*.hxx");
const String C_FileEnding_h("*.h");
const String C_FileEnding_idl("*.idl");
const String C_FileEnding_java("*.java");

inline void
CHECK( bool b, const String & text )
{
    if (NOT b)
        throw X_CommandLine( text );
}

}   // anonymous namespace



//**************************     S_LanguageInfo     ***********************//

S_LanguageInfo::~S_LanguageInfo()
{
}

void
S_LanguageInfo::do_Init( opt_iter &          it,
                         opt_iter            itEnd )
{
    ++it;   // Cur is language.
    CHECKOPT(   it != itEnd AND
              ( *it == C_arg_Cplusplus OR
                *it == C_arg_Idl OR
                *it == C_arg_Java ),
              "language",
              C_opt_LangAll );

    if ( *it == C_arg_Cplusplus )
        eLanguage = cpp;
    else if ( *it == C_arg_Idl )
        eLanguage = idl;
    else if ( *it == C_arg_Java )
        eLanguage = java;
    else
         csv_assert(false);

    switch (eLanguage)
    {
        case cpp:   aExtensions.push_back( C_FileEnding_hxx );
                    aExtensions.push_back( C_FileEnding_h );
                    break;
        case idl:   aExtensions.push_back( C_FileEnding_idl );
                    break;
        case java:  aExtensions.push_back( C_FileEnding_java );
                    break;
        // default: do nothing.
    }

    ++it;   // Cur is next option.
}

void
S_LanguageInfo::InitExtensions( opt_iter &          it,
                                opt_iter            itEnd )
{
    ++it;
    CHECKOPT( it != itEnd AND (*it).char_at(0) == '.',
              "extensions",
              C_opt_ExtensionsAll );

    StreamLock slCheck(150);
    slCheck() << C_opt_ExtensionsAll
              << " used without previous "
              << C_opt_LangAll;

    CHECK( eLanguage != none,
           slCheck().c_str() );

    do {
        aExtensions.push_back(*it);
        ++it;
    }   while (it != itEnd AND (*it).char_at(0) == '.');
}



//**************************     Parse     ***********************//

Parse::Parse()
    :   sRepositoryName(),
        aGlobalLanguage(),
        aProjects(),
        sDevelopersManual_RefFilePath()
{
}

Parse::~Parse()
{
    csv::erase_container_of_heap_ptrs(aProjects);
}

void
Parse::do_Init( opt_iter &          it,
                opt_iter            itEnd )
{
    for ( ; it != itEnd;  )
    {
        if (*it == C_opt_Name)
            do_clName(it, itEnd);
        else if (*it == C_opt_LangAll)
            aGlobalLanguage.Init(it, itEnd);
        else if (*it == C_opt_ExtensionsAll)
            aGlobalLanguage.InitExtensions(it, itEnd);
        else if (*it == C_opt_DevmanFile)
            do_clDevManual(it, itEnd);
        else if (*it == C_opt_Project)
            do_clProject(it, itEnd);
        else if (    *it == C_opt_SourceTree
                  OR *it == C_opt_SourceDir
                  OR *it == C_opt_SourceFile )
            do_clDefaultProject(it, itEnd);
        else
            break;
    }   // for
}

void
Parse::do_clName( opt_iter &    it,
                  opt_iter      itEnd )
{
    ++it;
    CHECKOPT( it != itEnd AND (*it).char_at(0) != '-',
              "name",
              C_opt_Name );
    sRepositoryName = *it;
    ++it;
}

void
Parse::do_clDevManual( opt_iter &    it,
                       opt_iter      itEnd )
{
    ++it;
    CHECKOPT( it != itEnd AND (*it).char_at(0) != '-',
              "link file path",
              C_opt_DevmanFile );
    sDevelopersManual_RefFilePath = *it;
    ++it;
}

void
Parse::do_clProject( opt_iter &    it,
                     opt_iter      itEnd )
{
    if ( aProjects.size() == 1 )
    {
         if ( aProjects.front()->IsDefault() )
            throw X_CommandLine( "Both, named projects and a default project, cannot be used together." );
    }

    S_ProjectData * dpProject = new S_ProjectData(aGlobalLanguage);
    ++it;
    dpProject->Init(it, itEnd);
    aProjects.push_back(dpProject);
}

void
Parse::do_clDefaultProject( opt_iter &    it,
                            opt_iter      itEnd )
{
    if ( aProjects.size() > 0 )
    {
        throw X_CommandLine( "Both, named projects and a default project, cannot be used together." );
    }

    S_ProjectData * dpProject = new S_ProjectData( aGlobalLanguage,
                                                   S_ProjectData::default_prj );
    dpProject->Init(it, itEnd);
    aProjects.push_back(dpProject);
}

bool
Parse::do_Run() const
{
    run::Parser
        aParser(*this);
    return aParser.Perform();
}

int
Parse::inq_RunningRank() const
{
    return static_cast<int>(rank_Parse);
}



//**************************     S_Sources      ***********************//

void
S_Sources::do_Init( opt_iter &          it,
                    opt_iter            itEnd )
{
    StringVector *
        pList = 0;
    csv_assert((*it)[0] == '-');

    for ( ; it != itEnd; ++it)
    {
        if ((*it)[0] == '-')
        {
            if (*it == C_opt_SourceTree)
                pList = &aTrees;
            else if (*it == C_opt_SourceDir)
                pList = &aDirectories;
            else if (*it == C_opt_SourceFile)
                pList = &aFiles;
            else
                return;
        }
        else
            pList->push_back(*it);
    }   // end for
}



//**************************     S_ProjectData     ***********************//


S_ProjectData::S_ProjectData( const S_LanguageInfo & i_globalLanguage )
    :   sName(),
        aRootDirectory(),
        aLanguage(i_globalLanguage),
        aFiles(),
        bIsDefault(false)
{
}

S_ProjectData::S_ProjectData( const S_LanguageInfo & i_globalLanguage,
                              E_Default              )
    :   sName(),
        aRootDirectory("."),
        aLanguage(i_globalLanguage),
        aFiles(),
        bIsDefault(true)
{
}

S_ProjectData::~S_ProjectData()
{
}

void
S_ProjectData::do_Init( opt_iter &          it,
                        opt_iter            itEnd )
{
    if (NOT IsDefault())
    {
        CHECKOPT( it != itEnd AND (*it).char_at(0) != '-',
                  "name",
                  C_opt_Project );
        sName = *it;
        ++it;

        CHECKOPT( it != itEnd AND  (*it).char_at(0) != '-',
                  "root directory",
                  C_opt_Project );
        aRootDirectory.Set((*it).c_str(), true);
        ++it;
    }

    for ( ; it != itEnd; )
    {
        if (    *it == C_opt_SourceTree
             OR *it == C_opt_SourceDir
             OR *it == C_opt_SourceFile )
            aFiles.Init(it, itEnd);
//        else if (*it == C_opt_Lang)
//            aLanguage.Init(it, itEnd);
//        else if (*it == C_opt_Extensions)
//            aLanguage.InitExtensions(it, itEnd);
        else
            break;
    }   // for
}

}   // namespace command
}   // namespace autodoc



