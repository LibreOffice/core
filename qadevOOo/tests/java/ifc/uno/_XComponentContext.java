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
        byValue = (XMultiComponentFactory) UnoRuntime.queryInterface(
                          XMultiComponentFactory.class, value);

        String[] vNames = byValue.getAvailableServiceNames();
        String[] dNames = directly.getAvailableServiceNames();

        boolean res = ValueComparer.equalValue(byValue, directly);
        tRes.tested("getValueByName()", res);
    }
}