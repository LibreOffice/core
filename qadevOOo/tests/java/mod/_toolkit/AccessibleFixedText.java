/*************************************************************************
 *
 *  $RCSfile: AccessibleFixedText.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Date: 2003-09-08 12:59:43 $
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
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.accessibility.XAccessibleText;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XFixedText;
import com.sun.star.awt.XWindow;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Test for object which is represented by accesible component
 * of the fixed text label in 'Hyperlink' Dialog. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleExtendedComponent</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleEventBroadcaster</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleComponent</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleContext</code></li>
 *  <li> <code>drafts::com::sun::star::accessibility::XAccessibleText</code></li>
 * </ul> <p>
 *
 * @see com.sun.star.accessibility.XAccessibleExtendedComponent
 * @see com.sun.star.accessibility.XAccessibleEventBroadcaster
 * @see com.sun.star.accessibility.XAccessibleComponent
 * @see com.sun.star.accessibility.XAccessibleContext
 * @see com.sun.star.accessibility.XAccessibleText
 * @see ifc.accessibility._XAccessibleExtendedComponent
 * @see ifc.accessibility._XAccessibleEventBroadcaster
 * @see ifc.accessibility._XAccessibleComponent
 * @see ifc.accessibility._XAccessibleContext
 * @see ifc.accessibility._XAccessibleText
 */
public class AccessibleFixedText extends TestCase {

    XTextDocument xTextDoc = null;
    XAccessibleAction action = null;
    private XWindow xWinDlg = null ;

    /**
     * Creates a new dialog adds fixed text control to it and
     * displays it. Then the text's accessible component is
     * obtained.
     */
    protected TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log) {

        XInterface oObj = null;
        XMultiServiceFactory xMSF =  (XMultiServiceFactory) Param.getMSF() ;
        XControlModel dlgModel = null ;

        XControl txtControl = null ;
        XControlModel txtModel = null ;

        try {
            dlgModel = (XControlModel) UnoRuntime.queryInterface
                (XControlModel.class, xMSF.createInstance
                ("com.sun.star.awt.UnoControlDialogModel")) ;

            XControl dlgControl = (XControl) UnoRuntime.queryInterface
                (XControl.class, xMSF.createInstance
                ("com.sun.star.awt.UnoControlDialog")) ;

            dlgControl.setModel(dlgModel) ;

            txtModel = (XControlModel) UnoRuntime.queryInterface
                (XControlModel.class, xMSF.createInstance
                ("com.sun.star.awt.UnoControlFixedTextModel")) ;

            txtControl = (XControl) UnoRuntime.queryInterface
                (XControl.class, xMSF.createInstance
                ("com.sun.star.awt.UnoControlFixedText")) ;

            txtControl.setModel(txtModel) ;

            XFixedText xFT = (XFixedText) UnoRuntime.queryInterface
                (XFixedText.class, txtControl);
            xFT.setText("FxedText");

            XControlContainer ctrlCont = (XControlContainer)
                UnoRuntime.queryInterface(XControlContainer.class, dlgControl) ;

            ctrlCont.addControl("Text", txtControl) ;

            xWinDlg = (XWindow) UnoRuntime.queryInterface
                (XWindow.class, dlgControl) ;

            xWinDlg.setVisible(true) ;

            xWinDlg.setPosSize(0, 0, 200, 100, PosSize.SIZE) ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error creating dialog :") ;
            e.printStackTrace(log) ;
        }

        try {
            oObj = (XInterface) ( (XMultiServiceFactory) Param.getMSF()).createInstance
                ("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e );
        }

        XExtendedToolkit tk = (XExtendedToolkit)
                        UnoRuntime.queryInterface(XExtendedToolkit.class,oObj);

        shortWait();

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = (XWindow)
            UnoRuntime.queryInterface(XWindow.class,tk.getActiveTopWindow());

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        at.printAccessibleTree(log, xRoot);

        oObj = at.getAccessibleObjectForRole(xRoot, AccessibleRole.LABEL);

        log.println("ImplementationName " + utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XWindow xWin = (XWindow) UnoRuntime.queryInterface
            (XWindow.class, txtControl);

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    xWin.setEnable(false);
                    xWin.setEnable(true);
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
     * Closes dialog using action of button 'Close'
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println( "    Closing dialog ... " );
        xWinDlg.dispose();
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
