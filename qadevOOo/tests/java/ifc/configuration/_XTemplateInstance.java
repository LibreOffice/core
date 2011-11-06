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



package ifc.configuration;

import com.sun.star.configuration.XTemplateInstance;
import lib.MultiMethodTest;


public class _XTemplateInstance extends MultiMethodTest {

    public XTemplateInstance oObj;


    public void _getTemplateName() {
        String expected = (String) tEnv.getObjRelation("TemplateInstance");
        String getting = oObj.getTemplateName();
        boolean res = getting.equals(expected);
        if (!res) {
            log.println("Expected: "+expected);
            log.println("Getting: "+getting);
        }
        tRes.tested("getTemplateName()",res);
    }

}
