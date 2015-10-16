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

package ifc.text;

import lib.MultiMethodTest;
import util.XInstCreator;

import com.sun.star.text.XFootnote;


/**
 * Testing <code>com.sun.star.text.XFootnote</code>
 * interface methods :
 * <ul>
 *  <li><code> getLabel()</code></li>
 *  <li><code> setLabel()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XFootnote
 */
public class _XFootnote extends MultiMethodTest {

    public XFootnote oObj = null;        // oObj filled by MultiMethodTest
    XInstCreator info = null;               // instance creator

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getLabel(){

        boolean result;

        // testing getLabel
        log.println("Testing getLabel() ... ");
        String oldLabel = oObj.getLabel();
        log.println("getLabel: Old Value: " + oldLabel);
        result = (oldLabel != null);
        if (result) {
            log.println(" ... getLabel() - OK");
        }
        else {
            log.println(" ... getLabel() - FAILED");
        }
        tRes.tested("getLabel()", result);

    } // finished getLabel


    /**
     * Sets a new label, then using <code>getLabel</code> method
     * checks if the label was set. <p>
     *
     * Has <b>OK</b> status if set and get values are equal.
     */
    public void _setLabel(){

        boolean result;
        String str = "New XFootnote Label";

        // testing getLabel
        log.println("Testing setLabel() ... ");
        log.println("New label : " + str);

        String oldLabel = oObj.getLabel();
        log.println("Old label was: " + oldLabel);
        oObj.setLabel(str);

        String res = oObj.getLabel();

        log.println("verify setLabel result");
        result = (res.equals(str));
        if (result) {
            log.println(" ... setLabel() - OK");
        }
        else {
            log.println(" ... setLabel() - FAILED");
        }
        tRes.tested("setLabel()", result);

        log.println("restoring the old label value");
        oObj.setLabel(oldLabel);
    } // finished setLabel

}


