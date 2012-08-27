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


public class CharacterUnderline {
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
    @Test@Ignore //bug120657_underline color lost and individual words option disable when save to doc
    public void testCharacterUnderlineSetting() throws Exception {
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
        xCursorProps.setPropertyValue("CharWordMode", true);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.DOUBLE));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x00FF00FF);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.WAVE));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x00FF00FF);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.SMALLWAVE));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x00FF00FF);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", true);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.SINGLE));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", true);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.NONE));
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.LONGDASH));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.DOUBLEWAVE));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.DONTKNOW));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.DOTTED));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.DASHDOTDOT));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.DASHDOT));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.DASH));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.BOLDWAVE));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.BOLDLONGDASH));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.BOLDDOTTED));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.BOLDDASHDOTDOT));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.BOLDDASHDOT));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.BOLDDASH));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
        xTextCursor.gotoRange(xTextCursor, false);
        xTextCursor.goRight((short) 100, true);
        xCursorProps.setPropertyValue("CharWordMode", false);
        xCursorProps.setPropertyValue("CharUnderline", new Short(com.sun.star.awt.FontUnderline.BOLD));
        xCursorProps.setPropertyValue("CharUnderlineHasColor", true);
        xCursorProps.setPropertyValue("CharUnderlineColor", 0x0000FF00);
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
        XTextCursor xTextCursor_assert_odt = assertDocument_odt.getText().createTextCursor();
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_odt);

        xTextCursor_assert_odt.gotoStart(false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individual word setting",true,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is double",com.sun.star.awt.FontUnderline.DOUBLE,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x00FF00FF,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individual word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is wave",com.sun.star.awt.FontUnderline.WAVE,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x00FF00FF,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individual word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is smallwave",com.sun.star.awt.FontUnderline.SMALLWAVE,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x00FF00FF,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individual word setting",true,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is single",com.sun.star.awt.FontUnderline.SINGLE,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individual word setting",true,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is without",com.sun.star.awt.FontUnderline.NONE,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individual word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is LONGDASH",com.sun.star.awt.FontUnderline.LONGDASH,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individual word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DOUBLEWAVE",com.sun.star.awt.FontUnderline.DOUBLEWAVE,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DONTKNOW",com.sun.star.awt.FontUnderline.NONE,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DOTTED",com.sun.star.awt.FontUnderline.DOTTED,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DASHDOTDOT",com.sun.star.awt.FontUnderline.DASHDOTDOT,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DASHDOT",com.sun.star.awt.FontUnderline.DASHDOT,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DASH",com.sun.star.awt.FontUnderline.DASH,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDWAVE",com.sun.star.awt.FontUnderline.BOLDWAVE,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDLONGDASH",com.sun.star.awt.FontUnderline.BOLDLONGDASH,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDDOTTED",com.sun.star.awt.FontUnderline.BOLDDOTTED,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDDASHDOTDOT",com.sun.star.awt.FontUnderline.BOLDDASHDOTDOT,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDDASHDOT",com.sun.star.awt.FontUnderline.BOLDDASHDOT,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDDASH",com.sun.star.awt.FontUnderline.BOLDDASH,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_odt.gotoRange(xTextCursor_assert_odt, false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_odt.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is bold",com.sun.star.awt.FontUnderline.BOLD,xCursorProps_assert_odt.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_odt.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_odt.getPropertyValue("CharUnderlineColor"));

        //reopen the document and assert row height setting
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XTextCursor xTextCursor_assert_doc = assertDocument_doc.getText().createTextCursor();
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor_assert_doc);

        xTextCursor_assert_odt.gotoStart(false);
        xTextCursor_assert_odt.goRight((short) 100, true);
        assertEquals("assert individual word setting",true,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is double",com.sun.star.awt.FontUnderline.DOUBLE,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x00FF00FF,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individual word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is wave",com.sun.star.awt.FontUnderline.WAVE,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x00FF00FF,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individual word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is smallwave",com.sun.star.awt.FontUnderline.SMALLWAVE,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x00FF00FF,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individual word setting",true,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is single",com.sun.star.awt.FontUnderline.SINGLE,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individual word setting",true,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is without",com.sun.star.awt.FontUnderline.NONE,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individual word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is LONGDASH",com.sun.star.awt.FontUnderline.LONGDASH,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color is true",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individual word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DOUBLEWAVE",com.sun.star.awt.FontUnderline.DOUBLEWAVE,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DONTKNOW",com.sun.star.awt.FontUnderline.NONE,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DOTTED",com.sun.star.awt.FontUnderline.DOTTED,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DASHDOTDOT",com.sun.star.awt.FontUnderline.DASHDOTDOT,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DASHDOT",com.sun.star.awt.FontUnderline.DASHDOT,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is DASH",com.sun.star.awt.FontUnderline.DASH,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDWAVE",com.sun.star.awt.FontUnderline.BOLDWAVE,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDLONGDASH",com.sun.star.awt.FontUnderline.BOLDLONGDASH,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDDOTTED",com.sun.star.awt.FontUnderline.BOLDDOTTED,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDDASHDOTDOT",com.sun.star.awt.FontUnderline.BOLDDASHDOTDOT,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDDASHDOT",com.sun.star.awt.FontUnderline.BOLDDASHDOT,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is BOLDDASH",com.sun.star.awt.FontUnderline.BOLDDASH,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
        xTextCursor_assert_doc.gotoRange(xTextCursor_assert_doc, false);
        xTextCursor_assert_doc.goRight((short) 100, true);
        assertEquals("assert individula word setting",false,xCursorProps_assert_doc.getPropertyValue("CharWordMode"));
        assertEquals("assert underline is bold",com.sun.star.awt.FontUnderline.BOLD,xCursorProps_assert_doc.getPropertyValue("CharUnderline"));
        assertEquals("assert has underline color",true,xCursorProps_assert_doc.getPropertyValue("CharUnderlineHasColor"));
        assertEquals("assert underline color",0x0000FF00,xCursorProps_assert_doc.getPropertyValue("CharUnderlineColor"));
    }
}
