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

package complex.toolkit.awtgrid;

import com.sun.star.awt.grid.GridDataEvent;
import com.sun.star.awt.grid.XMutableGridDataModel;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.lang.IndexOutOfBoundsException;
import static org.junit.Assert.*;
import static complex.toolkit.Assert.*;

/** test for the <code>css.awt.grid.XMutableGridData</code> interface
 *
 * @author frank.schoenheit@oracle.com
 */
public class TMutableGridDataModel
{
    public TMutableGridDataModel( final XMutableGridDataModel i_dataModel )
    {
        m_dataModel = i_dataModel;

        m_listener = new GridDataListener();
        m_dataModel.addGridDataListener( m_listener );
    }

    /*
     * tests the XMutableGridDataModel.addRow method
     */
    public void testAddRow() throws IndexOutOfBoundsException
    {
        m_dataModel.addRow( m_rowHeadings[0], m_rowValues[0] );
        GridDataEvent event = m_listener.assertSingleRowInsertionEvent();
        m_listener.reset();
        assertEquals( "row insertion: wrong FirstRow (1)", 0, event.FirstRow );
        assertEquals( "row insertion: wrong LastRow (1)", 0, event.LastRow );
        impl_assertRowData( 0 );

        m_dataModel.addRow( m_rowHeadings[1], m_rowValues[1] );
        event = m_listener.assertSingleRowInsertionEvent();
        m_listener.reset();
        assertEquals( "row insertion: wrong FirstRow (2)", 1, event.FirstRow );
        assertEquals( "row insertion: wrong LastRow (2)", 1, event.LastRow );
        impl_assertRowData( 1 );
    }

    /**
     * tests the XMutableGridDataModel.addRows method
     */
    public void testAddRows() throws IndexOutOfBoundsException, IllegalArgumentException
    {
        assertEquals( "precondition not met: call this directly after testAddRow, please!", 2, m_dataModel.getRowCount() );

        m_dataModel.addRows(
                new Object[] { m_rowHeadings[2], m_rowHeadings[3], m_rowHeadings[4] },
                new Object[][] { m_rowValues[2], m_rowValues[3], m_rowValues[4] } );
        GridDataEvent event = m_listener.assertSingleRowInsertionEvent();
        assertEquals( "row insertion: wrong FirstRow (1)", 2, event.FirstRow );
        assertEquals( "row insertion: wrong LastRow (1)", 4, event.LastRow );
        m_listener.reset();

        assertEquals( "data model's row count is not adjusted when adding rows", m_rowValues.length, m_dataModel.getRowCount() );
        assertEquals( "data model's column count is not adjusted when adding rows", m_rowValues[0].length, m_dataModel.getColumnCount() );
        for ( int row=0; row<m_rowValues.length; ++row )
        {
            for ( int col=0; col<m_rowValues[row].length; ++col )
            {
                assertEquals( "added row values are not preserved",
                    m_rowValues[row][col], m_dataModel.getCellData( col, row ) );
            }
        }

        assertException( "addRows is expected to throw when invoked with different-sized arrays",
            m_dataModel, "addRows", new Object[] { new Object[0], new Object[1][2] }, IllegalArgumentException.class );
    }

    /**
     * tests the XMutableGridDataModel.insertRow method
     */
    public void testInsertRow() throws IndexOutOfBoundsException
    {
        int expectedRowCount = m_rowValues.length;
        assertEquals( "precondition not met: call this directly after testAddRows, please!", expectedRowCount, m_dataModel.getRowCount() );

        // inserting some row somewhere between the other rows
        final Object heading = "inbetweenRow";
        final Object[] inbetweenRow = new Object[] { "foo", "bar", 3, 4, 5 };
        final int insertionPos = 2;
        m_dataModel.insertRow( insertionPos, heading, inbetweenRow );
        ++expectedRowCount;
        assertEquals( "inserting a row is expected to increment the row count",
                expectedRowCount, m_dataModel.getRowCount() );

        final GridDataEvent event = m_listener.assertSingleRowInsertionEvent();
        assertEquals( "inserting a row results in wrong FirstRow being notified", insertionPos, event.FirstRow );
        assertEquals( "inserting a row results in wrong LastRow being notified", insertionPos, event.LastRow );
        m_listener.reset();

        for ( int row=0; row<expectedRowCount; ++row )
        {
            final Object[] actualRowData = m_dataModel.getRowData( row );
            final Object[] expectedRowData =
                ( row < insertionPos )
                ?   m_rowValues[ row ]
                :   ( row == insertionPos )
                    ?   inbetweenRow
                    :   m_rowValues[ row - 1 ];
            assertArrayEquals( "row number " + row + " has wrong content content after inserting a row",
                    expectedRowData, actualRowData );

            final Object actualHeading = m_dataModel.getRowHeading(row);
            final Object expectedHeading =
                ( row < insertionPos )
                ?   m_rowHeadings[ row ]
                :   ( row == insertionPos )
                    ?   heading
                    :   m_rowHeadings[ row - 1 ];
            assertEquals( "row " + row + " has a wrong heading after invoking insertRow",
                    expectedHeading, actualHeading );
        }

        // exceptions
        assertException( "inserting a row at a position > rowCount is expected to throw",
                m_dataModel, "insertRow",
                new Class[] { Integer.class, Object.class, Object[].class },
                new Object[] { expectedRowCount + 1, "", new Object[] { "1", 2, 3 } },
                IndexOutOfBoundsException.class );
        assertException( "inserting a row at a position < 0 is expected to throw",
                m_dataModel, "insertRow",
                new Class[] { Integer.class, Object.class, Object[].class },
                new Object[] { -1, "", new Object[] { "1", 2, 3 } },
                IndexOutOfBoundsException.class );

        // remove the row, to create the situation expected by the next test
        m_dataModel.removeRow( insertionPos );
        m_listener.reset();
    }

    /**
     * tests the XMutableGridDataModel.insertRows method
     */
    public void testInsertRows() throws IndexOutOfBoundsException, IllegalArgumentException
    {
        int expectedRowCount = m_rowValues.length;
        assertEquals( "precondition not met: call this directly after testInsertRow, please!", expectedRowCount, m_dataModel.getRowCount() );

        // inserting some rows somewhere between the other rows
        final int insertionPos = 3;
        final Object[] rowHeadings = new Object[] { "A", "B", "C" };
        final Object[][] rowData = new Object[][] {
            new Object[] { "A", "B", "C", "D", "E" },
            new Object[] { "J", "I", "H", "G", "F" },
            new Object[] { "K", "L", "M", "N", "O" }
        };
        final int insertedRowCount = rowData.length;
        assertEquals( "invalid test data", rowHeadings.length, insertedRowCount );

        m_dataModel.insertRows( insertionPos, rowHeadings, rowData );
        expectedRowCount += insertedRowCount;

        final GridDataEvent event = m_listener.assertSingleRowInsertionEvent();
        assertEquals( "inserting multiple rows results in wrong FirstRow being notified",
                insertionPos, event.FirstRow );
        assertEquals( "inserting multiple rows results in wrong LastRow being notified",
                insertionPos + insertedRowCount - 1, event.LastRow );
        m_listener.reset();

        for ( int row=0; row<expectedRowCount; ++row )
        {
            final Object[] actualRowData = m_dataModel.getRowData( row );
            final Object[] expectedRowData =
                ( row < insertionPos )
                ?   m_rowValues[ row ]
                :   ( row >= insertionPos ) && ( row < insertionPos + insertedRowCount )
                    ?   rowData[ row - insertionPos ]
                    :   m_rowValues[ row - insertedRowCount ];
            assertArrayEquals( "row number " + row + " has wrong content content after inserting multiple rows",
                    expectedRowData, actualRowData );

            final Object actualHeading = m_dataModel.getRowHeading(row);
            final Object expectedHeading =
                ( row < insertionPos )
                ?   m_rowHeadings[ row ]
                :   ( row >= insertionPos ) && ( row < insertionPos + insertedRowCount )
                    ?   rowHeadings[ row - insertionPos ]
                    :   m_rowHeadings[ row - insertedRowCount ];
            assertEquals( "row " + row + " has a wrong heading after invoking insertRows",
                    expectedHeading, actualHeading );
        }

        // exceptions
        assertException( "inserting multiple rows at a position > rowCount is expected to throw an IndexOutOfBoundsException",
                m_dataModel, "insertRows",
                new Class[] { Integer.class, Object[].class, Object[][].class },
                new Object[] { expectedRowCount + 1, new Object[0], new Object[][] { } },
                IndexOutOfBoundsException.class );
        assertException( "inserting multiple rows at a position < 0 is expected to throw an IndexOutOfBoundsException",
                m_dataModel, "insertRows",
                new Class[] { Integer.class, Object[].class, Object[][].class },
                new Object[] { -1, new Object[0], new Object[][] { } },
                IndexOutOfBoundsException.class );
        assertException( "inserting multiple rows with inconsistent array lengths is expected to throw an IllegalArgumentException",
                m_dataModel, "insertRows",
                new Class[] { Integer.class, Object[].class, Object[][].class },
                new Object[] { 0, new Object[0], new Object[][] { new Object[0] } },
                IllegalArgumentException.class );

        // remove the row, to create the situation expected by the next test
        for ( int i=0; i<insertedRowCount; ++i )
        {
            m_dataModel.removeRow( insertionPos );
            m_listener.reset();
        }
    }

    /**
     * tests the XMutableGridDataModel.removeRow method
     */
    public void testRemoveRow() throws IndexOutOfBoundsException
    {
        assertEquals( "precondition not met: call this directly after testAddRows, please!", m_rowValues.length, m_dataModel.getRowCount() );

        final int rowToRemove = 2;
        m_dataModel.removeRow( rowToRemove );
        GridDataEvent event = m_listener.assertSingleRowRemovalEvent();
        assertEquals( "incorrect notification of row removal (FirstRow)", rowToRemove, event.FirstRow );
        assertEquals( "incorrect notification of row removal (LastRow)", rowToRemove, event.LastRow );
        m_listener.reset();

        assertEquals( "data model's row count does not reflect the removed row", m_rowValues.length - 1, m_dataModel.getRowCount() );
        for ( int row = rowToRemove; row<m_rowValues.length-1; ++row )
        {
            for ( int col=0; col<m_rowValues[row].length; ++col )
            {
                assertEquals( "unexpected row values after removing a row (col: " + col + ", row: " + row + ")",
                    m_rowValues[row+1][col], m_dataModel.getCellData( col, row ) );
            }
        }

        assertException( "removeRow silently ignores an invalid index (1)",
            m_dataModel, "removeRow", new Object[] { -1 }, IndexOutOfBoundsException.class );
        assertException( "removeRow silently ignores an invalid index (2)",
            m_dataModel, "removeRow", new Object[] { m_dataModel.getRowCount() }, IndexOutOfBoundsException.class );
    }

    /**
     * tests the XMutableGridDataModel.removeAllRows method
     */
    public void testRemoveAllRows()
    {
        assertEquals( "precondition not met: call this directly after testRemoveRow, please!", m_rowValues.length - 1, m_dataModel.getRowCount() );

        m_dataModel.removeAllRows();
        final GridDataEvent event = m_listener.assertSingleRowRemovalEvent();
        if ( event.FirstRow != -1 )
        {   // notifying "-1" is allowed, this means "all rows affected", by definition
            assertEquals( "removeAllRows is not notifying properly (1)", 0, event.FirstRow );
            assertEquals( "removeAllRows is not notifying properly (2)", m_rowValues.length - 1, event.LastRow );
        }
        m_listener.reset();
    }

    /**
     * tests the XMutableGridDataModel.updateCellData method
     */
    public void testUpdateCellData() throws IndexOutOfBoundsException, IllegalArgumentException
    {
        assertEquals( "precondition not met: call this directly after testRemoveAllRows, please!", 0, m_dataModel.getRowCount() );

        m_dataModel.addRows( m_rowHeadings, m_rowValues );
        m_listener.assertSingleRowInsertionEvent();
        m_listener.reset();

        final Object[][] modifyValues = new Object[][] {
            new Object[] { 2, 1, "text" },
            new Object[] { 3, 0, null },
            new Object[] { 0, 4, new Double( 33.0 ) }
        };
        for ( int i = 0; i < modifyValues.length; ++i )
        {
            final int row = ((Integer)modifyValues[i][0]).intValue();
            final int col = ((Integer)modifyValues[i][1]).intValue();
            final Object value = modifyValues[i][2];
            m_dataModel.updateCellData( col, row, value );

            final GridDataEvent event = m_listener.assertSingleDataChangeEvent();
            assertEquals( "data change notification: FirstRow is invalid", row, event.FirstRow );
            assertEquals( "data change notification: LastRow is invalid", row, event.LastRow );
            assertEquals( "data change notification: FirstColumn is invalid", col, event.FirstColumn );
            assertEquals( "data change notification: LastColumn is invalid", col, event.LastColumn );
            m_listener.reset();

            assertEquals( "data change at (" + col + ", " + row + ") not successful", value, m_dataModel.getCellData( col, row ) );
        }

        assertException( "updateCellData silently ignores an invalid index (1)",
            m_dataModel, "updateCellData", new Class[] { int.class, int.class, Object.class },
            new Object[] { -1, -1, "text" }, IndexOutOfBoundsException.class );
        assertException( "updateCellData silently ignores an invalid index (2)",
            m_dataModel, "updateCellData", new Class[] { int.class, int.class, Object.class },
            new Object[] { 0, m_dataModel.getRowCount(), "text" }, IndexOutOfBoundsException.class );
        assertException( "updateCellData silently ignores an invalid index (3)",
            m_dataModel, "updateCellData", new Class[] { int.class, int.class, Object.class },
            new Object[] { m_dataModel.getColumnCount(), 0, "text" }, IndexOutOfBoundsException.class );
    }

    /**
     * tests the XMutableGridDataModel.updateRowData method
     */
    public void testUpdateRowData() throws IndexOutOfBoundsException, IllegalArgumentException
    {
        assertEquals( "precondition not met: call this directly after testRemoveAllRows, please!", m_rowValues.length, m_dataModel.getRowCount() );

        // get data from before the update
        final Object[][] preUpdateValues = impl_getCurrentData();

        // do the update
        final int[] colIndexes = new int[] {
            0, 3, 4
        };
        final Object[] values = new Object[] {
            13, null, 42.0
        };
        final int rowToUpdate = 2;
        m_dataModel.updateRowData( colIndexes, rowToUpdate, values );
        final GridDataEvent event = m_listener.assertSingleDataChangeEvent();
        assertEquals( "row update notification: FirstRow is invalid", rowToUpdate, event.FirstRow );
        assertEquals( "row update notification: LastRow is invalid", rowToUpdate, event.LastRow );
        assertEquals( "row update notification: FirstColumn is invalid", 0, event.FirstColumn );
        assertEquals( "row update notification: LastColumn is invalid", 4, event.LastColumn );
        m_listener.reset();

        // reflect the changes made in the pre-update data
        for ( int i=0; i<colIndexes.length; ++i )
        {
            preUpdateValues[rowToUpdate][colIndexes[i]] = values[i];
        }

        // get data from after the update
        final Object[][] postUpdateValues = impl_getCurrentData();

        // ensure both the manually updated pre-update data and the post-update data are identical
        assertArrayEquals( preUpdateValues, postUpdateValues );


        assertException( "updateRowData silently ignores an invalid index (1)",
            m_dataModel, "updateRowData", new Class[] { int[].class, int.class, Object[].class },
            new Object[] { new int[] { -1 }, 0, new Object[] { "text" } }, IndexOutOfBoundsException.class );
        assertException( "updateRowData silently ignores an invalid index (2)",
            m_dataModel, "updateRowData", new Class[] { int[].class, int.class, Object[].class },
            new Object[] { new int[] { 0 }, -1, new Object[] { "" } }, IndexOutOfBoundsException.class );
        assertException( "updateRowData silently ignores different-sized arrays",
            m_dataModel, "updateRowData", new Class[] { int[].class, int.class, Object[].class },
            new Object[] { new int[] { 0, 0 }, 0, new Object[] { "" } }, IllegalArgumentException.class );
    }

    /**
     * tests the XMutableGridDataModel.updateRowHeading method
     */
    public void testUpdateRowHeading() throws IndexOutOfBoundsException
    {
        assertEquals( "precondition not met: call this directly after testUpdateRowData, please!", m_rowValues.length, m_dataModel.getRowCount() );

        final Object[] preUpdateHeadings = impl_getCurrentRowHeadings();

        final int rowToUpdate = 2;
        final String valueToUpdate = "some text";
        m_dataModel.updateRowHeading( rowToUpdate, valueToUpdate );
        final GridDataEvent event = m_listener.assertSingleRowHeadingChangeEvent();
        assertEquals( "row heading update notification: FirstRow is invalid", rowToUpdate, event.FirstRow );
        assertEquals( "row heading update notification: FirstRow is invalid", rowToUpdate, event.LastRow );
        m_listener.reset();

        preUpdateHeadings[rowToUpdate] = valueToUpdate;

        final Object[] postUpdateHeadings = impl_getCurrentRowHeadings();
        assertArrayEquals( preUpdateHeadings, postUpdateHeadings );

        assertException( "updateRowHeading silently ignores an invalid index",
            m_dataModel, "updateRowHeading", new Class[] { int.class, Object.class },
            new Object[] { -1, "" }, IndexOutOfBoundsException.class );
    }

    public void cleanup()
    {
        m_dataModel.removeGridDataListener( m_listener );
    }

    private Object[][] impl_getCurrentData() throws IndexOutOfBoundsException
    {
        final int rowCount = m_dataModel.getRowCount();
        final int colCount = m_dataModel.getColumnCount();
        final Object[][] data = new Object[rowCount][colCount];
        for ( int row=0; row<rowCount; ++row )
        {
            for ( int col=0; col<colCount; ++col )
            {
                data[row][col] = m_dataModel.getCellData( col, row );
            }
        }
        return data;
    }

    private Object[] impl_getCurrentRowHeadings() throws IndexOutOfBoundsException
    {
        final int rowCount = m_dataModel.getRowCount();
        final Object[] headings = new Object[rowCount];
        for ( int row=0; row<rowCount; ++row )
            headings[row] = m_dataModel.getRowHeading( row );
        return headings;
    }

    private void impl_assertRowData( final int i_rowIndex ) throws IndexOutOfBoundsException
    {
        for ( int i=0; i<m_rowValues[i_rowIndex].length; ++i )
        {
            assertEquals( m_rowValues[i_rowIndex][i], m_dataModel.getCellData( i, i_rowIndex ) );
        }
    }

    private final XMutableGridDataModel m_dataModel;
    private final GridDataListener      m_listener;

    private final static Object[][] m_rowValues = new Object[][] {
            new Object[] { 1, 2, "3", 4, 5 },
            new Object[] { 2, 3, 4, "5", 6 },
            new Object[] { "3", 4, 5, 6, 7 },
            new Object[] { 4, 5, 6, 7, "8" },
            new Object[] { 5, "6", 7, 8, 9 },
        };

    private final static Object[] m_rowHeadings = new Object[] {
        "1", 2, 3.0, "4", (float)5.0
    };
}
