/*************************************************************************
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
 *************************************************************************/

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
        m_dataModel.addRow( 1, m_rowValues[0] );
        GridDataEvent event = m_listener.assertSingleRowInsertionEvent();
        m_listener.reset();
        assertEquals( "row insertion: wrong FirstRow (1)", 0, event.FirstRow );
        assertEquals( "row insertion: wrong LastRow (1)", 0, event.LastRow );
        impl_assertRowData( 0 );

        m_dataModel.addRow( 2, m_rowValues[1] );
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

        m_dataModel.addRows( new Object[] { "3", 4.0, "5" }, new Object[][] { m_rowValues[2], m_rowValues[3], m_rowValues[4] } );
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

        m_dataModel.addRows( new Object[] { 1, 2, 3, 4, 5 }, m_rowValues );
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
}
