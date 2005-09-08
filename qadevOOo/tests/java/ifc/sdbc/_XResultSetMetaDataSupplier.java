/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XResultSetMetaDataSupplier.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:31:15 $
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

package ifc.sdbc;

import lib.MultiMethodTest;

import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;

/**
* Testing <code>com.sun.star.sdbc.XResultSetMetaDataSupplier</code>
* interface methods :
* <ul>
*  <li><code> getMetaData()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.sdbc.XResultSetMetaDataSupplier
*/
public class _XResultSetMetaDataSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XResultSetMetaDataSupplier oObj = null ;

    /**
    * Tries to get the ResultSetMetaData of the Object</br>
    * an XResultSetMetaData is returned.</br>
    * The test is OK if an not null ResultSetMetaData is returned
    */
    public void _getMetaData() {

        try {
            XResultSetMetaData the_Meta = oObj.getMetaData();
            if (the_Meta == null) log.println("'getMetaData()' returns NULL");
            tRes.tested("getMetaData()",the_Meta != null);
        } catch (com.sun.star.sdbc.SQLException e) {
            log.println("Exception while checking 'createResultSet()'");
            e.printStackTrace(log);
            tRes.tested("getMetaData()",false);
        }

    }
}  // finish class _XResultSetMetaDataSupplier


