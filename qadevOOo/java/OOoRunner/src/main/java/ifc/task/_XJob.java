/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package ifc.task;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.NamedValue;
import com.sun.star.task.XJob;

/**
 * Testing <code>com.sun.star.frame._XJobExecutor</code>
 * interface methods:
 * <ul>
 *  <li><code> trigger() </code></li>
 * </ul><p>
 *
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'CallCounter'</code>
 *      (of type <code>com.sun.star.container.XNamed</code>):
 *      the <code>getName()</code> method of which must
 *      return number of calls to <code>XJob.execute</code>
 *      method which is registered for event 'TestEvent'
 *  </li>
 * <ul> <p>
 * @see com.sun.star.frame.XJobExecutor
 */
public class _XJob extends MultiMethodTest {
    public static XJob oObj = null;

    /**
     * Tries to query the tested component for object relation
     * <code>XJobArgs</code> [<code>Object[]</code>] which contains
     * <code>executeArgs</code> [<code>NamedValue[]</code>]
     * @throw StatusException If relations are not found
     */
    public void before() {
        Object[] XJobArgs = (Object[]) tEnv.getObjRelation("XJobArgs") ;
        if (XJobArgs == null)
            throw new StatusException(Status.failed
                ("'XJobArgs' relation not found ")) ;
    }



    /**
     * Gets the number of Job calls before and after triggering event.
     *
     * Has <b>OK</b> status if the Job was called on triggering
     * event.
     */
    public void _execute() {
        Object[] XJobArgs = (Object[]) tEnv.getObjRelation("XJobArgs");

    boolean bOK = true;

        for (int n = 0; n<XJobArgs.length; n++) {
            log.println("running XJobArgs[" + n + "]");
            try {
                oObj.execute((NamedValue[])XJobArgs[n]);
            } catch ( com.sun.star.lang.IllegalArgumentException e) {
                bOK = false;
                log.println("Could not success XJobArgs[" + n + "]: " + e);
            } catch ( com.sun.star.uno.Exception e) {
                bOK = false;
                log.println("Could not success XJobArgs[" + n + "]: " + e);
            }
        }
        tRes.tested("execute()", bOK);
    }
}
