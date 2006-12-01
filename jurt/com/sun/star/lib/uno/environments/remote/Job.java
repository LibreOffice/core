/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Job.java,v $
 *
 *  $Revision: 1.16 $
 *
 *  last change: $Author: rt $ $Date: 2006-12-01 14:51:12 $
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


import java.io.IOException;
import java.io.PrintWriter;
import java.io.StringWriter;


import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

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
 * @version     $Revision: 1.16 $ $ $Date: 2006-12-01 14:51:12 $
 * @author      Kay Ramme
 * @see         com.sun.star.lib.uno.environments.remote.ThreadID
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
     * @param message       the parameter for the call
     * @param resultClass   the result type as an out parameter
     * @param status        the status as an out parameter
     * @param o_outs        the out parameters of the call as out parameters
     * @param o_out_sig     the out signature as an out parameter
     */
    protected Object dispatch_queryInterface(Type type) {
        Class zInterface = type.getTypeDescription().getZClass();

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
                    if (exception != null
                        && !(exception instanceof com.sun.star.uno.Exception)
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
