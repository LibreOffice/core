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

package ifc.beans;

import lib.MultiMethodTest;

import com.sun.star.beans.XIntrospection;
import com.sun.star.beans.XIntrospectionAccess;

/**
* Testing <code>com.sun.star.beans.XIntrospection</code>
* interface methods :
* <ul>
*  <li><code> inspect() </code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.beans.XIntrospection
*/
public class _XIntrospection extends MultiMethodTest {

    public XIntrospection oObj = null;

    /**
    * Test inspects the object tested, and checks if it
    * has a base <code>XInterface</code> method
    * <code>acquire</code>.
    */
    public void _inspect() {
       boolean result = true;
       XIntrospectionAccess xIA = oObj.inspect(oObj);
       result = (xIA.hasMethod("acquire",1));
       tRes.tested("inspect()",result);
    }

}  // finish class _XIntrospection


