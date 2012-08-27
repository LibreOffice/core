package testcase.uno.sw.paragraph;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.table.BorderLine;
import com.sun.star.text.*;
import com.sun.star.beans.*;
import com.sun.star.frame.XStorable;
import com.sun.star.uno.UnoRuntime;

public class ParagraphBorder {
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
     * 3.set paragraph border
     * 4.save and close the document
     * 5.reload the saved document and check the paragraph border
     */
    @Test@Ignore //bug120694_the paragraph border line style change when save to doc.
    public void testParagraphBorderSetting() throws Exception {

        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.we are all living in one earth!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!Hello,world!" +
                "Hello,world!Hello,world!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        BorderLine[]borderLine=new BorderLine[] {new BorderLine(),new BorderLine(),new BorderLine(),new BorderLine()};
        borderLine[0].Color=0x00FF0000;
        borderLine[0].InnerLineWidth=101;
        borderLine[0].OuterLineWidth=19;
        borderLine[0].LineDistance=100;
        borderLine[1].Color =0x00FFFF00;
        borderLine[1].InnerLineWidth=101;
        borderLine[1].OuterLineWidth=19;
        borderLine[1].LineDistance=101;
        borderLine[2].Color =0x0000FF00;
        borderLine[2].InnerLineWidth=150;
        borderLine[2].OuterLineWidth=19;
        borderLine[2].LineDistance=101;
        borderLine[3].Color =0x0000FF00;
        borderLine[3].InnerLineWidth=150;
        borderLine[3].OuterLineWidth=19;
        borderLine[3].LineDistance=101;
        xCursorProps.setPropertyValue("LeftBorder", borderLine[0]);
        xCursorProps.setPropertyValue("RightBorder", borderLine[1]);
        xCursorProps.setPropertyValue("TopBorder", borderLine[2]);
        xCursorProps.setPropertyValue("BottomBorder", borderLine[3]);
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
        //verify paragraph border
        Object borderLine_odt[]={xCursorProps_Assert_odt.getPropertyValue("LeftBorder"),xCursorProps_Assert_odt.getPropertyValue("RightBorder"),xCursorProps_Assert_odt.getPropertyValue("TopBorder"),xCursorProps_Assert_odt.getPropertyValue("BottomBorder")};
        BorderLine leftborderLine_odt=(BorderLine)UnoRuntime.queryInterface(BorderLine.class,borderLine_odt[0]);
        assertEquals("assert paragraph left border",0x00FF0000,leftborderLine_odt.Color);
        assertEquals("assert paragraph left border",101,leftborderLine_odt.InnerLineWidth);
        assertEquals("assert paragraph left border",19,leftborderLine_odt.OuterLineWidth);
        assertEquals("assert paragraph left border",101,leftborderLine_odt.LineDistance);

        BorderLine rightborderLine_odt=(BorderLine)UnoRuntime.queryInterface(BorderLine.class,borderLine_odt[1]);
        assertEquals("assert paragraph left border",0x00FFFF00,rightborderLine_odt.Color);
        assertEquals("assert paragraph left border",101,rightborderLine_odt.InnerLineWidth);
        assertEquals("assert paragraph left border",19,rightborderLine_odt.OuterLineWidth);
        assertEquals("assert paragraph left border",101,rightborderLine_odt.LineDistance);

        BorderLine topborderLine_odt=(BorderLine)UnoRuntime.queryInterface(BorderLine.class,borderLine_odt[2]);
        assertEquals("assert paragraph left border",0x0000FF00,topborderLine_odt.Color);
        assertEquals("assert paragraph left border",150,topborderLine_odt.InnerLineWidth);
        assertEquals("assert paragraph left border",19,topborderLine_odt.OuterLineWidth);
        assertEquals("assert paragraph left border",101,topborderLine_odt.LineDistance);

        BorderLine bottomtborderLine_odt=(BorderLine)UnoRuntime.queryInterface(BorderLine.class,borderLine_odt[3]);
        assertEquals("assert paragraph left border",0x0000FF00,bottomtborderLine_odt.Color);
        assertEquals("assert paragraph left border",150,bottomtborderLine_odt.InnerLineWidth);
        assertEquals("assert paragraph left border",19,bottomtborderLine_odt.OuterLineWidth);
        assertEquals("assert paragraph left border",101,bottomtborderLine_odt.LineDistance);

        //reopen the document
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_Assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify paragraph border
        Object borderLine_doc[]={xCursorProps_Assert_doc.getPropertyValue("LeftBorder"),xCursorProps_Assert_doc.getPropertyValue("RightBorder"),xCursorProps_Assert_doc.getPropertyValue("TopBorder"),xCursorProps_Assert_doc.getPropertyValue("BottomBorder")};
        BorderLine leftborderLine_doc=(BorderLine)UnoRuntime.queryInterface(BorderLine.class,borderLine_doc[0]);
        assertEquals("assert paragraph left border",0x00FF0000,leftborderLine_doc.Color);
        assertEquals("assert paragraph left border",101,leftborderLine_doc.InnerLineWidth);
        assertEquals("assert paragraph left border",19,leftborderLine_doc.OuterLineWidth);
        assertEquals("assert paragraph left border",101,leftborderLine_doc.LineDistance);

        BorderLine rightborderLine_doc=(BorderLine)UnoRuntime.queryInterface(BorderLine.class,borderLine_doc[1]);
        assertEquals("assert paragraph left border",0x00FFFF00,rightborderLine_doc.Color);
        assertEquals("assert paragraph left border",101,rightborderLine_doc.InnerLineWidth);
        assertEquals("assert paragraph left border",19,rightborderLine_doc.OuterLineWidth);
        assertEquals("assert paragraph left border",101,rightborderLine_doc.LineDistance);

        BorderLine topborderLine_doc=(BorderLine)UnoRuntime.queryInterface(BorderLine.class,borderLine_doc[2]);
        assertEquals("assert paragraph left border",0x0000FF00,topborderLine_doc.Color);
        assertEquals("assert paragraph left border",150,topborderLine_doc.InnerLineWidth);
        assertEquals("assert paragraph left border",19,topborderLine_doc.OuterLineWidth);
        assertEquals("assert paragraph left border",101,topborderLine_doc.LineDistance);

        BorderLine bottomtborderLine_doc=(BorderLine)UnoRuntime.queryInterface(BorderLine.class,borderLine_doc[3]);
        assertEquals("assert paragraph left border",0x0000FF00,bottomtborderLine_doc.Color);
        assertEquals("assert paragraph left border",150,bottomtborderLine_doc.InnerLineWidth);
        assertEquals("assert paragraph left border",19,bottomtborderLine_doc.OuterLineWidth);
        assertEquals("assert paragraph left border",101,bottomtborderLine_doc.LineDistance);
    }
}
