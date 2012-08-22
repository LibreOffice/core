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
import static org.junit.Assert.assertTrue;

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
import com.sun.star.style.NumberingType;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextField;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.uno.UnoRuntime;


public class PageNumberField {

    private static final UnoApp app = new UnoApp();
    private static XTextDocument odtDocument = null;
    private static XTextDocument docDocument = null;
    private  static String odtSample = "testcase/uno/sw/field/PageNumberFieldTest.odt";
    private static String docSample = "testcase/uno/sw/field/PageNumberFieldTest.doc";

    private  static String odtSaveAsDocSample = "testcase/uno/sw/field/PageNumberFieldTest_1.doc";
    private static String docSaveAsODTSample = "testcase/uno/sw/field/PageNumberFieldTest_1.odt";

    @Before
    public void setUpDocument() throws Exception {

    }

    @After
    public void tearDownDocument() {


    }
    @BeforeClass
    public static void setUpConnection() throws Exception {
        app.start();
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
     * 4.Save it as doc format and reload
     * @throws Throwable
     */
    @Test
    @Ignore
    public void testPageNumberFieldODT() throws Throwable {
        odtDocument = SWUtil.openDocument(Testspace.prepareData(odtSample), app);
        createPageNumberFiled(odtDocument);
        int pageNumber = getPageNumber(odtDocument);
        assertEquals("Verify page number created in exist odt sample file.", 3, pageNumber);
        odtDocument = SWUtil.saveAndReload(odtDocument, app);
        assertTrue("Test page number field still exist after odt sample file saved", isContainPageNumberField(odtDocument));
        pageNumber = getPageNumber(odtDocument);
        assertEquals("Verify page number value still exist after saved.", 3, pageNumber);
        SWUtil.saveAsDoc(odtDocument, Testspace.getUrl(odtSaveAsDocSample));
        app.closeDocument(odtDocument);
        docDocument = SWUtil.openDocumentFromURL(Testspace.getUrl(odtSaveAsDocSample), app);

        assertTrue("Test page number field still exist after odt sample file save as doc format", isContainPageNumberField(docDocument));
        pageNumber = getPageNumber(docDocument);
        assertEquals("Verify page number value still exist after saved as doc format.", 3, pageNumber);
        app.closeDocument(docDocument);
    }

    /**
     * Bug 120625
     * Test Page Number Field Can created and Saved in Doc file
     * 1.launch a doc document
     * 2.Create a page number field at end of this page
     * 3.Save and Reopen this document, check page number field
     * 3.Save as odt format and reload
     * @throws Throwable
     */
    @Test
    public void testPageNumberFieldDOC() throws Throwable {
        docDocument = SWUtil.openDocument(Testspace.prepareData(docSample), app);
        createPageNumberFiled(docDocument);
        int pageNumber = getPageNumber(docDocument);
        assertEquals("Verify page number created in exist doc sample file.", 2, pageNumber);
        docDocument = SWUtil.saveAndReload(docDocument, app);
        assertTrue("Test page number field still exist after doc sample file saved", isContainPageNumberField(docDocument));
        pageNumber = getPageNumber(docDocument);
        assertEquals("Verify page number value still exist after saved.", 2, pageNumber);
        SWUtil.saveAsODT(docDocument, Testspace.getUrl(docSaveAsODTSample));
        app.closeDocument(docDocument);
        odtDocument = SWUtil.openDocumentFromURL(Testspace.getUrl(docSaveAsODTSample), app);

        assertTrue("Test page number field still exist after doc sample file save as odt format", isContainPageNumberField(odtDocument));
        pageNumber = getPageNumber(odtDocument);
        assertEquals("Verify page number value still exist after saved as doc format.", 2, pageNumber);
        app.closeDocument(odtDocument);
    }



    /**
     * Create a page number field at end of this document
     * @param document
     * @throws Exception
     */
    private void createPageNumberFiled(XTextDocument document) throws Exception {
        XMultiServiceFactory sevriceFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        XTextField  pageNumberFiled = (XTextField)UnoRuntime.queryInterface(XTextField.class, sevriceFactory.createInstance("com.sun.star.text.textfield.PageNumber"));

        XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, pageNumberFiled);
        props.setPropertyValue("NumberingType", NumberingType.ARABIC);//Set page number display as Arabic

        SWUtil.moveCuror2End(document);
        document.getText().insertTextContent(document.getText().getEnd(), pageNumberFiled, true);


    }
    /**
     * Get the page number by getText
     * This page number is at end of this document
     * @param document
     * @return
     */
    private int getPageNumber(XTextDocument document) {
        try {
            Thread.sleep(1000); //sleep before get page number field, there is a bug:120625
        } catch (InterruptedException e) {
            e.printStackTrace();
        }
        String documentString = document.getText().getString().trim();
        int length = documentString.length();
        String strNum = String.valueOf(documentString.charAt(length -1));
        int number = Integer.valueOf(strNum);
        return number;
    }


    /**
     * Check is contain page number field
     * @param document
     * @throws Exception
     */
    private boolean isContainPageNumberField(XTextDocument document) throws Exception {
        XTextFieldsSupplier fieldsSupplier = UnoRuntime.queryInterface(XTextFieldsSupplier.class, document);
        XEnumerationAccess xEnumeratedFields = fieldsSupplier.getTextFields();
        XEnumeration enumeration = xEnumeratedFields.createEnumeration();
        while (enumeration.hasMoreElements()) {
                Object field =  enumeration.nextElement();
                XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, field);
                short numberType = (Short) props.getPropertyValue("NumberingType");
                return numberType == 4;

        }
        return false;

    }
}
