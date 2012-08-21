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

import java.util.Calendar;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XEnumeration;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextField;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.DateTime;
/**
 * Test Date and Time Field can be created and Load
 * @author test
 *
 */
public class CheckDateTimeField {



    private String tempPath = "testcase/uno/sw/temp/" ;
    private String tempFileName =  String.valueOf(System.currentTimeMillis());
    private static final UnoApp app = new UnoApp();

    private XTextDocument document = null;
    @Before
    public void setUpDocument() throws Exception {
        document = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));
    }

    @After
    public void tearDownDocument() {
         app.closeDocument(document);
         FileUtil.deleteFile(Testspace.getFile(tempPath));
    }



    @BeforeClass
    public static void setUpConnection() throws Exception {
        app.start();
    }

    @AfterClass
    public static void tearDownConnection() throws InterruptedException, Exception {
        app.close();
    }



    /**
     * Test Time field can be created correctly.
     * 1.Create a Time in new document
     * 2.Verify the Time is created by check the date hour in the new document
     * 3.Save and close the new document to doc format
     * 4.Reload the new save doc file, check the  time field
     * @throws Exception
     */

    @Test
    public void testCreateTimeFieldSaveDoc() throws Exception {

        String url = Testspace.getUrl(tempPath + tempFileName + ".doc");
        PropertyValue[] propsValue = new PropertyValue[1];
        propsValue[0] = new PropertyValue();
        propsValue[0].Name = "FilterName";
        propsValue[0].Value = "MS Word 97";

        createTimeFiled(document, url, propsValue);
    }
    /**
     * Test Time Field can be created correctly.
     * 1.Create a Time Field in new document
     * 2.Verify the Time Field is created by check the date hour in the new document
     * 3.Save and close the new document to doc format
     * 4.Reload the new save odt file, check the Time Field
     * @throws Exception
     */
    @Test
    public void testCreateTimeFieldSaveODT() throws Exception {

        String url = Testspace.getUrl(tempPath + tempFileName + ".odt");
        PropertyValue[] propsValue = new PropertyValue[0];
        createTimeFiled(document, url, propsValue);

    }

    private void createTimeFiled(XTextDocument document, String url, PropertyValue[] propsValue) throws Exception {
        XMultiServiceFactory sevriceFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        XTextField  dateFiled = (XTextField)UnoRuntime.queryInterface(XTextField.class, sevriceFactory.createInstance("com.sun.star.text.textfield.DateTime"));


        XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, dateFiled);
        props.setPropertyValue("IsDate", false);

        document.getText().insertTextContent(document.getText().getEnd(), dateFiled, false);
        DateTime dateField = (DateTime) props.getPropertyValue("DateTimeValue");

        String dateString = document.getText().getString();
        assertTrue("Verify time field is creatd, by verify it's hour",  dateString.indexOf(String.valueOf(dateField.Hours).trim()) != -1);
        assertTrue("Verify time field is creatd, by verify it's minutes",  dateString.indexOf(String.valueOf(dateField.Minutes).trim()) != -1);
        int expectHour = Calendar.getInstance().get(Calendar.HOUR_OF_DAY);
        assertEquals("Verify time field is creatd, value is right, by compare Hour", expectHour, dateField.Hours);
        XStorable store = UnoRuntime.queryInterface(XStorable.class, document);
        store.storeAsURL(url, propsValue);
        app.closeDocument(document);
        document = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.loadDocumentFromURL(url));
        XTextFieldsSupplier fieldsSupplier = UnoRuntime.queryInterface(XTextFieldsSupplier.class, document);
        XEnumerationAccess xEnumeratedFields = fieldsSupplier.getTextFields();
        XEnumeration enumeration = xEnumeratedFields.createEnumeration();
        while (enumeration.hasMoreElements()) {
              Object field =  enumeration.nextElement();
                XPropertySet props2 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, field);
                DateTime dateField2 = (DateTime) props2.getPropertyValue("DateTimeValue");
                assertEquals("Verify time field is creatd correct by save and reload.", expectHour, dateField2.Hours);
        }


    }
    /**
     * Test Date Field can be created correctly.
     * 1.Create a Date in new document
     * 2.Verify the Date is created by check the date hour in the new document
     * 3.Save and close the new document to doc format
     * 4.Reload the new save doc file, check the  Date field
     * @throws Exception
     */

    @Test
    public void testCreateDateFieldSaveDoc() throws Exception {

        String url = Testspace.getUrl(tempPath + tempFileName + ".doc");
        PropertyValue[] propsValue = new PropertyValue[1];
        propsValue[0] = new PropertyValue();
        propsValue[0].Name = "FilterName";
        propsValue[0].Value = "MS Word 97";

        createDateFiled(document, url, propsValue);
    }
    /**
     * Test Date Field can be created correctly.
     * 1.Create a Date field in new document
     * 2.Verify the dateField is created by check the date hour in the new document
     * 3.Save and close the new document to doc format
     * 4.Reload the new save odt file, check the date  field
     * @throws Exception
     */
    @Test
    public void testCreateDateFieldSaveODT() throws Exception {

        String url = Testspace.getUrl(tempPath + tempFileName + ".odt");
        PropertyValue[] propsValue = new PropertyValue[0];
        createDateFiled(document, url, propsValue);

    }
    private void createDateFiled(XTextDocument document, String url, PropertyValue[] propsValue) throws Exception {
        XMultiServiceFactory sevriceFactory = (XMultiServiceFactory) UnoRuntime.queryInterface(XMultiServiceFactory.class, document);
        XTextField  dateFiled = (XTextField)UnoRuntime.queryInterface(XTextField.class, sevriceFactory.createInstance("com.sun.star.text.textfield.DateTime"));


        XPropertySet props = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, dateFiled);
        props.setPropertyValue("IsDate", true);

        document.getText().insertTextContent(document.getText().getEnd(), dateFiled, false);
        DateTime dateField = (DateTime) props.getPropertyValue("DateTimeValue");

        String dateString = document.getText().getString();
        assertTrue("Verify date field is creatd, by verify it's Month",  dateString.indexOf(String.valueOf(dateField.Month).trim()) != -1);
        assertTrue("Verify date field is creatd, by verify it's Day",  dateString.indexOf(String.valueOf(dateField.Day).trim()) != -1);
        int expectDay = Calendar.getInstance().get(Calendar.DAY_OF_MONTH);
        assertEquals("Verify date field is creatd, value is right, by compare Day", expectDay, dateField.Day);
        XStorable store = UnoRuntime.queryInterface(XStorable.class, document);
        store.storeAsURL(url, propsValue);
        app.closeDocument(document);
        document = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.loadDocumentFromURL(url));
        XTextFieldsSupplier fieldsSupplier = UnoRuntime.queryInterface(XTextFieldsSupplier.class, document);
        XEnumerationAccess xEnumeratedFields = fieldsSupplier.getTextFields();
        XEnumeration enumeration = xEnumeratedFields.createEnumeration();
        while (enumeration.hasMoreElements()) {
              Object field =  enumeration.nextElement();
                XPropertySet props2 = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, field);
                DateTime dateField2 = (DateTime) props2.getPropertyValue("DateTimeValue");
                assertEquals("Verify date field is creatd correct by save and reload.", expectDay, dateField2.Day);
        }

    }




}
