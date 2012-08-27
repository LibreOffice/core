package testcase.uno.sw.paragraph;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.style.DropCapFormat;
import com.sun.star.text.*;
import com.sun.star.beans.*;
import com.sun.star.frame.XStorable;
import com.sun.star.uno.UnoRuntime;

public class ParagraphDropcap {
    private static final UnoApp app = new UnoApp();
    XText xText = null;

    @Before
    public void setUp() throws Exception {
        app.start();

    }

    @After
    public void tearDown() throws Exception {
        app.close();
    }
    /*
     * test paragraph background color
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph drop
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph drop cap
     */
    @Test
    public void testParagraphDropcapSetting() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph dropcaps
        DropCapFormat dropcapFormat=new DropCapFormat();
        dropcapFormat.Lines=3;
        dropcapFormat.Distance=101;
        dropcapFormat.Count=9;
        xCursorProps.setPropertyValue("DropCapFormat",dropcapFormat);
        //xCursorProps.setPropertyValue("DropCapWholeWord",true);
        //save to odt
        XStorable xStorable_odt = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties_odt = new PropertyValue[2];
        aStoreProperties_odt[0] = new PropertyValue();
        aStoreProperties_odt[1] = new PropertyValue();
        aStoreProperties_odt[0].Name = "Override";
        aStoreProperties_odt[0].Value = true;
        aStoreProperties_odt[1].Name = "FilterName";
        aStoreProperties_odt[1].Value = "StarOffice XML (Writer)";
        xStorable_odt.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties_odt);
        //save to doc
        XStorable xStorable_doc = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties_doc = new PropertyValue[2];
        aStoreProperties_doc[0] = new PropertyValue();
        aStoreProperties_doc[1] = new PropertyValue();
        aStoreProperties_doc[0].Name = "Override";
        aStoreProperties_doc[0].Value = true;
        aStoreProperties_doc[1].Name = "FilterName";
        aStoreProperties_doc[1].Value = "MS Word 97";
        xStorable_doc.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.doc")), aStoreProperties_doc);
        app.closeDocument(xTextDocument);

        //reopen the document
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XPropertySet xCursorProps_Assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_odt.getText().createTextCursor());
        //verify paragraph dropcap
        DropCapFormat dropcapFormat_assert_odt = (DropCapFormat) UnoRuntime.queryInterface(DropCapFormat.class,xCursorProps_Assert_odt.getPropertyValue("DropCapFormat"));
        assertEquals("assert paragraph dropcaps",9,dropcapFormat_assert_odt.Count);
        assertEquals("assert paragraph dropcaps",3,dropcapFormat_assert_odt.Lines);
        assertEquals("assert paragraph dropcaps",101,dropcapFormat_assert_odt.Distance);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph dropcap
        DropCapFormat dropcapFormat_assert_doc = (DropCapFormat) UnoRuntime.queryInterface(DropCapFormat.class,xCursorProps_Assert_doc.getPropertyValue("DropCapFormat"));
        assertEquals("assert paragraph dropcaps",9,dropcapFormat_assert_doc.Count);
        assertEquals("assert paragraph dropcaps",3,dropcapFormat_assert_doc.Lines);
        assertEquals("assert paragraph dropcaps",101,dropcapFormat_assert_doc.Distance);
    }
    @Test
    public void testParagraphDropcapWholewordSetting() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph dropcaps
        DropCapFormat dropcapFormat=new DropCapFormat();
        dropcapFormat.Lines=3;
        dropcapFormat.Distance=101;
        xCursorProps.setPropertyValue("DropCapFormat",dropcapFormat);
        xCursorProps.setPropertyValue("DropCapWholeWord",true);
        //save to odt
        XStorable xStorable_odt = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties_odt = new PropertyValue[2];
        aStoreProperties_odt[0] = new PropertyValue();
        aStoreProperties_odt[1] = new PropertyValue();
        aStoreProperties_odt[0].Name = "Override";
        aStoreProperties_odt[0].Value = true;
        aStoreProperties_odt[1].Name = "FilterName";
        aStoreProperties_odt[1].Value = "StarOffice XML (Writer)";
        xStorable_odt.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.odt")), aStoreProperties_odt);
        //save to doc
        XStorable xStorable_doc = (XStorable) UnoRuntime.queryInterface(XStorable.class, xTextDocument);
        PropertyValue[] aStoreProperties_doc = new PropertyValue[2];
        aStoreProperties_doc[0] = new PropertyValue();
        aStoreProperties_doc[1] = new PropertyValue();
        aStoreProperties_doc[0].Name = "Override";
        aStoreProperties_doc[0].Value = true;
        aStoreProperties_doc[1].Name = "FilterName";
        aStoreProperties_doc[1].Value = "MS Word 97";
        xStorable_doc.storeToURL(FileUtil.getUrl(Testspace.getPath("output/test.doc")), aStoreProperties_doc);
        app.closeDocument(xTextDocument);

        //reopen the document
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XPropertySet xCursorProps_Assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_odt.getText().createTextCursor());
        //verify paragraph dropcap
        DropCapFormat dropcapFormat_assert_odt = (DropCapFormat) UnoRuntime.queryInterface(DropCapFormat.class,xCursorProps_Assert_odt.getPropertyValue("DropCapFormat"));
        assertEquals("assert paragraph dropcaps",true,xCursorProps_Assert_odt.getPropertyValue("DropCapWholeWord"));
        assertEquals("assert paragraph dropcaps",3,dropcapFormat_assert_odt.Lines);
        assertEquals("assert paragraph dropcaps",101,dropcapFormat_assert_odt.Distance);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph dropcap
        DropCapFormat dropcapFormat_assert_doc = (DropCapFormat) UnoRuntime.queryInterface(DropCapFormat.class,xCursorProps_Assert_doc.getPropertyValue("DropCapFormat"));
        assertEquals("assert paragraph dropcaps",3,dropcapFormat_assert_doc.Lines);
        assertEquals("assert paragraph dropcaps",101,dropcapFormat_assert_doc.Distance);
        //when save to doc,the DropCapWholeWord option will disable,and enable dropcapFormat.count
        assertEquals("assert paragraph dropcaps",false,xCursorProps_Assert_doc.getPropertyValue("DropCapWholeWord"));
    }
}
