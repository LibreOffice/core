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

import com.sun.star.drawing.XControlShape;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.WriterTools;
import util.utils;


public class TabControllerModel extends TestCase {
    private static XTextDocument xTextDoc;

    @Override
    protected void initialize(TestParameters Param, PrintWriter log) throws Exception {
        log.println("creating a textdocument");
        xTextDoc = WriterTools.createTextDoc(Param.getMSF());
    }

    @Override
    protected void cleanup(TestParameters param, PrintWriter log) {
        log.println("disposing xTextDoc");
        util.DesktopTools.closeDoc(xTextDoc);
    }

    @Override
    public TestEnvironment createTestEnvironment(TestParameters param,
                                                 PrintWriter log) throws Exception {
        XInterface oObj = null;

        log.println("inserting some ControlShapes");

        XControlShape shape1 = FormTools.createControlShape(xTextDoc, 3000,
                                                            4500, 15000, 1000,
                                                            "CommandButton");
        XControlShape shape2 = FormTools.createControlShape(xTextDoc, 5000,
                                                            3500, 7500, 5000,
                                                            "TextField");

        oObj = (XInterface) param.getMSF().createInstance(
                       "com.sun.star.awt.TabControllerModel");

        log.println("creating a new environment for TabControllerModel");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("OBJNAME",
                            "stardiv.vcl.controlmodel.TabController");
        tEnv.addObjRelation("Model1", shape1.getControl());
        tEnv.addObjRelation("Model2", shape2.getControl());


        //Object Relation for XPersistObject
        tEnv.addObjRelation("noPS", Boolean.TRUE);

        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment
} // finish class TabControllerModel
