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

import com.sun.star.container.XIndexAccess;
import com.sun.star.text.XText;
import com.sun.star.text.XTextContent;
import com.sun.star.text.XTextCursor;
import com.sun.star.uno.XInterface;
import lib.StatusException;

/**
 * Testing <code>com.sun.star.text.XText</code>
 * interface methods :
 * <ul>
 *  <li><code> insertTextContent()</code></li>
 *  <li><code> removeTextContent()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'XTEXTINFO'</code> (of type <code>lib.XInstCreator</code>):
 *   creator which can create instances of <code>XTextContent</code>
 *   implementations. </li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XText
 */
public class _XText extends MultiMethodTest {

    public static XText oObj = null;        // oObj filled by MultiMethodTest
    XTextCursor oCursor = null;             // textcursor
    XInstCreator info = null;               // instance creator
    XInterface oInt = null;                 // instance to insert and remove

    /**
     * First an instance of text content is created using relation
     * and inserted into text. Then the number of contents is checked
     * using the relation. Second a <code>null</code> content is tried
     * to insert. <p>
     *
     * Has <b> OK </b> status if in the first case after inserting number
     * of content objects is greater than zero and in the second
     * case <code>IllegalArgumentException</code> is thrown. <p>
     */
    public void _insertTextContent() {
        boolean result = false;
        info = (XInstCreator)tEnv.getObjRelation( "XTEXTINFO" );
        oInt = info.createInstance();

        // write to log what we try next
        log.println( "test for createTextCursor()" );
        oCursor = oObj.createTextCursor();

        // write to log what we try next
        log.println( "test for insertTextContent()" );
        try {
            oObj.insertTextContent(oCursor, (XTextContent)oInt, false);
        }
        catch( com.sun.star.lang.IllegalArgumentException iaE ){
            throw new StatusException("Couldn't insert textcontent",iaE);
        }

        // get indexaccess to the tablecollection
        XIndexAccess xIA = info.getCollection();

        // this comparison works just because it has to be at least one
        // table at this point regardless which thread inserted it
        // there is although the possibility that the first threads call
        // failed, the second not and comparison happens after second threads
        // otherwise if something fails it should have thrown an exception
        //tRes.tested("insertTextContent()",    xIA.getCount() > 0 );

        if (xIA != null ) {
            result = (xIA.getCount()>0);
        } else {
            result = true;
        }

        if (!result) log.println("The TextContent wasn't inserted");


        // try to insert an invalid TextContent
        log.println( "test for insertTextContent" );
        try {
            oObj.insertTextContent(oCursor, null, false);
            log.println("The expected Exception doesn't occurred");
            result &= false;
        }
        catch( com.sun.star.lang.IllegalArgumentException iaE ){
            // Some exception.FAILED
            log.println("Expected Exception occurred");
            String msg = iaE.getMessage();
            if (msg.equals("")) {
                log.println("But there is not detailed message");
            } else {
                log.println("Detailed message: "+msg);
            }

            result &= true;
        }

        tRes.tested("insertTextContent()",    result );
    }


    /**
     * Removes the text content added before. <p>
     * Has <b> OK </b> status if the method successfully returns
     * and no exceptions were thrown. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> insertTextContent() </code> : inserts the content
     *  to be removed in this test. </li>
     * </ul>
     */
    public void _removeTextContent() {

        // leads to a method which should be called previously
        requiredMethod( "insertTextContent()" );

        // write to log what we try next
        log.println( "test for  removeTextContent" );
        try {
            oObj.removeTextContent( (XTextContent)oInt );
        }
        catch( com.sun.star.container.NoSuchElementException nseE ){
            // Some exception.FAILED
            log.println(nseE.toString());
            return;
        }

        // no exception occurred so it works
        tRes.tested( "removeTextContent()", true );

    }
}  // finish class _XText


