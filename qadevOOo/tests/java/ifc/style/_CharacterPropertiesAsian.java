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
