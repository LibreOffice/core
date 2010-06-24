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

