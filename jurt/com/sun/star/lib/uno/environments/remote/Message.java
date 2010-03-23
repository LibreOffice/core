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

import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.XCurrentContext;

/**
   A remote request or reply message.
*/
public class Message {
    public Message(
        ThreadId threadId, boolean request, String objectId,
        ITypeDescription type, IMethodDescription method, boolean synchronous,
        XCurrentContext currentContext, boolean abnormalTermination,
        Object result, Object[] arguments)
    {
        this.threadId = threadId;
        this.request = request;
        this.objectId = objectId;
        this.type = type;
        this.method = method;
        this.synchronous = synchronous;
        this.currentContext = currentContext;
        this.abnormalTermination = abnormalTermination;
        this.result = result;
        this.arguments = arguments;
    }

    /**
       Returns the thread ID of the message.

       <p>Valid for all kinds of messages.</p>

       @return the (non-<code>null</code>) thread ID
    */
    public final ThreadId getThreadId() {
        return threadId;
    }

    /**
       Returns whether the message is a request or a reply.

       <p>Valid for all kinds of messages.</p>

       @return <code>true</code> for a request, <code>false</code> for a reply
    */
    public final boolean isRequest() {
        return request;
    }

    /**
       Returns the object ID of a request message.

       <p>Valid only for request messages.</p>

       @return the (non-<code>null</code>) object ID for a request,
       <code>null</code> for a reply
    */
    public final String getObjectId() {
        return objectId;
    }

    /**
       Returns the type of a request message.

       <p>Valid only for request messages.</p>

       @return the (non-<code>null</code>) type for a request, <code>null</code>
       for a reply
    */
    public final ITypeDescription getType() {
        return type;
    }

    /**
       Returns the method description of a request message.

       <p>Valid only for request messages.  The returned
       <code>IMethodDescription</code> is consistent with the type of the
       message.</p>

       @return the (non-<code>null</code>) method description for a request,
       <code>null</code> for a reply
    */
    public final IMethodDescription getMethod() {
        return method;
    }

    /**
       Returns whether the request message is synchronous.

       <p>Valid only for request messages.</p>

       @return <code>true</code> for a synchronous request, <code>false</code>
       for an asynchronous request or a reply
    */
    public final boolean isSynchronous() {
        return synchronous;
    }

    /**
       Returns the current context of a request message.

       <p>Valid only for request messages.</p>

       @return the current context (which may be <code>null</code>) for a
       request, <code>null</code> for a reply
    */
    public XCurrentContext getCurrentContext() {
        return currentContext;
    }

    /**
       Returns whether the reply message represents abnormal termination.

       <p>Valid only for reply messages.</p>

       @return <code>true</code> for a reply that represents abnormal
       termination, <code>false</code> for a reply that represents normal
       termination or a request
    */
    public final boolean isAbnormalTermination() {
        return abnormalTermination;
    }

    /**
       Returns the result of a reply message.

       <p>Valid only for reply messages.</p>

       @return any (possibly <code>null</code>) return value for a reply that
       represents normal termination, the (non-<code>null</code>) exception for
       a reply that represents abnormal termination, <code>null</code> for a
       request
    */
    public final Object getResult() {
        return result;
    }

    /**
       Returns the arguments of a message.

       <p>Valid only for request messages and reply messages that represent
       normal termination.  Any returned array must not be modified.</p>

       @return the in and in&ndash {
    }out arguments for a request (possibly
       <code>null</code> for a paramterless function), the out and in&dash {
    }out
       arguments for a reply that represents normal termination (possibly
       <code>null</code> for a parameterless function), <code>null</code> for a
       reply that represents abnormal termination
    */
    public final Object[] getArguments() {
        return arguments;
    }

    private final ThreadId threadId;
    private final boolean request;
    private final String objectId;
    private final ITypeDescription type;
    private final IMethodDescription method;
    private final boolean synchronous;
    private final XCurrentContext currentContext;
    private final boolean abnormalTermination;
    private final Object result;
    private final Object[] arguments;
}
