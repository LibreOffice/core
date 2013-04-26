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


package mod._basctl;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.awt.PosSize;
import com.sun.star.awt.Rectangle;
import com.sun.star.awt.XWindow;
import com.sun.star.beans.PropertyValue;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XDispatchHelper;
import com.sun.star.frame.XDispatchProvider;
import com.sun.star.frame.XFrame;
import com.sun.star.frame.XModel;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.DesktopTools;
import util.WriterTools;
import util.utils;

public class AccessibleShape extends TestCase {

    XTextDocument xTextDoc = null;
    XInterface oObj = null;

    protected void cleanup(TestParameters Param, PrintWriter log) {
        log.println("Cleaning up");
        DesktopTools.closeDoc(xTextDoc);
        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory) Param.getMSF();
            Object o = xMSF.createInstance("com.sun.star.frame.Desktop");
            XDesktop xDesk = UnoRuntime.queryInterface(XDesktop.class, o);
            DesktopTools.closeDoc(xDesk.getCurrentFrame());
        } catch (Exception e) {
            log.println("Couldn't close IDE");
        }
    }

    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
        log.println( "creating a test environment" );
        String aURL=utils.getFullTestURL("basDialog.odt");
        xTextDoc = WriterTools.loadTextDoc(xMSF,aURL);
        XModel xModel = UnoRuntime.queryInterface(XModel.class, xTextDoc);
        XFrame xFrame = xModel.getCurrentController().getFrame();
        XDispatchProvider xDPP = UnoRuntime.queryInterface(XDispatchProvider.class, xFrame);

        log.println( "opening the basic dialog editor" );
        try {
            Object o = xMSF.createInstance("com.sun.star.frame.DispatchHelper");
            XDispatchHelper xDPH = UnoRuntime.queryInterface(XDispatchHelper.class, o);
            PropertyValue[] aArgs = new PropertyValue[4];
            aArgs[0] = new PropertyValue();
            aArgs[0].Name = "Document";
            aArgs[0].Value = aURL;
            aArgs[1] = new PropertyValue();
            aArgs[1].Name = "LibName";
            aArgs[1].Value = "basctl";
            aArgs[2] = new PropertyValue();
            aArgs[2].Name = "Name";
            aArgs[2].Value = "Dialog1";
            aArgs[3] = new PropertyValue();
            aArgs[3].Name = "Type";
            aArgs[3].Value = "Dialog";
            xDPH.executeDispatch(xDPP, ".uno:BasicIDEAppear", "", 0, aArgs);
        } catch (Exception e) {
            throw new StatusException("Couldn't open Basic Dialog",e);
        }

        utils.shortWait(3000);

        try {
            oObj = (XInterface) ((XMultiServiceFactory)tParam.getMSF()).createInstance
                    ("com.sun.star.awt.Toolkit") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e );
        }

        AccessibilityTools at = new AccessibilityTools();

        final XWindow basicIDE = xFrame.getContainerWindow();

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(basicIDE);

        AccessibilityTools.printAccessibleTree(log, xRoot, tParam.getBool(util.PropertyName.DEBUG_IS_ACTIVE));

        oObj = AccessibilityTools.getAccessibleObjectForRole(xRoot, AccessibleRole.SHAPE);

        // create test environment here
        TestEnvironment tEnv = new TestEnvironment( oObj );

        log.println("Implementation Name: " + utils.getImplName(oObj));

        tEnv.addObjRelation("Destroy", Boolean.TRUE);

        tEnv.addObjRelation("EventProducer",
                new ifc.accessibility._XAccessibleEventBroadcaster.EventProducer() {
            public void fireEvent() {
                Rectangle oldPosSize = basicIDE.getPosSize();
                Rectangle newPosSize = new Rectangle();
                newPosSize.Width = oldPosSize.Width/2;
                newPosSize.Height = oldPosSize.Height/2;
                newPosSize.X = oldPosSize.X + 20;
                newPosSize.Y = oldPosSize.Y + 20;
                basicIDE.setPosSize(newPosSize.X, newPosSize.Y, newPosSize.Width,
                                newPosSize.Height, PosSize.POSSIZE);
                utils.shortWait(1000);
                basicIDE.setPosSize(oldPosSize.X, oldPosSize.Y, oldPosSize.Width,
                                oldPosSize.Height, PosSize.POSSIZE);
            }
        });

        return tEnv;
    }



}
