 /*************************************************************************
 *
 *  $RCSfile: _XJob.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:11:54 $
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
