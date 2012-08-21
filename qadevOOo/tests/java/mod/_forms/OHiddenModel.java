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

import com.sun.star.beans.PropertyValue;
import java.io.PrintWriter;

import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.FormTools;
import util.SOfficeFactory;

import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
 * Test for object which is represented by service
 * <code>com.sun.star.form.component.HiddenControl</code>. <p>
 * Object implements the following interfaces :
 * <ul>
 *  <li> <code>com::sun::star::io::XPersistObject</code></li>
 *  <li> <code>com::sun::star::container::XChild</code></li>
 *  <li> <code>com::sun::star::form::FormControlModel</code></li>
 *  <li> <code>com::sun::star::form::component::HiddenControl</code></li>
 *  <li> <code>com::sun::star::form::FormComponent</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyAccess</code></li>
*  <li> <code>com::sun::star::beans::XPropertyContainer</code></li>
*  <li> <code>com::sun::star::beans::XPropertySet</code></li>
 *  <li> <code>com::sun::star::beans::XFastPropertySet</code></li>
 *  <li> <code>com::sun::star::beans::XPropertyState</code></li>
 *  <li> <code>com::sun::star::container::XNamed</code></li>
 *  <li> <code>com::sun::star::beans::XMultiPropertySet</code></li>
 *  <li> <code>com::sun::star::lang::XComponent</code></li>
 * </ul> <p>
 * This object test <b> is NOT </b> designed to be run in several
 * threads concurently.
 * @see com.sun.star.io.XPersistObject
 * @see com.sun.star.container.XChild
 * @see com.sun.star.form
 * @see com.sun.star.form.component.HiddenControl
 * @see com.sun.star.form.FormComponent
 * @see com.sun.star.beans.XPropertyAccess
* @see com.sun.star.beans.XPropertyContainer
* @see com.sun.star.beans.XPropertySet
 * @see com.sun.star.beans.XFastPropertySet
 * @see com.sun.star.beans.XPropertyState
 * @see com.sun.star.container.XNamed
 * @see com.sun.star.beans.XMultiPropertySet
 * @see com.sun.star.lang.XComponent
 * @see ifc.io._XPersistObject
 * @see ifc.container._XChild
 * @see ifc.form._FormControlModel
 * @see ifc.form.component._HiddenControl
 * @see ifc.form._FormComponent
 * @see ifc.beans._XPropertySet
 * @see ifc.beans._XFastPropertySet
 * @see ifc.beans._XPropertyState
 * @see ifc.container._XNamed
 * @see ifc.beans._XMultiPropertySet
 * @see ifc.lang._XComponent
 */
public class OHiddenModel extends TestCase {
    XComponent xDrawDoc;

    /**
     * Creates Drawing document.
     */
    protected void initialize(TestParameters tParam, PrintWriter log) {
        SOfficeFactory SOF = SOfficeFactory.getFactory(((XMultiServiceFactory) tParam.getMSF()));

        try {
            log.println("creating a draw document");
            xDrawDoc = SOF.createDrawDoc(null);
        } catch (com.sun.star.uno.Exception e) {
            // Some exception occures.FAILED
            e.printStackTrace(log);
            throw new StatusException("Couldn't create document", e);
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
     * Creates hidden component, then adds Form into draw page,
     * and inserts the component into Form components' collection.
     *     Object relations created :
     * <ul>
     *  <li> <code>'OBJNAME'</code> for
     *      {@link ifc.io._XPersistObject} : name of service which is
     *    represented by this object. </li>
     * </ul>
     */
    protected synchronized TestEnvironment createTestEnvironment(TestParameters Param,
            PrintWriter log) {
        XInterface oObj = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        // get a soffice factory object
        SOfficeFactory SOF = SOfficeFactory.getFactory(((XMultiServiceFactory) Param.getMSF()));
        String objName = "HiddenControl";
        XInterface ctrl = SOF.createControl(xDrawDoc, objName);

        try {
            XDrawPage oDP = DrawTools.getDrawPage(xDrawDoc, 0);

            XNameContainer nc = FormTools.getForms(oDP);
            FormTools.insertForm(xDrawDoc, nc, "OHiddenModelForm");

            Object frm = nc.getByName("OHiddenModelForm");

            XNameContainer frmNC = UnoRuntime.queryInterface(
                    XNameContainer.class, frm);

            frmNC.insertByName("OHiddenModel", ctrl);
            oObj = (XInterface) AnyConverter.toObject(
                    new Type(XInterface.class),
                    frmNC.getByName("OHiddenModel"));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't create and add control", e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't create and add control", e);
        } catch (com.sun.star.container.NoSuchElementException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't create and add control", e);
        } catch (com.sun.star.container.ElementExistException e) {
            e.printStackTrace(log);
            throw new StatusException("Can't create and add control", e);
        }

        log.println("creating a new environment for drawpage object");

        TestEnvironment tEnv = new TestEnvironment(oObj);

        util.dbg.getSuppServices(oObj);

        log.println("adding DrawDocument as obj relation to environment");
        tEnv.addObjRelation("OBJNAME", "stardiv.one.form.component.Hidden");

        PropertyValue prop = new PropertyValue();
        prop.Name = "Name";
        prop.Value = "new Text since XPropertyAccess";
        tEnv.addObjRelation("XPropertyAccess.propertyToChange", prop);
        tEnv.addObjRelation("XPropertyContainer.propertyNotRemovable", "Name");

        return tEnv;
    } // finish method getTestEnvironment
} // finish class OHiddenModel
