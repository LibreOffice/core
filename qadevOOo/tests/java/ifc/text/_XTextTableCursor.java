/*************************************************************************
 *
 *  $RCSfile: _XTextTableCursor.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:21:58 $
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

import com.sun.star.table.XCell;
import com.sun.star.text.XTextTableCursor;


/**
 * Testing <code>com.sun.star.text.XTextTableCursor</code>
 * interface methods :
 * <ul>
 *  <li><code> getRangeName()</code></li>
 *  <li><code> gotoCellByName()</code></li>
 *  <li><code> goLeft()</code></li>
 *  <li><code> goRight()</code></li>
 *  <li><code> goUp()</code></li>
 *  <li><code> goDown()</code></li>
 *  <li><code> gotoStart()</code></li>
 *  <li><code> gotoEnd()</code></li>
 *  <li><code> mergeRange()</code></li>
 *  <li><code> splitRange()</code></li>
 * </ul> <p>
 *
 * <b>Prerequisites : </b> the table must have a size
 * 2x2 and current cursor position must be in the upper
 * left cell. <p>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 *
 * After test completion object environment has to be recreated
 * because after merging and splitting cells their names
 * differ from initial.
 *
 * @see com.sun.star.text.XTextTableCursor
 */
public class _XTextTableCursor extends MultiMethodTest {

    public XTextTableCursor oObj = null;    // oObj filled by MultiMethodTest

    XCell oCell = null;

    /**
     * Moves the cursor to upper-left cell and checks
     * its range name. <p>
     *
     * Has <b>OK</b> status if the name returned is 'A1'.
     */
    public void _getRangeName(){
        log.println( "test for getRangeName()" );
        oObj.gotoStart(false);
        String oCellName = oObj.getRangeName();
        log.println( "CellName = " + oCellName );

        tRes.tested("getRangeName()", oCellName.equals("A1") );
    }

    /**
     * Calls the method which moves the cursor down by 1 cell,
     * after that returns cursor to the old position. <p>
     *
     * Has <b>OK</b> status if the method returns
     * <code>true</code>.
     */
    public void _goDown(){
        boolean bDown = false;
        log.println( "test for goDown()" );
        short i = 1;

        bDown = oObj.goDown(i,false);

        tRes.tested("goDown()", bDown );
        oObj.goUp(i,false);
    }

    /**
     * Calls the method which moves the cursor to the right by 1 cell,
     * after that returns cursor to the old position. <p>
     *
     * Has <b>OK</b> status if the method returns
     * <code>true</code>.
     */
    public void _goRight(){
        boolean bRight = false;
        log.println( "test for goRight()" );
        short i = 1;

        bRight = oObj.goRight(i,false);

        tRes.tested("goRight()", bRight );
        oObj.goLeft(i,false);
    }

    /**
     * Calls the method which moves the cursor to the left by 1 cell,
     * after that returns cursor to the old position. <p>
     *
     * Has <b>OK</b> status if the method returns
     * <code>true</code>.
     */
    public void _goLeft(){
        boolean bLeft = false;
        log.println( "test for goLeft()" );
        short i = 1;

        oObj.goRight(i,false);
        bLeft = oObj.goLeft(i,false);

        tRes.tested("goLeft()", bLeft );
    }

    /**
     * Calls the method which moves the cursor up by 1 cell,
     * after that returns cursor to the old position. <p>
     *
     * Has <b>OK</b> status if the method returns
     * <code>true</code>.
     */
    public void _goUp(){
        boolean bUp = false;
        log.println( "test for goUp()" );
        short i = 1;

        oObj.gotoEnd(true);

        bUp = oObj.goUp(i,false);
        tRes.tested("goUp()", bUp );
    }

    /**
     * Moves the cursor to the cell with name 'B1', then
     * checks the current range name. <p>
     * Has <b>OK</b> status if the returned range name is
     * 'B1'.
     */
    public void _gotoCellByName(){
        log.println( "test for gotoCellByName()" );

        oObj.gotoCellByName("B1",false);
        String oCellName = oObj.getRangeName();

        tRes.tested("gotoCellByName()", oCellName.equals("B1") );
    }

    /**
     * Moves cursor to the start (upper-left cell). Then
     * checks the current range name. <p>
     *
     * Has <b>OK</b> status if current range name is 'A1'.
     */
    public void _gotoStart(){
        log.println( "test for gotoStart()" );

        oObj.gotoStart(false);
        String oCellName = oObj.getRangeName();

        tRes.tested("gotoStart()", oCellName.equals("A1") );
    }

    /**
     * Moves cursor to the end (lower-right cell). Then
     * checks the current range name. <p>
     *
     * Has <b>OK</b> status if current range name is 'B2'
     * (the table is assumed to be of size 2x2).
     */
    public void _gotoEnd(){
        log.println( "test for gotoEnd()" );

        oObj.gotoEnd(false);
        String oCellName = oObj.getRangeName();

        tRes.tested("gotoEnd()", oCellName.equals("B2") );
    }

    /**
     * Selects all cells in the table and merges them.
     * Finally move the cursor to the end and checks
     * current range name.<p>
     *
     * Has <b>OK</b> status if the end cell has a name
     * 'A1'.
     *
     * The following method tests are to be executed before :
     * <ul>
     *  <li> <code> getRangeName(), gotoStart(),  gotoEnd()
     *    goLeft(), goRight(), goUp(), goDown(), gotoCellByName()</code>
     *    : these methods must be completed before all cells of the
     *    table are merged into one cell </li>
     * </ul>
     */
    public void _mergeRange(){
        executeMethod("getRangeName()") ;
        executeMethod("gotoStart()") ;
        executeMethod("gotoEnd()") ;
        executeMethod("goLeft()") ;
        executeMethod("goRight()") ;
        executeMethod("goUp()") ;
        executeMethod("goDown()") ;
        executeMethod("gotoCellByName()") ;

        boolean bMerge = false;
        log.println( "test for mergeRange()" );

        oObj.gotoStart(false);
        oObj.gotoEnd(true);
        bMerge = oObj.mergeRange();

        oObj.gotoEnd(false);

        String curName = oObj.getRangeName() ;
        bMerge &= "A1".equals(curName) ;

        tRes.tested("mergeRange()", bMerge );
    }

    /**
     * First splits the cell horyzontally. Then the end cell
     * name is checked. Second split all cells vertically and
     * again the end cell name is checked<p>
     *
     * Has <b> OK </b> status if in the first case the end cell name
     * is not 'A1', and in the second case the end cell name is not
     * equal to the name gotten in the first case. <p>
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> mergeRange() </code> : to have one cell in a table
     *    which this test splits. </li>
     * </ul>
     */
    public void _splitRange(){
        requiredMethod("mergeRange()") ;

        boolean bSplit = true ;
        log.println( "test for splitRange" ) ;
        short i = 1 ;

        bSplit &= oObj.splitRange(i, true) ;

        oObj.gotoEnd(false);
        String horName = oObj.getRangeName() ;
        log.println("The end cell after horiz. split : " + horName) ;
        bSplit &= !"A1".equals(horName) ;

        oObj.gotoStart(false);
        oObj.gotoEnd(true);
        bSplit &= oObj.splitRange(i, false) ;

        oObj.gotoEnd(false);
        String vertName = oObj.getRangeName() ;
        log.println("The end cell after vert. split : " + vertName) ;
        bSplit &= !horName.equals(vertName) ;

        tRes.tested("splitRange()", bSplit ) ;
    }

    /**
     * Forces object environment recreation.
     */
    public void after() {
        disposeEnvironment() ;
    }

 }  // finish class _XTextTableCursor


