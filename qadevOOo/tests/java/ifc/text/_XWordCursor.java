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
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XWordCursor
 */
public class _XWordCursor extends MultiMethodTest {

    public XWordCursor oObj = null;     // oObj filled by MultiMethodTest

    /**
     * Moves the cursor to start of the text.
     */
    @Override
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

