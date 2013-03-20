/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package complex.writer;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XServiceInfo;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XEnumeration;
import com.sun.star.frame.XStorable;
import com.sun.star.util.XCloseable;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextField;
import com.sun.star.text.XTextFieldsSupplier;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextSection;
import com.sun.star.text.XParagraphCursor;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XDependentTextField;
import org.openoffice.test.OfficeConnection;
import com.sun.star.text.XWordCursor;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;

import java.util.Set;
import java.util.HashSet;
import java.io.File;
import java.net.URI;


/**
 * This is the efforts to create a unit test to reproduce fdo#55814
 *
 * To preserve the document, set env var MY_TMP_PATH=file:///tmp/foo and run
 * make JunitTest_sw_complex, then check /tmp/foo/foo.odt.
 *
 * We still have problems programmatically reproduce the original bug fdo#61950:
 * 1. variable field is not shown
 *    enter "Enter" and "Remove" or
 *    toggle Invisible/Visible in writer solve it though =>
 *    "0" is shown
 * 2. section is empty
 * TODO:
 * a) fill section with some text
 * b) trigger the value change event of the `foo` var from 0 to 1
 * c) retrieve the section and check that the condition get corrupted:
 *    "foo EQ 1" => "0"
 **/

public class VarFields
{
    private static final OfficeConnection connection = new OfficeConnection();

    @BeforeClass public static void setUpConnection() throws Exception {
        connection.setUp();
        //Thread.sleep(5000);
    }

    @AfterClass public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        connection.tearDown();
    }

    private XMultiServiceFactory m_xMSF = null;
    private XComponentContext m_xContext = null;
    private XTextDocument m_xDoc = null;
    private String m_TmpDir = null;
    private String m_FileName = null;

    @Before public void before() throws Exception
    {
        m_xMSF = UnoRuntime.queryInterface(
            XMultiServiceFactory.class,
            connection.getComponentContext().getServiceManager());
        m_xContext = connection.getComponentContext();
        assertNotNull("could not get component context.", m_xContext);
        m_xDoc = util.WriterTools.createTextDoc(m_xMSF);
        m_TmpDir = util.utils.getOfficeTemp/*Dir*/(m_xMSF);
        String myPath = System.getenv("MY_TMP_DIR");
        if (myPath != null && myPath.length() > 0)
        {
            m_TmpDir = myPath;
            checkTmpDirExists(myPath);
        }
        m_FileName = m_TmpDir + File.separator + "foo.odt";
        System.out.println("file: " + m_FileName);
    }

    @After public void after()
    {
        util.DesktopTools.closeDoc(m_xDoc);
    }

    void checkTmpDirExists(String target) throws Exception
    {
        File f = new File(new URI(target));
        if (!f.exists()) {
            if (!f.mkdirs()) {
                throw new Exception("cannot mkdir: " + target);
            }
        }
    }

    @Test
    public void test_fdo_55814() throws Exception
    {
        XMultiServiceFactory xDocFactory = UnoRuntime.queryInterface(XMultiServiceFactory.class, m_xDoc);
        XText xBodyText = m_xDoc.getText();
        XTextCursor xCursor = xBodyText.createTextCursor();
        // 0. create text field
        Object xField = xDocFactory.createInstance("com.sun.star.text.textfield.SetExpression");
        // 1. fill it with properties
        XPropertySet xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xField);
        xPropSet.setPropertyValue("Content", "0");
        xPropSet.setPropertyValue("IsVisible", true);
        xPropSet.setPropertyValue("Hint", "trying to reproduce fdo#61950");
        xPropSet.setPropertyValue("SubType", 0);// VAR
        xPropSet.setPropertyValue("Value", 0.0);
        xPropSet.setPropertyValue("Content", "0");
        // 2. create master field
        Object xMaster = xDocFactory.createInstance("com.sun.star.text.fieldmaster.SetExpression");
        xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xMaster);
        // 3. set name of the master field to "foo"
        xPropSet.setPropertyValue("Name", "foo");
        // 4. get Dependent Field
        XDependentTextField xDependentTextField = UnoRuntime.queryInterface(XDependentTextField.class, xField);
        // 5. connect real field to the master
        xDependentTextField.attachTextFieldMaster(xPropSet);
        // 6. insert text field into the document

        // TODO: field is no shown ("0" is expected). UNO bug?

        xBodyText.insertTextContent(xCursor, UnoRuntime.queryInterface(XTextContent.class, xField), false);
        // 7. retrieve paragraph cursor
        XParagraphCursor xParagraphCursor = UnoRuntime.queryInterface(XParagraphCursor.class, xCursor);
        xParagraphCursor.gotoEndOfParagraph(false /*not select*/);
        // 8. enter new line
        xBodyText.insertControlCharacter( xCursor, ControlCharacter.PARAGRAPH_BREAK, false );
        // 9. create new section
        Object xSection = xDocFactory.createInstance("com.sun.star.text.TextSection");
        XTextSection xTextSection = UnoRuntime.queryInterface(XTextSection.class, xSection);
        // 10. fill the properties of section
        xPropSet = UnoRuntime.queryInterface(XPropertySet.class, xTextSection);
        xPropSet.setPropertyValue("Condition", "foo EQ 1");
        xPropSet.setPropertyValue("IsVisible", false);
        Object readContent = xPropSet.getPropertyValue("Condition");
        assertEquals("foo EQ 1", readContent);
        // 11. insert section
        XTextContent xTextContext = UnoRuntime.queryInterface(XTextContent.class, xSection);
        xBodyText.insertTextContent(xCursor, xTextContext, true);
        // 12. tell me what am i missing here? The section is empty.
        // ???
        // 12. store document
        doStore(m_xDoc, m_FileName);
    }

    private void doStore(XComponent xComp, String file) throws Exception
    {
        System.out.println("Storing test document: " + file);
        XStorable xStor = UnoRuntime.queryInterface(XStorable.class, xComp);
        xStor.storeToURL(file, new PropertyValue[0]);
        System.out.println("...done");
    }

}
