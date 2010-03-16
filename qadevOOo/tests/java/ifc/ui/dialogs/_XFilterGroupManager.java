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

package ifc.ui.dialogs;

import lib.MultiMethodTest;

import com.sun.star.beans.StringPair;
import com.sun.star.ui.dialogs.XFilterGroupManager;

/**
* Testing <code>com.sun.star.ui.dialogs.XFilterGroupManager</code>
* @see com.sun.star.ui.XFilterGroupManager
*/
public class _XFilterGroupManager extends MultiMethodTest {

    public XFilterGroupManager oObj = null;

    /**
    * Appends a new FilterGroup (for extension 'txt'). <p>
    * Has <b>OK</b> status if no runtime exceptions ocured.
    */
    public void _appendFilterGroup() {
        boolean res = true;
        try {
            StringPair[] args = new StringPair[1];
            args[0] = new StringPair();
            args[0].First = "ApiTextFiles";
            args[0].Second = "txt";
            oObj.appendFilterGroup("TestFilter", args);
        } catch (com.sun.star.lang.IllegalArgumentException e) {
            e.printStackTrace(log);
            res=false;
        }

        tRes.tested("appendFilterGroup()", res) ;
    }

}

