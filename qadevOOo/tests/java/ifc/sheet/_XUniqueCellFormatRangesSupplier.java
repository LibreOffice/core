/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XUniqueCellFormatRangesSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:59:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.container.XIndexAccess;
import com.sun.star.sheet.XUniqueCellFormatRangesSupplier;

public class _XUniqueCellFormatRangesSupplier extends MultiMethodTest {

    public XUniqueCellFormatRangesSupplier oObj;

    /**
     * call the method getUniqueCellFormatRanges and returns OK result if
     * the gained XIndexAccess isn't null and the method checkIndexAccess
     * returns true.
     */

    public void _getUniqueCellFormatRanges() {
        boolean res = true;
        XIndexAccess xIA = oObj.getUniqueCellFormatRanges();
        if (xIA != null) {
            res = checkIndexAccess(xIA);
        } else {
            log.println("The gained IndexAccess is null");
            res = false;
        }
        tRes.tested("getUniqueCellFormatRanges()",res);
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
