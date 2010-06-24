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
* Test is <b> NOT </b> multithread compilant. <p>
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
     * Gets data flavour from system data type, which was gotten
     * from 'text/html' MIME type. <p>
     * Has <b>OK</b> status if DataFlavour returned has 'text/hrml' MIME
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

