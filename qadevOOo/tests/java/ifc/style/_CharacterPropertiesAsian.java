/*************************************************************************
 *
 *  $RCSfile: _CharacterPropertiesAsian.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:08:04 $
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