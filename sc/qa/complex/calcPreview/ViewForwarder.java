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

package complex.calcPreview;

// import complexlib.ComplexTestCase;

import com.sun.star.awt.XWindow;
import com.sun.star.container.XIndexAccess;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
// import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
// import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
// import com.sun.star.uno.Any;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;
import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.accessibility.XAccessibleValue;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;

import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.beans.XPropertySet;
// import com.sun.star.beans.XPropertySetInfo;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XHeaderFooterContent;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XText;
// import com.sun.star.drawing.XDrawPageSupplier;
// import com.sun.star.drawing.XDrawPage;
// import com.sun.star.drawing.XShape;
// import com.sun.star.drawing.XShapes;


// import com.sun.star.beans.Property;
// import com.sun.star.lang.XServiceInfo;



// import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.OfficeConnection;
import static org.junit.Assert.*;

/**
 * A complex test for the preview of Calc documents. This complex test
 * needs interaction from the user: documents have to be resized and moved.
 */
public class ViewForwarder {

    /** The MultiServiceFactory **/
    private XMultiServiceFactory mXMSF = null;

    /** Get all test methods.
     * @return The test methods.
     */
//    public String[] getTestMethodNames() {
//        return new String[]{"checkPositiveViewForwarder",
//                            "checkNegativeViewForwarder",
//                            "checkPreviewHeaderCells",
//                            "checkPreviewShape"
//                            };
//    }

    /**
     * Get a MultiServiceFactory from the Office before the test.
     */
    @Before public void before()
    {
        mXMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, connection.getComponentContext().getServiceManager());
        // SOfficeFactory SOF = SOfficeFactory.getFactory(mXMSF);

        // param = new TestParameters();
        // param.put("ServiceFactory", xMsf);
        // mXMSF = (XMultiServiceFactory)param.getMSF();
    }


    /** Create a spreadsheet document, insert some text, header and footer.
     * Let the user resize the document and check the contents.
     */
    @Test public void checkPositiveViewForwarder() {
        SOfficeFactory SOF = SOfficeFactory.getFactory( mXMSF );
        XSpreadsheetDocument xSpreadsheetDoc = null;

        try {
            System.out.println("Creating a spreadsheet document");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            fail(e.getMessage());
            return;
        }

        XInterface oObj = null;

        // inserting some content to have non-empty page preview
        XCell xCell = null;
        try {
            XSpreadsheets oSheets = xSpreadsheetDoc.getSheets() ;
            XIndexAccess oIndexSheets =
                UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            Object o = oIndexSheets.getByIndex(0);
            XSpreadsheet oSheet = (XSpreadsheet)AnyConverter.toObject(
                            new com.sun.star.uno.Type(XSpreadsheet.class), o);
            xCell = oSheet.getCellByPosition(0, 0) ;
            xCell.setFormula("ScAccessiblePageHeader");

            xCell = oSheet.getCellByPosition(0, 1) ;
            xCell.setFormula("Cell 1");
            xCell = oSheet.getCellByPosition(0, 2) ;
            xCell.setFormula("Cell 2");
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Exception ceating relation :");
            fail(e.getMessage());
        } catch(com.sun.star.lang.WrappedTargetException e) {
            System.out.println("Exception ceating relation :");
            fail(e.getMessage());
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Exception ceating relation :");
            fail(e.getMessage());
        }

        XModel aModel =
            UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);

        XController xController = aModel.getCurrentController();

        // get page styles
        XStyleFamiliesSupplier StyleFam =
            UnoRuntime.queryInterface(
                XStyleFamiliesSupplier.class,
                xSpreadsheetDoc );
        XNameAccess StyleFamNames = StyleFam.getStyleFamilies();
        XStyle StdStyle = null;

        try{
            Object o = StyleFamNames.getByName("PageStyles");
            XNameAccess PageStyles = (XNameAccess)AnyConverter.toObject(
                            new com.sun.star.uno.Type(XNameAccess.class), o);

            o = PageStyles.getByName("Default");
            StdStyle = (XStyle)AnyConverter.toObject(
                            new com.sun.star.uno.Type(XStyle.class), o);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            fail(e.getMessage());
        } catch(com.sun.star.lang.WrappedTargetException e){
            fail(e.getMessage());
            return;
        } catch(com.sun.star.container.NoSuchElementException e){
            fail(e.getMessage());
            return;
        }

        //get the property-set
        final XPropertySet PropSet =
            UnoRuntime.queryInterface(XPropertySet.class, StdStyle);

        XHeaderFooterContent RPHFC = null;

        // get the header
        try {
            Object o = PropSet.getPropertyValue("RightPageHeaderContent");
            RPHFC = (XHeaderFooterContent)AnyConverter.toObject(
                    new com.sun.star.uno.Type(XHeaderFooterContent.class), o);

        } catch(com.sun.star.lang.IllegalArgumentException e) {
            fail(e.getMessage());
        } catch(com.sun.star.lang.WrappedTargetException e){
            fail(e.getMessage());
            return;
        } catch(com.sun.star.beans.UnknownPropertyException e){
            fail(e.getMessage());
            return;
        }

        // insert text into header
        XText center = RPHFC.getCenterText();
        center.setString("First line \n\r Second line \n\r third line ");
        try {
            PropSet.setPropertyValue("RightPageHeaderContent",RPHFC);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.lang.WrappedTargetException e) {}

        // get footer
        RPHFC = null;
        try {
            Object o = PropSet.getPropertyValue("RightPageHeaderContent");
            RPHFC = (XHeaderFooterContent)AnyConverter.toObject(
                    new com.sun.star.uno.Type(XHeaderFooterContent.class), o);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            fail(e.getMessage());
        } catch(com.sun.star.lang.WrappedTargetException e){
            fail(e.getMessage());
            return;
        } catch(com.sun.star.beans.UnknownPropertyException e){
            fail(e.getMessage());
            return;
        }

        // insert text into footer
        center = RPHFC.getCenterText();
        center.setString("Third last line\n\r Second last line\n\r last line");
        try {
            PropSet.setPropertyValue("RightPageFooterContent",RPHFC);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.lang.WrappedTargetException e) {}

        // switching to 'Page Preview' mode
        try {
            XDispatchProvider xDispProv =
                UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            XURLTransformer xParser =
                UnoRuntime.queryInterface(XURLTransformer.class,
            mXMSF.createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an
            // array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:PrintPreview";
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            if(xDispatcher != null)
            {
                xDispatcher.dispatch( aURL, null );
            }
        } catch (com.sun.star.uno.Exception e) {
            fail("Couldn't change mode");
            return;
        }

        try {
            Thread.sleep(500);
        } catch (InterruptedException ex) {}

        System.out.println("Press any key after resizing ");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }

        // get the accessible content
        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow(mXMSF, aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);
        XAccessible parent = null;

        try {
            parent = at.getAccessibleObjectForRole
                (xRoot, AccessibleRole.HEADER, "").getAccessibleChild(0);
            oObj = parent.getAccessibleContext().getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }

        System.out.println("ImplementationName " + utils.getImplName(oObj));

        XAccessibleComponent accPC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, parent);

        System.out.println("Parent-BoundsX= "+accPC.getBounds().X);
        System.out.println("Parent-BoundsY= "+accPC.getBounds().Y);
        System.out.println("Parent-BoundsWidth= "+accPC.getBounds().Width);
        System.out.println("Parent-BoundsHeight= "+accPC.getBounds().Height);

        XAccessibleComponent accPPC =
                        UnoRuntime.queryInterface(XAccessibleComponent.class,
                        parent.getAccessibleContext().getAccessibleParent());

        System.out.println("P-Parent-BoundsX= "+accPPC.getBounds().X);
        System.out.println("P-Parent-BoundsY= "+accPPC.getBounds().Y);
        System.out.println("P-Parent-BoundsWidth= "+accPPC.getBounds().Width);
        System.out.println("P-Parent-BoundsHeight= "+accPPC.getBounds().Height);

        try {
            Thread.sleep(500);
        } catch (InterruptedException ex) {}

        System.out.println("Press any key when the second line is on top");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }

        try {
            System.out.println("ChildCount: "+
                    parent.getAccessibleContext().getAccessibleChildCount());
            System.out.println("Getting child 0 again");
            oObj = parent.getAccessibleContext().getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }

        XAccessibleText accT =
                UnoRuntime.queryInterface(XAccessibleText.class, oObj);
        System.out.println("Getting the text: "+accT.getText());

        XAccessibleComponent accC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, oObj);
        System.out.println("BoundsX= "+accC.getBounds().X);
        System.out.println("BoundsY= "+accC.getBounds().Y);
        System.out.println("BoundsWidth= "+accC.getBounds().Width);
        System.out.println("BoundsHeight= "+accC.getBounds().Height);

        accPC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, parent);

        System.out.println("Parent-BoundsX= "+accPC.getBounds().X);
        System.out.println("Parent-BoundsY= "+accPC.getBounds().Y);
        System.out.println("Parent-BoundsWidth= "+accPC.getBounds().Width);
        System.out.println("Parent-BoundsHeight= "+accPC.getBounds().Height);

        accPPC =
        UnoRuntime.queryInterface(XAccessibleComponent.class,
                        parent.getAccessibleContext().getAccessibleParent());

        System.out.println("P-Parent-BoundsX= "+accPPC.getBounds().X);
        System.out.println("P-Parent-BoundsY= "+accPPC.getBounds().Y);
        System.out.println("P-Parent-BoundsWidth= "+accPPC.getBounds().Width);
        System.out.println("P-Parent-BoundsHeight= "+accPPC.getBounds().Height);



        System.out.println("Press any key when the footer is visible.");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }

        try {
            parent = at.getAccessibleObjectForRole
                (xRoot, AccessibleRole.FOOTER, "").getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }

        accPC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, parent);

        try {
            System.out.println("ChildCount: "+
                    parent.getAccessibleContext().getAccessibleChildCount());
            System.out.println("Getting child 0 again");
            oObj = parent.getAccessibleContext().getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }

        accT =
                UnoRuntime.queryInterface(XAccessibleText.class, oObj);
        System.out.println("Getting the text: "+accT.getText());

        accC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, oObj);
        System.out.println("BoundsX= "+accC.getBounds().X);
        System.out.println("BoundsY= "+accC.getBounds().Y);
        System.out.println("BoundsWidth= "+accC.getBounds().Width);
        System.out.println("BoundsHeight= "+accC.getBounds().Height);

        accPC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, parent);

        System.out.println("Parent-BoundsX= "+accPC.getBounds().X);
        System.out.println("Parent-BoundsY= "+accPC.getBounds().Y);
        System.out.println("Parent-BoundsWidth= "+accPC.getBounds().Width);
        System.out.println("Parent-BoundsHeight= "+accPC.getBounds().Height);

        accPPC =
        UnoRuntime.queryInterface(XAccessibleComponent.class,
                        parent.getAccessibleContext().getAccessibleParent());

        System.out.println("P-Parent-BoundsX= "+accPPC.getBounds().X);
        System.out.println("P-Parent-BoundsY= "+accPPC.getBounds().Y);
        System.out.println("P-Parent-BoundsWidth= "+accPPC.getBounds().Width);
        System.out.println("P-Parent-BoundsHeight= "+accPPC.getBounds().Height);


        System.out.println("Press any key when the page content is on top");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }

        try {
            parent = at.getAccessibleObjectForRole
                (xRoot, AccessibleRole.DOCUMENT, "").getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }
        System.out.println("PARENT: " + parent.getAccessibleContext().getAccessibleName());
        accPC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, parent);

        int cCount = 0;
        try {
            cCount =
               parent.getAccessibleContext().getAccessibleChildCount();
            System.out.println("ChildCount: "+cCount);
            System.out.println("Getting child 0 again");
            oObj = parent.getAccessibleContext().getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }

        for (int i=0; i<cCount; i++) {
            try {
                XAccessible xA = parent.getAccessibleContext().getAccessibleChild(i);
                System.out.println("NAME object " + i + ": " + xA.getAccessibleContext().getAccessibleName());
            } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
                fail(iabe.getMessage());
                return;
            }
        }

        System.out.println("SERVICES:");
        util.dbg.getSuppServices(oObj);

        XAccessibleValue accV =
                UnoRuntime.queryInterface(XAccessibleValue.class, oObj);
        Object o = accV.getCurrentValue();
        if (o instanceof String)
        {
            System.out.println("Value: " + (String)o);
        }
        else
        {
            System.out.println("Name of Object: " + o.getClass().getName());
        }
        System.out.println("Getting the value: "+accV.getCurrentValue());




        accC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, oObj);
        System.out.println("BoundsX= "+accC.getBounds().X);
        System.out.println("BoundsY= "+accC.getBounds().Y);
        System.out.println("BoundsWidth= "+accC.getBounds().Width);
        System.out.println("BoundsHeight= "+accC.getBounds().Height);

        accPC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, parent);

        System.out.println("Parent-BoundsX= "+accPC.getBounds().X);
        System.out.println("Parent-BoundsY= "+accPC.getBounds().Y);
        System.out.println("Parent-BoundsWidth= "+accPC.getBounds().Width);
        System.out.println("Parent-BoundsHeight= "+accPC.getBounds().Height);

        accPPC =
                UnoRuntime.queryInterface(XAccessibleComponent.class,
                        parent.getAccessibleContext().getAccessibleParent());

        System.out.println("P-Parent-BoundsX= "+accPPC.getBounds().X);
        System.out.println("P-Parent-BoundsY= "+accPPC.getBounds().Y);
        System.out.println("P-Parent-BoundsWidth= "+accPPC.getBounds().Width);
        System.out.println("P-Parent-BoundsHeight= "+accPPC.getBounds().Height);

        XComponent xComp = UnoRuntime.queryInterface(
                                    XComponent.class, xSpreadsheetDoc);
        xComp.dispose();
    }








    /**
     * Create a spreadsheet document, insert some text, header and footer.
     * Let the user resize the document and check the contents.
     */
    @Test public void checkNegativeViewForwarder() {
        SOfficeFactory SOF = SOfficeFactory.getFactory( mXMSF );
        XSpreadsheetDocument xSpreadsheetDoc = null;

        try {
            System.out.println("Creating a spreadsheet document");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            fail(e.getMessage());
            return;
        }

        XInterface oObj = null;

        // inserting some content to have non-empty page preview
        XCell xCell = null;
        try {
            XSpreadsheets oSheets = xSpreadsheetDoc.getSheets() ;
            XIndexAccess oIndexSheets =
                UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            Object o = oIndexSheets.getByIndex(0);
            XSpreadsheet oSheet = (XSpreadsheet)AnyConverter.toObject(
                            new com.sun.star.uno.Type(XSpreadsheet.class), o);
            xCell = oSheet.getCellByPosition(0, 0) ;
            xCell.setFormula("ScAccessiblePageHeader");

            xCell = oSheet.getCellByPosition(0, 1) ;
            xCell.setFormula("Cell 1");
            xCell = oSheet.getCellByPosition(0, 2) ;
            xCell.setFormula("Cell 2");
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("Exception ceating relation :");
            fail(e.getMessage());
        } catch(com.sun.star.lang.WrappedTargetException e) {
            System.out.println("Exception ceating relation :");
            fail(e.getMessage());
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            System.out.println("Exception ceating relation :");
            fail(e.getMessage());
        }

        XModel aModel =
            UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);

        XController xController = aModel.getCurrentController();

        // get page styles
        XStyleFamiliesSupplier StyleFam =
            UnoRuntime.queryInterface(
                XStyleFamiliesSupplier.class,
                xSpreadsheetDoc );
        XNameAccess StyleFamNames = StyleFam.getStyleFamilies();
        XStyle StdStyle = null;

        try{
            Object o = StyleFamNames.getByName("PageStyles");
            XNameAccess PageStyles = (XNameAccess)AnyConverter.toObject(
                            new com.sun.star.uno.Type(XNameAccess.class), o);

            o = PageStyles.getByName("Default");
            StdStyle = (XStyle)AnyConverter.toObject(
                            new com.sun.star.uno.Type(XStyle.class), o);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            fail(e.getMessage());
        } catch(com.sun.star.lang.WrappedTargetException e){
            fail(e.getMessage());
            return;
        } catch(com.sun.star.container.NoSuchElementException e){
            fail(e.getMessage());
            return;
        }

        //get the property-set
        final XPropertySet PropSet =
            UnoRuntime.queryInterface(XPropertySet.class, StdStyle);

        XHeaderFooterContent RPHFC = null;

        // get the header
        System.out.println( "Creating a test environment" );
        try {
            Object o = PropSet.getPropertyValue("RightPageHeaderContent");
            RPHFC = (XHeaderFooterContent)AnyConverter.toObject(
                    new com.sun.star.uno.Type(XHeaderFooterContent.class), o);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            fail(e.getMessage());
        } catch(com.sun.star.lang.WrappedTargetException e){
            fail(e.getMessage());
            return;
        } catch(com.sun.star.beans.UnknownPropertyException e){
            fail(e.getMessage());
            return;
        }

        // insert text into header
        XText center = RPHFC.getCenterText();
        center.setString("First line \n\r Second line \n\r third line ");
        try {
            PropSet.setPropertyValue("RightPageHeaderContent",RPHFC);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.lang.WrappedTargetException e) {}

        // get footer
        RPHFC = null;
        try {
            Object o = PropSet.getPropertyValue("RightPageFooterContent");
            RPHFC = (XHeaderFooterContent)AnyConverter.toObject(
                    new com.sun.star.uno.Type(XHeaderFooterContent.class), o);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            fail(e.getMessage());
        } catch(com.sun.star.lang.WrappedTargetException e){
            fail(e.getMessage());
            return;
        } catch(com.sun.star.beans.UnknownPropertyException e){
            fail(e.getMessage());
            return;
        }

        // insert text into footer
        center = RPHFC.getCenterText();
        center.setString("Third last line\n\r Second last line\n\r last line");
        try {
            PropSet.setPropertyValue("RightPageFooterContent",RPHFC);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.lang.WrappedTargetException e) {}

        // switching to 'Page Preview' mode
        try {
            XDispatchProvider xDispProv =
                UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            XURLTransformer xParser =
                UnoRuntime.queryInterface(XURLTransformer.class,
            mXMSF.createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an
            // array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:PrintPreview";
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            if(xDispatcher != null)
            {
                xDispatcher.dispatch( aURL, null );
            }
        } catch (com.sun.star.uno.Exception e) {
            fail("Couldn't change mode");
            return;
        }

        try {
            Thread.sleep(500);
        } catch (InterruptedException ex) {}

        System.out.println("Press any key after resizing ");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }

        // get the accessible content
        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow(mXMSF, aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);
        XAccessible parent = null;

        try {
            parent = at.getAccessibleObjectForRole
                (xRoot, AccessibleRole.HEADER, "").getAccessibleChild(0);
            oObj = parent.getAccessibleContext().getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }

        System.out.println("ImplementationName " + utils.getImplName(oObj));

        XAccessibleComponent accPC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, parent);

        System.out.println("Parent-BoundsX= "+accPC.getBounds().X);
        System.out.println("Parent-BoundsY= "+accPC.getBounds().Y);
        System.out.println("Parent-BoundsWidth= "+accPC.getBounds().Width);
        System.out.println("Parent-BoundsHeight= "+accPC.getBounds().Height);

        XAccessibleComponent accPPC =
                        UnoRuntime.queryInterface(XAccessibleComponent.class,
                        parent.getAccessibleContext().getAccessibleParent());

        System.out.println("P-Parent-BoundsX= "+accPPC.getBounds().X);
        System.out.println("P-Parent-BoundsY= "+accPPC.getBounds().Y);
        System.out.println("P-Parent-BoundsWidth= "+accPPC.getBounds().Width);
        System.out.println("P-Parent-BoundsHeight= "+accPPC.getBounds().Height);

        try {
            Thread.sleep(500);
        } catch (InterruptedException ex) {}

        System.out.println("Press any key when the header is not visible.");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }

        int childCount = 0;
        childCount =
                parent.getAccessibleContext().getAccessibleChildCount();
        System.out.println("ChildCount: "+childCount);

        if (childCount != 0)
        {
            fail("Could access header although it was not visible on page.");
        }


        try {
            parent = at.getAccessibleObjectForRole
                (xRoot, AccessibleRole.FOOTER, "").getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }

        accPC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, parent);

        System.out.println("Press any key when the footer is not visible.");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }

        childCount =
                parent.getAccessibleContext().getAccessibleChildCount();
        System.out.println("ChildCount: "+childCount);

        if (childCount != 0)
        {
            fail("Could access footer although it was not visible on page.");
        }

        XComponent xComp =UnoRuntime.queryInterface(
                                        XComponent.class, xSpreadsheetDoc);
        xComp.dispose();

    }




    /**
     * Check the preview of header cells
     */
    @Test public void checkPreviewHeaderCells() {

        XInterface oObj = null;
        SOfficeFactory SOF = SOfficeFactory.getFactory( mXMSF);
        XSpreadsheetDocument xSpreadsheetDoc = null;

        try {
            System.out.println("Creating a spreadsheet document");
            xSpreadsheetDoc = SOF.createCalcDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            fail(e.getMessage());
            return;
        }

        XModel xModel =
            UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);

        XController xController = xModel.getCurrentController();

        //setting value of cell A1
        XCell xCell = null;
        try {
            System.out.println("Getting spreadsheet") ;
            XSpreadsheets oSheets = xSpreadsheetDoc.getSheets() ;
            XIndexAccess oIndexSheets =
            UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
            Object o = oIndexSheets.getByIndex(0);
            XSpreadsheet oSheet = (XSpreadsheet)AnyConverter.toObject(
                            new com.sun.star.uno.Type(XSpreadsheet.class), o);

            System.out.println("Getting a cell from sheet") ;
            xCell = oSheet.getCellByPosition(0, 0);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            fail(e.getMessage());
        } catch (com.sun.star.lang.WrappedTargetException e) {
            fail(e.getMessage());
            return;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            fail(e.getMessage());
            return;
        }

        xCell.setFormula("Value");

        //setting property 'PrintHeaders' of the style 'Default'
        XStyleFamiliesSupplier xSFS =
            UnoRuntime.queryInterface(XStyleFamiliesSupplier.class,
                                                            xSpreadsheetDoc);
        XNameAccess xNA = xSFS.getStyleFamilies();
        XPropertySet xPropSet = null;
        try {
            Object oPageStyles = xNA.getByName("PageStyles");
            xNA =
                UnoRuntime.queryInterface(XNameAccess.class, oPageStyles);
            Object oDefStyle = xNA.getByName("Default");
            xPropSet =
                UnoRuntime.queryInterface(XPropertySet.class, oDefStyle);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            fail(e.getMessage());
            return;
        } catch(com.sun.star.container.NoSuchElementException e) {
            fail(e.getMessage());
            return;
        }

        try {
            xPropSet.setPropertyValue("PrintHeaders", new Boolean(true));
        } catch(com.sun.star.lang.WrappedTargetException e) {
            fail(e.getMessage());
            return;
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            fail(e.getMessage());
            return;
        } catch(com.sun.star.beans.PropertyVetoException e) {
            fail(e.getMessage());
            return;
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            fail(e.getMessage());
            return;
        }

        //switching to 'Print Preview' mode
        try {
            XDispatchProvider xDispProv =
                UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            XURLTransformer xParser =
                UnoRuntime.queryInterface(XURLTransformer.class,
            mXMSF.createInstance("com.sun.star.util.URLTransformer"));
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:PrintPreview";
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            if(xDispatcher != null)
            {
                xDispatcher.dispatch( aURL, null );
            }
        } catch (com.sun.star.uno.Exception e) {
            fail(e.getMessage());
            return;
        }

        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow(mXMSF, xModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

//        System.out.println("ImplementationName " + utils.getImplName(oObj));

        System.out.println("Press any key when the header cell is on top.");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }


        XAccessible parent = null;

        try {
            System.out.println("Count: " +at.getAccessibleObjectForRole
                (xRoot, AccessibleRole.TABLE, "").getAccessibleChildCount());
//            parent = at.getAccessibleObjectForRole
//                (xRoot, AccessibleRole.TABLE, "").getAccessibleChild(0);
//            oObj = parent.getAccessibleContext().getAccessibleChild(1);
            System.out.println("Getting child 2");
            oObj = at.getAccessibleObjectForRole
                (xRoot, AccessibleRole.TABLE, "").getAccessibleChild(2);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }

        System.out.println("*** DEBUG INFO ***");
        util.dbg.getSuppServices(oObj);
        System.out.println("****** IFC ******");
        util.dbg.printInterfaces(oObj);
        System.out.println("*****************");


/*        XAccessibleContext accCtx = (XAccessibleContext)
                UnoRuntime.queryInterface(XAccessibleContext.class, oObj);
        try {
            System.out.println("Children now: " + accCtx.getAccessibleChild(0).getAccessibleContext().getAccessibleChildCount());
        } catch (com.sun.star.lang.IndexOutOfBoundsException iabe) {
            fail(iabe.getMessage());
            return;
        }
*/

        XAccessibleValue accV =
                UnoRuntime.queryInterface(XAccessibleValue.class, oObj);
        Object o = accV.getCurrentValue();
        System.out.println("Getting the value: "+o + " is void " + util.utils.isVoid(o));


        XAccessibleComponent accC =
                UnoRuntime.queryInterface(XAccessibleComponent.class, oObj);

        System.out.println("BoundsX= "+accC.getBounds().X);
        System.out.println("BoundsY= "+accC.getBounds().Y);
        System.out.println("BoundsWidth= "+accC.getBounds().Width);
        System.out.println("BoundsHeight= "+accC.getBounds().Height);

        XComponent xComp = UnoRuntime.queryInterface(
                                        XComponent.class, xSpreadsheetDoc);
        xComp.dispose();

    }





    /**
     * Check the preview of Shapes: load a document with shapes and see, if they
     * are accessible.
     */
    @Test public void checkPreviewShape() {
        SOfficeFactory SOF = SOfficeFactory.getFactory( mXMSF );
        XSpreadsheetDocument xSpreadsheetDoc = null;
        XComponent xComp = null;
        XInterface oObj = null;

        try {
            String docName = "calcshapes.sxc";
            System.out.println("Loading a spreadsheetdocument.");
            // String url = utils.getFullURL((String)param.get("TestDocumentPath") + "/" + docName);
            String url = TestDocument.getUrl(docName);
            System.out.println("loading document '" + url + "'");
            xComp = SOF.loadDocument(url);
            assertNotNull(xComp);
        }
        catch (com.sun.star.uno.Exception e) {
            fail(e.getMessage());
            return;
        }

        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }

        if (xComp == null) {
            fail("loading document failed.");
            return;
        }

        xSpreadsheetDoc = UnoRuntime.queryInterface(
                                            XSpreadsheetDocument.class, xComp);
        XModel aModel =
            UnoRuntime.queryInterface(XModel.class, xSpreadsheetDoc);

        XController xController = aModel.getCurrentController();

        // switching to 'Page Preview' mode
        try {
            XDispatchProvider xDispProv =
                UnoRuntime.queryInterface(XDispatchProvider.class, xController);
            XURLTransformer xParser =
                UnoRuntime.queryInterface(XURLTransformer.class,
            mXMSF.createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an
            // array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = ".uno:PrintPreview";
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch(aURL, "", 0);
            if(xDispatcher != null)
            {
                xDispatcher.dispatch( aURL, null );
            }
        } catch (com.sun.star.uno.Exception e) {
            fail("Couldn't change mode");
            return;
        }

        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }



        System.out.println("Press any key when a shape is on top.");
        try{
            byte[]b = new byte[16];
            System.in.read(b);
        } catch (Exception e) {
            e.printStackTrace();
        }

        // get the accessible content
        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow(mXMSF, aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);
        XAccessible parent = null;

        try {
            XAccessibleContext con = at.getAccessibleObjectForRole
                                (xRoot, AccessibleRole.SHAPE, "");
            System.out.println("Name of AccessibleContext: " + con.getAccessibleName());
            oObj = con;
        } catch (Exception e) {
            fail(e.getMessage());
            return;
        }

        System.out.println("ImplementationName: " + utils.getImplName(oObj));
        util.dbg.printInterfaces(oObj);

        xComp.dispose();
    }

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private static final OfficeConnection connection = new OfficeConnection();

}


