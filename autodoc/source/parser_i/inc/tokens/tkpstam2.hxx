/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tkpstam2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:09:52 $
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


        // FOR TESTS ONLY:
        void            PrintOut();

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


