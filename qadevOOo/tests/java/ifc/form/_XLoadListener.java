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

package ifc.form;

import lib.MultiMethodTest;

import com.sun.star.form.XLoadListener;

/**
* Testing <code>com.sun.star.form.XLoadListener</code>
* interface methods :
* <ul>
*  <li><code> loaded()</code></li>
*  <li><code> unloading()</code></li>
*  <li><code> unloaded()</code></li>
*  <li><code> reloading()</code></li>
*  <li><code> reloaded()</code></li>
* </ul> <p>
* This interface methods are not testable, because their
* behaviour is not documented.
* @see com.sun.star.form.XLoadListener
*/
public class _XLoadListener extends MultiMethodTest {

    public XLoadListener oObj = null;

    /**
    * Always has <b>OK</b> status.
    */
    public void _loaded() {
        log.println("nothing to test here");
        tRes.tested("loaded()",true);
    }

    /**
    * Always has <b>OK</b> status.
    */
    public void _reloaded() {
        log.println("nothing to test here");
        tRes.tested("reloaded()",true);
    }

    /**
    * Always has <b>OK</b> status.
    */
    public void _reloading() {
        log.println("nothing to test here");
        tRes.tested("reloading()",true);
    }

    /**
    * Always has <b>OK</b> status.
    */
    public void _unloaded() {
        log.println("nothing to test here");
        tRes.tested("unloaded()",true);
    }

    /**
    * Always has <b>OK</b> status.
    */
    public void _unloading() {
        log.println("nothing to test here");
        tRes.tested("unloading()",true);
    }

} //EOF of XLoadListener


