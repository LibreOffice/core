/*************************************************************************
 *
 *  $RCSfile: ComplexTestCase.java,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Date: 2003-10-06 12:37:52 $
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
public abstract class ComplexTestCase implements ComplexTest {
    /** The test parameters **/
    protected static TestParameters param = null;
    /** Log writer **/
    protected static LogWriter log = null;
    /** Description entry **/
    protected DescEntry subEntry = null;
    /** State of the current test method **/
    protected boolean state = true;
    /** The message if the test did fail **/
    protected String message = null;
    /** Maximal time one method is allowed to execute
     * Can be set with parameter 'ThreadTimeOut'
     **/
    protected int mThreadTimeOut = 0;

    /**
     * Call test. It is expected, that an environment is
     * given to this test.
     *
     * @param method The name of the test method that should be called.
     * @param environment The environment for the test.
     */
    public void executeMethods(DescEntry entry, TestParameters environment) {

        // get the environment
        param = environment;
        log = entry.Logger;

        mThreadTimeOut = param.getInt("ThreadTimeOut");
        if (mThreadTimeOut == 0) mThreadTimeOut = 300000;

        // start with the before() method
        boolean beforeWorked = true;
        try {
            Method before = this.getClass().getMethod("before",null);
            before.invoke(this, null);
        }
        catch(java.lang.NoSuchMethodException e) {
            // simply ignore
        }
        catch(java.lang.IllegalAccessException e) {
            log.println("Cannot access the 'before()' method, although it" +
                                            " is there. Is this ok?");
        }
        catch(java.lang.reflect.InvocationTargetException e) {
            beforeWorked = false;
            Throwable t = e.getTargetException();
            if (!(t instanceof RuntimeException) || state) {
                log.println(t.toString());
                if ( message == null ) {
                    message = "Exception in before() method.\n\r" + t.getMessage();
                }
                state = false;
                t.printStackTrace((PrintWriter)log);
            }
        }


        //executeMethodTests
        for (int i=0; i<entry.SubEntries.length; i++) {
            subEntry = entry.SubEntries[i];
            if (beforeWorked) {
                state = true;
                message = "";
            }
            else  {
                // set all test methods on failed, if 'before()' did not work.
                subEntry.State = message;
                subEntry.hasErrorMsg = true;
                subEntry.ErrorMsg = message;
                continue;
            }
            Method testMethod = null;
            try {
                 testMethod = this.getClass().getMethod(
                                        subEntry.entryName,null);
                 MethodThread th = new MethodThread(testMethod, this,
                                                    (java.io.PrintWriter)log);
                 th.start();
                 try {
                     int sleepingStep = 1000;
                     int factor = 0;
                     while(th.isAlive() && factor*sleepingStep<mThreadTimeOut) {
                        Thread.sleep(sleepingStep);
                        factor++;
                     }

                 }
                 catch(InterruptedException e) {}
                 if (th.isAlive()) {
                    log.println("Destroy " + testMethod.getName());
                    th.destroy();
                    subEntry.State="Test did sleep for " +
                                        (mThreadTimeOut / 1000) +
                                        " seconds and has been killed!";
                    subEntry.hasErrorMsg = true;
                    subEntry.ErrorMsg = subEntry.State;
                    continue;
                 } else {
                     System.out.println("Finished " + testMethod.getName());
                     if (th.hasErrorMessage()) {
                        subEntry.State="Test did sleep for " +
                                            (mThreadTimeOut / 1000) +
                                            " seconds and has been killed!";
                        subEntry.hasErrorMsg = true;
                        subEntry.ErrorMsg = subEntry.State;
                        continue;
                     }
                 }
            }
            catch(java.lang.Exception e) {
                log.println(e.getClass().getName());
                String msg = e.getMessage();
                log.println("Message: " + msg);
                e.printStackTrace((PrintWriter)log);
                subEntry.State="SKIPPED.FAILED";
                subEntry.hasErrorMsg = true;
                subEntry.ErrorMsg = (msg == null?"":msg);
                continue;
            }
            subEntry.State = (state?"PASSED.OK":message);
            subEntry.hasErrorMsg = !state;
            subEntry.ErrorMsg = message;
        }

        if (beforeWorked) {
            // the after() method
            try {
                Method after = this.getClass().getMethod("after",null);
                after.invoke(this, null);
            }
            catch(java.lang.NoSuchMethodException e) {
                // simply ignore
            }
            catch(java.lang.IllegalAccessException e) {
                // simply ignore
            }
            catch(java.lang.reflect.InvocationTargetException e) {
                Throwable t = e.getTargetException();
                if (!(t instanceof StatusException)) {
                    log.println(t.toString());
                    if ( message == null ) {
                        message = "Exception in after() method.\n\r" + t.getMessage();
                    }
                    else  {
                        message += "Exception in 'after()' method.\n\r" + t.getMessage();
                    }
                    log.println("Message: " + message);
                    t.printStackTrace((PrintWriter)log);
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
    public String getTestObjectName() {
        return this.getClass().getName();
    }

    /**
     * Assure that s is true.
     * This function generates "Assure failed." as standard message.
     * @param s The condition that should be true.
     */
    protected void assure(boolean s) {
        assure("Assure failed.", s, false);
    }

    /**
     * Assure that s is true.
     * The given message will be only evaluated, if s is false.
     * @param msg The message that is evaluated.
     * @param s The condition that should be true.
     */
    protected void assure(String msg, boolean s) {
        assure(msg, s, false);
    }

    /**
     * Mark the currently executed method as failed.
     * This function generates "Test did fail." as standard message.
     */
    protected void failed() {
        assure("Test did fail.", false, false);
    }

    /**
     * Mark the currently executed method as failed.
     * with the given message.
     * @param msg The message of the failure.
     */
    protected void failed(String msg) {
        assure(msg, false, false);
    }

    /**
     * Assure that s is true.
     * The given message will be only evaluated, if s is false.
     * Normally, assure() leaves the current test method, and the next one
     * is executed. With the parameter 'cont' set to true, the current test
     * method will continue.<br>
     * The current method will of course marked as failed.
     * @param msg The message that is evaluated.
     * @param s The condition that should be true.
     * @param cont Continue with test method, even if s is false.
     */
    protected void assure(String msg, boolean s, boolean cont) {
        state &= s;
        if (!s) {
            message += msg + "\r\n";
            log.println(msg);
            if (!cont) {
                throw new RuntimeException(msg);
            }
        }
    }

    /**
     * Mark the currently executed method as failed.
     * with the given message.
     * The given message will be only evaluated, if s is false.
     * With the parameter 'cont' set to true, the current test
     * method will continue.<br>
     * The current method will of course marked as failed.
     * @param msg The message that is evaluated.
     * @param cont Continue with test method, even if s is false.
     */
    protected void failed(String msg, boolean cont) {
        assure(msg, false, cont);
    }

    /**
     * @deprecated
     */
    protected void addResult(String message, boolean state) {
        String msg = message + " - " + state;
        this.state &= state;
        this.message += msg + "\r\n";
        log.println(msg);
    }
}


