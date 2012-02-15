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



package mod._fwk;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.XInterface;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;

/**
 */
public class MenuBarFactory extends TestCase {
    XInterface oObj = null;

    protected TestEnvironment createTestEnvironment(TestParameters tParam, PrintWriter log) {
        TestEnvironment tEnv = null;
        try {
            XMultiServiceFactory xMSF = (XMultiServiceFactory)tParam.getMSF();
            oObj = (XInterface)xMSF.createInstance("com.sun.star.comp.framework.MenuBarFactory");

            tEnv = new TestEnvironment(oObj);
        }
        catch(com.sun.star.uno.Exception e) {
            throw new StatusException("Cannot create test object 'MenuBarFactory'", e);
        }
        return tEnv;
    }
}


