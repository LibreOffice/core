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



package ifc.util;

import lib.MultiMethodTest;

import com.sun.star.util.XStringEscape;

public class _XStringEscape extends MultiMethodTest {

    public XStringEscape oObj;

    public void _escapeString() {
        log.println("The Implementation of this Interface doesn't really do anything");
        boolean res = true;
        try {
            String toCheck = ";:<>/*";
            String eString = oObj.escapeString(toCheck);
            res = toCheck.equals(eString);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("IllegalArgument");
        }
        tRes.tested("escapeString()",res);
    }

    public void _unescapeString() {
        log.println("The Implementation of this Interface doesn't really do anything");
        boolean res = true;
        try {
            String toCheck = ";:<>/*";
            String ueString = oObj.unescapeString(toCheck);
            res = toCheck.equals(ueString);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            System.out.println("IllegalArgument");
        }
        tRes.tested("unescapeString()",res);
    }

}
