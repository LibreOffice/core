/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SvxShapeConnector.java,v $
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
 *
 * initial description
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.document.LinkTarget
 * @see com.sun.star.document.XLinkTargetSupplier
 * @see com.sun.star.drawing.ConnectorShapeDescriptor
 * @see com.sun.star.drawing.LineShapeDescriptor
 * @see com.sun.star.drawing.PolyPolygonDescriptor
 * @see com.sun.star.drawing.RotationDescriptor
 * @see com.sun.star.drawing.ShadowDescriptor
 * @see com.sun.star.drawing.ShapeDescriptor
 * @see com.sun.star.drawing.Text
 * @see com.sun.star.drawing.XConnectorShape
 * @see com.sun.star.drawing.XShape
 * @see com.sun.star.drawing.XShapeDescriptor
 * @see com.sun.star.lang.XComponent
 * @see com.sun.star.style.CharacterProperties
 * @see com.sun.star.style.ParagraphProperties
 * @see com.sun.star.text.XSimpleText
 * @see com.sun.star.text.XText
 * @see com.sun.star.text.XTextRange
 *
 */
public class SvxShapeConnector extends TestCase {

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
    protected void initialize( TestParameters tParam, PrintWriter log ) {

        try {
            log.println( "creating a drawdoc" );
            xDrawDoc = DrawTools.createDrawDoc((XMultiServiceFactory)tParam.getMSF());
        } catch ( Exception e ) {
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
        util.DesktopTools.closeDoc(xDrawDoc);
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
            XMultiServiceFactory oDocMSF = (XMultiServiceFactory)
                UnoRuntime.queryInterface(XMultiServiceFactory.class,xDrawDoc);
            Object oInt = oDocMSF.createInstance
                ( "com.sun.star.drawing.ConnectorShape" );

            oShape = (XShape)UnoRuntime.queryInterface( XShape.class, oInt );
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);
        }
        catch (Exception e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
        }

        // create testobject here
        oObj = oShape;

        TestEnvironment tEnv = new TestEnvironment( oObj );

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());
        oShape = SOF.createShape(xDrawDoc,3000,4500,15000,1000,"Ellipse");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

        for (int i=0;i<10;i++) {
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(
                SOF.createShape(xDrawDoc,
                3000,4500,7510+10*i,5010+10*i,"Rectangle"));
        }

        log.println( "adding two style as ObjRelation for ShapeDescriptor" );
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

        DefaultDsc tDsc = new DefaultDsc("com.sun.star.text.XTextContent",
                                            "com.sun.star.text.TextField.URL");
        log.println( "    adding InstCreator object" );
        tEnv.addObjRelation( "XTEXTINFO", new InstCreator( xDrawDoc, tDsc ) );
        tEnv.addObjRelation("NoSetSize","SvxShapeConnector");
        // adding relation for XConnectorShape
        XShape oShape1 = SOF.createShape(xDrawDoc,1000,2000,1500,1000,"Line");
        XShape oShape2 = SOF.createShape(xDrawDoc,1000,2000,4000,3000,"Line");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape1);
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape2);
        tEnv.addObjRelation("XConnectorShape.Shapes",
            new XShape[] {oShape1, oShape2}) ;
        System.out.println("IName: "+util.utils.getImplName(oObj));
        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SvxShapeConnector

