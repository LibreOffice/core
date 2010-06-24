/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.presentation.CustomPresentationAccess</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XNameReplace</code></li>
*  <li> <code>com::sun::star::lang::XSingleServiceFactory</code></li>
*  <li> <code>com::sun::star::container::XNameContainer</code></li>
* </ul>
* @see com.sun.star.presentation.CustomPresentationAccess
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XNameReplace
* @see com.sun.star.lang.XSingleServiceFactory
* @see com.sun.star.container.XNameContainer
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XNameReplace
* @see ifc.lang._XSingleServiceFactory
* @see ifc.container._XNameContainer
*/
public class SdXCustomPresentationAccess extends TestCase {
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
            xImpressDoc = SOF.createImpressDoc(null);;
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Impress document.
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing xImpressDoc");
        util.DesktopTools.closeDoc(xImpressDoc);;
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of the CustomPresentation from the document
    * using the interface <code>XCustomPresentationSupplier</code>.
    * The retrieved collection is the instance of the servcie
    * <code>com.sun.star.presentation.CustomPresentationAccess</code>.
    * Creates two new instances of the custom presentation using the interface
    * <code>XSingleServiceFactory</code>. Inserts one of them to the
    * retrieved collection. Creates and inserts several instances of
    * the presentation.
    * Object relations created :
    * <ul>
    *  <li> <code>'SecondInstance'</code> for
    *      {@link ifc.container._XNameContainer}(the second created instance
    *      of the custom presentation)</li>
    *  <li> <code>'XNameContainerINDEX'</code> for
    *      {@link ifc.container._XNameContainer}(the number of current running
    *      threads)</li>
    *  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> for
    *      {@link ifc.container._XNameReplace},
    *      {@link ifc.container._XNameContainer}(the created instances of
    *      the presentation)</li>
    * </ul>
    * @see com.sun.star.presentation.XCustomPresentationSupplier
    * @see com.sun.star.lang.XSingleServiceFactory
    * @see com.sun.star.presentation.CustomPresentationAccess
    */
    public TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {


        log.println( "creating a test environment" );

        log.println( "get presentation" );
        XCustomPresentationSupplier oPS = (XCustomPresentationSupplier)
            UnoRuntime.queryInterface(
                XCustomPresentationSupplier.class, xImpressDoc);
        XInterface oObj = oPS.getCustomPresentations();


        XSingleServiceFactory oSingleMSF = (XSingleServiceFactory)
            UnoRuntime.queryInterface(XSingleServiceFactory.class, oObj);

        XInterface oInstance = null;
        XInterface oSecondInstance = null;
        try{
            oInstance = (XInterface) oSingleMSF.createInstance();
            oSecondInstance = (XInterface) oSingleMSF.createInstance();
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create instance", e);
        }

        XNameContainer aContainer = (XNameContainer)
            UnoRuntime.queryInterface(XNameContainer.class, oObj);

        if (aContainer.hasByName("FirstPresentation")) {
            try {
                aContainer.removeByName("FirstPresentation");
            } catch (com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace(log);
                throw new StatusException("Exception while removing instance", e);
            } catch (com.sun.star.container.NoSuchElementException e) {
                e.printStackTrace(log);
                throw new StatusException("Exception while removing instance", e);
            }
        }

        try {
            aContainer.insertByName("FirstPresentation",oInstance);
        } catch (com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException("Could't insert Instance", e);
        } catch (com.sun.star.container.ElementExistException e){
            e.printStackTrace(log);
            throw new StatusException("Could't insert Instance", e);
        } catch (com.sun.star.lang.IllegalArgumentException e){
            e.printStackTrace(log);
            throw new StatusException("Could't insert Instance", e);
        }

        log.println( "creating a new environment for XPresentation object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        // adding an instance for insertByName ('XNameContainer')
        tEnv.addObjRelation("SecondInstance", oSecondInstance);

        int THRCNT = 1;
        if ((String)Param.get("THRCNT") != null) {
            THRCNT = Integer.parseInt((String)Param.get("THRCNT"));
        }

        // INDEX : _XNameContainer
        log.println( "adding XNameContainerINDEX as mod relation to environment" );
        tEnv.addObjRelation("XNameContainerINDEX",
            (new Integer(THRCNT)).toString());

        // INSTANCEn : _XNameContainer; _XNameReplace
        log.println( "adding INSTANCEn as mod relation to environment" );
        try {
            for (int n = 1; n < (2*THRCNT+1) ;n++ ) {
                log.println( "adding INSTANCE" + n
                    +" as mod relation to environment" );
                oInstance = (XInterface) oSingleMSF.createInstance();
                tEnv.addObjRelation("INSTANCE" + n, oInstance);
            }
        } catch (com.sun.star.uno.Exception e) {
            log.println("Could't adding INSTANCEn: " + e);
        }

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SdXCustomPresentationAccess

