/*************************************************************************
 *
 *  $RCSfile: _XTasksSupplier.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:40:28 $
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

