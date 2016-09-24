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

