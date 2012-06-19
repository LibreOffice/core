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
