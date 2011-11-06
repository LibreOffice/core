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

import com.sun.star.ui.ConfigurationEvent;
import com.sun.star.ui.XUIConfigurationListener;
import lib.MultiMethodTest;

public class _XUIConfigurationListener extends MultiMethodTest {

    public XUIConfigurationListener oObj = null;

    public void _elementInserted() {
        ConfigurationEvent event = new ConfigurationEvent();
        oObj.elementInserted(event);
        tRes.tested("elementInserted()", true);
    }
    public void _elementRemoved() {
        ConfigurationEvent event = new ConfigurationEvent();
        oObj.elementRemoved(event);
        tRes.tested("elementRemoved()", true);
    }
    public void _elementReplaced() {
        ConfigurationEvent event = new ConfigurationEvent();
        oObj.elementReplaced(event);
        tRes.tested("elementReplaced()", true);
    }
}
