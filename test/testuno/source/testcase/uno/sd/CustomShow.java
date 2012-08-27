package testcase.uno.sd;

import junit.framework.Assert;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;
import org.openoffice.test.uno.UnoApp;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexContainer;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XSingleServiceFactory;
import com.sun.star.presentation.XCustomPresentationSupplier;
import com.sun.star.presentation.XPresentation;
import com.sun.star.presentation.XPresentationSupplier;
import com.sun.star.uno.UnoRuntime;

import testlib.uno.ShapeUtil;

public class CustomShow {
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
    public void testCustomShow() throws Exception {

        String aNameArray[] = { "Page 1", "Page 2",  "Page 3", "Page 4", "Page 5"};

        //insert five  pages
        while (drawpages.getCount() < aNameArray.length)
            drawpages.insertNewByIndex(0);

        //add text shape to each page
        for (int i = 0; i < aNameArray.length; i++) {
            XDrawPage xDrawPage = (XDrawPage) UnoRuntime.queryInterface(
                    XDrawPage.class, drawpages.getByIndex(i));
            XNamed xPageName = (XNamed) UnoRuntime.queryInterface(XNamed.class,
                    xDrawPage);
            xPageName.setName(aNameArray[i]);

            XShape xTextObj = ShapeUtil.createShape(impressDocument, new Point(
                    10000, 9000), new Size(10000, 5000),
                    "com.sun.star.drawing.TextShape");
            XShapes xShapes = (XShapes) UnoRuntime.queryInterface(
                    XShapes.class, xDrawPage);
            xShapes.add(xTextObj);
            ShapeUtil.addPortion(xTextObj, aNameArray[i], true);
        }


        //create two custom shows
        //one will play slide 3 to 5 and is named "Part"
        //the other one will play slide 1 t0 5 and is named "All"
        XCustomPresentationSupplier xCustPresSupplier = (XCustomPresentationSupplier) UnoRuntime
                .queryInterface(XCustomPresentationSupplier.class, impressDocument);


        // the following container is a container for further container which
        // includes the list of pages that are to play within a custom show
        XNameContainer xNameContainer = xCustPresSupplier
                .getCustomPresentations();
        XSingleServiceFactory xFactory = (XSingleServiceFactory) UnoRuntime
                .queryInterface(XSingleServiceFactory.class, xNameContainer);

        Object xObj;
        XIndexContainer xContainer;

        //instantiate an IndexContainer that will take a list of draw pages for
        //the first custom show
        xObj = xFactory.createInstance();
        xContainer = (XIndexContainer) UnoRuntime.queryInterface(
                XIndexContainer.class, xObj);
        for (int i = 3; i < 5; i++)
            xContainer.insertByIndex(xContainer.getCount(),
                    drawpages.getByIndex(i));
        xNameContainer.insertByName("Part", xContainer);


        //instantiate an IndexContainer that will take a list of draw page for
        //the second custom show
        xObj = xFactory.createInstance();
        xContainer = (XIndexContainer) UnoRuntime.queryInterface(
                XIndexContainer.class, xObj);
        for (int i = 1; i < 5; i++)
            xContainer.insertByIndex(xContainer.getCount(),
                    drawpages.getByIndex(i));
        xNameContainer.insertByName("All", xContainer);


        //set which custom show is to used
        XPresentationSupplier xPresSupplier = (XPresentationSupplier) UnoRuntime
                .queryInterface(XPresentationSupplier.class, impressDocument);
        XPresentation xPresentation = xPresSupplier.getPresentation();
        XPropertySet xPresPropSet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xPresentation);
        xPresPropSet.setPropertyValue("CustomShow", "Part");

        saveAndLoadSlide();

        //assert if custom show is set
        Assert.assertEquals("Set Part as CustomSHow", xPresPropSet.getPropertyValue("CustomShow"), "Part");
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
        filePath = Testspace.getPath("tmp/customshow." + sExtension);
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
