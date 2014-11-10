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
 * Test is <b> NOT </b> multithread compliant. <p>
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
     * First jumps to the first page to have at least one
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
     * First jumps to the last page to have at least one
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

