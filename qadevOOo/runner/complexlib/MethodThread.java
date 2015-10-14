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

package complexlib;

import java.io.PrintWriter;
import java.lang.reflect.Method;

/**
 * Invoke a method of a class in an own thread. Provide a method to end
 * the thread.
 */
public class MethodThread extends Thread
{

    /** The method that should be executed **/
    private final Method mTestMethod;
    /** The object that implements the method **/
    private final Object mInvokeClass;
    /** A PrintWriter for debug Output **/
    private final PrintWriter mLog;
    /** An Error String **/
    private String mErrMessage = null;
    /** Did an Exception happen? **/
    private boolean mExceptionHappened = false;
    private Object[] mParameter = null;

    public MethodThread(Method testMethod, Object invokeClass, Object[] parameter, PrintWriter log)
    {
        mTestMethod = testMethod;
        mInvokeClass = invokeClass;
        mParameter = parameter;
        mLog = log;
    }

    /**
     * Invoke the method.
     */
    @Override
    public void run()
    {
        try
        {
            mTestMethod.invoke(mInvokeClass, mParameter);
        }
        catch (IllegalAccessException e)
        {
            e.printStackTrace(mLog);
            mErrMessage = e.getMessage();
            mExceptionHappened = true;
        }
        catch (java.lang.reflect.InvocationTargetException e)
        {
            Throwable t = e.getTargetException();
            if (!(t instanceof ComplexTestCase.AssureException))
            {
                t.printStackTrace(mLog);
                mErrMessage = t.getMessage();
                if (mErrMessage == null)
                {
                    mErrMessage = t.toString();
                }
                mExceptionHappened = true;
            }

        }
    }

    /**
     * Get the error message
     * @return The error message.
     */
    public String getErrorMessage()
    {
        return mErrMessage;
    }

    /**
     * Is there an error message?
     * @return True, if an error did happen.
     */
    public boolean hasErrorMessage()
    {
        return mExceptionHappened;
    }

    /**
     * Stop the running method.
     */
    public void stopRunning()
    {
        try
        {
            interrupt();
        }
        catch (SecurityException e)
        {
            e.printStackTrace(mLog);
            mErrMessage = e.getMessage();
            mExceptionHappened = true;
        }
    }
}
