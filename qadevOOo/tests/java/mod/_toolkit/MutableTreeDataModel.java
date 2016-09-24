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

import com.sun.star.awt.tree.XMutableTreeDataModel;
import com.sun.star.awt.tree.XMutableTreeNode;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.WriterTools;
import util.utils;


public class MutableTreeDataModel extends TestCase {
    private static XTextDocument xTextDoc;
    private static XInterface oObj = null;

    /**
     * Creates StarOffice Writer document.
     */
    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {
        log.println("creating a textdocument");
        xTextDoc = WriterTools.createTextDoc(
            tParam.getMSF());
    }

    /**
     * Disposes StarOffice Writer document.
     */
    @Override
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        util.DesktopTools.closeDoc(xTextDoc);
    }

    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
        PrintWriter log) {

        try {
            oObj = (XInterface) Param.getMSF().createInstance(
                "com.sun.star.awt.tree.MutableTreeDataModel");
        } catch (Exception e) {
        }

        log.println(
            "creating a new environment for MutableTreeDataModel object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("OBJNAME", "toolkit.MutableTreeDataModel");
        log.println("ImplementationName: " + utils.getImplName(oObj));

        tEnv.addObjRelation("XTreeDataModelListenerEvent", new XTreeDataModelListenerEvent());

        return tEnv;
    } // finish method getTestEnvironment

    private class XTreeDataModelListenerEvent implements ifc.awt.tree._XTreeDataModel.XTreeDataModelListenerEvent{

        public void fireEvent(){

            XMutableTreeDataModel xModel = UnoRuntime.queryInterface(XMutableTreeDataModel.class, oObj);
            XMutableTreeNode node = xModel.createNode("EventNode", true);
            try {
                xModel.setRoot(node);
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not perform event: " + ex.toString());
                ex.printStackTrace();
            }

        }
    }
} // finish class UnoControlListBoxModel
