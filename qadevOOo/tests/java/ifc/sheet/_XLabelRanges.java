/*************************************************************************
 *
 *  $RCSfile: _XLabelRanges.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:01:48 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XLabelRanges;
import com.sun.star.table.CellRangeAddress;

/**
* Testing <code>com.sun.star.sheet.XLabelRanges</code>
* interface methods :
* <ul>
*  <li><code> addNew()</code></li>
*  <li><code> removeByIndex()</code></li>
* </ul> <p>
* @see com.sun.star.sheet.XLabelRanges
*/
public class _XLabelRanges extends MultiMethodTest {

    public XLabelRanges oObj = null;

    /**
    * Test calls the method and compares number of label range before method
    * call and after. <p>
    * Has <b>OK</b> status if number of label range before method call is less
    * than after and no exceptions were thrown. <p>
    */
    public void _addNew() {
        int anz = oObj.getCount();
        oObj.addNew(
            new CellRangeAddress((short)0, 1, 0, 1, 0),
            new CellRangeAddress((short)0, 1, 1, 1, 6) );
        tRes.tested("addNew()", anz < oObj.getCount());
    }

    /**
    * Test removes an existent label range first and tries to remove
    * non-existent label range. <p>
    * Has <b> OK </b> status if number of range decreased by one after first
    * call and exception was thrown in second. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> addNew() </code> : to have one label range at least </li>
    * </ul>
    */
    public void _removeByIndex() {
        requiredMethod("addNew()");
        int anz = oObj.getCount();
        log.println("First remove an existent LabelRange");

        oObj.removeByIndex(anz - 1);
        boolean result = (anz - 1 == oObj.getCount());

        log.println("Remove a nonexistent LabelRange");
        try {
            oObj.removeByIndex(anz);
            log.println("No Exception thrown while removing nonexisting "+
                "LabelRange");
            result &= false;
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println("Expected exception thrown while removing "+
                "nonexisting LabelRange: "+e);
            result &= true;
        }

        tRes.tested("removeByIndex()", result);
    }

}  // finish class _XLabelRanges


