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

#ifndef LUIDL_PARSENV2_HXX
#define LUIDL_PARSENV2_HXX


// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/tokproct.hxx>
    // COMPONENTS
#include <s2_luidl/semnode.hxx>
    // PARAMETERS
#include <ary/idl/i_types4idl.hxx>
#include <ary/idl/i_module.hxx>



class ParserInfo;

namespace ary
{
    class Repository;

namespace doc
{
    class OldIdlDocu;
}

namespace idl
{
    class CodeEntity;
}
}



namespace csi
{
namespace uidl
{


class Token;
class SemanticNode;


class UnoIDL_PE : virtual protected TokenProcessing_Types
{
  public:
    virtual             ~UnoIDL_PE();

    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &
                                                io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
//  virtual void        EstablishContacts(
//                          UnoIDL_PE *         io_pParentPE,
//                          ary::idl::Gate &
//                                                io_rGate,
//                          TokenProcessing_Result &
//                                              o_rResult );
    virtual void        Enter(
                            E_EnvStackAction    i_eWayOfEntering );
    virtual void        Leave(
                            E_EnvStackAction    i_eWayOfLeaving );
    virtual void        ProcessToken(
                            const Token &       i_rToken ) = 0;

    void                SetDocu(
                            DYN ary::doc::OldIdlDocu *
                                                let_dpDocu );
    void                SetPublished();
    void                SetOptional();
    void                PassDocuAt(
                            ary::idl::CodeEntity &
                                                io_rCe );

    UnoIDL_PE *         Parent() const          { return aMyNode.Parent(); }

    void                SetResult(
                            E_TokenDone         i_eDone,
                            E_EnvStackAction    i_eWhat2DoWithEnvStack,
                            UnoIDL_PE *         i_pParseEnv2Push = 0 )
                                                { aMyNode.SetTokenResult( i_eDone, i_eWhat2DoWithEnvStack, i_pParseEnv2Push ); }
    virtual const ary::idl::Module &
                        CurNamespace() const;
    virtual const ParserInfo &
                        ParseInfo() const;
    ary::idl::Gate &    Gate() const            { return aMyNode.AryGate(); }
    TokenProcessing_Result &
                        TokenResult() const     { return aMyNode.TokenResult(); }
    DYN ary::doc::OldIdlDocu *
                        ReleaseDocu()           { return pDocu.Release(); }
  protected:
                        UnoIDL_PE();
    ary::Repository &   MyRepository()          { csv_assert(pRepository != 0);
                                                  return *pRepository;  }
  private:
    virtual void        InitData();
    virtual void        TransferData() = 0;
    virtual void        ReceiveData();

    SemanticNode        aMyNode;
    Dyn<ary::doc::OldIdlDocu>
                        pDocu;
    ary::Repository *   pRepository;
};




}   // namespace uidl
}   // namespace csi
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
