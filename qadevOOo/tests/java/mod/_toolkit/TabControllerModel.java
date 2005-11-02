/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TabControllerModel.java,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 18:20:03 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
package mod._toolkit;

import com.sun.star.drawing.XControlShape;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.XInterface;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.FormTools;
import util.WriterTools;
import util.utils;


public class TabControllerModel extends TestCase {
    private static XTextDocument xTextDoc;

    protected void initialize(TestParameters param, PrintWriter log) {
        try {
            log.println("creating a textdocument");
            xTextDoc = WriterTools.createTextDoc(
                               (XMultiServiceFactory) param.getMSF());
        } catch (Exception e) {
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

        log.println("inserting some ControlShapes");

        XControlShape shape1 = FormTools.createControlShape(xTextDoc, 3000,
                                                            4500, 15000, 1000,
                                                            "CommandButton");
        XControlShape shape2 = FormTools.createControlShape(xTextDoc, 5000,
                                                            3500, 7500, 5000,
                                                            "TextField");

        try {
            oObj = (XInterface) ((XMultiServiceFactory) param.getMSF()).createInstance(
                           "com.sun.star.awt.TabControllerModel");
        } catch (Exception e) {
            e.printStackTrace(log);
            throw new StatusException("Couldn't create " +
                                      "TabControllerModel", e);
        }

        log.println("creating a new environment for TabControllerModel");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        tEnv.addObjRelation("OBJNAME",
                            "stardiv.vcl.controlmodel.TabController");
        tEnv.addObjRelation("Model1", shape1.getControl());
        tEnv.addObjRelation("Model2", shape2.getControl());


        //Object Relation for XPersistObject
        tEnv.addObjRelation("noPS", new Boolean(true));

        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment
} // finish class TabControllerModel
