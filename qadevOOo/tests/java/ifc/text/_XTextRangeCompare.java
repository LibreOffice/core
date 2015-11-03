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
import lib.Status;
import lib.StatusException;

import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;
import com.sun.star.text.XTextRange;
import com.sun.star.text.XTextRangeCompare;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;

/**
 * Testing <code>com.sun.star.text.XTextRangeCompare</code>
 * interface methods :
 * <ul>
 *  <li><code> compareRegionStarts()</code></li>
 *  <li><code> compareRegionEnds()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'TEXTDOC'</code> <b>optional</b>
 *  (must implement <code>XTextDocument</code>):
 *   can be used to obtain <code>Text</code> of the document from
 *   which cursors can be created.
 *   If the relation does not exist, the relation <code>TEXT</code>
 *   must be specified. </li>
 *  <li> <code>'TEXT'</code> <b>optional</b>
 *  (of type <code>XText</code>):
 *   used to create text cursor.
 *   If the relation does not exist, the relation <code>TEXTDOC</code>
 *   must be specified. </li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XTextRangeCompare
 */
public class _XTextRangeCompare extends MultiMethodTest {

    /**
     * the test object
     */
    public XTextRangeCompare oObj = null;
    String nameStr = null;
    XTextCursor cursor1 = null;
    XTextCursor cursor2 = null;
    XText oText = null;

    /**
     * Retrieves <code>XText</code> interface from relation 'TEXTDOC'
     * or from 'TEXT'.
     * @throws StatusException If neither 'TEXTDOC' nore 'TEXT'
     * relation exists.
     */
    @Override
    public void before() {
        nameStr = this.getClass().getName();

        XInterface oIfc = (XInterface)tEnv.getObjRelation("TEXTDOC");
        if (oIfc!=null) {
            XTextDocument oTDoc = UnoRuntime.queryInterface(
                                      XTextDocument.class, oIfc);
            oText = oTDoc.getText();
        }
        XText aText = (XText) tEnv.getObjRelation("TEXT");
        if (aText != null) {
            oText = aText;
        }

        if (oText == null) {
            throw new StatusException(Status.failed
                ("Neither 'TEXTDOC' nore 'TEXT' relation not found")) ;
        }
    }

    /**
     * One cursor is created and to its position a paragraph
     * inserted, then the fist five characters was selected.
     * A second cursor was created and the last 7 characteres
     * was selected.<p>
     *
     * Has <b>OK</b> status if the compare returns 1, i.e.
     * the second cursor end is before the first.
     */
    public void _compareRegionEnds() {
        boolean bResult = false;
        short n = 0;
        log.println( "testing compareRegionEnds()" );

        try{
            cursor1 = oText.createTextCursor();
            oText.insertString(cursor1, nameStr, false);

            cursor1.gotoStart(false);
            cursor1.goRight((short)5, true);
            cursor2 = oText.createTextCursor();
            cursor2.gotoEnd(false);
            cursor2.goLeft((short)7, true);

            log.println("hole text: '" + oText.getString() + "'");
            log.println("cursor1: '"+cursor1.getString() + "'");
            log.println("cursor2: '"+cursor2.getString() + "'");
            log.println("check: oObj.compareRegionStarts(cursor1, cursor2)");

            n = oObj.compareRegionEnds(cursor1, cursor2);

            log.println( "Result (short) : " + n );
        }catch(com.sun.star.lang.IllegalArgumentException e){
            log.println( "Exception: " + e);
            e.printStackTrace(log);
        }

        if (n == 1){bResult = true;}
        tRes.tested( "compareRegionEnds()", bResult );
    }

    /**
     * One cursor is created and to its position a paragraph
     * inserted, then the fist five characters was selected.
     * A second cursor was created and the last 7 characters
     * was selected.<p>
     *
     * Has <b>OK</b> status if the compare returns 1, i.e.
     * the second cursor start is before the first.
     */
    public void _compareRegionStarts() {
        boolean bResult = false;
        short n = 0;

        try{
            cursor1 = oText.createTextCursor();
            oText.insertString(cursor1, nameStr, false);

            cursor1.gotoStart(false);
            cursor1.goRight((short)5, true);
            cursor2 = oText.createTextCursor();
            cursor2.gotoEnd(false);
            cursor2.goLeft((short)7, true);

            log.println("hole text: '" + oText.getString() + "'");
            log.println("cursor1: '"+cursor1.getString() + "'");
            log.println("cursor2: '"+cursor2.getString() + "'");
            log.println("check: oObj.compareRegionStarts(cursor1, cursor2)");
            n = oObj.compareRegionStarts(cursor1, cursor2);

            log.println( "Result (short) : " + n );
        }catch(com.sun.star.lang.IllegalArgumentException e){
            log.println( "Exception: " + e);
            e.printStackTrace(log);
        }
        if (n == 1){bResult = true;}
        tRes.tested( "compareRegionStarts()", bResult );
    }

}

