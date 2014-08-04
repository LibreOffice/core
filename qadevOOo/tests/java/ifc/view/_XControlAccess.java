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

package ifc.view;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.FormTools;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlModel;
import com.sun.star.drawing.XControlShape;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageSupplier;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.view.XControlAccess;

/**
 * Testing <code>com.sun.star.view.XControlAccess</code>
 * interface methods :
 * <ul>
 *  <li><code> getControl()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'DOCUMENT'</code> (of type <code>XComponent</code>):
 *   the document, which controller is tested here. Is used
 *   for adding a component (such as CommandButton) and obtaining
 *   its control via tested interface. </li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.view.XControlAccess
 */
public class _XControlAccess extends MultiMethodTest {

    public XControlAccess oObj = null;

    /**
     * Retrieves a document from relation, then using it adds
     * a CommandButton to a document and obtains button model.
     * After that button control is tried to get using the
     * tested interface. <p>
     * Has <b> OK </b> status if non <code>null</code> control
     * is returned. <p>
     * @throws StatusException If the relation was not found.
     */
    public void _getControl(){
        boolean bResult = true;
        try {
            XComponent oDoc = (XComponent)tEnv.getObjRelation("DOCUMENT");
            if (oDoc == null) {
                throw new StatusException
                    (Status.failed("NO 'DOCUMENT' ObjRelation!"));
            }
            Boolean isSheet = (Boolean) tEnv.getObjRelation("XControlAccess.isSheet");
            XDrawPage oDP = null;
            if (isSheet != null) {
                XDrawPagesSupplier oDPS = UnoRuntime.queryInterface(XDrawPagesSupplier.class, oDoc);
                oDP = UnoRuntime.queryInterface(XDrawPage.class, oDPS.getDrawPages().getByIndex(0));
            } else {


            XDrawPageSupplier oDPS = UnoRuntime.queryInterface(XDrawPageSupplier.class, oDoc);
            oDP = oDPS.getDrawPage();
            }
            XShapes shapes = UnoRuntime.queryInterface
                (XShapes.class, oDP);
            XShape button = FormTools.createControlShape
                (oDoc, 100, 100, 10000, 50000, "CommandButton");
            shapes.add(button);

            XControlModel CM = ((XControlShape)button).getControl();
            log.println("Getting ControlModel "
                + ((CM == null) ? "FAILED" : "OK"));

            XControl oControl = oObj.getControl(CM);
            log.println("Getting Control " + ((CM == null) ? "FAILED" : "OK"));

            bResult &= oControl != null;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("Exception occurred calling the method: " + e);
            bResult = false;
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception occurred calling the method: " + e);
            bResult = false;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occurred calling the method: " + e);
            bResult = false;
        }
        tRes.tested("getControl()", bResult);
    }
}

