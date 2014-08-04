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
 * Test is <b> NOT </b> multithread compliant. <p>
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


