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
    @Override
    public TestEnvironment createTestEnvironment( TestParameters Param,
        PrintWriter log ) throws StatusException {

        XInterface oObj = null;
        XFrame xFrame = null;
        XDispatchRecorder xDR = null;

        try {
            SOfficeFactory SOF = SOfficeFactory.getFactory(Param.getMSF());
            oDoc = SOF.createTextDoc(null);
            util.utils.pause(1000);

            XModel model = UnoRuntime.queryInterface(XModel.class, oDoc);
            xFrame = model.getCurrentController().getFrame();

            XPropertySet xFramePS = UnoRuntime.queryInterface
                (XPropertySet.class, xFrame);
            XDispatchRecorderSupplier xDRS = null;
            xDRS = (XDispatchRecorderSupplier) AnyConverter.toObject(
                             new Type(XDispatchRecorderSupplier.class),
                          xFramePS.getPropertyValue("DispatchRecorderSupplier"));
            if (xDRS == null) {

                Object oDRS = Param.getMSF().createInstance(
                    "com.sun.star.comp.framework.DispatchRecorderSupplier");
                xFramePS.setPropertyValue("DispatchRecorderSupplier", oDRS);
                xDRS = UnoRuntime.queryInterface(XDispatchRecorderSupplier.class,oDRS);
            }

            xDR = xDRS.getDispatchRecorder();
            if (xDR != null) {
                oObj = xDR;
            } else {
                oObj = (XInterface)Param.getMSF().createInstance(
                    "com.sun.star.comp.framework.DispatchRecorder");
                xDR = UnoRuntime.queryInterface
                    (XDispatchRecorder.class, oObj);
                xDRS.setDispatchRecorder(xDR);
            }
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create component", e);
        }


        // fill recorder with content. It's needed for XIndexReplace
        URL dispURL = utils.parseURL(Param.getMSF(), ".uno:InsertDateField");
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
            instanceURL = utils.parseURL(Param.getMSF(), ".uno:InsertText");
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

    @Override
    protected void cleanup( TestParameters Param, PrintWriter log) {
        util.DesktopTools.closeDoc(oDoc);
    }

}
