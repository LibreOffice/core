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
package mod._forms;

import java.io.PrintWriter;
import java.util.Comparator;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.FormTools;
import util.SOfficeFactory;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameContainer;
import com.sun.star.container.XNamed;
import com.sun.star.drawing.XControlShape;
import com.sun.star.form.XGridColumnFactory;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
* Test for object which is represented by service
* <code>com.sun.star.form.component.GridControl</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::io::XPersistObject</code></li>
*  <li> <code>com::sun::star::container::XNameReplace</code></li>
*  <li> <code>com::sun::star::form::XReset</code></li>
*  <li> <code>com::sun::star::script::XEventAttacherManager</code></li>
*  <li> <code>com::sun::star::form::FormComponent</code></li>
*  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
*  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::form::component::GridControl</code></li>
*  <li> <code>com::sun::star::view::XSelectionSupplier</code></li>
*  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
*  <li> <code>com::sun::star::beans::XPropertyState</code></li>
*  <li> <code>com::sun::star::form::FormControlModel</code></li>
*  <li> <code>com::sun::star::container::XIndexReplace</code></li>
*  <li> <code>com::sun::star::container::XNamed</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XNameContainer</code></li>
*  <li> <code>com::sun::star::form::XGridColumnFactory</code></li>
*  <li> <code>com::sun::star::lang::XComponent</code></li>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
*  <li> <code>com::sun::star::beans::XPropertyAccess</code></li>
*  <li> <code>com::sun::star::beans::XPropertyContainer</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
*  <li> <code>com::sun::star::container::XIndexContainer</code></li>
*  <li> <code>com::sun::star::container::XChild</code></li>
*  <li> <code>com::sun::star::container::XContainer</code></li>
* </ul> <p>
* This object test <b> is NOT </b> designed to be run in several
* threads concurently.
*
* @see com.sun.star.io.XPersistObject
* @see com.sun.star.container.XNameReplace
* @see com.sun.star.form.XReset
* @see com.sun.star.script.XEventAttacherManager
* @see com.sun.star.form.FormComponent
* @see com.sun.star.beans.XFastPropertySet
* @see com.sun.star.beans.XMultiPropertySet
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.form.component.GridControl
* @see com.sun.star.view.XSelectionSupplier
* @see com.sun.star.container.XEnumerationAccess
* @see com.sun.star.beans.XPropertyState
* @see com.sun.star.form
* @see com.sun.star.container.XIndexReplace
* @see com.sun.star.container.XNamed
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XNameContainer
* @see com.sun.star.form.XGridColumnFactory
* @see com.sun.star.lang.XComponent
* @see com.sun.star.container.XNameAccess
* @see com.sun.star.beans.XPropertyAccess
* @see com.sun.star.beans.XPropertyContainer
* @see com.sun.star.beans.XPropertySet
* @see com.sun.star.container.XIndexContainer
* @see com.sun.star.container.XChild
* @see com.sun.star.container.XContainer
* @see ifc.io._XPersistObject
* @see ifc.container._XNameReplace
* @see ifc.form._XReset
* @see ifc.script._XEventAttacherManager
* @see ifc.form._FormComponent
* @see ifc.beans._XFastPropertySet
* @see ifc.beans._XMultiPropertySet
* @see ifc.container._XElementAccess
* @see ifc.form.component._GridControl
* @see ifc.view._XSelectionSupplier
* @see ifc.container._XEnumerationAccess
* @see ifc.beans._XPropertyState
* @see ifc.form._FormControlModel
* @see ifc.container._XIndexReplace
* @see ifc.container._XNamed
* @see ifc.container._XIndexAccess
* @see ifc.container._XNameContainer
* @see ifc.form._XGridColumnFactory
* @see ifc.lang._XComponent
* @see ifc.container._XNameAccess
* @see ifc.beans._XPropertySet
* @see ifc.container._XIndexContainer
* @see ifc.container._XChild
* @see ifc.container._XContainer
*/

public class OGridControlModel extends GenericModelTest {

    /**
     * Set some member variable of the super class <CODE>GenericModelTest</CODE>:
     * <pre>
     *    super.m_kindOfControl="CommandButton";
     *    super.m_ObjectName = "com.sun.star.form.component.CommandButton";
     *    super.m_LCShape_Type = "CommandButton";
     * </pre>
     * Then <CODE>super.initialize()</CODE> was called.
     * @param tParam the test parameter
     * @param log the log writer
     */

    protected void initialize(TestParameters tParam, PrintWriter log) {

        super.initialize(tParam, log);

        super.m_kindOfControl="GridControl";

        super.m_ObjectName = "stardiv.one.form.component.Grid";

        super.m_LCShape_Type = "GridControl";

    }
    /**
     * calls <CODE>cleanup()</CODE> from it's super class
     * @param tParam the test parameter
     * @param log the log writer
     */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        super.cleanup(tParam, log);
    }


    /**
     * calls <CODE>createTestEnvironment()</CODE> from it's super class
     * @param Param the test parameter
     * @param log the log writer
     * @return lib.TestEnvironment
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
            PrintWriter log) {
        TestEnvironment tEnv = super.createTestEnvironment(Param, log);

        XInterface oObj = tEnv.getTestObject();

        XPropertySet aControl = null;
        XPropertySet aControl2 = null;
        XPropertySet aControl3 = null;
        XPropertySet aControl4 = null;
        XPropertySet aControl5 = null;

        try {
            XGridColumnFactory columns = UnoRuntime.queryInterface(
                              XGridColumnFactory.class, oObj);
            aControl = columns.createColumn("TextField");
            aControl2 = columns.createColumn("DateField");
            aControl3 = columns.createColumn("TextField");
            aControl4 = columns.createColumn("TextField");
            aControl5 = columns.createColumn("TextField");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            // Some exception occures.FAILED
            log.println("!!! Couldn't create instance : " + e);
            throw new StatusException("Can't create column instances.", e);
        }

        XNameContainer aContainer = UnoRuntime.queryInterface(
                                            XNameContainer.class, oObj);

        try {
            aContainer.insertByName("First", aControl);
            aContainer.insertByName("Second", aControl2);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("!!! Could't insert column Instance");
            e.printStackTrace(log);
            throw new StatusException("Can't insert columns", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("!!! Could't insert column Instance");
            e.printStackTrace(log);
            throw new StatusException("Can't insert columns", e);
        } catch (com.sun.star.container.ElementExistException e) {
            log.println("!!! Could't insert column Instance");
            e.printStackTrace(log);
            throw new StatusException("Can't insert columns", e);
        }

        //Relations for XSelectionSupplier
        tEnv.addObjRelation("Selections", new Object[] { aControl, aControl2 });

        // adding relation for XNameContainer
        tEnv.addObjRelation("XNameContainer.AllowDuplicateNames", new Object());


        // adding relation for XContainer
        tEnv.addObjRelation("INSTANCE", aControl3);
        tEnv.addObjRelation("INSTANCE1", aControl4);
        tEnv.addObjRelation("INSTANCE2", aControl5);


        tEnv.addObjRelation("Comparer",
                            new Comparator<Object>() {
            public int compare(Object o1, Object o2) {
                XNamed named1 = UnoRuntime.queryInterface(
                                        XNamed.class, o1);
                XNamed named2 = UnoRuntime.queryInterface(
                                        XNamed.class, o2);

                if (named1.getName().equals(named2.getName())) {
                    return 0;
                }

                return -1;
            }
        });


//        HashSet exclude = new HashSet();
//
//        exclude.add("FormatKey");
//
//        tEnv.addObjRelation("XUpdateBroadcaster.Checker",
//                            new Checker(m_XFormLoader, m_XPS, m_XCtrl, m_ChangePropertyName, m_ChangePropertyValue));
        return tEnv;
    }

}    // finish class OButtonModelold


class OGridControlModelold extends TestCase {
    XComponent xDrawDoc = null;

    /**
    * Creates Drawing document.
    */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(((XMultiServiceFactory) tParam.getMSF()));

        log.println("creating a draw document");

        try {
            xDrawDoc = SOF.createDrawDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            throw new StatusException("Can't create Draw document", e);
        }
    }

    /**
    * Disposes drawing document.
    */
    protected void cleanup(TestParameters tParam, PrintWriter log) {
        log.println("    disposing xDrawDoc ");

        try {
            XCloseable closer = UnoRuntime.queryInterface(
                                        XCloseable.class, xDrawDoc);
            closer.close(true);
        } catch (com.sun.star.util.CloseVetoException e) {
            log.println("couldn't close document");
        } catch (com.sun.star.lang.DisposedException e) {
            log.println("couldn't close document");
        }
    }

    /**
    * <code>GridControl</code> component created and added to the draw
    * page. Then two columns are created and added to the grid.
    *     Object relations created :
    * <ul>
    *  <li> <code>'INSTANCE1' ... 'INSTANCEN'</code> for
    *    <code>XNameReplace, XNameContainer, XIndexReplace,
    *     XIndexContainer </code> : objects to be inserted
    *     or replaced with in interface tests. Number of relations
    *     depends on number of interface test threads. For each
    *     thread there must be an individual element. </li>
    *  <li> <code>'XNameContainer.AllowDuplicateNames'</code> :
    *   if this relation exists then container elements can have duplicate
    *   names. <code>GridControl</code> can have.</li>
    *  <li> <code>'OBJNAME'</code> for
    *      {@link ifc.io._XPersistObject} : name of service which is
    *    represented by this object. </li>
    *  <li> <code>'INSTANCE'</code> for
    *      {@link ifc.container._XContainer} : a column instance. </li>
    * </ul>
    */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;
        XInterface oInstance = null;
        XPropertySet aControl = null;
        XPropertySet aControl2 = null;
        XPropertySet aControl3 = null;
        XPropertySet aControl4 = null;
        XGridColumnFactory columns = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        XControlShape shape = FormTools.insertControlShape(xDrawDoc, 5000,
                                                           7000, 2000, 2000,
                                                           "GridControl");
        oObj = shape.getControl();

        log.println("creating a new environment for drawpage object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        try {
            columns = UnoRuntime.queryInterface(
                              XGridColumnFactory.class, oObj);
            aControl = columns.createColumn("TextField");
            aControl2 = columns.createColumn("DateField");
            aControl3 = columns.createColumn("TextField");
            aControl4 = columns.createColumn("TextField");
            columns.createColumn("TextField");
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            // Some exception occures.FAILED
            log.println("!!! Couldn't create instance : " + e);
            throw new StatusException("Can't create column instances.", e);
        }

        XNameContainer aContainer = UnoRuntime.queryInterface(
                                            XNameContainer.class, oObj);

        try {
            aContainer.insertByName("First", aControl);
            aContainer.insertByName("Second", aControl2);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("!!! Could't insert column Instance");
            e.printStackTrace(log);
            throw new StatusException("Can't insert columns", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("!!! Could't insert column Instance");
            e.printStackTrace(log);
            throw new StatusException("Can't insert columns", e);
        } catch (com.sun.star.container.ElementExistException e) {
            log.println("!!! Could't insert column Instance");
            e.printStackTrace(log);
            throw new StatusException("Can't insert columns", e);
        }

        //Relations for XSelectionSupplier
        tEnv.addObjRelation("Selections", new Object[] { aControl, aControl2 });

        // COMPARER

        int THRCNT = 1;
        String count = (String)Param.get("THRCNT");
        if (count != null)
            THRCNT = Integer.parseInt(count);


        // INSTANCEn : _XNameContainer; _XNameReplace
        log.println("adding INSTANCEn as obj relation to environment");

        try {
            for (int n = 1; n < (3 * THRCNT + 1); n++) {
                log.println("adding INSTANCE" + n +
                            " as obj relation to environment");
                oInstance = columns.createColumn("TextField");
                tEnv.addObjRelation("INSTANCE" + n, oInstance);
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't create 'INSTANCEn' relations", e);
        }


        // adding relation for XNameContainer
        tEnv.addObjRelation("XNameContainer.AllowDuplicateNames", new Object());

       // OK tEnv.addObjRelation("OBJNAME", "stardiv.one.form.component." + objName);


        // adding relation for XContainer
        tEnv.addObjRelation("INSTANCE", aControl3);
        tEnv.addObjRelation("INSTANCE2", aControl4);


        //adding ObjRelation for XPersistObject
        // OK tEnv.addObjRelation("PSEUDOPERSISTENT", new Boolean(true)); // OK

        return tEnv;
    } // finish method getTestEnvironment
} // finish class OGridControlModel
