/*************************************************************************
 *
 *  $RCSfile: TestCase.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-02-13 17:33:38 $
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

import com.sun.star.uno.XInterface;

import java.util.Hashtable;

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
     * @see #initializeTestCase()
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
     * @see #createTestEnvironment()
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
            System.out.println("Exception while getting Environment "+e.getMessage());
        }
        return tEnv;
    }

    /**
     * Disposes the <code>TestEnvironment</code> when it is not needed anymore.
     * The method calls <code>cleanupTestEnvironment()</code>.
     *
     * @param tEnv the environment to dispose
     * @param tParam test parameters
     *
     * @see #cleanupTestEnvironment()
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
     * @see #getTestEnvironment()
     */
    protected abstract TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log );

    /**
     * Called while disposing a <code>TestEnvironment</code>. In the
     * implementation does nothing. Subclasses can override to clean up
     * the environments created by them.
     *
     * @param tParam test parameters
     * @param tEnv the environment to cleanup
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #disposeTestEnvironment()
     */
    protected void cleanupTestEnvironment( TestParameters Param,
            TestEnvironment tEnv, PrintWriter log ) {
    }

    /**
     * @return the name of the object
     */
    public String getObjectName() {
        String clName = this.getClass().getName();
        return clName.substring( clName.lastIndexOf('.') + 1 );
    }

}