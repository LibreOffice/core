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
            xImpressDoc = SOF.createImpressDoc(null);;
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
        util.DesktopTools.closeDoc(xImpressDoc);;
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

