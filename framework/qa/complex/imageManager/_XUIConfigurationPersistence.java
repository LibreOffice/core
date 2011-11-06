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



package complex.imageManager;

import com.sun.star.embed.XStorage;
import com.sun.star.ui.XUIConfigurationPersistence;
import lib.TestParameters;



public class _XUIConfigurationPersistence {


    TestParameters tEnv = null;
    public XUIConfigurationPersistence oObj;
    private XStorage xStore = null;

    public _XUIConfigurationPersistence(TestParameters tEnv, XUIConfigurationPersistence oObj) {

        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    public void before() {
        xStore = (XStorage)tEnv.get("XUIConfigurationStorage.Storage");
    }

    public boolean _reload() {
        try {
            oObj.reload();
        }
        catch(com.sun.star.uno.Exception e) {

        }
        return true;
    }

    public boolean _store() {
        try {
            oObj.store();
        }
        catch(com.sun.star.uno.Exception e) {

        }
        return true;
    }

    public boolean _storeToStorage() {
        boolean result = true;
        try {
            oObj.storeToStorage(xStore);
        }
        catch(com.sun.star.uno.Exception e) {
            result = false;

        }
        return result;
    }

    public boolean _isModified() {
        return !oObj.isModified();
    }

    public boolean _isReadOnly() {
        return  !oObj.isReadOnly();
    }

}
