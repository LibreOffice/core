/*************************************************************************
 *
 *  $RCSfile: _XDDELink.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:59:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

