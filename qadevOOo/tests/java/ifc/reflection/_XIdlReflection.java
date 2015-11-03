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

package ifc.reflection;

import com.sun.star.reflection.XIdlClass;
import com.sun.star.reflection.XIdlReflection;
import com.sun.star.uno.TypeClass;
import lib.MultiMethodTest;

/**
* Testing <code>com.sun.star.reflection.XIdlReflection</code>
* interface methods :
* <ul>
*  <li><code> forName()</code></li>
*  <li><code> getType()</code></li>
* </ul> <p>
* @see com.sun.star.reflection.XIdlReflection
*/
public class _XIdlReflection extends MultiMethodTest{
    public XIdlReflection oObj = null;
    protected static final String typeName = "com.sun.star.container.XNameAccess";

    /**
    * Test calls the method and checks returned interface
    * <code>com.sun.star.container.XNameAccess</code>: gets the name and the
    * type and checks it. <p>
    * Has <b> OK </b> status if returned name is equal to the name of the
    * interface that was passed as parameter in the method call and if returned
    * type is equal to <code>com.sun.star.uno.TypeClass.INTERFACE</code>. <p>
    */
    public void _forName() {
        boolean result = true;
        XIdlClass cls = oObj.forName(typeName);

        if (cls != null) {
            log.println("Class name: " + cls.getName());
            result &= cls.getTypeClass() == TypeClass.INTERFACE;
            result &= typeName.equals(cls.getName());
        } else {
            log.println("Method returned null");
            result = false;
        }

        tRes.tested("forName()", result);
    }

    /**
    * Test creates the instance of <code>com.sun.star.io.Pipe</code>,
    * calls the method using this instance as parameter and checks returned
    * value. <p>
    * Has <b> OK </b> status if the instance was created successfully, if
    * returned value isn't null and no exceptions were thrown. <p>
    */
    public void _getType() {
        Object obj = null;

        try {
            obj = tParam.getMSF().
                                    createInstance("com.sun.star.io.Pipe") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create object");
            tRes.tested("getType()", false);
            return;
        }

        if (obj == null) {
            log.println("Object wasn't created !");
            tRes.tested("getType()", false);
        }

        XIdlClass cls = oObj.getType(obj);

        if (cls != null) {
            log.println("The name is " + cls.getName());
        }

        tRes.tested("getType()", cls != null);
    }
}


