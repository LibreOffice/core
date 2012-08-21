package testcase.mix;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.uno.UnoApp;
import org.openoffice.test.vcl.widgets.VclApp;
import org.openoffice.test.vcl.widgets.VclListBox;
import org.openoffice.test.vcl.widgets.VclTabPage;
import org.openoffice.test.vcl.widgets.VclWindow;

import com.sun.star.beans.XPropertySet;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;

/**
 * Demo for testing with both UNO and VCLAuto
 * @author test
 *
 */
public class MixedTest {
    UnoApp unoApp = new UnoApp();
    VclApp vclApp = new VclApp();
    VclWindow writer = new VclWindow("SW_HID_EDIT_WIN");
    VclTabPage effectsPage = new VclTabPage("CUI_HID_SVXPAGE_CHAR_EFFECTS");
    VclListBox colorList = new VclListBox("cui:ListBox:RID_SVXPAGE_CHAR_EFFECTS:LB_FONTCOLOR");
    XTextDocument textDocument = null;
    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        unoApp.start();
    }

    @After
    public void tearDown() throws Exception {
        unoApp.closeDocument(textDocument);
        unoApp.close();
    }

    @Test
    public void testUseBothUNOAndGuiAPI()  throws Exception  {
        //Use UNO API to create a new document
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, unoApp.newDocument("swriter"));
        XText xText = textDocument.getText();
        xText.setString("UNO: Hello World!");
        //Input something by typing keyboard
        writer.typeKeys("GUI: Hello world!");
        //Set text color to green via GUI
        writer.drag(10, 10, 300, 400);
        writer.menuItem("Format->Character...").select();
        effectsPage.select();
        colorList.select("Light green");
        effectsPage.ok();
        //Verify the result via UNO API
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        Assert.assertEquals("Text Color", 0x0000FF00, xps.getPropertyValue("CharColor"));
    }
}
