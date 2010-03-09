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

import com.sun.star.text.XWordCursor;

/**
 * Testing <code>com.sun.star.text.XWordCursor</code>
 * interface methods :
 * <ul>
 *  <li><code> isStartOfWord()</code></li>
 *  <li><code> isEndOfWord()</code></li>
 *  <li><code> gotoNextWord()</code></li>
 *  <li><code> gotoPreviousWord()</code></li>
 *  <li><code> gotoEndOfWord()</code></li>
 *  <li><code> gotoStartOfWord()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XWordCursor
 */
public class _XWordCursor extends MultiMethodTest {

    public XWordCursor oObj = null;     // oObj filled by MultiMethodTest

    /**
     * Moves the cursor to start of the text.
     */
    public void before() {
        oObj.gotoStart(false);
    }

    /**
     * First moves the cursor to the next word to be sure that
     * at least one word is situated before. Then moves cursor
     * to the previous word and checks the value returned. <p>
     *
     * Has <b>OK</b> status if method returns <code>true</code>.
     */
    public void _gotoPreviousWord(){
        oObj.gotoNextWord(false);
        tRes.tested("gotoPreviousWord()", oObj.gotoPreviousWord(false) );
    }

    /**
     * First moves the cursor to the previous word to be sure that
     * at least one word is situated after. Then moves cursor
     * to the next word and checks the value returned. <p>
     *
     * Has <b>OK</b> status if method returns <code>true</code>.
     */
    public void _gotoNextWord(){
        oObj.gotoPreviousWord(false) ;
        tRes.tested("gotoNextWord()", oObj.gotoNextWord(false) );
    }

    /**
     * First moves the cursor to the start of the current word,
     * then to the end and checks the value returned. <p>
     *
     * Has <b>OK</b> status if method returns <code>true</code>.
     */
    public void _gotoEndOfWord(){
        oObj.gotoStart(false);
        tRes.tested("gotoEndOfWord()", oObj.gotoEndOfWord(false) );
    }

    /**
     * Move cursor to the start, then to the end. After that the
     * method is called and returned value is checked. <p>
     * Has <b>OK</b> status if the method returns <code>true</code>.
     */
    public void _isEndOfWord(){
        log.println("gotoStartOfWord() = " + oObj.gotoStartOfWord(false)) ;
        log.println("gotoEndOfWord() = " + oObj.gotoEndOfWord(false));

        tRes.tested("isEndOfWord()", oObj.isEndOfWord() );
    }

    /**
     * Move cursor to the end, then to the start. After that the
     * method is called and returned value is checked. <p>
     * Has <b>OK</b> status if the method returns <code>true</code>.
     */
    public void _isStartOfWord(){

        oObj.gotoEndOfWord(false);
        oObj.gotoStartOfWord(false);
        tRes.tested("isStartOfWord()", oObj.isStartOfWord() );
    }

    /**
     * First moves the cursor to the start of the current word,
     * then shifts it 2 symbols to the right. After that the
     * method is called and returned value is checked.<p>
     *
     * Has <b>OK</b> status if method returns <code>true</code>.
     */
    public void _gotoStartOfWord(){
        oObj.gotoStartOfWord(false);
        oObj.goRight((short) 2, false) ;
        tRes.tested("gotoStartOfWord()", oObj.gotoStartOfWord(false) );
    }

 }  // finish class _XWordCursor

