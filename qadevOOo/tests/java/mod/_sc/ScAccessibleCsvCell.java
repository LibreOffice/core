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

package mod._sc;

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
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class ScAccessibleCsvCell extends TestCase {

    Thread lThread = null;
    static XAccessibleAction accAction = null;

    /**
     * Called to create an instance of <code>TestEnvironment</code> with an
     * object to test and related objects. Subclasses should implement this
     * method to provide the implementation and related objects. The method is
     * called from <code>getTestEnvironment()</code>.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see TestEnvironment
     * @see #getTestEnvironment()
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XInterface oObj = null;

        shortWait();

        try {
            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e );
        }


        XExtendedToolkit tk = UnoRuntime.queryInterface(XExtendedToolkit.class,oObj);

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,tk.getActiveTopWindow());

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        AccessibilityTools.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        oObj = AccessibilityTools.getAccessibleObjectForRole
            (xRoot, AccessibleRole.PUSH_BUTTON, "Cancel");

        accAction = UnoRuntime.queryInterface(XAccessibleAction.class, oObj);

        oObj = AccessibilityTools.getAccessibleObjectForRole
            (xRoot, AccessibleRole.TABLE, true);

        //util.dbg.printInterfaces(oObj);

        XAccessibleContext cont = UnoRuntime.queryInterface(XAccessibleContext.class, oObj);

        String name = "";
        try {
            XAccessible acc = cont.getAccessibleChild(3);
            name = acc.getAccessibleContext().getAccessibleName();
            log.println("Child: "+ name);
            log.println("ImplementationName " + utils.getImplName(acc));
            oObj = acc;
        }
        catch(com.sun.star.lang.IndexOutOfBoundsException e) {}

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("EditOnly",
                    "This method isn't supported in this dialog");

        tEnv.addObjRelation("XAccessibleText.Text", name);

        return tEnv;
    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes calc document.
    * @param Param test parameters
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println( "    closing Dialog " );
        try {
            accAction.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iae) {
            log.println("Couldn't close dialog");
        }
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param Param test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase()
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        log.println("opening dialog");

        PropertyValue[] args = new PropertyValue[1];
        try {
            args[0] = new PropertyValue();
            args[0].Name = "InteractionHandler";
            args[0].Value = ((XMultiServiceFactory)Param.getMSF()).createInstance(
                "com.sun.star.comp.uui.UUIInteractionHandler");
        } catch(com.sun.star.uno.Exception e) {
        }

        lThread = new loadThread(SOF, args);
        lThread.start();
        shortWait();
    }

    /**
    * Sleeps for 2 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(2000) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }

    public class loadThread extends Thread {

        private SOfficeFactory SOF = null ;
        private PropertyValue[] args = null;
        public XComponent xSpreadSheedDoc = null;

        public loadThread(SOfficeFactory SOF, PropertyValue[] Args) {
            this.SOF = SOF;
            this.args = Args;
        }

        public void run() {
            try {
                String url= utils.getFullTestURL("10test.csv");
                log.println("loading "+url);
                SOF.loadDocument(url,args);
            } catch (com.sun.star.uno.Exception e) {
                e.printStackTrace();
                throw new StatusException( "Couldn't create document ", e );
            }
        }
    }


}
