/*************************************************************************
 *
 *  $RCSfile: SvxShapeGroup.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:38:27 $
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
import util.InstCreator;
import util.SOfficeFactory;
import util.ShapeDsc;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapeGrouper;
import com.sun.star.drawing.XShapes;
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
 * @see com.sun.star.drawing._XDrawPage
 *
 */

public class SvxShapeGroup extends TestCase {

    XComponent xDrawDoc;

           /**
     * in general this method initializes the document
     */

       protected void initialize(TestParameters Param, PrintWriter log) {

       // get a soffice factory object
    SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

    try {
        log.println( "creating a draw document" );
        xDrawDoc = SOF.createDrawDoc(null);;
     } catch ( Exception e ) {
        // Some exception occures.FAILED
        e.printStackTrace( log );
        throw new StatusException( "Couldn't create document", e );
     }

       }

           /**
     * in general this method disposes the document
     */

       protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println("disposing xDrawDoc");
        xDrawDoc.dispose();

       }

    /**
     *    creating a Testenvironment for the interfaces to be tested
     */
    public synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log ) {

        XInterface oObj = null;
        XShapes oShapes = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        // get the drawpage of drawing here
        try {
            log.println( "getting Drawpage" );
            XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
                UnoRuntime.queryInterface(XDrawPagesSupplier.class,xDrawDoc);
            XDrawPages oDPn = oDPS.getDrawPages();
            XIndexAccess oDPi = (XIndexAccess)
                UnoRuntime.queryInterface(XIndexAccess.class,oDPn);
            oObj = (XDrawPage) AnyConverter.toObject(
                new Type(XDrawPage.class),oDPi.getByIndex(0));
        } catch ( Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't get DrawPage", e );
        }

        if (oObj == null) {
            System.out.println("**************************");
            System.out.println("    XDrawPage is NULL");
            System.out.println("**************************");
        }

        //put something on the drawpage
        log.println( "inserting some Shapes" );
        oShapes = (XShapes) UnoRuntime.queryInterface(XShapes.class,oObj);
        XShape Shape1 = SOF.createShape(xDrawDoc,
            3000,4500,15000,1000,"Ellipse");
        oShapes.add(SOF.createShape(xDrawDoc,
            2000,1500,1000,1000,"Line"));
        oShapes.add(Shape1);
        XShape Shape2 = SOF.createShape(xDrawDoc,
            5000,3500,7500,5000,"Rectangle");
        oShapes.add(Shape2);

        log.println( "adding two style as ObjRelation for ShapeDescriptor" );
        XPropertySet oShapeProps = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class,Shape1);
        XStyle aStyle1 = null;
        try {
            aStyle1 = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (Exception e) {}
        oShapeProps = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class,Shape2);
        XStyle aStyle2 = null;
        try {
            aStyle2 = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (Exception e) {}


       //get the XShapeGrouper
       try{
            log.println("get XShapeGroup");
            XShapeGrouper oSG = (XShapeGrouper)UnoRuntime.queryInterface
                (XShapeGrouper.class, oObj);
            oObj = oSG.group(oShapes);
        } catch ( Exception e) {
                e.printStackTrace( log );
                throw new StatusException(" Couldn't get XShapeGroup: ", e);
        }

        log.println( "creating a new environment for drawpage object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        ShapeDsc sDsc = new ShapeDsc(5000,3500,7500,10000,"Rectangle");
          log.println( "adding Shape as mod relation to environment" );
        tEnv.addObjRelation("Shape", new InstCreator( xDrawDoc, sDsc));
        //tEnv.addObjRelation("DrawPage", oObj);
        //tEnv.addObjRelation("MasterPageSupplier",oGroup);

        tEnv.addObjRelation("Style1",aStyle1);
        tEnv.addObjRelation("Style2",aStyle2);
        for (int i=0;i<6;i++) {
            Shape2 = SOF.createShape(xDrawDoc,
                5000+100*i,3500+100*i,7500+100*i,5000+100*i,"Rectangle");
            oShapes.add(Shape2);
        }
        return tEnv;
    } // finish method createTestEnvironment

}    // finish class SvxShapeGroup

