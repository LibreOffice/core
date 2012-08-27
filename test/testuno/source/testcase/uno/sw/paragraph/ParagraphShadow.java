package testcase.uno.sw.paragraph;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.table.ShadowFormat;
import com.sun.star.table.ShadowLocation;
import com.sun.star.text.*;
import com.sun.star.beans.*;
import com.sun.star.frame.XStorable;
import com.sun.star.uno.UnoRuntime;

public class ParagraphShadow {
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
     * 3.set paragraph shadow
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph shadow
     */
    @Test@Ignore //bug120697_paragraph shadow effect lost when save to doc.
    public void testParagraphShadow_BottomRight() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        ShadowFormat shadowFormat=new ShadowFormat();
        shadowFormat.Location=ShadowLocation.BOTTOM_RIGHT;
        shadowFormat.ShadowWidth=101;
        shadowFormat.Color=0x00FF00FF;
        xCursorProps.setPropertyValue("ParaShadowFormat",shadowFormat);
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
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert1=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_odt.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.BOTTOM_RIGHT,shadowFormat_Assert1.Location);
        assertEquals("assert shadow width",101,shadowFormat_Assert1.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert1.Color);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert2=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_doc.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.BOTTOM_RIGHT,shadowFormat_Assert2.Location);
        assertEquals("assert shadow width",100,shadowFormat_Assert2.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert2.Color);
    }
    @Test@Ignore //bug120697_paragraph shadow effect lost when save to doc.
    public void testParagraphShadow_BottomLeft() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        ShadowFormat shadowFormat=new ShadowFormat();
        shadowFormat.Location=ShadowLocation.BOTTOM_LEFT;
        shadowFormat.ShadowWidth=101;
        shadowFormat.Color=0x00FF00FF;
        xCursorProps.setPropertyValue("ParaShadowFormat",shadowFormat);
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
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert1=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_odt.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.BOTTOM_LEFT,shadowFormat_Assert1.Location);
        assertEquals("assert shadow width",101,shadowFormat_Assert1.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert1.Color);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert2=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_doc.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.BOTTOM_LEFT,shadowFormat_Assert2.Location);
        assertEquals("assert shadow width",100,shadowFormat_Assert2.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert2.Color);
    }
    @Test@Ignore //bug120697_paragraph shadow effect lost when save to doc.
    public void testParagraphShadow_TopLeft() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        ShadowFormat shadowFormat=new ShadowFormat();
        shadowFormat.Location=ShadowLocation.TOP_LEFT;
        shadowFormat.ShadowWidth=101;
        shadowFormat.Color=0x00FF00FF;
        xCursorProps.setPropertyValue("ParaShadowFormat",shadowFormat);
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
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert1=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_odt.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.TOP_LEFT,shadowFormat_Assert1.Location);
        assertEquals("assert shadow width",101,shadowFormat_Assert1.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert1.Color);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert2=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_doc.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.TOP_LEFT,shadowFormat_Assert2.Location);
        assertEquals("assert shadow width",100,shadowFormat_Assert2.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert2.Color);
    }
    @Test@Ignore //bug120697_paragraph shadow effect lost when save to doc.
    public void testParagraphShadow_TopRight() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        ShadowFormat shadowFormat=new ShadowFormat();
        shadowFormat.Location=ShadowLocation.TOP_RIGHT;
        shadowFormat.ShadowWidth=101;
        shadowFormat.Color=0x00FF00FF;
        xCursorProps.setPropertyValue("ParaShadowFormat",shadowFormat);
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
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert1=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_odt.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.TOP_RIGHT,shadowFormat_Assert1.Location);
        assertEquals("assert shadow width",101,shadowFormat_Assert1.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert1.Color);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert2=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_doc.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.TOP_RIGHT,shadowFormat_Assert2.Location);
        assertEquals("assert shadow width",100,shadowFormat_Assert2.ShadowWidth);
        assertEquals("assert shadow color",0x00FF00FF,shadowFormat_Assert2.Color);
    }
    @Test
    public void testParagraphShadow_None() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        ShadowFormat shadowFormat=new ShadowFormat();
        shadowFormat.Location=ShadowLocation.NONE;
        shadowFormat.ShadowWidth=101;
        shadowFormat.Color=0x00FF00FF;
        xCursorProps.setPropertyValue("ParaShadowFormat",shadowFormat);
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
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert1=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_odt.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.NONE,shadowFormat_Assert1.Location);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background color
        ShadowFormat shadowFormat_Assert2=(ShadowFormat) UnoRuntime.queryInterface(ShadowFormat.class, xCursorProps_Assert_doc.getPropertyValue("ParaShadowFormat"));
        assertEquals("assert shadow location",ShadowLocation.NONE,shadowFormat_Assert2.Location);
    }
}
