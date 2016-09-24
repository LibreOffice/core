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
 * Test is <b> NOT </b> multithread compliant. <p>
 * @see com.sun.star.text.XTextTable
 */
public class _XTextTable extends MultiMethodTest {

    public XTextTable oObj = null;        // oObj filled by MultiMethodTest
    int nRow;
    int nCol;

    String cellNamesList[] = null ;

    @Override
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


