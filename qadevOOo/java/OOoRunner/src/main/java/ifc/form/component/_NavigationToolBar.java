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



package ifc.form.component;

import lib.MultiPropertyTest;

public class _NavigationToolBar extends MultiPropertyTest {

    public void _TextColor() {
        testProperty("TextColor", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? new Integer(20010507) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }

    public void _TextLineColor() {
        testProperty("TextLineColor", new PropertyTester() {
            protected Object getNewValue(String prop, Object old) {
                return util.utils.isVoid(old) ? new Integer(20050507) :
                    super.getNewValue(prop, old) ;
            }
        }) ;
    }

}