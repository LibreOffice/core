/*************************************************************************
 *
 *  $RCSfile: _XResultSet.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 10:54:07 $
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

package ifc.sdbc;

import lib.MultiMethodTest;

import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.uno.UnoRuntime;

/**
/**
* Testing <code>com.sun.star.sdbc.XResultSet</code>
* interface methods :
* <ul>
*  <li><code> next()</code></li>
*  <li><code> isBeforeFirst()</code></li>
*  <li><code> isAfterLast()</code></li>
*  <li><code> isFirst()</code></li>
*  <li><code> isLast()</code></li>
*  <li><code> beforeFirst()</code></li>
*  <li><code> afterLast()</code></li>
*  <li><code> first()</code></li>
*  <li><code> last()</code></li>
*  <li><code> getRow()</code></li>
*  <li><code> absolute()</code></li>
*  <li><code> relative()</code></li>
*  <li><code> previous()</code></li>
*  <li><code> refreshRow()</code></li>
*  <li><code> rowUpdated()</code></li>
*  <li><code> rowInserted()</code></li>
*  <li><code> rowDeleted()</code></li>
*  <li><code> getStatement()</code></li>
* </ul> <p>
* This test needs the following object relations :
* <ul>
*  <li> <code>'XResultSet.hasStatement'</code> (<b>optional</b> of type
* <code>Object</code>):
*  it the relation exists than <code>getStatement</code> method
*  must not return <code>null</code> </li>
* <ul> <p>
* Test places DB cursor to different positions and then checks
* its current position. <p>
* Test is <b> NOT </b> multithread compilant. <p>
* @see com.sun.star.sdbc.XResultSet
*/
public class _XResultSet extends MultiMethodTest {

    // oObj filled by MultiMethodTest
    public XResultSet oObj = null ;

    /**
     * Positions the cursor to the first row.
     * Forces method tests to be executed in definite order.
     */
    public void before() {
        try {
            oObj.last() ;
            log.println("Totally number of rows is " + oObj.getRow()) ;
            oObj.first() ;
        } catch (SQLException e) {
            log.println("Ignored exception :") ;
            e.printStackTrace(log);
        }

        executeMethod("isBeforeFirst()") ;
        executeMethod("isAfterLast()") ;
        executeMethod("isLast()") ;
        executeMethod("isFirst()") ;
        executeMethod("next()") ;
        executeMethod("previous()") ;
    }

    /**
    * Places the cursor before the first row. <p>
    * Has <b>OK</b> status if no exceptions were thrown.
    */
    public void _beforeFirst() {
        try {
            oObj.beforeFirst() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            tRes.tested("beforeFirst()", false) ;
            return ;
        }
        tRes.tested("beforeFirst()", true) ;
    }

    /**
    * The method is called immediatly after <code>beforeFirst</code>
    * method test. <p>
    * Has <b>OK</b> status if method returns <code>true</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> beforeFirst </code> : to position cursor before
    *   the first row. </li>
    * </ul>
    */
    public void _isBeforeFirst() {
        requiredMethod("beforeFirst()") ;

        boolean result = true ;

        try {
            result = oObj.isBeforeFirst() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("isBeforeFirst()", result) ;
    }

    /**
    * Places the cursor after the last row. <p>
    * Has <b>OK</b> status if no exceptions were thrown.
    */
    public void _afterLast() {
        try {
            oObj.afterLast() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            tRes.tested("afterLast()", false) ;
            return ;
        }
        tRes.tested("afterLast()", true) ;
    }

    /**
    * The method is called immediatly after <code>afterLast</code>
    * method test. <p>
    * Has <b>OK</b> status if method returns <code>true</code> <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> afterLast </code> : to position cursor after
    *   the last row. </li>
    * </ul>
    */
    public void _isAfterLast() {
        requiredMethod("afterLast()") ;

        boolean result = true ;

        try {
            result = oObj.isAfterLast() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("isAfterLast()", result) ;
    }

    /**
    * Places the cursor on the first row. <p>
    * Has <b>OK</b> status if no exceptions were thrown.
    */
    public void _first() {
        try {
            oObj.first() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            tRes.tested("first()", false) ;
            return ;
        }
        tRes.tested("first()", true) ;
    }

    /**
    * The method is called immediatly after <code>first</code>
    * method test. <p>
    * Has <b>OK</b> status if method returns <code>true</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> first </code> : to position cursor on
    *   the first row. </li>
    * </ul>
    */
    public void _isFirst() {
        requiredMethod("first()") ;

        boolean result = true ;

        try {
            result = oObj.isFirst() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("isFirst()", result) ;
    }

    /**
    * Places the cursor on the last row. <p>
    * Has <b>OK</b> status if no exceptions were thrown.
    */
    public void _last() {
        try {
            oObj.last() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            tRes.tested("last()", false) ;
            return ;
        }
        tRes.tested("last()", true) ;
    }

    /**
    * The method is called immediatly after <code>last</code>
    * method test. <p>
    * Has <b>OK</b> status if method returns <code>true</code>. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> last </code> : to position cursor on
    *   the last row. </li>
    * </ul>
    */
    public void _isLast() {
        requiredMethod("last()") ;
        boolean result = true ;

        try {
            result = oObj.isLast() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("isLast()", result) ;
    }

    /**
    * Places the cursor on the row number 1. <p>
    * Has <b>OK</b> status if no exceptions were thrown.
    */
    public void _absolute() {
        boolean result = true ;

        try {
            oObj.absolute(1) ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("absolute()", result) ;
    }

    /**
    * The method is called immediatly after <code>absolute</code>
    * method test. <p>
    * Has <b>OK</b> status if method returns 1. <p>
    * The following method tests are to be completed successfully before :
    * <ul>
    *  <li> <code> absolute </code> : to position cursor on
    *   the row number 1. </li>
    * </ul>
    */
    public void _getRow() {
        requiredMethod("absolute()");
        boolean result = true;

        try {
            result &= oObj.getRow() == 1;
        } catch (SQLException e) {
            log.println("Exception occured:");
            e.printStackTrace(log);
            result = false;
        }

        tRes.tested("getRow()", result);
    }

    /**
    * Positions the cursor to the next row. Current row
    * number is retrieved before and after method call. <p>
    * Has <b>OK</b> status if current row number increases
    * by 1 after method call.
    */
    public void _next() {
        boolean result = true ;

        try {
            int prevRow = oObj.getRow() ;
            oObj.next() ;

            log.println("Row was : " + prevRow + ", row is : " + oObj.getRow());
            result &= prevRow + 1 == oObj.getRow() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("next()", result) ;
    }

    /**
    * Positions the cursor to the previous row. Current row
    * number is retrieved before and after method call. <p>
    * Has <b>OK</b> status if current row number decreases
    * by 1 after method call.
    */
    public void _previous() {
        boolean result = true ;

        try {
            int prevRow = oObj.getRow() ;
            oObj.previous() ;

            log.println("Row was : " + prevRow + ", row is : " + oObj.getRow());
            result &= prevRow - 1 == oObj.getRow() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("previous()", result) ;
    }

    /**
    * Positions the cursor relatively by 2 rows forward.
    * Current row number is retrieved before and after method call. <p>
    * Has <b>OK</b> status if current row number increases
    * by 2 after method call.
    */
    public void _relative() {
        boolean result = true ;

        try {
            oObj.first() ;
            int prevRow = oObj.getRow() ;
            oObj.relative(2) ;

            log.println("Row was : " + prevRow + ", row is : " + oObj.getRow());

            result &= prevRow + 2 == oObj.getRow() ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            result = false ;
        }
        tRes.tested("relative()", result) ;
    }

    /**
    * If component supports XRow and XRowUpdate then:
    *   test saves current value of string field, updates string,
    *   calls refreshRow() and checks that value of
    *   string field was refetched from DB
    * else: just calls method.<p>
    * Has <b>OK</b> status if no exceptions were thrown and value after
    * refreshRow() equals to saved value.
    */
    public void _refreshRow() {
        XRowUpdate xRowUpdate = (XRowUpdate)
            UnoRuntime.queryInterface(XRowUpdate.class, oObj);
        XRow xRow = (XRow)UnoRuntime.queryInterface(XRow.class, oObj);

        if (xRowUpdate == null || xRow == null) {
            log.println("Test must be modified because XRow or XRowUpdate is't supported");
            log.println("Only call method");
            try {
                oObj.refreshRow() ;
                tRes.tested("refreshRow()", true) ;
            } catch (SQLException e) {
                log.println("Exception occured :") ;
                e.printStackTrace(log) ;
                tRes.tested("refreshRow()", false) ;
            }
        } else {
            log.println("Testing of refreshRow()...");
            try {
                String oldValue = xRow.getString(util.DBTools.TST_STRING);
                log.println("Old value: " + oldValue);
                xRowUpdate.updateString(util.DBTools.TST_STRING,
                    "Test method refreshRow");
                log.println("New value: "
                    + xRow.getString(util.DBTools.TST_STRING));
                oObj.refreshRow();
                String valAfterRefresh =
                    xRow.getString(util.DBTools.TST_STRING);
                log.println("Value after refresh: " + valAfterRefresh);
                tRes.tested("refreshRow()", valAfterRefresh.equals(oldValue));
            } catch(SQLException e) {
                log.println("Exception occured :");
                e.printStackTrace(log);
                tRes.tested("refreshRow()", false);
            }
        }
    }

    /**
    * Just the method is called. <p>
    * Has <b>OK</b> status if no exceptions were thrown.
    */
    public void _rowUpdated() {

        try {
            boolean res = oObj.rowUpdated() ;
            tRes.tested("rowUpdated()", true) ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            tRes.tested("rowUpdated()", false) ;
        }
    }

    /**
    * Just the method is called. <p>
    * Has <b>OK</b> status if no exceptions were thrown.
    */
    public void _rowInserted() {
        try {
            boolean res = oObj.rowInserted() ;
            tRes.tested("rowInserted()", true) ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            tRes.tested("rowInserted()", false) ;
        }
    }

    /**
    * Just the method is called. <p>
    * Has <b>OK</b> status if no exceptions were thrown.
    */
    public void _rowDeleted() {
        try {
            boolean res = oObj.rowDeleted() ;
            tRes.tested("rowDeleted()", true) ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            tRes.tested("rowDeleted()", false) ;
        }
    }

    /**
    * Just the method is called. <p>
    * Has <b>OK</b> status if the statement returned isn't null or
    * the relation exists that informs that statement absent (e.g. for
    * MetaData row set).
    */
    public void _getStatement() {
        try {
            boolean hasStatement =
                tEnv.getObjRelation("XResultSet.hasStatement") != null ;
            Object res = oObj.getStatement() ;
            tRes.tested("getStatement()",
                (hasStatement && res != null) || !hasStatement) ;
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
            tRes.tested("getStatement()", false) ;
        }
    }

    /**
     * Moves the cursor to the first row to avoid affection to
     * the following interfaces tests
     */
    public void after() {
        log.println("Finally moving cursor to the first row ...");
        try {
            oObj.first();
        } catch (SQLException e) {
            log.println("Exception occured :") ;
            e.printStackTrace(log) ;
        }
    }

}  // finish class _XResultSet

