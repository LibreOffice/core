/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef ADC_DISPLAY_HI_FACTORY_HXX
#define ADC_DISPLAY_HI_FACTORY_HXX


// USED SERVICES
    // BASE CLASSES
#include <toolkit/htmlfactory.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/stdconstiter.hxx>
#include <ary/idl/i_types4idl.hxx>
#include <toolkit/out_position.hxx>


namespace ary
{
namespace doc
{
    class OldIdlDocu;
}
}


class HtmlEnvironment_Idl;
class HF_TitleTable;
class HF_SubTitleTable;


class HtmlFactory_Idl : public HtmlFactory<HtmlEnvironment_Idl>
{
  public:
    enum E_MemberViewType
    {
        viewtype_summary,   // the summary of the members
        viewtype_details,   // the details of the members
        viewtype_complete   // everything
    };

    enum E_DocType
    {
        doctype_summaryOnly,    // only the summary
        doctype_complete        // the complete documentation
    };

  public:
    typedef ary::idl::CodeEntity                client;
    typedef ary::idl::Ce_id                     ce_id;
    typedef ary::idl::Type_id                   type_id;
    typedef ary::doc::OldIdlDocu                ce_info;

    typedef ary::Dyn_StdConstIterator<ce_id>    dyn_ce_list;
    typedef ary::Dyn_StdConstIterator<type_id>  dyn_type_list;
    typedef ary::StdConstIterator<ce_id>        ce_list;
    typedef ary::StdConstIterator<type_id>      type_list;

    typedef HtmlEnvironment_Idl                 Environment;
    typedef output::Position                    OutPosition;

  protected:
                        HtmlFactory_Idl(
                            Environment &       io_rEnv,
                            Xml::Element *      o_pOut = 0 )
                            :   HtmlFactory<Environment>(io_rEnv, o_pOut)
                            { }
    virtual             ~HtmlFactory_Idl() {}

    /** The default version only calls ->produce_InternalLink().
        This may be overwritten by derived classes.
    */
    virtual void        produce_SummaryDeclaration(
                            Xml::Element &      o_row,
                            const client &      i_ce ) const;
    void                produce_InternalLink(
                            Xml::Element &      o_row,
                            const client &      i_ce ) const;
    void                produce_ShortDoc(
                            Xml::Element &      o_row,
                            const client &      i_ce ) const;

    // KORR_FUTURE: Does not belong here (implementation inheritance)!
    void                produce_Bases(
                                Xml::Element &   o_screen,
                                const client &   i_ce,
                                const String &   i_sLabel ) const;
    void                produce_Members(
                            ce_list &               it_list,
                            const String &          i_summaryTitle,
                            const String &          i_summaryLabel,
                            const String &          i_detailsTitle,
                            const String &          i_detailsLabel,
                            const E_MemberViewType  i_viewType = viewtype_complete ) const;

    void                produce_Title(
                            HF_TitleTable &     o_title,
                            const String &      i_label,
                            const client &      i_ce ) const;
    void                get_Annotations(
                            StreamStr &         o_out,
                            const client &      i_ce ) const;

    /// Writes complete docu in standard format.
    void                write_Docu(
                            Xml::Element &      o_screen,
                            const client &      i_ce ) const;

    void                write_ManualLinks(
                            Xml::Element &      o_screen,
                            const client &      i_ce ) const;
  private:
    // Dummy does nothing
    virtual void        produce_MemberDetails(
                            HF_SubTitleTable &  o_table,
                            const client &      i_ce ) const;
    void                recursive_ShowBases(
                            Xml::Element &      o_screen,
                            type_id             i_baseType,
                            int &               io_nDepth ) const;
    type_id             baseOf(
                            const client &      i_ce ) const
                                                { return inq_BaseOf(i_ce); }
    virtual type_id     inq_BaseOf(
                            const client &      i_ce ) const;
};


extern const String
    C_sCellStyle_SummaryLeft;
extern const String
    C_sCellStyle_SummaryRight;
extern const String
    C_sCellStyle_MDetail;
extern const String
    C_sMemberTitle;


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
