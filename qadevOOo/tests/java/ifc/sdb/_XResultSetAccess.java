/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XResultSetAccess.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:27:29 $
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

package ifc.sdb;

import lib.MultiMethodTest;

import com.sun.star.sdb.XResultSetAccess;
import com.sun.star.sdbc.XResultSet;

/**
* Testing <code>com.sun.star.sdb.XResultSetAccess</code>
* interface methods :
* <ul>
*  <li><code> createResultSet()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.sdb.XResultSetAccess
*/
public class _XResultSetAccess extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XResultSetAccess oObj = null ;

    /**
    * tries to create a ResultSet for the Object</br>
    * an XResultSet is returned.</br>
    * The test is OK if an not null ResultSet is returned
    */
    public void _createResultSet() {

        try {
            XResultSet the_Set = oObj.createResultSet();
            if (the_Set == null) log.println("'createResulSet()' returns NULL");
            tRes.tested("createResultSet()",the_Set != null);
        } catch (com.sun.star.sdbc.SQLException e) {
            log.println("Exception while checking 'createResultSet()'");
            e.printStackTrace(log);
            tRes.tested("createResultSet()",false);
        }

    }
}  // finish class _XResultSetAccess


