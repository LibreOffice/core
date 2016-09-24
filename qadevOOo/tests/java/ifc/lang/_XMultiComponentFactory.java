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

package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;

/**
* Testing <code>com.sun.star.lang.XMultiComponentFactory</code>
* interface methods :
* <ul>
*  <li><code> createInstanceWithContext()</code></li>
*  <li><code> createInstanceWithArgumentsAndContext()</code></li>
*  <li><code> getAvailableServiceNames()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.lang.XMultiComponentFactory
*/
public class _XMultiComponentFactory extends MultiMethodTest {
    public XMultiComponentFactory oObj = null;

    public XComponentContext xContext = null;
    private String[] availableServiceNames = null;

    @Override
    public void before(){
        xContext = (XComponentContext)tEnv.getObjRelation("DC");
        availableServiceNames = (String[])tEnv.getObjRelation("XMultiComponentFactory.ServiceNames");
    }

    /**
    * Calls the method with one of the available service names
    * obtained by method getAvailableServiceNames. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is not null.
    */
    public void _createInstanceWithContext() {
        requiredMethod("getAvailableServiceNames()");
        boolean result = true;

        try {
            XInterface component = (XInterface)
                oObj.createInstanceWithContext(
                    availableServiceNames[0], xContext);
            result = (component != null);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create instance " + availableServiceNames[0]);
            result = false;
        }

        tRes.tested("createInstanceWithContext()", result);
    }

    /**
    * Calls the method with one of the available service names
    * obtained by method getAvailableServiceNames. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is not null.
    */
    public void _createInstanceWithArgumentsAndContext() {
        requiredMethod("getAvailableServiceNames()");
        boolean result = true;
        XInterface component = null;

        try {
            component = (XInterface)oObj.createInstanceWithArgumentsAndContext(
                    availableServiceNames[0], new Object[0], xContext);
            result = (component != null);
        } catch (com.sun.star.uno.Exception e) {
            log.println("Couldn't create instance " + availableServiceNames[0]);
            result = false;
        }

        tRes.tested("createInstanceWithArgumentsAndContext()", result);
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred
    * and returned value is not null.
    */
    public void _getAvailableServiceNames() {
        boolean result = true;
        if (availableServiceNames == null) {
            availableServiceNames = oObj.getAvailableServiceNames();
            result = (availableServiceNames != null);
        }
        else { // if service names are given, ignore result
            String[]erg = oObj.getAvailableServiceNames();
            result = (erg != null);
        }

        log.println("Available service names:");
        for(int i = 0; i < availableServiceNames.length; i++) {
            log.println("   " + availableServiceNames[i]);
        }

        tRes.tested("getAvailableServiceNames()", result);
    }
}

