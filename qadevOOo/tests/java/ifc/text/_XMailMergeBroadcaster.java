/*************************************************************************
 *
 *  $RCSfile: _XMailMergeBroadcaster.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:17:08 $
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

