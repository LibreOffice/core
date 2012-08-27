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

import static org.junit.Assert.assertTrue;

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
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextField;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.uno.UnoRuntime;
public class SubjectField {

    private static final UnoApp app = new UnoApp();
    private static XTextDocument odtDocument = null;
    private static XTextDocument docDocument = null;
    private  static String odtSample = "testcase/uno/sw/field/SubjectField.odt";
    private static String docSample = "testcase/uno/sw/field/Test_Sample.doc";

    private  static String odtSaveAsDocSample = "testcase/uno/sw/field/odtSaveAsDOC.doc";
    private static String docSaveAsODTSample = "testcase/uno/sw/field/docSaveAsODT.odt";

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
     * Test Subject Field Can created and Saved in odt file
     * 1.launch a odt document
     * 2.Create a page count field at end of this page
     * 3.Save and Reopen this document
     * 4.Save it as doc format and reload
     * @throws Throwable
     */
    @Test
    public void testSubjectFieldODT() throws Throwable {
        odtDocument = SWUtil.openDocument(Testspace.prepareData(odtSample), app);


        String subjectStr = "Test Subject Field";
        createSubjectFiled(odtDocument);

        assertTrue("Verify Subject field is created use exsit subject", isContainSubjectField(odtDocument, "UNO API Subject"));
        assertTrue("Verify Subject field is created use exsit subject, can get text from document",
                isContainSubjectText(odtDocument, "UNO API Subject"));
        //Set subject
        SWUtil.setDocumentProperty(odtDocument, "Subject", subjectStr);
        //Verfiy after set subject
        assertTrue("Verify Subject field is created, can get text from document",
                isContainSubjectText(odtDocument, subjectStr));

        odtDocument = SWUtil.saveAndReload(odtDocument, app);
        //verify after save and reload
        assertTrue("Verify Subject field is created, after save, still exist.", isContainSubjectField(odtDocument, subjectStr));
        assertTrue("Verify Subject field is created, can get text from document after saved.",
                isContainSubjectText(odtDocument, subjectStr));
        SWUtil.saveAsDoc(odtDocument, Testspace.getUrl(odtSaveAsDocSample));
        app.closeDocument(odtDocument);
        docDocument = SWUtil.openDocumentFromURL(Testspace.getUrl(odtSaveAsDocSample), app);
        assertTrue("Verify Subject field is created, after saved to doc format, field still exist.", isContainSubjectField(docDocument, subjectStr));
        assertTrue("Verify Subject field is created, after saved to doc format, can get text from document",
                isContainSubjectText(docDocument, subjectStr));
        app.closeDocument(docDocument);
    }

    /**
     *  Test Subject Field Can created and Saved in Doc file
     * 1.launch a doc document
     * 2.Create a Subject field at end of this page
     * 3.Save and Reopen this document, check Subject field
     * 3.Save as odt format and reload
     * @throws Throwable
     */
    @Test
    public void testPageCountFieldDOC() throws Throwable {
        docDocument = SWUtil.openDocument(Testspace.prepareData(docSample), app);
        String subjectStr = "Test Subject Field";
        SWUtil.setDocumentProperty(docDocument, "Subject", subjectStr);
        createSubjectFiled(docDocument);

        assertTrue("Verify Subject field is created.", isContainSubjectField(docDocument, subjectStr));
        assertTrue("Verify Subject field is created, can get text from document",
                isContainSubjectText(docDocument, subjectStr));

        docDocument = SWUtil.saveAndReload(docDocument, app);
        //verify after save and reload
        assertTrue("Verify Subject field is created, after save, still exist.", isContainSubjectField(docDocument, subjectStr));
        assertTrue("Verify Subject field is created, can get text from document after saved.",
                isContainSubjectText(docDocument, subjectStr));
        SWUtil.saveAsODT(docDocument, Testspace.getUrl(docSaveAsODTSample));
        app.closeDocument(docDocument);
        odtDocument = SWUtil.openDocumentFromURL(Testspace.getUrl(docSaveAsODTSample), app);
        assertTrue("Verify Subject field is created, after saved to doc format, field still exist.", isContainSubjectField(odtDocument, subjectStr));
        assertTrue("Verify Subject field is created, after saved to doc format, can get text from document",
                isContainSubjectText(odtDocument, subjectStr));
        app.closeDocument(odtDocument);
    }



    /**
     * Create a subject field at start of this document
     * @param document
     * @throws Exception
     */
    private void createSubjectFiled(XTextDocument document) throws Exception {

        XMultiServiceFactory sevriceFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        XTextField  subjectField = (XTextField)UnoRuntime.queryInterface(XTextField.class, sevriceFactory.createInstance("com.sun.star.text.textfield.docinfo.Subject"));


        SWUtil.moveCuror2Start(document);
        document.getText().insertTextContent(document.getText().getStart(), subjectField, true);

    }


    /**
     * Check is contain subject content at start of this document
     *
     * @param document
     * @param content
     * @return
     */
    private boolean isContainSubjectText(XTextDocument document, String content) {
        String documentString = document.getText().getString().trim();
        return documentString.indexOf(content) == 0;
    }

    /**
     * Check is contain subject field
     * @param document
     * @throws Exception
     */
    private boolean isContainSubjectField(XTextDocument document, String content) throws Exception {
        XTextFieldsSupplier fieldsSupplier = UnoRuntime.queryInterface(XTextFieldsSupplier.class, document);
        XEnumerationAccess xEnumeratedFields = fieldsSupplier.getTextFields();
        XEnumeration enumeration = xEnumeratedFields.createEnumeration();
        while (enumeration.hasMoreElements()) {
                Object field =  enumeration.nextElement();
                XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, field);
                String strContent = (String) props.getPropertyValue("Content");
                return content.equals(strContent);

        }
        return false;

    }

}
