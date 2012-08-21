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

package ifc.text;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sheet.XSpreadsheet;
import com.sun.star.sheet.XSpreadsheetDocument;
import com.sun.star.sheet.XSpreadsheets;
import com.sun.star.table.XCell;
import com.sun.star.text.XDependentTextField;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import lib.MultiMethodTest;
import lib.StatusException;

/**
 *
 */
public class _XTextFieldsSupplier extends MultiMethodTest {

    public XTextFieldsSupplier oObj = null;                // oObj filled by MultiMethodTest
    private boolean mDispose = false;
    private boolean mbCreateFieldMaster = true;

    /**
     * Insert some text fields into a cell on the sheet, so this interface test
     * makes sense.
     */
    protected void before() {
        Object o = tEnv.getObjRelation("XTextFieldsSupplier.MAKEENTRY");
        if (o != null && ((Boolean)o).booleanValue()) {
            mDispose = true;
            mbCreateFieldMaster = false;
            XCell xCell = (XCell)tEnv.getObjRelation("MAKEENTRYINCELL");

            XSpreadsheetDocument xSheetDoc = (XSpreadsheetDocument)tEnv.getObjRelation("SPREADSHEET");

            XText oText = null;
            XTextContent oContent = null;
            XInterface aField = null;

            try {
                // we want to create an instance of ScCellFieldObj.
                // to do this we must get an MultiServiceFactory.

                XMultiServiceFactory _oMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xSheetDoc);

                aField = (XInterface)
                    _oMSF.createInstance("com.sun.star.text.TextField.URL");
                oContent = UnoRuntime.queryInterface(XTextContent.class, aField);

                XSpreadsheets oSheets = xSheetDoc.getSheets() ;
                XIndexAccess oIndexSheets = UnoRuntime.queryInterface(XIndexAccess.class, oSheets);
                AnyConverter.toObject(
                        new Type(XSpreadsheet.class),oIndexSheets.getByIndex(0));

                String[] services = _oMSF.getAvailableServiceNames();
                for (int i=0; i<services.length; i++) {
                    if (services[i].startsWith("com.sun.star.text.FieldMaster")) {
                        mbCreateFieldMaster = true;
                        log.println("service " + i + ": " + services[i]);
                    }
                }

                if (mbCreateFieldMaster) {
                    Object FieldMaster = _oMSF.createInstance("com.sun.star.text.FieldMaster.User");
                    XPropertySet PFieldMaster = UnoRuntime.queryInterface
                        (XPropertySet.class,FieldMaster);

                    XDependentTextField xTF = UnoRuntime.queryInterface(XDependentTextField.class,aField);

                    PFieldMaster.setPropertyValue("Content","Some content");

                    xTF.attachTextFieldMaster(PFieldMaster);
                }

                oText = UnoRuntime.queryInterface(XText.class, xCell);
                XTextCursor the_Cursor = oText.createTextCursor();

                oText.insertTextContent(
                    the_Cursor, oContent, true);

            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Exception occurred while creating test Object.");
                e.printStackTrace(log);
                throw new StatusException("Couldn't insert textField.URL", e);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Exception occurred while creating test Object.");
                e.printStackTrace(log);
                throw new StatusException("Couldn't insert textField.URL", e);
            } catch (com.sun.star.lang.IllegalArgumentException e) {
                log.println("Exception occurred while creating test Object.");
                e.printStackTrace(log);
                throw new StatusException("Couldn't insert textField.URL", e);
            } catch (com.sun.star.uno.Exception e) {
                log.println("Exception occurred while creating test Object.");
                e.printStackTrace(log);
                throw new StatusException("Couldn't insert textField.URL", e);
            }

        }
    }

    public void _getTextFields() {
        XEnumerationAccess xEnumAccess = oObj.getTextFields();
        XEnumeration xEnum = xEnumAccess.createEnumeration();
        while(xEnum != null && xEnum.hasMoreElements()) {
            try {
                xEnum.nextElement();
            }
            catch(com.sun.star.container.NoSuchElementException e) {
                setMethodFalse("getTextFields()", e);
            }
            catch(com.sun.star.lang.WrappedTargetException e) {
                setMethodFalse("getTextFields()", e);
            }
        }
        tRes.tested("getTextFields()", xEnum != null);
    }

    public void _getTextFieldMasters() {
        if (mbCreateFieldMaster) {
            XNameAccess xName = oObj.getTextFieldMasters();
            util.dbg.printInterfaces(xName);
            tRes.tested("getTextFieldMasters()", xName != null);
        }
        else {
            log.println("Could not test 'getTextFieldMasters' because no field masters can be created on this object.");
            tRes.tested("getTextFieldMasters()", true);
        }
    }

    /**
     * Just for convenience: log the exception and set the method false.
     * @param method The name of the method to set to false.
     * @param e The Exception that occurred.
     */
    private void setMethodFalse(String method, Exception e) {
        log.println("Exception while executing '" + method + "'");
        e.printStackTrace(log);
        tRes.tested(method, false);
    }

    protected void after() {
        if (mDispose)
            disposeEnvironment();
    }

}
