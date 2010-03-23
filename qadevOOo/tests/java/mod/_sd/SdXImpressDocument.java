/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
package mod._sd;

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.XShape;
import com.sun.star.frame.XController;
import com.sun.star.frame.XModel;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.view.XSelectionSupplier;
import ifc.view._XPrintJobBroadcaster;
import java.io.File;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.DrawTools;
import util.SOfficeFactory;
import util.utils;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.presentation.PresentationDocument</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::lang::XMultiServiceFactory</code></li>
 *  <li> <code>com::sun::star::drawing::XMasterPagesSupplier</code></li>
 *  <li> <code>com::sun::star::presentation::XCustomPresentationSupplier</code></li>
 *  <li> <code>com::sun::star::document::XLinkTargetSupplier</code></li>
 *  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::drawing::XLayerSupplier</code></li>
 *  <li> <code>com::sun::star::presentation::XPresentationSupplier</code></li>
 *  <li> <code>com::sun::star::style::XStyleFamiliesSupplier</code></li>
 *  <li> <code>com::sun::star::drawing::DrawingDocument</code></li>
 *  <li> <code>com::sun::star::drawing::XDrawPageDuplicator</code></li>
 *  <li> <code>com::sun::star::drawing::XDrawPagesSupplier</code></li>
 * </ul>
 * @see com.sun.star.presentation.PresentationDocument
 * @see com.sun.star.lang.XMultiServiceFactory
 * @see com.sun.star.drawing.XMasterPagesSupplier
 * @see com.sun.star.presentation.XCustomPresentationSupplier
 * @see com.sun.star.document.XLinkTargetSupplier
 * @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.drawing.XLayerSupplier
 * @see com.sun.star.presentation.XPresentationSupplier
 * @see com.sun.star.style.XStyleFamiliesSupplier
 * @see com.sun.star.drawing.DrawingDocument
 * @see com.sun.star.drawing.XDrawPageDuplicator
 * @see com.sun.star.drawing.XDrawPagesSupplier
 * @see ifc.lang._XMultiServiceFactory
 * @see ifc.drawing._XMasterPagesSupplier
 * @see ifc.presentation._XCustomPresentationSupplier
 * @see ifc.document._XLinkTargetSupplier
 * @see ifc.beans._XPropertySet
 * @see ifc.drawing._XLayerSupplier
 * @see ifc.presentation._XPresentationSupplier
 * @see ifc.style._XStyleFamiliesSupplier
 * @see ifc.drawing._DrawingDocument
 * @see ifc.drawing._XDrawPageDuplicator
 * @see ifc.drawing._XDrawPagesSupplier
 */
public class SdXImpressDocument extends TestCase {
    XComponent xImpressDoc;
    XComponent xImpressDoc2;

    /**
     * Called while disposing a <code>TestEnvironment</code>.
     * Disposes Impress document.
     * @param tParam test parameters
     * @param tEnv the environment to cleanup
     * @param log writer to log information while testing
     */
    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("disposing xImpressDoc");
        util.DesktopTools.closeDoc(xImpressDoc);;
        util.DesktopTools.closeDoc(xImpressDoc2);;
    }

    /**
     * Creating a Testenvironment for the interfaces to be tested.
     * Creates new impress document that is the instance of the service
     * <code>com.sun.star.presentation.PresentationDocument</code>.
     * @see com.sun.star.presentation.PresentationDocument
     */
    public synchronized TestEnvironment createTestEnvironment(TestParameters Param,
        PrintWriter log)
        throws StatusException {
        log.println("creating a test environment");

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(
            (XMultiServiceFactory) Param.getMSF());

        try {
            log.println("creating two impress documents");
            xImpressDoc2 = SOF.createImpressDoc(null);
            xImpressDoc = SOF.createImpressDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create documents", e);
        }

        XModel xModel1 = (XModel) UnoRuntime.queryInterface(XModel.class,
            xImpressDoc);
        XModel xModel2 = (XModel) UnoRuntime.queryInterface(XModel.class,
            xImpressDoc2);

        XController cont1 = xModel1.getCurrentController();
        XController cont2 = xModel2.getCurrentController();

        cont1.getFrame().setName("cont1");
        cont2.getFrame().setName("cont2");

        XSelectionSupplier sel = (XSelectionSupplier) UnoRuntime.queryInterface(
            XSelectionSupplier.class, cont1);

        XShape aShape = SOF.createShape(xImpressDoc, 5000, 3500, 7500, 5000,
            "Rectangle");


        XPropertySet xShapeProps = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, aShape);

        try {
            xShapeProps.setPropertyValue("FillStyle", com.sun.star.drawing.FillStyle.SOLID);
            xShapeProps.setPropertyValue("FillTransparence", new Integer(50));
        } catch (UnknownPropertyException ex) {
            ex.printStackTrace(log);
            throw new StatusException("Couldn't make shape transparent", ex);
        } catch (PropertyVetoException ex) {
            ex.printStackTrace(log);
            throw new StatusException("Couldn't make shape transparent", ex);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            ex.printStackTrace(log);
            throw new StatusException("Couldn't make shape transparent", ex);
        } catch (WrappedTargetException ex) {
            ex.printStackTrace(log);
            throw new StatusException("Couldn't make shape transparent", ex);
        }

        DrawTools.getDrawPage(xImpressDoc, 0).add(aShape);

        log.println("creating a new environment for drawpage object");

        TestEnvironment tEnv = new TestEnvironment(xImpressDoc);

        log.println("adding Controller as ObjRelation for XModel");
        tEnv.addObjRelation("CONT2", cont2);

        log.println("Adding SelectionSupplier and Shape to select for XModel");
        tEnv.addObjRelation("SELSUPP", sel);
        tEnv.addObjRelation("TOSELECT", aShape);

        // create object relation for XPrintJobBroadcaster
        String fileName = utils.getOfficeTempDirSys((XMultiServiceFactory) Param.getMSF())+"printfile.prt" ;
        File f = new File(fileName);
        if (f.exists()) {
            f.delete();
        }
        _XPrintJobBroadcaster.MyPrintJobListener listener = new _XPrintJobBroadcaster.MyPrintJobListener(xImpressDoc, fileName);
        tEnv.addObjRelation("XPrintJobBroadcaster.XPrintJobListener", listener);

        return tEnv;
    } // finish method getTestEnvironment

} // finish class SdDrawPage
