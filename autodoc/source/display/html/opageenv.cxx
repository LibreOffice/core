/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: opageenv.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:56:28 $
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


#include <precomp.h>
#include "opageenv.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/ploc_dir.hxx>
#include <ary/ce.hxx>
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/cg_file.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <udm/html/htmlitem.hxx>
#include "hdimpl.hxx"
#include "strconst.hxx"
#include <estack.hxx>

const udmstri C_sProjectDir( "prj" );
const udmstri C_sCppDir( "names" );
const udmstri C_sIndexDir( "ix" );


//************************         Implementation          ********************//

namespace
{

void    CreateDirectory( const udmstri &            i_sPath );
void    CreateDirectory( const csv::ploc::Path &    i_rPath );

void
CreateDirectory( const udmstri & i_sPath )
{
    csv::ploc::Directory aDirectory(i_sPath);
    if (NOT aDirectory.Exists())
        aDirectory.PhysicalCreate();
}

void
CreateDirectory( const csv::ploc::Path & i_rPath )
{
    csv::ploc::Directory aDirectory(i_rPath);
    if (NOT aDirectory.Exists())
        aDirectory.PhysicalCreate();
}

//************************         CheshireCat          ********************//

struct InNamespaceTree
{
    enum E_Type
    {
        t_unknown,
        t_namespace,
        t_type,
        t_operations,
        t_data
    };

    EStack< const ary::cpp::Namespace * >
                        aNamespaces; /// never empty.
    EStack< const ary::cpp::Class * >
                        aClasses;      /// maybe empty.
    const ary::CodeEntity *
                        pCe;        /// CurFileCe, maybe 0
    E_Type              eType;

                        InNamespaceTree(
                            const ary::cpp::Namespace &
                                                i_rNsp );
                        ~InNamespaceTree();
    void                GoDown(
                            const ary::cpp::Namespace &
                                                i_rNsp );
    void                GoDown(
                            const ary::cpp::Class &
                                                i_rClass );
    void                GoUp();
};

InNamespaceTree::InNamespaceTree( const ary::cpp::Namespace & i_rNsp )
    :   // aNamespaces,
        // aClasses,
        pCe(0),
        eType(t_unknown)
{
    aNamespaces.push( &i_rNsp );
}

InNamespaceTree::~InNamespaceTree()
{
}

void
InNamespaceTree::GoDown( const ary::cpp::Namespace & i_rNsp )
{
    aNamespaces.push(&i_rNsp);
    aClasses.erase_all();
    pCe = 0;
    eType = t_unknown;
}

void
InNamespaceTree::GoDown( const ary::cpp::Class & i_rClass )
{
    aClasses.push(&i_rClass);
    pCe = 0;
    eType = t_unknown;
}

void
InNamespaceTree::GoUp()
{
    if ( NOT aClasses.empty() )
        aClasses.pop();
    else
        aNamespaces.pop();
    pCe = 0;
    eType = t_unknown;
}

struct InProjectTree
{
    enum E_Type
    {
        t_project,
        t_file,
        t_defs
    };

    const ary::cpp::ProjectGroup *
                        pProject;       /// Always != 0.
    const ary::cpp::FileGroup *
                        pFile;          /// Maybe 0.
    E_Type              eType;
};

struct InIndex
{
    char                cLetter;

                        InIndex() : cLetter('A') {}
};


}   // anonymous namespace





struct OuputPage_Environment::CheshireCat
{
    csv::ploc::Path     aOutputRoot;
    csv::ploc::Path     aMyPath;
    csv::StreamStr      aFileName;

    const ary::cpp::DisplayGate *
                        pGate;
    const display::CorporateFrame *
                        pLayout;
    intt                nDepth;

    Dyn<InNamespaceTree>
                        pInNamespace;
    Dyn<InProjectTree>  pInProject;
    Dyn<InIndex>        pInIndex;

                        CheshireCat(
                            const csv::ploc::Path &
                                                io_rOutputDir,
                            const ary::cpp::DisplayGate &
                                                i_rGate,
                            const display::CorporateFrame &
                                                i_rLayout );
                        ~CheshireCat();
    void                AddQualifiedName2Path(
                            const ary::CodeEntity &
                                                i_rCe,
                            bool                i_bIncludeLocalName );

    const Dyn<InNamespaceTree> &
                        NspEnv() const          { return pInNamespace; }
    Dyn<InNamespaceTree> &
                        NspEnv()                { return pInNamespace; }
    const ary::cpp::Namespace *
                        Namespace() const       { return pInNamespace ? pInNamespace->aNamespaces.top() : 0; }
    const ary::cpp::Class *
                        Class() const           { return pInNamespace ? (pInNamespace->aClasses.empty() ? 0 : pInNamespace->aClasses.top()) : 0; }
};

OuputPage_Environment::
CheshireCat::CheshireCat( const csv::ploc::Path &         io_rOutputDir,
                          const ary::cpp::DisplayGate &   i_rGate,
                          const display::CorporateFrame & i_rLayout )
    :   aOutputRoot(io_rOutputDir),
        aMyPath(io_rOutputDir),
        aFileName(500),
        pGate(&i_rGate),
        pLayout(&i_rLayout),
        nDepth(0)
        // pInNamespace,
        // pInProject,
        // pInIndex
{
}

OuputPage_Environment::
CheshireCat::~CheshireCat()
{
}

void
OuputPage_Environment::
CheshireCat::AddQualifiedName2Path( const ary::CodeEntity & i_rCe,
                                    bool                    i_bIncludeLocalName )
{
    if ( i_rCe.Owner() == 0 )
    {
        aMyPath.DirChain().PushBack( C_sCppDir );
        return;
    }

    const ary::CodeEntity * pParent = pGate->Find_Ce( i_rCe.Owner() );
    csv_assert( pParent != 0 );
    AddQualifiedName2Path( *pParent, true );

    if ( i_bIncludeLocalName )
        aMyPath.DirChain().PushBack( i_rCe.LocalName() );
}



//************************         OuputPage_Environment          ********************//

OuputPage_Environment::OuputPage_Environment( const csv::ploc::Path &           io_rOutputDir,
                                              const ary::cpp::DisplayGate &     i_rGate,
                                              const display::CorporateFrame &   i_rLayout )
    :   pi( new CheshireCat(io_rOutputDir, i_rGate, i_rLayout) )
{
}

OuputPage_Environment::~OuputPage_Environment()
{
}

void
OuputPage_Environment::MoveDir_2Root()
{
    pi->NspEnv() = 0;
    pi->pInProject = 0;
    pi->pInIndex = 0;
    pi->nDepth = 0;
    while ( pi->aMyPath.DirChain().Size() > pi->aOutputRoot.DirChain().Size() )
        pi->aMyPath.DirChain().PopBack();
    pi->aMyPath.SetFile(udmstri::Null_());
}

void
OuputPage_Environment::MoveDir_Down2( const ary::cpp::Namespace & i_rNsp )
{
    if ( pi->NspEnv() )
    {
         csv_assert( pi->Namespace()->Id() == i_rNsp.Owner() );
        pi->NspEnv()->GoDown( i_rNsp );
        pi->aMyPath.DirChain().PushBack(i_rNsp.LocalName());
    }
    else
    {
        csv_assert( i_rNsp.Owner() == 0 );

        MoveDir_2Root();
        pi->NspEnv() = new InNamespaceTree( i_rNsp );
        pi->aMyPath.DirChain().PushBack( C_sCppDir );
    }

    pi->aMyPath.SetFile(udmstri::Null_());
    ++pi->nDepth;

       CreateDirectory( pi->aMyPath );
}

void
OuputPage_Environment::MoveDir_Down2( const ary::cpp::Class & i_rClass )
{
    csv_assert( pi->NspEnv() );
    if ( i_rClass.Protection() == ary::cpp::PROTECT_global )
    {
        csv_assert( pi->Namespace()->Id() == i_rClass.Owner() );
    }
    else
    {
        csv_assert( pi->Class() != 0 );
        csv_assert( pi->Class()->Id() == i_rClass.Owner() );
    }

    pi->NspEnv()->GoDown(i_rClass);
    pi->aMyPath.DirChain().PushBack(i_rClass.LocalName());
    pi->aMyPath.SetFile(udmstri::Null_());
    ++pi->nDepth;

       CreateDirectory( pi->aMyPath );
}

void
OuputPage_Environment::MoveDir_2Project( const ary::cpp::ProjectGroup & )
{
    // KORR_FUTURE
}

void
OuputPage_Environment::MoveDir_2Index()
{
    MoveDir_2Root();
    pi->pInIndex = new InIndex;
    pi->aMyPath.DirChain().PushBack( udmstri(C_sDIR_Index) );
    pi->aMyPath.SetFile(udmstri::Null_());
    pi->nDepth = 1;

       CreateDirectory( pi->aMyPath );
}

void
OuputPage_Environment::MoveDir_Up()
{
     if ( pi->nDepth == 1 )
    {
         MoveDir_2Root();
        return;
    }
    else if ( pi->NspEnv() )
    {
        pi->NspEnv()->GoUp();
        pi->aMyPath.DirChain().PopBack();
        pi->aMyPath.SetFile(udmstri::Null_());
        --pi->nDepth;
    }

/*  // KORR_FUTURE
    else if ( pi_>pInProject )
    {

        --pi->nDepth;
    }
*/
}

void
OuputPage_Environment::SetFile_Css()
{
    pi->aMyPath.SetFile( C_sHFN_Css );
}

void
OuputPage_Environment::SetFile_Overview()
{
    pi->aMyPath.SetFile( C_sHFN_Overview );
}

void
OuputPage_Environment::SetFile_AllDefs()
{
    // Provisorium
    pi->aMyPath.SetFile("def-all.html");
}

void
OuputPage_Environment::SetFile_Index( char i_cLetter )
{
    csv_assert( 'A' <= i_cLetter AND i_cLetter <= 'Z' OR i_cLetter == '_' );

    static StreamStr sIndexFileName(40);
    sIndexFileName.seekp(0);
    sIndexFileName << "index-";
    if ( i_cLetter == '_' )
    {
        sIndexFileName << "27";
    }
    else
    {
        sIndexFileName << int(i_cLetter -'A' + 1);
    }
    sIndexFileName << ".html";

    pi->aMyPath.SetFile( sIndexFileName.c_str() );
}

void
OuputPage_Environment::SetFile_Help()
{
    pi->aMyPath.SetFile( C_sHFN_Help );
}

void
OuputPage_Environment::SetFile_CurNamespace()
{
    csv_assert( pi->NspEnv() );
    pi->aMyPath.SetFile("index.html");
    pi->NspEnv()->pCe = pi->Namespace();
    pi->NspEnv()->eType = InNamespaceTree::t_namespace;
}

void
OuputPage_Environment::SetFile_Class( const ary::cpp::Class & i_rClass )
{
    csv_assert( pi->NspEnv() );
    pi->aMyPath.SetFile( ClassFileName(i_rClass.LocalName()) );
    pi->NspEnv()->pCe = &i_rClass;
    pi->NspEnv()->eType = InNamespaceTree::t_type;
}

void
OuputPage_Environment::SetFile_Enum( const ary::cpp::Enum & i_rEnum )
{
    csv_assert( pi->NspEnv() );
    pi->aMyPath.SetFile( EnumFileName(i_rEnum.LocalName()) );
    pi->NspEnv()->pCe = &i_rEnum;
    pi->NspEnv()->eType = InNamespaceTree::t_type;
}

void
OuputPage_Environment::SetFile_Typedef( const ary::cpp::Typedef & i_rTypedef )
{
    csv_assert( pi->NspEnv() );
    pi->aMyPath.SetFile( TypedefFileName(i_rTypedef.LocalName()) );
    pi->NspEnv()->pCe = &i_rTypedef;
    pi->NspEnv()->eType = InNamespaceTree::t_type;
}

void
OuputPage_Environment::SetFile_Operations( const ary::cpp::FileGroup * i_pFile )
{
    csv_assert( pi->NspEnv() );
    if ( CurClass() != 0 )
        pi->aMyPath.SetFile( "o.html" );
    else
    {
        csv_assert( i_pFile != 0 );
        pi->aMyPath.SetFile( HtmlFileName("o-", i_pFile->FileName()) );
    }
    pi->NspEnv()->pCe = 0;
    pi->NspEnv()->eType = InNamespaceTree::t_operations;
}

void
OuputPage_Environment::SetFile_Data( const ary::cpp::FileGroup * i_pFile )
{
    csv_assert( pi->NspEnv() );
    if ( CurClass() != 0 )
        pi->aMyPath.SetFile( "d.html" );
    else
    {
        csv_assert( i_pFile != 0 );
        pi->aMyPath.SetFile( HtmlFileName("d-", i_pFile->FileName()) );
    }
    pi->NspEnv()->pCe = 0;
    pi->NspEnv()->eType = InNamespaceTree::t_data;
}

void
OuputPage_Environment::SetFile_CurProject()
{
    // KORR_FUTURE
}

void
OuputPage_Environment::SetFile_File( const ary::cpp::FileGroup & )
{
    // KORR_FUTURE
}

void
OuputPage_Environment::SetFile_Defs( const ary::cpp::FileGroup & )
{
    // KORR_FUTURE
}

const ary::cpp::Namespace *
OuputPage_Environment::CurNamespace() const
{
    return pi->Namespace();
}

const ary::cpp::Class *
OuputPage_Environment::CurClass() const
{
    return pi->Class();
}

const csv::ploc::Path &
OuputPage_Environment::CurPath() const
{
    return pi->aMyPath;
}

const ary::cpp::DisplayGate &
OuputPage_Environment::Gate() const
{
    return *pi->pGate;
}

const display::CorporateFrame &
OuputPage_Environment::Layout() const
{
    return *pi->pLayout;
}

uintt
OuputPage_Environment::Depth() const
{
    return pi->nDepth;
}


