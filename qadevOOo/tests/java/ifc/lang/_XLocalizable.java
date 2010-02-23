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