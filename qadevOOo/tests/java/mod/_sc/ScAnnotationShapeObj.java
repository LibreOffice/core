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

import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSheetAnnotation;
import com.sun.star.sheet.XSheetAnnotationAnchor;
import com.sun.star.sheet.XSheetAnnotationShapeSupplier;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.CellAddress;
import com.sun.star.table.XCell;
import com.sun.star.table.XCellRange;
import com.sun.star.text.XSimpleText;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.SOfficeFactory;
import util.utils;

import java.io.PrintWriter;
import util.DefaultDsc;
import util.InstCreator;


/**
* Test for object which represents some text annotation
* anchored to some cell in spreadsheet (implement
* <code>com.sun.star.sheet.CellAnnotation</code>).<p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::text::XSimpleText</code></li>
*  <li> <code>com::sun::star::text::XTextRange</code></li>
*  <li> <code>com::sun::star::sheet::XSheetAnnotation</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.sheet.CellAnnotation
* @see com.sun.star.text.XSimpleText
* @see com.sun.star.text.XTextRange
* @see com.sun.star.sheet.XSheetAnnotation
* @see ifc.text._XSimpleText
* @see ifc.text._XTextRange
* @see ifc.sheet._XSheetAnnotation
*/
public class ScAnnotationShapeObj extends TestCase {
    private XSpreadsheetDocument xSheetDoc = null;

    /**
    * Creates a spreadsheet document.
    */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        SOfficeFactory SOF =
            SOfficeFactory.getFactory((XMultiServiceFactory) tParam
                .getMSF());

        try {
            log.println("creating a Spreadsheet document");
            log.println("Loading: "+utils.getFullTestURL(
                            "ScAnnotationShapeObj.sxc"));
            xSheetDoc =
                UnoRuntime.queryInterface(XSpreadsheetDocument.class,
                    SOF.loadDocument(utils.getFullTestURL(
                            "ScAnnotationShapeObj.sxc")));
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
    * Disposes a spreadsheet document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xSheetDoc ");

        XComponent oComp =
            UnoRuntime.queryInterface(XComponent.class,
            xSheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Retrieves a collection of spreadsheets from a document,
    * and takes one them. Then a single cell is retrieved, and
    * using its <code>com.sun.star.sheet.XSheetAnnotationAnchor</code>
    * interface an annotation is got.
    * Object relations created :
    * <ul>
    *  <li> <code>'CELLPOS'</code> for
    *      {@link ifc.sheet._XSheetAnnotation} (of <code>
    *       com.sun.star.table.CellAddress</code> type) which
    *      contains the annotation cell address.</li>
    * </ul>
    */
    public synchronized TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) throws StatusException {
        XInterface oObj = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("Creating a test environment");

        CellAddress cellPos = new CellAddress((short) 0, 1, 2);

        log.println("Getting test object ");

        XSpreadsheetDocument xArea =
            UnoRuntime.queryInterface(XSpreadsheetDocument.class,
            xSheetDoc);
        XSpreadsheets oSheets = xArea.getSheets();

        XIndexAccess XAccess =
            UnoRuntime.queryInterface(XIndexAccess.class,
            oSheets);
        XCell oCell = null;

        try {
            XSpreadsheet oSheet =
                (XSpreadsheet) AnyConverter.toObject(new Type(
                        XSpreadsheet.class),
                    XAccess.getByIndex(cellPos.Sheet));
            XCellRange oCRange =
                UnoRuntime.queryInterface(XCellRange.class,
                oSheet);
            oCell =
                oCRange.getCellByPosition(cellPos.Column, cellPos.Row);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Error getting test object from spreadsheet document",
                e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
            throw new StatusException("Error getting test object from spreadsheet document",
                e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Error getting test object from spreadsheet document",
                e);
        }

        XSheetAnnotationAnchor oAnnoA =
            UnoRuntime.queryInterface(XSheetAnnotationAnchor.class,
            oCell);
        XSheetAnnotation oAnno = oAnnoA.getAnnotation();

        XSimpleText xAnnoText =
            UnoRuntime.queryInterface(XSimpleText.class,
            oAnno);
        xAnnoText.setString("ScAnnotationShapeObj");

        XSheetAnnotationShapeSupplier xSheetAnnotationShapeSupplier =
            UnoRuntime.queryInterface(XSheetAnnotationShapeSupplier.class,
            oAnno);

        oObj = xSheetAnnotationShapeSupplier.getAnnotationShape();

        log.println("ImplementationName: "
            + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        //adding ObjRelation for RotationDescriptor
        tEnv.addObjRelation("NoShear", Boolean.TRUE);

        //adding ObjRelation for XText
        DefaultDsc tDsc = new DefaultDsc("com.sun.star.text.XTextContent",
                                    "com.sun.star.text.TextField.DateTime");

        log.println( "adding InstCreator object" );
        tEnv.addObjRelation(
            "XTEXTINFO", new InstCreator( xSheetDoc, tDsc ) );

        return tEnv;
    }
}
 // finish class ScAnnotationShapeObj
