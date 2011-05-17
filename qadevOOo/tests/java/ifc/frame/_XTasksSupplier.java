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

package ifc.frame;

import lib.MultiMethodTest;

import com.sun.star.frame.XTasksSupplier;


/**
* Testing <code>com.sun.star.frame.XTasksSupplier</code>
* interface methods:
* <ul>
*  <li><code> getActiveTask() </code></li>
*  <li><code> getTasks() </code></li>
* </ul><p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.frame.XTasksSupplier
*/
public class _XTasksSupplier extends MultiMethodTest {
    public static XTasksSupplier oObj = null;

    /**
    * DEPRECATED. <p>
    * Has <b> OK </b> status .
    */
    public void _getActiveTask() {
//        XTask active = oObj.getActiveTask() ;
//        if (active == null)
//            log.println("There is no active task");
//        else
//            log.println("Active task: " + active.getName());
        log.println("DEPRECATED");

        tRes.tested("getActiveTask()", true) ;
    } // finished _getTasks()

    /**
    * DEPRECATED. <p>
    * Has <b> OK </b> status.
    */
    public void _getTasks() {
//        int cnt = 0 ;
//        boolean result = true ;
//        XTask task = null ;
//
//        XEnumerationAccess enumAcc = oObj.getTasks() ;
//        XEnumeration enum = enumAcc.createEnumeration() ;
//        while (enum.hasMoreElements()) {
//            cnt ++ ;
//            try {
//                task = (XTask) enum.nextElement() ;
//            } catch (com.sun.star.container.NoSuchElementException e) {
//                e.printStackTrace(log);
//            } catch (com.sun.star.lang.WrappedTargetException e) {
//                e.printStackTrace(log);
//            }
//            if (task == null)
//                log.println("" + cnt + " task: null !!!") ;
//            else
//                log.println("" + cnt + " task: " + task.getName()) ;
//            result &= (task != null) ;
//        }
//        log.println("Totaly tasks: " + cnt) ;
//
//        if (cnt > 0 && result) tRes.tested("getTasks()", true) ;

        log.println("DEPRECATED");

        tRes.tested("getTasks()", true);

    } // finished _removeResetListener()

} // finished class _XTaskSupplier

