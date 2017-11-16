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
    * to 'Courier' and viceversa.
    */
    public void _CharFontName() {
        testProperty("CharFontName", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object old) {
                return "Courier".equals(old) ? "Times New Roman" : "Courier" ;
            }
        }) ;
    }

    /**
    * Locale values are predefined and can't be arbitrary changed.
    * In this property value is changed from ('de', 'DE', '')
    * to ('es', 'ES', '') and vice versa.
    */
    public void _CharLocale() {
        testProperty("CharLocale", new PropertyTester() {
            @Override
            protected Object getNewValue(String p, Object old) {
                return old == null || ((Locale)old).Language.equals( "de" ) ?
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
            @Override
            protected Object getNewValue(String p, Object old) {
                return old == null ? new com.sun.star.table.ShadowFormat() :
                    super.getNewValue(p, old) ;
            }
        }) ;
    }

} //finish class _TableAutoFormatField



