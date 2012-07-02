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

package ifc.i18n;

import lib.MultiMethodTest;

import com.sun.star.i18n.XIndexEntrySupplier;
import com.sun.star.lang.Locale;

/**
* Testing <code>com.sun.star.i18n.XIndexEntrySupplier</code>
* interface methods:
* <ul>
*  <li><code> getIndexCharacter() </code></li>
*  <li><code> getIndexFollowPageWord() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.i18n.XIndexEntrySupplier
*/
public class _XIndexEntrySupplier extends MultiMethodTest {
    public XIndexEntrySupplier oObj = null;
    public String[] languages = new String[]{"de","en","es","fr","ja","ko","zh"};
    public String[] countries = new String[]{"DE","US","ES","FR","JP","KR","CN"};
    public String[] onePage = new String[]{"f.","p."," s."," sv","p.","",""};
    public String[] morePages = new String[]{"ff.","pp."," ss."," sv","pp.","",""};

    /**
    * Test calls the method, then result is checked. <p>
    * Has <b> OK </b> status if the method returns right index for several
    * locales and word.
    */
    public void _getIndexCharacter() {
        boolean res = true;
        log.println("getIndexCharacter('chapter', getLocale(i), '')");
        for (int i=0; i<7; i++) {
            log.print("getIndexCharacter('chapter', " + countries[i] + ") :");
            String get = oObj.getIndexCharacter("chapter", getLocale(i), "");
            log.println(get);
            res &= get.equals("C");
        }
        tRes.tested("getIndexCharacter()", res);
    }

    /**
    * Test calls the method with two different parameters: for one page and
    * for several pages, after every call result is checked. <p>
    * Has <b> OK </b> status if method returns right index for several locales.
    */
    public void _getIndexFollowPageWord() {
        boolean res = true;

        for (int i=0; i<7; i++) {
            String get = oObj.getIndexFollowPageWord(true, getLocale(i));
            if (! get.equals(morePages[i]) ) {
                log.println("Language: " + languages[i]);
                log.println("Getting: #" + get + "#");
                log.println("Expected: #" + morePages[i] + "#");
            }
            res &= get.equals(morePages[i]);
            get = oObj.getIndexFollowPageWord(false,getLocale(i));
            if (! get.equals(onePage[i]) ) {
                log.println("Language: " + languages[i]);
                log.println("Getting: #" + get + "#");
                log.println("Expected: #" + onePage[i] + "#");
            }
            res &= get.equals(onePage[i]);
        }
        tRes.tested("getIndexFollowPageWord()", res);
    }

    /**
    * Method returns locale for a given language and country.
    * @param k index of needed locale.
    * @return Locale by the index from arrays defined above
    */
    public Locale getLocale(int k) {
        return new Locale(languages[k], countries[k], "");
    }


} // end XIndexEntrySupplier

