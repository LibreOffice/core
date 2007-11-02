/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_linklist.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2007-11-02 16:36:29 $
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


