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

import com.sun.star.util.XSearchDescriptor;

/**
 * Testing <code>com.sun.star.util.XSearchDescriptor</code>
 * interface methods :
 * <ul>
 *  <li><code> getSearchString()</code></li>
 *  <li><code> setSearchString()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.util.XSearchDescriptor
 */
public class _XSearchDescriptor extends MultiMethodTest {

       // oObj filled by MultiMethodTest
    public XSearchDescriptor oObj = null;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getSearchString(){
        boolean bResult = false;
        String searchStr = null;

        log.println("test for getSearchString() ");
        searchStr = oObj.getSearchString();

        if (searchStr != null){ bResult = true; }
        tRes.tested("getSearchString()", bResult);
    }

    /**
     * Set a new string and checks the result. <p>
     * Has <b> OK </b> status if the string before setting differs
     * from string after setting. <p>
     */
    public void _setSearchString(){
        boolean bResult = false;
        String oldSearchStr = null;
        String cmpSearchStr = null;
        String newSearchStr = "_XSearchDescriptor";
        log.println("test for setSearchString() ");

        oldSearchStr = oObj.getSearchString();
        oObj.setSearchString(newSearchStr);
        cmpSearchStr = oObj.getSearchString();

        if(!(cmpSearchStr.equals(oldSearchStr))){ bResult = true; }
        tRes.tested("setSearchString()", bResult);
    }

}  // finish class _XSearchDescriptor

