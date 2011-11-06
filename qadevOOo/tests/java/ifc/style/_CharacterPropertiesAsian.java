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



package ifc.style;

import lib.MultiPropertyTest;

import com.sun.star.awt.FontSlant;
import com.sun.star.awt.FontWeight;

/**
* Testing <code>com.sun.star.style.CharacterPropertiesAsian</code>
* service properties :
* <ul>
*  <li><code> CharHeightAsian</code></li>
*  <li><code> CharWeightAsian</code></li>
*  <li><code> CharFontNameAsian</code></li>
*  <li><code> CharFontStyleNameAsian</code></li>
*  <li><code> CharFontFamilyAsian</code></li>
*  <li><code> CharFontCharSetAsian</code></li>
*  <li><code> CharFontPitchAsian</code></li>
*  <li><code> CharPostureAsian</code></li>
*  <li><code> CharLocaleAsian</code></li>
*  <li><code> ParaIsCharacterDistance</code></li>
*  <li><code> ParaIsForbiddenRules</code></li>
*  <li><code> ParaIsHangingPunctuation</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.style.CharacterPropertiesAsian
*/
public class _CharacterPropertiesAsian extends MultiPropertyTest {

    public void _CharPostureAsian() {
        testProperty("CharPostureAsian", FontSlant.NONE, FontSlant.ITALIC);
    }

    public void _CharWeightAsian() {
        testProperty("CharWeightAsian", new Float(FontWeight.NORMAL),
            new Float(FontWeight.BOLD));
    }

    public void _ParaIsCharacterDistance() {
        testProperty("ParaIsCharacterDistance", Boolean.TRUE, Boolean.FALSE);
    }

    public void _ParaIsForbiddenRules() {
        testProperty("ParaIsForbiddenRules", Boolean.TRUE, Boolean.FALSE);
    }

    public void _ParaIsHangingPunctuation() {
        testProperty("ParaIsHangingPunctuation", Boolean.TRUE, Boolean.FALSE);
    }
} //finish class _CharacterPropertiesAsian
