/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package mod._svx;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.InstCreator;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.PolyPolygonBezierCoords;
import com.sun.star.drawing.PolygonFlags;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.style.XStyle;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.drawing.ClosedBezierShape</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
 *  <li> <code>com::sun::star::drawing::PolyPolygonBezierDescriptor</code></li>
 *  <li> <code>com::sun::star::drawing::LineProperties</code></li>
 *  <li> <code>com::sun::star::drawing::FillProperties</code></li>
 *  <li> <code>com::sun::star::drawing::ShadowProperties</code></li>
 *  <li> <code>com::sun::star::drawing::XGluePointsSupplier</code></li>
 *  <li> <code>com::sun::star::style::CharacterProperties</code></li>
 *  <li> <code>com::sun::star::drawing::RotationDescriptor</code></li>
 *  <li> <code>com::sun::star::text::XTextRange</code></li>
 *  <li> <code>com::sun::star::drawing::XShape</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 *  <li> <code>com::sun::star::drawing::TextProperties</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::text::XText</code></li>
 *  <li> <code>com::sun::star::drawing::XShapeDescriptor</code></li>
 *  <li> <code>com::sun::star::text::XSimpleText</code></li>
 *  <li> <code>com::sun::star::drawing::Shape</code></li>
 * </ul> <p>
 *
 * The following files used by this test :
 * <ul>
 *  <li><b> TransparencyChart.sxs </b> : to load predefined chart
 *       document where two 'automatic' transparency styles exists :
 *       'Transparency 1' and 'Transparency 2'.</li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 *
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.drawing.PolyPolygonBezierDescriptor
 * @see com.sun.star.drawing.LineProperties
 * @see com.sun.star.drawing.FillProperties
 * @see com.sun.star.drawing.ShadowProperties
 * @see com.sun.star.drawing.XGluePointsSupplier
 * @see com.sun.star.style.CharacterProperties
 * @see com.sun.star.drawing.RotationDescriptor
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.drawing.XShape
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.drawing.TextProperties
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.text.XText
 * @see com.sun.star.drawing.XShapeDescriptor
 * @see com.sun.star.text.XSimpleText
 * @see com.sun.star.drawing.Shape
 * @see ifc.style._ParagraphProperties
 * @see ifc.drawing._PolyPolygonBezierDescriptor
 * @see ifc.drawing._LineProperties
 * @see ifc.drawing._FillProperties
 * @see ifc.drawing._ShadowProperties
 * @see ifc.drawing._XGluePointsSupplier
 * @see ifc.style._CharacterProperties
 * @see ifc.drawing._RotationDescriptor
 * @see ifc.text._XTextRange
 * @see ifc.drawing._XShape
 * @see ifc.lang._XComponent
 * @see ifc.drawing._TextProperties
 * @see ifc.beans._XPropertySet
 * @see ifc.text._XText
 * @see ifc.drawing._XShapeDescriptor
 * @see ifc.text._XSimpleText
 * @see ifc.drawing._Shape
 */
public class SvxShapePolyPolygonBezier extends TestCase {

    static XComponent xDrawDoc;

    /**
     * in general this method creates a testdocument
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *  *    @see PrintWriter
     *
     */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());
        log.println( "creating a chartdocument" );
        xDrawDoc = SOF.loadDocument(
                         utils.getFullTestURL("SvxShape.sxd"));
    }

    /**
     * in general this method disposes the testenvironment and document
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *  *    @see PrintWriter
     *
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }


    /**
     *  *    creating a TestEnvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *  *    @see PrintWriter
     */
    @Override
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        try {

            SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF());

            XMultiServiceFactory xMSF = UnoRuntime.queryInterface(XMultiServiceFactory.class, xDrawDoc) ;

            XInterface oInst = (XInterface) xMSF.createInstance
                ("com.sun.star.drawing.ClosedBezierShape") ;
            oShape = UnoRuntime.queryInterface
                (XShape.class, oInst) ;

            Point[] points = new Point[2];
            points[0] = new Point();
            points[0].X = 50;
            points[0].Y = 50;
            points[1] = new Point();
            points[1].X = 5000;
            points[1].Y = 5000;

            Point[][] the_points = new Point[1][2];
            the_points[0] = points;

            PolygonFlags[] flags = new PolygonFlags[2];
            flags[0] = PolygonFlags.NORMAL;
            flags[1] = PolygonFlags.NORMAL;

            PolygonFlags[][] the_flags = new PolygonFlags[1][2];
            the_flags[0] = flags;

            PolyPolygonBezierCoords coords = new PolyPolygonBezierCoords();
            coords.Coordinates=the_points;
            coords.Flags = the_flags;

            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

            oShape.setSize(new Size(3000,3000)) ;
            oShape.setPosition(new Point(4000,4000)) ;

            oObj = oShape ;

            XPropertySet shapeProps = UnoRuntime.queryInterface
                (XPropertySet.class, oObj);

            shapeProps.setPropertyValue("PolyPolygonBezier",coords);

            oShape = SOF.createShape(xDrawDoc,5000,3500,7500,5000,"Line");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape) ;
        }
        catch (Exception e) {
            log.println("Couldn't create instance");
            e.printStackTrace(log);
        }

        // test environment creation

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println( "adding two styles as ObjRelation for ShapeDescriptor" );
        XPropertySet oShapeProps = UnoRuntime.queryInterface(XPropertySet.class,oObj);
        XStyle aStyle = null;
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (Exception e) {
            System.out.println("caught exception: " + e);
        }
        tEnv.addObjRelation("Style1",aStyle);
        oShapeProps = UnoRuntime.queryInterface(XPropertySet.class,oShape);
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (Exception e) {
            System.out.println("caught exception: " + e);
        }
        tEnv.addObjRelation("Style2",aStyle);

        // adding relation for XText
        util.DefaultDsc tDsc = new util.DefaultDsc
            ("com.sun.star.text.XTextContent",
             "com.sun.star.text.TextField.URL");
        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xDrawDoc, tDsc ) );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SvxShapePolyPolygonBezier
