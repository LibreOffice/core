/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/
package testcase.uno.sd;

import java.io.File;
import java.io.IOException;

import org.junit.After;
import static org.junit.Assert.*;

import org.junit.AfterClass;
import org.junit.Before;
import org.junit.Test;
import org.openoffice.test.uno.UnoApp;
import org.openoffice.test.common.FileUtil;
import org.openoffice.test.common.Testspace;

import testlib.uno.PageUtil;
import testlib.uno.ShapeUtil;

import com.sun.star.uno.UnoRuntime;
import com.sun.star.presentation.XPresentation;
import com.sun.star.presentation.XPresentationSupplier;
import com.sun.star.awt.Gradient;
import com.sun.star.awt.GradientStyle;
import com.sun.star.awt.Size;
import com.sun.star.awt.Point;
import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.FillStyle;
import com.sun.star.drawing.Hatch;
import com.sun.star.drawing.HatchStyle;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;

public class ShapeProperties {
    UnoApp unoApp = new UnoApp();
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
        if(filePath!=null)
            FileUtil.deleteFile(filePath);
    }

    /**
     * test Insert a new slide and Insert a new EllipseShape
     *
     * @throws Exception
     */
    @Test
    public void testInsertShape() throws Exception {
        Point po = new Point(1000, 8000);
        xShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, xpage);
        XShape xShape = ShapeUtil.createShape(impressDocument, po, new Size(
                5000, 5000), "com.sun.star.drawing.EllipseShape");
        xShapes.add(xShape);
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xShape);
        xPropSet.setPropertyValue("Name", "test");
        XShape shap = ShapeUtil.getShape(impressDocument, po,
                "com.sun.star.drawing.EllipseShape");
        XPropertySet xPropSet2 = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xShape);
        assertEquals("Not the same shape","test",xPropSet2.getPropertyValue("Name"));
        assertEquals("Not EllopseShape","com.sun.star.drawing.EllipseShape",shap.getShapeType());
    }

    /**
     * test Insert text to an EllopseShape
     *
     * @throws Exception
     */
    @Test
    public void testInsertTextToShape() throws Exception {
        Point po = new Point(1000, 8000);
        xShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, xpage);
        XShape xShape = ShapeUtil.createShape(impressDocument, po, new Size(
                5000, 5000), "com.sun.star.drawing.EllipseShape");
        xShapes.add(xShape);
        ShapeUtil.addPortion(xShape, "test", false);
        assertEquals("Not put text correctly","test",ShapeUtil.getPortion(xShape));
    }

    /**
     * test Shape fill with Gradient
     *
     * @throws Exception
     */
    @Test
    public void testShapeFillGradient() throws Exception {
        Point po = new Point(1000, 8000);
        xShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, xpage);
        XShape xShape = ShapeUtil.createShape(impressDocument, po, new Size(
                5000, 5000), "com.sun.star.drawing.EllipseShape");
        xShapes.add(xShape);
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xShape);
        xPropSet.setPropertyValue("FillStyle", FillStyle.GRADIENT);
        Gradient aGradient = new Gradient();
        aGradient.Style = GradientStyle.LINEAR;
        aGradient.StartColor = 0x00ff00;
        aGradient.EndColor = 0xffff00;
        aGradient.Angle = 450;
        aGradient.Border = 0;
        aGradient.XOffset = 0;
        aGradient.YOffset = 0;
        aGradient.StartIntensity = 100;
        aGradient.EndIntensity = 100;
        aGradient.StepCount = 10;
        xPropSet.setPropertyValue("FillGradient", aGradient);
        // --------------------------
        xShape = saveAndLoadShape(po, "com.sun.star.drawing.EllipseShape");
        // ----------------------------
        assertEquals("Not Gradient Fill Style",FillStyle.GRADIENT,xPropSet.getPropertyValue("FillStyle"));
        aGradient=(Gradient) xPropSet.getPropertyValue("FillGradient");
        assertEquals("Not Linear Gradient",GradientStyle.LINEAR,aGradient.Style);
    }

    /**
     * test Shape fill with yellow color
     *
     * @throws Exception
     */
    @Test
    public void testShapeFillColor() throws Exception {
        Point po = new Point(1000, 8000);
        xShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, xpage);
        XShape xShape = ShapeUtil.createShape(impressDocument, po, new Size(
                5000, 5000), "com.sun.star.drawing.EllipseShape");
        xShapes.add(xShape);
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xShape);
        xPropSet.setPropertyValue("FillStyle", FillStyle.SOLID);
        xPropSet.setPropertyValue("FillColor", 0xffff00);
        // --------------------------
        xShape = saveAndLoadShape(po, "com.sun.star.drawing.EllipseShape");
        // ----------------------------------------------------
        assertEquals("Not Color Fill Style",FillStyle.SOLID,xPropSet.getPropertyValue("FillStyle"));
        assertEquals("Not Yellow Color Fill",0xffff00,xPropSet.getPropertyValue("FillColor"));
    }

    /**
     * test shape fill with Hatch Style(Pattern in MS)
     * @throws Exception
     */
    @Test
    public void testShapeFillHatch() throws Exception {
        Point po = new Point(1000, 8000);
        xShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class, xpage);
        XShape xShape = ShapeUtil.createShape(impressDocument, po, new Size(
                5000, 5000), "com.sun.star.drawing.EllipseShape");
        xShapes.add(xShape);
        XPropertySet xPropSet = (XPropertySet) UnoRuntime.queryInterface(
                XPropertySet.class, xShape);
        xPropSet.setPropertyValue("FillStyle", FillStyle.HATCH);
        Hatch aHatch=new Hatch();
        aHatch.Style=HatchStyle.DOUBLE;
        aHatch.Color=0x00ff00;
        aHatch.Distance=100;
        aHatch.Angle=450;
        xPropSet.setPropertyValue("FillHatch", aHatch);

        // --------------------------
        xShape = saveAndLoadShape(po, "com.sun.star.drawing.EllipseShape");
        // ----------------------------
        assertEquals("Not Gradient Fill Style",FillStyle.HATCH,xPropSet.getPropertyValue("FillStyle"));

        aHatch=(Hatch) xPropSet.getPropertyValue("FillHatch");
        assertEquals("Not Double Hatch",HatchStyle.DOUBLE,aHatch.Style);
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
        filePath = Testspace.getPath("tmp/presentationtest."
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
