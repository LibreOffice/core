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
#include "pm_class.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/cp_ce.hxx>
#include <ary/loc/loc_file.hxx>
#include <ary/loc/locp_le.hxx>
#include <ary/getncast.hxx>
#include "hd_chlst.hxx"
#include "hd_docu.hxx"
#include "hdimpl.hxx"
#include "html_kit.hxx"
#include "navibar.hxx"
#include "opageenv.hxx"
#include "pagemake.hxx"
#include "strconst.hxx"

using namespace adcdisp;

using namespace csi;
using csi::html::HorizontalLine;
using csi::html::LineBreak;
using csi::html::Link;
using csi::html::Table;
using csi::html::TableRow;
using csi::html::TableCell;

using ary::cpp::CesConstIterator;
using ary::doc::OldCppDocu;

const char * const C_sTitle_InnerClasses    = "Classes";
const char * const C_sTitle_InnerStructs    = "Structs";
const char * const C_sTitle_InnerUnions     = "Unions";
const char * const C_sTitle_Methods         = "Methods";
const char * const C_sTitle_StaticMethods   = "Static Methods";
const char * const C_sTitle_Data            = "Data";
const char * const C_sTitle_StaticData      = "Static Data";

const char * const C_sLabel_StaticOperations    = "static_ops";
const char * const C_sLabel_StaticVariables     = "static_vars";

const char * const   C_sTitlePublic      = "Public Members";
const char * const   C_sTitleProtected   = "Protected Members";
const char * const   C_sTitlePrivate     = "Private Members";
const char * const   C_sMprTitles[3]     = { C_sTitlePublic,
                                             C_sTitleProtected,
                                             C_sTitlePrivate
                                           };
const char * const   C_sSummaryTitlePublic      = "Public Members";
const char * const   C_sSummaryTitleProtected   = "Protected Members";
const char * const   C_sSummaryTitlePrivate     = "Private Members";
const char *
        C_sMprSummaryTitles[3] =
        { C_sSummaryTitlePublic, C_sSummaryTitleProtected, C_sSummaryTitlePrivate };
const char *
        C_sMprPrefixes[3] =
        { "publ_", "prot_", "priv_" };
const char *
        C_sSummaryItems_Titles[PageMaker_Class::cl_MAX] =
        { C_sTitle_InnerClasses, C_sTitle_InnerStructs, C_sTitle_InnerUnions,
          C_sTitle_Enums, C_sTitle_Typedefs,
          C_sTitle_Methods, C_sTitle_StaticMethods, C_sTitle_Data, C_sTitle_StaticData };
const char *
        C_sSummaryItems_Labels[PageMaker_Class::cl_MAX] =
        { C_sLabel_Classes, C_sLabel_Structs, C_sLabel_Unions,
          C_sLabel_Enums, C_sLabel_Typedefs,
          C_sLabel_Operations, C_sLabel_StaticOperations,
          C_sLabel_Variables, C_sLabel_StaticVariables };


const ary::cpp::E_Protection
        aProt[3] = { ary::cpp::PROTECT_public,
                     ary::cpp::PROTECT_protected,
                     ary::cpp::PROTECT_private };


PageMaker_Class::PageMaker_Class( PageDisplay &             io_rPage,
                                  const ary::cpp::Class &   i_rClass )
    :   SpecializedPageMaker(io_rPage),
        pMe( &i_rClass ),
        pChildDisplay( new ChildList_Display(io_rPage.Env(), i_rClass) ),
        pNavi(0)
        // pProtectionArea,
        // bChildLists_Exist
{
    int i_max = 3 * cl_MAX;
    for (int i = 0; i < i_max; i++)
    {
        bChildLists_Exist[i] = false;
    }  // end for
}

PageMaker_Class::~PageMaker_Class()
{
}

void
PageMaker_Class::MakePage()
{
    pNavi = new NavigationBar( Env(), Me() );

    Write_NavBar();
    Write_TopArea();
    Write_DocuArea();
    Write_ChildLists();

    pNavi->Write_SubRows();
    pNavi = 0;
}

void
PageMaker_Class::Write_NavBar()
{
    NavigationBar   aNavi( Env(), Me() );
    pNavi->Write( CurOut() );
    CurOut() << new HorizontalLine;
}

inline bool
IsInterface(const ary::doc::Documentation & i_doc)
{
    const OldCppDocu *
        doc = Get_CppDocu(i_doc);
    return doc != 0
            ?   doc->IsInterface()
            :   false;
}

void
PageMaker_Class::Write_TopArea()
{
    TemplateClause fTemplateClause;
    PageTitle_Std fTitle;

    Page().Write_NameChainWithLinks( Me() );

    fTemplateClause( CurOut(), Me().TemplateParameters() );
    fTitle( CurOut(), Get_ClassTypeKey(Me()), Me().LocalName() );

    CurOut() << new HorizontalLine;

    Write_BaseHierarchy();
    Write_DerivedList();

    CurOut() << new LineBreak;

    adcdisp::FlagTable
        aFlags( CurOut(), 4 );
    aFlags.SetColumn( 0, "virtual",
                      Me().Virtuality() != ary::cpp::VIRTUAL_none );
    aFlags.SetColumn( 1, "abstract",
                      Me().Virtuality() == ary::cpp::VIRTUAL_abstract );
    aFlags.SetColumn( 2, "interface",
                      IsInterface(Me().Docu())
                      OR  Me().Virtuality() == ary::cpp::VIRTUAL_abstract );
    aFlags.SetColumn( 3, "template",
                      Me().TemplateParameters().size() > 0 );
}

void
PageMaker_Class::Write_DocuArea()
{
    Docu_Display aDocuShow( Env() );

    aDocuShow.Assign_Out(CurOut());
    Me().Accept( aDocuShow );
    aDocuShow.Unassign_Out();

    ary::loc::File &
        rFile = Env().Gate().Locations().Find_File( Me().Location() );

    adcdisp::ExplanationList
        aFileText( CurOut() );
    aFileText.AddEntry("File");
    aFileText.Def()
        << rFile.LocalName();

    CurOut() << new HorizontalLine;
}

void
PageMaker_Class::Write_ChildLists()
{
    int i_max = 3 * cl_MAX;
    for (int i = 0; i < i_max; i++)
    {
        bChildLists_Exist[i] = false;
    }  // end for

    csi::html::DefListDefinition &
            rPublic = Setup_MemberSegment_Out( mp_public );
    csi::html::DefListDefinition &
            rProtected = Setup_MemberSegment_Out( mp_protected );
    csi::html::DefListDefinition &
            rPrivate = Setup_MemberSegment_Out( mp_private );

    Write_ChildList_forClasses( rPublic,
                                rProtected,
                                rPrivate,
                                C_sLabel_Classes,
                                C_sTitle_InnerClasses,
                                ary::cpp::CK_class );
    Write_ChildList_forClasses( rPublic,
                                rProtected,
                                rPrivate,
                                C_sLabel_Structs,
                                C_sTitle_InnerStructs,
                                ary::cpp::CK_struct );
    Write_ChildList_forClasses( rPublic,
                                rProtected,
                                rPrivate,
                                C_sLabel_Unions,
                                C_sTitle_InnerUnions,
                                ary::cpp::CK_union );

    Write_ChildList( ary::cpp::Class::SLOT_Enums,
                     cl_Enums,
                     C_sLabel_Enums,
                     C_sTitle_Enums,
                     rPublic,
                     rProtected,
                     rPrivate );
    Write_ChildList( ary::cpp::Class::SLOT_Typedefs,
                     cl_Typedefs,
                     C_sLabel_Typedefs,
                     C_sTitle_Typedefs,
                     rPublic,
                     rProtected,
                     rPrivate );

    Write_ChildList( ary::cpp::Class::SLOT_Operations,
                     cl_Operations,
                     C_sLabel_Operations,
                     C_sTitle_Methods,
                     rPublic,
                     rProtected,
                     rPrivate );
    Write_ChildList( ary::cpp::Class::SLOT_StaticOperations,
                     cl_StaticOperations,
                     C_sLabel_StaticOperations,
                     C_sTitle_StaticMethods,
                     rPublic,
                     rProtected,
                     rPrivate );
    Write_ChildList( ary::cpp::Class::SLOT_Data,
                     cl_Data,
                     C_sLabel_Variables,
                     C_sTitle_Data,
                     rPublic,
                     rProtected,
                     rPrivate );
    Write_ChildList( ary::cpp::Class::SLOT_StaticData,
                     cl_StaticData,
                     C_sLabel_StaticVariables,
                     C_sTitle_StaticData,
                     rPublic,
                     rProtected,
                     rPrivate );

    Create_NaviSubRow(mp_public);       // Also puts out or deletes pPublic.
    Create_NaviSubRow(mp_protected);    // Also puts out or deletes pProtected.
    Create_NaviSubRow(mp_private);      // Also puts out or deletes pPrivate.
}

void
PageMaker_Class::Write_ChildList( ary::SlotAccessId   i_nSlot,
                                  E_ChidList          i_eChildListIndex,
                                  const char *        i_sLabel,
                                  const char *        i_sListTitle,
                                  csi::xml::Element & o_rPublic,
                                  csi::xml::Element & o_rProtected,
                                  csi::xml::Element & o_rPrivate )

{
    bool    bPublic_ChildrenExist = false;
    bool    bProtected_ChildrenExist = false;
    bool    bPrivate_ChildrenExist = false;

    ChildList_Display::Area_Result
            aPublic_Result( bPublic_ChildrenExist, o_rPublic );
    ChildList_Display::Area_Result
            aProtected_Result( bProtected_ChildrenExist, o_rProtected );
    ChildList_Display::Area_Result
            aPrivate_Result( bPrivate_ChildrenExist, o_rPrivate );

    String sLabelPublic = ChildListLabel(i_sLabel, mp_public);
    String sLabelProtected = ChildListLabel(i_sLabel, mp_protected);
    String sLabelPrivate = ChildListLabel(i_sLabel, mp_private);

    pChildDisplay->Run_Members( aPublic_Result,
                                aProtected_Result,
                                aPrivate_Result,
                                i_nSlot,
                                sLabelPublic,
                                sLabelProtected,
                                sLabelPrivate,
                                i_sListTitle );

    bChildLists_Exist[i_eChildListIndex]
                = bPublic_ChildrenExist;
    bChildLists_Exist[i_eChildListIndex + cl_MAX]
                = bProtected_ChildrenExist;
    bChildLists_Exist[i_eChildListIndex + 2*cl_MAX]
                = bPrivate_ChildrenExist;

    if (bPublic_ChildrenExist)
        o_rPublic << new HorizontalLine;
    if (bProtected_ChildrenExist)
        o_rProtected << new HorizontalLine;
    if (bPrivate_ChildrenExist)
        o_rPrivate << new HorizontalLine;
}

void
PageMaker_Class::Write_ChildList_forClasses( csi::xml::Element &    o_rPublic,
                                             csi::xml::Element &    o_rProtected,
                                             csi::xml::Element &    o_rPrivate,
                                             const char *           i_sLabel,
                                             const char *           i_sListTitle,
                                             ary::cpp::E_ClassKey   i_eFilter )
{
    bool    bPublic_ChildrenExist = false;
    bool    bProtected_ChildrenExist = false;
    bool    bPrivate_ChildrenExist = false;

    ChildList_Display::Area_Result
            aPublic_Result( bPublic_ChildrenExist, o_rPublic );
    ChildList_Display::Area_Result
            aProtected_Result( bProtected_ChildrenExist, o_rProtected );
    ChildList_Display::Area_Result
            aPrivate_Result( bPrivate_ChildrenExist, o_rPrivate );

    String sLabelPublic = ChildListLabel(i_sLabel, mp_public);
    String sLabelProtected = ChildListLabel(i_sLabel, mp_protected);
    String sLabelPrivate = ChildListLabel(i_sLabel, mp_private);

    pChildDisplay->Run_MemberClasses( aPublic_Result,
                                      aProtected_Result,
                                      aPrivate_Result,
                                      ary::cpp::Class::SLOT_NestedClasses,
                                      sLabelPublic,
                                      sLabelProtected,
                                      sLabelPrivate,
                                      i_sListTitle,
                                      i_eFilter );

    bChildLists_Exist[int(cl_NestedClasses)+int(i_eFilter)]
                = bPublic_ChildrenExist;
    bChildLists_Exist[int(cl_NestedClasses)+int(i_eFilter) + cl_MAX]
                = bProtected_ChildrenExist;
    bChildLists_Exist[int(cl_NestedClasses)+int(i_eFilter) + 2*cl_MAX]
                = bPrivate_ChildrenExist;

    if (bPublic_ChildrenExist)
        o_rPublic << new HorizontalLine;
    if (bProtected_ChildrenExist)
        o_rProtected << new HorizontalLine;
    if (bPrivate_ChildrenExist)
        o_rPrivate << new HorizontalLine;
}

const char *
PageMaker_Class::ChildListLabel( const char * i_sLabel, E_MemberProtection i_eMpr )
{
     static char sResult[100];
    strcpy( sResult, C_sMprPrefixes[i_eMpr] );  // SAFE STRCPY (#100211# - checked)
    strcat( sResult, i_sLabel );                // SAFE STRCAT (#100211# - checked)
    return sResult;
}

csi::html::DefListDefinition &
PageMaker_Class::Setup_MemberSegment_Out( E_MemberProtection i_eMpr )
{
    html::DefList * pDefList = new html::DefList;
    pProtectionArea[i_eMpr] = pDefList;

    pDefList->AddTerm()
        << new html::ClassAttr("subtitle")
        >> *new html::Label( C_sMprPrefixes[i_eMpr] )
                >> *new html::Headline(3)
                        << C_sMprTitles[i_eMpr];
    return pDefList->AddDefinition();
}

void
PageMaker_Class::Create_NaviSubRow( E_MemberProtection i_eMpr )
{
    int nIndexAdd = int(cl_MAX) * int(i_eMpr);

    bool bEmpty = true;
    for (int e = 0; e < cl_MAX; e++)
    {
        if ( bChildLists_Exist[e + nIndexAdd] )
        {
            bEmpty = false;
            break;
        }
    }  // end for
    if (bEmpty)
    {
        pProtectionArea[i_eMpr] = 0;
        return;
    }
    else //
    {
        CurOut() << pProtectionArea[i_eMpr].Release();
    }  // endif

    pNavi->MakeSubRow( C_sMprSummaryTitles[i_eMpr] );
    for (int i = 0; i < cl_MAX; i++)
    {
        pNavi->AddItem( C_sSummaryItems_Titles[i],
                        ChildListLabel( C_sSummaryItems_Labels[i], i_eMpr ),
                        bChildLists_Exist[i+nIndexAdd] );
    }  // end for
}

void
PageMaker_Class::Write_DerivedList()
{
    adcdisp::ExplanationList
        aDeriveds( CurOut() );
    aDeriveds.AddEntry( "Known Derived Classes" );

    if ( Me().KnownDerivatives().Size() == 0 )
    {
        aDeriveds.Def() << "None.";
        return;
    }

    typedef ary::List_Rid  RidList;

    CesConstIterator
        itEnd = Me().KnownDerivatives().End();
    for ( CesConstIterator it = Me().KnownDerivatives().Begin();
          it != itEnd;
          ++it )
    {
        const ary::cpp::CodeEntity &
            rCe = Env().Gate().Ces().Find_Ce(*it);

        aDeriveds.Def()
            >> *new html::Link( Link2Ce(Env(),rCe) )
                << rCe.LocalName();
        aDeriveds.Def()
            << new html::LineBreak;
    }   // end for
}


// ==============  Creating a classes base hierarchy  ====================== //


namespace
{

class Node
{
  public:
                        Node(
                            const ary::cpp::Class &
                                                i_rClass,
                            ary::cpp::Type_id   i_nClassType,
                            const ary::cpp::Gate &
                                                i_rGate,
                            intt                i_nPositionOffset,
                            Node *              io_pDerived = 0,
                            ary::cpp::E_Protection
                                                i_eProtection = ary::cpp::PROTECT_global,
                            bool                i_bVirtual = false );
                        ~Node();

    void                FillPositionList(
                            std::vector< const Node* > &
                                                o_rPositionList ) const;
    void                Write2(
                            csi::xml::Element & o_rOut,
                            const OuputPage_Environment &
                                                i_rEnv ) const;

    intt                BaseCount() const       { return nCountBases; }
    intt                Position() const        { return nPosition; }
    int                 Xpos() const            { return 3*Position(); }
    int                 Ypos() const            { return 2*Position(); }
    const Node *        Derived() const         { return pDerived; }

  private:
    typedef std::vector< DYN Node* > BaseList;

    void                IncrBaseCount();

    // DATA
    BaseList            aBases;
    intt                nCountBases;
    Node *              pDerived;

    String              sName;
    const ary::cpp::Class *
                        pClass;
    ary::cpp::Type_id   nClassType;
    ary::cpp::E_Protection
                        eProtection;
    bool                bVirtual;

    intt                nPosition;
};

void                WriteNodeHierarchy(
                        csi::xml::Element & o_rOut,
                        const OuputPage_Environment &
                                            i_rEnv,
                        const Node &        i_rClass );

const ary::cpp::Class *
                    HereFind_Class(
                        const ary::cpp::Gate &
                                            i_rGate,
                        ary::cpp::Type_id   i_nReferingTypeId );

}   // anonymous namespace

void
PageMaker_Class::Write_BaseHierarchy()
{
    adcdisp::ExplanationList aBases( CurOut() );
    aBases.AddEntry( "Base Classes" );

    if (   Me().BaseClasses().size() == 0 )
    {
        aBases.Def() << "None.";
    }
    else
    {
        Dyn< Node >
            pBaseGraph( new Node(Me(), ary::cpp::Type_id(0), Env().Gate(), 0) );
        WriteNodeHierarchy( aBases.Def(), Env(), *pBaseGraph );
    }
}



namespace
{

void
WriteNodeHierarchy( csi::xml::Element &             o_rOut,
                    const OuputPage_Environment &   i_rEnv,
                    const Node &                    i_rClass )
{
    typedef const Node *            NodePtr;
    typedef std::vector<NodePtr>    NodeList;

    NodeList aPositionList;
    intt nSize = i_rClass.Position()+1;
    aPositionList.reserve(nSize);
    i_rClass.FillPositionList( aPositionList );

    xml::Element &
        rPre = o_rOut
               >> *new xml::AnElement("pre")
                   << new html::StyleAttr("font-family:monospace;");

    for ( int line = 0; line < nSize; ++line )
    {
        char * sLine1 = new char[2 + line*5];
        char * sLine2 = new char[1 + line*5];
        *sLine1 = '\0';
        *sLine2 = '\0';

        bool bBaseForThisLineReached = false;
         for ( int col = 0; col < line; ++col )
        {
            intt nDerivPos = aPositionList[col]->Derived()->Position();

            if ( nDerivPos >= line )
                strcat(sLine1, "  |  ");
            else
                strcat(sLine1, "     ");

            if ( nDerivPos > line )
            {
                strcat(sLine2, "  |  ");
            }
            else if ( nDerivPos == line )
            {
                if (bBaseForThisLineReached)
                    strcat(sLine2, "--+--");
                else
                {
                    bBaseForThisLineReached = true;
                    strcat(sLine2, "  +--");
                }
            }
            else // nDerivPos < line
            {
                if (bBaseForThisLineReached)
                    strcat(sLine2, "-----");
                else
                    strcat(sLine2, "     ");
            }
        }  // end for (col)
        strcat(sLine1,"\n");
        rPre
            << sLine1
            << sLine2;
        delete [] sLine1;
        delete [] sLine2;

        aPositionList[line]->Write2( rPre, i_rEnv );
        rPre << "\n";
    }   // end for (line)
}

const ary::cpp::Class *
HereFind_Class( const ary::cpp::Gate & i_rGate,
                ary::cpp::Type_id      i_nReferingTypeId )
{
    const ary::cpp::CodeEntity *
        pCe = i_rGate.Search_RelatedCe( i_nReferingTypeId );

    if ( pCe != 0 )
    {
        if  ( ary::is_type<ary::cpp::Class>(*pCe) )
        {
            return ary::ary_cast<ary::cpp::Class>(pCe);
        }
        else if ( ary::is_type<ary::cpp::Typedef>(*pCe) )
        {
            const ary::cpp::Typedef *
                pTydef = ary::ary_cast<ary::cpp::Typedef>(pCe);
            return  HereFind_Class( i_rGate, pTydef->DescribingType() );
        }
    }

    static const ary::cpp::Class aClassNull_( "Base class not found",
                                              ary::cpp::Ce_id(0),
                                              ary::cpp::PROTECT_global,
                                              ary::loc::Le_id(0),
                                              ary::cpp::CK_class );
    return &aClassNull_;
}



//*********************        Node        ***********************//

Node::Node( const ary::cpp::Class &         i_rClass,
            ary::cpp::Type_id               i_nClassType,
            const ary::cpp::Gate &          i_rGate,
            intt                            i_nPositionOffset,
            Node *                          io_pDerived,
            ary::cpp::E_Protection          i_eProtection,
            bool                            i_bVirtual )
    :   aBases(),
        nCountBases(0),
        pDerived(io_pDerived),
        pClass(&i_rClass),
        nClassType(i_nClassType),
        eProtection(i_eProtection),
        bVirtual(i_bVirtual),
        nPosition(i_nPositionOffset)
{
    typedef ary::cpp::List_Bases  BList;

    for ( BList::const_iterator it = i_rClass.BaseClasses().begin();
          it != i_rClass.BaseClasses().end();
          ++it )
    {
        const ary::cpp::Class *
                pBaseClass = HereFind_Class( i_rGate, (*it).nId );

        Dyn<Node>
            pBase( new Node(*pBaseClass,
                            (*it).nId,
                            i_rGate,
                            nPosition,
                            this,
                            (*it).eProtection,
                            (*it).eVirtuality == ary::cpp::VIRTUAL_virtual)
                 );
        IncrBaseCount();
        nPosition += pBase->BaseCount() + 1;
        aBases.push_back( pBase.Release() );
    }   // end for
}

Node::~Node()
{
}

void
Node::FillPositionList( std::vector< const Node* > & o_rPositionList ) const
{
    for ( BaseList::const_iterator it = aBases.begin();
          it != aBases.end();
          ++it )
    {
        (*it)->FillPositionList(o_rPositionList);
    }  // end for

    if( o_rPositionList.size() != uintt(Position()) )
    {
        csv_assert(false);
    }
    o_rPositionList.push_back(this);
}

void
Node::Write2( csi::xml::Element &           o_rOut,
              const OuputPage_Environment & i_rEnv ) const
{
    if ( Derived() == 0 )
    {
        o_rOut
            >> *new xml::AnElement("span")
                << new html::ClassAttr("btself")
                << pClass->LocalName();
        return;
    }

    csi::xml::Element *
        pOut = & ( o_rOut >> *new xml::AnElement("span") );
    switch ( eProtection )
    {
         case ary::cpp::PROTECT_public:
                     if (bVirtual)
                        *pOut << new html::ClassAttr("btvpubl");
                    else
                        *pOut << new html::ClassAttr("btpubl");
                    break;
        case ary::cpp::PROTECT_protected:
                     if (bVirtual)
                        *pOut << new html::ClassAttr("btvprot");
                    else
                        *pOut << new html::ClassAttr("btprot");
                    break;
        case ary::cpp::PROTECT_private:
                     if (bVirtual)
                        *pOut << new html::ClassAttr("btvpriv");
                    else
                        *pOut << new html::ClassAttr("btpriv");
                    break;
        default:    // do nothing.
                    ;
    }   // end switch

    csi::xml::Element & rOut = *pOut;

    Get_LinkedTypeText( rOut, i_rEnv, nClassType, false );
    rOut << " (";
    if ( bVirtual )
        rOut << "virtual ";
    switch ( eProtection )
    {
         case ary::cpp::PROTECT_public:
                    rOut << "public)";
                    break;
         case ary::cpp::PROTECT_protected:
                    rOut << "protected)";
                    break;
         case ary::cpp::PROTECT_private:
                    rOut << "private)";
                    break;
        default:    // do nothing.
                    ;
    }   // end switch
}

void
Node::IncrBaseCount()
{
    ++nCountBases;
    if (pDerived != 0)
        pDerived->IncrBaseCount();
}


}   // anonymous namespace


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
