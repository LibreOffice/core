package testcase.uno.sw.puretext;

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


public class CharacterStrikeThrough {
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

    @Test@Ignore //bug 120656_bold,"/","X" strike through change to single when save to doc
    public void testCharacterStrikeThroughSetting() throws Exception {
        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharStrikeout", new Short(com.sun.star.awt.FontStrikeout.BOLD));
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharStrikeout", new Short(com.sun.star.awt.FontStrikeout.DONTKNOW));
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharStrikeout", new Short(com.sun.star.awt.FontStrikeout.DOUBLE));
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharStrikeout", new Short(com.sun.star.awt.FontStrikeout.NONE));
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharStrikeout", new Short(com.sun.star.awt.FontStrikeout.SINGLE));
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharStrikeout", new Short(com.sun.star.awt.FontStrikeout.SLASH));
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharStrikeout", new Short(com.sun.star.awt.FontStrikeout.X));
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

        //reopen the document and assert row height setting
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XTextCursor xTextCursor_assert_odt=assertDocument_odt.getText().createTextCursor();
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_odt);
        //verify set property
        xTextCursor_assert_odt.gotoStart(false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert strikethrough is bold",com.sun.star.awt.FontStrikeout.BOLD,xCursorProps_assert_odt.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert strikethrough is don'tknow",com.sun.star.awt.FontStrikeout.NONE,xCursorProps_assert_odt.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert strikethrough is double",com.sun.star.awt.FontStrikeout.DOUBLE,xCursorProps_assert_odt.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert strikethrough is without",com.sun.star.awt.FontStrikeout.NONE,xCursorProps_assert_odt.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert strikethrough is single",com.sun.star.awt.FontStrikeout.SINGLE,xCursorProps_assert_odt.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert strikethrough is with /",com.sun.star.awt.FontStrikeout.SLASH,xCursorProps_assert_odt.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert strikethrough is with X",com.sun.star.awt.FontStrikeout.X,xCursorProps_assert_odt.getPropertyValue("CharStrikeout"));

        //reopen the document and assert row height setting
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XTextCursor xTextCursor_assert_doc=assertDocument_doc.getText().createTextCursor();
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_doc);
        //verify set property
        xTextCursor_assert_doc.gotoStart(false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert strikethrough is bold",com.sun.star.awt.FontStrikeout.BOLD,xCursorProps_assert_doc.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert strikethrough is don'tknow",com.sun.star.awt.FontStrikeout.NONE,xCursorProps_assert_doc.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert strikethrough is double",com.sun.star.awt.FontStrikeout.DOUBLE,xCursorProps_assert_doc.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert strikethrough is without",com.sun.star.awt.FontStrikeout.NONE,xCursorProps_assert_doc.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert strikethrough is single",com.sun.star.awt.FontStrikeout.SINGLE,xCursorProps_assert_doc.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert strikethrough is with /",com.sun.star.awt.FontStrikeout.SLASH,xCursorProps_assert_doc.getPropertyValue("CharStrikeout"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert strikethrough is with X",com.sun.star.awt.FontStrikeout.X,xCursorProps_assert_doc.getPropertyValue("CharStrikeout"));
    }
}
