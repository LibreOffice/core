/*************************************************************************
 *
 *  $RCSfile: SimpleStatus.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-11-18 16:15:38 $
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
class SimpleStatus extends Status {
    /**
     * The field is holding state of the status.
     */
    protected final boolean state;

    /**
     * The field is holding reason of the status.
     */
    protected final int runState;

    /**
     * The constructor initialize state and reson field.
     */
    protected SimpleStatus( int runState, boolean state ) {
        this.state = state;
        this.runState = runState;
    }

    /**
     * getState implementation. Just returns the state field value.
     */
    public int getState() {
        return (state ? OK : FAILED);
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
        int runState = getRunState();

        if ( runState == PASSED ) {
            return "PASSED";
        } else if ( runState == EXCLUDED ) {
            return "EXCLUDED";
        } else if ( runState == SKIPPED ) {
            return "SKIPPED";
        } else {
            return "UNKNOWN";
        }
    }

    /**
     * Compares this Status with obj.
     *
     * @return <tt>true</tt> if obj is a SimpleStatus instance and it has
     * the same state and runstate, <tt>false</tt> otherwise.
     */
/*    public boolean equals(Object obj) {
        if (obj == null || !(obj instanceof SimpleStatus)) {
            return false;
        }

        SimpleStatus other = (SimpleStatus)obj;

        return this.getState() == other.getState()
                && this.getRunState() == other.getRunState();
    } */
}