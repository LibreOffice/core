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
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScIndexEnumeration_DDELinksEnumeration extends TestCase {
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
    * Disposes Spreadsheet document and testdocument.
    */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.
                                queryInterface(XComponent.class, xSheetDoc) ;
        util.DesktopTools.closeDoc(oComp);
        util.DesktopTools.closeDoc(oDoc);
    }

    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "Creating a test environment" );

        // create testobject here

        XMultiServiceFactory oMSF = (XMultiServiceFactory)Param.getMSF();

        // load the predefined testdocument
        String testdoc = utils.getFullTestURL("ScDDELinksObj.ods");
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
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException( "Couldn't get a spreadsheet", e);
        }

        testdoc = utils.getFullTestDocName("ScDDELinksObj.ods");
        log.println("filling some cells");
        try {
            oSheet.getCellByPosition(5, 5).setFormula(
                "=DDE(\"soffice\";\""+testdoc+"\";\"Sheet1.A1\"");
            oSheet.getCellByPosition(1, 4).setFormula(
                "=DDE(\"soffice\";\""+testdoc+"\";\"Sheet1.A1\"");
            oSheet.getCellByPosition(2, 0).setFormula(
                "=DDE(\"soffice\";\""+testdoc+"\";\"Sheet1.A1\"");
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException(
                "Exception occurred while filling cells", e);
        }

        try {
            log.println("Getting test object ") ;

            // Getting named ranges.
            XPropertySet docProps = UnoRuntime.queryInterface(XPropertySet.class, xSheetDoc);
            oObj = (XInterface)AnyConverter.toObject(
                new Type(XInterface.class),docProps.getPropertyValue("DDELinks"));
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
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log) ;
            throw new StatusException(
                "Error getting test object from spreadsheet document", e) ;
        }

        XEnumerationAccess ea = UnoRuntime.queryInterface(XEnumerationAccess.class,oObj);

        oObj = ea.createEnumeration();

        log.println("ImplementationName: "+util.utils.getImplName(oObj));
        // creating test environment
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("ENUM",ea);

        return tEnv;
    }

}


