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
    typedef ::ary::idl::CommentedRelation       comref;
    typedef ::ary::StdConstIterator< comref >   comref_list;

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

    void                produce_Link(
                            Xml::Element &      o_row,
                            type_id             i_type ) const;
    void                produce_LinkSummary(
                            const client &      i_ce,
                            Xml::Element &      o_row,
                            const comref &  i_commentedRef ) const;

    void                produce_MemberDetails(        /// of property
                            HF_SubTitleTable &  o_table,
                            const client &      i_ce ) const;

    void                produce_CommentedRelations(
                            const client &  i_ce,
                            comref_list &   it_list,
                            const String &  i_summaryTitle,
                            const String &  i_summaryLabel,
                            const String &  i_detailsTitle,
                            const String &  i_detailsLabel,
                            const E_DocType i_docType ) const;

};



// IMPLEMENTATION

extern const String
    C_sCePrefix_Service;



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
