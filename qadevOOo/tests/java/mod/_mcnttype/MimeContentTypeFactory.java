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



package mod._mcnttype;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.mcnttype.MimeContentTypeFactory</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::datatransfer::XMimeContentTypeFactory</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
*  <li> <code>com::sun::star::lang::XTypeProvider</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurrently.
* @see com.sun.star.datatransfer.XMimeContentTypeFactory
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.lang.XTypeProvider
* @see com.sun.star.mcnttype.MimeContentTypeFactory
* @see ifc.datatransfer._XMimeContentTypeFactory
* @see ifc.lang._XServiceInfo
* @see ifc.lang._XTypeProvider
*/
public class MimeContentTypeFactory extends TestCase {

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates an instance of the service
    * <code>com.sun.star.datatransfer.MimeContentTypeFactory</code>. <p>
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
                                                  PrintWriter log )
                                                    throws StatusException {
        XInterface oObj = null;
        Object oInterface = null;
        XMultiServiceFactory xMSF = (XMultiServiceFactory)Param.getMSF();


        try {
            oInterface = xMSF.createInstance
                ( "com.sun.star.datatransfer.MimeContentTypeFactory" );
        } catch( com.sun.star.uno.Exception e ) {
            log.println("Can't create an object." );
            throw new StatusException( "Can't create an object", e );
        }

        oObj = (XInterface) oInterface;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method getTestEnvironment

}

