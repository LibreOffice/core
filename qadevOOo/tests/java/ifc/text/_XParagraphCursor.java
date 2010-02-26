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
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XParagraphCursor
 */
public class _XParagraphCursor extends MultiMethodTest {

    public XParagraphCursor oObj = null;     // oObj filled by MultiMethodTest
    XInstCreator info = null;               // instance creator

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

