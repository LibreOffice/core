/*************************************************************************
 *
 *  $RCSfile: _XTextTable.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-09-08 11:21:46 $
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


