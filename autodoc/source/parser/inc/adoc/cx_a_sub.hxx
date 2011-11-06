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



#ifndef ADC_ADOC_CX_A_SUB_HXX
#define ADC_ADOC_CX_A_SUB_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
#include <tokens/tkpstama.hxx>
    // PARAMETERS
#include <tokens/token.hxx>


namespace adoc {




class Cx_LineStart : public TkpContext
{
  public:
                        Cx_LineStart(
                            TkpContext &        i_rFollowUpContext );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();

    void                SetCurToken(
                            TextToken::F_CRTOK  i_fTokenCreateFunction )
                                                { fCur_TokenCreateFunction = i_fTokenCreateFunction; }
    void                AssignDealer(
                            TokenDealer &       o_rDealer )
                                                { pDealer = &o_rDealer; }
  private:
    // DATA
    TokenDealer *       pDealer;
    TkpContext *        pFollowUpContext;

    Dyn<TextToken>      pNewToken;

    TextToken::F_CRTOK  fCur_TokenCreateFunction;
};


/**
@descr
*/

class Cx_CheckStar : public TkpContext
{
  public:
    // LIFECYCLE
                        Cx_CheckStar(
                            TkpContext &        i_rFollowUpContext );
    void                Set_End_FollowUpContext(
                            TkpContext &        i_rEnd_FollowUpContext )
                                                { pEnd_FollowUpContext = &i_rEnd_FollowUpContext; }

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();

    void                SetCanBeEnd(
                            bool                i_bCanBeEnd  )
                                                { bCanBeEnd = i_bCanBeEnd; }
    virtual TkpContext &
                        FollowUpContext();
    void                AssignDealer(
                            TokenDealer &       o_rDealer )
                                                { pDealer = &o_rDealer; }
  private:
    // DATA
    TokenDealer *       pDealer;
    TkpContext *        pFollowUpContext;
    TkpContext *        pEnd_FollowUpContext;

    Dyn<TextToken>      pNewToken;

    bool                bCanBeEnd;
    bool                bEndTokenFound;
};


class Cx_AtTagCompletion : public TkpContext
{
  public:
                        Cx_AtTagCompletion(
                            TkpContext &        i_rFollowUpContext );

    virtual void        ReadCharChain(
                            CharacterSource &   io_rText );
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();

    void                SetCurToken(
                            TextToken::F_CRTOK  i_fTokenCreateFunction )
                                                { fCur_TokenCreateFunction = i_fTokenCreateFunction; }
    void                AssignDealer(
                            TokenDealer &       o_rDealer )
                                                { pDealer = &o_rDealer; }
  private:
    // DATA
    TokenDealer *       pDealer;
    TkpContext *        pFollowUpContext;

    Dyn<TextToken>      pNewToken;

    TextToken::F_CRTOK  fCur_TokenCreateFunction;
};


}   // namespace adoc


#endif

