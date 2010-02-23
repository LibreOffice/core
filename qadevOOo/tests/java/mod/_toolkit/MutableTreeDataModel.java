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

import com.sun.star.awt.tree.XMutableTreeDataModel;
import com.sun.star.awt.tree.XMutableTreeNode;
import com.sun.star.lang.XMultiServiceFactory;
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
    protected void initialize(TestParameters tParam, PrintWriter log) {
        log.println("creating a textdocument");
        xTextDoc = WriterTools.createTextDoc(
            (XMultiServiceFactory) tParam.getMSF());
    }

    /**
     * Disposes StarOffice Writer document.
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        util.DesktopTools.closeDoc(xTextDoc);
    }

    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
        PrintWriter log) {

        try {
            oObj = (XInterface) ((XMultiServiceFactory) Param.getMSF()).createInstance(
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

    public class XTreeDataModelListenerEvent implements ifc.awt.tree._XTreeDataModel.XTreeDataModelListenerEvent{

        public void fireEvent(){

            XMutableTreeDataModel xModel = (XMutableTreeDataModel) UnoRuntime.queryInterface(XMutableTreeDataModel.class, oObj);
            XMutableTreeNode node = xModel.createNode("EventNode", true);
            try {
                xModel.setRoot(node);
            } catch (com.sun.star.lang.IllegalArgumentException ex) {
                log.println("ERROR: could not preform event: " + ex.toString());
                ex.printStackTrace();
            }

        }
    }
} // finish class UnoControlListBoxModel
