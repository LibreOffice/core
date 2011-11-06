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


package ifc.container;

import com.sun.star.container.XHierarchicalName;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;


public class _XHierarchicalName extends MultiMethodTest {
    public XHierarchicalName oObj;

    public void _composeHierarchicalName() {
        String expName = (String) tEnv.getObjRelation("HierachicalName");
        String element = (String) tEnv.getObjRelation("ElementName");
        boolean res = false;

        try {
            String hn = oObj.composeHierarchicalName(element);
            res = hn.startsWith(expName);

            if (!res) {
                log.println("Getting : " + hn +
                            " but expected it to start with " + expName);
            }
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            log.println("Exception " + e.getMessage());
        } catch (com.sun.star.lang.NoSupportException e) {
            log.println("Exception " + e.getMessage());
        }

        tRes.tested("composeHierarchicalName()", res);
    }

    public void _getHierarchicalName() {
        String hName = oObj.getHierarchicalName();
        String expName = (String) tEnv.getObjRelation("HierachicalName");
        boolean res = true;
System.out.println("### "+hName);
        if (expName != null) {
            res = hName.startsWith(expName);

            if (!res) {
                log.println("Expected the name to start with " + expName);
                log.println("got " + hName);
            }
        } else {
            throw new StatusException(Status.failed(
                                              "ObjectRelation 'HierachicalName' missing"));
        }

        tRes.tested("getHierarchicalName()", res);
    }
}