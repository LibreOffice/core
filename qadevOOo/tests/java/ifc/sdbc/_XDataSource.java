/*************************************************************************
 *
 *  $RCSfile: _XDataSource.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:53:11 $
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

import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDataSource;

/**
* Testing <code>com.sun.star.sdbc.XDataSource</code>
* interface methods :
* <ul>
*  <li><code>getConnection()</code></li>
*  <li><code>setLoginTimeout()</code></li>
*  <li><code>getLoginTimeout()</code></li>
* </ul> <p>
* @see com.sun.star.sdbc.XDataSource
*/
public class _XDataSource extends MultiMethodTest {
    // oObj filled by MultiMethodTest
    public XDataSource oObj = null;

    /**
     * Calls the method and checks returned value.
     * Has OK status if exception wasn't thrown and
     * if returned value isn't null.
     */
    public void _getConnection() {
        boolean res = true;

        try {
            XConnection connection = oObj.getConnection("", "");
            res = connection != null;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception:");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("getConnection()", res);
    }

    /**
     * Sets new timeout, compares with timeout returned by the method
     * <code>getLoginTimeout()</code>.
     * Has OK status if exception wasn't thrown and if timeout values are equal.
     */
    public void _setLoginTimeout() {
        requiredMethod("getLoginTimeout()");
        boolean res = true;

        try {
            final int TO = 111;
            log.println("setLoginTimeout(" + TO +  ")");
            oObj.setLoginTimeout(TO);
            int timeout = oObj.getLoginTimeout();
            res = timeout == TO;
            log.println("getLoginTimeout(): " + timeout);
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception:");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("setLoginTimeout()", res);
    }

    /**
     * Calls the method and checks returned value.
     * Has OK status if exception wasn't thrown and
     * if returned value is equal to zero.
     */
    public void _getLoginTimeout() {
        boolean res = true;

        try {
            int timeout = oObj.getLoginTimeout();
            log.println("getLoginTimeout(): " + timeout);
            res = timeout == 0;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception:");
            e.printStackTrace(log);
            res = false;
        }

        tRes.tested("getLoginTimeout()", res);
    }
}