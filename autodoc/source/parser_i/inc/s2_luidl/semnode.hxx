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



#ifndef ADC_SEMNODE_HXX
#define ADC_SEMNODE_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/tokproct.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/qualiname.hxx>
// #include <udm/ref.hxx>


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


/*
class Trying_PE
{
  public:
   virtual              ~Trying_PE() {}

  protected:
                        Trying_PE();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    void                StartTry(
                            UnoIDL_PE &         i_rFirstTry );
    void                Add2Try(
                            UnoIDL_PE &         i_rTry );
    bool                AmITrying() const;
    UnoIDL_PE *         NextTry() const;
    void                FinishTry();

  private:
    std::vector<UnoIDL_PE*>
                        aTryableSubEnvironments;
    uintt               nTryCounter;
};

*/


// IMPLEMENTATION


}   // namespace uidl
}   // namespace csi

#endif

