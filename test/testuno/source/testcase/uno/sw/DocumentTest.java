package testcase.uno.sw;

import static org.openoffice.test.common.Testspace.*;

import java.io.File;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.Assert;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XText;
import com.sun.star.beans.XPropertySet;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.*;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XCloseable;
import com.sun.star.lang.XComponent;
import com.sun.star.container.XEnumerationAccess;
import com.sun.star.container.XEnumeration;


public class DocumentTest {
    UnoApp unoApp = new UnoApp();
    XTextDocument textDocument = null;
    File temp = null;
    String workingFilePath = "";
    String workingTemplatePath = "";

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        unoApp.start();

        FileUtil.deleteFile(getPath("temp"));
        temp = new File(getPath("temp"));
        temp.mkdirs();

        //copy sample file to temp folder
        String originalFilePath = prepareData("testcase/uno/sw/DocumentTest.odt");
        String originalTemplatePath = prepareData("testcase/uno/sw/DocumentTest.ott");
        workingFilePath = temp + "/DocumentTest.odt";
        workingTemplatePath = temp + "/DocumentTest.ott";
        FileUtil.copyFile(new File(originalFilePath), new File(workingFilePath));
        FileUtil.copyFile(new File(originalTemplatePath), new File(workingTemplatePath));
    }

    @After
    public void tearDown() throws Exception {
        unoApp.close();
    }

    private XComponent newDocumentFromTemplate(String templatePath) throws Exception
    {
        XComponentLoader componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, unoApp.getDesktop());
        PropertyValue[] pros = new PropertyValue[1];
        pros[0] = new PropertyValue();
        pros[0].Name = "AsTemplate";
        pros[0].Value = new Boolean(true);

        XComponent component = componentLoader.loadComponentFromURL(FileUtil.getUrl(workingTemplatePath), "_blank", 0,pros);
        return component;
    }


    /**
     * test close document
     * @throws Exception
     */
    @Test
    public void testCloseDocument() throws Exception
    {
        XComponent component = unoApp.newDocument("swriter");
        unoApp.closeDocument(component);
        XModel xModel = unoApp.getDesktop().getCurrentFrame().getController().getModel();
        Assert.assertTrue("Document has been closed.",xModel==null);
    }

    /**
     * test new document
     * @throws Exception
     */
    @Test
    public void testNewDocument() throws Exception
    {
        XComponentLoader componentLoader = (XComponentLoader) UnoRuntime.queryInterface(XComponentLoader.class, unoApp.getDesktop());
        XComponent component = componentLoader.loadComponentFromURL("private:factory/" + "swriter", "_blank", 0, new PropertyValue[0]);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, component);
        XTitle xTitle = (XTitle)UnoRuntime.queryInterface(XTitle.class, textDocument);
        String title = xTitle.getTitle();
        Assert.assertEquals("New Document title start with \"Untitled\"",true, title.startsWith("Untitled"));
        unoApp.closeDocument(textDocument);
    }

    /**
     * test new document from template
     * @throws Exception
     */
    @Test
    public void testNewDocumentFromTemplate() throws Exception
    {
        XComponent component = this.newDocumentFromTemplate(workingTemplatePath);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, component);
        XText xText = textDocument.getText();
        XTitle xTitle = (XTitle)UnoRuntime.queryInterface(XTitle.class, textDocument);
        xText = textDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd(true);
        XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        String paraStyle = (String)xPropertySet.getPropertyValue("ParaStyleName");

        Assert.assertEquals("new document from template, heading style in template is remained. ", "Heading 1", paraStyle);

        Assert.assertEquals("new document from template, title start with \"Untitled\".", true, xTitle.getTitle().startsWith("Untitled"));

        unoApp.closeDocument(textDocument);
    }

    /**
     * test save document as odt
     * @throws Exception
     */
    @Test
    public void testSaveDocument() throws Exception
    {
        XComponent component = unoApp.loadDocument(workingFilePath);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, component);
        XText xText = textDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd(true);
        XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);

        xPropertySet.setPropertyValue("ParaStyleName", "Heading 1");

        XStorable xStorable = (XStorable)UnoRuntime.queryInterface(XStorable.class, component);
        xStorable.store();
        unoApp.closeDocument(textDocument);

        component = unoApp.loadDocument(workingFilePath);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, component);
        xText = textDocument.getText();
        xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd(true);
        xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);

        Assert.assertEquals("Modify plain text to heading 1 style. ", "Heading 1", (String)xPropertySet.getPropertyValue("ParaStyleName"));
        unoApp.closeDocument(textDocument);
    }

    /**
     * test save document as doc
     * @throws Exception
     */
    @Test
    public void testSaveAsDocument() throws Exception
    {
        File saveAsFile = new File(workingFilePath + ".doc");
        XComponent component = unoApp.loadDocument(workingFilePath);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, component);
        XText xText = textDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);

        xPropertySet.setPropertyValue("ParaStyleName", "Heading 1");
        xText.insertString(xTextCursor, "test Save odt as doc.", false);

        XStorable xStorable = (XStorable)UnoRuntime.queryInterface(XStorable.class, component);
        PropertyValue[] storeProps = new PropertyValue[2];

        storeProps[0] = new PropertyValue();
        storeProps[0].Name = "Overwrite";
        storeProps[0].Value = new Boolean(true);

        storeProps[1] = new PropertyValue();
        storeProps[1].Name = "FilterName";
        storeProps[1].Value = "MS Word 97";

        xStorable.storeAsURL(FileUtil.getUrl(saveAsFile), storeProps);
        Assert.assertTrue("Save odt document as doc the file exist: " + saveAsFile.getAbsolutePath(), saveAsFile.exists());
        unoApp.closeDocument(textDocument);
    }

    /**
     * test export document as pdf
     * @throws Exception
     */
    @Test
    public void testExportAsPDF() throws Exception
    {
        File saveAsFile = new File(workingFilePath + ".pdf");
        XComponent component = unoApp.loadDocument(workingFilePath);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, component);

        XStorable xStorable = (XStorable)UnoRuntime.queryInterface(XStorable.class, component);
        PropertyValue[] storeProps = new PropertyValue[3];

        storeProps[0] = new PropertyValue();
        storeProps[0].Name = "Overwrite";
        storeProps[0].Value = new Boolean(true);

        storeProps[1] = new PropertyValue();
        storeProps[1].Name = "FilterName";
        storeProps[1].Value = "writer_pdf_Export";

        storeProps[2] = new PropertyValue();
        storeProps[2].Name = "CompressionMode";
        storeProps[2].Value = "1";

        xStorable.storeToURL(FileUtil.getUrl(saveAsFile), storeProps);

        Assert.assertTrue("Export document as PDF.", saveAsFile.exists());

        unoApp.closeDocument(textDocument);
    }

    /**
     * test save document as template
     * @throws Exception
     */
    @Test
    public void testSaveAsTemplate() throws Exception
    {
        File saveAsFile = new File(workingFilePath + ".ott");
        XComponent component = unoApp.loadDocument(workingFilePath);
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, component);
        XText xText = textDocument.getText();
        XTextCursor xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd(true);
        XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);

        xPropertySet.setPropertyValue("ParaStyleName", "Heading 1");

        XStorable xStorable = (XStorable)UnoRuntime.queryInterface(XStorable.class, component);
        xStorable.store();

        PropertyValue[] storeProps = new PropertyValue[3];
        storeProps[0] = new PropertyValue();
        storeProps[0].Name="TemplateName";
        storeProps[0].Value="MyNewCreatedTemplate";

        storeProps[1] = new PropertyValue();
        storeProps[1].Name="TemplateRegionName";
        storeProps[1].Value="My Templates";

        storeProps[2] = new PropertyValue();
        storeProps[2].Name="AsTemplate";
        storeProps[2].Value=new Boolean(true);

        xStorable.storeToURL(FileUtil.getUrl(saveAsFile), storeProps);
        unoApp.closeDocument(textDocument);

        component = this.newDocumentFromTemplate(saveAsFile.getAbsolutePath());
        textDocument = (XTextDocument) UnoRuntime.queryInterface(XTextDocument.class, component);
        XTitle xTitle = (XTitle)UnoRuntime.queryInterface(XTitle.class, textDocument);
        xText = textDocument.getText();
        xTextCursor = xText.createTextCursor();
        xTextCursor.gotoEnd(true);
        xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTextCursor);
        String paraStyle = (String)xPropertySet.getPropertyValue("ParaStyleName");
        Assert.assertEquals("Save document as template, heading style is remained. ", "Heading 1", paraStyle);
        Assert.assertEquals("Save document as template, title start with \"Untitled\".", true, xTitle.getTitle().startsWith("Untitled"));
        unoApp.closeDocument(textDocument);
    }

}
