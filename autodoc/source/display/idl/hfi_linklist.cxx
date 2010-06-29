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
#include "hfi_linklist.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/idl/i_ce.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/ip_ce.hxx>
#include <ary/idl/ip_type.hxx>
#include <toolkit/hf_docentry.hxx>
#include <toolkit/hf_title.hxx>
#include "hfi_doc.hxx"
#include "hfi_tag.hxx"
#include "hfi_typetext.hxx"
#include "hi_ary.hxx"
#include "hi_env.hxx"




//*******************           HF_CommentedLink_Table         **********************************//

HF_CommentedLink_Table::HF_CommentedLink_Table( Environment  &          io_rEnv,
                                                Xml::Element &          o_rOut,
                                                const String &          i_sTitle,
                                                const String &          i_sLabel,
                                                bool                    i_bBorder )
    :    HtmlFactory_Idl(io_rEnv,&o_rOut),
         pTable( new Html::Table( (i_bBorder ? "1" : "0"), "100%", "5", "0") ),
         pCurLinkColumn(0),
         pCurCommentColumn(0)
{
    *pTable
        << new Html::ClassAttr("commentedlinks");

    CurOut()
        >> *new Html::Label(i_sLabel)
           << new Html::LineBreak;
    CurOut()
        << pTable;
//    HF_SubTitle aTitle(*pTable);
//    aTitle.Produce_it(i_sTitle);
}

HF_CommentedLink_Table::~HF_CommentedLink_Table()
{
}

void
HF_CommentedLink_Table::Add_Line()
{
    Html::TableRow &
        rRow = pTable->AddRow();

    pCurLinkColumn = & (rRow.AddCell()
                            << new Html::WidthAttr("30%")
                            << new Xml::AnAttribute("valign","top") );
    pCurCommentColumn  = & rRow.AddCell();
}

Xml::Element &
HF_CommentedLink_Table::Cur_LinkColumn()
{
    csv_assert(pCurLinkColumn != 0);
    return *pCurLinkColumn;
}

Xml::Element &
HF_CommentedLink_Table::Cur_CommentColumn()
{
    csv_assert(pCurCommentColumn != 0);
    return *pCurCommentColumn;
}


//*******************           HF_MemberTable         **********************************//

HF_MemberTable::HF_MemberTable( Environment  &      io_rEnv,
                                Xml::Element &      o_rOut,
                                const String &      i_sTitle,
                                const String &      i_sLabel,
                                bool                i_bInline )
    :    HtmlFactory_Idl(io_rEnv,&o_rOut),
         pTable( new Html::Table("1", "100%", "5", "0") ),
         pCurDeclaration(0),
         pCurDescription(0),
         bInline(i_bInline)
{
    *pTable
        << new Html::ClassAttr("memberlist");

    CurOut()
        >> *new Html::Label(i_sLabel)
           << new Html::LineBreak;
    CurOut()
        << pTable;
//    HF_SubTitle aTitle(*pTable);
//    aTitle.Produce_it(i_sTitle);
}

HF_MemberTable::~HF_MemberTable()
{
}

void
HF_MemberTable::Add_Line()
{
    if (bInline)
    {
        Html::TableRow & rRow = pTable->AddRow();

        pCurDeclaration = &( rRow.AddCell()
                                << new Xml::AnAttribute("valign","top")
                                << new Html::WidthAttr("30%") );
        pCurDescription = & rRow.AddCell();
    }
    else
    {
        Html::DefList *
            pMemberSpace = new Html::DefList;
        *pMemberSpace
            << new Html::ClassAttr("member");

        pTable->AddRow().AddCell() << pMemberSpace;

        pCurDeclaration =
            & ( *pMemberSpace
                    >> *new Html::DefListTerm
                        << new Html::ClassAttr("member") );
        pCurDescription =
            & ( *pMemberSpace
                    >> *new Html::DefListDefinition()
                        << new Html::ClassAttr("member") );
    }
}

Xml::Element &
HF_MemberTable::Cur_Declaration()
{
    csv_assert(pCurDeclaration != 0);
    return *pCurDeclaration;
}

Xml::Element &
HF_MemberTable::Cur_Description()
{
    csv_assert(pCurDescription != 0);
    return *pCurDescription;
}



//*******************           HF_IdlLinkList         **********************************//

HF_IdlLinkList::HF_IdlLinkList( Environment  &   io_rEnv,
                                Xml::Element *   o_pOut )
    :    HtmlFactory_Idl(io_rEnv,o_pOut)
{
}

HF_IdlLinkList::~HF_IdlLinkList()
{
}

void
HF_IdlLinkList::Produce_NamespaceMembers( const String &                        i_sTitle,
                                          const String &                        i_sLabel,
                                          const std::vector<ary::idl::Ce_id> &  i_rList,
                                          bool                                  i_bNestedNamespaces ) const
{
    HF_CommentedLink_Table
        aTableMaker( Env(), CurOut(),
                     i_sTitle, i_sLabel,
                     true );

    std::vector<ary::idl::Ce_id>::const_iterator itEnd = i_rList.end();
    for ( std::vector<ary::idl::Ce_id>::const_iterator it = i_rList.begin();
          it != itEnd;
          ++it )
    {
        static String   sEntryName;
        static String   sEntryLink;
        const ce_info *
                        pDocu = 0;
        Get_EntryData_NamespaceMembers( sEntryName, sEntryLink, pDocu, *it, i_bNestedNamespaces );
        aTableMaker.Add_Line();

        aTableMaker.Cur_LinkColumn()
           >> *new Html::Link(sEntryLink)
              << sEntryName;

        if ( pDocu != 0 )
        {
            HF_IdlShortDocu
                aTextDisplay(Env(), aTableMaker.Cur_CommentColumn() );
            aTextDisplay.Produce_byData( pDocu );
        }
    }   // end for
}

void
HF_IdlLinkList::Produce_GlobalLinks( const String &      i_sTitle,
                                     const String &      i_sLabel,
                                     ce_list &           i_rList ) const
{
    HF_CommentedLink_Table
        aTableMaker( Env(), CurOut(),
                     i_sTitle, i_sLabel,
                     true );

    for ( ; i_rList; ++i_rList )
    {
        aTableMaker.Add_Line();
        HF_IdlTypeText
            aLinkText( Env(), aTableMaker.Cur_LinkColumn(), true );
        aLinkText.Produce_byData(*i_rList);

        const ce_info *
            pDocu = Get_EntryDocu(*i_rList);
        if ( pDocu != 0 )
        {
            HF_IdlShortDocu
                aTextDisplay(Env(), aTableMaker.Cur_CommentColumn() );
            aTextDisplay.Produce_byData( pDocu, *i_rList );
        }
    }
}

void
HF_IdlLinkList::Produce_GlobalCommentedLinks( const String &          i_sTitle,
                                              const String &          i_sLabel,
                                              comref_list &           i_rList ) const
{
    HF_CommentedLink_Table
        aTableMaker( Env(), CurOut(),
                     i_sTitle, i_sLabel,
                     true );
/*
    for ( ; i_rList; ++i_rList )
    {
        aTableMaker.Add_Line();
        HF_IdlTypeText
            aLinkText( Env(), aTableMaker.Cur_LinkColumn(), true );
        aLinkText.Produce_byData( (*i_rList).first );

        HF_DocEntryList
            aDocList( aTableMaker.Cur_CommentColumn() );
        if ( (*i_rList).second != 0 )
        {
            HF_IdlDocu
                aDocuDisplay( Env(), aDocList );
            aDocuDisplay.Produce_byData( (*i_rList).second );
        }
        else
        {
            const ce_info *
                pShort = Get_EntryDocu(
                            Env().Gate().Types().Search_CeRelatedTo(
                                            (*i_rList).first) );
            if ( pShort != 0 )
            {
                if (pShort->IsDeprecated())
                {
                    aDocList.Produce_Term()
                        << "[ DEPRECATED ]";
                }
                if (pShort->IsOptional())
                {
                    aDocList.Produce_Term()
                        << "[ OPTIONAL ]";
                }

                aDocList.Produce_Term()
                    << "Description";

                HF_IdlDocuTextDisplay
                    aShortDisplay( Env(), &aDocList.Produce_Definition() );
                aShortDisplay.Set_CurScopeTo(
                        Env().Gate().Types().Search_CeRelatedTo((*i_rList).first) );
                pShort->Short().DisplayAt(aShortDisplay);
            }
        }
    }
*/
}

void
HF_IdlLinkList::Produce_MemberLinks( const String &     i_sTitle,
                                     const String &     i_sLabel,
                                     ce_list &          i_rList ) const
{
    HF_CommentedLink_Table
        aTableMaker( Env(), CurOut(),
                     i_sTitle, i_sLabel,
                     true );

/*
    for ( ; i_rList; ++i_rList )
    {
        const ary::idl::CodeEntity &
            rCe = Env().Gate().Ces().Find_Ce(*i_rList);

        aTableMaker.Add_Line();
        aTableMaker.Cur_LinkColumn()
            >> *new Html::Link(
                    StreamLock(200)() << "#" << rCe.LocalName() << c_str)
               << rCe.LocalName();

        const ce_info *
            pDocu = rCe.Docu();
        if ( pDocu != 0 )
        {
            HF_IdlShortDocu
                aTextDisplay(Env(), aTableMaker.Cur_CommentColumn() );
            aTextDisplay.Produce_byData( *pDocu );
        }
    }   // end for
*/
}

void
HF_IdlLinkList::Get_EntryData_NamespaceMembers(
                            String &                o_sEntryName,
                            String &                o_sEntryLink,
                            const ce_info * &       o_pDocu,
                            ce_id                   i_nMemberId,
                            bool                    i_bIsNestedNamespace  ) const
{
    const ary::idl::CodeEntity &
        rCe = Env().Data().Find_Ce(i_nMemberId);

    o_sEntryName = rCe.LocalName();
    o_sEntryLink = StreamLock(200)() << rCe.LocalName()
                                     << ( i_bIsNestedNamespace
                                                ?   "/module-ix"
                                                :   "" )
                                     << ".html"
                                     << c_str;
    o_pDocu = rCe.Docu();
}

const ary::doc::OldIdlDocu *
HF_IdlLinkList::Get_EntryDocu(ce_id i_nMemberId) const
{
    if (i_nMemberId.IsValid())
        return Env().Data().Find_Ce(i_nMemberId).Docu();
    else
        return 0;
}


