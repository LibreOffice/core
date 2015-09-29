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
    public static final int PASSED = 0;

    /**
     * The constant represents EXCEPTION runtime state.
     */
    public static final int EXCEPTION = 3;

    /**
     * The constant represents SKIPPED runtime state.
     */
    public static final int SKIPPED = 1;

    /**
     * This is a private indicator for a user defined runtime state
     */
    private static final int USER_DEFINED = 4;

    /* Test states */

    /**
     * The constant represents FAILED state.
     */
    public static final boolean FAILED = false;



    /**
     * The field is holding state of the status.
     */
    private final boolean state;

    /**
     * The field is holding reason of the status.
     */
    private final int runState;

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
        } else if ( runState == SKIPPED ) {
            runStateString = "SKIPPED";
        } else if ( runState == EXCEPTION ) {
            runStateString = "EXCEPTION";
        } else {
            runStateString = "UNKNOWN";
        }
    }

    /**
     * The constructor initialize state and reason field.
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
     * Get the result: passed or failed.
     */
    public String getStateString() {
        if (state)
            return "OK";
        return "FAILED";

    }
}
