/*************************************************************************
 *
 *  $RCSfile: AccessibleCheckBox.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Date: 2003-09-08 12:58:46 $
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
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
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

/**
 * Test for object which is represented by accessible compoent of
 * check box in 'InsertTable' dialog. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleValue</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleAction</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleText</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleValue
 * @see com.sun.star.accessibility.XAccessibleAction
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleText
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleValue
 * @see ifc.accessibility._XAccessibleAction
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleText
 */
public class AccessibleCheckBox extends TestCase {

    XTextDocument xTextDoc = null;
    XAccessibleAction action = null;
    DiagThread psDiag = null;

    /**
     * Opens 'Insert Table' dialog using document dispatch provider
     * running in a separate thread. Finds active top window (the dialog
     * window) and finds first accessible check box walking through the
     * accessible component tree.
     */
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {
        XMultiServiceFactory msf = (XMultiServiceFactory) Param.getMSF();
        log.println("Creating text document");
        if (xTextDoc == null) {
            try {
                SOfficeFactory SOF = SOfficeFactory.getFactory( msf);
                xTextDoc = SOF.createTextDoc(null);
            } catch (com.sun.star.uno.Exception e) {
                throw new StatusException("Can't create document", e);
            }
        }

        shortWait();

        XInterface oObj = null;

        log.println("getting toolkit");

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

        log.println("Opening Dialog in second thread");

        psDiag = new DiagThread(xTextDoc,msf);
        psDiag.start();

        AccessibilityTools at = new AccessibilityTools();

        shortWait();

        log.println("Getting the active TopWindow");

        XWindow xWindow = (XWindow)
                UnoRuntime.queryInterface(XWindow.class,tk.getActiveTopWindow());

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        //at.printAccessibleTree(log, xRoot);

        oObj = at.getAccessibleObjectForRole
            (xRoot, AccessibleRole.PUSH_BUTTON,"Cancel");

        action = (XAccessibleAction)
                    UnoRuntime.queryInterface(XAccessibleAction.class, oObj);

        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.CHECK_BOX);

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XAccessibleComponent acomp = (XAccessibleComponent)
                    UnoRuntime.queryInterface(XAccessibleComponent.class,oObj) ;

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    System.out.println("Grabbing focus ... ");
                    acomp.grabFocus();
                }
            });

        XAccessibleText text = (XAccessibleText)
                    UnoRuntime.queryInterface(XAccessibleText.class,oObj) ;

        tEnv.addObjRelation("XAccessibleText.Text", text.getText());

        tEnv.addObjRelation("EditOnly",
                    "This method isn't supported in this component");

        tEnv.addObjRelation("LimitedBounds", "yes");

        return tEnv;

    }

    /**
     * Closes the dialog using accessible button 'Cancel' found in
     * <code>createTestEnvironment()</code>.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        try {
            log.println( "closing dialog" );
            action.doAccessibleAction(0);
            log.println( "interrupting corresponding Thread" );
            psDiag.interrupt();
            log.println( "closing the document" );
            xTextDoc.dispose();
            log.println( "reinitialize the variable" );
            xTextDoc = null;
        } catch (com.sun.star.lang.IndexOutOfBoundsException ioe) {
            log.println("Couldn't close dialog");
        } catch (com.sun.star.lang.DisposedException de) {
            log.println("Dialog already disposed");
        }
    }

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

    /**
     * Thread for opening modal dialog 'Insert Table'.
     */
    public class DiagThread extends Thread {

        public XTextDocument xTextDoc = null;
        public XMultiServiceFactory msf = null;

        public DiagThread(XTextDocument xTextDoc, XMultiServiceFactory msf) {
            this.xTextDoc = xTextDoc ;
            this.msf = msf;
        }

        public void run() {
        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xTextDoc);

        XController xController = aModel.getCurrentController();

        //Opening PrinterSetupDialog
        try {
            String aSlotID = ".uno:InsertTable";
            XDispatchProvider xDispProv = (XDispatchProvider)
                UnoRuntime.queryInterface( XDispatchProvider.class, xController );
            XURLTransformer xParser = (com.sun.star.util.XURLTransformer)
                UnoRuntime.queryInterface(XURLTransformer.class,
            msf.createInstance("com.sun.star.util.URLTransformer"));
            // Because it's an in/out parameter we must use an array of URL objects.
            URL[] aParseURL = new URL[1];
            aParseURL[0] = new URL();
            aParseURL[0].Complete = aSlotID;
            xParser.parseStrict(aParseURL);
            URL aURL = aParseURL[0];
            XDispatch xDispatcher = xDispProv.queryDispatch( aURL,"",0);
            if( xDispatcher != null )
                    xDispatcher.dispatch( aURL, null );
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't open dialog");
        }
        }
    }
}
