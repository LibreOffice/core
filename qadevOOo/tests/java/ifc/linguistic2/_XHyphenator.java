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


