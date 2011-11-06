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



#ifndef ADC_STMSTFI2_HXX
#define ADC_STMSTFI2_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/stmstat2.hxx>
    // COMPONENTS
    // PARAMETERS


class TkpContext;
class StateMachineContext;

/**
**/
class StmBoundsStatu2 : public StmStatu2
{
  public:
    // LIFECYCLE
                        StmBoundsStatu2(
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
    virtual StmBoundsStatu2 *
                        AsBounds();

  private:
    StateMachineContext *
                        pOwner;
    TkpContext *        pFollowUpContext;
    uintt               nStatusFunctionNr;
    bool                bIsDefault;
};

inline TkpContext *
StmBoundsStatu2::FollowUpContext()
    { return pFollowUpContext; }
inline uintt
StmBoundsStatu2::StatusFunctionNr() const
    { return nStatusFunctionNr; }


#endif


