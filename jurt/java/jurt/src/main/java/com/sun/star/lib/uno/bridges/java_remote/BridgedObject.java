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



package com.sun.star.lib.uno.bridges.java_remote;

import com.sun.star.bridge.XBridge;

/**
 * A back door to access the bridge associated with a bridged object.
 */
public final class BridgedObject {
    /**
     * Obtains the bridge associated with a bridged object.
     *
     * @param object a reference to a (Java representation of a) UNO object;
     *     must not be null
     * @return the bridge associated with the given object, if it is indeed
     *     bridged; otherwise, null is returned
     */
    public static XBridge getBridge(Object obj) {
        return ProxyFactory.getBridge(obj);
    }

    private BridgedObject() {} // do not instantiate
}
