/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: TestEnvironment.java,v $
 * $Revision: 1.3 $
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
import com.sun.star.uno.XInterface;

import java.util.Hashtable;


/**
 * The class contains an instance of a given implementation object and
 * auxiliary objects associated with it and required for the object testing.
 *
 * @see TestCase
 */

public class TestEnvironment {
    /**
     * Contains object relations - auxiliary objects associated with the
     * tested object and required for testing.
     */
    protected Hashtable relations = new Hashtable(10);

    /**
     * An instance of the tested implementation object.
     */
    protected XInterface testObject;

    /**
     * Indicates that the testObject is in invalid state and should notbe
     * used for testing anymore.
     */
    protected boolean disposed = false;

    /**
     * A reference to TestCase which has created the test environment.
     */
    private TestCase tCase;

    /**
     * Creates an instance of test environment with testObject.
     *
     * @param testObject object to test
     *
     * @throws java.lang.IllegalArgumentException if the testObject is
     * <tt>null</tt>.
     */
    public TestEnvironment( XInterface testObject ) {
        if (testObject == null) {
            throw new IllegalArgumentException(
                    "Couldn't create a test object");
            }
        this.testObject = testObject;
    }

    /**
     * @return the object to test.
     */
    public XInterface getTestObject() {
        return testObject;
    }

    /**
     * Adds to the environment an auxiliary object required for testing.
     *
     * @param name a name to reference the auxiliary object
     *
     * @param relation the auxiliary object related to the tested one
     */
    public void addObjRelation( String name, Object relation) {
        relations.put( name, relation );
    }

    /**
     * Returns an auxiliary object referenced by tname.
     *
     * @param name a name of the object relation
     *
     * @return the auxiliary object(object relation)
     */
    public Object getObjRelation( String name ) {
        return relations.get( name );
    }

    /**
     * Checks if an auxiliary object has been registered with name
     *
     * @param name a name referencing an auxiliarx object
     *
     * @return <tt>true</tt> if the object has been associated, <tt>false</tt>
     * otherwise.
     */
    public boolean hasObjRelation(String name) {
        return (relations.get(name) != null) ;
    }

    /**
     * Sets the <code>TestCase</code> that created the environment.
     */
    public void setTestCase( TestCase tCase) {
        this.tCase = tCase;
    }

    /**
     * @return the <code>TestCase</code> created the environment.
     */
    public TestCase getTestCase() {
        return tCase;
    }

    /**
     * Makes the environment invalid, i.e. it should not be used for
     * testing anymore.
     */
    public void dispose() {
        disposed = true;
    }

    /**
     * Checks if the environment has been disposed.
     *
     * @return <tt>true</tt< if it has been disposed, <tt>false</tt> otherwise.
     *
     * @see #dispose()
     */
    public boolean isDisposed() {
        return disposed;
    }
}