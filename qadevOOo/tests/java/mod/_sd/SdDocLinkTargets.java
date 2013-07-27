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



package mod._sd;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.document.XLinkTargetSupplier;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.document.LinkTargets</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
* </ul>
* @see com.sun.star.document.LinkTargets
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
*/
public class SdDocLinkTargets extends TestCase {
    XComponent xDrawDoc;

    /**
    * Creates Drawing document.
    */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a draw document" );
            xDrawDoc = SOF.createDrawDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Drawing document.
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xDrawDoc");
        util.DesktopTools.closeDoc(xDrawDoc);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the possible links to childrens of the drawing document using
    * the interface <code>XLinkTargetSupplier</code>. The retrieved collection
    * is the instance of the service <code>com.sun.star.document.LinkTargets</code>.
    * @see com.sun.star.document.XLinkTargetSupplier
    * @see com.sun.star.document.LinkTargets
    */
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {

        // get the Links here
        log.println( "getting Links" );
        XLinkTargetSupplier oLTS = (XLinkTargetSupplier)
            UnoRuntime.queryInterface(XLinkTargetSupplier.class,xDrawDoc);
        XInterface oObj = oLTS.getLinks();

        log.println( "creating a new environment for drawpage object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        return tEnv;
    } // finish method createTestEnvironment

}    // finish class SdDocLinkTargets

