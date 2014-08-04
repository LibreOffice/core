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

import com.sun.star.text.XSentenceCursor;

/**
 * Testing <code>com.sun.star.text.XSentenceCursor</code>
 * interface methods :
 * <ul>
 *  <li><code> isStartOfSentence()</code></li>
 *  <li><code> isEndOfSentence()</code></li>
 *  <li><code> gotoNextSentence()</code></li>
 *  <li><code> gotoPreviousSentence()</code></li>
 *  <li><code> gotoStartOfSentence()</code></li>
 *  <li><code> gotoEndOfSentence()</code></li>
 * </ul> <p>
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XSentenceCursor
 */
public class _XSentenceCursor extends MultiMethodTest {

    public XSentenceCursor oObj = null;     // oObj filled by MultiMethodTest

    /**
     * First goes to next sentence (to be sure that previous exists),
     * then calls the method. <p>
     *
     * Has <b>OK</b> status if the method returns <code>true</code>.
     */
    public void _gotoPreviousSentence(){
        oObj.gotoNextSentence(false);
        oObj.gotoNextSentence(false);
        tRes.tested("gotoPreviousSentence()", oObj.gotoPreviousSentence(false) );
    }

    /**
     * First goes to previous sentence (to be sure that next exists),
     * then calls the method. <p>
     *
     * Has <b>OK</b> status if the method returns <code>true</code>.
     */
    public void _gotoNextSentence(){
        oObj.gotoPreviousSentence(false) ;
        oObj.gotoPreviousSentence(false) ;
        tRes.tested("gotoNextSentence()", oObj.gotoNextSentence(false) );
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> gotoPreviousSentence() </code> : to be sure the
     *   cursor is in range of some sentence. </li>
     * </ul>
     */
    public void _gotoEndOfSentence(){
        requiredMethod( "gotoPreviousSentence()" );
        tRes.tested("gotoEndOfSentence()", oObj.gotoEndOfSentence(false) );
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns <code>true</code>. <p>
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> gotoPreviousSentence() </code> : to be sure the
     *   cursor is in range of some sentence. </li>
     * </ul>
     */
    public void _gotoStartOfSentence(){
        requiredMethod( "gotoPreviousSentence()" );
        tRes.tested("gotoStartOfSentence()", oObj.gotoStartOfSentence(false) );
    }

    /**
     * First moves the cursor to the start of sentence and then calls
     * the method. <p>
     *
     * Has <b>OK</b> status if the method returns <code>true</code>.
     */
    public void _isStartOfSentence(){
        oObj.gotoStartOfSentence(false) ;
        tRes.tested("isStartOfSentence()", oObj.isStartOfSentence() );
    }

    /**
     * First moves the cursor to the end of sentence and then calls
     * the method. <p>
     *
     * Has <b>OK</b> status if the method returns <code>true</code>.
     */
    public void _isEndOfSentence(){
        oObj.gotoEndOfSentence(false) ;
        tRes.tested("isEndOfSentence()", oObj.isEndOfSentence() );
    }

 }  // finish class _XSentenceCursor

