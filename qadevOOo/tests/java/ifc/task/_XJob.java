 /*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
