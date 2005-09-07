/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Job.java,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:00:55 $
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

import com.sun.star.uno.ITypeDescription;

import com.sun.star.uno.Any;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;


/**
 * The Job is an abstraction for tasks which have to be done
 * remotely because of a method invocation.
 * <p>
 * @version     $Revision: 1.15 $ $ $Date: 2005-09-07 19:00:55 $
 * @author      Kay Ramme
 * @see         com.sun.star.lib.uno.environments.remote.ThreadID
 * @see         com.sun.star.lib.uno.environments.remote.IReceiver
 * @since       UDK1.0
 */
public class Job {
    public static final boolean DEBUG = false;

    protected Job _next;

    protected IReceiver _iReceiver;
    protected IMessage  _iMessage;
              Object    _disposeId;

    protected Object    _object;

    public Job(Object object, IReceiver iReceiver, IMessage iMessage) {
        _object       = object;
        _iReceiver    = iReceiver;
        _iMessage     = iMessage;

          if(DEBUG) System.err.println("##### " + getClass().getName() + ".<init>:" + _iReceiver + " " + _iMessage + " " + iMessage.getOperation() + " " + iMessage.getData(new Object[1][]));
    }

    /**
     * Dispatches a regular method call
     * <p>
     * @return  the result of the call
     * @param message       the parameter for the call
     * @param resultClass   the result type as an out parameter
     * @param status        the status as an out parameter
     * @param o_outs        the out parameters of the call as out parameters
     * @param o_out_sig     the out signature as an out parameter
     */
    protected Object dispatch_MethodCall(Object params[]) throws InvocationTargetException, IllegalAccessException {
          Method method = _iMessage.getInterface().getMethodDescription(_iMessage.getOperation()).getMethod();

          if(DEBUG) System.err.println("##### " + getClass().getName() + ".dispatch_MethodCall:" + _object + " " + method.getName() + " " + params);

        return method.invoke(_object, params);
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

         if(DEBUG) System.err.println("##### " + getClass().getName() + ".dispatch_queryInterface:" + _object + " " + zInterface + " result:" + result);

        return result;
    }

    /**
     * Execute the job.
     *
     * @return the result of the message.
     */
    public Object execute() throws Throwable {
        Object[][] msgParams = new Object[1][];
        Object msgResult = _iMessage.getData(msgParams);

        if (DEBUG) {
            System.err.println("##### " + getClass().getName() + ".execute: "
                               + msgResult + " " + _iMessage.isException());
        }

        if (_iMessage.isException()) {
            throw remoteUnoRequestRaisedException(msgResult);
        }

        String operation = _iMessage.getOperation();
        if (operation != null) { // if it is a request
            Object result = null;
            Throwable exception = null;

            try {
                result = operation.equals("queryInterface")
                    ? dispatch_queryInterface((Type) msgParams[0][0])
                    : dispatch_MethodCall(msgParams[0]);
            } catch (InvocationTargetException e) {
                if (DEBUG) {
                    e.printStackTrace(System.err);
                }
                exception = e.getTargetException();
                if (exception == null) {
                    exception = e;
                }
            } catch (Exception e) {
                if (DEBUG) {
                    e.printStackTrace(System.err);
                }
                exception = e;
            }

            if (_iMessage.mustReply()) {
                if (exception == null) {
                    _iReceiver.sendReply(false, _iMessage.getThreadId(),
                                         result);
                } else {
                    // Here we have to be aware of non-UNO exceptions, because
                    // they may kill a remote side which does not know anything
                    // about their types:
                    if (exception != null
                        && !(exception instanceof com.sun.star.uno.Exception)
                        && !(exception instanceof
                             com.sun.star.uno.RuntimeException)) {
                        StringWriter writer = new StringWriter();
                        exception.printStackTrace(new PrintWriter(writer));
                        exception = new com.sun.star.uno.RuntimeException(
                            "Java exception: <" + writer + ">", null);
                    }

                    _iReceiver.sendReply(true, _iMessage.getThreadId(),
                                         exception);
                }
            }
        }

        return msgResult;
    }

    /**
     * Indicates whether the job is synchron or asynchron.
     * <p>
     * @return  returns <code>true</code> if the operation is synchron
     */
    public boolean isSynchron() {
        return _iMessage.isSynchron(); //_synchron || (_operation == null);
    }

    /**
     * Indicates whether the job is a reply but not an exception
     * <p>
     * @return  returns <code>true</code> is this job is final
     */
    public boolean isFinal() {
        return _iMessage.getOperation() == null; // && !_iMessage.isException();
    }

    /**
     * Gives the thread id of the job
     * <p>
     * @return  returns the thread id
     */
    public ThreadId getThreadId() {
        return _iMessage.getThreadId();
    }

    /**
     * Gives the object id of the job
     * <p>
     * @return  returns the object id
     */
    public String getOID() {
          return null; //_oId;
    }

    /**
     * Gives the operation of the job
     * <p>
     * @return  returns the operation
     */
    public String getOperation() {
        return _iMessage.getOperation();
    }

    /**
     * Gives the interface of the object to call on
     * <p>
     * @return  returns the interface
     */
    public ITypeDescription getInterface() {
        return _iMessage.getInterface();
    }

    /**
     * Gives a descriptive <code>String</code> of the job
     * <p>
     * @return  returns the description
     */
    public String toString() {
        return "job: "; //+ _operation + " " + _requestId;
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
