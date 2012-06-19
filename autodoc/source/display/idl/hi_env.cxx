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
#include "hi_env.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/ploc_dir.hxx>
#include <cfrstd.hxx>
#include <toolkit/out_tree.hxx>
#include "hi_ary.hxx"
#include "hi_linkhelper.hxx"



const String C_s_index_files("index-files");

const String C_sUseFileSuffix("-use.html");
const String C_IndexA_FileName("index-1.html");


HtmlEnvironment_Idl::HtmlEnvironment_Idl( const csv::ploc::Path &           i_rOutputDir,
                                          const ary::idl::Gate &            i_rGate,
                                          const display::CorporateFrame &   i_rLayout )
    :   aOutputRoot(i_rOutputDir),
        pData(new AryAccess(i_rGate)),
        pGate(&i_rGate),
        pOutputTree(new output::Tree),
        aCurPosition(pOutputTree->Root()),
        pCurPageCe(0),
        pLayout(&i_rLayout),
        pLinker()
{
    StringVector    aHelp;
    pOutputTree->Set_NamesRoot(aHelp);

    aHelp.push_back(output::IndexFilesDirName());
    pOutputTree->Set_IndexRoot(aHelp);

    (*aHelp.begin()) = String("com");
    aHelp.push_back(String("sun"));
    aHelp.push_back(String("star"));
    pOutputTree->Set_Overview(aHelp, output::ModuleFileName() );

    pLinker = new LinkHelper(*this);
}

HtmlEnvironment_Idl::~HtmlEnvironment_Idl()
{
}

namespace
{
StringVector    G_aChain;
}

void
HtmlEnvironment_Idl::Goto_Directory( output::Position   i_pos,
                                     bool               i_bCreateDirectoryIfNecessary )
{
    aCurPosition = i_pos;
    aCurPath = aOutputRoot.MyPath();

    aCurPosition.Get_Chain(G_aChain);
    for ( StringVector::const_iterator it = G_aChain.begin();
          it != G_aChain.end();
          ++it )
    {
        aCurPath.DirChain() += *it;
    }

    if (i_bCreateDirectoryIfNecessary)
        create_Directory(aCurPath);
}

void
HtmlEnvironment_Idl::Goto_DirectoryLevelDown( const String & i_subDirName,
                                              bool           i_bCreateDirectoryIfNecessary )
{
    aCurPosition +=(i_subDirName);

    aCurPath.SetFile(String::Null_());
    aCurPath.DirChain() += i_subDirName;

    if (i_bCreateDirectoryIfNecessary)
        create_Directory(aCurPath);
}

void
HtmlEnvironment_Idl::Goto_DirectoryLevelUp()
{
    aCurPosition -= 1;

    aCurPath.SetFile(String::Null_());
    aCurPath.DirChain() -= 1;
}

void
HtmlEnvironment_Idl::Set_CurFile( const String & i_fileName )
{
    aCurPath.SetFile(i_fileName);
}

void
HtmlEnvironment_Idl::create_Directory( const csv::ploc::Path & i_path )

{
    csv::ploc::Directory aCurDir(i_path);
    if (NOT aCurDir.Exists())
        aCurDir.PhysicalCreate();
}

inline bool
IsAbsoluteLink(const char * i_link)
{
    const char
        shttp[] = "http://";
    const char
        sfile[] = "file://";
    const int
        csize = sizeof shttp - 1;
    csv_assert(csize == sizeof sfile - 1);

    return      strncmp(i_link,shttp,csize) == 0
            OR  strncmp(i_link,sfile,csize) == 0;
}


const char *
HtmlEnvironment_Idl::Link2Manual( const String & i_link ) const
{
    if ( IsAbsoluteLink(i_link.c_str()) )
        return i_link;

    static StreamStr aLink_(200);
    aLink_.reset();
    String
        sDvgRoot(pLayout->DevelopersGuideHtmlRoot());
    if (sDvgRoot.empty())
        sDvgRoot = "../DevelopersGuide";

    // KORR_FUTURE
    // Enhance performance by calculating this only one time:
    if ( NOT IsAbsoluteLink(sDvgRoot.c_str()) )
        aCurPosition.Get_LinkToRoot(aLink_);
    aLink_ << sDvgRoot
           << "/"
           << i_link;
    return aLink_.c_str();
}

String
HtmlEnvironment_Idl::CurPageCe_AsText() const
{
    return CurPageCe_AsFile(".html");
}

String
HtmlEnvironment_Idl::CurPageCe_AsFile(const char * i_sEnding) const
{
    if (pCurPageCe == 0)
        return String::Null_();

    static StringVector aModule_;
    String sCe;
    String sDummy;
    Data().Get_CeText(aModule_, sCe, sDummy, *pCurPageCe);
    StreamLock slCe(500);
    if (aModule_.size() > 0)
        slCe().operator_join(aModule_.begin(), aModule_.end(), "/");
    if (NOT sCe.empty())
        slCe() << "/" << sCe << i_sEnding;
    return String(slCe().c_str());
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
