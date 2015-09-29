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
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XHeaderFooterContent;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.style.XStyle;
import com.sun.star.style.XStyleFamiliesSupplier;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by collection of
 * text fields contained in the text of a page header. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::util::XRefreshable</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.container.XEnumerationAccess
 * @see com.sun.star.util.XRefreshable
 * @see com.sun.star.container.XElementAccess
 * @see ifc.container._XEnumerationAccess
 * @see ifc.util._XRefreshable
 * @see ifc.container._XElementAccess
 */
public class ScHeaderFieldsObj extends TestCase {
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
     * Retrieves the interface <code>XHeaderFooterContent</code> from the style
     * using the property <code>'RightPageHeaderContent'</code>. Creates the
     * instance of the service <code>com.sun.star.text.TextField.Time</code> .
     * Obtains the text (the interface <code>XText</code>) which is printed in
     * the left part of the header or footer and inserts in it's content
     * the created field instance. Then the tested component is obtained
     * through <code>XTextFieldsSupplier</code> interface of a text.
     *
     * @see com.sun.star.style.XStyleFamiliesSupplier
     * @see com.sun.star.sheet.XHeaderFooterContent
     * @see com.sun.star.text.XText
     * @see com.sun.star.text.XTextContent
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XPropertySet PropSet;
        XNameAccess PageStyles = null;
        XStyle StdStyle = null;

        XStyleFamiliesSupplier StyleFam = UnoRuntime.queryInterface(XStyleFamiliesSupplier.class,
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

        XText left = RPHC.getLeftText();

        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            xSpreadsheetDoc );

        XTextContent the_Field = null;
        oObj = (XInterface)
            oDocMSF.createInstance( "com.sun.star.text.TextField.Time" );

        the_Field = UnoRuntime.queryInterface(XTextContent.class,oObj);

        XTextCursor the_Cursor = left.createTextCursor();

        left.insertTextContent(the_Cursor,the_Field, false);
        PropSet.setPropertyValue("RightPageHeaderContent", RPHC);

        XTextFieldsSupplier xTFSupp = UnoRuntime.queryInterface(XTextFieldsSupplier.class, left);

        oObj = xTFSupp.getTextFields();

        TestEnvironment tEnv = new TestEnvironment(oObj);

        return tEnv;
    } // finish method getTestEnvironment
}

