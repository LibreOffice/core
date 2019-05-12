/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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


