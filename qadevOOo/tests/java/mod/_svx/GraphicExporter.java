/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: GraphicExporter.java,v $
 * $Revision: 1.7.8.1 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
package mod._svx;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XExporter;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.DrawTools;
import util.SOfficeFactory;
import util.XMLTools;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.drawing.GraphicExportFilter</code>. <p>
 *
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::document::XFilter</code></li>
 *  <li> <code>com::sun::star::document::XMimeTypeInfo</code></li>
 *  <li> <code>com::sun::star::document::XExporter</code></li>
 * </ul> <p>
 *
 * The following files used by this test :
 * <ul>
 *  <li><b> space-metal.jpg </b> : the file used for GraphicObject
 *  creation. This image must be then exported. </li>
 * </ul> <p>
 *
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 *
 * @see com.sun.star.document.XFilter
 * @see com.sun.star.document.XMimeTypeInfo
 * @see com.sun.star.document.XExporter
 * @see ifc.document._XFilter
 * @see ifc.document._XMimeTypeInfo
 * @see ifc.document._XExporter
 */
public class GraphicExporter extends TestCase {
    static XComponent xDrawDoc;

    /**
     * Creates a new draw document.
     */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        log.println("creating a drawdoc");
        xDrawDoc = DrawTools.createDrawDoc(
                           (XMultiServiceFactory) tParam.getMSF());
    }

    /**
     * Disposes the draw document created before
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xDrawDoc ");
        util.DesktopTools.closeDoc(xDrawDoc);
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates an instance of the service
     * <code>com.sun.star.drawing.GraphicExportFilter</code> as
     * a tested component. Then a <code>GraphicObjectShape</code>
     * instance is added into the document and its image is obtained
     * from JPEG file. This shape content is intended to be exported.
     *
     *     Object relations created :
     * <ul>
     *  <li> <code>'MediaDescriptor'</code> for
     *      {@link ifc.document._XFilter} :
     *      descriptor which contains target file name in
     *      the temporary directory, file type (JPEG)
     *      </li>
     *  <li> <code>'XFilter.Checker'</code> for
     *      {@link ifc.document._XFilter} :
     *      checks if the target file exists.
     *      In the case if SOffice is started in 'Hide' mode
     *      ('soapi.test.hidewindows' test parameter is 'true')
     *      the checker always returns <code>true</code>.
     *      </li>
     *  <li> <code>'SourceDocument'</code> for
     *      {@link ifc.document._XExporter} :
     *      the <code>GraphicObjectShape</code> component
     *      with loaded image.
     *   </li>
     * </ul>
     */
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) {
        XInterface oObj = null;
        XShape oShape = null;
        Object go = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        try {
            go = ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                         "com.sun.star.drawing.GraphicExportFilter");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create instance");
            e.printStackTrace(log);
        }

        // create testobject here
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     (XMultiServiceFactory) tParam.getMSF());
        oShape = SOF.createShape(xDrawDoc, 5000, 5000, 1500, 1000,
                                 "GraphicObject");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc, 0)).add(oShape);

        XPropertySet oShapeProps = (XPropertySet) UnoRuntime.queryInterface(
                                           XPropertySet.class, oShape);
        XComponent xComp = null;

        try {
            oShapeProps.setPropertyValue("GraphicURL",
                                         util.utils.getFullTestURL(
                                                 "space-metal.jpg"));
            xComp = (XComponent) UnoRuntime.queryInterface(XComponent.class,
                                                           oShape);

            XExporter xEx = (XExporter) UnoRuntime.queryInterface(
                                    XExporter.class, (XInterface) go);
            xEx.setSourceDocument(xComp);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Error while preparing component", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Error while preparing component", e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            e.printStackTrace(log);
            throw new StatusException("Error while preparing component", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            e.printStackTrace(log);
            throw new StatusException("Error while preparing component", e);
        }

        final URL aURL = new URL();
        aURL.Complete = util.utils.getOfficeTemp(
                                (XMultiServiceFactory) tParam.getMSF()) +
                        "picture.jpg";

        final XSimpleFileAccess fAcc;

        try {
            Object oFAcc = ((XMultiServiceFactory) tParam.getMSF()).createInstance(
                                   "com.sun.star.ucb.SimpleFileAccess");
            fAcc = (XSimpleFileAccess) UnoRuntime.queryInterface(
                           XSimpleFileAccess.class, oFAcc);

            if (fAcc.exists(aURL.Complete)) {
                fAcc.kill(aURL.Complete);
            }
        } catch (com.sun.star.uno.Exception e) {
            log.println("Error accessing file system :");
            e.printStackTrace(log);
            throw new StatusException("Error accessing file system.", e);
        }

        oObj = (XInterface) go;
        log.println("ImplName " + util.utils.getImplName(oObj));

        TestEnvironment tEnv = new TestEnvironment(oObj);
        tEnv.addObjRelation("MediaDescriptor",
                            XMLTools.createMediaDescriptor(
                                    new String[] {
            "FilterName", "URL", "MediaType"
        }, new Object[] { "JPG", aURL, "image/jpeg" }));
        tEnv.addObjRelation("SourceDocument", xComp);

        log.println("adding ObjRelation for XFilter");
        log.println("This Component doesn't really support the cancel method");
        log.println("See #101725");
        tEnv.addObjRelation("NoFilter.cancel()", new Boolean(true));

        final String hideMode = (String) tParam.get("soapi.test.hidewindows");
        tEnv.addObjRelation("XFilter.Checker",
                            new ifc.document._XFilter.FilterChecker() {
            public boolean checkFilter() {
                try {
                    if ((hideMode != null) && hideMode.equals("true")) {
                        return true;
                    }

                    return fAcc.exists(aURL.Complete);
                } catch (com.sun.star.uno.Exception e) {
                    return false;
                }
            }
        });

        return tEnv;
    } // finish method getTestEnvironment
} // finish class GraphicExporter
