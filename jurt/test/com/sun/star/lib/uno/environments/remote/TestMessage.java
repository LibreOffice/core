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


import com.sun.star.uno.Type;

import com.sun.star.uno.ITypeDescription;


class TestMessage implements IMessage {
    boolean  _synchron;
    ITypeDescription     _iTypeDescription;
    String   _oid;
    ThreadId _threadId;
    Object   _result;
    String   _operation;
    Object   _params[];

    TestMessage(boolean synchron, ITypeDescription iTypeDescription, String oid, ThreadId threadId, Object result, String operation, Object params[]) {
        _synchron  = synchron;
        _iTypeDescription      = iTypeDescription;
        _oid       = oid;
        _threadId  = threadId;
        _result    = result;
        _operation = operation;
        _params    = params;
    }

    public String getOperation() {
        return _operation;
    }

    public ThreadId getThreadId() {
        return _threadId;
    }

    public ITypeDescription getInterface() {
        return _iTypeDescription;
    }

    public boolean isSynchron() {
        return _synchron;
    }

    public boolean mustReply() {
        return _synchron;
    }

    public boolean isException() {
        return false;
    }

    public String getOid() {
        return _oid;
    }

    public Object getData(Object params[][]) {
        params[0] = _params;
        return _result;
//              return new Integer(_requestId);
    }
}


