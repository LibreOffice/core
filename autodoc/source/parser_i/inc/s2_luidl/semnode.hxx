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
    class QualifiedName;
    class Repository;

namespace idl
{
    class Gate;
    class Module;
}   // namespace idl
}   // namespace ary


namespace csi
{
namespace uidl
{


class Struct;
class Token;


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
