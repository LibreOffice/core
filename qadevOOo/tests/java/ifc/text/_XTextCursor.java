/*************************************************************************
 *
 *  $RCSfile: _XTextCursor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:19:08 $
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

package ifc.text;

import lib.MultiMethodTest;

import com.sun.star.text.XTextCursor;


/**
 * Testing <code>com.sun.star.text.XTextCursor</code>
 * interface methods :
 * <ul>
 *  <li><code> collapseToStart()</code></li>
 *  <li><code> collapseToEnd()</code></li>
 *  <li><code> isCollapsed()</code></li>
 *  <li><code> goLeft()</code></li>
 *  <li><code> goRight()</code></li>
 *  <li><code> gotoStart()</code></li>
 *  <li><code> gotoEnd()</code></li>
 *  <li><code> gotoRange()</code></li>
 * </ul> <p>
 *
 * During this test the component text is changed,
 * that's why it must be stored before methods' tests,
 * and restored after. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextCursor
 */
public class _XTextCursor extends MultiMethodTest {

    public XTextCursor oObj = null;     // oObj filled by MultiMethodTest
    String oldText = null ;

    /**
     * Stores component's text.
     */
    public void before() {
        oObj.gotoStart(false);
        oObj.gotoEnd(true);
        oldText = oObj.getString() ;
    }

    /**
     * First some text is set (for component to has at least some
     * text), cursor is expanded to the whole text (to be not collapsed),
     * the <code>collapseToEnd</code> is called. Then current cursor
     * text is examined. <p>
     *
     * Has <b>OK</b> status if the current cursor text is an
     * empty string.
     */
    public void _collapseToEnd(){
        boolean bCol = false;

        oObj.setString("XTextCursor");
        oObj.gotoStart(false);
        oObj.gotoEnd(true);
        oObj.collapseToEnd();
        bCol = oObj.getString().equals("");
        tRes.tested("collapseToEnd()", bCol );
    }

    /**
     * First some text is set (for component to has at least some
     * text), cursor is expanded to the whole text (to be not collapsed),
     * the <code>collapseToStart</code> is called. Then current cursor
     * text is examined. <p>
     *
     * Has <b>OK</b> status if the current cursor text is an
     * empty string.
     */
    public void _collapseToStart(){
        boolean bCol = false;
        oObj.setString("XTextCursor");
        oObj.gotoStart(false);
        oObj.gotoEnd(true);

        oObj.collapseToStart();
        bCol = oObj.getString().equals("");
        tRes.tested("collapseToStart()", bCol );
    }

    /**
     * First the cursor is moved to the end of text (to have a space
     * for left cursor moving, and moves the cursor left by a number
     * of characters. <p>
     *
     * Has <b>OK</b> status if the method returns <code>true</code>,
     * and the current cursor string has the same length as number
     * of characters the cursor was moved by.
     */
    public void _goLeft(){
        boolean bLeft = false;
        short n = 5;

        oObj.gotoEnd(false);
        bLeft = oObj.goLeft(n, true);
        String gStr = oObj.getString() ;
        log.println("'" + gStr + "'") ;
        bLeft &= gStr.length() == n ;

        tRes.tested("goLeft()", bLeft );
    }

    /**
     * First the cursor is moved to the start of text (to have a space
     * for right cursor moving, and moves the cursor right by a number
     * of characters. <p>
     *
     * Has <b>OK</b> status if the method returns <code>true</code>,
     * and the current cursor string has the same length as number
     * of characters the cursor was moved by.
     */
    public void _goRight(){
        boolean bRight = false;
        short n = 5;

        oObj.gotoStart(false);
        bRight = oObj.goRight(n, true);

        String gStr = oObj.getString() ;
        log.println("'" + gStr + "'") ;
        bRight &= gStr.length() == n ;

        tRes.tested("goRight()", bRight );
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method <code>goRight()</code>
     * returns <code>false</code> (cursor can't move to the right).
     */
    public void _gotoEnd(){
        boolean bEnd = false;
        short n = 1;

        oObj.gotoEnd(false);
        bEnd = !oObj.goRight(n, false) ;

        tRes.tested("gotoEnd()", bEnd );
    }

    /**
     * First the whole text is set to a string, and cursor
     * is moved to the range situated at the start of the
     * text. <p>
     *
     * Has <b>OK</b> status if some characters to the right
     * of the current cursor position are the beginning of
     * the text.
     */
    public void _gotoRange(){
        boolean bRange = false;

        oObj.gotoStart(false);
        oObj.gotoEnd(true);
        oObj.setString("XTextCursor,XTextCursor");
        oObj.gotoRange(oObj.getStart(),false);
        oObj.goRight((short) 5, true);
        bRange = oObj.getString().equals("XText");

        if (!bRange) log.println("getString() returned '" +
            oObj.getString() + "'") ;

        tRes.tested("gotoRange()", bRange );
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method <code>goLeft()</code>
     * returns <code>false</code> (cursor can't move to the left).
     */
    public void _gotoStart(){
        boolean bStart = false;
        short n = 1;

        oObj.gotoStart(false);
        bStart = !oObj.goLeft(n, false) ;

        tRes.tested("gotoStart()", bStart );
    }

    /**
     * First the cusor is moved to start without expanding
     * (must be collapsed), and then it's expanded to the
     * whole text (must not be collapsed). <p>
     *
     * Has <b>OK</b> status if in the first case method
     * <code>isCollapsed</code> returns <code>true</code>,
     * and in the second <code>false</code>
     */
    public void _isCollapsed(){
        boolean bCol = false;

        oObj.gotoStart(false);
        bCol = oObj.isCollapsed();

        oObj.gotoEnd(true);
        bCol &= !oObj.isCollapsed() ;

        tRes.tested("isCollapsed()", bCol );
    }

    /**
     * Restores the text of the component to the
     * state it was before this interafce test.
     */
    public void after() {
        oObj.gotoStart(false);
        oObj.gotoEnd(true);
        oObj.setString(oldText) ;
    }

}  // finish class _XTextCursor

