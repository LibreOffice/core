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

import com.sun.star.document.XImporter;
import com.sun.star.lang.XComponent;

/**
* Testing <code>com.sun.star.document.XImporter</code>
* interface methods :
* <ul>
*  <li><code> setTargetDocument()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'TargetDocument'</code> (of type <code>XComponent</code>):
*   the target document to be passed to the method. </li>
* <ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.document.XImporter
*/
public class _XImporter extends MultiMethodTest {

    public XImporter oObj = null;

    /**
    * Retrieves relation and sets target document. <p>
    * Has <b> OK </b> status if no runtime exceptions occurred,
    * really this method tested when the whole import result
    * checked.
    */
    public void _setTargetDocument() {
        boolean res = true;
        try {
            XComponent the_doc = (XComponent)
                tEnv.getObjRelation("TargetDocument");
            oObj.setTargetDocument(the_doc);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception while checking 'setTargetDocument'");
            ex.printStackTrace(log);
            res = false;
        }

        tRes.tested("setTargetDocument()",res);

    }

}  // finish class _XImporter

