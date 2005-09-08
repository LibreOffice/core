/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _SheetLink.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:38:33 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


