/*************************************************************************
 *
 *  $RCSfile: AccessibleRadioButton.java,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change:$Date: 2003-10-06 13:34:13 $
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
import util.DesktopTools;
import util.SOfficeFactory;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleComponent;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.accessibility.XAccessibleValue;
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
 *  <li><code>
 *  drafts::com::sun::star::accessibility::XAccessibleValue</code></li>
 *  <li><code>
 *  drafts::com::sun::star::accessibility::XAccessibleText</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleValue
 * @see com.sun.star.accessibility.XAccessibleText
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility.XAccessibleExtendedComponent
 * @see ifc.accessibility.XAccessibleValue
 * @see ifc.accessibility.XAccessibleText
 */
public class AccessibleRadioButton extends TestCase {

    XDesktop the_Desk;
    XTextDocument xTextDoc;
    XAccessibleAction accCloseButton = null;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = (XDesktop) UnoRuntime.queryInterface(
                    XDesktop.class, DesktopTools.createDesktop( (XMultiServiceFactory) Param.getMSF()));
    }

    /**
     * Closes a Hyperlink dialog, disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        try {
            if (accCloseButton != null) {
                log.println("closing HyperlinkDialog");
                accCloseButton.doAccessibleAction(0);
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.lang.DisposedException de) {
            log.println("Already disposed");
        }

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            closeDoc();
        }
    }

    /**
     * Creates a text document, opens a hypelink dialog, selects a first item
     * in IconChoiceCtrl.
     * Then obtains an accessible object with
     * the role <code>AccessibleRole.RADIOBUTTON</code> with the name <code>
     * "Internet"</code>.
     * Object relations created :
     * <ul>
     *  <li> <code>'EventProducer'</code> for
     *      {@link ifc.accessibility._XAccessibleEventBroadcaster}:
     *      grabs focus </li>
     *  <li> <code>'XAccessibleText.Text'</code> for
     *      {@link ifc.accessibility._XAccessibleText}:
     *      the text of the component </li>
     *  <li> <code>'XAccessibleValue.anotherFromGroup'</code> for
     *      {@link ifc.accessibility._XAccessibleValue}:
     *       <code>'FTP'</code> button </li>
     * </ul>
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see com.sun.star.awt.Toolkit
     * @see com.sun.star.accessibility.AccessibleRole
     * @see ifc.accessibility._XAccessibleEventBroadcaster
     * @see ifc.accessibility._XAccessibleText
     * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
     * @see com.sun.star.accessibility.XAccessibleText
     */
    protected TestEnvironment createTestEnvironment(
        TestParameters tParam, PrintWriter log) {

        log.println( "creating a test environment" );

        try {
            if (accCloseButton != null) {
                log.println("closing HyperlinkDialog");
                accCloseButton.doAccessibleAction(0);
            }
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace(log);
        } catch(com.sun.star.lang.DisposedException de) {
            log.println("already disposed");
        }

        if (xTextDoc != null) {
            log.println("dispose a text document");
            closeDoc();
            xTextDoc = null;
        }

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(  (XMultiServiceFactory) tParam.getMSF());

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
            XInterface transf = (XInterface)( (XMultiServiceFactory) tParam.getMSF()).createInstance
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
            oObj = (XInterface) ( (XMultiServiceFactory) tParam.getMSF()).createInstance
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

        XAccessibleContext iconChoiceCtrl = at.getAccessibleObjectForRole(
            xRoot, AccessibleRole.TREE, "IconChoiceControl");

        XAccessibleSelection sel = (XAccessibleSelection)UnoRuntime.
            queryInterface(XAccessibleSelection.class, iconChoiceCtrl);

        try {
            sel.selectAccessibleChild(0);
        } catch(com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Unexpected exception");
            e.printStackTrace(log);
        }

        oObj = at.getAccessibleObjectForRole(xRoot,
            AccessibleRole.RADIO_BUTTON, "Web");

        XAccessibleContext anotherButton = at.getAccessibleObjectForRole(xRoot,
            AccessibleRole.RADIO_BUTTON, "FTP");

        XAccessibleContext closeButton = at.getAccessibleObjectForRole(xRoot,
            AccessibleRole.PUSH_BUTTON, "Close");

        accCloseButton = (XAccessibleAction)
            UnoRuntime.queryInterface(XAccessibleAction.class, closeButton);

        log.println("ImplementationName: "+ util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("EditOnly",
                    "This method isn't supported in this component");

        tEnv.addObjRelation("LimitedBounds", "yes");

        final XAccessibleComponent acomp = (XAccessibleComponent)
            UnoRuntime.queryInterface(XAccessibleComponent.class,oObj);

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    acomp.grabFocus();
                }
            });

        tEnv.addObjRelation("XAccessibleText.Text", "Web");

        tEnv.addObjRelation("EditOnly","Can't change or select Text in AccessibleRadioButton");

        XAccessibleValue anotherButtonValue = (XAccessibleValue)
            UnoRuntime.queryInterface(XAccessibleValue.class, anotherButton);

        tEnv.addObjRelation("XAccessibleValue.anotherFromGroup",
            anotherButtonValue);

        return tEnv;
    }

    /**
    * Sleeps for 0.5 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.currentThread().sleep(2000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
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