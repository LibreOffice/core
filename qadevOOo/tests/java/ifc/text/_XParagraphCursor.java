/*************************************************************************
 *
 *  $RCSfile: _XParagraphCursor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:17:38 $
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

