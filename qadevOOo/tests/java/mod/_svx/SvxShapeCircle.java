/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SvxShapeCircle.java,v $
 * $Revision: 1.7.8.1 $
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

import lib.StatusException;
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
import com.sun.star.lang.XMultiServiceFactory;
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
 * threads concurently.
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
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a drawdoc" );
            xDrawDoc = SOF.loadDocument(
                             utils.getFullTestURL("SvxShape.sxd"));
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    /**
     * Disposes the Draw document loaded before.
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }


    /**
     * Creating a Testenvironment for the interfaces to be tested.
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
     *      creator which can create instnaces of
     *      <code>com.sun.star.text.TextField.URL</code>
     *      service. </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory)tParam.getMSF()) ;
        oShape = SOF.createShape(xDrawDoc,5000,5000,3000,3000,"Ellipse");

        XPropertySet props = (XPropertySet) UnoRuntime.queryInterface
            (XPropertySet.class, oShape) ;

        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape) ;
        try {
            props.setPropertyValue("CircleKind",
                com.sun.star.drawing.CircleKind.FULL) ;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Cann't set 'CircleKind' property : " + e);
            throw new StatusException("Can't create component", e) ;
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Cann't set 'CircleKind' property : " + e);
            throw new StatusException("Can't create component", e) ;
        } catch (com.sun.star.beans.PropertyVetoException e) {
            log.println("Cann't set 'CircleKind' property : " + e);
            throw new StatusException("Can't create component", e) ;
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            log.println("Cann't set 'CircleKind' property : " + e);
            throw new StatusException("Can't create component", e) ;
        }


        oObj = oShape ;

        oShape = SOF.createShape(xDrawDoc,5000,3500,7500,5000,"Line");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape) ;

        // test environment creation

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println( "adding two styles as ObjRelation for ShapeDescriptor" );
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

        DefaultDsc tDsc = new DefaultDsc("com.sun.star.text.XTextContent",
                                            "com.sun.star.text.TextField.URL");
        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xDrawDoc, tDsc ) );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SvxShapeCircle
