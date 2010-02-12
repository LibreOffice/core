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
 * Test is <b> NOT </b> multithread compilant. <p>
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

