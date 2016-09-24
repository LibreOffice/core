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
package ifc.uno;

import lib.MultiMethodTest;
import util.ValueComparer;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


public class _XComponentContext extends MultiMethodTest {
    public XComponentContext oObj;
    protected XMultiComponentFactory byValue = null;
    protected XMultiComponentFactory directly = null;

    public void _getServiceManager() {
        log.println("getting manager");

        directly = oObj.getServiceManager();
        String[] names = directly.getAvailableServiceNames();
        boolean res = true;

        for (int i = 0; i < names.length; i++) {
            try {
                if (names[i].equals("com.sun.star.i18n.ConversionDictionary_ko")) continue;
                if (names[i].equals("com.sun.star.i18n.TextConversion_ko")) continue;
                log.println("try to instanciate found servicename " +
                            names[i]);
                directly.createInstanceWithContext(names[i], oObj);
                log.println("worked .... ok");
                res &= true;
            } catch (com.sun.star.uno.Exception e) {
                log.println("Exception occurred " + e.getMessage());
                res &= false;
            }
        }

        tRes.tested("getServiceManager()", res);
    }

    public void _getValueByName() {
        requiredMethod("getServiceManager()");

        Object value = oObj.getValueByName(
                               "/singletons/com.sun.star.lang.theServiceManager");
        byValue = UnoRuntime.queryInterface(
                          XMultiComponentFactory.class, value);

        byValue.getAvailableServiceNames();
        directly.getAvailableServiceNames();

        boolean res = ValueComparer.equalValue(byValue, directly);
        tRes.tested("getValueByName()", res);
    }
}