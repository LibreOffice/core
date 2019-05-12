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



package ifc.sdb;

import lib.MultiMethodTest;

import com.sun.star.container.XNameAccess;
import com.sun.star.sdb.XFormDocumentsSupplier;

/**
* <code>com.sun.star.sdb.XFormDocumentsSupplier</code> interface
* testing.
* @see com.sun.star.sdb.XFormDocumentsSupplier
*/
public class _XFormDocumentsSupplier extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XFormDocumentsSupplier oObj = null ;

    /**
    * Has OK status if not null returned. <p>
    */
    public void _getFormDocuments() {

        XNameAccess docs = oObj.getFormDocuments() ;

        String[] docNames = docs.getElementNames() ;
        if (docNames != null) {
            log.println("Totally " + docNames.length + " documents :") ;
            for (int i = 0; i < docNames.length; i++)
                log.println("  " + docNames[i]) ;
        }

        tRes.tested("getFormDocuments()", docNames != null) ;
    }

}  // finish class _XFormDocumentsSupplier


