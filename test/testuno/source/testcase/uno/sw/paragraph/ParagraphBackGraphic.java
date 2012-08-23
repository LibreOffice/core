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

public class ParagraphBackGraphic {
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
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is left bottom
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_LeftBottom() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.LEFT_BOTTOM);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.LEFT_BOTTOM,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.LEFT_BOTTOM,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is left middle
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_LeftMiddle() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.LEFT_MIDDLE);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.LEFT_MIDDLE,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.LEFT_MIDDLE,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is left top
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_LeftTop() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.LEFT_TOP);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.LEFT_TOP,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.LEFT_TOP,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is middle bottom
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_MiddleBottom() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.MIDDLE_BOTTOM);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.MIDDLE_BOTTOM,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.MIDDLE_BOTTOM,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is middle middle
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_MiddleMiddle() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.MIDDLE_MIDDLE);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.MIDDLE_MIDDLE,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.MIDDLE_MIDDLE,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is middle top
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_MiddleTop() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.MIDDLE_TOP);
        //save and reload text document
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.MIDDLE_TOP,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.MIDDLE_TOP,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is right bottom
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_RightBottom() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.RIGHT_BOTTOM);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.RIGHT_BOTTOM,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.RIGHT_BOTTOM,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is right middle
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_RightMiddle() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.RIGHT_MIDDLE);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.RIGHT_MIDDLE,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.RIGHT_MIDDLE,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is right top
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_RightTop() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.RIGHT_TOP);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.RIGHT_TOP,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.RIGHT_TOP,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is area
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_Area() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.AREA);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.AREA,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.AREA,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is titled
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test@Ignore //bug120638_graphic background lost when save to doc
    public void testParagraphBackGraphic_Titled() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicURL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")));
        xCursorProps.setPropertyValue("ParaBackGraphicFilter","draw_jpg_Export");
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.TILED);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.TILED,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraphtable/Desert.jpg")),xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicURL"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.TILED,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));
        assertEquals("verify paragraph backgraphic fileter","draw_jpg_Export",xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicFilter"));
        assertEquals("verify paragraph backgraphic URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/paragraph/Desert.jpg")),xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicURL"));
    }
    /*
     * test paragraph background graphic
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph background with graphic and graphic location is none
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph background graphic
     */
    @Test
    public void testParagraphBackGraphic_Nonne() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph background color
        xCursorProps.setPropertyValue("ParaBackGraphicLocation",com.sun.star.style.GraphicLocation.NONE);
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
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.NONE,xCursorProps_Assert_odt.getPropertyValue("ParaBackGraphicLocation"));

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph background graphic
        assertEquals("verify paragraph backgraphic location",com.sun.star.style.GraphicLocation.NONE,xCursorProps_Assert_doc.getPropertyValue("ParaBackGraphicLocation"));

    }
}
