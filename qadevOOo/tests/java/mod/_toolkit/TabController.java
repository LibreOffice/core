/*************************************************************************
 *
 *  $RCSfile: TabController.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-11-18 16:31:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package mod._toolkit;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.WriterTools;
import util.utils;

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

public class TabController extends TestCase{

    XTextDocument xTextDoc = null;

    protected void initialize(TestParameters param, PrintWriter log) {
        try {
            log.println( "creating a textdocument" );
            xTextDoc = WriterTools.createTextDoc((XMultiServiceFactory)param.getMSF());
        } catch ( Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    protected void cleanup(TestParameters param, PrintWriter log) {
        log.println( "disposing xTextDoc" );
        xTextDoc.dispose();
    }

    public TestEnvironment createTestEnvironment(TestParameters param,
                                                    PrintWriter log) {
        XInterface oObj = null;
        XControl xCtrl1 = null;
        XTabControllerModel tabCtrlModel = null;
        XControlContainer aCtrlContainer = null;

        // create object relations
        FormTools.insertForm(xTextDoc,
                FormTools.getForms(WriterTools.getDrawPage(xTextDoc)),"MyForm");

        XControlShape aShape = FormTools.createUnoControlShape(
                                xTextDoc,3000,4500,15000,10000,"CommandButton",
                                "UnoControlButton");
        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        XControlModel model = aShape.getControl();
        XControlAccess access = (XControlAccess) UnoRuntime.queryInterface
                (XControlAccess.class,xTextDoc.getCurrentController());

        try {
            xCtrl1 = access.getControl(model);
        } catch (Exception e) {}

        XForm form = null;
        try {
            form = (XForm) AnyConverter.toObject(new Type(XForm.class),
                             (FormTools.getForms(
                        WriterTools.getDrawPage(xTextDoc))).getByName("MyForm"));
        }
        catch ( Exception e ) {
            log.println("Couldn't get Form");
            e.printStackTrace(log);
        }

        tabCtrlModel = (XTabControllerModel) UnoRuntime.
                        queryInterface(XTabControllerModel.class, form);

        aCtrlContainer = (XControlContainer)
                        UnoRuntime.queryInterface(XControlContainer.class,
                        xCtrl1.getContext());
        // create object
        try {
            oObj = (XInterface)((XMultiServiceFactory)param.getMSF()).
                            createInstance ("com.sun.star.awt.TabController");
        } catch (Exception e) {}

        TestEnvironment tEnv = new TestEnvironment(oObj);

        String objName = "TabController";
        tEnv.addObjRelation("OBJNAME", "toolkit." + objName);
        tEnv.addObjRelation("MODEL", tabCtrlModel);
        tEnv.addObjRelation("CONTAINER", aCtrlContainer);
        System.out.println("ImplementationName: " + utils.getImplName(oObj ));
        return tEnv;
    }
}

