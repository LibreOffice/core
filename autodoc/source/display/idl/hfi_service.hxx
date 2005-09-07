/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hfi_service.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 17:45:43 $
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

#ifndef ADC_DISPLAY_HFI_SERVICE_HXX
#define ADC_DISPLAY_HFI_SERVICE_HXX


// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_comrela.hxx>

class HF_NaviSubRow;
class HF_SubTitleTable;

class HF_IdlService : public HtmlFactory_Idl
{
  public:
    typedef ::ary::StdConstIterator< ::ary::idl::CommentedRelation> comref_list;
                        HF_IdlService(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut );
    virtual             ~HF_IdlService();

    void                Produce_byData(
                           const client &       i_ce ) const;
   private:
    // Overwritten from HtmlFactory_Idl:
    virtual void        produce_SummaryDeclaration(
                            Xml::Element &      o_row,
                            const client &      i_ce ) const;

    // Locals
    DYN HF_NaviSubRow & make_Navibar(
                            const client &      i_ce ) const;

    void                produce_IncludedServices(
                            const client &      i_ce,
                            comref_list &       it_list ) const;
    void                produce_ExportedInterfaces(
                            const client &      i_ce,
                            comref_list &       it_list ) const;

    void                produce_Link(
                            Xml::Element &      o_row,
                            type_id             i_type ) const;
    void                produce_LinkDoc(
                            const client &      i_ce,
                            Xml::Element &      o_row,
                            comref_list &       i_commentedRef ) const;

    void                produce_MemberDetails(        /// of property
                            HF_SubTitleTable &  o_table,
                            const client &      i_ce ) const;
};



// IMPLEMENTATION

extern const String
    C_sCePrefix_Service;



#endif


