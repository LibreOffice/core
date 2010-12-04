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
 *   the creator which can create instances of soem text content
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
            XText theText = (XText)
                UnoRuntime.queryInterface(XText.class,oObj);
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

