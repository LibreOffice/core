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

import java.io.PrintWriter;

import lib.TestParameters;
/**
 * <code>TestCase</code> represent a factory for <code>TestEnvironment</code>s
 * creation and disposing for a given implementation object. The
 * <code>TestEnvironment</code> contains an instance of the implementation
 * object and all additional objects needed to perform tests on the object.
 *
 * <p>The <code>TestCase</code> provides four methods for its subclasses to
 * define its functionality: <code>initialize()</code>, <code>cleanup()</code>,
 * <code>createTestEnvironment()</code> and <code>disposeTestEnvironment()</code>.
 * The first two are intended to initialize and cleanup common objects shared
 * among all instances of <code>TestEnvironment</code> produced by the
 * <code>TestCase</code>, and they are called at the beginning and at the end of
 * the <code>TestCase</code> lifecycle accordingly.
 *
 * <p>The other two are intended to produce and dispose
 * <code>TestEnvironment</code> instances. The
 * <code>createTestEnvironment()</code> is called to create a
 * <code>TestEnvironment</code> instance and the
 * <code>disposeTestEnvironment()</code> is called when the instane is not used
 * anymore.
 *
 * @see lib.TestEnvironment
 */
public abstract class TestCase {

    /**
     * Specifies the PrintWriter to log information.
     */
    public PrintWriter log;

    //public static TestCase tCase;

    /**
     * Sets the log to write information during testing.
     */
    public void setLogWriter( PrintWriter log ) {
        this.log = log;
    }

    /**
     * Initializes the <code>TestCase</code>. Calls <code>initialize()</code>
     * method.
     *
     * @param tParam test parameters.
     */
    public void initializeTestCase( TestParameters tParam ) {
        initialize( tParam, log );
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
    }


    /**
     * Cleans up the <code>TestCase</code>. Calls <code>cleanup()</code>.
     *
     * @param tParam test parameters
     */
    public void cleanupTestCase( TestParameters tParam ) {
        cleanup( tParam, log );
    }

    /**
     * Called while the <code>TestCase</code> cleanup. In the implementation
     * does nothing. Subclasses can override to cleanup objects shared among
     * all <code>TestEnvironment</code>s.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see #cleanupTestCase
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
    }

    /**
     * Creates a <code>TestEnvironment</code> containing an instance of the
     * implementation object and related objects needed to perform test.
     *
     * @param tParam test parameters
     *
     * @return the created <code>TestEnvironment</code>
     *
     * @see #createTestEnvironment
     * @see lib.TestEnvironment
     */
    public synchronized TestEnvironment getTestEnvironment( TestParameters tParam ) {
        TestEnvironment tEnv = null;
        try {
            tEnv = createTestEnvironment( tParam, log );
            System.out.println("Environment created");
            if (tEnv != null) {
                tEnv.setTestCase(this);
            }
        } catch (Exception e) {
            String message = e.getMessage();
            if (message == null)
                message = e.toString();
            System.out.println("Exception while getting Environment "+message);
            e.printStackTrace();
            cleanup(tParam, log);
        }
        return tEnv;
    }

    /**
     * Disposes the <code>TestEnvironment</code> when it is not needed anymore.
     *
     * @param tEnv the environment to dispose
     * @param tParam test parameters
     */
    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
        TestParameters tParam ) {
        cleanup( tParam, log );
    }

    /**
     * Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects. Subclasses should implement this
     * method to provide the implementation and related objects. The method is
     * called from <code>getTestEnvironment()</code>.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment
     */
    protected abstract TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log );

    /**
     * @return the name of the object
     */
    public String getObjectName() {
        String clName = this.getClass().getName();
        return clName.substring( clName.lastIndexOf('.') + 1 );
    }

}
