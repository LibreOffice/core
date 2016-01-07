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

package ifc.datatransfer;

import lib.MultiMethodTest;

import com.sun.star.datatransfer.DataFlavor;
import com.sun.star.datatransfer.XDataFormatTranslator;
import com.sun.star.uno.Type;

/**
* Testing <code>com.sun.star.datatransfer.XDataFormatTranslator</code>
* interface methods :
* <ul>
*  <li><code> getSystemDataTypeFromDataFlavor()</code></li>
*  <li><code> getDataFlavorFromSystemDataType()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compliant. <p>
* @see com.sun.star.datatransfer.XDataFormatTranslator
*/
public class _XDataFormatTranslator extends MultiMethodTest {

    public XDataFormatTranslator oObj = null;

    /**
    * Get a system data type for 'text/html' MIME type. <p>
    * Has <b> OK </b> status if not <code>null</code> value returned.
    */
    public void _getSystemDataTypeFromDataFlavor() {
        DataFlavor df = new DataFlavor
            ("text/html","HTML-Documents", new Type());
        Object res = oObj.getSystemDataTypeFromDataFlavor(df);
        tRes.tested("getSystemDataTypeFromDataFlavor()",res != null);
    }

    /**
     * Gets data flavor from system data type, which was gotten
     * from 'text/html' MIME type. <p>
     * Has <b>OK</b> status if DataFlavour returned has 'text/html' MIME
     * type.
     */
    public void _getDataFlavorFromSystemDataType() {
        DataFlavor in = new DataFlavor
            ("text/html","HTML-Documents", new Type());
        Object res = oObj.getSystemDataTypeFromDataFlavor(in);
        DataFlavor out = oObj.getDataFlavorFromSystemDataType(res);
        tRes.tested("getDataFlavorFromSystemDataType()",
            out.MimeType.equals("text/html"));
    }

}

