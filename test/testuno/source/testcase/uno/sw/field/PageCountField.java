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

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
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

public class PageCountField {

    private static final UnoApp app = new UnoApp();
    private static XTextDocument odtDocument = null;
    private static XTextDocument docDocument = null;
    private  static String odtSample = "testcase/uno/sw/field/PageCountField.odt";
    private static String docSample = "testcase/uno/sw/field/PageCountField.doc";

    private  static String odtSaveAsDocSample = "testcase/uno/sw/field/PageCountFieldNewSave.doc";
    private static String docSaveAsODTSample = "testcase/uno/sw/field/PageCountFieldNewSave.odt";

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
     *
     * Test Page count Field Can created and Saved in odt file
     * 1.launch a odt document
     * 2.Create a page count field at end of this page
     * 3.Save and Reopen this document
     * 4.Save it as doc format and reload
     * @throws Throwable
     */
    @Test
    public void testPageCountFieldODT() throws Throwable {
        odtDocument = SWUtil.openDocument(Testspace.prepareData(odtSample), app);
        createPageCountField(odtDocument);
        int PageCount = getPageCount(odtDocument);
        assertEquals("Verify page count created in exist odt sample file.", 3, PageCount);
        odtDocument = SWUtil.saveAndReload(odtDocument, app);
        assertTrue("Test page count field still exist after odt sample file saved", isContainPageCountField(odtDocument));
        PageCount = getPageCount(odtDocument);
        assertEquals("Verify page count value still exist after saved.", 3, PageCount);
        SWUtil.saveAsDoc(odtDocument, Testspace.getUrl(odtSaveAsDocSample));
        app.closeDocument(odtDocument);
        docDocument = SWUtil.openDocumentFromURL(Testspace.getUrl(odtSaveAsDocSample), app);

        assertTrue("Test page count field still exist after odt sample file save as doc format", isContainPageCountField(docDocument));
        PageCount = getPageCount(docDocument);
        assertEquals("Verify page count value still exist after saved as doc format.", 3, PageCount);
        app.closeDocument(docDocument);
    }

    /**
     *  Test Page count Field Can created and Saved in Doc file
     * 1.launch a doc document
     * 2.Create a page count field at end of this page
     * 3.Save and Reopen this document, check page count field
     * 3.Save as odt format and reload
     * @throws Throwable
     */
    @Test
    public void testPageCountFieldDOC() throws Throwable {
        docDocument = SWUtil.openDocument(Testspace.prepareData(docSample), app);
        createPageCountField(docDocument);
        int PageCount = getPageCount(docDocument);
        assertEquals("Verify page count created in exist doc sample file.", 4, PageCount);
        docDocument = SWUtil.saveAndReload(docDocument, app);
        assertTrue("Test page count field still exist after doc sample file saved", isContainPageCountField(docDocument));
        PageCount = getPageCount(docDocument);
        assertEquals("Verify page count value still exist after saved.", 4, PageCount);
        SWUtil.saveAsODT(docDocument, Testspace.getUrl(docSaveAsODTSample));
        app.closeDocument(docDocument);
        odtDocument = SWUtil.openDocumentFromURL(Testspace.getUrl(docSaveAsODTSample), app);

        assertTrue("Test page count field still exist after doc sample file save as odt format", isContainPageCountField(odtDocument));
        PageCount = getPageCount(odtDocument);
        assertEquals("Verify page count value still exist after saved as doc format.", 4, PageCount);
        app.closeDocument(odtDocument);
    }



    /**
     * Create a page count field at start of this document
     * @param document
     * @throws Exception
     */
    private void createPageCountField(XTextDocument document) throws Exception {
        XMultiServiceFactory sevriceFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        XTextField  PageCountField = (XTextField)UnoRuntime.queryInterface(XTextField.class, sevriceFactory.createInstance("com.sun.star.text.textfield.PageCount"));

        XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, PageCountField);
        props.setPropertyValue("NumberingType", NumberingType.ARABIC);//Set page count display as Arabic

        SWUtil.moveCuror2Start(document);
        document.getText().insertTextContent(document.getText().getStart(), PageCountField, true);


    }
    /**
     * Get the page count by getText
     * This page count is at end of this document
     * @param document
     * @return
     */
    private int getPageCount(XTextDocument document) {
        String documentString = document.getText().getString().trim();
        String strNum = String.valueOf(documentString.charAt(0));
        int count = Integer.valueOf(strNum);
        return count;
    }


    /**
     * Check is contain page count field
     * @param document
     * @throws Exception
     */
    private boolean isContainPageCountField(XTextDocument document) throws Exception {
        XTextFieldsSupplier fieldsSupplier = UnoRuntime.queryInterface(XTextFieldsSupplier.class, document);
        XEnumerationAccess xEnumeratedFields = fieldsSupplier.getTextFields();

        XEnumeration enumeration = xEnumeratedFields.createEnumeration();
        while (enumeration.hasMoreElements()) {
                Object field =  enumeration.nextElement();

                XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, field);
                short countType = (Short) props.getPropertyValue("NumberingType");
                return countType == NumberingType.ARABIC;

        }
        return false;

    }

}
