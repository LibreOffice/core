/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XActiveDataSource.java,v $
 * $Revision: 1.5 $
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

