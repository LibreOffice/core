/*************************************************************************
 *
 *  $RCSfile: AccessibleBrowseBox.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:32:45 $
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
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
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

public class AccessibleBrowseBox extends TestCase {

    XDesktop the_Desk;
    XTextDocument xTextDoc;

    /**
     * Creates the Desktop service (<code>com.sun.star.frame.Desktop</code>).
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = (XDesktop) UnoRuntime.queryInterface(
                    XDesktop.class, DesktopTools.createDesktop((XMultiServiceFactory)Param.getMSF()) );
    }

    /**
     * Disposes the document, if exists, created in
     * <code>createTestEnvironment</code> method.
     */
    protected void cleanup( TestParameters Param, PrintWriter log) {

        log.println("disposing xTextDoc");

        if (xTextDoc != null) {
            xTextDoc.dispose();
        }
    }

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


        XDispatchProvider aProv = (XDispatchProvider)
                UnoRuntime.queryInterface(XDispatchProvider.class,secondController);

        XDispatch getting = null;

        log.println( "opening DatasourceBrowser" );
        URL the_url = new URL();
        the_url.Complete = ".component:DB/DataSourceBrowser";
        getting = aProv.queryDispatch(the_url,"_beamer",12);
        PropertyValue[] noArgs = new PropertyValue[0];
        getting.dispatch(the_url,noArgs);

        shortWait();

        XFrame the_frame1 = the_Desk.getCurrentFrame();

        if (the_frame1 == null) {
            log.println("Current frame was not found !!!");
        }

        XFrame the_frame2 = the_frame1.findFrame("_beamer",4);

        the_frame2.setName("DatasourceBrowser");

        XInterface oObj = null;

        final XInitialization xInit = (XInitialization)
                UnoRuntime.queryInterface(
                        XInitialization.class, the_frame2.getController());

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

        final Object[] fParams = params;

        shortWait();

        try {
            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF()).createInstance
                ("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e );
        }


        XExtendedToolkit tk = (XExtendedToolkit)
                        UnoRuntime.queryInterface(XExtendedToolkit.class,oObj);


        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = (XWindow)
                UnoRuntime.queryInterface(XWindow.class,tk.getActiveTopWindow());

        XAccessible xRoot = at.getAccessibleObject(xWindow);

        //at.printAccessibleTree(log,xRoot);

        oObj = at.getAccessibleObjectForRole
            (xRoot, AccessibleRole.PANEL, "", "AccessibleBrowseBox");

        log.println("ImplementationName: "+util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment( oObj );


        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer(){
                public void fireEvent() {
                    try {
                        xInit.initialize(fParams);
                    } catch(com.sun.star.uno.Exception e) {
                        e.printStackTrace();
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
            Thread.currentThread().sleep(500) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }

}
