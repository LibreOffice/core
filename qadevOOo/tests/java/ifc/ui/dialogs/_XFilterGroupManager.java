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
    * Has <b>OK</b> status if no runtime exceptions occurred.
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

