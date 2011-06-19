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

package complex.imageManager;



import com.sun.star.lang.XTypeProvider;
import com.sun.star.uno.Type;
import lib.TestParameters;

/**
* Testing <code>com.sun.star.lang.XTypeProvider</code>
* interface methods :
* <ul>
*  <li><code> getTypes()</code></li>
*  <li><code> getImplementationId()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.lang.XTypeProvider
*/
public class _XTypeProvider {


    TestParameters tEnv = null;
    public static XTypeProvider oObj = null;
    public static Type[] types = null;

    public _XTypeProvider(TestParameters tEnv, XTypeProvider oObj) {

        this.tEnv = tEnv;
        this.oObj = oObj;
    }

    /**
    * Just calls the method.<p>
    * Has <b>OK</b> status if no runtime exceptions occurred.
    */
    public boolean _getImplementationId() {
        boolean result = true;
        System.out.println("testing getImplementationId() ... ");

        System.out.println("The ImplementationId is "+oObj.getImplementationId());
        result = true;

        return  result;

    } // end getImplementationId()


    /**
    * Calls the method and checks the return value.<p>
    * Has <b>OK</b> status if one of the return value equals to the
    * type <code>com.sun.star.lang.XTypeProvider</code>.
    */
    public boolean _getTypes() {
        boolean result = false;
        System.out.println("getting Types...");
        types = oObj.getTypes();
        for (int i=0;i<types.length;i++) {
            int k = i+1;
            System.out.println(k+". Type is "+types[i].toString());
            if (types[i].toString().equals
                    ("Type[com.sun.star.lang.XTypeProvider]")) {
                result = true;
            }
        }
        if (!result) {
            System.out.println("Component must provide Type "
                +"<com.sun.star.lang.XTypeProvider>");
        }

        return  result;

    } // end getTypes()

}

