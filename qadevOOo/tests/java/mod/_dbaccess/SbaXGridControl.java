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
package mod._dbaccess;

import java.io.PrintWriter;
import java.util.Comparator;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.AccessibilityTools;
import util.FormTools;
import util.SOfficeFactory;
import util.WriterTools;
import util.utils;

import com.sun.star.accessibility.AccessibleRole;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleAction;
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XDevice;
import com.sun.star.awt.XExtendedToolkit;
import com.sun.star.awt.XGraphics;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.XWindow;
import com.sun.star.awt.XWindowPeer;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XControlShape;
import com.sun.star.form.XBoundComponent;
import com.sun.star.form.XGridColumnFactory;
import com.sun.star.form.XLoadable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.text.XTextDocument;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.URL;
import com.sun.star.util.XCloseable;
import com.sun.star.view.XControlAccess;


/**
* Test for object which represents the control of the Grid model. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::util::XModifyBroadcaster</code></li>
*  <li> <code>com::sun::star::form::XGridFieldDataSupplier</code></li>
*  <li> <code>com::sun::star::view::XSelectionSupplier</code></li>
*  <li> <code>com::sun::star::form::XGrid</code></li>
*  <li> <code>com::sun::star::awt::XControl</code></li>
*  <li> <code>com::sun::star::util::XModeSelector</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::awt::XWindow</code></li>
*  <li> <code>com::sun::star::form::XUpdateBroadcaster</code></li>
*  <li> <code>com::sun::star::frame::XDispatch</code></li>
*  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
*  <li> <code>com::sun::star::form::XBoundComponent</code></li>
*  <li> <code>com::sun::star::frame::XDispatchProviderInterception</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::awt::XView</code></li>
*  <li> <code>com::sun::star::container::XContainer</code></li>
* </ul>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
* @see com.sun.star.util.XModifyBroadcaster
* @see com.sun.star.form.XGridFieldDataSupplier
* @see com.sun.star.view.XSelectionSupplier
* @see com.sun.star.form.XGrid
* @see com.sun.star.awt.XControl
* @see com.sun.star.util.XModeSelector
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.awt.XWindow
* @see com.sun.star.form.XUpdateBroadcaster
* @see com.sun.star.frame.XDispatch
* @see com.sun.star.container.XEnumerationAccess
* @see com.sun.star.form.XBoundComponent
* @see com.sun.star.frame.XDispatchProviderInterception
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.lang.XComponent
* @see com.sun.star.awt.XView
* @see com.sun.star.container.XContainer
* @see ifc.util._XModifyBroadcaster
* @see ifc.form._XGridFieldDataSupplier
* @see ifc.view._XSelectionSupplier
* @see ifc.form._XGrid
* @see ifc.awt._XControl
* @see ifc.util._XModeSelector
* @see ifc.container._XElementAccess
* @see ifc.awt._XWindow
* @see ifc.form._XUpdateBroadcaster
* @see ifc.frame._XDispatch
* @see ifc.container._XEnumerationAccess
* @see ifc.form._XBoundComponent
* @see ifc.frame._XDispatchProviderInterception
* @see ifc.container._XIndexAccess
* @see ifc.lang._XComponent
* @see ifc.awt._XView
* @see ifc.container._XContainer
*/
public class SbaXGridControl extends TestCase {
    XTextDocument xTextDoc;

    /**
    * Creates Writer document.
    */
    protected void initialize(TestParameters Param, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory((XMultiServiceFactory)Param.getMSF());

        try {
            log.println("creating a textdocument");
            xTextDoc = SOF.createTextDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
        }
    }

    /**
    * Disposes Writer document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        //closing the appearing dialog before disposing the document
        XInterface toolkit = null;

        try {
            toolkit = (XInterface) ((XMultiServiceFactory)tParam.getMSF())
                                         .createInstance("com.sun.star.awt.Toolkit");
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get toolkit");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get toolkit", e);
        }

        XExtendedToolkit tk = UnoRuntime.queryInterface(
                                      XExtendedToolkit.class, toolkit);

        Object atw = tk.getActiveTopWindow();

        XWindow xWindow = UnoRuntime.queryInterface(XWindow.class,
                                                              atw);

        XAccessible xRoot = AccessibilityTools.getAccessibleObject(xWindow);

        XInterface button = AccessibilityTools.getAccessibleObjectForRole(xRoot,
                                                          AccessibleRole.PUSH_BUTTON);

        XAccessibleAction action = UnoRuntime.queryInterface(
                                           XAccessibleAction.class, button);

        try {
            action.doAccessibleAction(0);
        } catch (com.sun.star.lang.IndexOutOfBoundsException iob) {
            log.println("couldn't close dialog");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close dialog");
        }

        log.println("    disposing xTextDoc ");

        try {
            XCloseable closer = UnoRuntime.queryInterface(
                                        XCloseable.class, xTextDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }

    /**
    * Creating a Testenvironment for the interfaces to be tested.
    * For object creation first a
    * <code>com.sun.star.form.component.GridControl<code> instance
    * is added to the <code>ControlShape</code>. Then this model's
    * control is retrieved.
    *
    *     Object relations created :
    * <ul>
    *  <li> <code>'GRAPHICS'</code> for
    *      {@link ifc.awt._XView} test : <code>XGraphics</code>
    *   object different that belong to the object tested.</li>
    *  <li> <code>'CONTEXT'</code> for
    *      {@link ifc.awt._XControl} </li>
    *  <li> <code>'WINPEER'</code> for
    *      {@link ifc.awt._XControl} </li>
    *  <li> <code>'TOOLKIT'</code> for
    *      {@link ifc.awt._XControl} </li>
    *  <li> <code>'MODEL'</code> for
    *      {@link ifc.awt._XControl} </li>
    *  <li> <code>'XWindow.AnotherWindow'</code> for
    *      {@link ifc.awt._XWindow} for switching focus.</li>
    *  <li> <code>'XDispatch.URL'</code> for
    *      {@link ifc.frame._XDispatch} the url which moves
    *      DB cursor to the next row (".uno:FormSlots/moveToNext").</li>
    *  <li> <code>'XContainer.Container'</code> for
    *      {@link ifc.container._XContainer} as the component created
    *      doesn't support <code>XContainer</code> itself, but
    *      it is supported by its model. So this model is passed.</li>
    *  <li> <code>'INSTANCE'</code> for
    *      {@link ifc.container._XContainer} the instance to be
    *      inserted into collection. Is a column instance.</li>
    * </ul>
    */
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                    PrintWriter log) {
        XInterface oObj = null;
        XWindowPeer the_win = null;
        XToolkit the_kit = null;
        XDevice aDevice = null;
        XGraphics aGraphic = null;
        XPropertySet aControl = null;
        XPropertySet aControl2 = null;
        XPropertySet aControl3 = null;
        XPropertySet aControl4 = null;
        XGridColumnFactory columns = null;

        //Insert a ControlShape and get the ControlModel
        XControlShape aShape = createGrid(xTextDoc, 3000, 4500, 15000, 10000);

        XControlModel the_Model = aShape.getControl();

        WriterTools.getDrawPage(xTextDoc).add(aShape);

        XLoadable formLoader = FormTools.bindForm(xTextDoc);

        //Try to query XControlAccess
        XControlAccess the_access = UnoRuntime.queryInterface(
                                            XControlAccess.class,
                                            xTextDoc.getCurrentController());

        try {
            columns = UnoRuntime.queryInterface(
                              XGridColumnFactory.class, the_Model);
            aControl = columns.createColumn("TextField");
            aControl.setPropertyValue("DataField", "Identifier");
            aControl.setPropertyValue("Label", "Identifier");
            aControl2 = columns.createColumn("TextField");
            aControl2.setPropertyValue("DataField", "Publisher");
            aControl2.setPropertyValue("Label", "Publisher");
            aControl3 = columns.createColumn("TextField");
            aControl3.setPropertyValue("DataField", "Author");
            aControl3.setPropertyValue("Label", "Author");
            aControl4 = columns.createColumn("TextField");
            aControl4.setPropertyValue("DataField", "Title");
            aControl4.setPropertyValue("Label", "Title");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            // Some exception occures.FAILED
            log.println("!!! Couldn't create instance : " + e);
            throw new StatusException("Can't create column instances.", e);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            // Some exception occures.FAILED
            log.println("!!! Couldn't create instance : " + e);
            throw new StatusException("Can't create column instances.", e);
        } catch (com.sun.star.beans.PropertyVetoException e) {
            // Some exception occures.FAILED
            log.println("!!! Couldn't create instance : " + e);
            throw new StatusException("Can't create column instances.", e);
        } catch (com.sun.star.beans.UnknownPropertyException e) {
            // Some exception occures.FAILED
            log.println("!!! Couldn't create instance : " + e);
            throw new StatusException("Can't create column instances.", e);
        }

        XNameContainer aContainer = UnoRuntime.queryInterface(
                                            XNameContainer.class, the_Model);

        try {
            aContainer.insertByName("First", aControl);
            aContainer.insertByName("Second", aControl2);
        } catch (com.sun.star.uno.Exception e) {
            log.println("!!! Could't insert column Instance");
            e.printStackTrace(log);
            throw new StatusException("Can't insert columns", e);
        }

        //now get the OGridControl
        try {
            oObj = the_access.getControl(the_Model);
            the_win = the_access.getControl(the_Model).getPeer();
            the_kit = the_win.getToolkit();
            aDevice = the_kit.createScreenCompatibleDevice(200, 200);
            aGraphic = aDevice.createGraphics();
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get GridControl");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get GridControl", e);
        }


        // creating another window
        aShape = FormTools.createControlShape(xTextDoc, 3000, 4500, 15000,
                                              10000, "TextField");

        WriterTools.getDrawPage(xTextDoc).add(aShape);

        the_Model = aShape.getControl();


        //Try to query XControlAccess
        the_access = UnoRuntime.queryInterface(
                             XControlAccess.class,
                             xTextDoc.getCurrentController());

        //now get the TextControl
        XWindow win = null;
        Object cntrl = null;

        try {
            cntrl = the_access.getControl(the_Model);
            win = UnoRuntime.queryInterface(XWindow.class, cntrl);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't get Control");
            e.printStackTrace(log);
            throw new StatusException("Couldn't get Control", e);
        }

        log.println("creating a new environment for object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        //Relations for XSelectionSupplier
        tEnv.addObjRelation("Selections",
                            new Object[] {
            new Object[] { new Integer(0) }, new Object[] { new Integer(1) }
        });
        tEnv.addObjRelation("Comparer",
                            new Comparator<Integer>() {
            public int compare(Integer o1, Integer o2) {
                return o1.compareTo(o2);
            }
        });


        //Realtion for XContainer
        tEnv.addObjRelation("XContainer.Container", aContainer);
        tEnv.addObjRelation("INSTANCE", aControl3);
        tEnv.addObjRelation("INSTANCE2", aControl4);


        //Adding ObjRelation for XView
        tEnv.addObjRelation("GRAPHICS", aGraphic);


        //Adding ObjRelation for XControl
        tEnv.addObjRelation("CONTEXT", xTextDoc);
        tEnv.addObjRelation("WINPEER", the_win);
        tEnv.addObjRelation("TOOLKIT", the_kit);
        tEnv.addObjRelation("MODEL", the_Model);


        // Adding relation for XWindow
        tEnv.addObjRelation("XWindow.AnotherWindow", win);

        // Adding relation for XDispatch
        URL url = new URL();
        url.Complete = ".uno:FormSlots/moveToNext";


        //url.Complete = ".uno:GridSlots/RowHeight";
        //url.Complete = ".uno:GridSlots/RowHeight" ;
        tEnv.addObjRelation("XDispatch.URL", url);

        log.println("ImplName: " + utils.getImplName(oObj));

        FormTools.switchDesignOf((XMultiServiceFactory)Param.getMSF(), xTextDoc);

        // adding relation for XUpdateBroadcaster
        final XInterface ctrl = oObj;
        final XLoadable formLoaderF = formLoader;
        final XPropertySet ps = UnoRuntime.queryInterface(
                                        XPropertySet.class, aControl2);
        tEnv.addObjRelation("XUpdateBroadcaster.Checker",
                            new ifc.form._XUpdateBroadcaster.UpdateChecker() {
            private String lastText = "";

            public void update() throws com.sun.star.uno.Exception {
                if (!formLoaderF.isLoaded()) {
                    formLoaderF.load();
                }

                lastText = "_" + ps.getPropertyValue("Text");
                ps.setPropertyValue("Text", lastText);
            }

            public void commit() throws com.sun.star.sdbc.SQLException {
                XBoundComponent bound = UnoRuntime.queryInterface(
                                                XBoundComponent.class, ctrl);
                XResultSetUpdate update = UnoRuntime.queryInterface(
                                                  XResultSetUpdate.class,
                                                  formLoaderF);

                bound.commit();
                update.updateRow();
            }

            public boolean wasCommited() throws com.sun.star.uno.Exception {
                String getS = (String) ps.getPropertyValue("Text");

                return lastText.equals(getS);
            }
        });

        return tEnv;
    } // finish method getTestEnvironment

    public static XControlShape createGrid(XComponent oDoc, int height,
                                           int width, int x, int y) {
        Size size = new Size();
        Point position = new Point();
        XControlShape oCShape = null;
        XControlModel aControl = null;

        //get MSF
        XMultiServiceFactory oDocMSF = UnoRuntime.queryInterface(
                                               XMultiServiceFactory.class,
                                               oDoc);

        try {
            Object oInt = oDocMSF.createInstance(
                                  "com.sun.star.drawing.ControlShape");
            Object aCon = oDocMSF.createInstance(
                                  "com.sun.star.form.component.GridControl");
            XPropertySet model_props = UnoRuntime.queryInterface(
                                               XPropertySet.class, aCon);
            model_props.setPropertyValue("DefaultControl",
                                         "com.sun.star.form.control.InteractionGridControl");
            aControl = UnoRuntime.queryInterface(
                               XControlModel.class, aCon);
            oCShape = UnoRuntime.queryInterface(
                              XControlShape.class, oInt);
            size.Height = height;
            size.Width = width;
            position.X = x;
            position.Y = y;
            oCShape.setSize(size);
            oCShape.setPosition(position);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            System.out.println("Couldn't create Grid" + e);
            throw new StatusException("Couldn't create Grid", e);
        }

        oCShape.setControl(aControl);

        return oCShape;
    } // finish createGrid
}