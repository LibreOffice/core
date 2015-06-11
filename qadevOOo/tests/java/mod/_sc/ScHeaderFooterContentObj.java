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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XComponent;
import com.sun.star.sheet.XHeaderFooterContent;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XText;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Test for object which is represented by service
* <code>com.sun.star.sheet.HeaderFooterContent</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::sheet::XHeaderFooterContent</code></li>
* </ul>
* @see com.sun.star.sheet.HeaderFooterContent
* @see com.sun.star.sheet.XHeaderFooterContent
* @see ifc.sheet._XHeaderFooterContent
*/
public class ScHeaderFooterContentObj extends TestCase {
    private XSpreadsheetDocument xSpreadsheetDoc;

    /**
    * Creates Spreadsheet document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a Spreadsheet document" );
        xSpreadsheetDoc = SOF.createCalcDoc(null);
    }

    /**
    * Disposes Spreadsheet document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xSheetDoc " );
        XComponent oComp = UnoRuntime.queryInterface (XComponent.class, xSpreadsheetDoc);
        util.DesktopTools.closeDoc(oComp);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested.
    * Retrieves the collection of style families available in the document
    * using the interface <code>XStyleFamiliesSupplier</code>.
    * Obtains default style from the style family <code>'PageStyles'</code>.
    * Retrieves value of the property <code>'RightPageHeaderContent'</code>.
    * Sets some text for every part of header or footer using the interface
    * <code>XHeaderFooterContent</code> and sets new value of the property
    * <code>'RightPageHeaderContent'</code>. The value of this property is the
    * instance of the service <code>com.sun.star.sheet.HeaderFooterContent</code>.
    * @see com.sun.star.style.XStyleFamiliesSupplier
    * @see com.sun.star.sheet.XHeaderFooterContent
    * @see com.sun.star.sheet.HeaderFooterContent
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XPropertySet PropSet;
        XNameAccess PageStyles = null;
        XStyle StdStyle = null;

        XStyleFamiliesSupplier StyleFam = UnoRuntime.queryInterface(
            XStyleFamiliesSupplier.class,
            xSpreadsheetDoc );
        XNameAccess StyleFamNames = StyleFam.getStyleFamilies();
        PageStyles = (XNameAccess) AnyConverter.toObject(
            new Type(XNameAccess.class),StyleFamNames.getByName("PageStyles"));
        StdStyle = (XStyle) AnyConverter.toObject(
                    new Type(XStyle.class),PageStyles.getByName("Default"));

        //get the property-set
        PropSet = UnoRuntime.queryInterface(XPropertySet.class, StdStyle);

        XHeaderFooterContent RPHC = null;
        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );
        RPHC = (XHeaderFooterContent) AnyConverter.toObject(
            new Type(XHeaderFooterContent.class),
                PropSet.getPropertyValue("RightPageHeaderContent"));

        XText center = RPHC.getCenterText();
        XText left = RPHC.getLeftText();
        XText right = RPHC.getRightText();

        center.setString("CENTER");
        left.setString("LEFT");
        right.setString("RIGHT");

        PropSet.setPropertyValue("RightPageHeaderContent", RPHC);

        // create testobject here
        oObj = RPHC;
        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class ScHeaderFooterContentObj
