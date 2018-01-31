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
        XDictionaryEntry aNegativEntry = oObj.queryDictionaryEntry("Negative",
                                    new Locale("en","US","WIN"),false,true);
        if (aNegativEntry == null) {
            log.println("'queryDictionary' didn't work for negative dictionaries");
        } else {
            log.println("'queryDictionary' works for negative dictionaries");
        }
        XDictionaryEntry aPositivEntry = oObj.queryDictionaryEntry("Positive",
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


