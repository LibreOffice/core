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

package ifc.qadevooo;

import lib.MultiMethodTest;

/**
* Testing <code>ifc.qadevooo._SelfTest</code>
* interface methods:
* <ul>
*   <li><code>testmethod()</code></li>
* </ul> <p>
*
*/
public class _SelfTest extends MultiMethodTest
{

    /**
    */
    @Override
    public void before()
        {
            log.println("_SelfTest.before()");
        }

    /**
    * Test calls the method
    */
    public void _testmethod()
        {
            log.println("_SelfTest._testmethod()");
            tRes.tested("testmethod()", true);
        }

    public void _testmethod2()
        {
            log.println("_SelfTest._testmethod2()");
            tRes.tested("testmethod2()", true);
        }

    public void _testmethod3()
        {
            log.println("_SelfTest._testmethod3()");
            tRes.tested("testmethod3()", true);
        }

    /**
    * Forces object environment recreation.
    */
    @Override
    public void after()
        {
            log.println("_SelfTest.after()");
            this.disposeEnvironment() ;
        }
}

