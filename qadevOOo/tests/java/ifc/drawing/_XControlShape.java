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
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.drawing.XControlShape
*/
public class _XControlShape extends MultiMethodTest {

    public XControlShape oObj = null;

    /**
    * Test calls the method. <p>
    * Has <b> OK </b> status if the method successfully returns
    * and no exceptions were thrown. <p>
    */
    public void _getControl() {
        oObj.getControl();
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

        XControlModel xControl = UnoRuntime.queryInterface
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


