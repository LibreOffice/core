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


