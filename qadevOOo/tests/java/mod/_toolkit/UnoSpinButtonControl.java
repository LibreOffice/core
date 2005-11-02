/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: UnoSpinButtonControl.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 18:26:30 $
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

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XShape;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;
import com.sun.star.view.XControlAccess;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

import util.FormTools;
import util.WriterTools;
import util.utils;


public class UnoSpinButtonControl extends TestCase {
    private static XTextDocument xTextDoc;

    protected void initialize(TestParameters Param, PrintWriter log) {
        log.println("creating a textdocument");
        xTextDoc = WriterTools.createTextDoc(
                           (XMultiServiceFactory) Param.getMSF());
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        util.DesktopTools.closeDoc(xTextDoc);
    }

    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) {
        XInterface oObj = null;
        XWindowPeer the_win = null;
        XToolkit the_kit = null;
        XDevice aDevice = null;
        XGraphics aGraphic = null;
        XControl aControl = null;

        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createUnoControlShape(xTextDoc, 3000,
                                                               4500, 15000,
                                                               10000,
                                                               "SpinButton",
                                                               "UnoControlSpinButton");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        XControlModel the_Model = aShape.getControl();

        XControlShape aShape2 = FormTools.createControlShape(xTextDoc, 3000,
                                                             4500, 5000, 10000,
                                                             "TextField");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape2);

        XControlModel the_Model2 = aShape2.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = (XControlAccess) UnoRuntime.queryInterface(
                                            XControlAccess.class,
                                            xTextDoc.getCurrentController());

        //get the SpinButtonControl for the needed Object relations
        try {
            oObj = the_access.getControl(the_Model);
            aControl = the_access.getControl(the_Model2);
            the_win = the_access.getControl(the_Model).getPeer();
            the_kit = the_win.getToolkit();
            aDevice = the_kit.createScreenCompatibleDevice(200, 200);
            aGraphic = aDevice.createGraphics();
        } catch (Exception e) {
            log.println("Couldn't get SpinButtonControl");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get SpinButtonControl", e);
        }

        log.println(
                "creating a new environment for UnoControlSpinButton object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        //adding Object-Relation for XScrollBar
        tEnv.addObjRelation("Document", xTextDoc);


        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS", aGraphic);


        //Adding ObjRelation for XControl
        tEnv.addObjRelation("CONTEXT", xTextDoc);
        tEnv.addObjRelation("WINPEER", the_win);
        tEnv.addObjRelation("TOOLKIT", the_kit);
        tEnv.addObjRelation("MODEL", the_Model);

        XWindow forObjRel = (XWindow) UnoRuntime.queryInterface(XWindow.class,
                                                                aControl);

        tEnv.addObjRelation("XWindow.AnotherWindow", forObjRel);

        tEnv.addObjRelation("Document", xTextDoc);

        System.out.println("ImplementationName: " + utils.getImplName(oObj));

        return tEnv;
    } // finish method getTestEnvironment
}