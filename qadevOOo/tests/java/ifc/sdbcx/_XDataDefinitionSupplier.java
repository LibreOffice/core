/*************************************************************************
 *
 *  $RCSfile: _XDataDefinitionSupplier.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Date: 2003-09-08 10:55:57 $
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

import com.sun.star.beans.PropertyValue;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDriver;
import com.sun.star.sdbcx.XDataDefinitionSupplier;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;

/**
* Testing <code>com.sun.star.sdbcx.XDataDefinitionSupplier</code>
* interface methods :
* <ul>
*  <li><code> getDataDefinitionByConnection()</code></li>
*  <li><code> getDataDefinitionByURL()</code></li>
* </ul> <p>
* Required object relations :
* <ul>
* <li> <code>'XDriver.URL'</code>:
*      is the URL of the database to which to connect</code></li>
* <li><code>'XDriver.UNSUITABLE_URL'</code>:
*      the wrong kind of URL to connect using given driver</li>
* <li><code>'XDriver.INFO'</code>:
*      a list of arbitrary string tag/value pairs as connection arguments</li>
* </ul> <p>
* @see com.sun.star.sdbcx.XDataDefinitionSupplier
*/
public class _XDataDefinitionSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XDataDefinitionSupplier oObj = null ;

    String url = null;
    String wrongUrl = null;
    PropertyValue[] info = null;

    /**
    * Retrieves relations.
    * @throw StatusException If any relation not found.
    */
    protected void before() {
        url = (String)tEnv.getObjRelation("XDriver.URL");
        if (url == null) {
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XDriver.URL'"));
        }
        wrongUrl = (String)tEnv.getObjRelation("XDriver.UNSUITABLE_URL");
        if (wrongUrl == null) {
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XDriver.WRONG_URL'"));
        }
        info = (PropertyValue[])tEnv.getObjRelation("XDriver.INFO");
        if (info == null) {
            throw new StatusException(Status.failed(
                "Couldn't get relation 'XDriver.INFO'"));
        }
    }

    XConnection connection = null;

    /**
     * Obtains the connection to url(relation <code>'XDriver.URL'</code>)
     * with info(relation <code>'XDriver.INFO'</code>).
     * Calls the method with obtained connection and checks that returned value
     * isn't null.
     */
    public void _getDataDefinitionByConnection() {
        boolean bRes = true;
        XDriver xDriver = (XDriver)
            UnoRuntime.queryInterface(XDriver.class, oObj);
        if (xDriver == null) {
            log.println("The XDriver interface isn't supported");
            tRes.tested("getDataDefinitionByConnection()",
                        Status.skipped(false));
            return;
        }
        try {
            connection = xDriver.connect(url, info);
        } catch(com.sun.star.sdbc.SQLException e) {
            e.printStackTrace(log);
            bRes = false;
        }
        if (connection == null) {
            log.println("Couldn't get connection to specified url using " +
                "specified info");
            tRes.tested("getDataDefinitionByConnection()",
                        Status.skipped(false));
            return;
        }
        XTablesSupplier xTS = null;
        try {
            log.println("getDataDefinitionByConnection(connection)");
            xTS = oObj.getDataDefinitionByConnection(connection);
            bRes = xTS != null;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception: " + e);
            bRes = false;
        }

        try {
            log.println("getDataDefinitionByConnection(null)");
            xTS = oObj.getDataDefinitionByConnection(null);
            bRes = xTS == null;
        } catch(com.sun.star.sdbc.SQLException e) {
            log.println("Exception: " + e);
            bRes = true;
        }

        tRes.tested("getDataDefinitionByConnection()", bRes);
    }

    /**
     * Calls the method with url and info obtained from the relations
     * <code>XDriver.URL</code> and <code>XDriver.INFO</code>.
     * Checks that retuned value isn't null.
     * Then calls the method with the unsuitable url obtained from the relation
     * <code>XDriver.UNSUITABLE_URL</code> and checks that SQLException
     * exception was thrown.
     */
    public void _getDataDefinitionByURL() {
        boolean bRes = false;
        XTablesSupplier xTS = null;

        try {
            log.println("getDataDefinitionByURL('" + url + "')");
            xTS = oObj.getDataDefinitionByURL(url, info);
            bRes = xTS != null;
        } catch (com.sun.star.sdbc.SQLException e) {
            log.println("Unexpected exception: " + e);
            bRes = false;
        }

        try {
            log.println("getDataDefinitionByURL('" + wrongUrl + "')");
            xTS = oObj.getDataDefinitionByURL(wrongUrl, info);
            log.println("Exception was expected");
            bRes = false;
        } catch (com.sun.star.sdbc.SQLException e) {
            log.println("Expected exception");
            bRes &= true;
        }

        tRes.tested("getDataDefinitionByURL()", true);

    }
}  // finish class _XDataDefinitionSupplier


