/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
     * @param method refers to the method which was tested
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
