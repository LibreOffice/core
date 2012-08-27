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

public class CharacterHyperlink {
    private static final UnoApp app = new UnoApp();
    XText xText = null;

    @Before
    public void setUp() throws Exception {
        app.start();

    }

    @After
    public void tearDown() throws Exception {
        //app.close();
    }

    @Test@Ignore //bug120676_the hyperlink name lost and hyperlinktarget change to "_blank" when save to doc
    public void testCharacterBackHyperlinkSetting() throws Exception {
        XTextDocument xTextDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, app.newDocument("swriter"));// new a text document
        xText = xTextDocument.getText();
        xText.setString("we are Chinese,they are American.We are all living in one earth!"
                + "and we all love our home very much!!!");
        // create text cursor for selecting and formatting text
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xCursorProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        xTextCursor.gotoStart(false);
        xTextCursor.goRight((short) 102, true);
        xCursorProps.setPropertyValue("HyperLinkURL", FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/puretext/Desert.jpg")));
        xCursorProps.setPropertyValue("HyperLinkTarget","picture");
        xCursorProps.setPropertyValue("HyperLinkName","testCharacterHyperlink");

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
        XPropertySet xCursorProps_assert_odt = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_odt.getText().createTextCursor());
        //verify set property
        assertEquals("assert character hyperlink URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/puretext/Desert.jpg")),xCursorProps_assert_odt.getPropertyValue("HyperLinkURL"));
        assertEquals("assert character hyperlink target name","picture",xCursorProps_assert_odt.getPropertyValue("HyperLinkTarget"));
        assertEquals("assert character hyperlink name","testCharacterHyperlink",xCursorProps_assert_odt.getPropertyValue("HyperLinkName"));

        //reopen the document and assert row height setting
        XTextDocument assertDocument_doc=(XTextDocument)UnoRuntime.queryInterface(XTextDocument.class, app.loadDocument(Testspace.getPath("output/test.doc")));
        XPropertySet xCursorProps_assert_doc = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, assertDocument_doc.getText().createTextCursor());
        //verify set property
        assertEquals("assert character hyperlink URL",FileUtil.getUrl(Testspace.prepareData("testcase/uno/sw/puretext/Desert.jpg")),xCursorProps_assert_doc.getPropertyValue("HyperLinkURL"));
        assertEquals("assert character hyperlink target name","picture",xCursorProps_assert_doc.getPropertyValue("HyperLinkTarget"));
        assertEquals("assert character hyperlink name","testCharacterHyperlink",xCursorProps_assert_doc.getPropertyValue("HyperLinkName"));
    }
}
