/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XDeleteRows.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:33:54 $
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

package ifc.sdbcx;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbcx.XDeleteRows;
import com.sun.star.sdbcx.XRowLocate;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sdbcx.XDeleteRows</code>
* interface methods :
* <ul>
*  <li><code> deleteRows()</code></li>
* </ul> <p>
* @see com.sun.star.sdbcx.XDeleteRows
*/
public class _XDeleteRows extends MultiMethodTest {
    // oObj filled by MultiMethodTest
    public XDeleteRows oObj = null ;

    /**
    * Retrieves bookmark using XRowLocate and deletes
    * row pointed by this bookmark. <p>
    * Has OK status if number of rows after deleting is less than before
    * and no exception rizes while method call, FAILED otherwise. <p>
    */
    public void _deleteRows() {
        XRowLocate xRowLocate = (XRowLocate)
            UnoRuntime.queryInterface(XRowLocate.class, oObj);
        XResultSet xResultSet = (XResultSet)
            UnoRuntime.queryInterface(XResultSet.class, oObj);
        if (xRowLocate == null || xResultSet == null) {
            log.println("The test must be modified according to "+
                "component testcase");
            throw new StatusException(Status.failed(
                "The component doesn't support one of the "+
                    "required interfaces"));
        }

        int rowsBefore = 0, rowsAfter = 0;
        Object bkmrk = null;
        try {
            xResultSet.last();
            rowsBefore = xResultSet.getRow();
            xResultSet.first();
            bkmrk = xRowLocate.getBookmark();
            oObj.deleteRows(new Object[] {bkmrk});
            xResultSet.last();
            rowsAfter = xResultSet.getRow();
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("SQLException:" + e);
            tRes.tested("deleteRows()", false);
            return;
        }

        log.println("Rows before: " + rowsBefore + ", after: " + rowsAfter);
        tRes.tested("deleteRows()", rowsBefore - 1 == rowsAfter);
    }

    protected void after() {
        disposeEnvironment();
    }
}  // finish class _XDeleteRows

