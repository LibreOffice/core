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



package ifc.presentation;

import lib.MultiPropertyTest;

public class _PresentationView extends MultiPropertyTest {

    /**
     * Property tester which changes DrawPage.
     */
    protected PropertyTester PageTester = new PropertyTester() {
        protected Object getNewValue(String propName, Object oldValue)
                throws java.lang.IllegalArgumentException {
            if (oldValue.equals(tEnv.getObjRelation("FirstPage")))
                return tEnv.getObjRelation("SecondPage"); else
                return tEnv.getObjRelation("FirstPage");
        }
    } ;

    /**
     * This property must be an XDrawPage
     */
    public void _CurrentPage() {
        log.println("Testing with custom Property tester") ;
        testProperty("CurrentPage", PageTester) ;
    }

}  // finish class _PresentationView


