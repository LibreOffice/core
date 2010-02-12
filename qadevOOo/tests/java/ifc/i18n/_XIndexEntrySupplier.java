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
    * @param localeIndex index of needed locale.
    * @return Locale by the index from arrays defined above
    */
    public Locale getLocale(int k) {
        return new Locale(languages[k], countries[k], "");
    }


} // end XIndexEntrySupplier

