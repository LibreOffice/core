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


