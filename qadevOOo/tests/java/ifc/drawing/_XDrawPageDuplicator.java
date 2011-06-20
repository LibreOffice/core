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
* Test is <b> NOT </b> multithread compilant. <p>
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
        XDrawPagesSupplier PS = (XDrawPagesSupplier)
            UnoRuntime.queryInterface(XDrawPagesSupplier.class, testobj);
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

