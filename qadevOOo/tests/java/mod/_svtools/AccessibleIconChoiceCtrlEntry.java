/*************************************************************************
 *
 *  $RCSfile: AccessibleIconChoiceCtrlEntry.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:33:52 $
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

package mod._svtools;

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
import com.sun.star.accessibility.XAccessibleContext;
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
 *  drafts::com::sun::star::accessibility::XAccessibleText</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleText
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility.XAccessibleText
 */
public class AccessibleIconChoiceCtrlEntry extends TestCase {

    XDesktop the_Desk;
    XTextDocument xTextDoc;
    XAccessibleAction accCloseButton = null;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = (XDesktop) UnoRuntime.queryInterface(
                    XDesktop.class, DesktopTools.createDesktop((XMultiServiceFactory)Param.getMSF()));
    }

    /**
     * Closes the Hyperlink dialog.
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println("closing HyperlinkDialog");

        try {
            if (accCloseButton != null) {
                accCloseButton.doAccessibleAction(0);
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        }

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            xTextDoc.dispose();
        }
    }

    /**
     * Creates a text document. Opens the Hyperlink dialog.
     * Creates an instance of the service
     * <code>com.sun.star.awt.Toolkit</code> and gets active top window.
     * Then obtains an accessible object with the role
     * <code>AccessibleRole.LABEL</code> and with the name
     * <code>"Internet"</code>.
     * Object relations created :
     * <ul>
     *  <li> <code>'EventProducer'</code> for
     *      {@link ifc.accessibility._XAccessibleEventBroadcaster}:
     *      method <code>fireEvent()</code> is empty because object is transient
     *   </li>
     *  <li> <code>'XAccessibleText.Text'</code> for
     *      {@link ifc.accessibility._XAccessibleText}:
     *         the string representation of the item text
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
    protected TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log) {

        log.println( "creating a test environment" );

        if (xTextDoc != null) xTextDoc.dispose();

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)tParam.getMSF());

        try {
            log.println( "creating a text document" );
            xTextDoc = SOF.createTextDoc(null);
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }

        shortWait();

        XModel aModel1 = (XModel)
                    UnoRuntime.queryInterface(XModel.class, xTextDoc);

        XController secondController = aModel1.getCurrentController();

        XDispatchProvider aProv = (XDispatchProvider)UnoRuntime.
            queryInterface(XDispatchProvider.class, secondController);

        XURLTransformer urlTransf = null;

        try {
            XInterface transf = (XInterface)((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.util.URLTransformer");
            urlTransf = (XURLTransformer)UnoRuntime.queryInterface
                (XURLTransformer.class, transf);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create URLTransformer", e );
        }

        XDispatch getting = null;
        log.println( "opening HyperlinkDialog" );
        URL[] url = new URL[1];
        url[0] = new URL();
        url[0].Complete = ".uno:HyperlinkDialog";
        urlTransf.parseStrict(url);
        getting = aProv.queryDispatch(url[0], "", 0);
        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(url[0], noArgs);

        shortWait();

        XInterface oObj = null;
        try {
            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e );
        }

        XExtendedToolkit tk = (XExtendedToolkit)
            UnoRuntime.queryInterface(XExtendedToolkit.class, oObj);

        AccessibilityTools at = new AccessibilityTools();

        shortWait();

        XWindow xWindow = (XWindow)
            UnoRuntime.queryInterface(XWindow.class, tk.getActiveTopWindow());

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        oObj = at.getAccessibleObjectForRole(xRoot,
            AccessibleRole.LABEL, "Internet");

        XAccessibleContext closeButton = at.getAccessibleObjectForRole(xRoot,
            AccessibleRole.PUSH_BUTTON, "Close");

        accCloseButton = (XAccessibleAction)
            UnoRuntime.queryInterface(XAccessibleAction.class, closeButton);

        log.println("ImplementationName: "+ util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                }
            });

        tEnv.addObjRelation("XAccessibleText.Text", "Internet");

        tEnv.addObjRelation("EditOnly",
                    "This method isn't supported in this dialog");

        tEnv.addObjRelation("LimitedBounds",
                    "only delivers senseful values for getCharacterBounds(0,length-1)");

        return tEnv;
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.currentThread().sleep(500) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }

}
