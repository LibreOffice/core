/*************************************************************************
 *
 *  $RCSfile: AccessibleMenu.java,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Date: 2003-09-08 13:00:25 $
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
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented accessible component of
 * main menu of a document. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleAction</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleSelection</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleValue</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleText</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleContext</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleAction
 * @see com.sun.star.accessibility.XAccessibleSelection
 * @see com.sun.star.accessibility.XAccessibleValue
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleText
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleAction
 * @see ifc.accessibility._XAccessibleSelection
 * @see ifc.accessibility._XAccessibleValue
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleText
 * @see ifc.accessibility._XAccessibleContext
 */
public class AccessibleMenu extends TestCase {

    XTextDocument xTextDoc = null;
    XAccessibleAction action = null;

    /**
     * Finds accessible component with role <code>MENU</code>
     * walking through the accessible component tree of a document.
     */
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {

        XMultiServiceFactory msf = (XMultiServiceFactory) Param.getMSF();
        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory( msf);
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create document", e);
        }

        XInterface oObj = null;

        try {
            oObj = (XInterface) msf.createInstance("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e );
        }

        XExtendedToolkit tk = (XExtendedToolkit)
                        UnoRuntime.queryInterface(XExtendedToolkit.class,oObj);

        shortWait();

        XWindow xWindow = (XWindow)
                UnoRuntime.queryInterface(XWindow.class,tk.getActiveTopWindow());

        AccessibilityTools at = new AccessibilityTools();


        XAccessible xRoot = at.getAccessibleObject(xWindow);

//        at.printAccessibleTree(log, xRoot);

        XAccessibleContext menubar = at.getAccessibleObjectForRole(xRoot, AccessibleRole.MENU_BAR);
        Object menu2 = null;

        try {
            oObj = menubar.getAccessibleChild(2);
            menu2 = menubar.getAccessibleChild(1);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {

        }

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XAccessibleAction act1 = (XAccessibleAction)
            UnoRuntime.queryInterface(XAccessibleAction.class,oObj) ;
        final XAccessibleAction act2 = (XAccessibleAction)
            UnoRuntime.queryInterface(XAccessibleAction.class,menu2) ;

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    try {
                        act2.doAccessibleAction(0);
                        try {
                            Thread.sleep(500) ;
                        } catch (InterruptedException e) {
                        }
                        act1.doAccessibleAction(0);
                    } catch(com.sun.star.lang.IndexOutOfBoundsException e){}
                }
            });

        XAccessibleText text = (XAccessibleText)
                    UnoRuntime.queryInterface(XAccessibleText.class,oObj) ;

        tEnv.addObjRelation("XAccessibleText.Text", text.getText());

        tEnv.addObjRelation("EditOnly","Can't change or select Text in Menu");

        tEnv.addObjRelation("LimitedBounds", "yes");

        return tEnv;

    }

    protected void cleanup( TestParameters Param, PrintWriter log) {
        try {
            xTextDoc.dispose();
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("Object already disposed");
        }
    }

    /**
     * Creates a new writer document
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
    }


    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(500) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }
}
