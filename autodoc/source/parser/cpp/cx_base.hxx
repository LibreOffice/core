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



#ifndef ADC_CPP_CX_BASE_HXX
#define ADC_CPP_CX_BASE_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
    // PARAMETERS


class TextToken;


namespace cpp
{

class Distributor;


class Cx_Base : public ::TkpContext
{
  public:
    virtual bool        PassNewToken();
    virtual TkpContext &
                        FollowUpContext();

    virtual void        AssignDealer(
                            Distributor &       o_rDealer );
  protected:
    // LIFECYCLE
                        Cx_Base(
                            TkpContext *        io_pFollowUpContext );

    void                SetNewToken(
                            DYN ::TextToken *   let_dpToken );
    void                SetFollowUpContext(
                            TkpContext  *       io_pContext );

    Distributor &       Dealer() const;

  private:
    // DATA
    Distributor *       pDealer;
    TkpContext *        pFollowUpContext;
    Dyn< ::TextToken >  pNewToken;
};




inline void
Cx_Base::SetNewToken( DYN ::TextToken * let_dpToken )
    { pNewToken = let_dpToken; }
inline void
Cx_Base::SetFollowUpContext( TkpContext * io_pContext )
    { pFollowUpContext = io_pContext; }
inline Distributor &
Cx_Base::Dealer() const
    { csv_assert(pDealer != 0);
      return *pDealer; }





} // namespace cpp

#endif

