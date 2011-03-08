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

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextRange;

/**
 * Testing <code>com.sun.star.text.XTextContent</code>
 * interface methods :
 * <ul>
 *  <li><code> attach()</code></li>
 *  <li><code> getAnchor()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'CONTENT'</code> <b>optional</b>
 *  (of type <code>XTextContent</code>):
 *   if this relation exists than it is used as the
 *   tested object. </li>
 *  <li> <code>'TEXT'</code> <b>optional</b>
 *  (of type <code>XText</code>):
 *   the relation must be specified if the 'CONTENT'
 *   relation exists. From this relation an anchor
 *   for <code>attach()</code> method is obtained.</li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextContent
 */
public class _XTextContent extends MultiMethodTest {
    public XTextContent oObj = null;
    public XTextRange oAnchor = null;

    /**
     * Tries to get the anchor of the text content
     * an XTextRange is returned. <p>
     * The test is OK if an not null text range is returned
     */
    public void _getAnchor() {
        log.println("getAnchor()");
        oAnchor = oObj.getAnchor();
        tRes.tested("getAnchor()", oAnchor != null ) ;

    } // end getAnchor()

    /**
     * Tries to attach the text content to the test range
     * gotten with getAnchor(). If relations are found
     * then they are are used for testing. <p>
     *
     * The test is OK if the method works without error.
     * @see #_getAnchor()
     */
    public void _attach() {
        requiredMethod("getAnchor()");
        try {
            XTextContent aContent = (XTextContent) tEnv.getObjRelation("CONTENT");
            XTextRange aRange = (XTextRange) tEnv.getObjRelation("RANGE");

            if ( aContent !=null) {
                aContent.attach(aRange);
            } else {
                oObj.attach(aRange);
            }
            tRes.tested("attach()", true ) ;
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            String noAttach = (String) tEnv.getObjRelation("NoAttach");
            if (noAttach != null) {
                log.println("Exception expected for "+noAttach);
                log.println("This Component doesn't support attach");
                tRes.tested("attach()",true);
            } else {
                ex.printStackTrace(log);
                tRes.tested("attach()",false);
            }
        } catch (com.sun.star.uno.RuntimeException re) {
            String noAttach = (String) tEnv.getObjRelation("NoAttach");
            if (noAttach != null) {
                log.println("Exception expected for "+noAttach);
                log.println("This Component doesn't support attach");
                tRes.tested("attach()",true);
            } else {
                re.printStackTrace(log);
                tRes.tested("attach()",false);
            }
        }
    }
}

