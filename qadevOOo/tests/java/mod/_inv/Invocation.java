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
    *   for interface {@link ifc.lang._XSingleServiceFactory} ;
    *   <code>String</code> relation; If its value 'true' then
    *   <code>createInstance</code> method for the object isn't
    *   supported. In this case object doesn't support this method.</li>
    *  <li> <code>'XSingleServiceFactory.arguments'</code> :
    *   for interface {@link ifc.lang._XSingleServiceFactory} ;
    *   has <code>Object[]</code> type. This relation is used as
    *   a parameter for <code>createInstanceWithArguments</code>
    *   method call. If this relation doesn't exist test pass
    *   zero length array as argument. Here
    *   <code>com.sun.star.io.Pipe</code> instance is passed.</li>
    *  <li> <code>'XSingleServiceFactory.MustSupport'</code> :
    *   for interface {@link ifc.lang._XSingleServiceFactory}.
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
