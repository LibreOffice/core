/*************************************************************************
 *
 *  $RCSfile: TestResult.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: pjunck $ $Date: 2004-11-02 11:43:46 $
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

import java.util.Hashtable;

/**
 * The class supports interface tests development and Status calculation.
 */
public class TestResult {
    /**
     * Contains methods having been tested and their results.
     */
    protected Hashtable testedMethods = new Hashtable();

    /**
     * The method makes method tested with the result, i.e. it adds to its
     * state OK (if result == true) or FAILED (if result == false) status
     * and makes the state of the method completed. It's equal to
     * tested(method, Status(result)) call.
     *
     * @param method reffers to the method whoch was tested
     * @param result the result of testing the method
     *
     * @return the result value
     *
     * @throw java.lang.IllegalArgumentException if the method is not
     * available in the interface.
     *
     * @see #tested(String, Status)
     */
    public boolean tested( String method, boolean result) {
        return tested( method, Status.passed( result ) );
    }

    /**
     * The method makes the method tested with the status, i.e. it adds the
     * status to its state and makes it completed.
     *
     * @param method reffers to the method whoch was tested
     * @param status describes the result of testing the method
     * @return <tt>true</tt> if status is OK, <tt>false</tt> otherwise.
     *
     * @throw java.lang.IllegalArgumentException if the method is not
     * available in the interface.
     */
    public boolean tested( String method, Status status ) {
        testedMethods.put(method,status);
        return true;
    }

    /**
     * @return methods available in the interface tested.
     */
    public String[] getTestedMethods() {
        return (String[])testedMethods.keySet().toArray(
                new String[testedMethods.size()]);
    }

    /**
     * @return <tt>true</tt> if the method belongs to the interface tested,
     * <tt>false</tt> otherwise.
     */
    public boolean hasMethod( String method ) {
        return testedMethods.containsKey( method );
    }

    /**
     * @return status of testing the method, if it is available (was set by
     * the tested or assert method), <tt>null</tt> otherwise.
     *
     * @see #tested(String, boolean)
     * @see #tested(String, Status)
     * @see #assert
     */
    public Status getStatusFor( String method ) {
        return (Status)testedMethods.get( method );
    }

}    