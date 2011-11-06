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



#ifndef ADC_STMSTAT2_HXX
#define ADC_STMSTAT2_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
    // PARAMETERS
class StmArrayStatu2;
class StmBoundsStatu2;

/** A StmStatu2 is a state within a StateMachin2.
    There are two kinds of it.  Either its an array of pointers to
    other states within the state machine - an ArrayStatus.

    Or it is a BoundsStatus, which shows, the token cannot be
    followed further within the StateMachin2.
**/
class StmStatu2 // := "State machine status"
{
  public:
    typedef intt        Branch;         /// Values >= 0 give a next #Status' ID.
                                        /// Values <= 0 tell, that a token is finished.
                                        /// a value < 0 returns the status back to an upper level state machine.
        // LIFECYCLE
    virtual             ~StmStatu2() {}

        // OPERATIONS
    virtual StmArrayStatu2 *
                        AsArray();
    virtual StmBoundsStatu2 *
                        AsBounds();

        // INQUIRY
    virtual bool        IsADefault() const = 0;
};



#endif


