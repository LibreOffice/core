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

package ifc.linguistic2;

import lib.MultiMethodTest;

import com.sun.star.beans.PropertyValue;
import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.XHyphenatedWord;
import com.sun.star.linguistic2.XHyphenator;
import com.sun.star.linguistic2.XPossibleHyphens;

/**
* Testing <code>com.sun.star.linguistic2.XHyphenator</code>
* interface methods:
* <ul>
*   <li><code>hyphenate()</code></li>
*   <li><code>queryAlternativeSpelling()</code></li>
*   <li><code>createPossibleHyphens()</code></li>
* </ul><p>
* @see com.sun.star.linguistic2.XHyphenator
*/
public class _XHyphenator extends MultiMethodTest {

    public XHyphenator oObj = null;

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _hyphenate() {
        boolean res = true;
        PropertyValue[] Props = null;
        try {
            XHyphenatedWord result = oObj.hyphenate(
                    "wacker",new Locale("de","DE",""),(short)3,Props);
            res &= (result != null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'hyphenate'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("hyphenate()",res);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _queryAlternativeSpelling() {
        boolean res = true;
        PropertyValue[] Props = null;
        try {
                XHyphenatedWord result = oObj.queryAlternativeSpelling(
                    "wacker",new Locale("de","DE",""),(short)2,Props);
                res &= (result != null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'queryAlternativeSpelling'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("queryAlternativeSpelling()",res);
    }

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value isn't null. <p>
    */
    public void _createPossibleHyphens() {
        boolean res = true;
        PropertyValue[] Props = null;
        try {
            XPossibleHyphens result = oObj.createPossibleHyphens(
                    "wacker",new Locale("de","DE",""),Props);
            res &= (result != null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'createPossibleHyphens'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("createPossibleHyphens()",res);
    }

}  // \u0422\u044B finish class XHyphenator


