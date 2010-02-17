/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

package mod._sm;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>::com::sun::star::accessibility::XAccessible</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>::com::sun::star::accessibility::XAccessibleText</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessible
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleText
 * @see ifc.accessibility._XAccessible
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleText
 */
public class SmEditAccessible extends TestCase {
    XComponent xMathDoc;

    /**
     * Creates a <code>StarMath</code> document and obtains an accessibility
     * component with the role <code>AccessibleRole.PANEL</code>.
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {

        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF() );
        try {
            xMathDoc = SOF.openDoc("smath","_blank");
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        } catch (com.sun.star.io.IOException ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        } catch (com.sun.star.uno.Exception ex) {
            ex.printStackTrace( log );
            throw new StatusException( "Couldn't create document", ex );
        }

        // setting a formula in document
        final String expFormula = "sum hat a";
        final XPropertySet xPS = (XPropertySet) UnoRuntime.queryInterface
            (XPropertySet.class, xMathDoc);
        try {
            xPS.setPropertyValue("Formula", expFormula);
        } catch(com.sun.star.lang.WrappedTargetException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
        } catch(com.sun.star.beans.PropertyVetoException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
        } catch(com.sun.star.beans.UnknownPropertyException e) {
            log.println("Couldn't set property value");
            e.printStackTrace(log);
        }

        XInterface oObj = null;

        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xMathDoc);


        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow((XMultiServiceFactory)Param.getMSF(), aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        oObj = at.getAccessibleObjectForRole
            (xRoot, AccessibleRole.PANEL, "", "SmEditAccessible");

        log.println("ImplementationName " + utils.getImplName(oObj));
        at.printAccessibleTree(log, xRoot, Param.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("Destroy", new Boolean(true));

        final XAccessibleContext con = (XAccessibleContext) UnoRuntime.queryInterface(XAccessibleContext.class, oObj);
        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    try {
                        System.out.println("Childs "+con.getAccessibleChildCount());
                        xPS.setPropertyValue("Formula", "sum hat x \n int a \n sum b");
                        shortWait();
                        System.out.println("Childs "+con.getAccessibleChildCount());
                        xPS.setPropertyValue("Formula", expFormula);
                        shortWait();
                    } catch(com.sun.star.lang.WrappedTargetException e) {
                    } catch(com.sun.star.lang.IllegalArgumentException e) {
                    } catch(com.sun.star.beans.PropertyVetoException e) {
                    } catch(com.sun.star.beans.UnknownPropertyException e) {
                    }
                }
            });

        return tEnv;
    }

    /**
     * Disposes the document created in <code>createTestEnvironment</code>
     * method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println( "    disposing xMathDoc " );
        xMathDoc.dispose();
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(1000) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}    // finish class SmModel
