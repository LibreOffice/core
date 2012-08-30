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

#ifndef ADC_DISPLAY_HFI_TYPETEXT_HXX
#define ADC_DISPLAY_HFI_TYPETEXT_HXX



// USED SERVICES
    // BASE CLASSES
#include "hi_factory.hxx"
    // COMPONENTS
    // PARAMETERS

namespace ary
{
namespace idl
{
    class Module;
}
}

class HF_IdlTypeText : public HtmlFactory_Idl
{
  public:
    enum E_Index { use_for_javacompatible_index };

                        HF_IdlTypeText(
                            Environment &       io_rEnv,
                            Xml::Element &      o_rOut,
                            const client *      i_pScopeGivingCe = 0 );
                        HF_IdlTypeText(
                            Environment &       io_rEnv,
                            E_Index             e );
    virtual             ~HF_IdlTypeText();

    void                Produce_byData(
                            ary::idl::Type_id   i_idType ) const;
    void                Produce_byData(
                            ary::idl::Ce_id     i_idCe ) const;
    void                Produce_byData(
                            const String &      i_sFullName ) const;
    void                Produce_LinkInDocu(
                            const String &      i_scope,
                            const String &      i_name,
                            const String &      i_member ) const;
    void                Produce_LocalLinkInDocu(
                            const String &      i_member ) const;

    /// Produce the first link for Java-help understood index entries.
    void                Produce_IndexLink(
                            Xml::Element &      o_out,
                            const client &      i_ce ) const;
    /** Produce the second link for Java-help understood index entries.
        For members this will be a link to their owner (this function is
        used), else see @->Produce_IndexSecondEntryLink();
    */
    void                Produce_IndexOwnerLink(
                            Xml::Element &      o_out,
                            const client &      i_owner ) const;
    /** Produce the second link for Java-help understood index entries.
        For non- members this will again be a link to to the entry itself
        (this function is used), else see @->Produce_IndexOwnerLink();
    */
    void                Produce_IndexSecondEntryLink(
                            Xml::Element &      o_out,
                            const client &      i_ce ) const;
  private:
    // Locals
    enum E_Existence
    {
        exists_dontknow,
        exists_yes,
        exists_no
    };

    void                produce_FromStd(
                            const StringVector &
                                                i_module,
                            const String &      i_ce,
                            const String &      i_member,
                            int                 i_sequenceCount,
                            E_Existence         i_ceExists,
                            const std::vector<ary::idl::Type_id> *
                                                i_templateParameters = 0 ) const;
    void                produce_BuiltIn(
                            const String &      i_type,
                            int                 i_sequenceCount ) const;
    void                produce_IndexLink(
                            const StringVector &
                                                i_module,
                            const String &      i_ce,
                            const String &      i_member,
                            bool                i_bIsOwner ) const;
    int                 count_Sequences(
                            const char *        i_sFullType ) const;
    void                start_Sequence(
                            int                 i_count ) const;
    void                finish_Sequence(
                            int                 i_count ) const;
    void                errorOut_UnresolvedLink(
                            const char *        i_name ) const;
    void                errorOut_UnresolvedLink(
                            const StringVector &
                                                i_module,
                            const String &      i_ce,
                            const String &      i_member ) const;
    bool                is_ExternLink(
                            const StringVector &
                                                i_module ) const;
    void                produce_ExternLink(
                            const StringVector &
                                                i_module,
                            const String &      i_ce,
                            const String &      i_member,
                            int                 i_sequenceCount,
                            const std::vector<ary::idl::Type_id> *
                                                i_templateParameters ) const;
    void                write_TemplateParameterList(
                            const std::vector<ary::idl::Type_id> &
                                                i_templateParameters ) const;
    const ary::idl::Module *
                        referingModule() const;
    const client *      referingCe() const;

    // DATA
    mutable const client *
                        pReferingCe;
};



// IMPLEMENTATION



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
