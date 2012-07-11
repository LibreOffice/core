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

#ifndef ADC_SEMNODE_HXX
#define ADC_SEMNODE_HXX

// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/qualiname.hxx>


namespace ary
{
namespace idl
{
    class Gate;
}   // namespace idl
}   // namespace ary


namespace csi
{
namespace uidl
{


/** is an implementation class for UnoIDL_PE s
*/
class SemanticNode : private TokenProcessing_Types
{
  public:
                        SemanticNode();
    void                EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::idl::Gate &    io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~SemanticNode();

    void                SetTokenResult(
                            E_TokenDone         i_eDone,
                            E_EnvStackAction    i_eWhat2DoWithEnvStack,
                            UnoIDL_PE *         i_pParseEnv2Push = 0 );
    UnoIDL_PE *         Parent() const          { return pParentPE; }
    ary::idl::Gate &    AryGate() const         { return *pAryGate; }
    TokenProcessing_Result &
                        TokenResult() const     { return *pTokenResult; }

  private:
    // DATA
    UnoIDL_PE *         pParentPE;
    ary::idl::Gate *    pAryGate;
    TokenProcessing_Result *
                        pTokenResult;
};


}   // namespace uidl
}   // namespace csi

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
