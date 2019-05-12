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



package ifc.sheet;

import lib.MultiMethodTest;
import lib.Status;
import lib.StatusException;

import com.sun.star.sheet.XDDELink;

/**
* Testing <code>com.sun.star.sheet.XDDELink</code>
* interface methods :
* <ul>
*  <li><code> getApplication()</code></li>
*  <li><code> getTopic()</code></li>
*  <li><code> getItem()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'APPLICATION'</code> (of type <code>String</code>):
*   to have application name </li>
*  <li> <code>'ITEM'</code> (of type <code>String</code>):
*   to have DDE item </li>
*  <li> <code>'TOPIC'</code> (of type <code>String</code>):
*   to have DDE topic </li>
* <ul> <p>
* @see com.sun.star.sheet.XDDELink
*/
public class _XDDELink extends MultiMethodTest {
    public XDDELink oObj = null;

    /**
    * Test calls the method and compares returned value to value obtained by
    * relation <code>'APPLICATION'</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    */
    public void _getApplication(){
        log.println("testing getApplication()");
        boolean bResult = false;

        String oAppl = (String)tEnv.getObjRelation("APPLICATION");
        if (oAppl == null) throw new StatusException(Status.failed
            ("Relation 'APPLICATION' not found"));

        bResult = oAppl.equals(oObj.getApplication());
        tRes.tested("getApplication()", bResult) ;
    }

    /**
    * Test calls the method and compares returned value to value obtained by
    * relation <code>'ITEM'</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    */
    public void _getItem(){
        log.println("testing getItem()");
        boolean bResult = false;
        String sItem = oObj.getItem();

        String oItem = (String)tEnv.getObjRelation("ITEM");
        if (oItem == null) throw new StatusException(Status.failed
            ("Relation 'ITEM' not found"));

        bResult = oItem.equals(sItem);
        tRes.tested("getItem()", bResult) ;
    }

    /**
    * Test calls the method and compares returned value to value obtained by
    * relation <code>'TOPIC'</code>. <p>
    * Has <b> OK </b> status if values are equal. <p>
    */
    public void _getTopic(){
        log.println("testing getTopic()");
        boolean bResult = false;
        String sTopic = oObj.getTopic();

        String oTopic = (String)tEnv.getObjRelation("TOPIC");
        if (oTopic == null) throw new StatusException(Status.failed
            ("Relation 'TOPIC' not found"));

        bResult = oTopic.equals(sTopic);
        tRes.tested("getTopic()", bResult) ;
    }
}

