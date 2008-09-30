/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: SvxShapeCollection.java,v $
 * $Revision: 1.6.8.1 $
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
import util.DrawTools;
import util.InstCreator;
import util.SOfficeFactory;
import util.ShapeDsc;

import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class SvxShapeCollection extends TestCase {

    static XComponent xDrawDoc;

    /**
     * in general this method creates a testdocument
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *
     *  @see TestParameters
     *    @see PrintWriter
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
     *    @see PrintWriter
     *
     */
    protected void cleanup( TestParameters tParam, PrintWriter log ) {
        log.println( "    disposing xDrawDoc " );
        util.DesktopTools.closeDoc(xDrawDoc);
    }


    /**
     *    creating a Testenvironment for the interfaces to be tested
     *
     *  @param tParam    class which contains additional test parameters
     *  @param log        class to log the test state and result
     *
     *  @return    Status class
     *
     *  @see TestParameters
     *    @see PrintWriter
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
            SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());
            Object col = ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.drawing.ShapeCollection");
            XShapes shapes = (XShapes) UnoRuntime.queryInterface
                (XShapes.class,col);

            oShape = SOF.createShape(xDrawDoc,3000,4500,15000,1000,"Ellipse");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);
            shapes.add(oShape);

            oShape = SOF.createShape(xDrawDoc,5000,3500,7500,5000,"Rectangle");
            DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape) ;
            shapes.add(oShape);

            oObj = (XInterface) col ;

        }
        catch (Exception e) {
            log.println("Couldn't create insance");
            e.printStackTrace(log);
        }

        // test environment creation

         TestEnvironment tEnv = new TestEnvironment(oObj);
         System.out.println("IName: "+util.utils.getImplName(oObj));
        ShapeDsc sDsc = new ShapeDsc(5000,3500,7500,10000,"Line");
        tEnv.addObjRelation("Shape", new InstCreator(xDrawDoc, sDsc)) ;

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class SvxShapeCollection

