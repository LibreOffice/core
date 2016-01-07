/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

package com.sun.star.lib.uno.environments.remote;

import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.ITypeDescription;
import com.sun.star.uno.XCurrentContext;

/**
 * A remote request or reply message.
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
     * Returns the thread ID of the message.
     *
     * <p>Valid for all kinds of messages.</p>
     *
     * @return the (non-<code>null</code>) thread ID.
     */
    public final ThreadId getThreadId() {
        return threadId;
    }

    /**
     * Returns whether the message is a request or a reply.
     *
     * <p>Valid for all kinds of messages.</p>
     *
     * @return <code>true</code> for a request, <code>false</code> for a reply.
     */
    public final boolean isRequest() {
        return request;
    }

    /**
     * Returns the object ID of a request message.
     *
     * <p>Valid only for request messages.</p>
     *
     * @return the (non-<code>null</code>) object ID for a request,
     * <code>null</code> for a reply.
     */
    public final String getObjectId() {
        return objectId;
    }

    /**
     * Returns the type of a request message.
     *
     * <p>Valid only for request messages.</p>
     *
     * @return the (non-<code>null</code>) type for a request, <code>null</code>
     * for a reply.
     */
    public final ITypeDescription getType() {
        return type;
    }

    /**
     * Returns the method description of a request message.
     *
     * <p>Valid only for request messages.  The returned
     * <code>IMethodDescription</code> is consistent with the type of the
     * message.</p>
     *
     * @return the (non-<code>null</code>) method description for a request,
     * <code>null</code> for a reply.
     */
    public final IMethodDescription getMethod() {
        return method;
    }

    /**
     * Returns whether the request message is synchronous.
     *
     * <p>Valid only for request messages.</p>
     *
     * @return <code>true</code> for a synchronous request, <code>false</code>
     * for an asynchronous request or a reply.
     */
    public final boolean isSynchronous() {
        return synchronous;
    }

    /**
     * Returns the current context of a request message.
     *
     * <p>Valid only for request messages.</p>
     *
     * @return the current context (which may be <code>null</code>) for a
     * request, <code>null</code> for a reply.
     */
    public XCurrentContext getCurrentContext() {
        return currentContext;
    }

    /**
     * Returns whether the reply message represents abnormal termination.
     *
     * <p>Valid only for reply messages.</p>
     *
     * @return <code>true</code> for a reply that represents abnormal
     * termination, <code>false</code> for a reply that represents normal
     * termination or a request.
     */
    public final boolean isAbnormalTermination() {
        return abnormalTermination;
    }

    /**
     * Returns the result of a reply message.
     *
     * <p>Valid only for reply messages.</p>
     *
     * @return any (possibly <code>null</code>) return value for a reply that
     * represents normal termination, the (non-<code>null</code>) exception for
     * a reply that represents abnormal termination, <code>null</code> for a
     * request.
     */
    public final Object getResult() {
        return result;
    }

    /**
     * Returns the arguments of a message.
     *
     * <p>Valid only for request messages and reply messages that represent
     * normal termination.  Any returned array must not be modified.</p>
     *
     * @return the in and in&ndash; {
     * }out arguments for a request (possibly
     * <code>null</code> for a parameterless function), the out and in&ndash; {
     * }out
     * arguments for a reply that represents normal termination (possibly
     * <code>null</code> for a parameterless function), <code>null</code> for a
     * reply that represents abnormal termination.
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
