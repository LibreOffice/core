/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkpstama.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 18:41:07 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


        // FOR TESTS ONLY:
        void            PrintOut();

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


