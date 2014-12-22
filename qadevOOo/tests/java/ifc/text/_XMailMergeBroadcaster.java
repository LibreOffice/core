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
* Test is <b> NOT </b> multithread compliant. <p>
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
    @Override
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

        util.utils.pause(200);

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

        util.utils.pause(200);

        tRes.tested("removeMailMergeEventListener()", !changed);
    }


} // finished class _XMailMergeBroadcaster

