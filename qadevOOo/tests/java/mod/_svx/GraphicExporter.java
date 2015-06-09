/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
package mod._svx;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XExporter;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XComponent;
import com.sun.star.ucb.XSimpleFileAccess;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;

import java.io.PrintWriter;

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
 * threads concurrently.
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
    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {
        log.println("creating a drawdoc");
        xDrawDoc = DrawTools.createDrawDoc(
                           tParam.getMSF());
    }

    /**
     * Disposes the draw document created before
     */
    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xDrawDoc ");
        util.DesktopTools.closeDoc(xDrawDoc);
    }

    /**
     * Creating a TestEnvironment for the interfaces to be tested.
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
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters tParam,
                                                    PrintWriter log) throws Exception {
        XInterface oObj = null;
        XShape oShape = null;
        Object go = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        try {
            go = tParam.getMSF().createInstance(
                         "com.sun.star.drawing.GraphicExportFilter");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create instance");
            e.printStackTrace(log);
        }

        // create testobject here
        SOfficeFactory SOF = SOfficeFactory.getFactory(
                                     tParam.getMSF());
        oShape = SOF.createShape(xDrawDoc, 5000, 5000, 1500, 1000,
                                 "GraphicObject");
        DrawTools.getShapes(DrawTools.getDrawPage(xDrawDoc, 0)).add(oShape);

        XPropertySet oShapeProps = UnoRuntime.queryInterface(
                                           XPropertySet.class, oShape);
        XComponent xComp = null;

        oShapeProps.setPropertyValue("GraphicURL",
                                     util.utils.getFullTestURL(
                                             "space-metal.jpg"));
        xComp = UnoRuntime.queryInterface(XComponent.class,
                                                       oShape);

        XExporter xEx = UnoRuntime.queryInterface(
                                XExporter.class, go);
        xEx.setSourceDocument(xComp);

        final URL aURL = new URL();
        aURL.Complete = util.utils.getOfficeTemp(
                                tParam.getMSF()) +
                        "picture.jpg";

        final XSimpleFileAccess fAcc;

        Object oFAcc = tParam.getMSF().createInstance(
                               "com.sun.star.ucb.SimpleFileAccess");
        fAcc = UnoRuntime.queryInterface(
                       XSimpleFileAccess.class, oFAcc);

        if (fAcc.exists(aURL.Complete)) {
            fAcc.kill(aURL.Complete);
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
        tEnv.addObjRelation("NoFilter.cancel()", Boolean.TRUE);

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
