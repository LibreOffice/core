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

import com.sun.star.util.XStringSubstitution;
import lib.MultiMethodTest;

public class _XStringSubstitution extends MultiMethodTest {

    public XStringSubstitution oObj;

    public void _getSubstituteVariableValue() {
        boolean res = true;
        try {
            log.println("try to get the valid variable $(user) ...");
            String toCheck = "$(user)";
            String eString = oObj.getSubstituteVariableValue(toCheck);
            res = eString.startsWith("file:///");
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("$(user) does not exist");
            tRes.tested("getSubstituteVariableValue()",false);
        }
        try {
            log.println("try to get a invalid variable...");
            String toCheck = "$(ThisVariableShouldNoExist)";
            String eString = oObj.getSubstituteVariableValue(toCheck);
            log.println("$(ThisVariableShouldNoExist) should not exist");
            tRes.tested("getSubstituteVariableValue()",false);

        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("expected exception was thrown.");
            res &= true;
        }

        tRes.tested("getSubstituteVariableValue()",res);
    }

    public void _substituteVariables() {
        boolean res = true;
        try {
            log.println("try to get a valid variable...");
            String toCheck = "$(user)";
            String eString = oObj.substituteVariables(toCheck, false);
            log.println(eString);
            res = eString.startsWith("file:///");
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("$(user) does not exist");
            tRes.tested("substituteVariables()",false);
        }
        try {
            log.println("try to get a invalid variable...");
            String toCheck = "$(ThisVariableShouldNoExist)";
            String eString = oObj.substituteVariables(toCheck,true);
            log.println("$(ThisVariableShouldNoExist) should not exist");
            tRes.tested("substituteVariables()",false);

        } catch (com.sun.star.container.NoSuchElementException e) {
            log.println("expected exception was thrown.");
            res &= true;
        }

        tRes.tested("substituteVariables()",res);
    }

    public void _reSubstituteVariables() {
        boolean res = true;
        log.println("try to get a valid variable...");
        String toCheck = "file:///";
        String eString = oObj.reSubstituteVariables(toCheck);
        log.println(eString);
        res = eString.startsWith("file:///");

        tRes.tested("reSubstituteVariables()",res);
    }

}
