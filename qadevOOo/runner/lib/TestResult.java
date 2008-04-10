/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestResult.java,v $
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
        System.out.println("Method "+method+" finished with state "+(result?"OK":"FAILED"));
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