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

package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.beans.XPropertySet;
import com.sun.star.util.XNumberFormats;
import com.sun.star.util.XNumberFormatsSupplier;

/**
 * Testing <code>com.sun.star.util.XNumberFormatsSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getNumberFormatSettings()</code></li>
 *  <li><code> getNumberFormats()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XNumberFormatsSupplier
 */
public class _XNumberFormatsSupplier extends MultiMethodTest {

    public XNumberFormatsSupplier oObj = null;

    /**
     * Get format settings and checks some properties for existence. <p>
     *
     * Has <b> OK </b> status if a number properties inherent to
     * <code>NumberFormatSettings</code> service exist in the
     * returned <code>XPropertySet</code>. <p>
     *
     * @see com.sun.star.util.NumberFormatSettings
     */
    public void _getNumberFormatSettings() {
        boolean result = true ;
        XPropertySet props = oObj.getNumberFormatSettings();

        if (props != null) {
            try {
              result &= props.getPropertyValue("NullDate") != null &&
                      props.getPropertyValue("StandardDecimals") != null &&
                      props.getPropertyValue("NoZero") != null &&
                      props.getPropertyValue("TwoDigitDateStart") != null ;
            } catch (com.sun.star.beans.UnknownPropertyException e) {
                log.println("Some property doesn't exist") ;
                e.printStackTrace(log) ;
                result = false ;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                e.printStackTrace(log) ;
                result = false ;
            }
        } else {
            log.println("Method returns null") ;
            result = false ;
        }

        tRes.tested("getNumberFormatSettings()", result) ;
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getNumberFormats() {
        XNumberFormats formats = oObj.getNumberFormats();

        tRes.tested("getNumberFormats()", formats != null) ;
    }

}


