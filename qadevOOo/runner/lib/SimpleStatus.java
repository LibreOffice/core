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

    /**
     * The field is holding state of the status.
     */
    private final boolean bSuccessful;

    /**
     * The field is holding reason of the status.
     */
    private final RunState runState;

    /**
     * This is the run state: either SKIPPED, COMPLETED, etc.
     * or user defined. Deriving classes can overwrite it for own run states.
     */
    protected String runStateString;

    /**
     * The constructor initialize state and reason field.
     */
    protected SimpleStatus( RunState runState, boolean bSuccessful ) {
        this.bSuccessful = bSuccessful;
        this.runState = runState;
        if ( runState == RunState.COMPLETED ) {
            runStateString = "COMPLETED";
        } else if ( runState == RunState.SKIPPED ) {
            runStateString = "SKIPPED";
        } else if ( runState == RunState.EXCEPTION ) {
            runStateString = "EXCEPTION";
        } else {
            runStateString = "UNKNOWN";
        }
    }

    /**
     * The constructor initialize state and reason field.
     */
    protected SimpleStatus(String runStateString, boolean bSuccessful) {
        this.bSuccessful = bSuccessful;
        this.runState = RunState.USER_DEFINED;
        this.runStateString = runStateString;
    }

    public boolean isSuccessful() {
        return bSuccessful;
    }

    /**
     * getRunState() implementation. Just returns th runState field value.
     */
    public RunState getRunState() {
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
        if (bSuccessful)
            return "OK";
        return "FAILED";

    }
}
