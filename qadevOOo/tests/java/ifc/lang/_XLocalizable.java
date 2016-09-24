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

import com.sun.star.lang.Locale;
import com.sun.star.lang.XLocalizable;
import lib.Status;


public class _XLocalizable extends MultiMethodTest {

    public XLocalizable oObj;
    protected Locale initialLocale;

    public void _getLocale() {
        initialLocale = oObj.getLocale();
        tRes.tested("getLocale()", initialLocale != null);
    }

    public void _setLocale() {
        requiredMethod("getLocale()");

        String ro = (String) tEnv.getObjRelation("XLocalizable.ReadOnly");
        if (ro != null) {
            log.println(ro);
            tRes.tested("setLocale()", Status.skipped(true));
            return;
        }
        Locale newLocale = new Locale("de", "DE", "");
        oObj.setLocale(newLocale);

        Locale getLocale = oObj.getLocale();
        boolean res = ((getLocale.Country.equals(newLocale.Country)) &&
                      (getLocale.Language.equals(newLocale.Language)));

        if (!res) {
            log.println("Expected Language " + newLocale.Language +
                        " and Country " + newLocale.Country);
            log.println("Getting Language " + getLocale.Language +
                        " and Country " + getLocale.Country);
        }

        oObj.setLocale(initialLocale);
        tRes.tested("setLocale()", res);
    }
}