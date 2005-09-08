/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _Spreadsheet.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:39:04 $
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

/**
* Testing <code>com.sun.star.sheet.Spreadsheet</code>
* service properties :
* <ul>
*  <li><code> IsVisible</code></li>
*  <li><code> PageStyle</code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.sheet.Spreadsheet
*/
public class _Spreadsheet extends MultiPropertyTest {

    /**
     *This class is destined to custom test of property <code>PageStyle</code>.
     */
    protected PropertyTester styleTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue) {
            String str = "Default";
            String str2= "Report";
            return str.equals(oldValue) ? str2 : str;
        }
    };

    /**
     * Test property <code>PageStyle</code> using custom <code>PropertyTest</code>.
     */
    public void _PageStyle() {
        testProperty("PageStyle", styleTester);
    }
} // finish class _Spreadsheet


