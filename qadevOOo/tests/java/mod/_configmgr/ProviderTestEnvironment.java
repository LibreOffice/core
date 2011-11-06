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



package mod._configmgr;

import com.sun.star.beans.NamedValue;
import com.sun.star.uno.XInterface;
import lib.TestEnvironment;

final class ProviderTestEnvironment {
    private ProviderTestEnvironment() {}

    public static TestEnvironment create(Object provider) {
        TestEnvironment env = new TestEnvironment((XInterface) provider);
        env.addObjRelation(
            "XMSF.serviceNamesWithArgs",
            new String[] {
                "com.sun.star.configuration.ConfigurationAccess",
                "com.sun.star.configuration.ConfigurationUpdateAccess" });
        Object[] args = new Object[] {
            new NamedValue("nodepath", "/org.openoffice.Setup") };
        env.addObjRelation("XMSF.Args", new Object[][] { args, args });
        env.addObjRelation(
            "needArgs", "com.sun.star.configuration.ConfigurationProvider");
        return env;
    }
}
