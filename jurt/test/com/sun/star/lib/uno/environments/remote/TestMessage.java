/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: TestMessage.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:12:54 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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


