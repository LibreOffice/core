/*************************************************************************
 *
 *  $RCSfile: ODatasourceBrowser.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 11:42:47 $
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
package mod._dbaccess;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;
import util.FormTools;
import util.SOfficeFactory;
import util.WriterTools;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XInitialization;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XCloseable;
import com.sun.star.view.XControlAccess;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.sdb.DataSourceBrowser</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::container::XChild</code></li>
 *  <li> <code>com::sun::star::lang::XInitialization</code></li>
 *  <li> <code>com::sun::star::util::XModifyBroadcaster</code></li>
 *  <li> <code>com::sun::star::awt::XTabController</code></li>
 *  <li> <code>com::sun::star::form::XFormController</code></li>
 *  <li> <code>com::sun::star::container::XElementAccess</code></li>
 *  <li> <code>com::sun::star::frame::XDispatchProvider</code></li>
 *  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
 *  <li> <code>com::sun::star::frame::XController</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.container.XChild
 * @see com.sun.star.lang.XInitialization
 * @see com.sun.star.util.XModifyBroadcaster
 * @see com.sun.star.awt.XTabController
 * @see com.sun.star.form.XFormController
 * @see com.sun.star.container.XElementAccess
 * @see com.sun.star.frame.XDispatchProvider
 * @see com.sun.star.container.XEnumerationAccess
 * @see com.sun.star.frame.XController
 * @see com.sun.star.lang.XComponent
 * @see ifc.container._XChild
 * @see ifc.lang._XInitialization
 * @see ifc.util._XModifyBroadcaster
 * @see ifc.awt._XTabController
 * @see ifc.form._XFormController
 * @see ifc.container._XElementAccess
 * @see ifc.frame._XDispatchProvider
 * @see ifc.container._XEnumerationAccess
 * @see ifc.frame._XController
 * @see ifc.lang._XComponent
 */
public class ODatasourceBrowser extends TestCase {
    XDesktop the_Desk;
    XTextDocument xTextDoc;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = (XDesktop) UnoRuntime.queryInterface(XDesktop.class,
                                                        DesktopTools.createDesktop(
                                                                (XMultiServiceFactory)Param.getMSF()));
        System.setProperty("hideMe", "false");
    }

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("disposing xTextDoc");
        System.setProperty("hideMe", "true");

        if (xTextDoc != null) {
            log.println("    disposing xTextDoc ");

            try {
                XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                            XCloseable.class, xTextDoc);
                closer.close(true);
            } catch (com.sun.star.util.CloseVetoException e) {
                log.println("couldn't close document");
            } catch (com.sun.star.lang.DisposedException e) {
                log.println("couldn't close document");
            }
        }
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     *
     * Creates a new text document disposing the old one if it was
     * created. Using <code>Desktop</code> service get the frame
     * of the document and with its help creates
     * <code>DataSourceBrowser</code> dispatching the URL
     * <code>'.component:DB/DataSourceBrowser'</code>. The
     * component for testing is the controller of the
     * <code>Browser</code> and it's got by searching its
     * frame with the help of TextDocument frame, and obtaining
     * the frame's controller. <p>
     *
     * <b>Note</b>: after creating the text document a short
     * pause is needed to give a possibility to a frame to be
     * created for the document. Else
     * <code>Desktop.getCurrentFrame()</code> method can return
     * <code>null</code> value. <p>
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'XDispatchProvider.URL'</code> for
     *      {@link ifc.frame._XDispatchProvider} </li>
     *  <li> <code>'SecondModel'</code> for
     *      {@link ifc.frame._XController} : the model of
     *      the TextDocument. </li>
     *  <li> <code>'otherWindow'</code> for
     *      {@link ifc.frame._XController} : the window of
     *      the added shape.  </li>
     *  <li> <code>'SecondController'</code> for
     *      {@link ifc.frame._XController} : the controller of
     *      the TextDocument. </li>
     *  <li> <code>'HasViewData'</code> for
     *      {@link ifc.frame._XController} : the
     *      <code>DataSourceBrowser</code> has no view data. </li>
     *  <li> <code>'XInitialization.args'</code> for
     *      {@link ifc.lang._XInitialization} : the arguments for
     *      tbe initialization</li>
     * </ul>
     *
     * @see com.sun.star.frame.Desktop
     * @see com.sun.star.frame.XModel
     * @see com.sun.star.frame.XFrame
     * @see com.sun.star.frame.XController
     * @see com.sun.star.frame.XDispatchProvider
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        log.println("creating a test environment");

        if (xTextDoc != null) {
            log.println("    disposing xTextDoc ");

            try {
                XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                            XCloseable.class, xTextDoc);
                closer.close(true);
            } catch (com.sun.star.util.CloseVetoException e) {
                log.println("couldn't close document");
            } catch (com.sun.star.lang.DisposedException e) {
                log.println("couldn't close document");
            }
        }

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(((XMultiServiceFactory) Param.getMSF()));

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

        XFrame the_frame1 = the_Desk.getCurrentFrame();

        if (the_frame1 == null) {
            log.println("Current frame was not found !!!");
        }

        XDispatchProvider aProv = (XDispatchProvider) UnoRuntime.queryInterface(
                                          XDispatchProvider.class, the_frame1);

        XDispatch getting = null;

        log.println("opening DatasourceBrowser");

        URL the_url = new URL();
        the_url.Complete = ".component:DB/DataSourceBrowser";
        getting = aProv.queryDispatch(the_url, "_beamer", 12);

        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(the_url, noArgs);

        XFrame the_frame2 = the_frame1.findFrame("_beamer", 4);

        the_frame2.setName("DatasourceBrowser");

        XInterface oObj = the_frame2.getController();

        Object[] params = new Object[3];
        PropertyValue param1 = new PropertyValue();
        param1.Name = "DataSourceName";
        param1.Value = "Bibliography";
        params[0] = param1;

        PropertyValue param2 = new PropertyValue();
        param2.Name = "CommandType";
        param2.Value = new Integer(com.sun.star.sdb.CommandType.TABLE);
        params[1] = param2;

        PropertyValue param3 = new PropertyValue();
        param3.Name = "Command";
        param3.Value = "biblio";
        params[2] = param3;

        try {
            XInitialization xInit = (XInitialization) UnoRuntime.queryInterface(
                                            XInitialization.class, oObj);
            xInit.initialize(params);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't initialize document", e);
        }

        shortWait();

        XControlShape aShape = FormTools.createControlShape(xTextDoc, 3000,
                                                            4500, 15000, 10000,
                                                            "CommandButton");
        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        XControlModel shapeModel = aShape.getControl();

        XControlAccess xCtrlAccess = (XControlAccess) UnoRuntime.queryInterface(
                                             XControlAccess.class,
                                             secondController);
        XControl xCtrl = null;

        try {
            xCtrl = xCtrlAccess.getControl(shapeModel);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
        }

        XWindow docWindow = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                                xCtrl);
        log.println("creating a new environment for ODatasourceBrowser object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        //Adding ObjRelations for XInitialization
        tEnv.addObjRelation("XInitialization.args", params);


        //Adding ObjRelations for XController
        tEnv.addObjRelation("Frame", the_frame1);
        tEnv.addObjRelation("SecondModel", aModel1);
        tEnv.addObjRelation("otherWindow", docWindow);
        tEnv.addObjRelation("SecondController", secondController);
        tEnv.addObjRelation("HasViewData", new Boolean(false));


        // Addig relation for XDispatchProvider
        tEnv.addObjRelation("XDispatchProvider.URL",
                            ".uno:DataSourceBrowser/FormLetter");

        return tEnv;
    } // finish method getTestEnvironment

    /**
    * Sleeps for 0.2 sec. to allow StarOffice to react on <code>
    * reset</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(5000);
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e);
        }
    }
} // finish class oDatasourceBrowser
