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


package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.container.XIndexAccess;
import com.sun.star.sheet.XCellFormatRangesSupplier;

/**
 * Interface test to check the interface XCellFormatRangesSupplier
 *
 * methods:
 *
 *  getCellFormatRanges()
 *
 */

public class _XCellFormatRangesSupplier extends MultiMethodTest {

    public XCellFormatRangesSupplier oObj = null;

    /**
     * call the method getCellFormatRanges and returns OK result if
     * the gained XIndexAccess isn't null and the method checkIndexAccess
     * returns true.
     */

    public void _getCellFormatRanges() {
        boolean res = true;
        XIndexAccess xIA = oObj.getCellFormatRanges();
        if (xIA != null) {
            res = checkIndexAccess(xIA);
        } else {
            log.println("The gained IndexAccess is null");
            res = false;
        }
        tRes.tested("getCellFormatRanges()",res);
    }

    /**
     * calls the method getCount at the IndexAccess, returns true is it is >0
     * and getByIndex() doesn't throw an exception for Indexes between 0 and count
     */

    protected boolean checkIndexAccess(XIndexAccess xIA) {
        boolean res = true;
        int count = xIA.getCount();
        log.println("Found "+count+" Elements");
        res &= count>0;
        for (int k=0; k<count; k++) {
            try {
                Object element = xIA.getByIndex(k);
                log.println("Element "+k+" = "+element);
            } catch (com.sun.star.lang.IndexOutOfBoundsException e) {
                log.println("Unexpected Exception while getting by Index ("+k+")"+e.getMessage());
                res &=false;
            } catch (com.sun.star.lang.WrappedTargetException e) {
                log.println("Unexpected Exception while getting by Index ("+k+")"+e.getMessage());
                res &=false;
            }
        }
        return res;
    }

}
