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