/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XColumnLocate.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 00:29:28 $
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
import lib.Status;
import lib.StatusException;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XColumnLocate;

/**
/**
* Testing <code>com.sun.star.sdbc.XColumnLocate</code>
* interface methods :
* <ul>
*  <li><code> findColumn()</code></li>
* </ul> <p>
* The test required the following relations :
* <ul>
* <li> <code> XColumnLocate.ColumnName </code> : The name of the first
* column. </li>
* </ul>
* @see com.sun.star.sdbc.XColumnLocate
*/
public class _XColumnLocate extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XColumnLocate oObj = null ;

    /**
    * Using column name from object relation trying to find out
    * the index of this column. <p>
    * Has OK status if column index returned by method equals to 1,
    * FAILED otherwise.
    */
    public void _findColumn() {
        boolean result = false ;
        String colName = (String) tEnv.getObjRelation("XColumnLocate.ColumnName") ;

        if (colName == null) {
            throw new StatusException(
                Status.failed("Object relation 'XColumnLocate.ColumnName' " +
                    "for this interface not found"));
        }

        try {
            int colIdx = oObj.findColumn(colName) ;
            result = colIdx == 1 ;
        } catch (SQLException e) {
            log.println("Exception occured:");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("findColumn()", result);
    }

}  // finish class _XColumnLocate

