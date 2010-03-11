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

import com.sun.star.util.XNumberFormatter;


/**
 * Testing <code>com.sun.star.util.XNumberFormatter</code>
 * interface methods :
 * <ul>
 *  <li><code> attachNumberFormatsSupplier()</code></li>
 *  <li><code> convertNumberToString()      </code></li>
 *  <li><code> convertStringToNumber()      </code></li>
 *  <li><code> detectNumberFormat()         </code></li>
 *  <li><code> formatString()               </code></li>
 *  <li><code> getInputString()             </code></li>
 *  <li><code> getNumberFormatsSupplier()   </code></li>
 *  <li><code> queryColorForNumber()        </code></li>
 *  <li><code> queryColorForString()        </code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.util.XNumberFormatter
 */
public class _XNumberFormatter extends MultiMethodTest {

    public XNumberFormatter oObj = null;

    /**
     * Not implemented yet.
     */
    public void _attachNumberFormatsSupplier() {
        log.println("Not yet implemented");
    }

    /**
     * Tries to convert a number to a string. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _convertNumberToString() {
        double dValue = 1.56;
        int key = 15;

        String gString = oObj.convertNumberToString(key,dValue);

        log.println("Getting: "+gString);
        tRes.tested("convertNumberToString",gString!=null);
    }

    /**
     * Not implemented yet.
     */
    public void _convertStringToNumber() {
        log.println("Not yet implemented");
    }

    /**
     * Not implemented yet.
     */
    public void _detectNumberFormat() {
        log.println("Not yet implemented");
    }

    /**
     * Not implemented yet.
     */
    public void _formatString() {
        log.println("Not yet implemented");
    }

    /**
     * Not implemented yet.
     */
    public void _getInputString() {
        log.println("Not yet implemented");
    }

    /**
     * Not implemented yet.
     */
    public void _getNumberFormatsSupplier() {
        log.println("Not yet implemented");
    }

    /**
     * Not implemented yet.
     */
    public void _queryColorForNumber() {
        log.println("Not yet implemented");
    }

    /**
     * Not implemented yet.
     */
    public void _queryColorForString() {
        log.println("Not yet implemented");
    }
}  // finish class _XNumberFormatter


