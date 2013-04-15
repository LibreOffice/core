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
import util.XInstCreator;

import com.sun.star.text.XRelativeTextContentInsert;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Testing <code>com.sun.star.text.XRelativeTextContentInsert</code>
 * interface methods :
 * <ul>
 *  <li><code> insertTextContentBefore()</code></li>
 *  <li><code> insertTextContentAfter()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'PARA'</code> (of type <code>XInstCreator</code>):
 *   the creator which can create instances of
 *   <code>com.sun.star.text.Paragraph</code> service. </li>
 *  <li> <code>'XTEXTINFO'</code> (of type <code>XInstCreator</code>):
 *   the creator which can create instances of some text content
 *   service (objects which implement <code>XTextContent</code>).
 *  </li>
 * <ul> <p>
 *
 * Tested component <b>must implement</b> <code>XText</code>
 * interface for proper testing. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XRelativeTextContentInsert
 */
public class _XRelativeTextContentInsert extends MultiMethodTest {

    public XRelativeTextContentInsert oObj = null;
    public XTextContent content = null;

    /**
     * First an instance of <code>Paragraph</code> service created
     * using relation and inserted into text. Then an instance
     * of text content is created and inserted after the paragraph. <p>
     *
     * Has <b>OK</b> status if no exceptions occurred.
     */
    public void _insertTextContentAfter() {

        try {
            XInstCreator para = (XInstCreator)tEnv.getObjRelation( "PARA" );
            XInterface oInt = para.createInstance();
            XTextContent new_content = (XTextContent) oInt;
            XText theText = UnoRuntime.queryInterface(XText.class,oObj);
            XTextCursor oCursor = theText.createTextCursor();
            XInstCreator info = (XInstCreator)
                tEnv.getObjRelation( "XTEXTINFO" );
            oInt = info.createInstance();
            content = (XTextContent) oInt;
            theText.insertTextContent(oCursor, content, false);
            oObj.insertTextContentAfter(new_content,content);
            tRes.tested("insertTextContentAfter()",true);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred while checking "+
                "insertTextContentAfter()");
            ex.printStackTrace(log);
            tRes.tested("insertTextContentAfter()",false);
        }


    } // end _insertTextContentAfter()

    /**
     * An instance of text content is created using relation
     * and inserted before the paragraph which was added into
     * text in <code>insertTextContentAfter</code> method test. <p>
     *
     * Has <b>OK</b> status if no exceptions occurred. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> insertTextContentAfter() </code> : here the
     *  <code>Paragraph</code> instance is inserted. </li>
     * </ul>
     */
    public void _insertTextContentBefore() {
        requiredMethod("insertTextContentAfter()");
        try {
            XInstCreator para = (XInstCreator)tEnv.getObjRelation( "PARA" );
            XInterface oInt = para.createInstance();
            XTextContent new_content = (XTextContent) oInt;
            oObj.insertTextContentBefore(new_content,content);
            tRes.tested("insertTextContentBefore()",true);
        }
        catch (com.sun.star.lang.IllegalArgumentException ex) {
            log.println("Exception occurred while checking "+
                "insertTextContentBefore()");
            ex.printStackTrace(log);
            tRes.tested("insertTextContentBefore()",false);
        }


    } // end _insertTextContentBefore()

}

