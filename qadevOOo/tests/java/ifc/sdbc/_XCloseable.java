/*************************************************************************
 *
 *  $RCSfile: _XCloseable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:52:49 $
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
import lib.StatusException;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sdbc.XCloseable</code>
* interface methods :
* <ul>
*  <li><code> close()</code></li>
* </ul> <p>
* After test object must be recreated.
* @see com.sun.star.sdbc.XCloseable
*/
public class _XCloseable extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XCloseable oObj = null ;

    /**
    * Closes row set. If the component implements the interface
    * <code>com.sun.star.sdbc.XResutlSet</code> then tries to move
    * the cursor to the first row in the result set.
    * Has OK status if no exceptions were thrown during first call and
    * if expected SQL exception was thrown during cursor moving.
    */
    public void _close() throws StatusException {
        boolean res = false;
        try {
            oObj.close();
            res = true;
        } catch (SQLException e) {
            log.println("Unexpected SQL Exception occured:" + e) ;
            res = false;
        }

        XResultSet resSet = (XResultSet)
            UnoRuntime.queryInterface(XResultSet.class, oObj);

        if (resSet != null) {
            try {
                resSet.first();
                log.println("Expected SQLException not occured !");
                res = false;
            } catch(SQLException e) {
                log.println("Expected SQLException occured");
                res = true;
            }
        }

        tRes.tested("close()", res);
    }

    /**
    * Forces environment recreation.
    */
    public void after() {
        disposeEnvironment() ;
    }

}  // finish class _XCloseable

