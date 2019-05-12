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


package ifc.uno;

import lib.MultiMethodTest;
import util.ValueComparer;

import com.sun.star.lang.XMultiComponentFactory;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;


public class _XComponentContext extends MultiMethodTest {
    public XComponentContext oObj;
    protected XMultiComponentFactory byValue = null;
    protected XMultiComponentFactory directly = null;

    public void _getServiceManager() {
        log.println("getting manager");

        directly = oObj.getServiceManager();
        String[] names = directly.getAvailableServiceNames();
        boolean res = true;

        for (int i = 0; i < names.length; i++) {
            try {
                if (names[i].equals("com.sun.star.i18n.ConversionDictionary_ko")) continue;
                if (names[i].equals("com.sun.star.i18n.TextConversion_ko")) continue;
                log.println("try to instanciate found servicename " +
                            names[i]);
                directly.createInstanceWithContext(names[i], oObj);
                log.println("worked .... ok");
                res &= true;
            } catch (com.sun.star.uno.Exception e) {
                log.println("Exception occurred " + e.getMessage());
                res &= false;
            }
        }

        tRes.tested("getServiceManager()", res);
    }

    public void _getValueByName() {
        requiredMethod("getServiceManager()");

        Object value = oObj.getValueByName(
                               "/singletons/com.sun.star.lang.theServiceManager");
        byValue = (XMultiComponentFactory) UnoRuntime.queryInterface(
                          XMultiComponentFactory.class, value);

        String[] vNames = byValue.getAvailableServiceNames();
        String[] dNames = directly.getAvailableServiceNames();

        boolean res = ValueComparer.equalValue(byValue, directly);
        tRes.tested("getValueByName()", res);
    }
}