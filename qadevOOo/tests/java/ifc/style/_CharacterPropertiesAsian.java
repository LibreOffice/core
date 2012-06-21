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
