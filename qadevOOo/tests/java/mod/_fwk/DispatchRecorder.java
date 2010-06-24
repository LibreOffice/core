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

package mod._fwk;

import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XModel;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.DispatchStatement;
import com.sun.star.frame.XDispatchRecorder;
import com.sun.star.frame.XDispatchRecorderSupplier;
import com.sun.star.frame.XFrame;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.util.URL;
import util.utils;

/**
* Test for object that implements the following interfaces :
* <ul>
*  <li><code>com::sun::star::container::XElementAccess</code></li>
*  <li><code>com::sun::star::container::XIndexAccess</code></li>
*  <li><code>com::sun::star::container::XIndexReplace</code></li>
*  <li><code>com::sun::star::frame::XDispatchRecorder</code></li>
* </ul><p>
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XIndexReplace
* @see com.sun.star.frame.XDispatchRecorder
* @see ifc.container._XElementAccess
* @see ifc.container._XIndexAccess
* @see ifc.container._XIndexReplace
* @see ifc.frame._XDispatchRecorder
*/
public class DispatchRecorder extends TestCase {
    XComponent oDoc = null;

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * Creates service <code>com.sun.star.frame.Desktop</code>.
    */
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws StatusException {

        XInterface oObj = null;
        XFrame xFrame = null;
        XDispatchRecorder xDR = null;

        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory)Param.getMSF());
            oDoc = SOF.createTextDoc(null);
            try {
                Thread.sleep(1000);
            }
            catch (InterruptedException ex) {
            }

            XModel model = (XModel) UnoRuntime.queryInterface(XModel.class, oDoc);
            xFrame = model.getCurrentController().getFrame();

            XPropertySet xFramePS = (XPropertySet) UnoRuntime.queryInterface
                (XPropertySet.class, xFrame);
            XDispatchRecorderSupplier xDRS = null;
            xDRS = (XDispatchRecorderSupplier) AnyConverter.toObject(
                             new Type(XDispatchRecorderSupplier.class),
                          xFramePS.getPropertyValue("DispatchRecorderSupplier"));
            if (xDRS == null) {

                Object oDRS = ((XMultiServiceFactory)Param.getMSF()).createInstance(
                    "com.sun.star.comp.framework.DispatchRecorderSupplier");
                xFramePS.setPropertyValue("DispatchRecorderSupplier", oDRS);
                xDRS = (XDispatchRecorderSupplier)
                    UnoRuntime.queryInterface(XDispatchRecorderSupplier.class,oDRS);
            }

            xDR = xDRS.getDispatchRecorder();
            if (xDR != null) {
                oObj = xDR;
            } else {
                oObj = (XInterface)((XMultiServiceFactory)Param.getMSF()).createInstance(
                    "com.sun.star.comp.framework.DispatchRecorder");
                xDR = (XDispatchRecorder) UnoRuntime.queryInterface
                    (XDispatchRecorder.class, oObj);
                xDRS.setDispatchRecorder(xDR);
            }
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create component", e);
        }


        // fill recorder with content. It's needed for XIndexReplace
        URL dispURL = utils.parseURL((XMultiServiceFactory) Param.getMSF(), ".uno:InsertDateField");
        PropertyValue prop = new PropertyValue();
        prop.Name = "Text";
        prop.Value = "XDispatchRecorder.recordDispatch()";
        PropertyValue[] dispArgs = new PropertyValue[] {prop};
        xDR.recordDispatch(dispURL, dispArgs);


        TestEnvironment tEnv = new TestEnvironment( oObj );

        // INSTANCEn : _XIndexReplace
        log.println("adding INSTANCEn as obj relation to environment");

        int THRCNT = 1;
        if (Param.get("THRCNT")!= null) {
            THRCNT = Integer.parseInt((String) Param.get("THRCNT"));
        }

        URL instanceURL = null;
        DispatchStatement instance = new DispatchStatement();
        PropertyValue dispProp = new PropertyValue();

        for (int n = 1; n < (THRCNT + 1); n++) {
            log.println("adding INSTANCE" + n +
                        " as obj relation to environment");
            instanceURL = utils.parseURL((XMultiServiceFactory) Param.getMSF(), ".uno:InsertText");
            dispProp.Name = "Text";
            dispProp.Value = "Instance " + n;
            dispArgs = new PropertyValue[] {dispProp};
            instance.aCommand = instanceURL.Complete;
            instance.aArgs = dispArgs;
            instance.aTarget = "_top";
            instance.nFlags = com.sun.star.frame.FrameSearchFlag.ALL;

            tEnv.addObjRelation("INSTANCE" + n, instance);
        }

        tEnv.addObjRelation("XDispatchRecorder.Frame", xFrame);
        log.println("Object created: TRUE");
        return tEnv;
    } // finish method getTestEnvironment

    protected void cleanup( TestParameters Param, PrintWriter log) {
        util.DesktopTools.closeDoc(oDoc);
    }

}
