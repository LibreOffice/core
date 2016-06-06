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

package ifc.task;



/**
* <code>com.sun.star.task.XInteractionHandler</code> interface testing.
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInteractionHandler.Request'</code>
*    (of type <code>com.sun.star.task.XInteractionRequest</code>):
*    this interface implementation is handler specific and is
*    passed as argument to method <code>handle</code>. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.task.XInteractionHandler
*/
import lib.MultiMethodTest;
import lib.Status;

import com.sun.star.task.XInteractionHandler;

/**
* <code>com.sun.star.task.XInteractionHandler</code> interface testing.
* This test needs the following object relations :
* <ul>
*  <li> <code>'XInteractionHandler.Request'</code>
*    (of type <code>com.sun.star.task.XInteractionRequest</code>):
*    this interface implementation is handler specific and is
*    passed as argument to method <code>handle</code>. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.task.XInteractionHandler
*/
public class _XInteractionHandler extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XInteractionHandler oObj = null ;

    /**
    * Retrieves an object relation. <p>
    */
    @Override
    public void before() {
    }

    /**
    * Since this test is interactive (dialog window can't be
    * disposed using API) it is skipped. <p>
    * Always has <b>SKIPPED.OK</b> status .
    */
    public void _handle() {
        tRes.tested("handle()", Status.skipped(true)) ;
    }
}


