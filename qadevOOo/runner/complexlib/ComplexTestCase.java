/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ComplexTestCase.java,v $
 * $Revision: 1.16.2.1 $
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
package complexlib;

import java.lang.reflect.Method;
import share.DescEntry;
import lib.TestParameters;
import lib.StatusException;
import share.LogWriter;
import share.ComplexTest;
import java.io.PrintWriter;

/**
 * Base class for all complex tests.
 */
public abstract class ComplexTestCase extends Assurance implements ComplexTest
{

    /** The test parameters **/
    protected static TestParameters param = null;
    /** Log writer **/
    protected static LogWriter log = null;
    /** Description entry **/
    protected DescEntry subEntry = null;
    /**
     * The method name which will be written into f.e. the data base
     **/
    protected String mTestMethodName = null;
    /** Maximal time one method is allowed to execute
     * Can be set with parameter 'ThreadTimeOut'
     **/
    protected int mThreadTimeOut = 0;
    /** Continue a test even if it did fail **/
    public static final boolean CONTINUE = true;
    /** End a test if it did fail **/
    public static final boolean BREAK = true;

    /**
     * Call test. It is expected, that an environment is
     * given to this test.
     *
     * @param entry The name of the test method that should be called.
     * @param environment The environment for the test.
     */
    public void executeMethods(DescEntry entry, TestParameters environment)
    {

        // get the environment
        param = environment;
        log = entry.Logger;

        mThreadTimeOut = param.getInt("ThreadTimeOut");
        if (mThreadTimeOut == 0)
        {
            mThreadTimeOut = 300000;
        }
        // start with the before() method
        boolean beforeWorked = true;
        try
        {
            Method before = this.getClass().getMethod("before", new Class[] {} );
            before.invoke(this, new Object[] {} );
        }
        catch (java.lang.NoSuchMethodException e)
        {
            // simply ignore
        }
        catch (java.lang.IllegalAccessException e)
        {
            log.println("Cannot access the 'before()' method, although it" + " is there. Is this ok?");
        }
        catch (java.lang.reflect.InvocationTargetException e)
        {
            beforeWorked = false;
            Throwable t = e.getTargetException();
            if (!(t instanceof RuntimeException) || state)
            {
                log.println(t.toString());
                if (message == null)
                {
                    message = "Exception in before() method.\n\r" + t.getMessage();
                }
                state = false;
                t.printStackTrace((PrintWriter) log);
            }
        }


        //executeMethodTests
        for (int i = 0; i < entry.SubEntries.length; i++)
        {
            subEntry = entry.SubEntries[i];
            if (beforeWorked)
            {
                state = true;
                message = "";
            }
            else
            {
                // set all test methods on failed, if 'before()' did not work.
                subEntry.State = message;
                subEntry.hasErrorMsg = true;
                subEntry.ErrorMsg = message;
                continue;
            }
            Method testMethod = null;
            try
            {
                String entryName = subEntry.entryName;
                Object[] parameter = null;

                if (entryName.indexOf("(") != -1)
                {
                    String sParameter = (entryName.substring(entryName.indexOf("(") + 1, entryName.indexOf(")")));
                    mTestMethodName = entryName;
                    parameter = new String[]
                            {
                                sParameter
                            };
                    entryName = entryName.substring(0, entryName.indexOf("("));
                    testMethod = this.getClass().getMethod(entryName, new Class[] { String.class });
                }
                else
                {
                    testMethod = this.getClass().getMethod(entryName, new Class[] {} );
                    mTestMethodName = entryName;
                }

                MethodThread th = new MethodThread(testMethod, this, parameter, (java.io.PrintWriter) log);
                log.println("Starting " + mTestMethodName);
                th.start();

                try
                {
                    // some tests are very dynamic in its exceution time so that
                    // a threadTimeOut fials. In this cases the logging mechanisim
                    // is a usefull way to detect that a office respective a test
                    // is running and not death.
                    // But way ThreadTimeOut?
                    // There exeitsts a complex test which uses no office. Therefore
                    // a logging mechanisim to detect a stalled test.
                    int lastPing = -1;
                    int newPing = 0;

                    int sleepingStep = 1000;
                    int factor = 0;

                    while (th.isAlive() && (lastPing != newPing || factor * sleepingStep < mThreadTimeOut))
                    {
                        Thread.sleep(sleepingStep);
                        factor++;
                        // if a test starts the office itself it the watcher is a
                        // new one.
                        share.Watcher ow = (share.Watcher) param.get("Watcher");
                        if (ow != null)
                        {
                            lastPing = newPing;
                            newPing = ow.getPing();
                            //System.out.println("lastPing: '" + lastPing + "' newPing '" + newPing + "'");
                            factor = 0;
                        }
                    }
                }
                catch (InterruptedException e)
                {
                }
                if (th.isAlive())
                {
                    log.println("Destroy " + mTestMethodName);
                    th.destroy();
                    subEntry.State = "Test did sleep for " + (mThreadTimeOut / 1000) + " seconds and has been killed!";
                    subEntry.hasErrorMsg = true;
                    subEntry.ErrorMsg = subEntry.State;
                    continue;
                }
                else
                {
                    log.println("Finished " + mTestMethodName);
                    if (th.hasErrorMessage())
                    {
                        subEntry.State = th.getErrorMessage();
                        subEntry.hasErrorMsg = true;
                        subEntry.ErrorMsg = subEntry.State;
                        continue;
                    }
                }
            }
            catch (java.lang.Exception e)
            {
                log.println(e.getClass().getName());
                String msg = e.getMessage();
                log.println("Message: " + msg);
                e.printStackTrace((PrintWriter) log);
                subEntry.State = "SKIPPED.FAILED";
                subEntry.hasErrorMsg = true;
                subEntry.ErrorMsg = (msg == null ? "" : msg);
                continue;
            }
            subEntry.State = (state ? "PASSED.OK" : message);
            subEntry.hasErrorMsg = !state;
            subEntry.ErrorMsg = message;
        }

        if (beforeWorked)
        {
            // the after() method
            try
            {
                Method after = this.getClass().getMethod("after", new Class[] {});
                after.invoke(this, new Object[] {} );
            }
            catch (java.lang.NoSuchMethodException e)
            {
                // simply ignore
            }
            catch (java.lang.IllegalAccessException e)
            {
                // simply ignore
            }
            catch (java.lang.reflect.InvocationTargetException e)
            {
                Throwable t = e.getTargetException();
                if (!(t instanceof StatusException))
                {
                    log.println(t.toString());
                    if (message == null)
                    {
                        message = "Exception in after() method.\n\r" + t.getMessage();
                    }
                    else
                    {
                        message += "Exception in \'after()\' method.\n\r" + t.getMessage();
                    }
                    log.println("Message: " + message);
                    t.printStackTrace((PrintWriter) log);
                }
            }
        }
    }

    /**
     * Implement this method in the Complex test.
     * @return All test method names.
     */
    public abstract String[] getTestMethodNames();

    /**
     * Return a name for the test or tested object.
     * Override to give an own name.
     * @return As default, the name of this class.
     */
    public String getTestObjectName()
    {
        return this.getClass().getName();
    }
}
