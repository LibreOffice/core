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

package mod._dbaccess;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.sdbc.SQLException;
import com.sun.star.task.XInteractionContinuation;
import com.sun.star.task.XInteractionRequest;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sdb.InteractionHandler</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::task::XInteractionHandler</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurrently.
* @see com.sun.star.task.XInteractionHandler
* @see com.sun.star.sdb.InteractionHandler
* @see ifc.task._XInteractionHandler
*/
public class OInteractionHandler extends TestCase {


    private static class TestRequest implements XInteractionRequest {
        public Object getRequest() {
            return new SQLException("Test exception") ;
        }

        public XInteractionContinuation[] getContinuations() {
            return new XInteractionContinuation[0] ;
        }
    }


    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.sdb.InteractionHandler</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XInteractionHandler.Request'</code> for
    *      {@link ifc.task._XInteractionHandler} : this relation
    *    is <code>com.sun.star.task.XInteractionRequest</code>
    *    interface implementation which depends on the component
    *    tested. In this case it emulates SQL error by returning
    *    <code>SQLException</code> object. </li>
    * </ul>
    */
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws Exception {
        Object oInterface = Param.getMSF().createInstance(
                          "com.sun.star.sdb.InteractionHandler" );
        XInterface oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("XInteractionHandler.Request", new TestRequest()) ;

        return tEnv;
    } // finish method getTestEnvironment

}

