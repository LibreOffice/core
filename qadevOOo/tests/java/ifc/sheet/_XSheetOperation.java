/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XSheetOperation.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:56:36 $
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

import com.sun.star.sheet.CellFlags;
import com.sun.star.sheet.GeneralFunction;
import com.sun.star.sheet.XSheetOperation;

/**
* Testing <code>com.sun.star.sheet.XSheetOperation</code>
* interface methods :
* <ul>
*  <li><code> computeFunction()</code></li>
*  <li><code> clearContents()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XSheetOperation
*/
public class _XSheetOperation extends MultiMethodTest {

    public XSheetOperation oObj = null;

    /**
    * Test clears formula and value contents, calls the method
    * <code>computeFunction</code> and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal to zero
    * and no exceptions were thrown. <p>
    */
    public void _clearContents() {
        boolean result = true;
        double resultVal = -1;

        log.println("Testing clearContents() ...");

        int allFlags;
        allFlags = CellFlags.VALUE | CellFlags.FORMULA;

        oObj.clearContents (allFlags) ;

        try {
            resultVal = oObj.computeFunction(GeneralFunction.SUM);
            result &= (resultVal == 0.0) || (resultVal == 0);
        } catch (com.sun.star.uno.Exception e) {
            result &= false ;
            log.println(
                    "Exception occured while checking results of method");
            e.printStackTrace(log);
        }

        tRes.tested("clearContents()", result);

    } // finished clearContents

    /**
    * Test calls the method and checks returned value. <p>
    * Has <b> OK </b> status if returned value is equal or greate than zero
    * and no exceptions were thrown. <p>
    */
    public void _computeFunction() {

        log.println("Testing computeFunction() ...");
        double resultVal = -1;
        boolean result = true;

        try {
            resultVal = oObj.computeFunction (GeneralFunction.COUNT) ;
            result = resultVal >= 0;
        } catch (com.sun.star.uno.Exception e) {
            result = false;
            log.println("Exception occured in method computeFunction.");
            e.printStackTrace(log);
        }

        tRes.tested("computeFunction()", result);
    } // finished computeFunction

} // finished class _XSheetOperation

