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

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.DDELink</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::util::XRefreshable</code></li>
*  <li> <code>com::sun::star::sheet::XDDELink</code></li>
* </ul>
* The following files used by this test :
* <ul>
*  <li><b> ScDDELinksObj.ods </b> : the predefined testdocument </li>
* </ul> <p>
* @see com.sun.star.sheet.DDELink
* @see com.sun.star.container.XNamed
* @see com.sun.star.util.XRefreshable
* @see com.sun.star.sheet.XDDELink
* @see ifc.container._XNamed
* @see ifc.util._XRefreshable
* @see ifc.sheet._XDDELink
*/
public class ScDDELinkObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;
    static XComponent oDoc = null;

    /**
    * Creates Spreadsheet document.
    */
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a Spreadsheet document" );
            xSheetDoc = SOF.createCalcDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

    }

    /**
    * Disposes Spreadsheet document and testdocument if it was loaded already.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface(XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
        if (oDoc != null) {
            util.DesktopTools.closeDoc(oDoc);
        }
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Disposes the testdocument if it was loaded already.
    * Creates an instance of the <code>com.sun.star.frame.Desktop</code>
    * and loads the predefined testdocument. Retrieves a collection of
    * spreadsheets from a document and takes one of them. Sets specific formula
    * to some cells in the spreadsheet(the formula specify DDE links to the
    * testdocument). Retrieves the collection of DDE links in the document and
    * retrives first DDE link from the collection.
    * The retrived DDE link is the instance of the service
    * <code>com.sun.star.sheet.DDELink</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'setName'</code> for
    *      {@link ifc.container._XNamed}(specify that the method
    *      <code>setName</code> must not be tested)</li>
    *  <li> <code>'APPLICATION'</code> for
    *      {@link ifc.sheet._XDDELink}(the name of the current application)</li>
    *  <li> <code>'ITEM'</code> for
    *      {@link ifc.sheet._XDDELink}(the DDE item that was set in the formula)</li>
    *  <li> <code>'TOPIC'</code> for
    *      {@link ifc.sheet._XDDELink}(the full testdocument name)</li>
    * </ul>
    * @see com.sun.star.frame.Desktop
    * @see com.sun.star.sheet.DDELink
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        if (oDoc != null) {
            util.DesktopTools.closeDoc(oDoc);
        }

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        // create testobject here

        XMultiServiceFactory oMSF = (XMultiServiceFactory)Param.getMSF();

        // load the predefined testdocument
        String testdoc = util.utils.getFullTestURL("ScDDELinksObj.ods");
        try {
            oDoc = SOfficeFactory.getFactory(oMSF).loadDocument(testdoc);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't load test document", e);
        } catch (com.sun.star.io.IOException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't load test document", e);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Can't load test document", e);
        }

        log.println("getting sheets");
        XSpreadsheets xSpreadsheets = xSheetDoc.getSheets();

        log.println("getting a sheet");
        XSpreadsheet oSheet = null;
        XIndexAccess oIndexAccess = UnoRuntime.queryInterface(XIndexAccess.class, xSpreadsheets);
        try {
            oSheet = (XSpreadsheet) AnyConverter.toObject(
                    new Type(XSpreadsheet.class),oIndexAccess.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't get a spreadsheet", e);
        }

        log.println("filling some cells");
        String sAppl = "soffice";
        String sItem = "Sheet1.A1";
        testdoc = util.utils.getFullTestDocName("ScDDELinksObj.ods");
        try {
            oSheet.getCellByPosition(5, 5).setFormula(
                "=DDE(\""+ sAppl +"\";\""+testdoc+"\";\""+ sItem +"\"");
            oSheet.getCellByPosition(1, 4).setFormula(
                "=DDE(\""+ sAppl +"\";\""+testdoc+"\";\""+ sItem +"\"");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception occurred while filling cells", e);
        }

        try {
            log.println("Getting test object ") ;

            // Getting named ranges.
            XPropertySet docProps = UnoRuntime.queryInterface(XPropertySet.class, xSheetDoc);
            XNameAccess links = (XNameAccess) AnyConverter.toObject(
                new Type(XNameAccess.class), UnoRuntime.queryInterface(
                    XNameAccess.class, docProps.getPropertyValue("DDELinks")));

            String[] linkNames = links.getElementNames();

            oObj = (XInterface) AnyConverter.toObject(
                new Type(XInterface.class),links.getByName(linkNames[0]));
            log.println("Creating object - " +
                                        ((oObj == null) ? "FAILED" : "OK"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document", e) ;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document", e) ;
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document", e) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document", e) ;
        }

        TestEnvironment tEnv = new TestEnvironment( oObj );

        // Other parameters required for interface tests
        tEnv.addObjRelation("APPLICATION", sAppl);
        tEnv.addObjRelation("ITEM", sItem);
        tEnv.addObjRelation("TOPIC", testdoc);
        tEnv.addObjRelation("setName", new Boolean(true));

        return tEnv;
    }

}


