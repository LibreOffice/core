/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: _XSimpleText.java,v $
 * $Revision: 1.4 $
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

import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XSimpleText;
import com.sun.star.text.XTextCursor;

/**
* Testing <code>com.sun.star.text.XSimpleText</code>
* interface methods :
* <ul>
*  <li><code> createTextCursor()</code></li>
*  <li><code> createTextCursorByRange()</code></li>
*  <li><code> insertString()</code></li>
*  <li><code> insertControlCharacter()</code></li>
* </ul> <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.text.XSimpleText
*/
public class _XSimpleText extends MultiMethodTest {

    XTextCursor oCursor = null;
    public XSimpleText oObj = null;

    /**
    * Creates text cursor. <p>
    * Has <b> OK </b> status if not null value returned. <p>
    */
    public void _createTextCursor() {

        log.println( "Testing createTextCursor()" );
        oCursor = oObj.createTextCursor();
        tRes.tested( "createTextCursor()", oCursor != null );
    }

    /**
    * Inserts a string at the cursor position.<p>
    * Has <b> OK </b> status if the whole result string has a string
    * inserted as its substring. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> createTextCursor() </code> : to have a cursor
    *   where text should be inserted. </li>
    * </ul>
    */
    public void _insertString() {
        requiredMethod("createTextCursor()");
        log.println( "Testing insertString" );
        String sStr = getInterfaceName() ;
        oObj.insertString( oCursor, sStr, false );
        String gStr = oObj.getText().getString() ;

        tRes.tested( "insertString()", gStr != null &&
            gStr.indexOf(sStr) >= 0) ;
    }

    /**
    * Inserts paragraph break character into text and then checks
    * if this character is present in the result string. <p>
    * Has <b> OK </b> status if the result string has
    * paragraph break character. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> createTextCursor </code> : to have a cursor object. </li>
    * </ul>
    */
    public void _insertControlCharacter() {
        boolean bOK = true;

        requiredMethod("createTextCursor()");
        log.println( "Testing insertControlCharacter()" );
        try {
            oObj.insertControlCharacter( oCursor,
                ControlCharacter.PARAGRAPH_BREAK, false);
            oObj.insertControlCharacter( oCursor,
                ControlCharacter.LINE_BREAK, false);
            oObj.insertString(oObj.createTextCursor(),"newLine",false);
        }
        catch(com.sun.star.lang.IllegalArgumentException e ) {
            // Some exception.FAILED
            Status.failed( e.toString() );
            bOK = false;
        }
        String gStr = oObj.getString() ;

        tRes.tested( "insertControlCharacter()", bOK && gStr != null &&
            gStr.indexOf("\n") > -1);
    }

    /**
    * Creates another text cursor using existing cursor's range. <p>
    * Has <b> OK </b> status if not null value returned. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> createTextCursor </code> : to have a cursor object. </li>
    * </ul>
    */
    public void _createTextCursorByRange() {

        requiredMethod("createTextCursor()");
        oCursor.gotoStart(false);
        log.println( "Testing createTextCursorByRange()" );
        XTextCursor oTCursor = oObj.createTextCursorByRange(oCursor);
        tRes.tested("createTextCursorByRange()", oTCursor != null) ;
    }
}  // finish class _XSimpleText

