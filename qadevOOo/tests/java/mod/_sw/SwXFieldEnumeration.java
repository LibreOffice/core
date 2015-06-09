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

package mod._sw;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XDependentTextField;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XEnumeration</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 * @see com.sun.star.container.XEnumeration
 * @see ifc.container._XEnumeration
 */
public class SwXFieldEnumeration extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates text document.
    */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a textdocument" );
        xTextDoc = SOF.createTextDoc( null );
    }

    /**
    * Disposes text document.
    */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xTextDoc " );
        util.DesktopTools.closeDoc(xTextDoc);
    }

    /**
    * Creating a TestEnvironment for the interfaces to be tested. FieldMaster
    * object is created and attached to Bibliography database. After setting
    * properties of created FieldMaster object, it is attached to the TextField
    * object. Then TextField is inserted as TextContent to the text document.
    * At the end, TextFields of text document are obtained and
    * enumeration of these fields is created.<br>
    * Creates instances of the services:
    * <code>com.sun.star.text.FieldMaster.Database</code>,
    * <code>com.sun.star.text.TextField.Database</code>.<br>
    *     Object relations created :
    * <ul>
    *  <li> <code>'ENUM'</code> for
    *     {@link ifc.container._XEnumeration} : text fields</li>
    * </ul>
    */
    @Override
    public TestEnvironment createTestEnvironment(
            TestParameters tParam, PrintWriter log ) throws StatusException {
        XInterface oObj = null;
        Object FieldMaster = null;
        XPropertySet PFieldMaster = null;
        XDependentTextField xTF = null;
        XEnumerationAccess xFEA = null;
        XText the_Text;
        XTextCursor the_Cursor;
        XTextContent the_Field;

        log.println( "creating a test environment" );
        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface( XMultiServiceFactory.class, xTextDoc );

        try {
            FieldMaster = oDocMSF.createInstance
                ( "com.sun.star.text.FieldMaster.Database" );
            PFieldMaster = UnoRuntime.queryInterface
                (XPropertySet.class,FieldMaster);
            oObj = (XInterface)
                oDocMSF.createInstance("com.sun.star.text.TextField.Database");
            xTF = UnoRuntime.queryInterface(XDependentTextField.class,oObj);
        } catch ( com.sun.star.uno.Exception e ) {
            e.printStackTrace(log);
        }

        try {
            PFieldMaster.setPropertyValue("DataBaseName","Bibliography");
            PFieldMaster.setPropertyValue("DataTableName","biblio");
            PFieldMaster.setPropertyValue("DataColumnName","Address");
        } catch ( com.sun.star.lang.WrappedTargetException e ) {
            log.println("Error: can't set PropertyValue to a FieldMaster");
            e.printStackTrace(log);
        } catch ( com.sun.star.lang.IllegalArgumentException e ) {
            log.println("Error: can't set PropertyValue to a FieldMaster");
            e.printStackTrace(log);
        } catch ( com.sun.star.beans.UnknownPropertyException e ) {
            log.println("Error: can't set PropertyValue to a FieldMaster");
            e.printStackTrace(log);
        } catch ( com.sun.star.beans.PropertyVetoException e ) {
            log.println("Error: can't set PropertyValue to a FieldMaster");
            e.printStackTrace(log);
        }

        the_Text = xTextDoc.getText();
        the_Cursor = the_Text.createTextCursor();
        the_Field = UnoRuntime.queryInterface(XTextContent.class, oObj);

        xTF.attachTextFieldMaster(PFieldMaster);
        the_Text.insertTextContent(the_Cursor,the_Field,false);

        // create testobject here
        XTextFieldsSupplier oTFS = UnoRuntime.queryInterface( XTextFieldsSupplier.class, xTextDoc );
        xFEA = oTFS.getTextFields();
        oObj = oTFS.getTextFields().createEnumeration();

        log.println( "creating a new environment for FieldEnumeration object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        tEnv.addObjRelation("ENUM", xFEA);

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SwXFieldEnumeration

