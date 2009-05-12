/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SvxGraphicObject.java,v $
 * $Revision: 1.8.8.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

package mod._svx;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DefaultDsc;
import util.DrawTools;
import util.InstCreator;
import util.SOfficeFactory;

import com.sun.star.awt.XBitmap;
import com.sun.star.beans.XPropertySet;
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
 * <code>com.sun.star.drawing.GraphicObject</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::drawing::ShadowProperties</code></li>
 *  <li> <code>com::sun::star::drawing::RotationDescriptor</code></li>
 *  <li> <code>com::sun::star::text::XSimpleText</code></li>
 *  <li> <code>com::sun::star::drawing::XShape</code></li>
 *  <li> <code>com::sun::star::drawing::GraphicObjectShape</code></li>
 *  <li> <code>com::sun::star::drawing::Shape</code></li>
 *  <li> <code>com::sun::star::text::XTextRange</code></li>
 *  <li> <code>com::sun::star::drawing::XGluePointsSupplier</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::drawing::TextProperties</code></li>
 *  <li> <code>com::sun::star::style::CharacterProperties</code></li>
 *  <li> <code>com::sun::star::drawing::XShapeDescriptor</code></li>
 *  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
 *  <li> <code>com::sun::star::text::XText</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.drawing.ShadowProperties
 * @see com.sun.star.drawing.RotationDescriptor
 * @see com.sun.star.text.XSimpleText
 * @see com.sun.star.drawing.XShape
 * @see com.sun.star.drawing.GraphicObjectShape
 * @see com.sun.star.drawing.Shape
 * @see com.sun.star.text.XTextRange
 * @see com.sun.star.drawing.XGluePointsSupplier
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.drawing.TextProperties
 * @see com.sun.star.style.CharacterProperties
 * @see com.sun.star.drawing.XShapeDescriptor
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.text.XText
 * @see com.sun.star.lang.XComponent
 * @see ifc.drawing._ShadowProperties
 * @see ifc.drawing._RotationDescriptor
 * @see ifc.text._XSimpleText
 * @see ifc.drawing._XShape
 * @see ifc.drawing._GraphicObjectShape
 * @see ifc.drawing._Shape
 * @see ifc.text._XTextRange
 * @see ifc.drawing._XGluePointsSupplier
 * @see ifc.beans._XPropertySet
 * @see ifc.drawing._TextProperties
 * @see ifc.style._CharacterProperties
 * @see ifc.drawing._XShapeDescriptor
 * @see ifc.style._ParagraphProperties
 * @see ifc.text._XText
 * @see ifc.lang._XComponent
 */
public class SvxGraphicObject extends TestCase {

    static XComponent xDrawDoc;

    /**
     * Creates a new Draw document.
     */
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        log.println( "creating a drawdoc" );
        xDrawDoc = DrawTools.createDrawDoc((XMultiServiceFactory)tParam.getMSF());
    }

    /**
     * Disposes the Draw document created before
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.drawing.GraphicObject</code> as a tested
     * component and inserts it to one of draw pages of the document.
     * <p>
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'Style1', 'Style2'</code> for
     *      {@link ifc.drawing._Shape} :
     *       two values of 'Style' property. The first is taken
     *       from the shape tested, the second from another
     *       shape added to the draw page. </li>
     *  <li> <code>'Bitmap1', 'Bitmap2'</code> for
     *      {@link ifc.drawing._GraphicObjectShape} :</li>
     *      two bitmap objects which are created by setting
     *      'GraphicURL' property of component tested and
     *      getting its 'GraphicObjectFillBitmap' property.
     *  <li> <code>'IMAP'</code> for
     *      {@link ifc.drawing._GraphicObjectShape} :
     *      <code>com.sun.star.image.ImageMapRectangleObject</code>
     *      service instance. </li>
     *  <li> <code>'XTEXTINFO'</code> for
     *      {@link ifc.text._XText} :
     *      creator which can create instnaces of
     *      <code>com.sun.star.text.TextField.URL</code>
     *      service. </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null;
        Object imap=null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        // create testobject here
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());
        oShape = SOF.createShape(xDrawDoc,5000,5000,1500,1000,"GraphicObject");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);
        oObj = oShape;

        TestEnvironment tEnv = new TestEnvironment( oShape );

        // adding two styles as ObjRelation for Shape
        oShape = SOF.createShape(xDrawDoc,3000,4500,15000,1000,"Ellipse");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

        XPropertySet oShapeProps = (XPropertySet)
                            UnoRuntime.queryInterface(XPropertySet.class,oObj);
        XStyle aStyle = null;
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
        tEnv.addObjRelation("Style1",aStyle);

        oShapeProps = (XPropertySet)
                            UnoRuntime.queryInterface(XPropertySet.class,oShape);
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        }
        tEnv.addObjRelation("Style2",aStyle);


        // adding graphic as ObjRelation for GraphicObjectShape
        oShapeProps = (XPropertySet)
                            UnoRuntime.queryInterface(XPropertySet.class,oObj);
        XBitmap aBitmap = null;
        try {
            oShapeProps.setPropertyValue(
                "GraphicURL",util.utils.getFullTestURL("space-metal.jpg"));
            aBitmap = (XBitmap) AnyConverter.toObject(
                new Type(XBitmap.class),oShapeProps.getPropertyValue
                    ("GraphicObjectFillBitmap"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        }


        tEnv.addObjRelation("Bitmap1",aBitmap);

        try {
             oShapeProps.setPropertyValue(
                "GraphicURL",util.utils.getFullTestURL("crazy-blue.jpg"));
            aBitmap = (XBitmap) AnyConverter.toObject(
                new Type(XBitmap.class),oShapeProps.getPropertyValue
                    ("GraphicObjectFillBitmap"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        }

        tEnv.addObjRelation("Bitmap2",aBitmap);

        try {
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class,xDrawDoc);
            imap = oDocMSF.createInstance
                ( "com.sun.star.image.ImageMapRectangleObject" );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create instance");
            e.printStackTrace(log);
        }


        tEnv.addObjRelation("IMAP",imap);
        tEnv.addObjRelation("ImapObject",imap);

        // adding relation for XText interface
        DefaultDsc tDsc = new DefaultDsc("com.sun.star.text.XTextContent",
                                            "com.sun.star.text.TextField.URL");
        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xDrawDoc, tDsc ) );

        tEnv.addObjRelation("NoShear",new Boolean(true));

        return tEnv;

    } // finish method getTestEnvironment

}    // finish class SvxGraphicObject
