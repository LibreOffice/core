/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XAvailableLocales.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:17:43 $
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

package ifc.linguistic2;

import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.XAvailableLocales;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.linguistic2.XAvailableLocales</code>
* interface methods:
* <ul>
*   <li><code>getAvailableLocales()</code></li>
* </ul> <p>
* @see com.sun.star.linguistic2.XAvailableLocales
*/
public class _XAvailableLocales extends MultiMethodTest {

    public XAvailableLocales oObj = null;
    public static Locale[] locales = new Locale[0];

    /**
    * Test calls the method, stores returned value and checks it. <p>
    * Has <b> OK </b> status if length of returned array isn't zero. <p>
    */
    public void _getAvailableLocales() {
        locales = oObj.getAvailableLocales("com.sun.star.linguistic2.Hyphenator");
        tRes.tested("getAvailableLocales()", locales.length > 0);
    }

}  // finish class XAvailableLocales


