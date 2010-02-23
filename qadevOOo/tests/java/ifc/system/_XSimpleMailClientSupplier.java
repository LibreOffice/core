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
*   is available, if NOT then the service is not availabel or
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
            ((Boolean)tEnv.getObjRelation("SystemMailExist"))) {

            XSimpleMailClient aClient = oObj.querySimpleMailClient();
            tRes.tested("querySimpleMailClient()",aClient != null);
        } else {
            log.println("SystemMail doesn't exist : nothing to test") ;
            tRes.tested("querySimpleMailClient()", Status.skipped(true)) ;
        }
    }


}  // finish class _XSimpleMailClientSupplier


