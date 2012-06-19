/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef ADC_TKPSTAM2_HXX
#define ADC_TKPSTAM2_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcont2.hxx>
    // COMPONENTS
#include <tokens/stmstar2.hxx>
#include <tokens/stmstfi2.hxx>

/** @descr
    This state-machine models state transitions from one state to another
    per indices of branches. If the indices represent ascii-char-values,
    the state-machine can be used for recognising tokens of text.

    The state-machine can be a status itself.

    StateMachin2 needs the array-size of all stati as a guess, how many stati
    the state machine will contain, when at work.


**/
class StateMachin2
{
    public:
        // Types
        typedef StmStatu2::Branch   Branch;
        typedef StmStatu2 * *       StatusList;

    //# Interface self
        // LIFECYCLE
                        StateMachin2(
                            intt            in_nStatusSize,
                            intt            in_nInitial_StatusListSize );   /// The user of the constructor should guess
                                                                            ///   the approximate number of stati here to
                                                                            ///   avoid multiple reallocations.
        /// @#AddStatus
        intt            AddStatus(      /// @return the new #Status' ID
                            DYN StmStatu2 * let_dpStatus);
        /// @#AddToken
        void            AddToken(
                            const char *        in_sToken,
                            UINT16              in_nTokenId,
                            const INT16 *       in_aBranches,
                            INT16               in_nBoundsStatus );
                        ~StateMachin2();

        // OPERATIONS
        StmBoundsStatu2 &
                        GetCharChain(
                            UINT16 &            o_nTokenId,
                            CharacterSource &   io_rText );
    private:
        // SERVICE FUNCTIONS
        StmStatu2 &     Status(
                            intt            in_nStatusNr) const;
        StmArrayStatu2 &
                        CurrentStatus() const;
        StmBoundsStatu2 *
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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
