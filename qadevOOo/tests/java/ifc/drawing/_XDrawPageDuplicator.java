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

import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XDrawPageDuplicator;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.uno.AnyConverter;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;


/**
* Testing <code>com.sun.star.drawing.XDrawPageDuplicator</code>
* interface methods :
* <ul>
*  <li><code> duplicate()</code></li>
* </ul> <p>
* The object tested <b> must implement </b>
* <code>XDrawPagesSupplier</code> interface to have access to draw
* pages collection. <p>
* Test is <b> NOT </b> multithread compliant. <p>
* After test completion object environment has to be recreated.
* @see com.sun.star.drawing.XDrawPageDuplicator
*/
public class _XDrawPageDuplicator extends MultiMethodTest {
    public XDrawPageDuplicator oObj = null;

    /**
    * First queries object tested for <code>XDrawPagesSupplier</code>
    * interface and obtains one draw page from document. Then it
    * tries to duplicate it.<p>
    * Has <b> OK </b> status if the method returns not null value and
    * this value is not equal to the page which was duplicated. <p>
    */
    public void _duplicate(){
        boolean result = false;
        XInterface testobj = tEnv.getTestObject();
        XDrawPagesSupplier PS = UnoRuntime.queryInterface(XDrawPagesSupplier.class, testobj);
        XDrawPages DPs = PS.getDrawPages();
        XDrawPage DP = null;
        try {
            DP = (XDrawPage) AnyConverter.toObject(
                    new Type(XDrawPage.class),DPs.getByIndex(0));
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("Exception occurred while testing: " + e);
        } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
            log.println("Exception occurred while testing: " + e);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception occurred while testing: " + e);
        }

        if (DP != null) {
            XDrawPage newPage = oObj.duplicate(DP);
            result = (newPage != null) && !(newPage.equals(DP));
        }
        tRes.tested("duplicate()", result);
    }

}

