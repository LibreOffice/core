/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stmstat2.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:08:41 $
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


