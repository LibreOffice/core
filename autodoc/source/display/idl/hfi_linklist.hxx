/*************************************************************************
 *
 *  $RCSfile: hfi_linklist.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2004-07-12 15:26:44 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#include <ary_i/codeinf2.hxx>




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


