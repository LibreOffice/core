/*************************************************************************
 *
 *  $RCSfile: TestEnvironment.java,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Date: 2003-01-27 16:27:40 $
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