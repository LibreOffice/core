/*************************************************************************
 *
 *  $RCSfile: SimpleStatus.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2004-11-02 11:37:13 $
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