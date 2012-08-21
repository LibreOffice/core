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
package util;

import java.util.HashMap;
// access the implementations via names
import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;

import com.sun.star.uno.UnoRuntime;
// staroffice interfaces to provide desktop and componentloader
// and components i.e. spreadsheets, writerdocs etc.
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;

// name - value pair
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.PropertyState;

// additional classes required for testcase
import com.sun.star.sheet.*;
import com.sun.star.text.*;
import com.sun.star.container.*;
import com.sun.star.chart.*;
import com.sun.star.drawing.*;
import com.sun.star.awt.*;

public class SOfficeFactory {

    private static HashMap<String, SOfficeFactory> lookup = new HashMap<String, SOfficeFactory>(10);
    protected XComponentLoader oCLoader;

    private SOfficeFactory(XMultiServiceFactory xMSF) {
        // get XInterface of Desktop service
        Object oInterface;
        try {
            oInterface = xMSF.createInstance("com.sun.star.frame.Desktop");
        } catch (com.sun.star.uno.Exception e) {
            throw new IllegalArgumentException(
                "Desktop Service not available");
        }

        // query the desktop interface and then it's componentloader
        XDesktop oDesktop = UnoRuntime.queryInterface(
            XDesktop.class, oInterface);

        oCLoader = UnoRuntime.queryInterface(
            XComponentLoader.class, oDesktop);
    }

    public static SOfficeFactory getFactory(XMultiServiceFactory xMSF) {

        SOfficeFactory soFactory = lookup.get(new Integer(xMSF.hashCode()).toString());

        if (soFactory == null) {
            soFactory = new SOfficeFactory(xMSF);
            lookup.put(new Integer(xMSF.hashCode()).toString(), soFactory);
        }

        return soFactory;
    }

    // *********************************************************
    // Document creation. The documents needed are created here.
    // *********************************************************
    /**
     * method which opens a new TextDocument
     *
     * @see XTextDocument
     */
    public XTextDocument createTextDoc(String frameName)
        throws com.sun.star.uno.Exception {

        XComponent oDoc = openDoc("swriter", frameName);

        if (oDoc != null) {
            DesktopTools.bringWindowToFront(oDoc);
            return UnoRuntime.queryInterface(XTextDocument.class, oDoc);
        } else {
            return null;
        }

    } // finished createTextDoc

    /**
     * method which opens a new TextDocument
     *
     * @see XTextDocument
     */
    public XTextDocument createTextDoc(String frameName, PropertyValue[] mediaDescriptor)
        throws com.sun.star.uno.Exception {

        XComponent oDoc = openDoc("swriter", frameName, mediaDescriptor);

        if (oDoc != null) {
            DesktopTools.bringWindowToFront(oDoc);
            return UnoRuntime.queryInterface(XTextDocument.class, oDoc);
        } else {
            return null;
        }
    } // finished createTextDoc

    /**
     * method which opens a new SpreadsheetDocument
     *
     * @see XSpreadsheetDocument
     */
    public XSpreadsheetDocument createCalcDoc(String frameName)
        throws com.sun.star.uno.Exception {

        XComponent oDoc = openDoc("scalc", frameName);

        if (oDoc != null) {
            DesktopTools.bringWindowToFront(oDoc);
            return UnoRuntime.queryInterface(XSpreadsheetDocument.class, oDoc);
        } else {
            return null;
        }
    } // finished createCalcDoc

    /**
     * method which opens a new SpreadsheetDocument
     *
     * @see XSpreadsheetDocument
     */
    public XSpreadsheetDocument createCalcDoc(String frameName, PropertyValue[] mediaDescriptor)
        throws com.sun.star.uno.Exception {

        XComponent oDoc = openDoc("scalc", frameName, mediaDescriptor);

        if (oDoc != null) {
            DesktopTools.bringWindowToFront(oDoc);
            return UnoRuntime.queryInterface(XSpreadsheetDocument.class, oDoc);
        } else {
            return null;
        }
    } // finished createCalcDoc

    /**
     * method which opens a new DrawDocument
     */
    public XComponent createDrawDoc(String frameName)
        throws com.sun.star.uno.Exception {

        return openDoc("sdraw", frameName);
    } // finished createDrawDoc

    /**
     * method which opens a new ImpressDocument
     */
    /**
     * method which opens a new DrawDocument
     */
    public XComponent createDrawDoc(String frameName, PropertyValue[] mediaDescriptor)
        throws com.sun.star.uno.Exception {

        return openDoc("sdraw", frameName, mediaDescriptor);
    } // finished createDrawDoc

    /**
     * method which opens a new ImpressDocument
     */
    public XComponent createImpressDoc(String frameName)
        throws com.sun.star.uno.Exception {

        return openDoc("simpress", frameName);
    } // finished createImpressDoc

    /**
     * method which opens a new ImpressDocument
     */
    public XComponent createImpressDoc(String frameName, PropertyValue[] mediaDescriptor)
        throws com.sun.star.uno.Exception {

        return openDoc("simpress", frameName, mediaDescriptor);
    } // finished createImpressDoc

    /**
     * method which opens a new MathDocument
     */
    public XComponent createMathDoc(String frameName)
        throws com.sun.star.uno.Exception {

        return openDoc("smath", frameName);
    } // finished createMathDoc

    /**
     * method which opens a new MathDocument
     */
    public XComponent createMathDoc(String frameName, PropertyValue[] mediaDescriptor)
        throws com.sun.star.uno.Exception {

        return openDoc("smath", frameName, mediaDescriptor);
    } // finished createMathDoc

    /**
     * method which opens a new ChartDocument
     *
     * @see XChartDocument
     */
    public XChartDocument createChartDoc(String frameName)
        throws com.sun.star.uno.Exception {

//        XComponent oDoc = loadDocument(
//                            util.utils.getFullTestURL("emptyChart.sds"));

        XComponent oDoc = loadDocument("private:factory/schart");

        if (oDoc != null) {
            DesktopTools.bringWindowToFront(oDoc);
            return UnoRuntime.queryInterface(XChartDocument.class, oDoc);
        } else {
            return null;
        }

    } // finished createChartDoc

    /**
     * creates a simple TextTable defaultet to 2 rows and 2 columns
     */
    public static XTextTable createTextTable(XTextDocument xTextDoc)
        throws com.sun.star.uno.Exception {

        TableDsc tDsc = new TableDsc();
        InstCreator instCreate = new InstCreator(xTextDoc, tDsc);

        XTextTable oTable = (XTextTable) instCreate.getInstance();
        return oTable;
    }

    /**
     * creates a TextTable with a specified count of rows and columns
     */
    public static XTextTable createTextTable(XTextDocument xTextDoc,
        int rows, int columns)
        throws com.sun.star.uno.Exception {

        TableDsc tDsc = new TableDsc(rows, columns);
        InstCreator instCreate = new InstCreator(xTextDoc, tDsc);

        XTextTable oTable = (XTextTable) instCreate.getInstance();
        return oTable;
    }

    /**
     * creates a simple TextFrame
     * ... to be continued
     */
    public static XTextFrame createTextFrame(XTextDocument xTextDoc)
        throws com.sun.star.uno.Exception {

        FrameDsc tDsc = new FrameDsc();
        InstCreator instCreate = new InstCreator(xTextDoc, tDsc);

        XTextFrame oFrame = (XTextFrame) instCreate.getInstance();
        return oFrame;
    }

    /**
     * creates a simple TextFrame
     * ... to be continued
     */
    public static XTextFrame createTextFrame(XTextDocument xTextDoc,
        int height, int width) {

        FrameDsc tDsc = new FrameDsc(height, width);
        InstCreator instCreate = new InstCreator(xTextDoc, tDsc);

        XTextFrame oFrame = (XTextFrame) instCreate.getInstance();
        return oFrame;
    }

    public static void insertString(XTextDocument xTextDoc, String cString)
        throws com.sun.star.uno.Exception {
        XText xText = xTextDoc.getText();
        XText oText = UnoRuntime.queryInterface(
            XText.class, xText);

        XTextCursor oCursor = oText.createTextCursor();
        oText.insertString(oCursor, cString, false);
    }

    public static void insertTextContent(XTextDocument xTextDoc,
        XTextContent xCont)
        throws com.sun.star.lang.IllegalArgumentException {
        XText xText = xTextDoc.getText();
        XText oText = UnoRuntime.queryInterface(
            XText.class, xText);

        XTextCursor oCursor = oText.createTextCursor();
        oText.insertTextContent(oCursor, xCont, false);
    }

    public static com.sun.star.table.XCell getFirstTableCell(
        XTextContent oTable) {

        String CellNames[] = ((XTextTable) oTable).getCellNames();

        com.sun.star.table.XCell oCell = ((XTextTable) oTable).getCellByName(
            CellNames[0]);
        return oCell;

    }

    /**
     * the method createBookmark
     */
    public static XTextContent createBookmark(XTextDocument xTextDoc)
        throws com.sun.star.uno.Exception {

        BookmarkDsc tDsc = new BookmarkDsc();
        InstCreator instCreate = new InstCreator(xTextDoc, tDsc);

        XTextContent oBookmark = (XTextContent) instCreate.getInstance();
        return oBookmark;

    } /// finish createBookmark

    /**
     * the method createReferenceMark
     */
    public static XTextContent createReferenceMark(XTextDocument xTextDoc)
        throws com.sun.star.uno.Exception {

        ReferenceMarkDsc tDsc = new ReferenceMarkDsc();
        InstCreator instCreate = new InstCreator(xTextDoc, tDsc);

        XTextContent oReferenceMark = (XTextContent) instCreate.getInstance();
        return oReferenceMark;

    } /// finish createReferenceMark

    /**
     * the method createFootnote
     */
    public static XTextContent createFootnote(XTextDocument xTextDoc)
        throws com.sun.star.uno.Exception {

        FootnoteDsc tDsc = new FootnoteDsc();
        InstCreator instCreate = new InstCreator(xTextDoc, tDsc);

        XTextContent oFootnote = (XTextContent) instCreate.getInstance();
        return oFootnote;

    } /// finish createFootnote

    /**
     * the method create Index
     */
    public static XTextContent createIndex(XTextDocument xTextDoc, String kind)
        throws com.sun.star.uno.Exception {

        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class,
            xTextDoc);

        Object oInt = oDocMSF.createInstance(kind);

        XTextContent xTC = UnoRuntime.queryInterface(XDocumentIndex.class, oInt);

        return xTC;

    }

    public static XSpreadsheet createSpreadsheet(XSpreadsheetDocument oDoc)
        throws com.sun.star.uno.Exception {

        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, oDoc);

        Object oInt = oDocMSF.createInstance(
            "com.sun.star.sheet.Spreadsheet");

        XSpreadsheet oSpreadsheet = UnoRuntime.queryInterface(XSpreadsheet.class, oInt);

        return oSpreadsheet;
    }

    public static XIndexAccess getTableCollection(XTextDocument oDoc) {

        XTextTablesSupplier oTTS = UnoRuntime.queryInterface(XTextTablesSupplier.class, oDoc);

        XNameAccess oNA = oTTS.getTextTables();
        XIndexAccess oIA = UnoRuntime.queryInterface(XIndexAccess.class, oNA);

        return oIA;
    }

    public static String getUniqueName(XInterface oInterface, String prefix) {
        XNameAccess oNameAccess = UnoRuntime.queryInterface(XNameAccess.class, oInterface);
        if (oNameAccess == null) {
            return null;
        }
        int i;
        for (i = 0; oNameAccess.hasByName(prefix + i); i++) {
        }
        return prefix + i;
    }

    public XShape createShape(XComponent oDoc, int height, int width, int x, int y, String kind) {
        //possible values for kind are 'Ellipse', 'Line' and 'Rectangle'

        ShapeDsc sDsc = new ShapeDsc(height, width, x, y, kind);
        InstCreator instCreate = new InstCreator(oDoc, sDsc);

        XShape oShape = (XShape) instCreate.getInstance();

        return oShape;

    }

    /**
     * creates a Diagram wich specified in kind(String)
     */
    public XDiagram createDiagram(XComponent oDoc, String kind) {
        XInterface oInterface = null;
        XDiagram oDiagram = null;

        //get LineDiagram
        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, oDoc);

        try {
            oInterface = (XInterface) oDocMSF.createInstance("com.sun.star.chart." + kind);
            oDiagram = UnoRuntime.queryInterface(XDiagram.class, oInterface);
        } catch (Exception e) {
            // Some exception occures.FAILED
            System.out.println("Couldn't create " + kind + "-Diagram " + e);
        }
        return oDiagram;
    }

    /*
    // create a Control-Instance which specified in kind(String)
     */
    public XInterface createControl(XComponent oDoc, String kind) {

        XInterface oControl = null;

        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, oDoc);

        try {
            oControl = (XInterface) oDocMSF.createInstance("com.sun.star.form.component." + kind);
        } catch (Exception e) {
            // Some exception occures.FAILED
            System.out.println("Couldn't create instance " + kind + ": " + e);
        }
        return oControl;
    }

    /*
    // create an Instance which is specified in kind(String)
     */
    public Object createInstance(XComponent oDoc, String kind) {

        Object oInstance = null;

        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, oDoc);

        try {
            oInstance = oDocMSF.createInstance(kind);
        } catch (Exception e) {
            // Some exception occures.FAILED
            System.out.println("Couldn't create instance " + kind + ": " + e);
        }
        return oInstance;
    }

    public XControlShape createControlShape(XComponent oDoc, int height, int width, int x, int y, String kind) {

        Size size = new Size();
        Point position = new Point();
        XControlShape oCShape = null;
        XControlModel aControl = null;

        //get MSF
        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, oDoc);

        try {
            Object oInt = oDocMSF.createInstance("com.sun.star.drawing.ControlShape");
            Object aCon = oDocMSF.createInstance("com.sun.star.form.component." + kind);
            aControl = UnoRuntime.queryInterface(XControlModel.class, aCon);
            oCShape = UnoRuntime.queryInterface(XControlShape.class, oInt);
            size.Height = height;
            size.Width = width;
            position.X = x;
            position.Y = y;
            oCShape.setSize(size);
            oCShape.setPosition(position);


        } catch (Exception e) {
            // Some exception occures.FAILED
            System.out.println("Couldn't create instance " + e);
        }

        try {
            oCShape.setControl(aControl);
        } catch (Exception e) {
            // Some exception occures.FAILED
            System.out.println("Couldn't get Control " + e);
        }


        return oCShape;

    }

    public XComponent loadDocument(String fileName)
        throws com.sun.star.lang.IllegalArgumentException,
        com.sun.star.io.IOException,
        com.sun.star.uno.Exception {

        // that noargs thing for load attributes
        PropertyValue[] szEmptyArgs = new PropertyValue[0];
        String frameName = "_blank";

        XComponent oDoc = oCLoader.loadComponentFromURL(
            fileName, frameName, 0, szEmptyArgs);

        if (oDoc == null) {
            return null;
        }
        DesktopTools.bringWindowToFront(oDoc);
        return oDoc;
    }

    public XComponent loadDocument(String fileName, PropertyValue[] Args)
        throws com.sun.star.lang.IllegalArgumentException,
        com.sun.star.io.IOException,
        com.sun.star.uno.Exception {

        // that noargs thing for load attributes
        String frameName = "_blank";

        XComponent oDoc = oCLoader.loadComponentFromURL(
            fileName, frameName, 0, Args);

        if (oDoc == null) {
            return null;
        }
        DesktopTools.bringWindowToFront(oDoc);

        return oDoc;
    }

    public XComponent openDoc(String kind, String frameName)
        throws com.sun.star.lang.IllegalArgumentException,
        com.sun.star.io.IOException,
        com.sun.star.uno.Exception {

        // that noargs thing for load attributes
        PropertyValue[] Args = null;
        if (kind.equals("simpress")) {
            Args = new PropertyValue[1];
            PropertyValue Arg = new PropertyValue();
            Arg.Name = "OpenFlags";
            Arg.Value = "S";
            Arg.Handle = -1;
            Arg.State = PropertyState.DEFAULT_VALUE;
            Args[0] = Arg;
        } else {
            Args = new PropertyValue[0];
        }

        if (frameName == null) {
            frameName = "_blank";
        }
        // load a blank a doc
        XComponent oDoc = oCLoader.loadComponentFromURL("private:factory/" + kind, frameName, 40, Args);
        DesktopTools.bringWindowToFront(oDoc);

        return oDoc;

    } // finished openDoc

    public XComponent openDoc(String kind, String frameName, PropertyValue[] mediaDescriptor)
        throws com.sun.star.lang.IllegalArgumentException,
        com.sun.star.io.IOException,
        com.sun.star.uno.Exception {

        if (frameName == null) {
            frameName = "_blank";
        }
        // load a blank a doc
        XComponent oDoc = oCLoader.loadComponentFromURL(
            "private:factory/" + kind, frameName, 40, mediaDescriptor);
        DesktopTools.bringWindowToFront(oDoc);

        return oDoc;

    } // finished openDoc

    // query for XServiceInfo
    public Object queryXServiceInfo(Object oObj) {
        if (oObj != null) {
            UnoRuntime.queryInterface(
                XServiceInfo.class, oObj);
            System.out.println("!!!! XServiceInfo n.a. !!!! ");
        } else {
            System.out.println("Object is empty!!!! ");
        }
        return null;
    } // finish queryXServiceInfo
}