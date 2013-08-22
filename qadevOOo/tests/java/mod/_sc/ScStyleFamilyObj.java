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

package mod._sc;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.style.StyleFamily</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameContainer</code></li>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XNameReplace</code></li>
* </ul>
* @see com.sun.star.style.StyleFamily
* @see com.sun.star.container.XNameContainer
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XNameReplace
* @see ifc.container._XNameContainer
* @see ifc.container._XNameAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XIndexAccess
* @see ifc.container._XNameReplace
*/
public class ScStyleFamilyObj extends TestCase {
    private XSpreadsheetDocument xSpreadsheetDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSpreadsheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
    * Disposes Spreadsheet document.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves the collection of style families from the document
    * using the interface <code>XStyleFamiliesSupplier</code>.
    * Obtains style family with index 0 from the collection.
    * Creates the two instance of the service <code>com.sun.star.style.CellStyle</code>.
    * One of the instance uses for inserting of new style to the obtained style
    * family that is the instance of the service
    * <code>com.sun.star.style.StyleFamily</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'SecondInstance'</code> for
    *      {@link ifc.container._XNameContainer}(the second created instance of
    *      the service <code>com.sun.star.style.CellStyle</code>)</li>
    *  <li> <code>'XNameReplaceINDEX'</code> for
    *      {@link ifc.container._XNameContainer}(the number of the current
    *      running threads multiplied by two)</li>
    *  <li> <code>'INSTANCE1', ..., 'INSTANCEN'</code> for
    *      {@link ifc.container._XNameContainer}(the created instances of
    *      the service <code>com.sun.star.style.CellStyle</code>)</li>
    * </ul>
    * @see com.sun.star.style.CellStyle
    */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XNameAccess oStyleFamilyNameAccess = null;
        // create testobject here
        log.println("getting style");
        XStyleFamiliesSupplier oStyleFamiliesSupplier = UnoRuntime.queryInterface(
            XStyleFamiliesSupplier.class, xSpreadsheetDoc);

        XNameAccess oStyleFamilies = oStyleFamiliesSupplier.getStyleFamilies();
        XIndexAccess oStyleFamiliesIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, oStyleFamilies);
        try {
            oStyleFamilyNameAccess = (XNameAccess) AnyConverter.toObject(
                new Type(XNameAccess.class),
                    oStyleFamiliesIndexAccess.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception occurred while getting StyleFamily", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception occurred while getting StyleFamily", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception occurred while getting StyleFamily", e);
        }

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());
        XComponent xComp = UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc);

        XInterface oInstance = (XInterface)
            SOF.createInstance(xComp,"com.sun.star.style.CellStyle");

        // insert a Style which can be replaced by name
        XNameContainer oContainer = UnoRuntime.queryInterface(
            XNameContainer.class, oStyleFamilyNameAccess);
        try {
            oContainer.insertByName("ScStyleFamilyObj", oInstance);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't insert new style family", e);
        } catch (com.sun.star.container.ElementExistException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't insert new style family", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't insert new style family", e);
        }

        TestEnvironment tEnv = new TestEnvironment(oStyleFamilyNameAccess);

        oInstance = (XInterface)
            SOF.createInstance(xComp, "com.sun.star.style.CellStyle");

        //second instance for insertByName in XNameContainer
        tEnv.addObjRelation("SecondInstance", oInstance);

        int THRCNT = 1;
        if ((String)tParam.get("THRCNT") != null) {
            THRCNT = Integer.parseInt((String)tParam.get("THRCNT"));
        }

        log.println( "adding XNameReplaceINDEX as mod relation to environment" );
        tEnv.addObjRelation("XNameReplaceINDEX", new Integer(2*THRCNT).toString());

        // INSTANCEn : _XNameContainer; _XNameReplace
        log.println( "adding INSTANCEn as mod relation to environment" );
        for (int n = 1; n < 2*(THRCNT+1) ;n++ ) {
            log.println( "adding INSTANCE" + n +
                                            " as mod relation to environment" );
            tEnv.addObjRelation("INSTANCE" + n, SOF.createInstance(
                                    xComp,"com.sun.star.style.CellStyle"));
        }
        // NAMEREPLACE : _XNameReplace
        log.println("adding NAMEREPLACE as mod relation to environment");
        String cName = "ScStyleFamilyObj";
        tEnv.addObjRelation("NAMEREPLACE", cName);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class ScStyleFamilyObj

