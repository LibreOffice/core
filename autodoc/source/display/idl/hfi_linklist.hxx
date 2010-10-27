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

#ifndef ADC_DISPLAY_HFI_LINKLIST_HXX
#define ADC_DISPLAY_HFI_LINKLIST_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_comrela.hxx>
#include <ary_i/ci_text2.hxx>
#include <ary/doc/d_oldidldocu.hxx>




class HF_CommentedLink_Table : public HtmlFactory_Idl
{
  public:
                        HF_CommentedLink_Table(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut,
                            const String &      i_sTitle,
                            const String &      i_sLabel,
                            bool                i_bBorder = false );
    virtual             ~HF_CommentedLink_Table();

    void                Add_Line();
    Xml::Element &      Cur_LinkColumn();
    Xml::Element &      Cur_CommentColumn();

  private:
    // DATA
    Html::Table *       pTable;
    Xml::Element *      pCurLinkColumn;
    Xml::Element *      pCurCommentColumn;
};

class HF_MemberTable : public HtmlFactory_Idl
{
  public:
                        HF_MemberTable(
                            Environment  &      io_rEnv,
                            Xml::Element &      o_rOut,
                            const String &      i_sTitle,
                            const String &      i_sLabel,
                            bool                i_bInline = false );
    virtual             ~HF_MemberTable();

    void                Add_Line();
    Xml::Element &      Cur_Declaration();
    Xml::Element &      Cur_Description();

  private:
    // DATA
    Html::Table *       pTable;
    Xml::Element *      pCurDeclaration;
    Xml::Element *      pCurDescription;
    bool                bInline;
};




class HF_IdlLinkList : public HtmlFactory_Idl
{
  public:
    typedef ary::StdConstIterator<ary::idl::CommentedRelation>
                                            comref_list;

                        HF_IdlLinkList(
                            Environment &       io_rEnv,
                            Xml::Element *      o_pOut );
    virtual             ~HF_IdlLinkList();

    void                Produce_NamespaceMembers(
                            const String &      i_sTitle,
                            const String &      i_sLabel,
                            const std::vector<ary::idl::Ce_id> &
                                                i_rList,
                            bool                i_bNestedNamespaces = false ) const;
    void                Produce_GlobalLinks(
                            const String &      i_sTitle,
                            const String &      i_sLabel,
                            ce_list &           i_rList ) const;
    void                Produce_GlobalCommentedLinks(
                            const String &      i_sTitle,
                            const String &      i_sLabel,
                            comref_list &       i_rList ) const;
    void                Produce_MemberLinks(
                            const String &      i_sTitle,
                            const String &      i_sLabel,
                            ce_list &           i_rList ) const;
  private:
    void                Get_EntryData_NamespaceMembers(
                            String &            o_sEntryName,
                            String &            o_sEntryLink,
                            const ce_info * &   o_pDocuText,
                            ce_id               i_nMemberId,
                            bool                i_bIsNestedNamespace ) const;
    const ce_info *     Get_EntryDocu(
                            ce_id               i_nMemberId ) const;
};








// IMPLEMENTATION


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
