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
        ;
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
        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        XControlModel model = aShape.getControl();
        XControlAccess access = (XControlAccess) UnoRuntime.queryInterface(
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

        tabCtrlModel = (XTabControllerModel) UnoRuntime.queryInterface(
                               XTabControllerModel.class, form);

        aCtrlContainer = (XControlContainer) UnoRuntime.queryInterface(
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