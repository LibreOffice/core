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

package ifc.sheet;

import lib.MultiPropertyTest;

import com.sun.star.lang.Locale;

/**
* Testing <code>com.sun.star.sheet.TableAutoFormatField</code>
* service properties :
* <ul>
*  <li><code> CharFontName</code></li>
*  <li><code> CharHeight</code></li>
*  <li><code> CharWeight</code></li>
*  <li><code> CharPosture</code></li>
*  <li><code> CharUnderline</code></li>
*  <li><code> CharCrossedOut</code></li>
*  <li><code> CharContoured</code></li>
*  <li><code> CharShadowed</code></li>
*  <li><code> CharColor</code></li>
*  <li><code> CharLocale</code></li>
*  <li><code> CellBackColor</code></li>
*  <li><code> IsCellBackgroundTransparent</code></li>
*  <li><code> ShadowFormat</code></li>
*  <li><code> ParaRightMargin </code></li>
*  <li><code> ParaLeftMargin </code></li>
*  <li><code> ParaBottomMargin    </code></li>
*  <li><code> ParaTopMargin </code></li>
*  <li><code> RotateReference </code></li>
*  <li><code> RotateAngle </code></li>
*  <li><code> Orientation </code></li>
*  <li><code> IsTextWrapped </code></li>
*  <li><code> VertJustify </code></li>
*  <li><code> HoriJustify </code></li>
*  <li><code> CharPostureComplex </code></li>
*  <li><code> CharPostureAsian    </code></li>
*  <li><code> CharWeightComplex </code></li>
*  <li><code> CharWeightAsian </code></li>
*  <li><code> CharHeightComplex </code></li>
*  <li><code> CharHeightAsian </code></li>
*  <li><code> CharFontPitchComplex    </code></li>
*  <li><code> CharFontPitchAsian </code></li>
*  <li><code> CharFontPitch </code></li>
*  <li><code> CharFontFamilyComplex </code></li>
*  <li><code> CharFontFamilyAsian </code></li>
*  <li><code> CharFontFamily </code></li>
*  <li><code> CharFontCharSetComplex </code></li>
*  <li><code> CharFontCharSetAsian    </code></li>
*  <li><code> CharFontCharSet </code></li>
*  <li><code> CharFontStyleNameComplex    </code></li>
*  <li><code> CharFontStyleNameAsian </code></li>
*  <li><code> CharFontStyleName </code></li>
*  <li><code> CharFontNameComplex </code></li>
*  <li><code> CharFontNameAsian </code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.TableAutoFormatField
*/
public class _TableAutoFormatField extends MultiPropertyTest {

    /**
    * Only some values can be used (which identify font name).
    * In this property value is changed from 'Times New Roman'
    * to 'Courier' and vise versa.
    */
    public void _CharFontName() {
        testProperty("CharFontName", new PropertyTester() {
            protected Object getNewValue(String p, Object old) {
                return "Courier".equals(old) ? "Times New Roman" : "Courier" ;
            }
        }) ;
    }

    /**
    * Locale values are predefined and can't be chaged arbitrary.
    * In this property value is changed from ('de', 'DE', '')
    * to ('es', 'ES', '') and vise versa.
    */
    public void _CharLocale() {
        testProperty("CharLocale", new PropertyTester() {
            protected Object getNewValue(String p, Object old) {
                return old == null || ((Locale)old).Language == "de" ?
                    new Locale("es", "ES", "") : new Locale("de", "DE", "") ;
            }
        }) ;
    }

    /**
    * This property can be void, so if old value is <code> null </code>
    * new value must be specified.
    */
    public void _ShadowFormat() {
        testProperty("ShadowFormat", new PropertyTester() {
            protected Object getNewValue(String p, Object old) {
                return old == null ? new com.sun.star.table.ShadowFormat() :
                    super.getNewValue(p, old) ;
            }
        }) ;
    }

} //finish class _TableAutoFormatField



