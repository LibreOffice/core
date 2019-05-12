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
import com.sun.star.linguistic2.XAvailableLocales;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.linguistic2.XAvailableLocales</code>
* interface methods:
* <ul>
*   <li><code>getAvailableLocales()</code></li>
* </ul> <p>
* @see com.sun.star.linguistic2.XAvailableLocales
*/
public class _XAvailableLocales extends MultiMethodTest {

    public XAvailableLocales oObj = null;
    public static Locale[] locales = new Locale[0];

    /**
    * Test calls the method, stores returned value and checks it. <p>
    * Has <b> OK </b> status if length of returned array isn't zero. <p>
    */
    public void _getAvailableLocales() {
        locales = oObj.getAvailableLocales("com.sun.star.linguistic2.Hyphenator");
        tRes.tested("getAvailableLocales()", locales.length > 0);
    }

}  // finish class XAvailableLocales


