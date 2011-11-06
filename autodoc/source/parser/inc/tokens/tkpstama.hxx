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



#ifndef ADC_TKPSTAMA_HXX
#define ADC_TKPSTAMA_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
#include <tokens/stmstarr.hxx>
#include <tokens/stmstfin.hxx>

/** @descr
    This state-machine models state transitions from one state to another
    per indices of branches. If the indices represent ascii-char-values,
    the state-machine can be used for recognising tokens of text.

    The state-machine can be a status itself.

    StateMachine needs the array-size of all stati as a guess, how many stati
    the state machine will contain, when at work.


**/
class StateMachine
{
    public:
        // Types
        typedef StmStatus::Branch   Branch;
        typedef StmStatus * *       StatusList;

    //# Interface self
        // LIFECYCLE
                        StateMachine(
                            intt            in_nStatusSize,
                            intt            in_nInitial_StatusListSize );   /// The user of the constructor should guess
                                                                            ///   the approximate number of stati here to
                                                                            ///   avoid multiple reallocations.
        /// @#AddStatus
        intt            AddStatus(      /// @return the new #Status' ID
                            DYN StmStatus * let_dpStatus);
        /// @#AddToken
        void            AddToken(
                            const char *        in_sToken,
                            TextToken::F_CRTOK  in_fTokenCreateFunction,
                            const INT16 *       in_aBranches,
                            INT16               in_nBoundsStatus );
                        ~StateMachine();


        // OPERATIONS
        StmBoundsStatus &
                        GetCharChain(
                            TextToken::F_CRTOK &
                                                o_nTokenCreateFunction,
                            CharacterSource &   io_rText );
    private:
        // SERVICE FUNCTIONS
        StmStatus &     Status(
                            intt            in_nStatusNr) const;
        StmArrayStatus &
                        CurrentStatus() const;
        StmBoundsStatus *
                        BoundsStatus() const;

        /// Sets the PeekedStatus.
        void            Peek(
                            intt            in_nBranch);

        void            ResizeStati();      // Adds space for 32 stati.

        // DATA
        StatusList      pStati;             /// List of Status, implemented as simple C-array of length #nStatiSpace
                                            /// with nStatiLength valid members (beginning from zero).
        intt            nCurrentStatus;
        intt            nPeekedStatus;

        intt            nStatusSize;        /// Size of the branch array of a single status.

        intt            nNrofStati;         /// Nr of Stati so far.
        intt            nStatiSpace;        /// Size of allocated array for #pStati (size in items).
};



/** @#AddToken
    @descr
    Adds a token, which will be recogniszeds by the
    statemachine.


**/



#endif


