/*************************************************************************
 *
 *  $RCSfile: AccessibleTabPage.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 13:02:14 $
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

import java.awt.Robot;
import java.awt.event.InputEvent;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.DesktopTools;
import util.SOfficeFactory;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.awt.Point;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XCloseable;
import com.sun.star.util.XURLTransformer;


/**
 * Test for object that implements the following interfaces :
 * <ul>
 *  <li><code>
 *  drafts::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li><code>
 *  drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster
 *  </code></li>
 *  <li><code>
 *  drafts::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li><code>
 *  drafts::com::sun::star::accessibility::XAccessibleExtendedComponent
 *  </code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility.XAccessibleExtendedComponent
 */
public class AccessibleTabPage extends TestCase {
    XDesktop the_Desk;
    XTextDocument xTextDoc;
    XAccessibleAction accCloseButton = null;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = (XDesktop) UnoRuntime.queryInterface(XDesktop.class,
                                                        DesktopTools.createDesktop(
        (XMultiServiceFactory) Param.getMSF()));
    }

    /**
     * Closes an InsertFields dialog, disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        try {
            if (accCloseButton != null) {
                log.println("closing InsertFields Dialog");
                accCloseButton.doAccessibleAction(0);
                shortWait();
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Couldn't close dialog " + e.getMessage());
        }

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            closeDoc();
        }
    }

    /**
     * Creates a text document, opens an InsertField dialog.
     * Then obtains an accessible object with
     * the role <code>AccessibleRole.PAGETAB</code>.
     * Object relations created :
     * <ul>
     *  <li> <code>'EventProducer'</code> for
     *      {@link ifc.accessibility._XAccessibleEventBroadcaster}:
     *      grabs focus </li>
     * </ul>
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see com.sun.star.awt.Toolkit
     * @see com.sun.star.accessibility.AccessibleRole
     * @see ifc.accessibility._XAccessibleEventBroadcaster
     * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        log.println("creating a test environment");

        try {
            if (accCloseButton != null) {
                log.println("closing InsertFields Dialog");
                accCloseButton.doAccessibleAction(0);
                shortWait();
            }
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Couldn't close dialog " + e.getMessage());
        }

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            closeDoc();
        }

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a text document");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }

        shortWait();

        XModel aModel1 = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                            xTextDoc);

        XController secondController = aModel1.getCurrentController();

        XDispatchProvider aProv = (XDispatchProvider) UnoRuntime.queryInterface(
                                          XDispatchProvider.class,
                                          secondController);

        XURLTransformer urlTransf = null;

        try {
            XInterface transf = (XInterface) ( (XMultiServiceFactory) tParam.getMSF())
                                                   .createInstance("com.sun.star.util.URLTransformer");
            urlTransf = (XURLTransformer) UnoRuntime.queryInterface(
                                XURLTransformer.class, transf);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create URLTransformer", e);
        }

        XDispatch getting = null;
        log.println("opening InsertField dialog");

        URL[] url = new URL[1];
        url[0] = new URL();
        url[0].Complete = ".uno:InsertField";
        urlTransf.parseStrict(url);
        getting = aProv.queryDispatch(url[0], "", 0);

        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(url[0], noArgs);

        shortWait();

        XInterface oObj = null;

        try {
            oObj = (XInterface) ( (XMultiServiceFactory) tParam.getMSF())
                                      .createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XExtendedToolkit tk = (XExtendedToolkit) UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, oObj);

        AccessibilityTools at = new AccessibilityTools();

        shortWait();

        XWindow xWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                              tk.getActiveTopWindow());

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        XAccessibleContext closeButton = at.getAccessibleObjectForRole(xRoot,
                                                                       AccessibleRole.PUSH_BUTTON,
                                                                       "Close");

        accCloseButton = (XAccessibleAction) UnoRuntime.queryInterface(
                                 XAccessibleAction.class, closeButton);

        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.PAGE_TAB);

        log.println("ImplementationName: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("EditOnly", "toolkit.AccessibleTabPage");
        tEnv.addObjRelation("LimitedBounds", "toolkit.AccessibleTabPage");

        XAccessibleComponent accComp = (XAccessibleComponent) UnoRuntime.queryInterface(
                                               XAccessibleComponent.class,
                                               oObj);
        final Point point = accComp.getLocationOnScreen();

        shortWait();

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                try {
                    Robot rob = new Robot();
                    rob.mouseMove(point.X + 20, point.Y + 70);
                    rob.mousePress(InputEvent.BUTTON1_MASK);
                } catch (java.awt.AWTException e) {
                    System.out.println("couldn't fire event");
                }
            }
        });

        return tEnv;
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.currentThread().sleep(1000);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }

    protected void closeDoc() {
        XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                    XCloseable.class, xTextDoc);

        try {
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("Couldn't close document " + e.getMessage());
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Couldn't close document " + e.getMessage());
        }
    }
}