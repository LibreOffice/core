/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TabController.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: kz $ $Date: 2005-11-02 18:19:51 $
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