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



#ifndef LUIDL_PE_SELEM_HXX
#define LUIDL_PE_SELEM_HXX



// USED SERVICES
    // BASE CLASSES
#include <s2_luidl/parsenv2.hxx>
#include <s2_luidl/pestate.hxx>
    // COMPONENTS
    // PARAMETERS
#include <ary/idl/i_gate.hxx>


namespace udm {
class Agent_Struct;
}   // namespace udm


namespace csi
{
namespace uidl
{

class PE_Type;
class StructElement;
class Struct;

class PE_StructElement : public UnoIDL_PE,
                         public ParseEnvState
{
  public:
    typedef ary::idl::Ce_id       RStructElement;
    typedef ary::idl::Ce_id       RStruct;

                        PE_StructElement(       /// Use for Struct-elements
                            RStructElement &    o_rResult,
                            const RStruct &     i_rCurStruct,
                            const String &      i_rCurStructTemplateParam );
                        PE_StructElement(       /// Use for Exception-elements
                            RStructElement &    o_rResult,
                            const RStruct &     i_rCurExc );
    virtual void        EstablishContacts(
                            UnoIDL_PE *         io_pParentPE,
                            ary::Repository &   io_rRepository,
                            TokenProcessing_Result &
                                                o_rResult );
                        ~PE_StructElement();

    virtual void        ProcessToken(
                            const Token &       i_rToken );

    virtual void        Process_Default();
    virtual void        Process_Identifier(
                            const TokIdentifier &
                                                i_rToken );
    virtual void        Process_Punctuation(
                            const TokPunctuation &
                                                i_rToken );

  private:
    enum E_State
    {
        e_none,
        expect_type,
        expect_name,
        expect_finish
    };

    virtual void        InitData();
    virtual void        TransferData();
    virtual UnoIDL_PE & MyPE();

    ary::idl::Type_id   lhf_FindTemplateParamType() const;

    // DATA
    E_State             eState;
    RStructElement *    pResult;
    const RStruct *     pCurStruct;
    bool                bIsExceptionElement;

    Dyn<PE_Type>        pPE_Type;
    ary::idl::Type_id   nType;
    String              sName;
    const String *      pCurStructTemplateParam;
};



}   // namespace uidl
}   // namespace csi


#endif
