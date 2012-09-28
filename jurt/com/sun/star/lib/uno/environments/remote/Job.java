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


import java.io.PrintWriter;
import java.io.StringWriter;


import java.lang.reflect.InvocationTargetException;

import com.sun.star.lib.uno.typedesc.MethodDescription;
import com.sun.star.uno.Any;
import com.sun.star.uno.IMethodDescription;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XCurrentContext;

/**
 * The Job is an abstraction for tasks which have to be done
 * remotely because of a method invocation.
 * <p>
 * @author      Kay Ramme
 * @see         com.sun.star.lib.uno.environments.remote.ThreadId
 * @see         com.sun.star.lib.uno.environments.remote.IReceiver
 * @since       UDK1.0
 */
public class Job {
    protected Job _next;

    protected IReceiver _iReceiver;
    protected Message  _iMessage;
              Object    _disposeId;

    protected Object    _object;

    public Job(Object object, IReceiver iReceiver, Message iMessage) {
        _object       = object;
        _iReceiver    = iReceiver;
        _iMessage     = iMessage;
    }

    /**
     * Dispatches a <code>queryInterface</code> call
     * <p>
     * @return  the result of the call (should be an <code>Any</code>)
     */
    protected Object dispatch_queryInterface(Type type) {
        Class<?> zInterface = type.getTypeDescription().getZClass();

        Object result = null;

        Object face = UnoRuntime.queryInterface(zInterface, _object);
        // the hell knows why, but empty interfaces a given back as void anys
        if(face != null)
            result = new Any(type, face);
        return result;
    }

    /**
     * Execute the job.
     *
     * @return the result of the message.
     */
    public Object execute() throws Throwable {
        Object msgResult = _iMessage.getResult();
        if (_iMessage.isRequest()) {
            Object result = null;
            Throwable exception = null;
            IMethodDescription md = _iMessage.getMethod();
            Object[] args = _iMessage.getArguments();
            XCurrentContext oldCC = UnoRuntime.getCurrentContext();
            UnoRuntime.setCurrentContext(_iMessage.getCurrentContext());
            try {
                result = md.getIndex() == MethodDescription.ID_QUERY_INTERFACE
                    ? dispatch_queryInterface((Type) args[0])
                    : md.getMethod().invoke(_object, args);
            } catch (InvocationTargetException e) {
                exception = e.getTargetException();
                if (exception == null) {
                    exception = e;
                }
            } catch (Exception e) {
                exception = e;
            } finally {
                UnoRuntime.setCurrentContext(oldCC);
            }
            if (_iMessage.isSynchronous()) {
                if (exception == null) {
                    _iReceiver.sendReply(
                        false, _iMessage.getThreadId(), result);
                } else {
                    // Here we have to be aware of non-UNO exceptions, because
                    // they may kill a remote side which does not know anything
                    // about their types:
                    if (!(exception instanceof com.sun.star.uno.Exception)
                        && !(exception instanceof
                             com.sun.star.uno.RuntimeException))
                    {
                        StringWriter writer = new StringWriter();
                        exception.printStackTrace(new PrintWriter(writer));
                        exception = new com.sun.star.uno.RuntimeException(
                            "Java exception: <" + writer + ">", null);
                    }
                    _iReceiver.sendReply(
                        true, _iMessage.getThreadId(), exception);
                }
            }
            return null;
        } else if (_iMessage.isAbnormalTermination()) {
            throw remoteUnoRequestRaisedException(_iMessage.getResult());
        } else {
            return _iMessage.getResult();
        }
    }

    public ThreadId getThreadId() {
        return _iMessage.getThreadId();
    }

    public boolean isRequest() {
        return _iMessage.isRequest();
    }

    public boolean isSynchronous() {
        return _iMessage.isSynchronous();
    }

    public void dispose() {
//          _oId        = null;
//          _iReceiver  = null;
//          _threadId   = null;
//          _object     = null;
//          _operation  = null;
//          _param      = null;
//          _exception  = null;
//          _zInterface = null;
//          _disposeId  = null;
    }

    // The name of this method is chosen to generate a somewhat self-explanatory
    // stack trace:
    private Exception remoteUnoRequestRaisedException(Object exception) {
        Exception e = (Exception) exception;
        e.fillInStackTrace();
        return e;
    }
}
