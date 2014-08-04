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
import lib.Status;
import lib.StatusException;

import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextRangeMover;

/**
 * Testing <code>com.sun.star.text.XTextRangeMover</code>
 * interface methods :
 * <ul>
 *  <li><code> moveTextRange()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'RangeForMove'</code> (of type <code>XTextRange</code>):
 *   the range to be moved. </li>
 *   <li> <code>'XTextRange'</code> (of type <code>XTextRange</code>):
 *   the range that includes moving range. </li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XTextRangeMover
 */
public class _XTextRangeMover extends MultiMethodTest {

    public XTextRangeMover oObj = null;

    XTextRange xTextRange = null;
    XTextRange oMoveRange = null;

    /**
     * Moves the range obtained from relation 'RangeForMove' by 1 paragraph
     * and compares index of moved string in the whole text obtained
     * from relation 'XTextRange'. <p>
     * Has <b>OK</b> status if index of moved range is changed after method call.
     */
    public void _moveTextRange(){
        oMoveRange = (XTextRange) tEnv.getObjRelation("RangeForMove");
        xTextRange = (XTextRange) tEnv.getObjRelation("XTextRange");

        if (oMoveRange == null) {
            throw new StatusException(
                Status.failed("Couldn't get relation 'RangeForMove'"));
        }

        if (xTextRange == null) {
            throw new StatusException(
                Status.failed("Couldn't get relation 'XTextRange'"));
        }

        log.println("Content before moving:");
        log.println(xTextRange.getString());
        log.println("Text range for moving:");
        log.println(oMoveRange.getString());
        int indexBefore = xTextRange.getString().indexOf(oMoveRange.getString());
        oObj.moveTextRange(oMoveRange,(short) 1);
        log.println("Content after moving:");
        log.println(xTextRange.getString());
        int indexAfter = xTextRange.getString().indexOf(oMoveRange.getString());

        boolean res = indexBefore != indexAfter;
        log.println("Index before moving:" + indexBefore);
        log.println("Index after moving:" + indexAfter);

        tRes.tested("moveTextRange()", res);
    }
}

