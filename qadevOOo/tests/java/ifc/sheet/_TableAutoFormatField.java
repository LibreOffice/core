/*************************************************************************
 *
 *  $RCSfile: _TableAutoFormatField.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:57:45 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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



