/*************************************************************************
 *
 *  $RCSfile: ComplexTestCase.java,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Date: 2003-04-01 09:18:04 $
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
import com.sun.star.lang.XMultiServiceFactory;
import share.LogWriter;
import share.ComplexTest;
import java.io.PrintWriter;

public abstract class ComplexTestCase implements ComplexTest {

    protected static TestParameters param;
    protected static LogWriter log;
    protected DescEntry subEntry;
    protected boolean state;
    protected String message;

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
        String errorMsg = null;

        // start with the before() method
        try {
            Method before = this.getClass().getMethod("before",null);
            before.invoke(this, null);
        }
        catch(java.lang.NoSuchMethodException e) {
            // simply ignore
        }
        catch(java.lang.IllegalAccessException e) {
            // simply ignore
        }
        catch(java.lang.reflect.InvocationTargetException e) {
            Throwable t = e.getTargetException();
            log.println(t.toString());
            errorMsg = "Exception in before() method.\n\r" +
                                                            t.getMessage();
            if (errorMsg == null)
                errorMsg = "";
            log.println("Message: " + errorMsg);
            t.printStackTrace((PrintWriter)log);
        }

        //executeMethodTests
        for (int i=0; i<entry.SubEntries.length; i++) {
            state = true;
            message = "";
            subEntry = entry.SubEntries[i];
            // set all test methods on failed, if 'before()' did not work.
            if (errorMsg != null) {
                subEntry.State = errorMsg;
                subEntry.hasErrorMsg = true;
                subEntry.ErrorMsg = errorMsg;
                continue;
            }
            Method testMethod = null;
            try {
                 testMethod = this.getClass().getMethod(
                                        subEntry.entryName,null);
                 testMethod.invoke(this, null);
            }
            catch(java.lang.reflect.InvocationTargetException e) {
                Throwable t = e.getTargetException();
                log.println(t.toString());
                String msg = t.getMessage();
                log.println("Message: " + msg);
                t.printStackTrace((PrintWriter)log);
                subEntry.State=message + (msg == null?"":msg);
                subEntry.hasErrorMsg = true;
                subEntry.ErrorMsg = message + "\n" + msg;
                continue;
            }
            catch(java.lang.Exception e) {
                log.println(e.getClass().getName());
                log.println("Message: " + e.getMessage());
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

        if (errorMsg == null) {
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
                log.println(t.toString());
                errorMsg = "Exception in 'after()' method.\n\r" +
                                                                t.getMessage();
                if (errorMsg == null)
                    errorMsg = "";
                log.println("Message: " + errorMsg);
                t.printStackTrace((PrintWriter)log);
            }
        }
    }

    public abstract String[] getTestMethodNames();

    public abstract String getTestObjectName();

    protected void assure(String msg, boolean s) {
        state &= s;
        if (!s) {
            message += msg + "\r\n";
            log.println(msg);
        }
    }

    protected void addResult(String message, boolean state) {
        String msg = message + " - " + state;
        this.state &= state;
        this.message += msg + "\r\n";
        log.println(msg);
    }

    protected void failed(String msg) {
        assure(msg, false);
    }
}
