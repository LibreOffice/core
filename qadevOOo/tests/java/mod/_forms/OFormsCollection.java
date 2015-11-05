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

import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.DrawTools;
import util.FormTools;

import com.sun.star.container.XNameContainer;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;


/**
* Test for object which is represented by service
* <code>com.sun.star.form.Forms</code>. <p>
* Object implements the following interfaces :
* <ul>
*  <li> <code>com::sun::star::container::XNameReplace</code></li>
*  <li> <code>com::sun::star::container::XIndexReplace</code></li>
*  <li> <code>com::sun::star::container::XNameContainer</code></li>
*  <li> <code>com::sun::star::container::XIndexAccess</code></li>
*  <li> <code>com::sun::star::container::XElementAccess</code></li>
*  <li> <code>com::sun::star::container::XEnumerationAccess</code></li>
*  <li> <code>com::sun::star::container::XIndexContainer</code></li>
*  <li> <code>com::sun::star::container::XNameAccess</code></li>
* </ul>
*
* This object test <b> is NOT </b> designed to be run in several
* threads concurrently.
*
* @see com.sun.star.container.XNameReplace
* @see com.sun.star.container.XIndexReplace
* @see com.sun.star.container.XNameContainer
* @see com.sun.star.container.XIndexAccess
* @see com.sun.star.container.XElementAccess
* @see com.sun.star.container.XEnumerationAccess
* @see com.sun.star.container.XIndexContainer
* @see com.sun.star.container.XNameAccess
* @see ifc.container._XNameReplace
* @see ifc.container._XIndexReplace
* @see ifc.container._XNameContainer
* @see ifc.container._XIndexAccess
* @see ifc.container._XElementAccess
* @see ifc.container._XEnumerationAccess
* @see ifc.container._XIndexContainer
* @see ifc.container._XNameAccess
*/
public class OFormsCollection extends TestCase {
    XComponent xDrawDoc;

    /**
    * Creates Drawing document.
    */
    @Override
    protected void initialize(TestParameters tParam, PrintWriter log) throws Exception {
        log.println("creating a draw document");
        xDrawDoc = DrawTools.createDrawDoc(tParam.getMSF());
    }

    /**
    * Disposes drawing document.
    */
    @Override
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
    * Adds some controls to the 'Standard' form of a draw page,
    * then adds an empty form to a collection of forms. Then
    * returns the collection as a test object. <p>
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
    *   names. In case of forms' collection forms can have equal names.</li>
    * </ul>
    */
    @Override
    protected TestEnvironment createTestEnvironment(TestParameters Param,
                                                                 PrintWriter log) {
        XInterface oObj = null;
        XDrawPage oDP = null;


        // creation of testobject here
        // first we write what we are intend to do to log file
        log.println("creating a test environment");

        oDP = DrawTools.getDrawPage(xDrawDoc, 0);

        (DrawTools.getShapes(oDP))
            .add(FormTools.createControlShape(xDrawDoc, 2000, 1500, 1000, 1000,
                                              "CheckBox"));
        (DrawTools.getShapes(oDP))
            .add(FormTools.createControlShape(xDrawDoc, 3000, 4500, 15000,
                                              1000, "CommandButton"));
        (DrawTools.getShapes(oDP))
            .add(FormTools.createControlShape(xDrawDoc, 5000, 3500, 7500, 5000,
                                              "TextField"));

        oObj = FormTools.getForms(oDP);
        FormTools.insertForm(xDrawDoc, (XNameContainer) oObj, "SecondForm");

        log.println("creating a new environment for drawpage object");

        TestEnvironment tEnv = new TestEnvironment(oObj);


        // INSTANCEn : _XNameContainer; _XNameReplace
        log.println("adding INSTANCEn as obj relation to environment");

        XComponent xComp = UnoRuntime.queryInterface(
                                   XComponent.class, xDrawDoc);
        int THRCNT = 1;
        if (Param.get("THRCNT")!= null) {
            THRCNT = Integer.parseInt((String) Param.get("THRCNT"));
        }

        for (int n = 1; n < (THRCNT + 1); n++) {
            log.println("adding INSTANCE" + n +
                        " as obj relation to environment");
            tEnv.addObjRelation("INSTANCE" + n,
                                FormTools.createControl(xComp, "Form"));
        }


        // adding indicator that this collection can have duplicate
        // elements with the same names for XNameContainer test.
        tEnv.addObjRelation("XNameContainer.AllowDuplicateNames", new Object());

        tEnv.addObjRelation("INSTANCE", FormTools.createControl(xComp, "Form"));
        tEnv.addObjRelation("INSTANCE2",
                            FormTools.createControl(xComp, "Form"));

        return tEnv;
    } // finish method getTestEnvironment
} // finish class OFormsCollection
