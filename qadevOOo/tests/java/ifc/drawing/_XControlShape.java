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

package ifc.drawing;

import lib.MultiMethodTest;
import util.FormTools;

import com.sun.star.awt.XControlModel;
import com.sun.star.drawing.XControlShape;
import com.sun.star.lang.XComponent;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.drawing.XControlShape</code>
* interface methods :
* <ul>
*  <li><code> getControl()</code></li>
*  <li><code> setControl()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'xDoc'</code> (of type <code>XComponent</code>):
*   the document where shape tested is situated. This document
*   must also implement <code>XMultiServiceFactory</code> interface
*   to create some control model. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.drawing.XControlShape
*/
public class _XControlShape extends MultiMethodTest {

    public XControlShape oObj = null;

    XControlModel model = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _getControl() {
        model = oObj.getControl() ;

        tRes.tested("getControl()", true) ;
    }

    /**
    * With the help of document passed as relation, a new button control
    * model is created and set as a control. <p>
    * Has <b> OK </b> status if <code>getControl()</code> method returns
    * the same control as was set. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getControl() </code>  </li>
    * </ul>
    */
    public void _setControl() {
        requiredMethod("getControl()") ;

        XInterface oNewControl = FormTools.createControl
            ((XComponent)tEnv.getObjRelation("xDoc"), "ComboBox") ;

        XControlModel xControl = (XControlModel) UnoRuntime.queryInterface
            (XControlModel.class, oNewControl) ;

        oObj.setControl(xControl) ;

        XControlModel gControl = oObj.getControl() ;

        if (oNewControl.equals(gControl))
            tRes.tested("setControl()", true) ;
        else {
            log.println("Control set is not equal to control get") ;
            tRes.tested("setControl()", false) ;
        }
    }

}  // finish class _XControlShape


