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
    static XSpreadsheetDocument xSpreadsheetDoc = null;

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
        XComponent oComp = (XComponent)
            UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc) ;
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
        XStyleFamiliesSupplier oStyleFamiliesSupplier = (XStyleFamiliesSupplier)
            UnoRuntime.queryInterface(
                XStyleFamiliesSupplier.class, xSpreadsheetDoc);

        XNameAccess oStyleFamilies = oStyleFamiliesSupplier.getStyleFamilies();
        XIndexAccess oStyleFamiliesIndexAccess = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oStyleFamilies);
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
        XComponent xComp = (XComponent)
            UnoRuntime.queryInterface(XComponent.class, xSpreadsheetDoc);

        XInterface oInstance = (XInterface)
            SOF.createInstance(xComp,"com.sun.star.style.CellStyle");

        // insert a Style which can be replaced by name
        XNameContainer oContainer = (XNameContainer)
            UnoRuntime.queryInterface(
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

