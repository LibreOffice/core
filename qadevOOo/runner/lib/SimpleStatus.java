/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SimpleStatus.java,v $
 * $Revision: 1.5 $
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

package lib;

/**
 * The class is a simple implementation of Status class. It implements simple
 * Status behaviour: its state, reason and log are defined when creating
 * the SimpleSTatus instance.
 */
class SimpleStatus {
    /* Run states. */

    /**
     * The constatnt represents PASSED runtime state.
     */
    public final static int PASSED = 0;

    /**
     * The constant represents EXCEPTION runtime state.
     */
    public final static int EXCEPTION = 3;

    /**
     * The constant represents EXCLUDED runtime state.
     */
    public final static int EXCLUDED = 2;

    /**
     * The constant represents SKIPPED runtime state.
     */
    public final static int SKIPPED = 1;

    /**
     * This is a private indicator for a user defined runtime state
     */
    private final static int USER_DEFINED = 4;

    /* Test states */

    /**
     * The constant represents FAILED state.
     */
    public final static boolean FAILED = false;

    /**
     * The constant represents OK state.
     */
    public final static boolean OK = true;

    /**
     * The field is holding state of the status.
     */
    protected final boolean state;

    /**
     * The field is holding reason of the status.
     */
    protected final int runState;

    /**
     * This is the run state: either SKIPPED, PASSED, etc.
     * or user defined. Deriving classes can overwrite it for own run states.
     */
    protected String runStateString;

    /**
     * The constructor initialize state and reason field.
     */
    protected SimpleStatus( int runState, boolean state ) {
        this.state = state;
        this.runState = runState;
        if ( runState == PASSED ) {
            runStateString = "PASSED";
        } else if ( runState == EXCLUDED ) {
            runStateString = "EXCLUDED";
        } else if ( runState == SKIPPED ) {
            runStateString = "SKIPPED";
        } else if ( runState == EXCEPTION ) {
            runStateString = "EXCEPTION";
        } else {
            runStateString = "UNKNOWN";
        }
    }

    /**
     * The constructor initialize state and reson field.
     */
    protected SimpleStatus(String runStateString, boolean state) {
        this.state = state;
        this.runState = USER_DEFINED;
        this.runStateString = runStateString;
    }

    /**
     * getState implementation. Just returns the state field value.
     */
    public boolean getState() {
        return state;
    }

    /**
     * getRunState() implementation. Just returns th runState field value.
     */
    public int getRunState() {
        return runState;
    }

    /**
     * getReason implementation. Just returns the reason field value.
     */
    public String getRunStateString() {
        return runStateString;
    }

    /**
     * Get the ressult: passed or failed.
     */
    public String getStateString() {
        if (state)
            return "OK";
        return "FAILED";

    }
}