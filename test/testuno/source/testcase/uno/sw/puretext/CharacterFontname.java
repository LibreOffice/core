package testcase.uno.sw.puretext;

import static org.junit.Assert.*;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;
import com.sun.star.text.*;
import com.sun.star.beans.*;
import com.sun.star.frame.XStorable;
import com.sun.star.uno.UnoRuntime;

public class CharacterFontname {
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
    @Test
    public void testCharacterFontSetting() throws Exception {
        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("We are Chinese,they are American. We are all living in one earth!"
                + "and we all love our home very much!!!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American." +
                " We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!We are all living in one earth!We are Chinese,they are American. " +
                "We are all living in one earth!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Times New Roman");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Arial Black");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Aharoni");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Agency FB");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Algerian");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Andalus");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Bodoni MT Black");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "BatangChe");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Britannic Bold");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Cooper Black");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "DaunPenh");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Estrangelo Edessa");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Wingdings");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Rage Italic");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Symbol");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Tw Cen MT Condensed Extra Bold");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Vivaldi");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "SimSun");
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharFontName", "Lucida Bright");
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

        //reopen the document and assert font style
        XTextDocument assertDocument=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextCursor xTextCursor_assert = assertDocument.getText().createTextCursor();
        XPropertySet xCursorProps_assert = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert);

        xTextCursor_assert.gotoStart(false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Times New Roman",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Arial Black",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Aharoni",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Agency FB",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Algerian",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Andalus",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Bodoni MT Black",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("BatangChe",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Britannic Bold",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Cooper Black",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("DaunPenh",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Estrangelo Edessa",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Wingdings",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Rage Italic",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Symbol",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Tw Cen MT Condensed Extra Bold",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Vivaldi",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("SimSun",xCursorProps_assert.getPropertyValue("CharFontName"));
        xTextCursor_assert.gotoRange(xTextCursor_assert, false);
        xTextCursor_assert.goRight((short) 100, true);
        assertEquals("Lucida Bright",xCursorProps_assert.getPropertyValue("CharFontName"));

        //reopen the document and assert font style
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextCursor xTextCursor_assert_doc = assertDocument_doc.getText().createTextCursor();
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_doc);

        xTextCursor_assert_doc.gotoStart(false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Times New Roman",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Arial Black",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Aharoni",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Agency FB",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Algerian",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Andalus",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Bodoni MT Black",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("BatangChe",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Britannic Bold",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Cooper Black",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("DaunPenh",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Estrangelo Edessa",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Wingdings",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Rage Italic",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Symbol",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Tw Cen MT Condensed Extra Bold",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Vivaldi",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("SimSun",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("Lucida Bright",xCursorProps_assert_doc.getPropertyValue("CharFontName"));
    }
}
