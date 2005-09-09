 /*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: _XJobExecutor.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 01:09:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package ifc.task;

import lib.MultiMethodTest;

import com.sun.star.container.XNamed;
import com.sun.star.task.XJobExecutor;

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
public class _XJobExecutor extends MultiMethodTest {
    public static XJobExecutor oObj = null;

    /**
     * Gets the number of Job calls before and after triggering event.
     *
     * Has <b>OK</b> status if the Job was called on triggering
     * event.
     */
    public void _trigger() {
        XNamed counter = (XNamed) tEnv.getObjRelation("CallCounter");

        int before = Integer.parseInt(counter.getName());

        oObj.trigger("TestEvent");

        try {
            Thread.sleep(1000);
        } catch (InterruptedException ex) {}

        int after = Integer.parseInt(counter.getName());

        log.println("Calls before: " + before + ", after: " + after);

        tRes.tested("trigger()", after == before + 1);
    }
}
