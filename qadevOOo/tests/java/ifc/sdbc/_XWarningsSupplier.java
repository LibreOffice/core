/*************************************************************************
 *
 *  $RCSfile: _XWarningsSupplier.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:55:13 $
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

package ifc.sdbc;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;
import util.DBTools;
import util.utils;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.sdbc.XWarningsSupplier;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sdbc.XWarningsSupplier</code>
* interface methods :
* <ul>
*  <li><code> getWarnings()</code></li>
*  <li><code> clearWarnings()</code></li>
* </ul> <p>
* @see com.sun.star.sdbc.XWarningsSupplier
*/
public class _XWarningsSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XWarningsSupplier oObj = null ;

    /**
    * Updates value of int column by value '9999999999999999'.
    * Calls method and checks returned value. <p>
    * Has OK status if the method return not empty value.
    */
    public void _getWarnings() {
        final XRowUpdate xRowUpdate = (XRowUpdate)
            UnoRuntime.queryInterface(XRowUpdate.class, oObj);
        final XResultSetUpdate xResSetUpdate = (XResultSetUpdate)
            UnoRuntime.queryInterface(XResultSetUpdate.class, oObj);
        final XRow xRow = (XRow)
            UnoRuntime.queryInterface(XRow.class, oObj);
        if (xRowUpdate == null || xResSetUpdate == null || xRow == null) {
            throw new StatusException(Status.failed("Test must be modified"));
        }
        int oldVal = 0, newVal = 0;
        String valToSet = "9999999999999999";
        try {
            oldVal = xRow.getInt(DBTools.TST_INT);
            xRowUpdate.updateString(DBTools.TST_INT, valToSet);
            xResSetUpdate.updateRow();
            newVal = xRow.getInt(DBTools.TST_INT);
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected SQL exception");
            e.printStackTrace(log);
            tRes.tested("getWarnings()", false);
            return;
        }

        log.println("Old INT value: " + oldVal);
        log.println("Value that was set: " + valToSet);
        log.println("New INT value: " + newVal);

        boolean res = false;

        try {
            Object warns = oObj.getWarnings();
            res = (!utils.isVoid(warns));
        } catch (SQLException e) {
            log.println("Exception occured :");
            e.printStackTrace(log);
            tRes.tested("getWarnings()", res);
            return;
        }

        tRes.tested("getWarnings()", res);
    }

    /**
    * Calls method and checks value returned by the method
    * <code>getWarnings()</code>. <p>
    * Has OK status if the method <code>getWarnings()</code> return void value.
    */
    public void _clearWarnings() {
        executeMethod("getWarnings()");
        boolean res = false;

        try {
            oObj.clearWarnings();
            Object warns = oObj.getWarnings();
            res = (utils.isVoid(warns));
        } catch (SQLException e) {
            log.println("Exception occured :");
            e.printStackTrace(log);
            tRes.tested("clearWarnings()", res);
            return;
        }

        tRes.tested("clearWarnings()", res);
    }

}