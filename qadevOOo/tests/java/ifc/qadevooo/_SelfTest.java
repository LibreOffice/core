/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



package ifc.qadevooo;

import lib.MultiMethodTest;

// import com.sun.star.uno.UnoRuntime;
// import com.sun.star.uno.XInterface;

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
    public void after()
        {
            log.println("_SelfTest.after()");
            this.disposeEnvironment() ;
        }
}

