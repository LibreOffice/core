/*************************************************************************
 *
 *  $RCSfile: _XPageCursor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:17:18 $
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

import com.sun.star.text.ControlCharacter;
import com.sun.star.text.XPageCursor;
import com.sun.star.text.XText;
import com.sun.star.text.XTextCursor;
import com.sun.star.text.XTextDocument;


/**
 * Testing <code>com.sun.star.text.XPageCursor</code>
 * interface methods :
 * <ul>
 *  <li><code> jumpToFirstPage()</code></li>
 *  <li><code> jumpToLastPage()</code></li>
 *  <li><code> jumpToPage()</code></li>
 *  <li><code> getPage()</code></li>
 *  <li><code> jumpToNextPage()</code></li>
 *  <li><code> jumpToPreviousPage()</code></li>
 *  <li><code> jumpToEndOfPage()</code></li>
 *  <li><code> jumpToStartOfPage()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'TEXTDOC'</code> (of type <code>XTextDocument</code>):
 *   is used to insert text to document to fill at least two pages. </li>
 * <ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XPageCursor
 */
public class _XPageCursor extends MultiMethodTest {

    public XPageCursor oObj = null;     // oObj filled by MultiMethodTest
    short count = 0;

    /**
     * First adds text to the document using relation to fill at least two
     * pages. Then obtains the number of current page. <p>
     *
     * Has <b>OK</b> status if the returned value is positive.
     */
    public void _getPage(){
        log.println( "test for getPage()" );
        XTextDocument myText = (XTextDocument)tEnv.getObjRelation( "TEXTDOC" );
        XText aText = myText.getText();
        XTextCursor myCursor = aText.createTextCursor();

        try{
            for (int i = 0; i < 10; i++){
                for (int j = 0; j < 20; j++){
                    aText.insertString(myCursor, "The quick brown fox ",false);
                    aText.insertString(myCursor, "jumps over the lazy dog ",
                        false);
                }
                aText.insertControlCharacter(myCursor,
                    ControlCharacter.PARAGRAPH_BREAK, false);
            }
        } catch(com.sun.star.lang.IllegalArgumentException e) {
            log.println( "Exception :"  );
            e.printStackTrace(log);
        }

        count = oObj.getPage();
        tRes.tested("getPage()", count > 0 );
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _jumpToEndOfPage(){
        log.println( "test for jumpToEndOfPage()" );
        tRes.tested("jumpToEndOfPage()", oObj.jumpToEndOfPage());
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _jumpToFirstPage(){
        log.println( "test for jumpToFirstPage()" );
        tRes.tested("jumpToFirstPage()", oObj.jumpToFirstPage());
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _jumpToLastPage(){
        log.println( "test for jumpToLastPage()" );
        tRes.tested("jumpToLastPage()", oObj.jumpToLastPage());
    }

    /**
     * Firts jumps to the first page to have at least one
     * next page, then call the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _jumpToNextPage(){
        oObj.jumpToFirstPage() ;
        log.println( "test for jumpToNextPage()" );
        tRes.tested("jumpToNextPage()", oObj.jumpToNextPage());
    }

    /**
     * Tries to jump to the page with number 1. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _jumpToPage(){
        short n = 1;
        log.println( "test for jumpToPage()" );
        tRes.tested("jumpToPage()", oObj.jumpToPage(n));
    }

    /**
     * Firts jumps to the last page to have at least one
     * previous page, then call the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _jumpToPreviousPage(){
        log.println( "test for jumpToPreviousPage()" );
        oObj.jumpToLastPage();
        tRes.tested("jumpToPreviousPage()", oObj.jumpToPreviousPage());
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _jumpToStartOfPage(){
        log.println( "test for jumpToStartOfPage()" );
        tRes.tested("jumpToStartOfPage()", oObj.jumpToStartOfPage());
    }
}  // finish class _XPageCursor

