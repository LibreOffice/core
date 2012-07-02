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

package mod._inv;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.script.Invocation</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XSingleServiceFactory</code></li>
* </ul>
* @see com.sun.star.script.Invocation
* @see com.sun.star.lang.XSingleServiceFactory
* @see ifc.lang._XSingleServiceFactory
*/
public class Invocation extends TestCase {

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates service <code>com.sun.star.script.Invocation</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XSingleServiceFactory.createInstance.negative'</code> :
    *   for interface {@link _ifc.lang.XSingleServiceFactory} ;
    *   <code>String</code> relation; If its value 'true' then
    *   <code>createInstance</code> method for the object isn't
    *   supported. In this case object doesn't support this method.</li>
    *  <li> <code>'XSingleServiceFactory.arguments'</code> :
    *   for interface {@link _ifc.lang.XSingleServiceFactory} ;
    *   has <code>Object[]</code> type. This relation is used as
    *   a parameter for <code>createInstanceWithArguments</code>
    *   method call. If this relation doesn't exist test pass
    *   zerro length array as argument. Here
    *   <code>com.sun.star.io.Pipe</code> instance is passed.</li>
    *  <li> <code>'XSingleServiceFactory.MustSupport'</code> :
    *   for interface {@link _ifc.lang.XSingleServiceFactory}.
    *   Specifies that created instance must support
    *   <code>com.sun.star.script.XInvocation</code> interface.
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        try {
            XInterface xInt = (XInterface)xMSF.createInstance(
                    "com.sun.star.script.Invocation");

            TestEnvironment tEnv = new TestEnvironment(xInt);

            // the createInstance should fail according to the documentation
            tEnv.addObjRelation(
                    "XSingleServiceFactory.createInstance.negative", "true");

            // creating parameters to createInstanceWithArguments
            Object[] args = new Object[1];

            args[0] = xMSF.createInstance("com.suns.star.io.Pipe");

            tEnv.addObjRelation(
                    "XSingleServiceFactory.arguments", args);

            tEnv.addObjRelation("XSingleServiceFactory.MustSupport",
                new Class[] {com.sun.star.script.XInvocation.class});

            return tEnv;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }
    }
}
