/*************************************************************************
 *
 *  $RCSfile: SdUnoPresView.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 12:27:09 $
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
import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.frame.XController;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XModifiable;

/**
* Test for object which is represented by service
* <code>com.sun.star.drawing.DrawingDocumentDrawView</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::drawing::DrawingDocumentDrawView</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::lang::XServiceInfo</code></li>
*  <li> <code>com::sun::star::frame::XController</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::view::XSelectionSupplier</code></li>
*  <li> <code>com::sun::star::drawing::XDrawView</code></li>
* </ul>
* @see com.sun.star.drawing.DrawingDocumentDrawView
* @see com.sun.star.lang.XComponent
* @see com.sun.star.lang.XServiceInfo
* @see com.sun.star.frame.XController
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.view.XSelectionSupplier
* @see com.sun.star.drawing.XDrawView
* @see ifc.drawing._DrawingDocumentDrawView
* @see ifc.lang._XComponent
* @see ifc.lang._XServiceInfo
* @see ifc.frame._XController
* @see ifc.beans._XPropertySet
* @see ifc.view._XSelectionSupplier
* @see ifc.drawing._XDrawView
*/
public class SdUnoPresView extends TestCase {
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
    * instance of the service
    * <code>com.sun.star.drawing.DrawingDocumentDrawView</code>.
    * Object relations created :
    * <ul>
    *  <li> <code>'First'</code> for
    *      {@link ifc.view._XSelectionSupplier}(the first created shape) </li>
    *  <li> <code>'Second'</code> for
    *      {@link ifc.view._XSelectionSupplier}(the second created shape) </li>
    *  <li> <code>'Pages'</code> for
    *      {@link ifc.drawing._XDrawView}(the retrieved collection of the draw
    *      pages) </li>
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
    * @see com.sun.star.drawing.DrawingDocumentDrawView
    */
    protected synchronized TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {

        log.println( "creating a test environment" );

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                        (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a impress documents" );
            xImpressDoc = SOF.createImpressDoc(null);
            xSecondDrawDoc = SOF.createImpressDoc(null);
            shortWait();
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
        }

        // get the drawpage of drawing here
        log.println( "getting Drawpage" );
        XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class, xImpressDoc);
        XDrawPages the_pages = oDPS.getDrawPages();
        XIndexAccess oDPi = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class,the_pages);

        XDrawPage oDrawPage = null;
        XDrawPage secondDrawPage = null;
        try {
            oDrawPage = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDPi.getByIndex(0));
            the_pages.insertNewByIndex(0);
            secondDrawPage = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),oDPi.getByIndex(1));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get DrawPage", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get DrawPage", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get DrawPage", e);
        }

        //put something on the drawpage
        log.println( "inserting some Shapes" );
        XShapes oShapes = (XShapes)
            UnoRuntime.queryInterface(XShapes.class, oDrawPage);
        XShape shape1 = SOF.createShape(
            xImpressDoc, 3000, 4500, 15000, 1000, "Ellipse");
        XShape shape2 = SOF.createShape(
            xImpressDoc, 5000, 3500, 7500, 5000, "Rectangle");
        oShapes.add(shape1);
        oShapes.add(shape2);

        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xImpressDoc);

        XInterface oObj = aModel.getCurrentController();

        XModel aModel2 = (XModel)
            UnoRuntime.queryInterface(XModel.class, xSecondDrawDoc);

        XWindow anotherWindow = (XWindow) UnoRuntime.queryInterface(
                                XWindow.class,aModel2.getCurrentController());

        log.println( "creating a new environment for impress view object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        if (anotherWindow != null) {
            tEnv.addObjRelation("XWindow.AnotherWindow",anotherWindow);
        }

        tEnv.addObjRelation("FirstPage", oDrawPage);
        tEnv.addObjRelation("SecondPage", secondDrawPage);

        tEnv.addObjRelation("First", shape1);
        tEnv.addObjRelation("Second", shape2);

        tEnv.addObjRelation("Pages", the_pages);

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
                                    "slot:27010");

        log.println("Implementation Name: "+utils.getImplName(oObj));

        XModifiable modify = (XModifiable)
                              UnoRuntime.queryInterface(XModifiable.class,xImpressDoc);

        tEnv.addObjRelation("Modifiable",modify);

        return tEnv;

    } // finish method getTestEnvironment

    private void shortWait() {
        try {
            Thread.sleep(5000) ;
        } catch (InterruptedException e) {
            System.out.println("While waiting :" + e) ;
        }
    }


} // finish class SdUnoDrawView

