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

    @Override
    public void before() {
        alternative = (XSpellChecker) tEnv.getObjRelation("AlternativeChecker");
        if  (alternative == null) throw new StatusException(Status.failed
            ("Relation AlternativeChecker not found")) ;
    }

    /**
    * Test calls the method for a correctly spelled word and
    * for a incorrectly spelled word and checks returned values. <p>
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
    * Test calls the method for a incorrectly spelled word
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


