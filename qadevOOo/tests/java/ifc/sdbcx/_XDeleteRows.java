/*************************************************************************
 *
 *  $RCSfile: _XDeleteRows.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:56:08 $
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

