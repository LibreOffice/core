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

package ifc.text;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.beans.NamedValue;
import com.sun.star.task.XJob;
import com.sun.star.text.MailMergeEvent;
import com.sun.star.text.XMailMergeBroadcaster;
import com.sun.star.text.XMailMergeListener;

/**
* Testing <code>com.sun.star.text.XMailMergeBroadcaster</code>
* interface methods:
* <ul>
*  <li><code> addMailMergeEventListener() </code></li>
*  <li><code> removeMailMergeEventListener() </code></li>
* </ul><p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'Job'</code> (of type <code>XJob</code>):
*   used to fire MailMergeEvent</li>
*  <li> <code>'executeArgs'</code> (of type <code>NamedValue[]</code>):
*   used as parameter for <code>'Job'</code> </li>
* </ul> <p>
*
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.text.XMailMergeBroadcaster
*/
public class _XMailMergeBroadcaster extends MultiMethodTest {
    public static XMailMergeBroadcaster oObj = null;
    protected boolean changed = false;


    /**
    * Class we need to test methods
    */
    protected class MyMailMergeEventListener implements XMailMergeListener {
        public void notifyMailMergeEvent ( MailMergeEvent oEvent ) {
            System.out.println("Listener called");
            changed = true;
        }
      }

    protected XMailMergeListener listener = new MyMailMergeEventListener();

    /**
     * Tries to query the tested component for object relation
     * <code>executeArgs</code> [<code>NamedValue</code>] and <code>Job</code>
     * [<code>XJob</code>]
     * @throw StatusException If relations are not found
     */
    public void before() {
        NamedValue[] executeArgs = (NamedValue[]) tEnv.getObjRelation("executeArgs") ;
        if (executeArgs == null)
            throw new StatusException(Status.failed
                ("'executeArgs' relation not found ")) ;
        XJob Job = (XJob) tEnv.getObjRelation("Job") ;
        if (Job == null)
            throw new StatusException(Status.failed
                ("'Job' relation not found ")) ;
    }

    /**
    * Test executes mail merge process<p>
    * Has <b> OK </b> status if listener was called
    */
    public void _addMailMergeEventListener() {
        log.println("Testing addMailMergeEventListener ...");

        oObj.addMailMergeEventListener( listener );

        NamedValue[] executeArgs = (NamedValue[]) tEnv.getObjRelation("executeArgs");
        XJob Job = (XJob) tEnv.getObjRelation("Job");

        try {
            Job.execute(executeArgs);
        } catch ( com.sun.star.lang.IllegalArgumentException e) {
            throw new StatusException(Status.failed
                ("'could not fire event: " + e)) ;
        } catch ( com.sun.star.uno.Exception e) {
            throw new StatusException(Status.failed
                ("'could not fire event: " + e)) ;
        }

        shortWait();

        tRes.tested("addMailMergeEventListener()", changed);
    }

    /**
    * Test executes mail merge process<p>
    * Has <b> OK </b> status if listener was not called
    */
    public void _removeMailMergeEventListener() {
        log.println("Testing removeMailMergeEventListener ...");
        requiredMethod("addMailMergeEventListener()");
        changed = false;

        oObj.removeMailMergeEventListener( listener );

        NamedValue[] executeArgs = (NamedValue[]) tEnv.getObjRelation("executeArgs");
        XJob Job = (XJob) tEnv.getObjRelation("Job");

        try {
            Job.execute(executeArgs);
        } catch ( com.sun.star.lang.IllegalArgumentException e) {
            throw new StatusException(Status.failed
                ("'could not fire event: " + e)) ;
        } catch ( com.sun.star.uno.Exception e) {
            throw new StatusException(Status.failed
                ("'could not fire event: " + e)) ;
        }

        shortWait();

        tRes.tested("removeMailMergeEventListener()", !changed);
    }

    /**
    * Sleeps for 0.2 sec. to allow StarOffice to react on <code>
    * execute</code> call.
    */
    private void shortWait() {
        try {
            Thread.sleep(200) ;
        } catch (InterruptedException e) {
            log.println("While waiting :" + e) ;
        }
    }


} // finished class _XMailMergeBroadcaster

