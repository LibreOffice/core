/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

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
 * Test is <b> NOT </b> multithread compilant. <p>
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
                XDrawPagesSupplier oDPS = (XDrawPagesSupplier)
                UnoRuntime.queryInterface(XDrawPagesSupplier.class, oDoc);
                oDP = (XDrawPage) UnoRuntime.queryInterface(XDrawPage.class, oDPS.getDrawPages().getByIndex(0));
            } else {


            XDrawPageSupplier oDPS = (XDrawPageSupplier)
                UnoRuntime.queryInterface(XDrawPageSupplier.class, oDoc);
            oDP = oDPS.getDrawPage();
            }
            XShapes shapes = (XShapes) UnoRuntime.queryInterface
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

