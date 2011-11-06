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



package mod._fileacc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.ucb.SimpleFileAccess</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::ucb.XSimpleFileAccess</code></li>
* </ul>
* This object test <b> can </b> be run in several
* threads concurently.
* @see com.sun.star.ucb.XSimpleFileAccess
* @see com.sun.star.ucb.SimpleFileAccess
* @see ifc.ucb._XSimpleFileAccess
*/
public class SimpleFileAccess extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates service <code>com.sun.star.ucb.SimpleFileAccess</code>.
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;
        Object oIH = null;

        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();
            oInterface = xMSF.createInstance
                ("com.sun.star.comp.ucb.SimpleFileAccess");
            oIH = xMSF.createInstance("com.sun.star.sdb.InteractionHandler");
        } catch( com.sun.star.uno.Exception e ) {
            log.println("SimpleFileAccess Service not available" );
        }

        oObj = (XInterface) oInterface;

        log.println( "    creating a new environment for CoreReflection object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("InteractionHandler", oIH);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SimpleFileAccess

