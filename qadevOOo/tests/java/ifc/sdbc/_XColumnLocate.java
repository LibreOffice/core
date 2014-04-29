/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

