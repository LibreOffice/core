/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package testcase.uno.sw.crossreference;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XNamed;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.ReferenceFieldPart;
import com.sun.star.text.ReferenceFieldSource;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextField;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XRefreshable;

/**
 *
 */
public class CheckCrossReferences {

    private XEnumeration xParaEnum;
    private XEnumeration xPortionEnum;
    private com.sun.star.util.XRefreshable xFldsRefresh;
    private final static UnoApp app = new UnoApp();

    private XTextDocument document = null;

    @Before
    public void setUpDocument() throws Exception {
        app.start();
        document = UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.prepareData("testcase/uno/sw/CheckCrossReferences.odt")));
    }

    @After
    public void tearDownDocument() {
        app.closeDocument(document);
    }

    @AfterClass
    public static void tearDownConnection() throws InterruptedException, Exception {
        app.close();
    }

    public XTextField getNextField() throws Exception {
        if (xPortionEnum != null) {
            while (xPortionEnum.hasMoreElements()) {
                XPropertySet xPortionProps = UnoRuntime.queryInterface(XPropertySet.class, xPortionEnum.nextElement());
                final String sPortionType = xPortionProps.getPropertyValue("TextPortionType").toString();
                if (sPortionType.equals("TextField"))
                    return UnoRuntime.queryInterface(XTextField.class, xPortionProps.getPropertyValue("TextField"));
            }
        }

        while (xParaEnum.hasMoreElements()) {
            XEnumerationAccess aPara = UnoRuntime.queryInterface(XEnumerationAccess.class, xParaEnum.nextElement());
            xPortionEnum = aPara.createEnumeration();
            while (xPortionEnum.hasMoreElements()) {
                XPropertySet xPortionProps = UnoRuntime.queryInterface(XPropertySet.class, xPortionEnum.nextElement());
                final String sPortionType = xPortionProps.getPropertyValue("TextPortionType").toString();
                if (sPortionType.equals("TextField"))
                    return UnoRuntime.queryInterface(XTextField.class, xPortionProps.getPropertyValue("TextField"));
            }
        }

        return null;
    }

    public XPropertySet getFieldProps(XTextField xField) {
        XPropertySet xProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xField);
        return xProps;
    }

    public void checkField(XTextField xField, XPropertySet xProps, short nFormat, String aExpectedFldResult) throws Exception {
        // set requested format
        xProps.setPropertyValue("ReferenceFieldPart", new Short(nFormat));
        // refresh fields in order to get new format applied
        xFldsRefresh.refresh();
        String aFldResult = xField.getPresentation(false);
        assertEquals("set reference field format doesn't result in correct field result", aExpectedFldResult, aFldResult);
    }

    @Test
    public void checkCrossReferences() throws Exception {
        // setup paragraph enumeration
        xParaEnum = UnoRuntime.queryInterface(XEnumerationAccess.class, document.getText()).createEnumeration();

        // get field refresher
        XTextFieldsSupplier xFieldSupp = UnoRuntime.queryInterface(XTextFieldsSupplier.class, document);
        xFldsRefresh = UnoRuntime.queryInterface(XRefreshable.class, xFieldSupp.getTextFields());

        // strings for checking
        final String FldResult1 = "*i*";
        final String FldResult2 = "+b+*i*";
        final String FldResult3 = "-1-+b+*i*";
        final String FldResult4 = "1.";
        final String FldResult5 = " 1.";
        final String FldResult6 = "A. 1.";

        // variables for current field
        XTextField xField = null;
        XPropertySet xProps = null;

        xField = getNextField();
        xProps = getFieldProps(xField);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER, FldResult2);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult1);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult3);

        xField = getNextField();
        xProps = getFieldProps(xField);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER, FldResult1);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult1);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult3);

        xField = getNextField();
        xProps = getFieldProps(xField);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER, FldResult3);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult1);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult3);

        xField = getNextField();
        xProps = getFieldProps(xField);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER, FldResult5);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult4);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult6);

        xField = getNextField();
        xProps = getFieldProps(xField);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER, FldResult4);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult4);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult6);

        xField = getNextField();
        xProps = getFieldProps(xField);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER, FldResult6);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_NO_CONTEXT, FldResult4);
        checkField(xField, xProps, ReferenceFieldPart.NUMBER_FULL_CONTEXT, FldResult6);

        // insert a certain cross-reference bookmark and a reference field to this bookmark

        // restart paragraph enumeration
        xParaEnum = UnoRuntime.queryInterface(XEnumerationAccess.class, document.getText()).createEnumeration();

        // iterate on the paragraphs to find certain paragraph to insert the bookmark
        XTextRange xParaTextRange = null;
        while (xParaEnum.hasMoreElements()) {
            xParaTextRange = (XTextRange) UnoRuntime.queryInterface(XTextRange.class, xParaEnum.nextElement());
            if (xParaTextRange.getString().equals("J")) {
                break;
            } else {
                xParaTextRange = null;
            }
        }
        assertNotNull("Cannot find paragraph to insert cross-reference bookmark.", xParaTextRange);

        // insert bookmark
        XMultiServiceFactory xFac = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        final String cBookmarkName = "__RefNumPara__47114711";
        XTextContent xBookmark = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xFac.createInstance("com.sun.star.text.Bookmark"));

        XNamed xName = (XNamed) UnoRuntime.queryInterface(XNamed.class, xBookmark);
        xName.setName(cBookmarkName);
        xBookmark.attach(xParaTextRange.getStart());

        // insert reference field, which references the inserted bookmark
        XTextContent xNewField = (XTextContent) UnoRuntime.queryInterface(XTextContent.class, xFac.createInstance("com.sun.star.text.textfield.GetReference"));

        XPropertySet xFieldProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xNewField);
        xFieldProps.setPropertyValue("ReferenceFieldPart", new Short(ReferenceFieldPart.TEXT));
        xFieldProps.setPropertyValue("ReferenceFieldSource", new Short(ReferenceFieldSource.BOOKMARK));
        xFieldProps.setPropertyValue("SourceName", cBookmarkName);
        XTextRange xFieldTextRange = (XTextRange) UnoRuntime.queryInterface(XTextRange.class, xParaEnum.nextElement());
        xNewField.attach(xFieldTextRange.getEnd());
        xFldsRefresh.refresh();

        // check inserted reference field
        xField = UnoRuntime.queryInterface(XTextField.class, xNewField);
        assertEquals("inserted reference field doesn't has correct field result", "J", xField.getPresentation(false));
        xParaTextRange.getStart().setString("Hallo new bookmark: ");
        xFldsRefresh.refresh();
        assertEquals("inserted reference field doesn't has correct field result", "Hallo new bookmark: J", xField.getPresentation(false));

    }

}
