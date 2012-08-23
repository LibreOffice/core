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

public class ParagraphAlignment {
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
     * test paragraph alignment is justified
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph alignment is justified,and last line align to left,check expand single word
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph alignment
     */
    @Test@Ignore //bug120636_the expand single word option disable when save to doc
    public void testParagraphAlignmentJustified() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //apply paragraph alignment as justified and last line alignment
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short)180 , true);
        xCursorProps.setPropertyValue("ParaAdjust",com.sun.star.style.ParagraphAdjust.BLOCK);
        xCursorProps.setPropertyValue("ParaLastLineAdjust", com.sun.star.style.ParagraphAdjust.LEFT);
        xCursorProps.setPropertyValue("ParaExpandSingleWord", true);
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

        //reopen the document and assert table margin to page setting
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextCursor xTextCursor_assert_odt = assertDocument_odt.getText().createTextCursor();
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_odt);
        //verify paragraph alignment property
        assertEquals("assert first paragraph alignment is justified",(short)2,xCursorProps_assert_odt.getPropertyValue("ParaAdjust"));
        assertEquals("assert first paragraph last line alignment is left",(short)0, xCursorProps_assert_odt.getPropertyValue("ParaLastLineAdjust"));
        assertEquals("assert expand single word is true",true,xCursorProps_assert_odt.getPropertyValue("ParaExpandSingleWord"));

        //reopen the document and assert table margin to page setting
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XTextCursor xTextCursor_assert_doc = assertDocument_doc.getText().createTextCursor();
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_doc);
        //verify paragraph alignment property
        assertEquals("assert first paragraph alignment is justified",(short)2,xCursorProps_assert_doc.getPropertyValue("ParaAdjust"));
        assertEquals("assert first paragraph last line alignment is left",(short)0, xCursorProps_assert_doc.getPropertyValue("ParaLastLineAdjust"));
        assertEquals("assert expand single word is true",true,xCursorProps_assert_doc.getPropertyValue("ParaExpandSingleWord"));

    }
    /*
     * test paragraph alignment is left
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph alignment is left
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph alignment
     */
    @Test
    public void testParagraphAlignmentLeft() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short)180 , true);
        xCursorProps.setPropertyValue("ParaAdjust",com.sun.star.style.ParagraphAdjust.LEFT);
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

        //reopen the odt document and assert paragraph alignment
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextCursor xTextCursor_assert_odt = assertDocument_odt.getText().createTextCursor();
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_odt);
        //verify paragraph alignment property
        assertEquals("assert first paragraph alignment is left",(short)0,xCursorProps_assert_odt.getPropertyValue("ParaAdjust"));
        //reopen the doc document and assert paragraph alignment
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XTextCursor xTextCursor_assert_doc = assertDocument_doc.getText().createTextCursor();
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_doc);
        //verify paragraph alignment property
        assertEquals("assert first paragraph alignment is left",(short)0,xCursorProps_assert_doc.getPropertyValue("ParaAdjust"));


    }
    /*
     * test paragraph alignment is justified
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph alignment is right
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph alignment
     */
    @Test
    public void testParagraphAlignmentRight() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //apply paragraph alignment as justified and last line alignment
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short)180 , true);
        xCursorProps.setPropertyValue("ParaAdjust",com.sun.star.style.ParagraphAdjust.RIGHT);
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

        //reopen the document and assert paragraph alignment
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextCursor xTextCursor_assert_odt = assertDocument_odt.getText().createTextCursor();
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_odt);
        //verify paragraph alignment property
        assertEquals("assert first paragraph alignment is right",(short)1,xCursorProps_assert_odt.getPropertyValue("ParaAdjust"));
        //reopen the document and assert paragraph alignment
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XTextCursor xTextCursor_assert_doc = assertDocument_doc.getText().createTextCursor();
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_doc);
        //verify paragraph alignment property
        assertEquals("assert first paragraph alignment is right",(short)1,xCursorProps_assert_doc.getPropertyValue("ParaAdjust"));
    }
    /*
     * test paragraph alignment is justified
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph alignment is center
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph alignment
     */
    @Test
    public void testParagraphAlignmentCenter() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //apply paragraph alignment as justified and last line alignment
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short)180 , true);
        xCursorProps.setPropertyValue("ParaAdjust",com.sun.star.style.ParagraphAdjust.CENTER);
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

        //reopen the document and assert paragraph alignment
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextCursor xTextCursor_assert_odt = assertDocument_odt.getText().createTextCursor();
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_odt);
        assertEquals("assert first paragraph alignment is center",(short)3,xCursorProps_assert_odt.getPropertyValue("ParaAdjust"));
        //reopen the document and assert paragraph alignment
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XTextCursor xTextCursor_assert_doc = assertDocument_doc.getText().createTextCursor();
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_doc);
        assertEquals("assert first paragraph alignment is center",(short)3,xCursorProps_assert_doc.getPropertyValue("ParaAdjust"));
    }
}
