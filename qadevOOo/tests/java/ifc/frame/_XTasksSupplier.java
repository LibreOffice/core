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

//        if (cnt > 0 && result) tRes.tested("getTasks()", true) ;

        log.println("DEPRECATED");

        tRes.tested("getTasks()", true);

    } // finished _removeResetListener()

} // finished class _XTaskSupplier

