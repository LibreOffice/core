/*************************************************************************
 *
 *  $RCSfile: Status.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:15:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
public abstract class Status {
    /* Run states. */

    /**
     * The constatnt represents PASSED runtime state.
     */
    public final static int PASSED = 0;

    /**
     * The constant represents EXCEPTION runtime state.
     */
    public final static int EXCEPTION = 4;

    /**
     * The constant represents EXCLUDED runtime state.
     */
    public final static int EXCLUDED = 5;

    /**
     * The constant represents SKIPPED runtime state.
     */
    public final static int SKIPPED = 2;

    /* Test states */

    /**
     * The constant represents FAILED state.
     */
    public final static int FAILED = 1;

    /**
     * The constant represents OK state.
     */
    public final static int OK = 3;

    /**
     * Returns the test state of the Status.
     */
    public abstract int getState();

    /**
     * Returns the run state of the Status.
     */
    public abstract int getRunState();

    /**
     * Returns the string describing run state of the Status. For example,
     * "PASSED", "SKIPPED", "File file.txt is not found".
     */
    public abstract String getRunStateString();

    /**
     * This is a factory method for creating a Status representing normal
     * actibity termination.
     *
     * @param state describes a test state (OK if state == true, FAILED
     * otherwise).
     */
    public static Status passed( boolean state ) {
        return new SimpleStatus(PASSED, state );
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
        return new SimpleStatus( SKIPPED, state );
    }

    /**
     * This is a factory method for creating a Status representing that the
     * result of the activity was excluded. It alwas has FAILED state.
     */
    public static Status excluded() {
        return new SimpleStatus( EXCLUDED, false );
    }

    /**
     * Creates a Status representing an activity failed for an arbitrary reason.
     * It always has FAILED state.
     *
     * @param reason describes why the activity failed
     */
    public static Status failed(final String reason) {
        return new Status() {
            public int getState() {
                return FAILED;
            }

            public int getRunState() {
                return SKIPPED;
            }

            public String getRunStateString() {
                return reason;
            }

/*            public boolean equals(Object obj) {
                if (obj == null || this.getClass() != obj.getClass()) {
                    return false;
                }
                return this.getRunStateString().equals(
                        ((Status)obj).getRunStateString());
            } */
        };
    }

    /**
     * The method returns a human-readable description of the status.
     * The Status implementation of the method returns the status state
     * description and appends to it it the reason, for example:
     * "FAILED.The getLabel works wrong", "PASSED.OK".
     */
    public String toString() {
        String str = getRunStateString() + ".";

        // Getting state of the status and converting it to string.
        int state = getState();
        if (state == OK) {
            str += "OK";
        } else {
            // Although, normally, there should be only FAILED state,
            // all other marked as failed too.
            str += "FAILED";
        }

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
        return getState() == FAILED;
    }

    /**
     * Checks whether the status state is ok.
     */
    public boolean isOK() {
        return getState() == OK;
    }

    public String getDescription () {
        return getRunStateString();
    }
}