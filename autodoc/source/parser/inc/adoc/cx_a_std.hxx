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



#ifndef ADC_ADOC_CX_A_STD_HXX
#define ADC_ADOC_CX_A_STD_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
#include <tokens/tkpstama.hxx>
    // PARAMETERS

class TextToken;


namespace adoc {

class Cx_LineStart;
class Cx_CheckStar;
class Cx_AtTagCompletion;


/**
@descr
*/
class Context_AdocStd : public autodoc::TkpDocuContext,
                        private StateMachineContext
{
  public:
    //  LIFECYCLE
                        Context_AdocStd();
    virtual void        SetParentContext(
                            TkpContext &        io_rParentContext,
                            const char *        i_sMultiLineEndToken );
                        ~Context_AdocStd();

    //  OPERATIONS
    virtual void        AssignDealer(
                            TokenDealer &       o_rDealer );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();
    virtual void        SetMode_IsMultiLine(
                            bool                i_bTrue );
    //  INQUIRY
    virtual TkpContext &
                        FollowUpContext();
  private:
    //  SERVICE FUNCTIONS
    virtual void        PerformStatusFunction(
                            uintt               i_nStatusSignal,
                            F_CRTOK             i_fTokenCreateFunction,
                            CharacterSource &   io_rText );

    void                SetupStateMachine();

    //  DATA
    StateMachine        aStateMachine;
    TokenDealer *       pDealer;

        // Contexts
    TkpContext *        pParentContext;
    TkpContext *        pFollowUpContext;
    Dyn<Cx_LineStart>   pCx_LineStart;
    Dyn<Cx_CheckStar>   pCx_CheckStar;
    Dyn<Cx_AtTagCompletion>
                        pCx_AtTagCompletion;

        // Temporary data, used during ReadCharChain()
    Dyn<TextToken>      pNewToken;
    bool                bIsMultiline;
};


}   // namespace adoc


#endif

