/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AccessibleToolBox.java,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 18:19:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package mod._toolkit;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.awt.XWindow;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.AccessibilityTools;
import util.DesktopTools;
import util.SOfficeFactory;


/**
 * Test for object that implements the following interfaces :
 * <ul>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleEventBroadcaster
 *  </code></li>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li><code>
 *  ::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
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
public class AccessibleToolBox extends TestCase {
    private static XDesktop the_Desk;
    private static XTextDocument xTextDoc;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = (XDesktop) UnoRuntime.queryInterface(XDesktop.class,
                                                        DesktopTools.createDesktop(
                                                                (XMultiServiceFactory) Param.getMSF()));
    }

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            util.DesktopTools.closeDoc(xTextDoc);
            ;
        }
    }

    /**
     * Creates a text document.
     * Then obtains an accessible object with
     * the role <code>AccessibleRole.TOOLBAR</code>.
     * Object relations created :
     * <ul>
     *  <li> <code>'EventProducer'</code> for
     *      {@link ifc.accessibility._XAccessibleEventBroadcaster}:
     *      grabs focus
     *   </li>
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

        if (xTextDoc != null) {
            util.DesktopTools.closeDoc(xTextDoc);
        }

        ;

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());

        try {
            log.println("creating a text document");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }

        XModel aModel = (XModel) UnoRuntime.queryInterface(XModel.class,
                                                           xTextDoc);

        XInterface oObj = null;

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow(
                                  (XMultiServiceFactory) tParam.getMSF(),
                                  aModel);

        XAccessible xRoot = at.getAccessibleObject(xWindow);


        at.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));
        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.TOOL_BAR);

        log.println("ImplementationName: " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("LimitedBounds", "yes");

        XAccessible acc = at.getAccessibleObject(oObj);
        XAccessible child = null;

        try {
            child = acc.getAccessibleContext().getAccessibleChild(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
        }

        util.dbg.printInterfaces(child);

        final XAccessibleAction action = (XAccessibleAction) UnoRuntime.queryInterface(
                                                 XAccessibleAction.class,
                                                 child);

        tEnv.addObjRelation("EventProducer",
                            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                try {
                    action.doAccessibleAction(0);
                } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                    System.out.println("Couldn't fire event");
                }
            }
        });

        return tEnv;
    }
}