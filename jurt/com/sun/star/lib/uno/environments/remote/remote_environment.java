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



package com.sun.star.lib.uno.environments.remote;

import com.sun.star.uno.IEnvironment;
import com.sun.star.uno.Type;

public final class remote_environment implements IEnvironment {
    public remote_environment(Object context) {
        this.context = context;
    }

    public Object getContext() {
        return context;
    }

    public String getName() {
        return "remote";
    }

    public Object registerInterface(Object object, String[] oid, Type type) {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    public void revokeInterface(String oid, Type type) {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    public Object getRegisteredInterface(String oid, Type type) {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    public String getRegisteredObjectIdentifier(Object object) {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    public void list() {
        throw new UnsupportedOperationException(
            "java_remote environment is not functional");
    }

    private final Object context;
}
