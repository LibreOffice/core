/*************************************************************************
 *
 *  $RCSfile: Job.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kr $ $Date: 2000-11-17 10:45:40 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

package com.sun.star.lib.uno.environments.remote;


import java.io.IOException;


import java.lang.reflect.Array;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import com.sun.star.corba.giop.ReplyStatusType_1_2;

import com.sun.star.lib.uno.typeinfo.ParameterTypeInfo;

import com.sun.star.uno.Any;
import com.sun.star.uno.MappingException;
import com.sun.star.uno.Type;
import com.sun.star.uno.UnoRuntime;


/**
 * The Job is an abstraction for tasks which have to be done
 * remotely because of a method invocation.
 * <p>
 * @version     $Revision: 1.2 $ $ $Date: 2000-11-17 10:45:40 $
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

          if(DEBUG) System.err.println("##### " + getClass().getName() + ".<init>:" + _iReceiver + " " + _iMessage);
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
    protected Object dispatch_MethodCall(Object params[]) throws Exception {
          Method method = Protocol.__findMethod(_iMessage.getInterface(), _iMessage.getOperation());

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
    protected Object dispatch_queryInterface(Type type) throws Exception {
        Class zInterface = type.getDescription();

        Object result = null;

        Object face = UnoRuntime.queryInterface(zInterface, _object);
        // the hell knows why, but empty interfaces a given back as void anys
        if(face != null)
            result = new com.sun.star.uno.Any(type.getDescription(), face);

         if(DEBUG) System.err.println("##### " + getClass().getName() + ".dispatch_queryInterface:" + _object + " " + zInterface + " result:" + result);

        return result;
    }

    /**
     * Executes the job.
     * <p>
     * @return  returns true if the operation is a reply
     */
    public Object execute() throws Exception {
        Object params[][] = new Object[1][];

        Object result = _iMessage.getData(params);

        if(_iMessage.isException())
            throw (Exception)result;

        try {
            if(_iMessage.getOperation() != null) { // it is a request
                Object xresult = null;

                if(_iMessage.getOperation().equals("queryInterface"))
                    xresult = dispatch_queryInterface((Type)params[0][0]);
                else
                    xresult = dispatch_MethodCall(params[0]);

                if(_iMessage.isSynchron())
                    _iReceiver.sendReply(false, _iMessage.getThreadID(), xresult);
            }
        }
        catch(InvocationTargetException invocationTargetException) {
            Throwable theException = invocationTargetException;

            do {
                if(DEBUG) {
                    System.err.println("##### RemoteStub.request - exception occured:" + ((InvocationTargetException)theException).getTargetException());
                    ((InvocationTargetException)theException).getTargetException().printStackTrace();
                }

                theException = ((InvocationTargetException)theException).getTargetException();
            }
            while(theException instanceof InvocationTargetException);

            // is this an exception we now?
            if(theException instanceof com.sun.star.uno.Exception
            || theException instanceof com.sun.star.uno.RuntimeException) {

                if(DEBUG) System.err.println("#### RemoteStub.request - exception:" + theException);
            }
            else // wrap it as uno exception
                theException = new com.sun.star.uno.RuntimeException("java exception: " + theException.toString(), null);

            if(_iMessage.isSynchron())
                _iReceiver.sendReply(true, _iMessage.getThreadID(), theException);
        }

        return result;
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
    public ThreadID getThreadId() {
        return _iMessage.getThreadID();
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
    public Class getInterface() {
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
}


