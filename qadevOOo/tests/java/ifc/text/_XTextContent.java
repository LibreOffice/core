/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

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
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XTextContent
 */
public class _XTextContent extends MultiMethodTest {
    public XTextContent oObj = null;
    public XTextRange oAnchor = null;

    /**
     * Tries to get the anchor of the text content
     * an XTextRange is returned. <p>
     * The test is OK if a not null text range is returned
     */
    public void _getAnchor() {
        log.println("getAnchor()");
        oAnchor = oObj.getAnchor();
        tRes.tested("getAnchor()", oAnchor != null ) ;

    } // end getAnchor()

    /**
     * Tries to attach the text content to the test range
     * gotten with getAnchor(). If relations are found
     * then they are used for testing. <p>
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

