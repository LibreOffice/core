/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessiblePresentationOLEShape.java,v $
 * $Revision: 1.8.8.2 $
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
import util.AccessibilityTools;
import util.DrawTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.Size;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class AccessiblePresentationOLEShape extends TestCase {

    static XComponent xDrawDoc;
    static XModel aModel;

    protected void initialize( TestParameters tParam, PrintWriter log ) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a drawdoc" );
            xDrawDoc = SOF.createImpressDoc(null);
            aModel = (XModel)
                UnoRuntime.queryInterface(XModel.class, xDrawDoc);

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

    protected TestEnvironment createTestEnvironment
            (TestParameters tParam, PrintWriter log) {

        XInterface oObj = null;
        XShape oShape = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        XMultiServiceFactory docMSF = (XMultiServiceFactory)
            UnoRuntime.queryInterface(XMultiServiceFactory.class, xDrawDoc);
        try {
            oShape = (XShape) UnoRuntime.queryInterface(XShape.class,
                docMSF.createInstance("com.sun.star.presentation.OLE2Shape"));
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("couldn't create component", e);
        }


        //DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

        XPropertySet oShapeProps = (XPropertySet)
                            UnoRuntime.queryInterface(XPropertySet.class,oShape);
        try {
            oShapeProps.setPropertyValue("IsEmptyPresentationObject", new Boolean(false));
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        }

        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

        try {
            oShapeProps.setPropertyValue(
                "CLSID","12DCAE26-281F-416F-a234-c3086127382e");
            oShapeProps.setPropertyValue("IsEmptyPresentationObject", new Boolean(false));
        } catch (com.sun.star.lang.WrappedTargetException e) {
        } catch (com.sun.star.lang.IllegalArgumentException e) {
        } catch (com.sun.star.beans.PropertyVetoException e) {
        } catch (com.sun.star.beans.UnknownPropertyException e) {
        }

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow ((XMultiServiceFactory)tParam.getMSF(),aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);
        at.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

//        oObj = at.getAccessibleObjectForRole
//            (xRoot, AccessibleRole.SHAPE, "ImpressOLE");
        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.UNKNOWN, "ImpressOLE");

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oObj );

        final XShape fShape = oShape ;
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    try {
                        Size size = fShape.getSize();
                        size.Width += 100;
                        fShape.setSize(size);
                    } catch(com.sun.star.beans.PropertyVetoException e) {}
                }
            });

        log.println("Implementation Name: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment

}

