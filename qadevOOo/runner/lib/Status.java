/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Status.java,v $
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

import java.io.PrintWriter;

/**
 * Status represents a result of a testing activity performed. The result is
 * described in two ways: state and runtime state. The state describes if the
 * activity was successful (OK state) or not (FAILED state). The runtime state
 * describes what happend during the activity: the test can be:
 *   - PASSED - the activity completed normally (although it can complete with
 *     FAILED state)
 *   - SKIPPED - the activity was not performed because of a reason (it also can
 *     has OK or FAILED state)
 *   - EXCEPTION - the activity was abnormally terminated because of an
 *     unexpected exception. It always has a FAILED state.
 *   - EXCLUDED - the activity is expected to fail. The state represents how
 *     the state really completed: OK or FAILED.
 *   - other variants are not formalized now and can be represented by
 *     Status.failed() method. They always have a FAILED state.
 */
public class Status extends SimpleStatus {

    /**
     * Construct a status: use runState and state
     * @param runState: either PASSED, SKIPPED, etc.
     * @param state: OK or FAILED.
     */
    public Status(int runState, boolean state ) {
        super(runState, state);
    }

    /**
     * Construct a status: use own message and state.
     * @parame messaeg An own message for the status.
     * @param state: OK or FAILED.
     */
    public Status(String message, boolean state) {
        super( message, state );
    }

    /**
     * This is a factory method for creating a Status representing normal
     * actibity termination.
     *
     * @param state describes a test state (OK if state == true, FAILED
     * otherwise).
     */
    public static Status passed( boolean state ) {
        return new Status(PASSED, state );
    }

    /**
     * This is a factory method for creating a Status representing an exception
     * activity termination. The Status alway has FAILED state.
     *
     * @param t the exception with that the activity completed.
     */
    public static Status exception( Throwable t ) {
        return new ExceptionStatus( t );
    }

    /**
     * This is a factory method for creating a Status representing a skipped
     * activity.
     *
     * @param state describes a test state (OK if state == true, FAILED
     * otherwise).
     */
    public static Status skipped( boolean state ) {
        return new Status( SKIPPED, state );
    }

    /**
     * This is a factory method for creating a Status representing that the
     * result of the activity was excluded. It alwas has FAILED state.
     */
    public static Status excluded() {
        return new Status( EXCLUDED, false );
    }

    /**
     * Creates a Status representing an activity failed for an arbitrary reason.
     * It always has FAILED state.
     *
     * @param reason describes why the activity failed
     */
    public static Status failed(final String reason) {
        return new Status(reason, FAILED);
    }

    /**
     * The method returns a human-readable description of the status.
     * The Status implementation of the method returns the status state
     * description and appends to it it the reason, for example:
     * "FAILED.The getLabel works wrong", "PASSED.OK".
     */
    public String toString() {
        String str = getRunStateString() + "." + getStateString();;

        return str;
    }

    /**
     * Checks whether the status runstate is passed.
     */
    public boolean isPassed() {
        return getRunState() == PASSED;
    }

    /**
     * Checks whether the status runstate is skipped.
     */
    public boolean isSkipped() {
        return getRunState() == SKIPPED;
    }

    /**
     * Checks whether the status runstate is excluded.
     */
    public boolean isExcluded() {
        return getRunState() == EXCLUDED;
    }

    /**
     * Checks whether the status runstate is exception.
     */
    public boolean isException() {
        return getRunState() == EXCEPTION;
    }

    /**
     * Checks whether the status state is failed.
     */
    public boolean isFailed() {
        return !getState();
    }

    /**
     * Checks whether the status state is ok.
     */
    public boolean isOK() {
        return getState();
    }

    public String getDescription () {
        return getRunStateString();
    }
}