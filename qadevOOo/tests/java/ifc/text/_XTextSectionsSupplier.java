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
package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.text.XTextSectionsSupplier;


public class _XTextSectionsSupplier extends MultiMethodTest {
    public XTextSectionsSupplier oObj;

    public void _getTextSections() {
        XNameAccess sections = oObj.getTextSections();
        boolean res = checkSections(sections);
        tRes.tested("getTextSections()", res);
    }

    protected boolean checkSections(XNameAccess sections) {
        String[] sNames = sections.getElementNames();
        boolean res = true;

        for (int k = 0; k < sNames.length; k++) {
            try {
                res &= sections.hasByName(sNames[k]);
                res &= (sections.getByName(sNames[k]) != null);
                log.println("Works for ... " + sNames[k]);
            } catch (com.sun.star.container.NoSuchElementException e) {
                log.println("positive test failed " + e.getMessage());
                res = false;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("positive test failed " + e.getMessage());
                res = false;
            }
        }

        try {
            sections.getByName("unknown");
            log.println("negative test failed ... no Exception thrown");
            res = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("expected Exception for wrong argument ... OK");
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.println("negative test failed ... wrong Exception thrown");
            res = false;
        }

        return res;
    }
}