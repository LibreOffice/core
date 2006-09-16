/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: nav_main.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 16:42:09 $
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
#include "nav_main.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/template/tpltools.hxx>
#include <ary/ce.hxx>
#include <ary/cpp/c_disply.hxx>
#include <ary/cpp/crog_grp.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/cg_proj.hxx>
#include <ary/cpp/cg_file.hxx>
#include <udm/html/htmlitem.hxx>
#include "hdimpl.hxx"
#include "opageenv.hxx"
#include "strconst.hxx"


using namespace ::csi::html;
using namespace ::csi::xml;


const udmstri sOverview("Overview");
const udmstri sNamespace("Namespace");
const udmstri sClass("Class");
const udmstri sTree("Tree");
const udmstri sProject("Project");
const udmstri sFile("File");
const udmstri sIndex("Index");
const udmstri sHelp("Help");



//********************    MainItem and derived ones      ***************//
class MainItem
{
  public:
    virtual             ~MainItem() {}
    void                Write2(
                            TableRow &          o_rOut );
  private:
    virtual void        do_Write2(
                            TableRow &          o_rOut ) = 0;
};

inline void
MainItem::Write2( TableRow & o_rOut )
    { do_Write2(o_rOut); }


namespace
{

class MainRowItem : public MainItem
{
  public:
                        MainRowItem(
                            const udmstri &     i_sText,
                            const char *        i_sLink,
                            const char *        i_sTip );
                        ~MainRowItem();
  private:
    enum E_Style { eSelf, eNo, eStd };

    virtual void        do_Write2(
                            TableRow &          o_rOut );
    udmstri             sText;
    udmstri             sLink;
    udmstri             sTip;
};

MainRowItem::MainRowItem( const udmstri &     i_sText,
                          const char *        i_sLink,
                          const char *        i_sTip )
    :   sText(i_sText),
        sLink(i_sLink),
        sTip(i_sTip)
{
}

MainRowItem::~MainRowItem()
{
}

void
MainRowItem::do_Write2( TableRow & o_rOut )
{
    TableCell & rCell = o_rOut.AddCell();

    rCell
        << new ClassAttr( "navimain" )
        << new XmlCode("&nbsp;")
        >> *new Link(sLink.c_str())
            << sText.c_str();
    rCell
        << new XmlCode("&nbsp;");
}


class SelectedItem : public MainItem
{
  public:
                        SelectedItem(
                            const udmstri &     i_sText )
                                                :   sText(i_sText) {}
  private:
    virtual void        do_Write2(
                            TableRow &          o_rOut );
    udmstri             sText;
};

void
SelectedItem::do_Write2( TableRow & o_rOut )
{
    TableCell & rCell = o_rOut.AddCell();

    rCell
        << new ClassAttr( "navimainself" )
        << new XmlCode("&nbsp;")
        << sText.c_str()
        << new XmlCode("&nbsp;");
}

class UnavailableItem : public MainItem
{
  public:
                        UnavailableItem(
                            const udmstri &     i_sText )
                                                :   sText(i_sText) {}
  private:
    virtual void        do_Write2(
                            TableRow &          o_rOut );
    udmstri             sText;
};

void
UnavailableItem::do_Write2( TableRow & o_rOut )
{
    TableCell & rCell = o_rOut.AddCell();

    rCell
        << new ClassAttr( "navimainnone" )
        << new XmlCode("&nbsp;")
        << sText.c_str()
        << new XmlCode("&nbsp;");
}

}   // anonymous namespace

//************************      MainRow      ***************************//

MainRow::MainRow( const OuputPage_Environment & i_rEnv )
    :   // aItems,
        pEnv(&i_rEnv)
{
}

MainRow::~MainRow()
{
    csv::erase_container_of_heap_ptrs(aItems);
}

void
MainRow::SetupItems_Overview()
{
    Create_ItemList_Global( eSelf, eStd, eStd );
}

void
MainRow::SetupItems_AllDefs()
{
    Create_ItemList_Global( eStd, eStd, eStd );
}

void
MainRow::SetupItems_Index()
{
    Create_ItemList_Global( eStd, eSelf, eStd );
}

void
MainRow::SetupItems_Help()
{
    Create_ItemList_Global( eStd, eStd, eSelf );
}

void
MainRow::SetupItems_Ce( const ary::CodeEntity & i_rCe )
{
    csv_assert( pEnv->CurNamespace() != 0 );
    bool bIsNamespace = i_rCe.Id() == pEnv->CurNamespace()->Id();
    bool bHasClass = pEnv->CurClass() != 0;
    bool bIsClass = dynamic_cast< const ary::cpp::Class * >(&i_rCe) != 0;
    const ary::cpp::FileGroup *
         pFile = bIsNamespace
                    ?   0
                    :   pEnv->Gate().RoGroups().Search_FileGroup( i_rCe.Location() );
    const ary::cpp::ProjectGroup *
          pProj = pFile == 0
                    ?   0
                    :   pEnv->Gate().RoGroups().Find_ProjectGroup( pFile->OwningProject() );

    Create_ItemList_InDirTree_Cpp(
                ( bIsNamespace ? eSelf : eStd ),
                ( bIsClass ? eSelf : bHasClass ? eStd : eNo ),
                eNo, 0,
                ( pProj != 0 ? eStd : eNo ), pProj,
                ( pFile != 0 ? eStd : eNo ), pFile );
}

void
MainRow::SetupItems_FunctionGroup()
{
    csv_assert( pEnv->CurClass() != 0 );
    const ary::cpp::FileGroup *
        pFile = pEnv->Gate().RoGroups().Search_FileGroup( pEnv->CurClass()->Location() );
    csv_assert( pFile != 0 );
    const ary::cpp::ProjectGroup *
         pProj = pEnv->Gate().RoGroups().Find_ProjectGroup( pFile->OwningProject() );
    csv_assert( pProj != 0 );

    Create_ItemList_InDirTree_Cpp(
                eStd,
                eStd,
                eNo, 0,
                eStd, pProj,
                eStd, pFile );
}

void
MainRow::SetupItems_FunctionGroup( const ary::cpp::FileGroup & i_rFile )
{
    csv_assert( pEnv->CurClass() == 0 );
    const ary::cpp::ProjectGroup *
         pProj = pEnv->Gate().RoGroups().Find_ProjectGroup( i_rFile.OwningProject() );
    csv_assert( pProj != 0 );

    Create_ItemList_InDirTree_Cpp(
                eStd,
                eNo,
                eNo, 0,
                eStd, pProj,
                eStd, &i_rFile );
}

void
MainRow::SetupItems_DataGroup()
{
    SetupItems_FunctionGroup();
}

void
MainRow::SetupItems_DataGroup( const ary::cpp::FileGroup & i_rFile )
{
    SetupItems_FunctionGroup(i_rFile);
}

void
MainRow::SetupItems_Project()
{
    Create_ItemList_InDirTree_Prj( eSelf, 0, eNo, 0 );
}

void
MainRow::SetupItems_File( const ary::cpp::ProjectGroup & i_rProj )
{
    Create_ItemList_InDirTree_Prj( eStd, &i_rProj, eSelf, 0 );
}

void
MainRow::SetupItems_DefinitionsGroup( const ary::cpp::ProjectGroup & i_rProj,
                                      const ary::cpp::FileGroup &    i_rFile )
{
    Create_ItemList_InDirTree_Prj( eStd, &i_rProj, eStd, &i_rFile );
}


void
MainRow::Write2( csi::xml::Element & o_rOut ) const
{
    Table * pTable = new Table;
    o_rOut
        >> *pTable
                << new AnAttribute( "class", "navimain" )
                << new AnAttribute( "border", "0" )
                << new AnAttribute( "cellpadding", "1" )
                << new AnAttribute( "cellspacing", "0" );
    TableRow & rRow = pTable->AddRow();
    rRow
        << new AnAttribute( "align", "center" )
        << new AnAttribute( "valign", "top" );
    for ( ItemList::const_iterator it = aItems.begin();
          it != aItems.end();
          ++it )
    {
        (*it)->Write2( rRow );
    }
}

void
MainRow::Create_ItemList_Global( E_Style             i_eOverview,
                                 E_Style             i_eIndex,
                                 E_Style             i_eHelp )
{
    if ( i_eOverview == eStd )
    {
        udmstri sLinkOverview = ( i_eIndex == eSelf
                                        ?   dshelp::PathPerLevelsUp(
                                                1,
                                                C_sHFN_Overview )
                                        :   C_sHFN_Overview );
        Add_Item( i_eOverview, sOverview, sLinkOverview.c_str(), "" );
    }
    else
    {
        Add_Item( i_eOverview, sOverview, "", "" );
    }

    if ( i_eIndex == eSelf )
        Add_Item( eStd, sNamespace, "../names/index.html", "" );
    else
        Add_Item( eStd, sNamespace, "names/index.html", "" );

    Add_Item( eNo, sClass, "", "" );
#if 0   // Will be implemented later
    Add_Item( eNo, sTree, "", "" );
    Add_Item( eNo, sProject, "", "" );
    Add_Item( eNo, sFile, "", "" );
#endif  // Will be implemented later

    if ( i_eIndex == eStd )
    {
        Add_Item( i_eIndex, sIndex, C_sPath_Index, "" );
    }
    else
    {
        Add_Item( i_eIndex, sIndex, "", "" );
    }

    if ( i_eHelp == eStd )
    {
        udmstri sLinkHelp = ( i_eIndex == eSelf
                                    ?   PathPerLevelsUp(1,C_sHFN_Help)
                                    :   C_sHFN_Help );
        Add_Item( i_eHelp, sHelp, sLinkHelp.c_str(), "" );
    }
    else
    {
        Add_Item( i_eHelp, sHelp, "", "" );
    }
}

void
MainRow::Create_ItemList_InDirTree_Cpp( E_Style i_eNsp,
                                        E_Style i_eClass,
                                        E_Style ,
                                        const char *  ,
                                        E_Style ,
                                        const ary::cpp::ProjectGroup * ,
                                        E_Style ,
                                        const ary::cpp::FileGroup * )
{
    udmstri sLinkOverview = PathPerRoot(*pEnv, C_sHFN_Overview);
    Add_Item( eStd, sOverview, sLinkOverview.c_str(), "" );

    if ( i_eNsp == eStd )
    {
        udmstri sLinkNamespace = PathPerNamespace(*pEnv, "index.html");
        Add_Item( i_eNsp, sNamespace, sLinkNamespace.c_str(), "" );
    }
    else
    {
        Add_Item( i_eNsp, sNamespace, "", "" );
    }

    if ( i_eClass == eStd )
    {
        csv_assert( pEnv->CurClass() != 0 );

        StreamLock sLinkClass(300);
        sLinkClass() << PathPerNamespace(*pEnv, "c-")
                     << pEnv->CurClass()->LocalName()
                     << ".html";
        StreamLock sTipClass(300);
        sTipClass() << "Class "
                    << pEnv->CurClass()->LocalName();
        Add_Item( i_eClass, sClass, sLinkClass().c_str(), sTipClass().c_str() );
    }
    else
    {
        Add_Item( i_eClass, sClass, "", "" );
    }

#if 0  // Will be implemented later
    if ( i_eTree == eStd )
    {
         csv_assert( i_sTreeLink != 0 );
        Add_Item( i_eTree, sTree, i_sTreeLink, "" );
    }
    else
    {
        Add_Item( i_eTree, sTree, "", "" );
    }

    if ( i_eProj == eStd )
    {
         csv_assert( i_pProj != 0 );
        udmstri sProjectLink = PathPerRoot(*pEnv, "prj/");
        sProjectLink = csv::StringSum( sProjectLink.c_str(),
                                       i_pProj->Name().c_str(),
                                       "/index.html",
                                       NIL );
        udmstri sProjectTip = csv::StringSum( "Project ",
                                              i_pProj->Name().c_str(),
                                              NIL );
        Add_Item( i_eProj, sProject, sProjectLink, sProjectTip.c_str() );
    }
    else
    {
        Add_Item( i_eProj, sProject, "", "" );
    }
    if ( i_eProj == eStd )
    {
         csv_assert( i_pProj != 0 );
         csv_assert( i_pFile != 0 );

        udmstri sFileLink = PathPerRoot(*pEnv, "prj/");
        sFileLink = csv::StringSum( sFileLink.c_str(),
                                       i_pProj->Name().c_str(),
                                       "/f-",
                                       i_pFile->FileName().c_str(),
                                       ".html",
                                       NIL );
        udmstri sFileTip = csv::StringSum( "File ",
                                            i_pFile->FileName().c_str(),
                                            NIL );
        Add_Item( i_eFile, sFile, sFileLink, sFileTip.c_str() );
    }
    else
    {
        Add_Item( i_eFile, sFile, "", "" );
    }
#endif // Will be implemented later


    Add_Item( eStd, sIndex, PathPerRoot(*pEnv, C_sPath_Index), "" );
    udmstri sLinkHelp = PathPerRoot(*pEnv, "help.html");
    Add_Item( eStd, sHelp, sLinkHelp.c_str(), "" );
}

void
MainRow::Create_ItemList_InDirTree_Prj( E_Style                         i_eProj,
                                        const ary::cpp::ProjectGroup *  i_pProj,
                                        E_Style                         i_eFile,
                                        const ary::cpp::FileGroup *     i_pFile )
{
    Add_Item( eStd, sOverview, "../../index.html", "" );
    Add_Item( eNo, sNamespace, "", "" );
    Add_Item( eNo, sClass, "", "" );
    Add_Item( eNo, sTree, "", "" );

    udmstri sLinkProject = i_pProj != 0
                                ?   "index.html"
                                :   "";
    Add_Item( i_eProj, sProject, sLinkProject.c_str(), "" );
    StreamLock sLinkFile(120);
    if ( i_pFile != 0 )
        sLinkFile() << "f-" << i_pFile->FileName();

    Add_Item( i_eFile, sFile, sLinkFile().c_str(), "" );

    Add_Item( eStd, sIndex, "../../index-files/index-1.html", "" );
    Add_Item( eStd, sHelp, "../../help.html", "" );
}

void
MainRow::Add_Item( E_Style             i_eStyle,
                   const udmstri &     i_sText,
                   const char *        i_sLink,
                   const char *        i_sTip )
{
    switch (i_eStyle)
    {
         case eStd:      aItems.push_back( new MainRowItem(i_sText, i_sLink, i_sTip) );
                        break;
        case eNo:       aItems.push_back( new UnavailableItem(i_sText) );
                        break;
        case eSelf:     aItems.push_back( new SelectedItem(i_sText) );
                        break;
        default:
                        csv_assert(false);
    }
}



