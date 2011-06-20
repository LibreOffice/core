/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
            log.println("Exception occurred:");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("findColumn()", result);
    }

}  // finish class _XColumnLocate

