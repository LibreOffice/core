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



package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.beans.PropertyValue;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.Locale;
import com.sun.star.text.XDefaultNumberingProvider;

/**
* Testing <code>com.sun.star.text.XDefaultNumberingProvider</code>
* interface methods :
* <ul>
*  <li><code> getDefaultOutlineNumberings()</code></li>
*  <li><code> getDefaultContinuousNumberingLevels()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.text.XDefaultNumberingProvider
*/
public class _XDefaultNumberingProvider extends MultiMethodTest {

    public static XDefaultNumberingProvider oObj = null;
    private Locale loc = new Locale("en", "EN", "") ;

    /**
    * Just gets numberings for "en" locale. <p>
    * Has <b>OK</b> status if not <code>null</code> value returned
    * and no runtime exceptions occurred.
    */
    public void _getDefaultOutlineNumberings() {
        XIndexAccess xIA[] = oObj.getDefaultOutlineNumberings(loc) ;

        tRes.tested("getDefaultOutlineNumberings()", xIA != null) ;
    }

    /**
    * Just gets numberings for "en" locale. <p>
    * Has <b>OK</b> status if not <code>null</code> value returned
    * and no runtime exceptions occurred.
    */
    public void _getDefaultContinuousNumberingLevels() {
        PropertyValue[][] lev = oObj.getDefaultContinuousNumberingLevels(loc) ;

        tRes.tested("getDefaultContinuousNumberingLevels()", lev != null) ;
    }
}

