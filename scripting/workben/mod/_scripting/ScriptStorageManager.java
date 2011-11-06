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



package mod._scripting;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.beans.XPropertySet;
import com.sun.star.uno.XInterface;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import java.io.PrintWriter;
import lib.StatusException;
import lib.TestCase;
import lib.TestEnvironment;
import lib.TestParameters;
import util.utils;

public class ScriptStorageManager extends TestCase {

    public void initialize( TestParameters tParam, PrintWriter log ) {
    }

    public synchronized TestEnvironment createTestEnvironment(
        TestParameters Param, PrintWriter log ) throws StatusException {

        XInterface oObj = null;
    Object oInterface = null;

    try {
            XMultiServiceFactory xMSF = Param.getMSF();
            XPropertySet xProp = (XPropertySet)UnoRuntime.queryInterface(
                                                XPropertySet.class, xMSF);
            // get context
            XComponentContext xContext = (XComponentContext)
                            UnoRuntime.queryInterface(XComponentContext.class,
                            xProp.getPropertyValue("DefaultContext"));
            // get the script storage manager from context
            oInterface = xContext.getValueByName("/singletons/" +
                        "drafts.com.sun.star.script.framework.storage.theScriptStorageManager");
        }
        catch( Exception e ) {
            log.println("ScriptStorageManager singleton service not available" );
        e.printStackTrace();
        }
        oObj = (XInterface) oInterface;

        TestEnvironment tEnv = new TestEnvironment(oObj) ;
        TestDataLoader.setupData(tEnv, "ScriptStorageManager");

        return tEnv ;
    }

    public synchronized void disposeTestEnvironment( TestEnvironment tEnv,
            TestParameters tParam) {
    }
}


