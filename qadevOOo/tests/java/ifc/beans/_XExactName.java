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



package ifc.beans;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.XExactName;


public class _XExactName extends MultiMethodTest {
    protected String expectedName = "";
    public XExactName oObj = null;

    public void _getExactName() {
        String nameFor = expectedName.toLowerCase();

        log.println("Getting exact name for " + nameFor);

        String getting = oObj.getExactName(nameFor);

        boolean res = true;

        if (getting == null) {
            res = false;
        } else {
            res = getting.equals(expectedName);
        }

        if (!res) {
            log.println("didn't get the expected Name:");
            log.println("getting: " + getting);
            log.println("Expected: " + expectedName);
        }

        nameFor = expectedName.toUpperCase();

        log.println("Getting exact name for " + nameFor);
        getting = oObj.getExactName(nameFor);

        if (getting == null) {
            res = false;
        } else {
            res &= getting.equals(expectedName);
        }

        if (!getting.equals(expectedName)) {
            log.println("didn't get the expected Name:");
            log.println("getting: " + getting);
            log.println("Expected: " + expectedName);
        }

        tRes.tested("getExactName()", res);
    }

    /**
     * Checking if the Object relation expectedName is given
     * throw a StatusException if not.
     */
    protected void before() {
        expectedName = (String) tEnv.getObjRelation("expectedName");

        if (expectedName == null) {
            throw new StatusException(Status.failed(
                                              "Object relation expectedName is missing"));
        }
    }
}