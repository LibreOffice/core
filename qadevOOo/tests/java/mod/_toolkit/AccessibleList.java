/*************************************************************************
 *
 *  $RCSfile: AccessibleList.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Date: 2003-09-08 12:59:51 $
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
package mod._toolkit;

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
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;


public class AccessibleList extends TestCase {
    XTextDocument xTextDoc = null;
    XAccessibleAction action = null;
    XMultiServiceFactory msf = null;

    /**
     * Opens 'Insert Hyperlink' dialog using document dispatch provider.
     * Finds active top window (the dialog
     * window) and finds button 'Close' (for closing this dialog when
     * disposing) walking through the accessible component tree.
     * Then the TREE component is found and the 'New Document' tab is
     * selected to make list box visible. After that list box is obtained.
     */
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) {
        XInterface oObj = null;

        try {
            oObj = (XInterface) msf.createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, oObj);

        shortWait();

        XModel aModel1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                            xTextDoc);

        XController secondController = aModel1.getCurrentController();

        XDispatchProvider aProv = (XDispatchProvider) UnoRuntime.queryInterface(
                                          XDispatchProvider.class,
                                          secondController);

        XURLTransformer urlTransf = null;

        try {
            XInterface transf = (XInterface) msf.createInstance(
                                        "com.sun.star.util.URLTransformer");
            urlTransf = (XURLTransformer) UnoRuntime.queryInterface(
                                XURLTransformer.class, transf);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create URLTransformer", e);
        }

        XDispatch getting = null;
        log.println("opening HyperlinkDialog");

        URL[] url = new URL[1];
        url[0] = new URL();
        url[0].Complete = ".uno:HyperlinkDialog";
        urlTransf.parseStrict(url);
        getting = aProv.queryDispatch(url[0], "", 0);

        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(url[0], noArgs);

        shortWait();

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                              tk.getActiveTopWindow());

        XAccessible xRoot = at.getAccessibleObject(xWindow);


        //at.printAccessibleTree(log, xRoot);
        // obtaining 'Close' button
        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PUSH_BUTTON,
                                             "Close");
        action = (XAccessibleAction) UnoRuntime.queryInterface(
                         XAccessibleAction.class, oObj);

        // Selecting 'New Document' tab
        try {
            oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.TREE);

            XAccessibleSelection xAccSel = (XAccessibleSelection) UnoRuntime.queryInterface(
                                                   XAccessibleSelection.class,
                                                   oObj);
            xAccSel.selectAccessibleChild(3);
            shortWait();
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            throw new StatusException("Can't switch to required tab", e);
        }

        log.println("# Getting the ListBox");

        XAccessibleContext parent = at.getAccessibleObjectForRole(xRoot,
                                                                  AccessibleRole.PANEL,
                                                                  "",
                                                                  "com.sun.star.comp.toolkit.AccessibleListBox");

        log.println("# Getting the first child");

        try {
            oObj = parent.getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("XAccessibleSelection.multiSelection",
                            new Boolean(false));

        final XAccessibleComponent acomp = (XAccessibleComponent) UnoRuntime.queryInterface(
                                                   XAccessibleComponent.class,
                                                   oObj);
        final XAccessibleComponent acomp1 = (XAccessibleComponent) UnoRuntime.queryInterface(
                                                    XAccessibleComponent.class,
                                                    action);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                System.out.println("Grabbing focus ... ");
                acomp1.grabFocus();
                acomp.grabFocus();
            }
        });

        return tEnv;
    }

    /**
     * Closes dialog using action of button 'Close'
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("    Closing dialog ... ");

        try {
            action.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Couldn't close dialog");
        } catch (com.sun.star.lang.DisposedException de) {
            log.println("Dialog already disposed");
        }

        xTextDoc.dispose();
    }

    /**
     * Creates writer document
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        try {
            msf = (XMultiServiceFactory) Param.getMSF();

            SOfficeFactory SOF = SOfficeFactory.getFactory(msf);
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create document", e);
        }
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(1000);
        } catch (InterruptedException e) {
            log.println("While waiting :" + e);
        }
    }
}