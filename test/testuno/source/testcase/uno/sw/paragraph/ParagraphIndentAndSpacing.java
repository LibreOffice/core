package testcase.uno.sw.paragraph;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;
import com.sun.star.text.*;
import com.sun.star.beans.*;
import com.sun.star.frame.XStorable;
import com.sun.star.uno.UnoRuntime;

public class ParagraphIndentAndSpacing {
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
     * test paragraph spacing
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph spacing:before text,after text,above paragraph,below paragraph
     * 4.save to odt and close it
     * 5.reopen the saved document and check the paragraph spacing
     */
    @Test
    public void testParagraphSpacingSetting() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph margin with page border
        xCursorProps.setPropertyValue("ParaLeftMargin",2000);
        xCursorProps.setPropertyValue("ParaRightMargin",3000);
        xCursorProps.setPropertyValue("ParaTopMargin",1000);
        xCursorProps.setPropertyValue("ParaBottomMargin",4000);
        xCursorProps.setPropertyValue("ParaFirstLineIndent",4000);

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

        //reopen the document and assert line spacing
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XPropertySet xCursorProps_Assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_odt.getText().createTextCursor());
        //verify paragraph indent and spacing
        assertEquals("assert before text margin",2000,xCursorProps_Assert_odt.getPropertyValue("ParaLeftMargin"));
        assertEquals("assert after text margin",3000,xCursorProps_Assert_odt.getPropertyValue("ParaRightMargin"));
        assertEquals("assert above paragraph margin",1000,xCursorProps_Assert_odt.getPropertyValue("ParaTopMargin"));
        assertEquals("assert below paragraph margin",4001,xCursorProps_Assert_odt.getPropertyValue("ParaBottomMargin"));
        assertEquals("assert first line indent",4001,xCursorProps_Assert_odt.getPropertyValue("ParaFirstLineIndent"));
        assertEquals("assert paragraph first line is no autoindent",false,xCursorProps_Assert_odt.getPropertyValue("ParaIsAutoFirstLineIndent"));

        //reopen the document and assert line spacing
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph indent and spacing
        assertEquals("assert before text margin",2000,xCursorProps_Assert_doc.getPropertyValue("ParaLeftMargin"));
        assertEquals("assert after text margin",3000,xCursorProps_Assert_doc.getPropertyValue("ParaRightMargin"));
        assertEquals("assert above paragraph margin",1000,xCursorProps_Assert_doc.getPropertyValue("ParaTopMargin"));
        assertEquals("assert below paragraph margin",4001,xCursorProps_Assert_doc.getPropertyValue("ParaBottomMargin"));
        assertEquals("assert first line indent",4001,xCursorProps_Assert_doc.getPropertyValue("ParaFirstLineIndent"));
        assertEquals("assert paragraph first line is no autoindent",false,xCursorProps_Assert_doc.getPropertyValue("ParaIsAutoFirstLineIndent"));
    }
    @Test@Ignore //bug 120646_the auto indent effect of first line lost when save to doc
    public void testParagraphIndent() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph margin with page border
        xCursorProps.setPropertyValue("ParaIsAutoFirstLineIndent",true);

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


        //reopen the document and assert paragraph indent
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph auto indent
        assertEquals("assert paragraph first line is autoindent",true,xCursorProps_Assert_doc.getPropertyValue("ParaIsAutoFirstLineIndent"));

        //reopen the document and assert paragraph indent
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XPropertySet xCursorProps_Assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_odt.getText().createTextCursor());
        //verify paragraph auto indent
        assertEquals("assert paragraph first line is autoindent",true,xCursorProps_Assert_odt.getPropertyValue("ParaIsAutoFirstLineIndent"));
    }

}
