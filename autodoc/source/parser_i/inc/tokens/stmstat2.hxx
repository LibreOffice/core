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
