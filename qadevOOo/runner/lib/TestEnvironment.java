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
import com.sun.star.uno.XInterface;

import java.util.HashMap;


/**
 * The class contains an instance of a given implementation object and
 * auxiliary objects associated with it and required for the object testing.
 *
 * @see TestCase
 */

public final class TestEnvironment {
    /**
     * Contains object relations - auxiliary objects associated with the
     * tested object and required for testing.
     */
    private final HashMap<String, Object> relations = new HashMap<String, Object>(10);

    /**
     * An instance of the tested implementation object.
     */
    private final XInterface testObject;

    /**
     * Indicates that the testObject is in invalid state and should notbe
     * used for testing anymore.
     */
    private boolean disposed = false;

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