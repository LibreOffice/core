/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hi_factory.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2006-05-03 16:57:28 $
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

#ifndef ADC_DISPLAY_HI_FACTORY_HXX
#define ADC_DISPLAY_HI_FACTORY_HXX


// USED SERVICES
    // BASE CLASSES
#include <toolkit/htmlfactory.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/stdconstiter.hxx>
#include <ary/idl/i_language.hxx>
#include <toolkit/out_position.hxx>


namespace ary
{
    namespace idl
    {
        class Module;
    }
    namespace info
    {
        class CodeInformation;
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
    typedef ary::idl::CodeEntity                client;
    typedef ary::idl::Ce_id                     ce_id;
    typedef ary::idl::Type_id                   type_id;
    typedef ary::info::CodeInformation          ce_info;

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

    // KORR MI: Does not belong here (implementation inheritance)!
    void                produce_Bases(
                                Xml::Element &   o_screen,
                                const client &   i_ce,
                                const String &   i_sLabel ) const;
    void                produce_Members(
                            ce_list &           it_list,
                            const String &      i_summaryTitle,
                            const String &      i_summaryLabel,
                            const String &      i_detailsTitle,
                            const String &      i_detailsLabel ) const;

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
