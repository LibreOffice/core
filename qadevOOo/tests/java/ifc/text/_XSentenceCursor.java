/*************************************************************************
 *
 *  $RCSfile: _XSentenceCursor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:18:11 $
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

