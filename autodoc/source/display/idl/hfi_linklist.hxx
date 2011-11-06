/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


