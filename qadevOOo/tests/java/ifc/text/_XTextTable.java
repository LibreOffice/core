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
import lib.Status;
import lib.StatusException;

import com.sun.star.text.XTextTable;

/**
 * Testing <code>com.sun.star.text.XTextTable</code>
 * interface methods :
 * <ul>
 *  <li><code> initialize()</code></li>
 *  <li><code> getRows()</code></li>
 *  <li><code> getColumns()</code></li>
 *  <li><code> getCellByName()</code></li>
 *  <li><code> getCellNames()</code></li>
 *  <li><code> createCursorByCellName()</code></li>
 * </ul> <p>
 * This test needs the following object relations :
 * <ul>
 *  <li> <code>'NROW'</code> : the number of rows in table
 *  </li>
 *  <li> <code>'NCOL'</code> : the number of columns in table
 *  </li>
 *
 * Test is <b> NOT </b> multithread compilant. <p>
 * @see com.sun.star.text.XTextTable
 */
public class _XTextTable extends MultiMethodTest {

    public XTextTable oObj = null;        // oObj filled by MultiMethodTest
    int nRow;
    int nCol;

    String cellNamesList[] = null ;

    protected void before() {
        Integer num_row = (Integer)tEnv.getObjRelation("NROW");
        if (num_row == null) {
            throw new StatusException
                (Status.failed("Couldn't get relation 'NROW'"));
        }
        Integer num_col = (Integer)tEnv.getObjRelation("NCOL");
        if (num_col == null) {
            throw new StatusException
                (Status.failed("Couldn't get relation 'NCOL'"));
        }
        nRow = num_row.intValue();
        nCol = num_col.intValue();
    }

    /**
     * The method is not called directly here, because it must
     * be called before being inserted to the document. <p>
     *
     * Always has <b> OK </b> status. <p>
     */
    public void _initialize() {

        // initialize()
        log.println( "test for initialize()" );
        tRes.tested( "initialize()", true);
    }

    /**
     * Test calls the method passing as cell name the first
     * element from names returned by <code>getCellNames</code>
     * method. <p>
     *
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getCellNames() </code> : its result used by test. </li>
     * </ul>
     */
    public void _createCursorByCellName(){
        requiredMethod("getCellNames()") ;

        // createCursorByCellName()
        log.println( "test for createCursorByCellName()" );
        tRes.tested( "createCursorByCellName()",
                    oObj.createCursorByCellName( cellNamesList[0] ) != null );
    }

    /**
     * Test calls the method passing as cell name the first
     * element from names returned by <code>getCellNames</code>
     * method. <p>
     *
     * Has <b> OK </b> status if the method returns not
     * <code>null</code> value.
     *
     * The following method tests are to be completed successfully before :
     * <ul>
     *  <li> <code> getCellNames() </code> : its result used by test. </li>
     * </ul>
     */
    public void _getCellByName(){
        requiredMethod("getCellNames()") ;

        // getCellByName()
        log.println( "test for getCellByName()" );
        tRes.tested( "getCellByName()",
            oObj.getCellByName( cellNamesList[0] ) != null );
    }

    /**
     * Obtains cell names of the table. <p>
     *
     * Has <b>OK</b> status if number of elements in the returned
     * array is equal to [row number] * [column number]
     * and if the first name is 'A1'.
     */
    public void _getCellNames(){
        // getCellNames()
        log.println( "test for getCellNames()" );
        cellNamesList = oObj.getCellNames();

        boolean result = cellNamesList.length == ( nRow * nCol ) ;
        result &= cellNamesList[0].equals( "A1" ) ;

        tRes.tested( "getCellNames()", result ) ;
    }

    /**
     * Obtains columns of the table. <p>
     *
     * Has <b>OK</b> status if the number of element of returned
     * collection is equal to real number of columns in the table.
     */
    public void _getColumns(){
        // getColumns()
        log.println( "test for getColumns()" );
        tRes.tested( "getColumns()", nCol == oObj.getColumns().getCount() );
    }

    /**
     * Obtains rows of the table. <p>
     *
     * Has <b>OK</b> status if the number of element of returned
     * collection is equal to real number of rows in the table.
     */
    public void _getRows(){
        // getRows()
        log.println( "test for getRows()" );
        tRes.tested( "getRows()", nRow == oObj.getRows().getCount() );
    }

}


