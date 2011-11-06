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


package ifc.document;

import com.sun.star.beans.XPropertySet;
import com.sun.star.document.XDocumentInfo;
import com.sun.star.document.XDocumentInfoSupplier;
import com.sun.star.uno.UnoRuntime;
import lib.MultiMethodTest;

/**
 * test the XDocumentInfoSupplier interface.
 */
public class _XDocumentInfoSupplier extends MultiMethodTest {
    public XDocumentInfoSupplier oObj = null;

    public void _getDocumentInfo() {
        // returns a com.sun.star.document.DocumentInfo
        XDocumentInfo info = oObj.getDocumentInfo();
        XPropertySet xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, info);
        String title = null;
        try {
            title = (String)xPropertySet.getPropertyValue("Title");
        }
        catch(Exception e) {
            e.printStackTrace((java.io.PrintWriter)log);
            title = null;
        }
        log.println("Got document title: " + title);
        tRes.tested("getDocumentInfo()", title != null);
    }
}
