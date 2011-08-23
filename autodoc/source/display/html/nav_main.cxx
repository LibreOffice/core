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
#include "nav_main.hxx"


// NOT FULLY DEFINED SERVICES
#include <cosv/tpl/tpltools.hxx>
#include <ary/cpp/c_ce.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/loc/loc_file.hxx>
#include <udm/html/htmlitem.hxx>
#include "hdimpl.hxx"
#include "opageenv.hxx"
#include "strconst.hxx"


using namespace ::csi::html;
using namespace ::csi::xml;


const String  sOverview("Overview");
const String  sNamespace("Namespace");
const String  sClass("Class");
const String  sTree("Tree");
const String  sProject("Project");
const String  sFile("File");
const String  sIndex("Index");
const String  sHelp("Help");



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
                            const String  &     i_sText,
                            const char *        i_sLink,
                            const char *        i_sTip );
                        ~MainRowItem();
  private:
    enum E_Style { eSelf, eNo, eStd };

    virtual void        do_Write2(
                            TableRow &          o_rOut );
    String              sText;
    String              sLink;
    String              sTip;
};

MainRowItem::MainRowItem( const String  &     i_sText,
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
                            const String  &     i_sText )
                                                :   sText(i_sText) {}
  private:
    virtual void        do_Write2(
                            TableRow &          o_rOut );
    String              sText;
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
                            const String  &     i_sText )
                                                :   sText(i_sText) {}
  private:
    virtual void        do_Write2(
                            TableRow &          o_rOut );
    String              sText;
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
MainRow::SetupItems_Ce( const ary::cpp::CodeEntity & i_rCe )
{
    csv_assert( pEnv->CurNamespace() != 0 );
    bool bIsNamespace = i_rCe.Id() == pEnv->CurNamespace()->Id();
    bool bHasClass = pEnv->CurClass() != 0;
    bool bIsClass = dynamic_cast< const ary::cpp::Class * >(&i_rCe) != 0;

    Create_ItemList_InDirTree_Cpp(
                ( bIsNamespace ? eSelf : eStd ),
                ( bIsClass ? eSelf : bHasClass ? eStd : eNo ),
                eNo, 0 );
}

void
MainRow::SetupItems_FunctionGroup()
{
    Create_ItemList_InDirTree_Cpp(
                eStd,
                (pEnv->CurClass() != 0 ? eStd : eNo),
                eNo, 0 );
}

void
MainRow::SetupItems_DataGroup()
{
    SetupItems_FunctionGroup();
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
        String  sLinkOverview = ( i_eIndex == eSelf
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
        String  sLinkHelp = ( i_eIndex == eSelf
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
                                        const char *  )
{
    String
        sLinkOverview = PathPerRoot(*pEnv, C_sHFN_Overview);
    Add_Item( eStd, sOverview, sLinkOverview.c_str(), "" );

    if (i_eNsp == eStd)
    {
        String  sLinkNamespace = PathPerNamespace(*pEnv, "index.html");
        Add_Item( i_eNsp, sNamespace, sLinkNamespace.c_str(), "" );
    }
    else
    {
        Add_Item( i_eNsp, sNamespace, "", "" );
    }

    if (i_eClass == eStd)
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


    Add_Item( eStd, sIndex, PathPerRoot(*pEnv, C_sPath_Index), "" );
    String
        sLinkHelp = PathPerRoot(*pEnv, "help.html");
    Add_Item( eStd, sHelp, sLinkHelp.c_str(), "" );
}

void
MainRow::Add_Item( E_Style             i_eStyle,
                   const String  &     i_sText,
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



/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
