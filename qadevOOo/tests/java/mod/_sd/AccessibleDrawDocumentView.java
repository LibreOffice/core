/*************************************************************************
 *
 *  $RCSfile: AccessibleDrawDocumentView.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change:$Date: 2003-09-08 12:24:55 $
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
import util.AccessibilityTools;
import util.DrawTools;
import util.SOfficeFactory;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.XWindow;
import com.sun.star.container.XIndexAccess;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XDrawView;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

public class AccessibleDrawDocumentView extends TestCase {

    XComponent xDrawDoc;

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
    protected TestEnvironment createTestEnvironment
            (TestParameters Param, PrintWriter log) {
        XInterface oObj = null;

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                (XMultiServiceFactory)Param.getMSF());

        // get the drawpage of drawing here
        log.println( "getting Drawpage" );
        XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class, xDrawDoc);
        XDrawPages oDPn = oDPS.getDrawPages();
        final XDrawPage fDP2 = oDPn.insertNewByIndex(1);
        XIndexAccess oDPi = (XIndexAccess)
            UnoRuntime.queryInterface(XIndexAccess.class, oDPn);
        XDrawPage oDP = null;
        try {
            oDP = (XDrawPage) AnyConverter.toObject(
                                new Type(XDrawPage.class),oDPi.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get by index", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace( log );
            throw new StatusException("Couldn't get by index", e);
        }

        //get a Shape
        log.println( "inserting a Shape" );
        XShape oShape = SOF.createShape(xDrawDoc, 5000, 3500, 7500, 5000, "Rectangle");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc,0)).add(oShape);

        XModel aModel = (XModel)
            UnoRuntime.queryInterface(XModel.class, xDrawDoc);

        AccessibilityTools at = new AccessibilityTools();

        XWindow xWindow = at.getCurrentWindow (
                            (XMultiServiceFactory)Param.getMSF(),aModel);
        XAccessible xRoot = at.getAccessibleObject(xWindow);

        //com.sun.star.accessibility.AccessibleRole
        at.getAccessibleObjectForRole(xRoot, AccessibleRole.DOCUMENT);

        oObj = AccessibilityTools.SearchedContext;

        log.println("ImplementationName "+utils.getImplName(oObj));

        //at.printAccessibleTree(log, xRoot);

        TestEnvironment tEnv = new TestEnvironment(oObj);

        final XDrawView xView = (XDrawView) UnoRuntime.queryInterface
            (XDrawView.class, aModel.getCurrentController()) ;
        final XDrawPage fDP1 = oDP;

        tEnv.addObjRelation("EventProducer",
            new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
                public void fireEvent() {
                    xView.setCurrentPage(fDP2);
                    xView.setCurrentPage(fDP1);
                }
            });

        return tEnv;

    }

    /**
    * Called while disposing a <code>TestEnvironment</code>.
    * Disposes Impress documents.
    * @param tParam test parameters
    * @param tEnv the environment to cleanup
    * @param log writer to log information while testing
    */
    protected void cleanup( TestParameters Param, PrintWriter log) {
        log.println("disposing Draw document");
        xDrawDoc.dispose();
    }

    /**
     * Called while the <code>TestCase</code> initialization. In the
     * implementation does nothing. Subclasses can override to initialize
     * objects shared among all <code>TestEnvironment</code>s.
     *
     * @param tParam test parameters
     * @param log writer to log information while testing
     *
     * @see #initializeTestCase()
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                    (XMultiServiceFactory)Param.getMSF());

        try {
            log.println( "creating a draw document" );
            xDrawDoc = SOF.createDrawDoc(null);;
         } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException("Couldn't create document", e);
         }
    }

}
