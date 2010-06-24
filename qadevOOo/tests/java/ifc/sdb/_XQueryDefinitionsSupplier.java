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

package ifc.sdb;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.sdb.XQueryDefinitionsSupplier;

/**
 * Testing <code>com.sun.star.sdb.XQueryDefinitionsSupplier</code>
 * interface methods :
 * <ul>
 *  <li><code> getQueryDefinitions()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.sdb.XQueryDefinitionsSupplier
 */
public class _XQueryDefinitionsSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XQueryDefinitionsSupplier oObj = null ;

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     */
    public void _getQueryDefinitions() {

        XNameAccess docs = oObj.getQueryDefinitions() ;

        String[] docNames = docs.getElementNames() ;
        if (docNames != null) {
            log.println("Totally " + docNames.length + " queries :") ;
            for (int i = 0; i < docNames.length; i++)
                log.println("  " + docNames[i]) ;
        }

        tRes.tested("getQueryDefinitions()", docNames != null) ;
    }

}  // finish class _XQueryDefinitionsSupplier

