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



#ifndef ADC_STMSTARR_HXX
#define ADC_STMSTARR_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/stmstate.hxx>
    // COMPONENTS
    // PARAMETERS
#include <tokens/token.hxx>


class StmArrayStatus : public StmStatus
{
  public:
    typedef TextToken::F_CRTOK F_CRTOK;

    // LIFECYCLE
                        StmArrayStatus(
                            intt                i_nStatusSize,
                            const INT16 *       in_aArrayModel,
                            F_CRTOK             i_fTokenCreateFunction,
                            bool                in_bIsDefault );
                        ~StmArrayStatus();

    // INQUIRY
    StmStatus::Branch   NextBy(
                            intt                in_nFollowersIndex) const;
    F_CRTOK             TokenCreateFunction() const
                                                { return fTokenCreateFunction; }
    virtual bool        IsADefault() const;

    // ACCESS
    virtual StmArrayStatus *
                        AsArray();
    bool                SetBranch(
                            intt                in_nBranchIx,
                            StmStatus::Branch   in_nBranch );
    void                SetTokenCreateFunction(
                            F_CRTOK             i_fTokenCreateFunction );
  private:
    StmStatus::Branch * dpBranches;
    intt                nNrOfBranches;
    F_CRTOK             fTokenCreateFunction;
    bool                bIsADefault;
};


// IMPLEMENTATION

inline void
StmArrayStatus::SetTokenCreateFunction( F_CRTOK i_fTokenCreateFunction )
    { fTokenCreateFunction = i_fTokenCreateFunction; }



#endif


