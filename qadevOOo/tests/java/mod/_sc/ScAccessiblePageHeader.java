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

import lib.Status;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XHeaderFooterContent;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.table.XCell;
import com.sun.star.text.XText;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XURLTransformer;

/**
 * Test for object which is represented by accessible component of
 * a printed header in 'Page Preview' mode.
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleContext
 */
public class ScAccessiblePageHeader extends TestCase {

    private XSpreadsheetDocument xSpreadsheetDoc = null;

    /**
     * Called to create an instance of <code>TestEnvironment</code>
     * with an object to test and related objects.
     * Switches the document to Print Preview mode.
     * Obtains accessible object for the page view.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment
     */
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {

        XInterface oObj = null;

        // inserting some content to have non-empty page preview
        XCell xCell = null;
        try {
            XSpreadsheets oSheets = xSpreadsheetDoc.getSheets() ;
            XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            XSpreadsheet oSheet = null;
            try {
                oSheet = (XSpreadsheet) AnyConverter.toObject(
                        new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));
            } catch (com.sun.star.lang.IllegalArgumentException iae) {
                throw new StatusException("couldn't get sheet",iae);
            }
            xCell = oSheet.getCellByPosition(0, 0) ;
            xCell.setFormula("ScAccessiblePageHeader");
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception ceating relation :");
            e.printStackTrace(log);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception ceating relation :");
            e.printStackTrace(log);
        }

        XModel aModel = UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);

        XController xController = aModel.getCurrentController();

        // switching to 'Page Preview' mode
        try {
            XDispatchProvider xDispProv = UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            XURLTransformer xParser = UnoRuntime.queryInterface(XURLTransformer.class,
         ( (XMultiServiceFactory) Param.getMSF()).createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:PrintPreview";
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            if(xDispatcher != null)
                xDispatcher.dispatch( aURL, null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't change mode");
            throw new StatusException(Status.failed("Couldn't change mode"));
        }

        try {
            Thread.sleep(500);
        } catch (InterruptedException ex) {}

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = AccessibilityTools.getCurrentWindow( (XMultiServiceFactory) Param.getMSF(), aModel);
        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        oObj = AccessibilityTools.getAccessibleObjectForRole
            (xRoot, AccessibleRole.HEADER, "");

        log.println("ImplementationName " + utils.getImplName(oObj));
        AccessibilityTools.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        XStyleFamiliesSupplier StyleFam = UnoRuntime.queryInterface(
            XStyleFamiliesSupplier.class,
            xSpreadsheetDoc );
        XNameAccess StyleFamNames = StyleFam.getStyleFamilies();
        XStyle StdStyle = null;

        try{
            XNameAccess PageStyles = (XNameAccess) AnyConverter.toObject(
                            new Type(XNameAccess.class),
                                        StyleFamNames.getByName("PageStyles"));
            StdStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class), PageStyles.getByName("Default"));
        } catch(com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by name", e);
        } catch(com.sun.star.container.NoSuchElementException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get by name", e);
        } catch (com.sun.star.lang.IllegalArgumentException iae) {
           throw new StatusException("Couldn't convert any", iae);
        }

        //get the property-set
        final XPropertySet PropSet = UnoRuntime.queryInterface(XPropertySet.class, StdStyle);

        XHeaderFooterContent RPHC = null;
        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );
        try {
            RPHC = (XHeaderFooterContent) AnyConverter.toObject(
                new Type(XHeaderFooterContent.class),
                    PropSet.getPropertyValue("RightPageHeaderContent"));
        } catch(com.sun.star.lang.WrappedTargetException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get HeaderContent", e);
        } catch(com.sun.star.beans.UnknownPropertyException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get HeaderContent", e);
        } catch(com.sun.star.lang.IllegalArgumentException e){
            e.printStackTrace(log);
            throw new StatusException("Couldn't get HeaderContent", e);
        }

        final XHeaderFooterContent RPHC2 = RPHC;

        final XText center = RPHC2.getCenterText();
        final XText left = RPHC2.getLeftText();
        final XText right = RPHC2.getRightText();

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    center.setString("CENTER");
                    left.setString("LEFT");
                    right.setString("RIGHT");
                    try {
                        PropSet.setPropertyValue("RightPageHeaderContent",RPHC2);
                    } catch (com.sun.star.beans.UnknownPropertyException e) {
                    } catch (com.sun.star.beans.PropertyVetoException e) {
                    } catch (com.sun.star.lang.IllegalArgumentException e) {
                    } catch (com.sun.star.lang.WrappedTargetException e) {}
                }
            });


        return tEnv;

    }

    /**
     * Called while disposing a <code>TestEnvironment</code>.
     * Disposes calc document.
     * @param Param test parameters
     * @param log writer to log information while testing
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println( "    disposing xSheetDoc " );
        try {
        XCloseable oComp = UnoRuntime.queryInterface (XCloseable.class, xSpreadsheetDoc) ;
        oComp.close(true);
        } catch(com.sun.star.util.CloseVetoException e) {
            log.println("Couldn't close document: "+e.getMessage());
        }
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(  (XMultiServiceFactory) Param.getMSF());

        try {
            log.println("creating a spreadsheetdocument");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document ", e );
        }
    }
}
