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
import util.DefaultDsc;
import util.DrawTools;
import util.InstCreator;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.style.XStyle;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.drawing.EllipseShape</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
 *  <li> <code>com::sun::star::drawing::LineProperties</code></li>
 *  <li> <code>com::sun::star::drawing::EllipseShape</code></li>
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
 * The following files used by this test :
 * <ul>
 *  <li><b> SvxShape.sxd </b> : this document is loaded for
 *  adding a shape tested to it. </li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurrently.
 *
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.drawing.LineProperties
 * @see com.sun.star.drawing.EllipseShape
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
 * @see ifc.drawing._LineProperties
 * @see ifc.drawing._EllipseShape
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
public class SvxShapeCircle extends TestCase {

    static XComponent xDrawDoc;

    /**
     * Loads a Draw document with name 'SvxShape.sxd' from test
     * documents directory
     */
    @Override
    protected void initialize( TestParameters tParam, PrintWriter log ) throws Exception {
        SOfficeFactory SOF = SOfficeFactory.getFactory( tParam.getMSF() );
        log.println( "creating a drawdoc" );
        xDrawDoc = SOF.loadDocument(
                         utils.getFullTestURL("SvxShape.sxd"));
    }

    /**
     * Disposes the Draw document loaded before.
     */
    @Override
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }


    /**
     * Creating a TestEnvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.drawing.EllipseShape</code> as tested component
     * and adds it to the document. Then the FULL type of circle is set.
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'Style1', 'Style2'</code> for
     *      {@link ifc.drawing._Shape} :
     *       two values of 'Style' property. The first is taken
     *       from the shape tested, the second from another
     *       shape added to the draw page. </li>
     *  <li> <code>'XTEXTINFO'</code> for
     *      {@link ifc.text._XText} :
     *      creator which can create instances of
     *      <code>com.sun.star.text.TextField.URL</code>
     *      service. </li>
     * </ul>
     */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) throws Exception {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        SOfficeFactory SOF = SOfficeFactory.getFactory(tParam.getMSF()) ;
        oShape = SOF.createShape(xDrawDoc,5000,5000,3000,3000,"Ellipse");

        XPropertySet props = UnoRuntime.queryInterface
            (XPropertySet.class, oShape) ;

        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape) ;
        props.setPropertyValue("CircleKind",
            com.sun.star.drawing.CircleKind.FULL) ;

        oObj = oShape ;

        oShape = SOF.createShape(xDrawDoc,5000,3500,7500,5000,"Line");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape) ;

        // test environment creation

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println( "adding two styles as ObjRelation for ShapeDescriptor" );
        XPropertySet oShapeProps = UnoRuntime.queryInterface(XPropertySet.class,oObj);
        XStyle aStyle = null;

        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }

        tEnv.addObjRelation("Style1",aStyle);
        oShapeProps = UnoRuntime.queryInterface(XPropertySet.class,oShape);
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }

        tEnv.addObjRelation("Style2",aStyle);

        DefaultDsc tDsc = new DefaultDsc("com.sun.star.text.XTextContent",
                                            "com.sun.star.text.TextField.URL");
        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xDrawDoc, tDsc ) );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SvxShapeCircle
