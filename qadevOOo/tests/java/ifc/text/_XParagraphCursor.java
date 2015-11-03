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
import util.XInstCreator;

import com.sun.star.text.XParagraphCursor;

/**
 * Testing <code>com.sun.star.text.XParagraphCursor</code>
 * interface methods :
 * <ul>
 *  <li><code> isStartOfParagraph()</code></li>
 *  <li><code> isEndOfParagraph()</code></li>
 *  <li><code> gotoStartOfParagraph()</code></li>
 *  <li><code> gotoEndOfParagraph()</code></li>
 *  <li><code> gotoNextParagraph()</code></li>
 *  <li><code> gotoPreviousParagraph()</code></li>
 * </ul> <p>
 *
 * <b>Prerequisites :</b> the text must have at least
 * two paragraphs. <p>
 *
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XParagraphCursor
 */
public class _XParagraphCursor extends MultiMethodTest {

    public XParagraphCursor oObj = null;     // oObj filled by MultiMethodTest

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _gotoEndOfParagraph(){
        log.println( "test for gotoEndOfParagraph()" );
        if (oObj.isEndOfParagraph()) log.println("This is the end of the paragraph");
                                    else log.println("This isn't the end of the paragraph");
        log.println("gotoEndOfParagraph()");
        boolean result = oObj.gotoEndOfParagraph(false);
        tRes.tested("gotoEndOfParagraph()", result );
        if (oObj.isEndOfParagraph()) log.println("This is the end of the paragraph");
                                    else log.println("This isn't the end of the paragraph");
        if (!result) log.println("But 'gotoEndOfParagraph()' returns false");
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code>gotoPreviousParagraph()</code> : to be sure next paragraph
     *   exists. </li>
     * </ul>
     */
    public void _gotoNextParagraph(){
        requiredMethod( "gotoPreviousParagraph()" );
        log.println( "test for gotoNextParagraph()" );
        tRes.tested("gotoNextParagraph()", oObj.gotoNextParagraph(false) );
    }

    /**
     * First moves the cursor to the next paragraph to be sure
     * that previous paragraph exists and then calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _gotoPreviousParagraph(){
        //requiredMethod( "gotoNextParagraph()" );
        oObj.gotoNextParagraph(false);
        log.println( "test for gotoPreviousParagraph()" );
        tRes.tested("gotoPreviousParagraph()", oObj.gotoPreviousParagraph(false) );
    }

    /**
     * Test calls the method. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _gotoStartOfParagraph(){
        log.println( "test for gotoStartOfParagraph()" );
        tRes.tested("gotoStartOfParagraph()", oObj.gotoStartOfParagraph(false) );
    }

    /**
     * Moves the cursor to the end of paragraph then check if it is
     * at the end. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _isEndOfParagraph(){
        oObj.gotoEndOfParagraph(false);
        log.println( "test for isEndOfParagraph()" );
        tRes.tested("isEndOfParagraph()", oObj.isEndOfParagraph() );
    }

    /**
     * Moves the cursor to the start of paragraph then check if it is
     * at the start. <p>
     * Has <b> OK </b> status if the method returns
     * <code>true</code> value.
     */
    public void _isStartOfParagraph(){
        oObj.gotoStartOfParagraph(false);
        log.println( "test for isStartOfParagraph()" );
        tRes.tested("isStartOfParagraph()", oObj.isStartOfParagraph() );
    }

 }  // finish class _XParagraphCursor

