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

#ifndef ADC_DISPLAY_HI_ARY_HXX
#define ADC_DISPLAY_HI_ARY_HXX


// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
#include <ary/idl/i_types4idl.hxx>
    // PARAMETERS
#include <ary/idl/i_gate.hxx>
#include <ary/doc/d_docu.hxx>
#include <ary/doc/d_oldidldocu.hxx>


namespace ary
{
namespace idl
{
    class Module;
    class Gate;
    class CePilot;
    class TypePilot;
}
}




inline const ary::doc::OldIdlDocu *
Get_IdlDocu(const ary::doc::Documentation & i_doc)
{
    return dynamic_cast< const ary::doc::OldIdlDocu* >(i_doc.Data());
}





/** A helper class to wrap the access to data in the Autodoc Repository.
*/
class AryAccess
{
  public:
    // LIFECYCLE
                        AryAccess(
                            const ary::idl::Gate &
                                                i_rGate );
    // INQUIRY
    const ary::idl::Module &
                        GlobalNamespace() const;
    const ary::idl::Module &
                        Find_Module(
                            ary::idl::Ce_id     i_ce ) const;
    const ary::idl::CodeEntity &
                        Find_Ce(
                            ary::idl::Ce_id     i_ce ) const;
    const ary::idl::Type &
                        Find_Type(
                            ary::idl::Type_id   i_type ) const;
    ary::idl::Ce_id     CeFromType(
                            ary::idl::Type_id   i_type ) const;
    bool                IsBuiltInOrRelated(
                            const ary::idl::Type &
                                                i_type ) const;
    bool                Search_Ce(
                            StringVector &      o_module,
                            String &            o_mainEntity,
                            String &            o_memberEntity,
                            const char *        i_sText,
                            const ary::idl::Module &
                                                i_referingScope ) const;
    bool                Search_CesModule(
                            StringVector &      o_module,
                            const String &      i_scope,
                            const String &      i_ce,
                            const ary::idl::Module &
                                                i_referingScope ) const;
    const ary::idl::Module *
                        Search_Module(
                            const StringVector &
                                                i_nameChain ) const;

    void                Get_CeText(
                            StringVector &      o_module,
                            String &            o_ce,
                            String &            o_member,
                            const ary::idl::CodeEntity &
                                                i_ce ) const;
    void                Get_TypeText(
                            StringVector &      o_module,
                            String &            o_sCe,
                            ary::idl::Ce_id  &  o_nCe,
                            int &               o_sequenceCount,
                            const ary::idl::Type &
                                                i_type ) const;
    void                Get_IndexData(
                            std::vector<ary::idl::Ce_id> &
                                                o_data,
                            ary::idl::alphabetical_index::E_Letter
                                                i_letter ) const;

    const ary::idl::CePilot &
                        Ces() const;
  private:
    const ary::idl::Module *
                        find_SubModule(
                            const ary::idl::Module &
                                                i_parent,
                            const String &      i_name ) const;

    /// Gets "::"-separated names out of a string.
    bool                nextName(
                            const char * &      io_TextPtr,
                            String &            o_name ) const;


    const ary::idl::Gate &
                        gate() const;
    const ary::idl::CePilot &
                        ces() const;
    const ary::idl::TypePilot &
                        types() const;
    // DATA
    const ary::idl::Gate &
                        rGate;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
