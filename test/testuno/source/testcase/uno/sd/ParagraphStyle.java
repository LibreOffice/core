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
import com.sun.star.drawing.TextFitToSizeType;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.presentation.XPresentation;
import com.sun.star.presentation.XPresentationSupplier;
import com.sun.star.style.LineSpacing;
import com.sun.star.style.LineSpacingMode;
import com.sun.star.style.ParagraphAdjust;
import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextRange;
import com.sun.star.uno.UnoRuntime;

public class ParagraphStyle {
    XPresentationSupplier sdDocument = null;
    XPresentation pre = null;
    XComponent precomp = null;
    XComponent impressDocument = null;
    XComponent reLoadFile = null;
    XDrawPagesSupplier drawsupplier = null;
    XDrawPages drawpages = null;
    XShapes xShapes = null;
    XDrawPage xpage = null;
    String filePath=null;

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
    public void ParaStyle() throws Exception {
        Point po = new Point(5000, 5000);
        xShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, xpage);
        // create the shape
        XShape xRectangle = ShapeUtil.createShape(impressDocument, po, new Size(21000, 12500), "com.sun.star.drawing.RectangleShape");
         xShapes.add(xRectangle);
         XPropertySet xShapePropSet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xRectangle);
         // TextFitToSize
         xShapePropSet.setPropertyValue("TextFitToSize", TextFitToSizeType.PROPORTIONAL);

         XPropertySet xTextPropSet1 = addPortion(xRectangle, "New text paragraph", true);
         xTextPropSet1.setPropertyValue("ParaAdjust", ParagraphAdjust.CENTER);

         //Line Spacing
         LineSpacing xLineSpacing = new LineSpacing(LineSpacingMode.LEADING, (short)1);
         xTextPropSet1.setPropertyValue("ParaLineSpacing",  xLineSpacing);

         //left, right, top and bottom margin
         xTextPropSet1.setPropertyValue("ParaLeftMargin",  1000);
         xTextPropSet1.setPropertyValue("ParaRightMargin",  1000);
         xTextPropSet1.setPropertyValue("ParaTopMargin",  1000);
         xTextPropSet1.setPropertyValue("ParaBottomMargin",  1000);

         XPropertySet xTextPropSet2 = addPortion(xRectangle, "And another text paragraph", true);
         xTextPropSet2.setPropertyValue("CharColor", new Integer(0xff0000));

         xRectangle = saveAndLoadShape(po, "com.sun.star.drawing.EllipseShape");


         Assert.assertEquals("Paragraph Left Margin is 1000",1000, xTextPropSet1.getPropertyValue("ParaLeftMargin"));
         Assert.assertEquals("Paragraph Right Margin is 1000", 1000,xTextPropSet1.getPropertyValue("ParaRightMargin"));
         Assert.assertEquals("Paragraph Top Margin is 1000",1000, xTextPropSet1.getPropertyValue("ParaTopMargin") );
         Assert.assertEquals("Paragraph Bottom Margin is 1000 ",1000, xTextPropSet1.getPropertyValue("ParaBottomMargin"));
         Assert.assertEquals("Text Color is red",0xff0000,xTextPropSet2.getPropertyValue("CharColor"));

    }


    public static XShape createShape(XComponent xComponent, int x, int y,
            int width, int height, String sShapeType)
            throws java.lang.Exception {
        // query the document for the document-internal service factory
        XMultiServiceFactory xFactory = (XMultiServiceFactory) UnoRuntime
                .queryInterface(XMultiServiceFactory.class, xComponent);

        // get the given Shape service from the factory
        Object xObj = xFactory.createInstance(sShapeType);
        Point aPos = new Point(x, y);
        Size aSize = new Size(width, height);

        // use its XShape interface to determine position and size before
        // insertion
        XShape xShape = (XShape) UnoRuntime.queryInterface(XShape.class, xObj);
        xShape.setPosition(aPos);
        xShape.setSize(aSize);
        return xShape;
    }

    public static XPropertySet addPortion(XShape xShape, String sText, boolean bNewParagraph)
             throws com.sun.star.lang.IllegalArgumentException {
         XText xText = (XText)UnoRuntime.queryInterface(XText.class, xShape);
         XTextCursor xTextCursor = xText.createTextCursor();
         xTextCursor.gotoEnd(false);
         if (bNewParagraph) {
             xText.insertControlCharacter(xTextCursor, ControlCharacter.PARAGRAPH_BREAK, false);
             xTextCursor.gotoEnd(false);
         }
         XTextRange xTextRange = (XTextRange)UnoRuntime.queryInterface(XTextRange.class, xTextCursor);
         xTextRange.setString(sText);
         xTextCursor.gotoEnd(true);
         XPropertySet xPropSet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, xTextRange);
         return xPropSet;
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
        drawpages.insertNewByIndex(1);
        xpage = PageUtil.getDrawPageByIndex(impressDocument, 1);
    }

    /**
     * Save presentation and reLoad the presentation and shape in it.
     *
     * @param po
     * @param shapeType
     * @return
     * @throws Exception
     */
    public XShape saveAndLoadShape(Point po, String shapeType) throws Exception {
        reLoadFile = saveAndReloadDoc(impressDocument,
                "StarOffice XML (Impress)", "odp");
        drawsupplier = (XDrawPagesSupplier) UnoRuntime.queryInterface(
                XDrawPagesSupplier.class, reLoadFile);
        drawpages = drawsupplier.getDrawPages();
        xpage = PageUtil.getDrawPageByIndex(impressDocument, 1);
        xShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, xpage);
        XShape shap = ShapeUtil.getShape(impressDocument, po,
                "com.sun.star.drawing.EllipseShape");
        return shap;
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
        filePath = Testspace.getPath("tmp/paragraphstyle."
                + sExtension);
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
