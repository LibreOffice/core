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

package ifc.reflection;

import com.sun.star.lang.XMultiServiceFactory;
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
    protected final static String typeName = "com.sun.star.container.XNameAccess";

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
        boolean result = true;
        Object obj = null;

        try {
            obj = ((XMultiServiceFactory)tParam.getMSF()).
                                    createInstance("com.sun.star.io.Pipe") ;
        } catch (com.sun.star.uno.Exception e) {
            log.println("Can't create object");
            tRes.tested("getType()", false);
            return;
        }

        if (obj == null) {
            result = false;
            log.println("Object wasn't created !");
            tRes.tested("getType()", false);
        }

        XIdlClass cls = oObj.getType(obj);

        log.println("The name is " + cls.getName());

        tRes.tested("getType()", cls != null);
    }
}


