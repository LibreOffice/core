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

package ifc.lang;

import lib.MultiMethodTest;

import com.sun.star.lang.XEventListener;

/**
* Testing <code>com.sun.star.lang.XEventListener</code>
* interface methods :
* <ul>
*  <li><code> disposing()</code></li>
* </ul> <p>
* Tests nothing, all methods has <b>OK</b> status.
* @see com.sun.star.lang.XEventListener
*/
public class _XEventListener extends MultiMethodTest {

    public XEventListener oObj = null;

    /**
    * Nothing to test. Always has <b>OK</b> status.
    */
    public void _disposing() {
        log.println("The method 'disposing'");
        log.println("gets called when the broadcaster is about to be"+
            " disposed.") ;
        log.println("All listeners and all other objects which reference "+
            "the broadcaster should release the references.");
        log.println("So there is nothing to test here");
        tRes.tested("disposing()",true);

    }

} //EOF of XEventListener

