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



#ifndef ADC_CPP_SRECOVER_HXX
#define ADC_CPP_SRECOVER_HXX



// USED SERVICES
    // BASE CLASSES
#include "cxt2ary.hxx"
#include <ary/info/docstore.hxx>
    // COMPONENTS
    // PARAMETERS

namespace cpp
{

/** Implementation struct for cpp::ContextForAry.
*/
struct ContextForAry::S_RecoveryGuard
{
  public:
                        S_RecoveryGuard();
                        ~S_RecoveryGuard();

    void                Reset()                 { bIsWithinRecovery = false; nBlockBracketsCounter = 0; }

    void                StartWaitingFor_Recovery();

    void                Hdl_SwBracketOpen();
    void                Hdl_SwBracketClose();
    void                Hdl_Semicolon();

    bool                IsWithinRecovery() const;

  private:
    // DATA
    bool                bIsWithinRecovery;
    intt                nBlockBracketsCounter;
};



// IMPLEMENTATION

/*  The implementation is in header, though not all inline, because this file
    is included in cxt2ary.cxx only!
*/

ContextForAry::
S_RecoveryGuard::S_RecoveryGuard()
    :   bIsWithinRecovery(false),
        nBlockBracketsCounter(0)
{
}

ContextForAry::
S_RecoveryGuard::~S_RecoveryGuard()
{
}

inline void
ContextForAry::
S_RecoveryGuard::StartWaitingFor_Recovery()
{
    bIsWithinRecovery = true;
    nBlockBracketsCounter = 0;
}

void
ContextForAry::
S_RecoveryGuard::Hdl_SwBracketOpen()
{
    if ( bIsWithinRecovery )
        ++nBlockBracketsCounter;
}

void
ContextForAry::
S_RecoveryGuard::Hdl_SwBracketClose()
{
    if ( bIsWithinRecovery )
        --nBlockBracketsCounter;
}

inline void
ContextForAry::
S_RecoveryGuard::Hdl_Semicolon()
{
    if (bIsWithinRecovery AND nBlockBracketsCounter == 0)
       bIsWithinRecovery = false;
}

inline bool
ContextForAry::
S_RecoveryGuard::IsWithinRecovery() const
{
    return bIsWithinRecovery;
}



}   // namespace cpp


#endif

