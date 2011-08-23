/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef ADC_TKPSTAMA_HXX
#define ADC_TKPSTAMA_HXX

// USED SERVICES
    // BASE CLASSES
#include <tokens/tkpcontx.hxx>
    // COMPONENTS
#include <tokens/stmstarr.hxx>
#include <tokens/stmstfin.hxx>

/**	@descr
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
        typedef StmStatus::Branch	Branch;
        typedef StmStatus * *		StatusList;

    //# Interface self
        // LIFECYCLE
                        StateMachine(
                            intt			in_nStatusSize,
                            intt			in_nInitial_StatusListSize );	/// The user of the constructor should guess
                                                                            ///   the approximate number of stati here to
                                                                            ///	  avoid multiple reallocations.
        /// @#AddStatus
        intt			AddStatus(  	/// @return the new #Status' ID
                            DYN StmStatus *	let_dpStatus);
        /// @#AddToken
        void			AddToken(
                            const char *		in_sToken,
                            TextToken::F_CRTOK	in_fTokenCreateFunction,
                            const INT16 *		in_aBranches,
                            INT16				in_nBoundsStatus );
                        ~StateMachine();


        // OPERATIONS
        StmBoundsStatus &
                        GetCharChain(
                            TextToken::F_CRTOK &
                                                o_nTokenCreateFunction,
                            CharacterSource &	io_rText );
    private:
        // SERVICE FUNCTIONS
        StmStatus &		Status(
                            intt			in_nStatusNr) const;
        StmArrayStatus &
                        CurrentStatus() const;
        StmBoundsStatus *
                        BoundsStatus() const;

        /// Sets the PeekedStatus.
        void			Peek(
                            intt			in_nBranch);

        void			ResizeStati();		// Adds space for 32 stati.

        // DATA
        StatusList      pStati;				///	List of Status, implemented as simple C-array of length #nStatiSpace
                                            /// with nStatiLength valid members (beginning from zero).
        intt			nCurrentStatus;
        intt			nPeekedStatus;

        intt			nStatusSize;		/// Size of the branch array of a single status.

        intt			nNrofStati;			/// Nr of Stati so far.
        intt			nStatiSpace;        /// Size of allocated array for #pStati (size in items).
};



/**	@#AddToken
    @descr
    Adds a token, which will be recogniszeds by the
    statemachine.


**/



#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
