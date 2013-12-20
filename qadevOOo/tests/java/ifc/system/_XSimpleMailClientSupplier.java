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

package ifc.system;

import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.system.XSimpleMailClient;
import com.sun.star.system.XSimpleMailClientSupplier;


/**
* Testing <code>com.sun.star.system.XSimpleMailClientSupplier</code>
* interface methods :
* <ul>
*  <li><code> querySimpleMailClient()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'SystemMailExist'</code> (of type <code>Boolean</code>):
*   if <code>true</code> then <code>SimpleSystemMail</code> service
*   is available, if NOT then the service is not availablel or
*   OS doesn't has system mail.</li>
* <ul> <p>
* @see com.sun.star.system.XSimpleMailClientSupplier
*/
public class _XSimpleMailClientSupplier extends MultiMethodTest {

    public XSimpleMailClientSupplier oObj = null;

    /**
    * Test calls the method if the <code>SimpleSystemMail</code>
    * service is available on current OS. <p>
    * Has <b> OK </b> status if not null value returned or
    * has <b>SKIPPED.OK</b> status if SystemMail is not available. <p>
    */
    public void _querySimpleMailClient() {
        if (Boolean.TRUE.equals
            (tEnv.getObjRelation("SystemMailExist"))) {

            XSimpleMailClient aClient = oObj.querySimpleMailClient();
            tRes.tested("querySimpleMailClient()",aClient != null);
        } else {
            log.println("SystemMail doesn't exist : nothing to test") ;
            tRes.tested("querySimpleMailClient()", Status.skipped(true)) ;
        }
    }


}  // finish class _XSimpleMailClientSupplier


