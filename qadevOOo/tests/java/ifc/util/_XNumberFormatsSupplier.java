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
 * Test is <b> NOT </b> multithread compliant. <p>
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


