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

import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.presentation.XCustomPresentationSupplier;
import com.sun.star.presentation.XPresentationSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.presentation.Presentation</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::presentation::Presentation</code></li>
*  <li> <code>com::sun::star::presentation::XPresentation</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
* </ul>
* @see com.sun.star.presentation.Presentation
* @see com.sun.star.presentation.XPresentation
* @see com.sun.star.beans.XPropertySet
* @see ifc.presentation._Presentation
* @see ifc.presentation._XPresentation
* @see ifc.beans._XPropertySet
*/
public class SdXPresentation extends TestCase {
    XComponent xImpressDoc;

    /**
    * Creates Impress document.
    */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a draw document" );
            xImpressDoc = SOF.createImpressDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
    * Disposes Impress document.
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xImpressDoc");
        util.DesktopTools.closeDoc(xImpressDoc);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the presentation from the document using the interface
    * <code>XPresentationSupplier</code>. The retrieved presentation is the
    * instance of the service <code>com.sun.star.presentation.Presentation</code>.
    * Retrieves the collection of the customized presentations from the document
    * using the interface <code>XCustomPresentationSupplier</code>.
    * Creates and inserts two new instances of presentation to the retrieved
    * collection.
    * Object relations created :
    * <ul>
    *  <li> <code>'Presentation'</code> for
    *      {@link ifc.presentation._Presentation}(the retrieved presentation)</li>
    * </ul>
    * @see com.sun.star.presentation.XCustomPresentationSupplier
    * @see com.sun.star.presentation.Presentation
    * @see com.sun.star.presentation.XCustomPresentationSupplier
    */
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        log.println( "get presentation" );
        XPresentationSupplier oPS = (XPresentationSupplier)
            UnoRuntime.queryInterface(XPresentationSupplier.class, xImpressDoc);
        XInterface oObj = oPS.getPresentation();

        log.println( "get custom presentation" );
        XCustomPresentationSupplier oCPS = (XCustomPresentationSupplier)
            UnoRuntime.queryInterface(
                XCustomPresentationSupplier.class, xImpressDoc);
        XNameContainer xCP = oCPS.getCustomPresentations();

        XInterface oInstance = null;
        XInterface oInstance2 = null;

        XSingleServiceFactory oSingleMSF = (XSingleServiceFactory)
            UnoRuntime.queryInterface(XSingleServiceFactory.class, xCP);

        try{
            oInstance = (XInterface) oSingleMSF.createInstance();
            oInstance2 = (XInterface) oSingleMSF.createInstance();
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }

        try {
            xCP.insertByName("FirstPresentation",oInstance);
            xCP.insertByName("SecondPresentation", oInstance2);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Could't insert Instance", e);
        } catch (com.sun.star.container.ElementExistException e) {
            e.printStackTrace(log);
            throw new StatusException("Could't insert Instance", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Could't insert Instance", e);
        }

        log.println( "creating a new environment for XPresentation object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("Presentation",oObj);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SdPresentation

