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

import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.XDictionaryEntry;
import com.sun.star.linguistic2.XSearchableDictionaryList;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.linguistic2.XSearchableDictionaryList</code>
* interface methods:
* <ul>
*   <li><code>queryDictionaryEntry()</code></li>
* </ul> <p>
* @see com.sun.star.linguistic2.XSearchableDictionaryList
*/
public class _XSearchableDictionaryList extends MultiMethodTest {

    public XSearchableDictionaryList oObj = null;

    /**
    * Test calls the method for negative dictionary and for positive dictionary
    * and checks returned values. <p>
    * Has <b> OK </b> status if returned values are not null. <p>
    */
    public void _queryDictionaryEntry() {
        XDictionaryEntry aNegativEntry = oObj.queryDictionaryEntry("Negativ",
                                    new Locale("en","US","WIN"),false,true);
        if (aNegativEntry == null) {
            log.println("'queryDictionary' didn't work for negative dictionaries");
        } else {
            log.println("'queryDictionary' works for negative dictionaries");
        }
        XDictionaryEntry aPositivEntry = oObj.queryDictionaryEntry("Positiv",
                                    new Locale("en","US","WIN"),true,true);
        if (aPositivEntry == null) {
            log.println("'queryDictionary' didn't work for positive dictionaries");
        } else {
            log.println("'queryDictionary' works for positive dictionaries");
        }
        tRes.tested("queryDictionaryEntry()",(aNegativEntry != null) &&
                                                    (aPositivEntry != null) );
    }

}  // finish class _XSearchableDictionaryList


