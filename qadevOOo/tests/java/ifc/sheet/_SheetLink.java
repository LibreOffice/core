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
import util.ValueChanger;

/**
* Testing <code>com.sun.star.sheet.SheetLink</code>
* service properties :
* <ul>
*  <li><code> Url</code></li>
*  <li><code> Filter</code></li>
*  <li><code> FilterOptions</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.SheetLink
*/
public class _SheetLink extends MultiPropertyTest {

    /**
     *This class is destined to custom test of property <code>Url</code>.
     */
    protected PropertyTester UrlTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            String newValue = (String) ValueChanger.changePValue(oldValue);
            if ( !newValue.startsWith("file://") ) {
                newValue = "file://" + newValue;
            }

            return newValue;
        }
    };

    /**
     * Test property <code>Url</code> using custom <code>PropertyTest</code>.
     */
    public void _Url() {
        testProperty("Url", UrlTester);
    }

    /**
     *This class is destined to custom test of property <code>Filter</code>.
     */
    protected PropertyTester FilterTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            return "StarCalc 4.0";
        }
    };

    /**
     * Test property <code>Filter</code> using custom <code>PropertyTest</code>.
     */
    public void _Filter() {
        testProperty("Filter", FilterTester);
    }
}


