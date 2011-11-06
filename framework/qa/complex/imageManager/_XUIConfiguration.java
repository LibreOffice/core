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


import com.sun.star.ui.XUIConfiguration;
import com.sun.star.ui.XUIConfigurationListener;
import lib.TestParameters;



public class _XUIConfiguration {


    TestParameters tEnv = null;
    public XUIConfiguration oObj;
    XUIConfigurationListenerImpl xListener = null;

    public static interface XUIConfigurationListenerImpl
                                    extends XUIConfigurationListener {
        public void reset();
        public void fireEvent();
        public boolean actionWasTriggered();
    }


    public _XUIConfiguration(TestParameters tEnv, XUIConfiguration oObj) {
        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    public void before() {
        xListener = (XUIConfigurationListenerImpl)tEnv.get(
                        "XUIConfiguration.XUIConfigurationListenerImpl");
        XUIConfigurationListener l;
    }

    public boolean _addConfigurationListener() {
        oObj.addConfigurationListener(xListener);
        xListener.fireEvent();
        return xListener.actionWasTriggered();
    }

    public boolean _removeConfigurationListener() {
        oObj.removeConfigurationListener(xListener);
        xListener.reset();
        xListener.fireEvent();
        return !xListener.actionWasTriggered();
    }

    /**
     * Dispose because the UIConfigurationManager has to be recreated
     */
    public void after() {
//        disposeEnvironment();
    }
}
