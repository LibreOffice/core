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

package ifc.datatransfer.clipboard;

import lib.MultiMethodTest;

import com.sun.star.datatransfer.clipboard.XFlushableClipboard;

/**
* Testing <code>com.sun.star.datatransfer.clipboard.XFlushableClipboard</code>
* interface methods :
* <ul>
*  <li><code> flushClipboard()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.datatransfer.clipboard.XFlushableClipboard
*/
public class _XFlushableClipboard extends MultiMethodTest {

    public XFlushableClipboard oObj;

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
    */
    public void _flushClipboard() {
        oObj.flushClipboard();
        tRes.tested("flushClipboard()",true);
    }

}
