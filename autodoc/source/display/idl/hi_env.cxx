/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hi_env.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 16:50:26 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_autodoc.hxx"


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

const char *
HtmlEnvironment_Idl::Link2Manual( const String & i_link ) const
{
    static StreamStr aLink_(200);
    aLink_.reset();
    aCurPosition.Get_LinkToRoot(aLink_);
    String sDvgRoot(pLayout->DevelopersGuideHtmlRoot());
    if (sDvgRoot.empty())
        sDvgRoot = "../DevelopersGuide";
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
