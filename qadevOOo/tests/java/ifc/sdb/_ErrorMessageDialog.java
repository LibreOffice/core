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

package ifc.sdb;

import lib.MultiPropertyTest;

/**
 * Testing <code>com.sun.star.sdb.ErrorMessageDialog</code>
 * service properties :
 * <ul>
 *  <li><code> Title</code></li>
 *  <li><code> ParentWindow</code></li>
 *  <li><code> SQLException</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'ERR1', 'ERR2'</code>
 *  (of type <code>com.sun.star.sdbc.SQLException</code>):
 *   two objects which are used for changing 'SQLException'
 *   property. </li>
 * <ul> <p>
 * Properties testing is automated by <code>lib.MultiPropertyTest</code>.
 * @see com.sun.star.sdb.ErrorMessageDialog
 */
public class _ErrorMessageDialog extends MultiPropertyTest {

    /**
     * <code>SQLException</code> instances must be used as property
     * value.
     */
    public void _SQLException() {
        log.println("Testing with custom Property tester") ;
        testProperty("SQLException", tEnv.getObjRelation("ERR1"),
            tEnv.getObjRelation("ERR2")) ;
    }

    public void _ParentWindow(){
        log.println("Testing with custom Property tester");
        testProperty("ParentWindow", tEnv.getObjRelation("ERR_XWindow"), null);
    }

}  // finish class _ErrorMessageDialog


