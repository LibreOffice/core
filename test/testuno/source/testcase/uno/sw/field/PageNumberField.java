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
package testcase.uno.sw.field;
import static org.junit.Assert.assertEquals;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import testlib.uno.SWUtil;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextField;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.uno.UnoRuntime;


public class PageNumberField {

    private static final UnoApp app = new UnoApp();
    private static XTextDocument odtDocument = null;
    private static XTextDocument docDocument = null;
    private  static String odtSample = "testcase/uno/sw/field/PageNumberFieldTest.odt";
    private static String odtSampleWorking = "";
    private static String docSample = "testcase/uno/sw/field/PageNumberFieldTest.doc";
    private static String docSampleWorking = "";

    private  static String odtSaveAsDocSample = "testcase/uno/sw/field/PageNumberFieldTest_1.doc";
    private static String docSaveAsODTSample = "testcase/uno/sw/field/PageNumberFieldTest_1.odt";

    @Before
    public void setUpDocument() throws Exception {
        app.start();
    }

    @After
    public void tearDownDocument() {


    }

    @BeforeClass
    public static void setUpConnection() throws Exception {

    }


    @AfterClass
    public static void tearDownConnection() throws InterruptedException,
            Exception {
        app.close();
    }

    /**
     * There is a bug : Bug 120625
     * Test Page Number Field Can created and Saved in odt file
     * 1.launch a odt document
     * 2.Create a page number field at end of this page
     * 3.Save and Reopen this document
     * @throws Throwable
     */
    @Test
    @Ignore("Bug 120625")
    public void testPageNumberFieldODT() throws Throwable {
        odtSampleWorking = Testspace.prepareData(odtSample);
        odtDocument = SWUtil.openDocument(odtSampleWorking, app);
        createPageNumberFiled(odtDocument, 3, "odt");
        app.closeDocument(odtDocument);
    }

    /**
     * Bug 120625
     * Test Page Number Field Can created and Saved in Doc file
     * 1.launch a doc document
     * 2.Create a page number field at end of this page
     * 3.Save and Reopen this document, check page number field
     * @throws Throwable
     */
    @Test
    @Ignore("Bug 120625")
    public void testPageNumberFieldDOC() throws Throwable {
        docSampleWorking = Testspace.prepareData(docSample);
        docDocument = SWUtil.openDocument(docSampleWorking, app);
        createPageNumberFiled(docDocument, 2, "odt");
        app.closeDocument(docDocument);
    }

    /**
     * Test Page Number Field in odt file save to doc format works well
     * 1.Launch the new saved file
     * 2.Check page number filed.
     * @throws Throwable
     */
    @Test
    public void testSavedDoc2ODTPageNumberField() throws Throwable {
        odtDocument = SWUtil.openDocumentFromURL(Testspace.getUrl(docSaveAsODTSample), app);
        getPageNumberField(odtDocument, 2);
        app.closeDocument(odtDocument);
    }

    /**
     * Test Page Number Field in doc file save to odt format works well
     * 1.Launch the new saved file
     * 2.Check page number filed.
     * @throws Throwable
     */
    @Test
    public void testSavedODT2DOCPageNumberField() throws Throwable {
        docDocument = SWUtil.openDocumentFromURL(Testspace.getUrl(odtSaveAsDocSample), app);
        getPageNumberField(docDocument, 2);
        app.closeDocument(docDocument);
    }

    private void createPageNumberFiled(XTextDocument document, int expectNumber, String saveAsFormat) throws Exception {
        XMultiServiceFactory sevriceFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        XTextField  pageNumberFiled = (XTextField)UnoRuntime.queryInterface(XTextField.class, sevriceFactory.createInstance("com.sun.star.text.textfield.PageNumber"));

        XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, pageNumberFiled);
        props.setPropertyValue("NumberingType", 4);//Set page number display as Arabic


        SWUtil.moveCuror2End(document);
        document.getText().insertTextContent(document.getText().getEnd(), pageNumberFiled, true);

        String documentString = document.getText().getString().trim();
        int length = documentString.length();
        String strNum = String.valueOf(documentString.charAt(length -1));
        int number = Integer.valueOf(strNum);
        assertEquals("Test Page Number field can insert correctly", expectNumber, number);
        SWUtil.save(document);
        if("odt".equals(saveAsFormat)) {
            SWUtil.saveAsDoc(document, Testspace.getUrl(odtSaveAsDocSample));


        } else if ("doc".equals(saveAsFormat)) {
            SWUtil.saveAsDoc(document, Testspace.getUrl(docSaveAsODTSample));

        }
        app.closeDocument(document);

        //Verify after save.
        document = SWUtil.openDocument(odtSampleWorking, app);

        getPageNumberField(document, expectNumber);


    }

    private void getPageNumberField(XTextDocument document, int expectNumber) throws Exception {
        XTextFieldsSupplier fieldsSupplier = UnoRuntime.queryInterface(XTextFieldsSupplier.class, document);
        XEnumerationAccess xEnumeratedFields = fieldsSupplier.getTextFields();
        XEnumeration enumeration = xEnumeratedFields.createEnumeration();
        while (enumeration.hasMoreElements()) {
                Object field =  enumeration.nextElement();
                XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, field);
                short numberType = (Short) props.getPropertyValue("NumberingType");
                assertEquals("Verify page number field type is Arabic", 4, numberType);

        }

        String documentString = document.getText().getString().trim();
        int length = documentString.length();
        String strNum = String.valueOf(documentString.charAt(length -1));
        int number = Integer.valueOf(strNum);
        assertEquals("Test Page Number field can insert and saved correctly", expectNumber, number);


    }
}
