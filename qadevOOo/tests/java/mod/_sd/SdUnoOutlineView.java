/*************************************************************************
 *
 *  $RCSfile: SdUnoOutlineView.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-09-08 12:27:01 $
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

package mod._sd;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DesktopTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.awt.XWindow;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatch;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XURLTransformer;
/**
* Test for object which is represented by service
* <code>com.sun.star.present.OutlineView</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
*  <li> <code>com::sun::star::frame::XController</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::presentation::OutlineView</code></li>
*  <li> <code>com::sun::star::frame::XDispatchProvider</code></li>
*  <li> <code>com::sun::star::awt::XWindow</code></li>
* </ul>
* @see com.sun.star.lang.XComponent
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.frame.XController
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.presentation.OutlineView
* @see com.sun.star.awt.XWindow
* @see com.sun.star.frame.XDispatchProvider
* @see ifc.lang._XComponent
* @see ifc.lang._XServiceInfo
* @see ifc.frame._XController
* @see ifc.beans._XPropertySet
* @see ifc.awt._XWindow
* @see ifc.presentation._OutlineView
* @see ifc.frame._XDispatchProvider
*/
public class SdUnoOutlineView extends TestCase {
    XDesktop the_Desk;
    XComponent xImpressDoc;
    XComponent xSecondDrawDoc;

    /**
    * Creates the instance of the service <code>com.sun.star.frame.Desktop</code>.
    * @see com.sun.star.frame.Desktop
    */
    protected void initialize(TestParameters Param, PrintWriter log) {
        the_Desk = (XDesktop)
            UnoRuntime.queryInterface(
                XDesktop.class, DesktopTools.createDesktop(
                                    (XMultiServiceFactory)Param.getMSF()) );
    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes Impress documents.
    * @param tParam test parameters
    * @param tEnv the environment to cleanup
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing impress documents");
        xImpressDoc.dispose();
        xSecondDrawDoc.dispose();
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates two impress documents. After creating of the documents makes short
    * wait to allow frames to be loaded. Retrieves the collection of the draw pages
    * from the first document and takes one of them. Inserts some shapes to the
    * retrieved draw page. Obtains a current controller from the first document
    * using the interface <code>XModel</code>. The obtained controller is the
    * instance of the service <code>com.sun.star.presentation.OutlineView</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'FirstModel'</code> for
    *      {@link ifc.frame._XController}(the interface <code>XModel</code> of
    *      the first created document) </li>
    *  <li> <code>'Frame'</code> for
    *      {@link ifc.frame._XController}(the frame of the created
    *      document) </li>
    *  <li> <code>'SecondModel'</code> for
    *      {@link ifc.frame._XController}(the interface <code>XModel</code> of
    *      the second created document) </li>
    *  <li> <code>'SecondController'</code> for
    *      {@link ifc.frame._XController}(the current controller of the second
    *      created document) </li>
    * </ul>
    * @see com.sun.star.frame.XModel
    */
    protected synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {

        log.println( "creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory( (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a impress document" );
            xImpressDoc = SOF.createImpressDoc(null);
            shortWait();
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
        }

        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xImpressDoc);

        XInterface oObj = aModel.getCurrentController();

        //Change to Outline view
        try {
            String aSlotID = "slot:27010";
            XDispatchProvider xDispProv = (XDispatchProvider)
                UnoRuntime.queryInterface( XDispatchProvider.class, oObj );
            XURLTransformer xParser = (com.sun.star.util.XURLTransformer)
                UnoRuntime.queryInterface(XURLTransformer.class,
        ((XMultiServiceFactory)Param.getMSF()).createInstance("com.sun.star.util.URLTransformer"));
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
            log.println("Couldn't change mode");
        }

        try {
            log.println( "creating a second impress document" );
            xSecondDrawDoc = SOF.createImpressDoc(null);
            shortWait();
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
        }

        XModel aModel2 = (XModel)
            UnoRuntime.queryInterface(XModel.class, xSecondDrawDoc);

        XWindow anotherWindow = (XWindow) UnoRuntime.queryInterface(
                                XWindow.class,aModel2.getCurrentController());

        oObj = aModel.getCurrentController();

        log.println( "creating a new environment for impress view object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        if (anotherWindow != null) {
            tEnv.addObjRelation("XWindow.AnotherWindow", anotherWindow);
        }

         //Adding ObjRelations for XController
        tEnv.addObjRelation("FirstModel", aModel);

        XFrame the_frame = the_Desk.getCurrentFrame();
        tEnv.addObjRelation("Frame", the_frame);

         aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xSecondDrawDoc);
        //Adding ObjRelations for XController
        tEnv.addObjRelation("SecondModel", aModel);

        XController secondController = aModel.getCurrentController();
        tEnv.addObjRelation("SecondController", secondController);
        tEnv.addObjRelation("XDispatchProvider.URL",
                                    "slot:27069");

        log.println("Implementation Name: " + utils.getImplName(oObj));

        return tEnv;

    } // finish method getTestEnvironment

    private void shortWait() {
        try {
            Thread.sleep(5000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }


} // finish class SdUnoOutlineView

