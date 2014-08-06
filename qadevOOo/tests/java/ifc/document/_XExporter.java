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
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.document.XExporter
*/
public class _XExporter extends MultiMethodTest {

    public XExporter oObj = null;
    public XComponent source = null ;

    /**
    * Retrieves object relations.
    * @throws StatusException If one of relations not found.
    */
    @Override
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


