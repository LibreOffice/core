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
package mod._toolkit;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XTabControllerModel;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.form.XForm;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.view.XControlAccess;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.FormTools;
import util.WriterTools;
import util.utils;


public class TabController extends TestCase {
    private static XTextDocument xTextDoc = null;

    protected void initialize(TestParameters param, PrintWriter log) {
        try {
            log.println("creating a textdocument");
            xTextDoc = WriterTools.createTextDoc(
                               (XMultiServiceFactory) param.getMSF());
        } catch (Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    protected void cleanup(TestParameters param, PrintWriter log) {
        log.println("disposing xTextDoc");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    public TestEnvironment createTestEnvironment(TestParameters param,
                                                 PrintWriter log) {
        XInterface oObj = null;
        XControl xCtrl1 = null;
        XTabControllerModel tabCtrlModel = null;
        XControlContainer aCtrlContainer = null;


        // create object relations
        FormTools.insertForm(xTextDoc,
                             FormTools.getForms(WriterTools.getDrawPage(
                                                        xTextDoc)), "MyForm");

        XControlShape aShape = FormTools.createUnoControlShape(xTextDoc, 3000,
                                                               4500, 15000,
                                                               10000,
                                                               "CommandButton",
                                                               "UnoControlButton");
        WriterTools.getDrawPage(xTextDoc).add(aShape);

        XControlModel model = aShape.getControl();
        XControlAccess access = UnoRuntime.queryInterface(
                                        XControlAccess.class,
                                        xTextDoc.getCurrentController());

        try {
            xCtrl1 = access.getControl(model);
        } catch (Exception e) {
        }

        XForm form = null;

        try {
            form = (XForm) AnyConverter.toObject(new Type(XForm.class),
                                                 (FormTools.getForms(
                                                         WriterTools.getDrawPage(
                                                                 xTextDoc)))
                                                     .getByName("MyForm"));
        } catch (Exception e) {
            log.println("Couldn't get Form");
            e.printStackTrace(log);
        }

        tabCtrlModel = UnoRuntime.queryInterface(
                               XTabControllerModel.class, form);

        aCtrlContainer = UnoRuntime.queryInterface(
                                 XControlContainer.class, xCtrl1.getContext());

        // create object
        try {
            oObj = (XInterface) ((XMultiServiceFactory) param.getMSF()).createInstance(
                           "com.sun.star.awt.TabController");
        } catch (Exception e) {
        }

        TestEnvironment tEnv = new TestEnvironment(oObj);

        String objName = "TabController";
        tEnv.addObjRelation("OBJNAME", "toolkit." + objName);
        tEnv.addObjRelation("MODEL", tabCtrlModel);
        tEnv.addObjRelation("CONTAINER", aCtrlContainer);
        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    }
}