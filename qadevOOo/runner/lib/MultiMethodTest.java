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
package lib;

import java.io.PrintWriter;
import java.lang.reflect.Field;
import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;
import java.util.ArrayList;

import share.DescEntry;
import stats.Summarizer;

import com.sun.star.uno.UnoRuntime;

/**
 * The class supports method based interface tests development.
 *
 * <p>There are some points that should be fulfilled in a subclass to work
 * correctly in the multi-method framework:
 *
 *   1. each subclass schould define a public field named oObj of type tested
 *   by the subclass, e.g. 'public XText oObj;'. That field will be initialized
 *   by the MultiMethodTest code with the instance of the interface to test.
 *   In a case of service testing the field type should be XPropertySet.
 *
 *   2. for the test of each method of the tested interface(or a property in the
 *   case of service testing) should be method with the following signature
 *   provided: 'public void _<method name>()', e.g. 'public void _getText()'.
 *   The methods will be called by MultiMethodText code using reflection API
 *   for each method in the interface description.
 *
 *   3. to set status for a call 'tRes.tested(String method,
 *   boolean result)' should be used. For example 'tRes.tested("getText()",
 *   true)'. Also 'tRes.assert(String assertion, boolean result)' call can
 *   be used. Note, that one can call the methods not neccesarily from the
 *   test for the tested method, but from other method tests too (in the
 *   MultiMethodTest subclass). See also TestResult and MultiMethodTest.tRes
 *   documentation.
 *
 *   4. the before() and after() methods can be overriden to perform some
 *   actions, accordingly, before and after calling the test methods.
 *
 *   5. besides tRes, there are some fields initialized in the MultiMethodTest,
 *   that can be used for implementing tests:
 *
 *     - tEnv contains the environment tested
 *     - tParam contains parameters of the test
 *     - log a writer to log information about the test
 *
 * @see TestResult
 */
public class MultiMethodTest
{

    /**
     * Contains the TestEnvironment being tested, to allow for tests to access
     * it.
     */
    protected TestEnvironment tEnv;
    /**
     * Contains the TestParameters for the tests, to allow for tests to access
     * it.
     */
    protected TestParameters tParam;
    /**
     * Contains the Description for the test
     * it.
     */
    protected DescEntry entry;
    /**
     * Contains a writer to log an information about the interface testing, to
     * allows for tests to access it.
     */
    protected PrintWriter log;
    /**
     * Contains the TestResult instance for the interface test to collect
     * information about methods test.
     */
    protected TestResult tRes;
    /**
     * Contains names of the methods have been already called
     */
    private ArrayList<String> methCalled = new ArrayList<String>(10);

    /**
     * Disposes the test environment, which was corrupted by the test.
     *
     * @param tEnv the environment to dispose
     */
    public void disposeEnvironment(TestEnvironment tEnv)
    {
        disposeEnvironment();
    }

    /**
     * Disposes the current test environment, which was corrupted by the test.
     *
     * @see #disposeEnvironment(TestEnvironment)
     */
    public void disposeEnvironment()
    {
        tEnv.dispose();
        TestCase tCase = tEnv.getTestCase();
        tCase.disposeTestEnvironment(tEnv, tParam);
    }

    /**
     * Runs the interface test: its method tests. First, it initializes some
     * of MultiMethodTest fields, like tRes, log, tEnv, etc. Then, it queries
     * the tested interface and initializes 'oObj' field (defined in a
     * subclass). Before calling method tests, before() method calles to allow
     * initialization of s stuff before testing. Then, the method tests are
     * called. After them, after() method is called, to allow cleaning up the
     * stuff initialized in before() and test methods.
     *
     * @param entry the interface test state
     * @param tEnv the environment to test
     * @param tParam the parameters of the test
     *
     * @see #before
     * @see #after
     */
    public TestResult run(DescEntry entry, TestEnvironment tEnv, TestParameters tParam)
    {

        log = (PrintWriter) entry.Logger;

        this.tEnv = tEnv;
        this.tParam = tParam;
        // this.log = log;
        this.entry = entry;
        this.tRes = new TestResult();
        Class<?> testedClass;

        // Some fake code for a self test.
        // For normal test we must not be a "ifc.qadevooo._SelfTest"
        if (! ("ifc.qadevooo._SelfTest").equals(entry.entryName))
        {
            getInterfaceName();
            // System.out.println("checking : " + ifcName);
            System.out.print("checking: [" + entry.longName + "]");

            // defining a name of the class corresponding to the tested interface
            // or service
            String testedClassName;

            testedClassName = getTestedClassName();

            if (entry.EntryType.equals("service"))
            {
                testedClassName = "com.sun.star.beans.XPropertySet";
            }

            try
            {
                testedClass = Class.forName(testedClassName);
            }
            catch (ClassNotFoundException cnfE)
            {
                System.out.println();
                cnfE.printStackTrace(log);
                log.println("could not find a class : " + getTestedClassName());
                return null;
            }
            System.out.println(" is iface: [" + testedClassName + "] testcode: [" + entry.entryName + "]");

            tEnv.getTestObject();
            Object oObj = UnoRuntime.queryInterface(testedClass, tEnv.getTestObject());

            if (oObj == null)
            {
                if (entry.isOptional)
                {
                    Summarizer.summarizeDown(entry, "Not supported but optional.OK");
                }
                else
                {
                    Summarizer.summarizeDown(entry, "queryInterface returned null.FAILED");
                    entry.ErrorMsg = "queryInterface returned null";
                    entry.hasErrorMsg = true;
                }

                return null;
            }

            //setting the field oObj
            setField("oObj", oObj);
        }

        // to perform some stuff before all method tests
        try
        {
            before();
        }
        catch (Exception e)
        {
            e.printStackTrace();
            setSubStates(e.toString());
            return tRes;
        }

        // executing methods tests
        for (int i = 0; i < entry.SubEntryCount; i++)
        {
            DescEntry aSubEntry = entry.SubEntries[i];
            try
            {
                final String sEntryName = aSubEntry.entryName;
                executeMethod(sEntryName);
            }
            catch (Exception e)
            {
                log.println("Exception while checking: " + aSubEntry.entryName + " : " + e.getMessage());
            }
        }

        // to perform some stuff after all method tests
        try
        {
            after();
        }
        catch (Exception e)
        {
        }

        return tRes;
    }

    /**
     * Is called before calling method tests, but after initialization.
     * Subclasses may override to perform actions before method tests.
     */
    protected void before()
    {
    }

    /**
     * Is called after calling method tests. Subclasses may override
     * to perform actions after method tests.
     */
    protected void after()
    {
    }

    /**
     * @return the name of the interface or the service tested.
     */
    protected String getTestedClassName()
    {
        String clsName = this.getClass().getName();

        int firstDot = clsName.indexOf('.');
        int lastDot = clsName.lastIndexOf('.');

        String append = "com.sun.star.";

        if (entry.longName.indexOf("::drafts::com::") > -1)
        {
            append = "drafts.com.sun.star.";
        }

        return append + clsName.substring(firstDot + 1, lastDot + 1) + clsName.substring(lastDot + 2);
    }

    /**
     * Sets a method status.
     *
     * @param methName the method name to set status
     * @param methStatus the status to set to the method
     */
    protected void setStatus(String methName, Status methStatus)
    {
        tRes.tested(methName, methStatus);
    }

    /**
     * sets the substates
     */
    protected void setSubStates(String msg)
    {
        for (int k = 0; k < entry.SubEntryCount; k++)
        {
            entry.SubEntries[k].hasErrorMsg = true;
            entry.SubEntries[k].ErrorMsg = msg;
            if (entry.SubEntries[k].State.equals("UNKNOWN"))
            {
                entry.SubEntries[k].State = msg;
            }
        }

    }

    /**
     * Checks if the <code>method</code> is optional in the service.
     */
    protected boolean isOptional(String _method)
    {
        for (int k = 0; k < entry.SubEntryCount; k++)
        {
            final String sName = entry.SubEntries[k].entryName;
            if (sName.equals(_method))
            {
                final boolean bIsOptional = entry.SubEntries[k].isOptional;
                return bIsOptional;
            }
        }
        return false;
    }

    /**
     * Checks if the <code>method</code> test has been already called.
     */
    protected boolean isCalled(String method)
    {
        return methCalled.contains(method);
    }

    /**
     * Calling of the method indicates that the <code>method</code> test should
     * be called. The method checks this and if it is not called, calls it.
     * If the method is failed or skipped, it throws StatusException.
     */
    protected void requiredMethod(String method)
    {
        log.println("starting required method: " + method);
        executeMethod(method);
        Status mtStatus = tRes.getStatusFor(method);

        if (mtStatus != null && (!mtStatus.isPassed() || mtStatus.isFailed()))
        {
            log.println("! Required method " + method + " failed");
            throw new StatusException(mtStatus);
        }
    }

    /**
     * Checks if the <code>method</code> was called, and if not, call it.
     * On contrary to requiredMethod(), he method doesn't check its status.
     */
    protected void executeMethod(String method)
    {
        if (!isCalled(method))
        {
            log.println("Execute: " + method);
            callMethod(method);
            log.println(method + ": " + tRes.getStatusFor(method));
            log.println();
        }
    }

    /**
     * Just calls the <code>method</code> test.
     */
    protected void callMethod(String method)
    {
        methCalled.add(method);
        invokeTestMethod(getMethodFor(method), method);
    }

    /**
     * Invokes a test method of the subclass using reflection API. Handles
     * the method results and sets its status.
     *
     * @param meth the subclass' method to invoke
     * @param methName the name of the method
     */
    protected void invokeTestMethod(Method meth, String methName)
    {
        if (meth == null)
        {
            setStatus(methName, Status.skipped(false));
        }
        else
        {
            Status stat;

            try
            {
                meth.invoke(this, new Object[0]);
                return;
            }
            catch (InvocationTargetException itE)
            {
                Throwable t = itE.getTargetException();

                if (t instanceof StatusException)
                {
                    stat = ((StatusException) t).getStatus();
                }
                else
                {
                    t.printStackTrace(log);
                    stat = Status.exception(t);
                }
            }
            catch (IllegalAccessException iaE)
            {
                iaE.printStackTrace(log);
                stat = Status.exception(iaE);
            }
            catch (IllegalArgumentException iaE)
            {
                iaE.printStackTrace(log);
                stat = Status.exception(iaE);
            }
            catch (ClassCastException ccE)
            {
                ccE.printStackTrace(log);
                stat = Status.exception(ccE);
            }

            setStatus(methName, stat);
        }
    }

    /**
     * Finds a testing method for the <code>method</code> of the interface.
     *
     * @return the testing method, if found, <tt>null</tt> otherwise
     */
    protected Method getMethodFor(String method)
    {
        String mName = "_" + method;

        if (mName.endsWith("()"))
        {
            mName = mName.substring(0, mName.length() - 2);
        }

        final Class<?>[] paramTypes = new Class[0];

        try
        {
            return this.getClass().getDeclaredMethod(mName, paramTypes);
        }
        catch (NoSuchMethodException nsmE)
        {
            return null;
        }
    }

    /**
     * @return the name of the interface tested
     */
    public String getInterfaceName()
    {
        String clName = this.getClass().getName();
        return clName.substring(clName.lastIndexOf('.') + 1);
    }

    /**
     * Initializes <code>fieldName</code> of the subclass with
     * <code>value</code>.
     *
     * @return Status describing the result of the operation.
     */
    protected Status setField(String fieldName, Object value)
    {
        Field objField;

        try
        {
            objField = this.getClass().getField(fieldName);
        }
        catch (NoSuchFieldException nsfE)
        {
            return Status.exception(nsfE);
        }

        try
        {
            objField.set(this, value);
            return Status.passed(true);
        }
        catch (IllegalArgumentException iaE)
        {
            return Status.exception(iaE);
        }
        catch (IllegalAccessException iaE)
        {
            return Status.exception(iaE);
        }
    }
}
