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


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
