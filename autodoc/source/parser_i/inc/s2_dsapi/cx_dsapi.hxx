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



#ifndef ADC_CX_DSAPI_HXX
#define ADC_CX_DSAPI_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcont2.hxx>
    // COMPONENTS
#include <cosv/tpl/dyn.hxx>
#include <tokens/tkpstam2.hxx>
    // PARAMETERS


namespace csi
{
namespace dsapi
{

class Token_Receiver;
class Token;

class Cx_EoHtml;
class Cx_EoXmlConst;
class Cx_EoXmlLink_BeginTag;
class Cx_EoXmlLink_EndTag;
class Cx_EoXmlFormat_BeginTag;
class Cx_EoXmlFormat_EndTag;
class Cx_CheckStar;

/**
@descr
*/
class Context_Docu : public TkpDocuContext,
                     private StateMachineContext
{
  public:
    //  LIFECYCLE
                        Context_Docu(
                            Token_Receiver &    o_rReceiver );
    virtual void        SetParentContext(
                            TkpContext &        io_rParentContext,
                            const char *        i_sMultiLineEndToken );

                        ~Context_Docu();
    //  OPERATIONS
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
                            UINT16              i_nTokenId,
                            CharacterSource &   io_rText );

    void                SetupStateMachine();

    //  DATA
    StateMachin2        aStateMachine;
    Token_Receiver *    pReceiver;

        // Contexts
    TkpContext *        pParentContext;
    String              sMultiLineEndToken;

    Dyn<Cx_EoHtml>      pCx_EoHtml;
    Dyn<Cx_EoXmlConst>  pCx_EoXmlConst;
    Dyn<Cx_EoXmlLink_BeginTag>
                        pCx_EoXmlLink_BeginTag;
    Dyn<Cx_EoXmlLink_EndTag>
                        pCx_EoXmlLink_EndTag;
    Dyn<Cx_EoXmlFormat_BeginTag>
                        pCx_EoXmlFormat_BeginTag;
    Dyn<Cx_EoXmlFormat_EndTag>
                        pCx_EoXmlFormat_EndTag;
    Dyn<Cx_CheckStar>   pCx_CheckStar;

        // Temporary data, used during ReadCharChain()
    Dyn<Token>          pNewToken;
    ::TkpContext *      pFollowUpContext;
    bool                bIsMultiline;
};


}   // namespace dsapi
}   // namespace csi


#endif

