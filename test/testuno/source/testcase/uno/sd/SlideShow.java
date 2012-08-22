package testcase.uno.sd;

import junit.framework.Assert;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import testlib.uno.PageUtil;
import testlib.uno.ShapeUtil;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.presentation.XPresentation;
import com.sun.star.presentation.XPresentationSupplier;
import com.sun.star.uno.UnoRuntime;

public class SlideShow {
    XPresentationSupplier sdDocument = null;
    XPresentation pre = null;
    XComponent precomp = null;
    XComponent impressDocument = null;
    XComponent reLoadFile = null;
    XDrawPagesSupplier drawsupplier = null;
    XDrawPages drawpages = null;

    String filePath = null;

    UnoApp unoApp = new UnoApp();

    /**
     * @throws java.lang.Exception
     */
    @Before
    public void setUp() throws Exception {
        unoApp.start();
        createDocumentAndSlide();
    }

    @After
    public void tearDown() throws Exception {
        unoApp.closeDocument(impressDocument);
        unoApp.closeDocument(reLoadFile);
        unoApp.close();
    }

    @Test
    public void testSlideShow() throws Exception {
        Point po = new Point(5000, 5000);

        XDrawPage xPage1 = createSlide(0);
        XShapes xShapes1 = (XShapes) UnoRuntime.queryInterface(XShapes.class,
                xPage1);
        XShape xRectangle1 = ShapeUtil.createShape(impressDocument, po,
                new Size(21000, 12500), "com.sun.star.drawing.RectangleShape");
        xShapes1.add(xRectangle1);
        ShapeUtil.addPortion(xRectangle1, "Page1", false);

        XDrawPage xPage2 = createSlide(1);
        XShapes xShapes2 = (XShapes) UnoRuntime.queryInterface(XShapes.class,
                xPage2);
        XShape xRectangle2 = ShapeUtil.createShape(impressDocument, po,
                new Size(21000, 12500), "com.sun.star.drawing.RectangleShape");
        xShapes2.add(xRectangle2);
        ShapeUtil.addPortion(xRectangle2, "Page2", false);

        XDrawPage xPage3 = createSlide(2);
        XShapes xShapes3 = (XShapes) UnoRuntime.queryInterface(XShapes.class,
                xPage3);
        XShape xRectangle3 = ShapeUtil.createShape(impressDocument, po,
                new Size(21000, 12500), "com.sun.star.drawing.RectangleShape");
        xShapes3.add(xRectangle3);
        ShapeUtil.addPortion(xRectangle3, "Page3", false);

        XPropertySet xPresPropSet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, pre);
        xPresPropSet.setPropertyValue("IsEndless", Boolean.FALSE);
        xPresPropSet.setPropertyValue("IsFullScreen", Boolean.TRUE);
        xPresPropSet.setPropertyValue("Pause", new Integer(0));

        saveAndLoadSlide();

        Assert.assertEquals("IsEndless set to false",
                xPresPropSet.getPropertyValue("IsEndless"), Boolean.FALSE);
        Assert.assertEquals("IsFullScreen set to true",
                xPresPropSet.getPropertyValue("IsFullScreen"), Boolean.TRUE);
        Assert.assertEquals("Pause set to 0",
                xPresPropSet.getPropertyValue("Pause"), 0);
    }

    public XDrawPage createSlide(int index) throws Exception {
        drawpages.insertNewByIndex(index);
        XDrawPage xPage = PageUtil.getDrawPageByIndex(impressDocument, index);
        return xPage;
    }

    /**
     * create a new presentation document and insert a new slide.
     *
     * @throws Exception
     */
    public void createDocumentAndSlide() throws Exception {
        impressDocument = (XComponent) UnoRuntime.queryInterface(
                XComponent.class, unoApp.newDocument("simpress"));
        drawsupplier = (XDrawPagesSupplier) UnoRuntime.queryInterface(
                XDrawPagesSupplier.class, impressDocument);
        drawpages = drawsupplier.getDrawPages();
        sdDocument = (XPresentationSupplier) UnoRuntime.queryInterface(
                XPresentationSupplier.class, impressDocument);
        pre = sdDocument.getPresentation();
    }

    /**
     * Save presentation and reLoad the slide.
     *
     * @param no
     * @return void
     * @throws Exception
     */
    public void saveAndLoadSlide() throws Exception {
        reLoadFile = saveAndReloadDoc(impressDocument,
                "StarOffice XML (Impress)", "odp");
        drawsupplier = (XDrawPagesSupplier) UnoRuntime.queryInterface(
                XDrawPagesSupplier.class, reLoadFile);
        drawpages = drawsupplier.getDrawPages();

        sdDocument = (XPresentationSupplier) UnoRuntime.queryInterface(
                XPresentationSupplier.class, reLoadFile);
        pre = sdDocument.getPresentation();
    }

    /**
     * save and reload Presentation document.
     *
     * @param presentationDocument
     * @param sFilter
     * @param sExtension
     * @return
     * @throws Exception
     */
    private XComponent saveAndReloadDoc(XComponent presentationDocument,
            String sFilter, String sExtension) throws Exception {
        filePath = Testspace.getPath("tmp/slideshow." + sExtension);
        PropertyValue[] aStoreProperties = new PropertyValue[2];
        aStoreProperties[0] = new PropertyValue();
        aStoreProperties[1] = new PropertyValue();
        aStoreProperties[0].Name = "Override";
        aStoreProperties[0].Value = true;
        aStoreProperties[1].Name = "FilterName";
        aStoreProperties[1].Value = sFilter;
        XStorable xStorable = (XStorable) UnoRuntime.queryInterface(
                XStorable.class, presentationDocument);
        xStorable.storeToURL(FileUtil.getUrl(filePath), aStoreProperties);

        return UnoRuntime.queryInterface(XComponent.class,
                unoApp.loadDocument(filePath));
    }
}
