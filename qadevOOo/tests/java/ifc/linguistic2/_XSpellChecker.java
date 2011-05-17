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
import com.sun.star.linguistic2.XSpellAlternatives;
import com.sun.star.linguistic2.XSpellChecker;
import lib.Status;
import lib.StatusException;

/**
* Testing <code>com.sun.star.linguistic2.XSpellChecker</code>
* interface methods:
* <ul>
*   <li><code>isValid()</code></li>
*   <li><code>spell()</code></li>
* </ul><p>
* @see com.sun.star.linguistic2.XSpellChecker
*/
public class _XSpellChecker extends MultiMethodTest {

    public XSpellChecker oObj = null;
    XSpellChecker alternative = null;

    public void before() {
        alternative = (XSpellChecker) tEnv.getObjRelation("AlternativeChecker");
        if  (alternative == null) throw new StatusException(Status.failed
            ("Relation AlternativeChecker not found")) ;
    }

    /**
    * Test calls the method for a correctly spelled word and
    * for a uncorrectly spelled word and checks returned values. <p>
    * Has <b> OK </b> status if returned value is equal to true in first case,
    * if returned value is equal to false in second case and no exceptions
    * were thrown. <p>
    */
    public void _isValid() {
        boolean res = true;
        try {
            log.println("Checking 'original' Spellchecker");
            PropertyValue[] empty = new PropertyValue[0] ;
            res &= oObj.isValid("Sun", new Locale("en","US",""), empty);
            res &= !oObj.isValid("Summersun", new Locale("en","US","") ,empty);
            log.println("Result so far is - "+ (res ? "OK" : "failed"));
            log.println("Checking alternative Spellchecker");
            res &= alternative.isValid("Sun", new Locale("en","US",""), empty);
            res &= !alternative.isValid("Summersun", new Locale("en","US","") ,empty);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'isValid'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("isValid()",res);
    }

    /**
    * Test calls the method for a uncorrectly spelled word
    * and checks returned values. <p>
    * Has <b> OK </b> status if at least one spell alternative exists
    * and no exceptions were thrown. <p>
    */
    public void _spell() {
        boolean res = true;
        try {
            log.println("Checking 'original' Spellchecker");
            PropertyValue[] empty = new PropertyValue[0] ;
            XSpellAlternatives alt = oObj.spell(
                            "Summersun",new Locale("en","US",""),empty);
            String alternatives = alt.getAlternatives()[0];
            res = (alternatives != null);
            log.println("Result so far is - "+ (res ? "OK" : "failed"));
            log.println("Checking alternative Spellchecker");
            alt =alternative.spell(
                            "Summersun",new Locale("en","US",""),empty);
            alternatives = alt.getAlternatives()[0];
            res &= (alternatives != null);
        } catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'spell'");
            res = false;
            ex.printStackTrace(log);
        }
        tRes.tested("spell()",res);
    }

}  // finish class MTest


