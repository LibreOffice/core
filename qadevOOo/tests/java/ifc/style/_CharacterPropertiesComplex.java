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

/**
* Testing <code>com.sun.star.style.CharacterPropertiesComplex</code>
* service properties :
* <ul>
*  <li><code> CharHeightComplex</code></li>
*  <li><code> CharWeightComplex</code></li>
*  <li><code> CharFontNameComplex</code></li>
*  <li><code> CharFontStyleNameComplex</code></li>
*  <li><code> CharFontFamilyComplex</code></li>
*  <li><code> CharFontCharSetComplex</code></li>
*  <li><code> CharFontPitchComplex</code></li>
*  <li><code> CharPostureComplex</code></li>
*  <li><code> CharLocaleComplex</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.style.CharacterPropertiesComplex
*/
public class _CharacterPropertiesComplex extends MultiPropertyTest {

    public void _CharWeightComplex() {
        testProperty("CharWeightComplex", new Float(com.sun.star.awt.FontWeight.BOLD),
            new Float(com.sun.star.awt.FontWeight.THIN)) ;
    }

    public void _CharPostureComplex() {
        testProperty("CharPostureComplex", com.sun.star.awt.FontSlant.ITALIC,
            com.sun.star.awt.FontSlant.NONE) ;
    }

    /**
    * Forces environment recreation.
    */
    protected void after() {
        disposeEnvironment();
    }

} //finish class _CharacterPropertiesComplex

