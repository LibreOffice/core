/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

#include <precomp.h>
#include <s2_luidl/semnode.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/ary.hxx>
#include <ary/idl/i_gate.hxx>
#include <ary/idl/i_module.hxx>
#include <ary/doc/d_oldidldocu.hxx>
#include <s2_luidl/parsenv2.hxx>


namespace csi
{
namespace uidl
{



SemanticNode::SemanticNode()
    :   pParentPE(0),
        pAryGate(0),
        pTokenResult(0)
{
}

void
SemanticNode::EstablishContacts( UnoIDL_PE *                io_pParentPE,
                                 ary::idl::Gate &           io_rGate,
                                 TokenProcessing_Result &   o_rResult )
{
    pParentPE       =  io_pParentPE;
    pAryGate        = &io_rGate;
    pTokenResult    = &o_rResult;
}

SemanticNode::~SemanticNode()
{
}

void
SemanticNode::SetTokenResult( E_TokenDone       i_eDone,
                              E_EnvStackAction  i_eWhat2DoWithEnvStack,
                              UnoIDL_PE *       i_pParseEnv2Push )
{
    csv_assert(pTokenResult != 0);

    pTokenResult->eDone         = i_eDone;
    pTokenResult->eStackAction  = i_eWhat2DoWithEnvStack;
    pTokenResult->pEnv2Push     = i_pParseEnv2Push;
}


}   // namespace uidl
}   // namespace csi
