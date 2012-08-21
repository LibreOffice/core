/**
 * check character style
 * 1. new a impress
 * 2. insert one line text in the first textbox
 * 3. set the font color to red
 * 4. save, close, reopen, then check the font color
 * 5. set the underline to single
 * 6. save, close, reopen, then check the underline
 * 7. set the font size to 12
 * 8. save, close, reopen, then check the font size
 * 9. set font style to Bold, Italic
 * 10. save, close, reopen, then check the font style
 */
package testcase.uno.sd;

import static org.junit.Assert.*;

import java.io.File;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import testlib.uno.SDUtil;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShapes;
import com.sun.star.frame.XStorable;

import com.sun.star.lang.XComponent;

import com.sun.star.text.XText;
import com.sun.star.uno.UnoRuntime;

/**
 * @author LouQL
 *
 */
public class CheckCharacterStyle {

    private static final UnoApp app = new UnoApp();
    private static final SDUtil SDUtil = new SDUtil();
    private XComponent m_xSDComponent = null;
    private XText xShapeText = null;
    private String filePath = null;
    private XPropertySet xtextProps = null;
    /**
     * @throws java.lang.Exception
     */
    @BeforeClass
    public static void setUpConnection() throws Exception {
        app.start();
        File temp = new File(Testspace.getPath("temp"));
        temp.mkdirs();
    }

    @AfterClass
    public static void tearDownConnection() throws Exception {
        app.close();
        //remove the temp file
        FileUtil.deleteFile(Testspace.getPath("temp"));
    }

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        filePath = Testspace.getPath("temp/CheckCharacterStyle.odt");
        if(FileUtil.fileExists(filePath))
        {   //load
            m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class,
                        app.loadDocument(filePath));
            xShapeText = getFirstTextbox();
        }
        else{
            //create a sd
            m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class, app.newDocument("simpress"));
            xShapeText = getFirstTextbox();
            xShapeText.setString("test");
        }
        xtextProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xShapeText);
    }

    private XText getFirstTextbox() throws Exception
    {
        Object firstPage = SDUtil.getPageByIndex(m_xSDComponent, 0);
        Object firstTextBox = SDUtil.getShapeOfPageByIndex(firstPage, 0);
        return (XText)UnoRuntime.queryInterface(XText.class, firstTextBox);
    }

    /**
     * @throws java.lang.Exception
     */
    @After
    public void tearDown() throws Exception {
        //close odp after each test
        m_xSDComponent.dispose();
    }

    @Test
    public void testFontColor() throws Exception{
        //set font color to red
        xtextProps.setPropertyValue("CharColor", 0xFF0000);
        app.saveDocument(m_xSDComponent, filePath);
        m_xSDComponent.dispose();
        //reopen
        m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class,
                    app.loadDocument(filePath));
        xShapeText = getFirstTextbox();
        xtextProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xShapeText);
        //check character styles
        assertEquals("character color should be red", 0xFF0000,xtextProps.getPropertyValue("CharColor"));

    }
    @Test
    public void testFontUnderline() throws Exception{
        //set font color to red
        xtextProps.setPropertyValue("CharUnderline", com.sun.star.awt.FontUnderline.SINGLE);
        app.saveDocument(m_xSDComponent, filePath);
        m_xSDComponent.dispose();
        //reopen
        m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class,
                    app.loadDocument(filePath));
        xShapeText = getFirstTextbox();
        xtextProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xShapeText);
        //check character styles
        assertEquals("character should be underlined", com.sun.star.awt.FontUnderline.SINGLE, xtextProps.getPropertyValue("CharUnderline"));
    }

    @Test
    public void testFontSize() throws Exception{
        //set font color to red
        xtextProps.setPropertyValue("CharHeight", 12);
        app.saveDocument(m_xSDComponent, filePath);
        m_xSDComponent.dispose();
        //reopen
        m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class,
                    app.loadDocument(filePath));
        xShapeText = getFirstTextbox();
        xtextProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xShapeText);
        //check character styles
        assertEquals("font size should be 12.0", "12.0", xtextProps.getPropertyValue("CharHeight").toString());
    }
    @Test
    public void testFontBoldStyle() throws Exception  {
        //change the font style to Bold
        xtextProps.setPropertyValue("CharWeight", com.sun.star.awt.FontWeight.BOLD);
        app.saveDocument(m_xSDComponent, filePath);
        m_xSDComponent.dispose();
        //reopen
        m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class,
                    app.loadDocument(filePath));
        xShapeText = getFirstTextbox();
        xtextProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xShapeText);
        assertEquals("font style should be bold", com.sun.star.awt.FontWeight.BOLD, xtextProps.getPropertyValue("CharWeight"));
    }

    @Test
    public void testFontItalicStyle() throws Exception  {
        //change the font style to Bold
        xtextProps.setPropertyValue("CharPosture", com.sun.star.awt.FontSlant.ITALIC);
        app.saveDocument(m_xSDComponent, filePath);
        m_xSDComponent.dispose();
        //reopen
        m_xSDComponent = (XComponent) UnoRuntime.queryInterface(XComponent.class,
                    app.loadDocument(filePath));
        xShapeText = getFirstTextbox();
        xtextProps = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xShapeText);
        assertEquals("font style should be bold", com.sun.star.awt.FontSlant.ITALIC, xtextProps.getPropertyValue("CharPosture"));
    }
}
