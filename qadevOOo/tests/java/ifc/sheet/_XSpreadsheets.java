/*************************************************************************
 *
 *  $RCSfile: _XSpreadsheets.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:06:28 $
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

package ifc.sheet;

import lib.MultiMethodTest;

import com.sun.star.sheet.XSpreadsheets;

/**
* Testing <code>com.sun.star.sheet.XSpreadsheets</code>
* interface methods :
* <ul>
*  <li><code> insertNewByName()</code></li>
*  <li><code> moveByName()</code></li>
*  <li><code> copyByName()</code></li>
* </ul> <p>
* Test is multithread compilant. <p>
* @see com.sun.star.sheet.XSpreadsheets
*/
public class _XSpreadsheets extends MultiMethodTest {

    public XSpreadsheets oObj = null;
    protected static int uniqCount = 0;
    protected int uniqNumber = 0;

    /**
    * Sets the unique number for the current test.
    */
    protected synchronized void before() {
        uniqNumber = uniqCount ++ ;
    }

    /**
    * Test inserts new sheet using the name returned by the method
    * <code>newName</code>, copies inserted sheet with the new name,
    * checks existence of the sheet with this name in collection and removes
    * the both sheets from the collection. <p>
    * Has <b> OK </b> status if the sheet with the name of the copy exists
    * in the collection and no exceptions were thrown. <p>
    */
    public void _copyByName() {
        boolean result = true;
        //first insert one that should be copied
        String iS = newName("copyFrom");
        log.println("Inserting sheet '" + iS + "'") ;
        oObj.insertNewByName(iS, (short)0);
        String[] eNames = oObj.getElementNames();
        String NewSheet = newName("copyTo");
        log.println( "Try to copy "+eNames[0]+" to "+NewSheet);
        oObj.copyByName(eNames[0], NewSheet, (short)0);
        result = oObj.hasByName(NewSheet);
        //remove all inserted sheets
        try {
            oObj.removeByName(NewSheet);
            oObj.removeByName(iS);
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.print("Can't remove sheet by name");
            e.printStackTrace(log);
            result = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.print("Can't remove sheet by name");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("copyByName()", result);
    } // finished _copyByName

    /**
    * Test inserts new sheet using the name returned by the method
    * <code>newName</code>, moves the inserted sheet to the new position
    * in collection, gets all element names in collection and checks the name
    * of the sheet in the new position. <p>
    * Has <b> OK </b> status if the sheet name in the new position is equal to
    * the name of the sheet that was moved. <p>
    */
    public void _moveByName() {
        //first insert one that should be moved
        String iS = newName("move");
        oObj.insertNewByName(iS, (short) 0);
        String[] eNames = oObj.getElementNames();
        String sheetToMove = eNames[0];
        log.println( "Try to move "+ sheetToMove);
        oObj.moveByName(sheetToMove,(short) 2);
        eNames = oObj.getElementNames();
        tRes.tested("moveByName()", sheetToMove.equals(eNames[1]));
    } // finished _moveByName

    /**
    * Test inserts new sheet using the name returned by the method
    * <code>newName</code>, checks the existence of the inserted sheet in
    * the collection, removes the sheet, tries to insert the sheet with the
    * bad name returned by method <code>badName()</code>. <p>
    * Has <b> OK </b> status if the inserted sheet exists in the collection
    * after first method call and if exception occured during the second call. <p>
    */
    public void _insertNewByName() {

        boolean result = false;

        String NewSheet = newName("insert");
        log.println( "Try to insert " + NewSheet);
        oObj.insertNewByName(NewSheet, (short) 0);
        result = oObj.hasByName( NewSheet );
        try {
            oObj.removeByName( NewSheet );
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.print("Can't remove sheet '" + NewSheet + "':");
            e.printStackTrace(log);
            result = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.print("Can't remove sheet '" + NewSheet + "':");
            e.printStackTrace(log);
            result = false;
        }

        try {
            NewSheet = badName();
            log.println( "Try to insert " + NewSheet);
            oObj.insertNewByName(NewSheet, (short) 0);
            log.println(
                "No Exception thrown while inserting sheet with invalid name");
            result &= false;
            oObj.removeByName( NewSheet );
        } catch (com.sun.star.uno.RuntimeException e) {
            log.println(
                "Expected exception occured during testing 'insertNewByName'");
            result &= true;
        } catch (com.sun.star.lang.WrappedTargetException e) {
            log.print("Can't remove sheet '" + NewSheet + "':");
            e.printStackTrace(log);
            result = false;
        } catch (com.sun.star.container.NoSuchElementException e) {
            log.print("Can't remove sheet '" + NewSheet + "':");
            e.printStackTrace(log);
            result = false;
        }

         tRes.tested("insertNewByName()", result);
    } // finished _insertByName

    /**
    * Method returns unique new name using passed prefix and unique number
    * of the current test.
    */
    public String newName(String prefix) {
        return prefix + uniqNumber ;
    } // finished newName

    /**
    * Method return bad name for a sheet using the name of the current thread.
    */
    public String badName() {
          return Thread.currentThread().getName();
    } // finished badName


} //finish class _XSpreadsheets

