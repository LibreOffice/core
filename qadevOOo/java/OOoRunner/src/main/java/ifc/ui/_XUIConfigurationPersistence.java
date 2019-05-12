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



package ifc.ui;

import com.sun.star.embed.XStorage;
import com.sun.star.ui.XUIConfigurationPersistence;
import lib.MultiMethodTest;

public class _XUIConfigurationPersistence extends MultiMethodTest {

    public XUIConfigurationPersistence oObj;
    private XStorage xStore = null;

    public void before() {
        xStore = (XStorage)tEnv.getObjRelation("XUIConfigurationStorage.Storage");
    }

    public void _reload() {
        try {
            oObj.reload();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
        }
        tRes.tested("reload()", true);
    }

    public void _store() {
        try {
            oObj.store();
        }
        catch(com.sun.star.uno.Exception e) {
            e.printStackTrace(log);
        }
        tRes.tested("store()", true);
    }

    public void _storeToStorage() {
        boolean result = true;
        try {
            oObj.storeToStorage(xStore);
        }
        catch(com.sun.star.uno.Exception e) {
            result = false;
            e.printStackTrace(log);
        }
        tRes.tested("storeToStorage()", result);
    }

    public void _isModified() {
        tRes.tested("isModified()", !oObj.isModified());
    }

    public void _isReadOnly() {
        tRes.tested("isReadOnly()", !oObj.isReadOnly());
    }

}
