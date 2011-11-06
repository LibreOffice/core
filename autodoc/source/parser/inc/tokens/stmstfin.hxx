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



#ifndef ADC_STMSTFIN_HXX
#define ADC_STMSTFIN_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/stmstate.hxx>
    // COMPONENTS
    // PARAMETERS


class TkpContext;
class StateMachineContext;

/**
**/
class StmBoundsStatus : public StmStatus
{
  public:
    // LIFECYCLE
                        StmBoundsStatus(
                            StateMachineContext &
                                                o_rOwner,
                            TkpContext &        i_rFollowUpContext,
                            uintt               i_nStatusFunctionNr,
                            bool                i_bIsDefault  );
    // INQUIRY
    TkpContext *        FollowUpContext();
    uintt               StatusFunctionNr() const;
    virtual bool        IsADefault() const;

    // ACCESS
    virtual StmBoundsStatus *
                        AsBounds();

  private:
    StateMachineContext *
                        pOwner;
    TkpContext *        pFollowUpContext;
    uintt               nStatusFunctionNr;
    bool                bIsDefault;
};

inline TkpContext *
StmBoundsStatus::FollowUpContext()
    { return pFollowUpContext; }
inline uintt
StmBoundsStatus::StatusFunctionNr() const
    { return nStatusFunctionNr; }


#endif


