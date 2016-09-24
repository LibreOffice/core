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
* Test is multithread compliant. <p>
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
       result = xIA.hasMethod("acquire",1);
       tRes.tested("inspect()",result);
    }

}  // finish class _XIntrospection


