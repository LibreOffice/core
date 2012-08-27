package testcase.uno.sw.puretext;

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

public class CharacterRotationAndScaleWidth {
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
    public void testCharacterRotationZeroSetting() throws Exception {
        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short) 102, true);
        xCursorProps.setPropertyValue("CharRotation", (short)0);
        xCursorProps.setPropertyValue("CharScaleWidth", (short)150);
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

        //reopen the document and assert character rotation and scale width
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_odt.getText().createTextCursor());
        //verify set property
        assertEquals("assert character rotation ",(short)0,xCursorProps_assert_odt.getPropertyValue("CharRotation"));
        assertEquals("assert character rotation ",(short)150,xCursorProps_assert_odt.getPropertyValue("CharScaleWidth"));

        //reopen the document and assert row height setting
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify set property
        assertEquals("assert character rotation ",(short)0,xCursorProps_assert_doc.getPropertyValue("CharRotation"));
        assertEquals("assert character rotation ",(short)150,xCursorProps_assert_doc.getPropertyValue("CharScaleWidth"));
    }
    @Test
    public void testCharacterRotationNinetySetting() throws Exception {
        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short) 102, true);
        xCursorProps.setPropertyValue("CharRotation", (short)900);
        xCursorProps.setPropertyValue("CharScaleWidth", (short)200);
        xCursorProps.setPropertyValue("CharRotationIsFitToLine", true);
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

        //reopen the document and assert character rotation and scale width
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_odt.getText().createTextCursor());
        //verify set property
        assertEquals("assert character rotation ",(short)900,xCursorProps_assert_odt.getPropertyValue("CharRotation"));
        assertEquals("assert character rotation ",(short)200,xCursorProps_assert_odt.getPropertyValue("CharScaleWidth"));
        assertEquals("assert character rotation ",true,xCursorProps_assert_odt.getPropertyValue("CharRotationIsFitToLine"));

        //reopen the document and assert row height setting
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify set property
        assertEquals("assert character rotation ",(short)900,xCursorProps_assert_doc.getPropertyValue("CharRotation"));
        assertEquals("assert character rotation ",(short)200,xCursorProps_assert_doc.getPropertyValue("CharScaleWidth"));
        assertEquals("assert character rotation ",true,xCursorProps_assert_odt.getPropertyValue("CharRotationIsFitToLine"));
    }
    //test character rotation 270 degree
    @Test@Ignore //bug 120673_character rotation degree change to 90 from 270 when save to doc
    public void testCharacterRotationDefineSetting() throws Exception {
        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short) 102, true);
        xCursorProps.setPropertyValue("CharRotation", (short)2700);
        xCursorProps.setPropertyValue("CharScaleWidth", (short)300);
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

        //reopen the document and assert character rotation and scale width
        XTextDocument assertDocument_odt=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.odt")));
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_odt.getText().createTextCursor());
        //verify set property
        assertEquals("assert character rotation ",(short)2700,xCursorProps_assert_odt.getPropertyValue("CharRotation"));
        assertEquals("assert character rotation ",(short)300,xCursorProps_assert_odt.getPropertyValue("CharScaleWidth"));

        //reopen the document and assert row height setting
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify set property
        assertEquals("assert character rotation ",(short)2700,xCursorProps_assert_doc.getPropertyValue("CharRotation"));
        assertEquals("assert character rotation ",(short)300,xCursorProps_assert_doc.getPropertyValue("CharScaleWidth"));
    }
}
