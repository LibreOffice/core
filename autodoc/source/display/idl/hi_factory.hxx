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
namespace idl
{
    class Module;
}
namespace doc
{
    class OldIdlDocu;
}
}


class HtmlEnvironment_Idl;
class LinkHelper;
class HF_NaviSubRow;
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
