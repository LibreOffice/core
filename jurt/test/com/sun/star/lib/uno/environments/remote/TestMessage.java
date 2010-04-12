/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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


