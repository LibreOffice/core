/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

