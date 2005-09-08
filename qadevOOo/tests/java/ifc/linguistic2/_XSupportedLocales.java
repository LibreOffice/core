/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSupportedLocales.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:19:36 $
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

import lib.MultiMethodTest;

import com.sun.star.lang.Locale;
import com.sun.star.linguistic2.XSupportedLocales;

/**
* Testing <code>com.sun.star.linguistic2.XSupportedLocales</code>
* interface methods:
* <ul>
*   <li><code>getLocales()</code></li>
*   <li><code>hasLocale()</code></li>
* </ul> <p>
* @see com.sun.star.linguistic2.XSupportedLocales
*/
public class _XSupportedLocales extends MultiMethodTest {

    public XSupportedLocales oObj = null;
    public static Locale[] locales = new Locale[0];

    /**
    * Test calls the method, stores returned value and checks it. <p>
    * Has <b> OK </b> status if length of returned array isn't zero. <p>
    */
    public void _getLocales() {
        locales = oObj.getLocales();
        tRes.tested("getLocales()", locales.length > 0);
    }

    /**
    * Test calls the method for every language from list of supported languages,
    * calls the method for one unsupported language and checks
    * all returned values. <p>
    * Has <b> OK </b> status if all returned values for supported languages
    * are equal to true and if returned value for unsupported language is equal
    * to false. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> getLocales() </code> : to have list of
    *  all supported languages </li>
    * </ul>
    */
    public void _hasLocale() {
        requiredMethod("getLocales()");
        boolean res = true;

        // first check all available locales
        for (int i=0; i< locales.length; i++) {
            res &= oObj.hasLocale(locales[i]);
        }

        // now create a invalid locale
        Locale invalid = new Locale("myLanguage","myCountry","");
        res &= !oObj.hasLocale(invalid);

        tRes.tested("hasLocale()", res);
    }

}  // finish class XSupportedLocales


