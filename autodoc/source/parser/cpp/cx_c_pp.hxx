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



#ifndef ADC_CPP_CX_C_PP_HXX
#define ADC_CPP_CX_C_PP_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
#include "cx_base.hxx"
    // COMPONENTS
    // PARAMETERS


namespace cpp
{

class Context_Preprocessor : public Cx_Base
{
  public:
                        Context_Preprocessor(
                            TkpContext &        i_rFollowUpContext );
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual void        AssignDealer(
                            Distributor &       o_rDealer );
  private:
    // Locals
    void                ReadDefault(
                            CharacterSource &   io_rText );
    void                ReadDefine(
                            CharacterSource &   io_rText );

    // DATA
    TkpContext *        pContext_Parent;
    Dyn<Cx_Base>        pContext_PP_MacroParams;
    Dyn<Cx_Base>        pContext_PP_Definition;
};

class Context_PP_MacroParams : public Cx_Base
{
  public:
                        Context_PP_MacroParams(
                            Cx_Base &           i_rFollowUpContext );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
  private:
    // DATA
    Cx_Base *           pContext_PP_Definition;
};

class Context_PP_Definition : public Cx_Base
{
  public:
                        Context_PP_Definition(
                            TkpContext &        i_rFollowUpContext );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );

  private:
    // DATA
    TkpContext *        pContext_Parent;
};


}   // namespace cpp

#endif

