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
package com.sun.star.comp.sdbc;

import java.util.concurrent.atomic.AtomicInteger;

import org.apache.openoffice.comp.sdbc.dbtools.comphelper.ResourceBasedEventLogger;

public class ConnectionLog extends ResourceBasedEventLogger {
    public static enum ObjectType {
        CONNECTION,
        STATEMENT,
        RESULT
    }

    private static final AtomicInteger[] uniqueIds;

    static
    {
        uniqueIds = new AtomicInteger[ObjectType.values().length];
        for (int i = 0; i < uniqueIds.length; i++) {
            uniqueIds[i] = new AtomicInteger(0);
        }
    }

    private final int objectId;

    public ConnectionLog(ResourceBasedEventLogger logger, ObjectType objectType) {
        super(logger);
        objectId = uniqueIds[objectType.ordinal()].getAndIncrement();
    }

    public int getObjectId() {
        return objectId;
    }

    @Override
    public boolean log(int logLevel, int messageResID, Object... arguments) {
        Object[] argsWithId = new Object[arguments.length + 1];
        argsWithId[0] = objectId;
        System.arraycopy(arguments, 0, argsWithId, 1, arguments.length);
        return super.log(logLevel, messageResID, argsWithId);
    }

    @Override
    public boolean logp(int logLevel, int messageResID, Object... arguments) {
        Object[] argsWithId = new Object[arguments.length + 1];
        argsWithId[0] = objectId;
        System.arraycopy(arguments, 0, argsWithId, 1, arguments.length);
        return super.logp(logLevel, messageResID, arguments);
    }
}
