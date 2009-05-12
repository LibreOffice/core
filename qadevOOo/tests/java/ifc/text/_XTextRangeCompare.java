/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XTextRangeCompare.java,v $
 * $Revision: 1.5 $
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
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextRangeCompare
 */
public class _XTextRangeCompare extends MultiMethodTest {

    /**
     * the test object
     */
    public XTextRangeCompare oObj = null;


    String nameStr = null;

    XTextRange oRange = null;
    XTextCursor cursor1 = null;
    XTextCursor cursor2 = null;
    String startStr = null;
    String endStr = null;
    XText oText = null;

    /**
     * Retrieves <code>XText</code> interface from relation 'TEXTDOC'
     * or from 'TEXT'.
     * @throws StatusException If neither 'TEXTDOC' nore 'TEXT'
     * relation exists.
     */
    public void before() {
        nameStr = this.getClass().getName();

        XInterface oIfc = (XInterface)tEnv.getObjRelation("TEXTDOC");
        if (oIfc!=null) {
            XTextDocument oTDoc = (XTextDocument)UnoRuntime.queryInterface(
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

