/*************************************************************************
 *
 *  $RCSfile: OListBoxControl.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change:$Date: 2003-09-08 11:50:07 $
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

package mod._forms;

import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.SOfficeFactory;
import util.WriterTools;

import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XListBox;
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

public class OListBoxControl extends TestCase {

    XTextDocument xTextDoc;

    protected void initialize ( TestParameters Param, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory( ((XMultiServiceFactory) Param.getMSF()) );

        try {
            log.println( "creating a textdocument" );
            xTextDoc = SOF.createTextDoc( null );
        } catch ( com.sun.star.uno.Exception e ) {
            // Some exception occures.FAILED
            e.printStackTrace( log );
            throw new StatusException( "Couldn't create document", e );
        }
    }

    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = (XCloseable) UnoRuntime.queryInterface(
                                        XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }

    protected TestEnvironment createTestEnvironment(TestParameters Param, PrintWriter log) {
        XInterface oObj = null;
        Object anotherCtrl = null ;
        XWindowPeer the_win = null;
        XToolkit the_kit = null;
        XDevice aDevice = null;
        XGraphics aGraphic = null;
        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = FormTools.createControlShape(
                                xTextDoc,3000,4500,15000,10000,"ListBox");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape);

        XControlModel the_Model = aShape.getControl();

        XControlShape aShape2 = FormTools.createControlShape(
                                xTextDoc,3000,4500,5000,10000,"TextField");

        WriterTools.getDrawPage(xTextDoc).add((XShape) aShape2);

        XControlModel the_Model2 = aShape2.getControl();

        //Try to query XControlAccess
        XControlAccess the_access = (XControlAccess) UnoRuntime.queryInterface(
                        XControlAccess.class,xTextDoc.getCurrentController());

        //now get the OListBoxControl
        try {
            oObj = the_access.getControl(the_Model);
            anotherCtrl = the_access.getControl(the_Model2);
            the_win = the_access.getControl(the_Model).getPeer();
            the_kit = the_win.getToolkit();
            aDevice = the_kit.createScreenCompatibleDevice(200,200);
            aGraphic = aDevice.createGraphics();
        } catch (Exception e) {
            log.println("Couldn't get OListBoxControl");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get OListBoxControl", e );
        }

        log.println( "creating a new environment for OListBoxControl object" );
        TestEnvironment tEnv = new TestEnvironment( oObj );

        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS",aGraphic);

        //Adding ObjRelation for XControl
        tEnv.addObjRelation("CONTEXT",xTextDoc);
        tEnv.addObjRelation("WINPEER",the_win);
        tEnv.addObjRelation("TOOLKIT",the_kit);
        tEnv.addObjRelation("MODEL",the_Model);

        // Adding relation for XItemListener
        ifc.awt._XItemListener.TestItemListener listener =
            new ifc.awt._XItemListener.TestItemListener() ;
        final XListBox box = (XListBox) UnoRuntime.queryInterface(XListBox.class, oObj) ;
        box.addItemListener(listener) ;
        tEnv.addObjRelation("TestItemListener", listener) ;

        // Adding relation for XWindow
        XWindow forObjRel = (XWindow)
                            UnoRuntime.queryInterface(XWindow.class, anotherCtrl);

        XWindow objWin = (XWindow)
                            UnoRuntime.queryInterface(XWindow.class, oObj);

        tEnv.addObjRelation("XWindow.AnotherWindow",forObjRel);
        tEnv.addObjRelation("XWindow.ControlShape",aShape);

        tEnv.addObjRelation("Win1",objWin);
        tEnv.addObjRelation("Win2",forObjRel);

        tEnv.addObjRelation("CONTROL",anotherCtrl);

        // adding relation for XChangeBroadcaster
        box.addItem("Item1", (short) 0);
        box.addItem("Item2", (short) 1);

        tEnv.addObjRelation("XChangeBroadcaster.Changer",
            new ifc.form._XChangeBroadcaster.Changer() {
                public void change(){
                    box.addItem("Item1", (short) 0);
                    box.addItem("Item2", (short) 1);
                    box.selectItemPos((short) 0, true);
                    box.selectItemPos((short) 1, true);
                }
            }
        );

        return tEnv;
    } // finish method getTestEnvironment

}    // finish class OListBoxControl

