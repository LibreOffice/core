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

import lib.MultiPropertyTest;

/**
* Testing <code>com.sun.star.document.LinkTarget</code>
* service properties :
* <ul>
*  <li><code> LinkDisplayName </code></li>
* </ul> <p>
* Properties testing is automated by <code>lib.MultiPropertyTest</code>.
* @see com.sun.star.document.LinkTarget
*/
public class _LinkTarget extends MultiPropertyTest {

/*
    public XPropertySet oObj = null;  // oObj filled by MultiMethodTest

    public void _LinkDisplayName() {
        // check if Service is available
        XServiceInfo xInfo = (XServiceInfo)
            UnoRuntime.queryInterface(XServiceInfo.class, oObj);
        String [] sa = xInfo.getSupportedServiceNames();

        for (int i = 0; i < sa.length; i++) {
            System.out.println(sa[i]);
        }

        if ( ! xInfo.supportsService("com.sun.star.document.LinkTarget")) {
            log.println("Service not available !!!!!!!!!!!!!");
            tRes.tested("Supported", false);
        }
        //the only property is read only
        try {
            String LDN = (String) oObj.getPropertyValue("LinkDisplayName");
            tRes.tested("LinkDisplayName",LDN!=null);
        }
        catch (Exception ex) {
            log.println("Exception occurred during LinkDisplayName");
            ex.printStackTrace(log);
            tRes.tested("LinkDisplayName",false);
        }

    }// EOF LinkDisplayName
*/

}

