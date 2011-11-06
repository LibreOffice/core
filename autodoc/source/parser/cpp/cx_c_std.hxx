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



#ifndef ADC_CPP_CX_C_STD_HXX
#define ADC_CPP_CX_C_STD_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
#include "cx_base.hxx"
    // COMPONENTS
#include <tokens/tkpstama.hxx>
    // PARAMETERS



namespace cpp {

class Context_Comment;

/**
*/
class Context_CppStd : public Cx_Base,
                       private StateMachineContext
{
  public:
    //  LIFECYCLE
                        Context_CppStd(
                            DYN autodoc::TkpDocuContext &
                                                let_drContext_Docu  );
                        ~Context_CppStd();
    //  OPERATIONS
    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual void        AssignDealer(
                            Distributor &       o_rDealer );
  private:
    //  SERVICE FUNCTIONS
    void                PerformStatusFunction(
                            uintt               i_nStatusSignal,
                            StmArrayStatus::F_CRTOK
                                                i_fTokenCreateFunction,
                            CharacterSource &   io_rText );
    void                SetupStateMachine();

    //  DATA
    StateMachine        aStateMachine;

        // Contexts
    Dyn<autodoc::TkpDocuContext>
                        pDocuContext;

    Dyn<Context_Comment>
                        pContext_Comment;
    Dyn<Cx_Base>        pContext_Preprocessor;
    Dyn<Cx_Base>        pContext_ConstString;
    Dyn<Cx_Base>        pContext_ConstChar;
    Dyn<Cx_Base>        pContext_ConstNumeric;
    Dyn<Cx_Base>        pContext_UnblockMacro;
};



}   // namespace cpp


#endif

