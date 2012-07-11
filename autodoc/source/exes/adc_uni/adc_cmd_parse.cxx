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

#include <precomp.h>
#include "adc_cmd_parse.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <adc_cl.hxx>
#include "adc_cmds.hxx"
#include "cmd_run.hxx"



namespace autodoc
{
namespace command
{

namespace
{

const String C_FileEnding_idl("*.idl");

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
                *it == C_arg_Idl,
              "language",
              C_opt_LangAll );

    if ( *it == C_arg_Idl ) {
        eLanguage = idl;
    }
    else {
         csv_assert(false);
    }

    switch (eLanguage)
    {
        case idl:   aExtensions.push_back( C_FileEnding_idl );
                    CommandLine::Get_().Set_IdlUsed();
                    break;
        default:    //  do nothing.
                    ;
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
    if ( !aProjects.empty() )
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
