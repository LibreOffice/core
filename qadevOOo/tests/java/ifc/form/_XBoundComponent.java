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

import com.sun.star.form.XBoundComponent;

/**
* Testing <code>com.sun.star.form.XBoundComponent</code>
* interface methods :
* <ul>
*  <li><code> commit()</code></li>
* </ul> <p>
* More deep tests of this interface are made in
* <code>com.sun.star.form.XUpdateBroadcaster</code> interface
* test as it is the ancestor of <code>XBoundComponent</code>
* interface.
* @see com.sun.star.form.XBoundComponent
*/
public class _XBoundComponent extends MultiMethodTest {

    public XBoundComponent oObj = null;

    public void _commit() {
        oObj.commit() ;

        tRes.tested("commit()",true);
    }

} //EOF of XBoundComponent


