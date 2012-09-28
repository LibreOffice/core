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

    static XComponent xDrawDoc;

           /**
     * in general this method initializes the document
     */

       protected void initialize(TestParameters Param, PrintWriter log) {

       // get a soffice factory object
    SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

    try {
        log.println( "creating a draw document" );
        xDrawDoc = SOF.createDrawDoc(null);
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
        util.DesktopTools.closeDoc(xDrawDoc);

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
            XDrawPagesSupplier oDPS = UnoRuntime.queryInterface(XDrawPagesSupplier.class,xDrawDoc);
            XDrawPages oDPn = oDPS.getDrawPages();
            XIndexAccess oDPi = UnoRuntime.queryInterface(XIndexAccess.class,oDPn);
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
        oShapes = UnoRuntime.queryInterface(XShapes.class,oObj);
        XShape Shape1 = SOF.createShape(xDrawDoc,
            3000,4500,15000,1000,"Ellipse");
        oShapes.add(SOF.createShape(xDrawDoc,
            2000,1500,1000,1000,"Line"));
        oShapes.add(Shape1);
        XShape Shape2 = SOF.createShape(xDrawDoc,
            5000,3500,7500,5000,"Rectangle");
        oShapes.add(Shape2);

        log.println( "adding two style as ObjRelation for ShapeDescriptor" );
        XPropertySet oShapeProps = UnoRuntime.queryInterface(XPropertySet.class,Shape1);
        XStyle aStyle1 = null;
        try {
            aStyle1 = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (Exception e) {}
        oShapeProps = UnoRuntime.queryInterface(XPropertySet.class,Shape2);
        XStyle aStyle2 = null;
        try {
            aStyle2 = (XStyle) AnyConverter.toObject(
                new Type(XStyle.class),oShapeProps.getPropertyValue("Style"));
        } catch (Exception e) {}


       //get the XShapeGrouper
       try{
            log.println("get XShapeGroup");
            XShapeGrouper oSG = UnoRuntime.queryInterface
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

