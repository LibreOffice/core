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

package ifc.document;


import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.document.XExporter;
import com.sun.star.lang.XComponent;

/**
* Testing <code>com.sun.star.document.XExporter</code>
* interface methods :
* <ul>
*  <li><code> setSourceDocument()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'SourceDocument'</code> (of type <code>XComponent</code>):
*   the source document to be passed to the method. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.document.XExporter
*/
public class _XExporter extends MultiMethodTest {

    public XExporter oObj = null;
    public XComponent source = null ;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    public void before() {
        source = (XComponent) tEnv.getObjRelation("SourceDocument") ;

        if (source == null) throw new StatusException(Status.failed
            ("Relation not found")) ;
    }

    /**
    * Just calls the method. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred.
        * Usually this interface is supported both with <code>XFilter</code>
        * where source document setting is checked.
    */
    public void _setSourceDocument() {
        boolean result = true ;

        try {
            oObj.setSourceDocument(source);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking :");
            ex.printStackTrace(log);
            result = false;
        }

        tRes.tested("setSourceDocument()", result) ;
    }
}


