package testcase.uno.sw.paragraph;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;
//import org.openoffice.test.vcl.Tester.*;
import com.sun.star.text.*;
import com.sun.star.beans.*;
import com.sun.star.frame.XStorable;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.style.*;

public class ParagraphLineSpacing {
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
     * test paragraph line spacing is fix
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph line spacing is fix
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph line spacing
     */
    @Test
    public void testParagraphLineSpacingFix() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph line spacing
        LineSpacing lineSpacing = new LineSpacing();
        lineSpacing.Mode = LineSpacingMode.FIX;
        lineSpacing.Height = 5000;
        xCursorProps.setPropertyValue("ParaLineSpacing",lineSpacing);
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

        //reopen the document and assert paragraph line spacing
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_odt.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_odt=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_odt.getPropertyValue("ParaLineSpacing"));
        assertEquals("assert first paragraph line spacing is fix",LineSpacingMode.FIX,paraLineSpacing_assert_odt.Mode);
        assertEquals("assert first paragraph line spacing is fix",5001,paraLineSpacing_assert_odt.Height);

        //reopen the document and assert paragraph line spacing
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_doc.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_doc=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_doc.getPropertyValue("ParaLineSpacing"));
        assertEquals("assert first paragraph line spacing is fix",LineSpacingMode.FIX,paraLineSpacing_assert_doc.Mode);
        assertEquals("assert first paragraph line spacing is fix",5001,paraLineSpacing_assert_doc.Height);
    }
    /*
     * test paragraph line spacing is leading
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph line spacing is leading
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph line spacing
     */
    @Test@Ignore //bug 120647_line spacing leading setting change to at least when save to doc
    public void testParagraphLineSpacingLeading() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph line spacing
        LineSpacing lineSpacing = new LineSpacing();
        lineSpacing.Mode = LineSpacingMode.LEADING;
        lineSpacing.Height = 5000;
        xCursorProps.setPropertyValue("ParaLineSpacing",lineSpacing);
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
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_odt.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_odt=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_odt.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is fix",LineSpacingMode.LEADING,paraLineSpacing_assert_odt.Mode);
        assertEquals("assert first paragraph line spacing is fix",5001,paraLineSpacing_assert_odt.Height);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_doc.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_doc=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_doc.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is fix",LineSpacingMode.LEADING,paraLineSpacing_assert_doc.Mode);
        assertEquals("assert first paragraph line spacing is fix",5001,paraLineSpacing_assert_doc.Height);
    }
    /*
     * test paragraph line spacing is minimum
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph line spacing is minimum
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph line spacing
     */
    @Test
    public void testParagraphLineSpacingMinimum() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph line spacing
        LineSpacing lineSpacing = new LineSpacing();
        lineSpacing.Mode = LineSpacingMode.MINIMUM;
        lineSpacing.Height = 5000;
        xCursorProps.setPropertyValue("ParaLineSpacing",lineSpacing);
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
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_odt.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_odt=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_odt.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is fix",LineSpacingMode.MINIMUM,paraLineSpacing_assert_odt.Mode);
        assertEquals("assert first paragraph line spacing is fix",5001,paraLineSpacing_assert_odt.Height);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_doc.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_doc=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_doc.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is fix",LineSpacingMode.MINIMUM,paraLineSpacing_assert_doc.Mode);
        assertEquals("assert first paragraph line spacing is fix",5001,paraLineSpacing_assert_doc.Height);
    }
    /*
     * test paragraph line spacing is prop
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph alignment is prop
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph line spacing
     */
    @Test
    public void testParagraphLineSpacingProp() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph line spacing
        LineSpacing lineSpacing = new LineSpacing();
        lineSpacing.Mode = LineSpacingMode.PROP;
        lineSpacing.Height = 150;
        xCursorProps.setPropertyValue("ParaLineSpacing",lineSpacing);
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
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_odt.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_odt=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_odt.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert line spacing is prop",LineSpacingMode.PROP,paraLineSpacing_assert_odt.Mode);
        assertEquals("assert line spacing height is 150",150,paraLineSpacing_assert_odt.Height);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_doc.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_doc=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_doc.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert line spacing is prop",LineSpacingMode.PROP,paraLineSpacing_assert_doc.Mode);
        assertEquals("assert line spacing height is 150",150,paraLineSpacing_assert_doc.Height);
    }
    /*
     * test paragraph line spacing is single
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph line spacing is single
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph line spacing
     */
    @Test@Ignore //bug 120649_single line spacing change to "at least" of "0.07" when save to doc
    public void testParagraphLineSpacingSingle() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph line spacing
        LineSpacing lineSpacing = new LineSpacing();
        lineSpacing.Height = 100;
        xCursorProps.setPropertyValue("ParaLineSpacing",lineSpacing);
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
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_odt.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_odt=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_odt.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is single",100,paraLineSpacing_assert_odt.Height);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_doc.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_doc=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_doc.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is single",100,paraLineSpacing_assert_doc.Height);
    }
    /*
     * test paragraph line spacing is double
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph line spacing is double
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph line spacing
     */
    @Test
    public void testParagraphLineSpacingDouble() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph line spacing
        LineSpacing lineSpacing = new LineSpacing();
        lineSpacing.Height = 200;
        xCursorProps.setPropertyValue("ParaLineSpacing",lineSpacing);
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
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_odt.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_odt=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_odt.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is single",200,paraLineSpacing_assert_odt.Height);

        //reopen the document and assert line spacing
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_doc.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_doc=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_doc.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is single",200,paraLineSpacing_assert_doc.Height);
    }
    /*
     * test paragraph line spacing is 1.5line
     * 1.new a text document
     * 2.insert some text
     * 3.set paragraph line spacing is 1.5line
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph line spacing
     */
    @Test
    public void testParagraphLineSpacingUserDefine() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        //set paragraph line spacing
        LineSpacing lineSpacing = new LineSpacing();
        lineSpacing.Height = 150;
        xCursorProps.setPropertyValue("ParaLineSpacing",lineSpacing);
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
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_odt.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_odt=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_odt.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is single",150,paraLineSpacing_assert_odt.Height);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,assertDocument_doc.getText().createTextCursor());
        LineSpacing paraLineSpacing_assert_doc=(LineSpacing) UnoRuntime.queryInterface(LineSpacing.class, xCursorProps_assert_doc.getPropertyValue("ParaLineSpacing"));
        //verify paragraph line spacing property
        assertEquals("assert first paragraph line spacing is single",150,paraLineSpacing_assert_doc.Height);
    }
}
