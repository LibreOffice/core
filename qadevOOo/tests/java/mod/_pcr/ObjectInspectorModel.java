/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ObjectInspectorModel.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 11:54:14 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package mod._pcr;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XFrame;
import com.sun.star.inspection.XObjectInspectorModel;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.util.CloseVetoException;
import com.sun.star.util.XCloseable;
import helper.PropertyHandlerFactroy;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import com.sun.star.lang.XMultiServiceFactory;
import util.DesktopTools;
import util.utils;

/**
 * Test for object which is represented by service
 * <code>com.sun.star.reflection.ObjectInspectorModel</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::inspection::XObjectInspectorModel</code></li>
 * </ul>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.inspection.XObjectInspectorModel
 */
public class ObjectInspectorModel extends TestCase {

    /**
     * module variable which holds the Desktop
     * @see com.sun.star.frame.Desktop
     */
    protected static Object StarDesktop = null;

    /**
     * assign to the module variable <CODE>StarDesktop</CODE> the desktop
     * @param Param the test parameters
     * @param log the log writer
     * @see lib.TestParameters
     * @see share.LogWriter
     * @see com.sun.star.frame.Desktop
     */
    protected void initialize(TestParameters Param, PrintWriter log) {
        log.println("create a desktop...");
        StarDesktop = DesktopTools.createDesktop((XMultiServiceFactory) Param.getMSF());
        if (StarDesktop == null){
            throw new StatusException("Could not get a Desktop: null", null);
        }
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.inspection.ObjectInspectorModel</code> with a <code>DefaultContext</code> and
     * <code>PropertyHandlerFactroy[]</code> as parameter
     *
     * @param tParam the tests parameter
     * @param log the logger
     * @return the test environement
     * @see util.DesktopTools
     * @see helper.PropertyHandlerImpl
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {

        this.cleanup(tParam, log);

        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();

        XPropertySet xMSFProp = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, xMSF);
        XComponentContext xDefaultContext = null;
        try{
            // Get the default context from the office server.
            Object oDefaultContext = xMSFProp.getPropertyValue("DefaultContext");

            // Query for the interface XComponentContext.
            xDefaultContext = (XComponentContext) UnoRuntime.queryInterface(
                    XComponentContext.class, oDefaultContext);

        } catch (UnknownPropertyException e){
            throw new StatusException("could not get DefaultContext from xMSF", e);
        } catch (WrappedTargetException e){
            throw new StatusException("could not get DefaultContext from xMSF", e);
        } catch (Exception e){
            throw new StatusException("could not get DefaultContext from xMSF", e);
        }

        try {

            Object[] oHandlerFactories = new Object[1];
            oHandlerFactories[0] = new PropertyHandlerFactroy();

            int minHelpTextLines = 200;
            int maxHelpTextLines = 400;

            XObjectInspectorModel oInspectorModel = com.sun.star.inspection.ObjectInspectorModel.
                    createWithHandlerFactoriesAndHelpSection(xDefaultContext, oHandlerFactories,
                                                             minHelpTextLines, maxHelpTextLines);

            log.println("ImplementationName '" + utils.getImplName(oInspectorModel) + "'");

            TestEnvironment tEnv = new TestEnvironment(oInspectorModel);

            // com.sun.star.inspection.XObjectInspectorModel
            tEnv.addObjRelation("minHelpTextLines", new Integer(minHelpTextLines));
            tEnv.addObjRelation("maxHelpTextLines", new Integer(maxHelpTextLines));

            return tEnv;
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Unexpected exception", e);
        }

    }

    /**
     * Closes the ObjectOnspector using <CODE>XCloseable</CODE>
     * @see com.sun.star.util.XCloseable
     * @param Param the test parameter
     * @param log the logger
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("    Closing dialog if one exists ... ");

        XFrame existentInspector = null;

        XFrame xFrame = (XFrame) UnoRuntime.queryInterface(XFrame.class, StarDesktop);

        existentInspector = xFrame.findFrame( "ObjectInspector", 255 );

        if ( existentInspector != null ){
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                    XCloseable.class, existentInspector);
            try{
                closer.close(true);
            } catch (CloseVetoException e){
                log.println("Could not close inspector: " + e.toString());
            }
        }
    }
}
