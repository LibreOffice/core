/*************************************************************************
 *
 *  $RCSfile: SvxShapePolyPolygon.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:38:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
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
 * <code>com.sun.star.drawing.PolyPolygonShape</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::style::ParagraphProperties</code></li>
 *  <li> <code>com::sun::star::drawing::LineProperties</code></li>
 *  <li> <code>com::sun::star::drawing::FillProperties</code></li>
 *  <li> <code>com::sun::star::drawing::PolyPolygonDescriptor</code></li>
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
 *  <li><b> TransparencyChart.sxs </b> : to load predefined chart
 *       document where two 'automatic' transparency styles exists :
 *       'Transparency 1' and 'Transparency 2'.</li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.drawing.LineProperties
 * @see com.sun.star.drawing.FillProperties
 * @see com.sun.star.drawing.PolyPolygonDescriptor
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
 * @see ifc.drawing._FillProperties
 * @see ifc.drawing._PolyPolygonDescriptor
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
public class SvxShapePolyPolygon extends TestCase {

    XComponent xDrawDoc;

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
    protected void initialize( TestParameters tParam, PrintWriter log ) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a chartdocument" );
            xDrawDoc = SOF.loadDocument(
                             utils.getFullTestURL("SvxShape.sxd"));
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
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
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        xDrawDoc.dispose();
    }


    /**
     *  *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *  *    @see PrintWriter
     */
    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        try {
            // adding some shapes for testing.
            //SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());
            //oShape = SOF.createShape(xDrawDoc,6000,6000,5000,5000,"PolyPolygon");
            XMultiServiceFactory xMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class, xDrawDoc) ;
            XInterface oInst = (XInterface) xMSF.createInstance
                ("com.sun.star.drawing.PolyPolygonShape") ;
            oShape = (XShape) UnoRuntime.queryInterface
                (XShape.class, oInst) ;

            oShape.setSize(new Size(0,0)) ;
            oShape.setPosition(new Point(0,0)) ;

            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

            Point[] square1 = new Point[] {
                new Point(5000, 5000),
                new Point(10000, 5000),
                new Point(10000, 10000),
                new Point(5000, 10000)} ;

            Point[] square2 = new Point[] {
                new Point(6500, 6500),
                new Point(8500, 6500),
                new Point(8500, 8500),
                new Point(6500, 8500)} ;

            Point[][] polygon = new Point[][] {square1, square2};

            ((XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,
                oShape)).setPropertyValue("PolyPolygon", polygon) ;

            oObj = oShape ;

            SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory)tParam.getMSF()) ;
            oShape = SOF.createShape(xDrawDoc,5000,3500,7500,5000,"Line");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape) ;
        }
        catch (Exception e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
        }

        // test environment creation

        TestEnvironment tEnv = new TestEnvironment(oObj);

        log.println( "adding two styles as ObjRelation for ShapeDescriptor" );
        XPropertySet oShapeProps = (XPropertySet)
                            UnoRuntime.queryInterface(XPropertySet.class,oObj);
        XStyle aStyle = null;
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (Exception e) {}
        tEnv.addObjRelation("Style1",aStyle);
        oShapeProps = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class,oShape);
        try {
            aStyle = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (Exception e) {}
        tEnv.addObjRelation("Style2",aStyle);

        // adding relation for XText
        DefaultDsc tDsc = new DefaultDsc("com.sun.star.text.XTextContent",
                                            "com.sun.star.text.TextField.URL");
        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xDrawDoc, tDsc ) );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SvxShapeCollection

