/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: AccessiblePageShape.java,v $
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
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class AccessiblePageShape extends TestCase {

    static XComponent xDrawDoc;
    static XModel aModel;

    protected void initialize( TestParameters tParam, PrintWriter log ) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF() );

        try {
            log.println( "creating a drawdoc" );
            xDrawDoc = SOF.createDrawDoc(null);
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
        //XShape oShape = null;
        XDrawPage oPage = null;

        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println( "creating a test environment" );

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow ((XMultiServiceFactory)tParam.getMSF(),aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

//        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.SHAPE,
//            "PageShape");
        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.UNKNOWN, "PageShape");

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oObj );

        //at.getAccessibleObjectForRole(xRoot, AccessibleRole.SCROLLBAR);
        //final XAccessibleValue xAccVal = (XAccessibleValue)
        //  UnoRuntime.queryInterface
        //    (XAccessibleValue.class, at.SearchedContext) ;
        oPage = DrawTools.getDrawPage(xDrawDoc,0);
        final XPropertySet PageProps = (XPropertySet)
            UnoRuntime.queryInterface(XPropertySet.class, oPage);

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    try {
                        PageProps.setPropertyValue("Height",new Integer(5000));
                    } catch (com.sun.star.beans.UnknownPropertyException upe) {
                        System.out.println("Don't no the Property Height");
                    } catch (com.sun.star.beans.PropertyVetoException pve) {
                        System.out.println(
                        "PropertyVetoException Exception while changing Height");
                    } catch (com.sun.star.lang.IllegalArgumentException iae) {
                        System.out.println(
                        "IllegalArgumentException Exception while changing Height");
                    } catch (com.sun.star.lang.WrappedTargetException wte) {
                        System.out.println(
                        "WrappedTargetException Exception while changing Height");
                    }
                }
            });

        log.println("Implementation Name: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment

}

