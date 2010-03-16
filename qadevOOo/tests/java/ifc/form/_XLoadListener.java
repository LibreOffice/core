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


