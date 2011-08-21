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
* Test is <b> NOT </b> multithread compilant. <p>
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

