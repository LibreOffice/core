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



package ifc.sdb;

import lib.MultiPropertyTest;

/**
 * Testing <code>com.sun.star.sdb.ErrorMessageDialog</code>
 * service properties :
 * <ul>
 *  <li><code> Title</code></li>
 *  <li><code> ParentWindow</code></li>
 *  <li><code> SQLException</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'ERR1', 'ERR2'</code>
 *  (of type <code>com.sun.star.sdbc.SQLException</code>):
 *   two objects which are used for changing 'SQLException'
 *   property. </li>
 * <ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.sdb.ErrorMessageDialog
 */
public class _ErrorMessageDialog extends MultiPropertyTest {

    /**
     * <code>SQLException</code> instances must be used as property
     * value.
     */
    public void _SQLException() {
        log.println("Testing with custom Property tester") ;
        testProperty("SQLException", tEnv.getObjRelation("ERR1"),
            tEnv.getObjRelation("ERR2")) ;
    }

    public void _ParentWindow(){
        log.println("Testing with custom Property tester");
        testProperty("ParentWindow", tEnv.getObjRelation("ERR_XWindow"), null);
    }

}  // finish class _ErrorMessageDialog


