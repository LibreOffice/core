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



package mod._dbaccess;

import java.io.PrintWriter;

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
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
* threads concurently.
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
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.sdb.InteractionHandler</code>.
    *     Object relations created :
    * <ul>
    *  <li> <code>'XInteractionHandler.Request'</code> for
    *      {@link ifc.task._XInteractionHandler} : this realtion
    *    is <code>com.sun.star.task.XInteractionRequest</code>
    *    interface implementation which depends on the component
    *    tested. In this case it emulates SQL error by returning
    *    <code>SQLException</code> object. </li>
    * </ul>
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;

        try {
            oInterface = ((XMultiServiceFactory)Param.getMSF()).createInstance(
                            "com.sun.star.sdb.InteractionHandler" );

        }
        catch( com.sun.star.uno.Exception e ) {
            log.println("Service not available" );
            throw new StatusException("Service not available", e) ;
        }

        if (oInterface == null) {
            log.println("Service wasn't created") ;
            throw new StatusException(Status.failed("Service wasn't created")) ;
        }

          oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("XInteractionHandler.Request", new TestRequest()) ;

        return tEnv;
    } // finish method getTestEnvironment

}

