/*************************************************************************
 *
 *  $RCSfile: _XIsolatedConnection.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:53:43 $
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

import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XIsolatedConnection;
import com.sun.star.task.XInteractionHandler;

/**
 * Testing <code>com.sun.star.sdb.XCompletedConnection</code>
 * interface methods :
 * <ul>
 *  <li><code> getIsolatedConnectionWithCompletion()</code></li>
 *  <li><code> getIsolatedConnection()</code></li>
 * </ul> <p>
*    The following object relations required :
* <ul>
* <li> <code>'XCompletedConnection.Handler'</code> : passed as parameter
* to <code>connectWithCompletion</code> method. </li>
* </ul>
* @see com.sun.star.sdb.XIsolatedConnection
* @see com.sun.star.task.XInteractionHandler
* @see com.sun.star.sdbc.XConnection
*/
public class _XIsolatedConnection extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XIsolatedConnection oObj = null ;

    /**
    * Test call the method with handler passed as object relation.
    * Then value returned is checked.<p>
    * Has OK status if not null value returned. <&nbsp>
    * FAILED if exception occured, null value returned or object
    * relation was not found.
    */
    public void _getIsolatedConnectionWithCompletion() throws StatusException {
        XInteractionHandler handler = (XInteractionHandler)
            tEnv.getObjRelation("XCompletedConnection.Handler") ;

        if (handler == null) {
            log.println("Required object relation not found !") ;
            tRes.tested("getIsolatedConnectionWithCompletion()", false) ;
            return ;
        }

        XConnection con = null ;
        try {
            con = oObj.getIsolatedConnectionWithCompletion(handler) ;
        } catch (com.sun.star.sdbc.SQLException e) {
            throw new StatusException("Exception while method calling", e) ;
        }

        tRes.tested("getIsolatedConnectionWithCompletion()", con != null) ;
    }

    /**
    * Test call the method with handler passed as object relation.
    * Then value returned is checked.<p>
    * Has OK status if not null value returned. <&nbsp>
    * FAILED if exception occured, null value returned or object
    * relation was not found.
    */
    public void _getIsolatedConnection() throws StatusException {
        String[] userSettings = (String[])
                        tEnv.getObjRelation("UserAndPassword") ;

        String user = null;
        String pwd = null;
        if (userSettings == null) {
            log.println("Required object relation not found !") ;
        }

        if (userSettings[0] != null)
            user = userSettings[0].equals("")?"<empty>":userSettings[0];
        else
            user = "<null>";
        if (userSettings[1] != null)
            pwd = userSettings[1].equals("")?"<empty>":userSettings[1];
        else
            pwd = "<null>";

        log.println("Testing \"getIsolatedConnection('user', 'password')\"\n" +
                    "with user = '" + user + "'; password = '" + pwd + "'");
        XConnection con = null ;
        try {
            con = oObj.getIsolatedConnection(user, pwd) ;
        } catch (com.sun.star.sdbc.SQLException e) {
            throw new StatusException("Exception while method calling", e) ;
        }

        tRes.tested("getIsolatedConnection()", con != null) ;
    }
}  // finish class _XIsolatedConnection

