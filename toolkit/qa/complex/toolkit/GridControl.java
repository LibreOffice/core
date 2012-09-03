/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2011 Oracle and/or its affiliates.
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

package complex.toolkit;

import com.sun.star.awt.XControl;
import com.sun.star.awt.XControlContainer;
import com.sun.star.awt.XControlModel;
import com.sun.star.awt.XToolkit;
import com.sun.star.awt.grid.DefaultGridDataModel;
import com.sun.star.awt.grid.XGridColumn;
import com.sun.star.awt.grid.XGridColumnModel;
import com.sun.star.awt.grid.XGridDataModel;
import com.sun.star.awt.grid.XMutableGridDataModel;
import com.sun.star.awt.grid.XSortableMutableGridDataModel;
import com.sun.star.beans.Pair;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.ContainerEvent;
import com.sun.star.container.XContainerListener;
import com.sun.star.container.XNameContainer;
import com.sun.star.lang.EventObject;
import com.sun.star.lang.IndexOutOfBoundsException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XEventListener;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XComponentContext;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloneable;
import complex.toolkit.awtgrid.DummyColumn;
import complex.toolkit.awtgrid.TMutableGridDataModel;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Random;
import org.junit.AfterClass;
import org.junit.BeforeClass;
import org.junit.Test;
import static org.junit.Assert.*;
import org.openoffice.test.OfficeConnection;

/** is a unit test for the grid control related implementations
 * @author frank.schoenheit@sun.com
 */
public class GridControl
{
    // -----------------------------------------------------------------------------------------------------------------
    public GridControl()
    {
        m_context = m_connection.getComponentContext();
    }

    // -----------------------------------------------------------------------------------------------------------------
    private static void impl_dispose( final Object... i_components )
    {
        for ( int i=0; i<i_components.length; ++i )
        {
            if ( i_components[i] != null )
            {
                final XComponent component = UnoRuntime.queryInterface( XComponent.class, i_components[i] );
                component.dispose();
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_recreateGridModel() throws Exception
    {
        impl_dispose( m_gridControlModel, m_columnModel, m_dataModel );

        // create a grid control model, and ensure it has a proper data and column model already
        m_gridControlModel = UnoRuntime.queryInterface( XPropertySet.class,
            createInstance( "com.sun.star.awt.grid.UnoControlGridModel" ) );
        assertNotNull( "grid control model does not provide XPropertySet interface", m_gridControlModel );

        // ensure that the model has default column/data models
        m_columnModel = UnoRuntime.queryInterface( XGridColumnModel.class, m_gridControlModel.getPropertyValue( "ColumnModel" ) );
        assertNotNull( "the control model is expected to have an initial column model", m_columnModel );
        final XGridDataModel dataModel = UnoRuntime.queryInterface( XGridDataModel.class, m_gridControlModel.getPropertyValue( "GridDataModel" ) );
        assertNotNull( "the control model is expected to have an initial data model", dataModel );
        m_dataModel = UnoRuntime.queryInterface( XSortableMutableGridDataModel.class,
            dataModel );
        assertNotNull( "the out-of-the-box data model should be mutable and sortable", m_dataModel );
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void testGridControlCloning() throws Exception
    {
        impl_recreateGridModel();

        // give the test something to compare, actually
        XGridColumnModel columnModel = UnoRuntime.queryInterface( XGridColumnModel.class,
            m_gridControlModel.getPropertyValue( "ColumnModel" ) );
        columnModel.setDefaultColumns( 10 );

        // clone the grid model
        final XCloneable cloneable = UnoRuntime.queryInterface( XCloneable.class, m_gridControlModel );
        assertNotNull( "all UnoControlModel's are expected to be cloneable", cloneable );

        final XInterface clone = cloneable.createClone();
        final XPropertySet clonedProps = UnoRuntime.queryInterface( XPropertySet.class, clone );

        // TODO: check all those generic properties for equality

        // the data model and the column model should have been cloned, too
        // in particular, the clone should not share the sub models with the orignal
        final XMutableGridDataModel originalDataModel = UnoRuntime.queryInterface( XMutableGridDataModel.class,
            m_gridControlModel.getPropertyValue( "GridDataModel" ) );
        final XMutableGridDataModel clonedDataModel = UnoRuntime.queryInterface( XMutableGridDataModel.class,
            clonedProps.getPropertyValue( "GridDataModel" ) );
        assertFalse( "data model should not be shared after cloning", UnoRuntime.areSame( originalDataModel, clonedDataModel ) );
        impl_assertEquality( originalDataModel, clonedDataModel );

        final XGridColumnModel originalColumnModel = columnModel;
        final XGridColumnModel clonedColumnModel = UnoRuntime.queryInterface( XGridColumnModel.class,
            clonedProps.getPropertyValue( "ColumnModel" ) );
        assertFalse( "column model should not be shared after cloning", UnoRuntime.areSame( originalColumnModel, clonedColumnModel ) );
        impl_assertEquality( originalColumnModel, clonedColumnModel );
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void testDisposal() throws Exception
    {
        impl_recreateGridModel();

        final int columnCount = 3;
        m_columnModel.setDefaultColumns( columnCount );

        // add disposal listeners to all columns so far
        final XGridColumn[] columns = m_columnModel.getColumns();
        assertEquals( "creating default columns resulted in unexpected column count", columnCount, columns.length );
        final DisposeListener[] columnListeners = new DisposeListener[columnCount];
        for ( int i=0; i<columnCount; ++i )
            columnListeners[i] = new DisposeListener( columns[i] );

        // add another column, and check that upon removal, it is disposed
        final int newColumnIndex = m_columnModel.addColumn( m_columnModel.createColumn() );
        final DisposeListener columnListener = new DisposeListener( m_columnModel.getColumn( newColumnIndex ) );
        m_columnModel.removeColumn( newColumnIndex );
        assertTrue( "explicit column removal is expected to dispose the column", columnListener.isDisposed() );

        // by definition, the grid control model is the owner of both the column and the data model. So, setting
        // a new column/data model should implicitly dispose the old models
        final DisposeListener oldDataModelListener = new DisposeListener( m_dataModel );
        final DisposeListener oldColumnModelListener = new DisposeListener( m_columnModel );

        final Object newDataModel = createInstance( "com.sun.star.awt.grid.DefaultGridDataModel" );
        final Object newColumnModel = createInstance( "com.sun.star.awt.grid.DefaultGridColumnModel" );
        final DisposeListener newDataModelListener = new DisposeListener( newDataModel );
        final DisposeListener newColumnModelListener = new DisposeListener( newColumnModel );

        m_gridControlModel.setPropertyValue( "GridDataModel", newDataModel );
        assertTrue( "setting a new data model failed", impl_areSameInterface( newDataModel, m_gridControlModel.getPropertyValue( "GridDataModel" ) ) );
        m_gridControlModel.setPropertyValue( "ColumnModel", newColumnModel );
        assertTrue( "setting a new column model failed", impl_areSameInterface( newColumnModel, m_gridControlModel.getPropertyValue( "ColumnModel" ) ) );

        assertTrue( "old data model has not been disposed", oldDataModelListener.isDisposed() );
        assertTrue( "old column model has not been disposed", oldColumnModelListener.isDisposed() );
        for ( int i=0; i<columnCount; ++i )
            assertTrue( "column no. " + i + " has not been disposed", columnListeners[i].isDisposed() );

        // the same holds if the grid control model itself is disposed - it should dispose the depending models, too
        assertFalse( "new data model is already disposed - this is unexpected", newDataModelListener.isDisposed() );
        assertFalse( "new column model is already disposed - this is unexpected", newColumnModelListener.isDisposed() );
        impl_dispose( m_gridControlModel );
        assertTrue( "new data model is not disposed after disposing the grid column model", newDataModelListener.isDisposed() );
        assertTrue( "new column model is not disposed after disposing the grid column model", newColumnModelListener.isDisposed() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    /**
     * tests various aspects of the <code>XMutableGridDataModel</code> interface
     */
    @Test
    public void testMutableGridDataModel() throws Exception
    {
        impl_recreateGridModel();

        TMutableGridDataModel test = new TMutableGridDataModel( m_dataModel );
        test.testAddRow();
        test.testAddRows();
        test.testInsertRow();
        test.testInsertRows();
        test.testRemoveRow();
        test.testRemoveAllRows();
        test.testUpdateCellData();
        test.testUpdateRowData();
        test.testUpdateRowHeading();
        test.cleanup();

        // a somehwat less straight-forward test: the data model is expected to implicitly increase its column count
        // when you add a row which has more columns than currently known
        final XMutableGridDataModel dataModel = DefaultGridDataModel.create( m_context );
        dataModel.addRow( 0, new Object[] { 1 } );
        assertEquals( "unexpected column count after adding the most simple row", 1, dataModel.getColumnCount() );
        dataModel.addRow( 1, new Object[] { 1, 2 } );
        assertEquals( "implicit extension of the column count doesn't work", 2, dataModel.getColumnCount() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void testGridColumnModel() throws Exception
    {
        impl_recreateGridModel();

        ColumnModelListener listener = new ColumnModelListener();
        m_columnModel.addContainerListener( listener );

        // insert default columns into the previously empty model, ensure we get the right notifications
        final int defaultColumnsCount = 3;
        m_columnModel.setDefaultColumns( defaultColumnsCount );
        impl_assertColumnModelConsistency();
        List< ContainerEvent > events = listener.assertExclusiveInsertionEvents();
        listener.reset();
        assertEquals( "wrong number of events fired by setDefaulColumns", defaultColumnsCount, events.size() );
        for ( int i=0; i<defaultColumnsCount; ++i )
        {
            final ContainerEvent event = events.get(i);
            final int index = impl_assertInteger( event.Accessor );
            assertEquals( "unexpected Accessor value in insert notification", i, index );
            assertTrue( "wrong column object notified in insert notification",
                impl_areSameInterface( event.Element, m_columnModel.getColumn(i) ) );
        }

        // insert some more default columns, ensure that all previously existing columns are removed
        final int moreDefaultColumnsCount = 5;
        m_columnModel.setDefaultColumns( moreDefaultColumnsCount );
        impl_assertColumnModelConsistency();
        assertEquals( "setting default columns is expected to remove all previously existing columns",
            moreDefaultColumnsCount, m_columnModel.getColumnCount() );

        // in this situation, both removal and insertion events have been notified
        final List< ContainerEvent > removalEvents = listener.getRemovalEvents();
        final List< ContainerEvent > insertionEvents = listener.getInsertionEvents();
        listener.reset();

        // for the removal events, check the indexes
        assertEquals( "wrong number of columns removed (or notified) upon setting default columns",
            defaultColumnsCount, removalEvents.size() );
        for ( int i=0; i<removalEvents.size(); ++i )
        {
            final ContainerEvent event = removalEvents.get(i);
            final int removedIndex = impl_assertInteger( event.Accessor );

            // The implementation is allowed to remove the columns from the beginning, in which case the
            // index of the removed column must always be 0, since e.g. the second column has index 0
            // after the first column (which previously had index 0) had been removed.
            // Alternatively, the implementation is allowed to remove columns from the end, which means
            // that the column index given in the event is steadily increasing.
            assertTrue( "unexpected column removal event column index",
                ( removedIndex == 0 ) || ( removedIndex == removalEvents.size() - 1 - i ) );
        }

        // for the insertion events, check the indexes as well
        assertEquals( "wrong number of insertion events when setting default columns over existing columns",
            moreDefaultColumnsCount, insertionEvents.size() );
        for ( int i=0; i<insertionEvents.size(); ++i )
        {
            final ContainerEvent event = insertionEvents.get(i);
            final int index = impl_assertInteger( event.Accessor );
            assertEquals( i, index );
        }

        // okay, remove all those columns
        while ( m_columnModel.getColumnCount() != 0 )
        {
            final int columnCount = m_columnModel.getColumnCount();
            final int removeColumnIndex = m_randomGenerator.nextInt( columnCount );
            m_columnModel.removeColumn( removeColumnIndex );
            events = listener.assertExclusiveRemovalEvents();
            listener.reset();
            assertEquals( "removing a single column should notify a single event", 1, events.size() );
            final ContainerEvent event = events.get(0);
            final int removalIndex = impl_assertInteger( event.Accessor );
            assertEquals( "removing an arbitrary column does not notify the proper accessor",
                removeColumnIndex, removalIndex );
        }

        // calling addColumn with a column not created by the given model/implementatoion should not succeed
        boolean caughtExpected = false;
        try
        {
            m_columnModel.addColumn( new DummyColumn() );
        }
        catch( final com.sun.star.lang.IllegalArgumentException e )
        {
            assertTrue( impl_areSameInterface( e.Context, m_columnModel ) );
            caughtExpected = true;
        }
        assertTrue( "adding a dummy (self-implemented) grid column to the model should not succeed", caughtExpected );

        // adding a single column to the end should succeed, properly notify, and still be consistent
        final XGridColumn newColumn = m_columnModel.createColumn();
        m_columnModel.addColumn( newColumn );
        impl_assertColumnModelConsistency();
        events = listener.assertExclusiveInsertionEvents();
        listener.reset();
        assertEquals( "addColumn notifies the wrong number of insertion events", 1, events.size() );
        final int insertionIndex = impl_assertInteger( events.get(0).Accessor );
        assertEquals( insertionIndex, newColumn.getIndex() );
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void testDataModel() throws Exception
    {
        impl_recreateGridModel();

        // ensure that getCellData and getRowData have the same opinion on the data they deliver
        final Object[][] data = new Object[][] {
            new Object[] { 15, 17, 0 },
            new Object[] { 9, 8, 14 },
            new Object[] { 17, 2, 16 },
            new Object[] { 0, 7, 14 },
            new Object[] { 10, 16, 16 },
        };
        m_dataModel.addRows( new Object[ data.length ], data );

        for ( int row = 0; row < data.length; ++row )
        {
            assertArrayEquals( "getRowData delivers wrong data in row " + row, data[row], m_dataModel.getRowData( row ) );
            for ( int col = 0; col < data[row].length; ++col )
            {
                assertEquals( "getCellData delivers wrong data at position (" + col + ", " + row + ")",
                        data[row][col], m_dataModel.getCellData( col, row ) );
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void testSortableDataModel() throws Exception
    {
        impl_recreateGridModel();

        final int colCount = 3;
        final int rowCount = 10;
        // initialize with some data
        final Object[][] data = new Object[][] {
            new Object[] { 15, 17, 0 },
            new Object[] { 9, 8, 14 },
            new Object[] { 17, 2, 16 },
            new Object[] { 0, 7, 14 },
            new Object[] { 10, 16, 16 },
            new Object[] { 2, 8, 10 },
            new Object[] { 4, 8, 3 },
            new Object[] { 7, 9, 0 },
            new Object[] { 15, 6, 19 },
            new Object[] { 2, 14, 19 }
        };
        final Object[] rowHeadings = new Object[] {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9
        };
        // ensure consistency of the test data
        assertEquals( rowHeadings.length, rowCount );
        assertEquals( data.length, rowCount );
        for ( Object[] rowData : data )
            assertEquals( rowData.length, colCount );

        // add the test data
        m_dataModel.addRows( rowHeadings, data );
        assertEquals( rowCount, m_dataModel.getRowCount() );
        assertEquals( colCount, m_dataModel.getColumnCount() );

        // sort by each column
        for ( int colIndex = 0; colIndex < colCount; ++colIndex )
        {
            for ( boolean ascending : new boolean[] { true, false } )
            {
                m_dataModel.sortByColumn( colIndex, ascending );
                Pair<Integer,Boolean> currentSortOrder = m_dataModel.getCurrentSortOrder();
                assertEquals( "invalid current sort column (column " + colIndex + ")", currentSortOrder.First.intValue(), colIndex );
                assertEquals( "invalid current sort direction", currentSortOrder.Second.booleanValue(), ascending );

                /*for ( int i=0; i<rowCount; ++i )
                {
                    for ( int j=0; j<colCount; ++j )
                        System.out.print( m_dataModel.getCellData( j, i ).toString() + ", " );
                    System.out.println();
                }*/

                // verify the data is actually sorted by this column
                for ( int rowIndex = 0; rowIndex < rowCount - 1; ++rowIndex )
                {
                    final Object currentValue = m_dataModel.getCellData( colIndex, rowIndex );
                    final int currentIntValue = impl_assertInteger( currentValue );
                    final Object nextValue = m_dataModel.getCellData( colIndex, rowIndex + 1 );
                    final int nextIntValue = impl_assertInteger( nextValue );
                    assertTrue( "data in row " + rowIndex + " is actually not sorted " + ( ascending ? "ascending" : "descending" ),
                        ascending   ? currentIntValue <= nextIntValue
                                    : currentIntValue >= nextIntValue );

                    // ensure the data in the other columns, and the row headings, are sorted as well
                    final Object rowHeading = m_dataModel.getRowHeading( rowIndex );
                    final int unsortedRowIndex = impl_assertInteger( rowHeading );
                    for ( int innerColIndex = 0; innerColIndex < colCount; ++innerColIndex )
                    {
                        assertEquals( "sorted row " + rowIndex + ", unsorted row " + unsortedRowIndex + ", col " + innerColIndex +
                            ": wrong data",
                            data[unsortedRowIndex][innerColIndex], m_dataModel.getCellData( innerColIndex, rowIndex ) );
                    }
                }
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    @Test
    public void testModelViewInteraction() throws Exception
    {
        final List< Object > disposables = new ArrayList< Object >();
        try
        {
            // create a simple dialog model/control/peer trinity
            final XControlModel dialogModel = createInstance( XControlModel.class, "com.sun.star.awt.UnoControlDialogModel" );
            disposables.add( dialogModel );
            final XPropertySet dialogProps = UnoRuntime.queryInterface( XPropertySet.class, dialogModel );
            dialogProps.setPropertyValue( "Width", 200 );
            dialogProps.setPropertyValue( "Height", 100 );
            dialogProps.setPropertyValue( "Title", "Grid Control Unit Test" );
            final XControl dialogControl = createInstance( XControl.class, "com.sun.star.awt.UnoControlDialog" );
            disposables.add( dialogControl );
            dialogControl.setModel( dialogModel );
            dialogControl.createPeer( createInstance( XToolkit.class, "com.sun.star.awt.Toolkit" ), null );

            // insert a grid control model
            final XMultiServiceFactory controlModelFactory = UnoRuntime.queryInterface( XMultiServiceFactory.class,
                dialogModel );
            XPropertySet gridModelProps = UnoRuntime.queryInterface( XPropertySet.class,
                controlModelFactory.createInstance( "com.sun.star.awt.grid.UnoControlGridModel" ) );
            disposables.add( gridModelProps );
            gridModelProps.setPropertyValue( "PositionX", 6 );
            gridModelProps.setPropertyValue( "PositionY", 6 );
            gridModelProps.setPropertyValue( "Width", 188 );
            gridModelProps.setPropertyValue( "Height", 88 );
            final XNameContainer modelContainer = UnoRuntime.queryInterface( XNameContainer.class, dialogModel );
            modelContainer.insertByName( "grid", gridModelProps );

            // check the respective control has been created
            final XControlContainer controlContainer = UnoRuntime.queryInterface( XControlContainer.class, dialogControl );
            final XControl gridControl = controlContainer.getControl( "grid" );
            assertNotNull( "no grid control created in the dialog", gridControl );

            // in the current implementation (not sure this is a good idea at all), the control (more precise: the peer)
            // ensures that if there are no columns in the column model, but in the data model, then the column model
            // will implicitly have the needed columns added.
            // To ensure that clients which rely on this do not break in the future, check this here.
            final XMutableGridDataModel dataModel = UnoRuntime.queryInterface( XMutableGridDataModel.class,
                gridModelProps.getPropertyValue( "GridDataModel" ) );
            assertNotNull( dataModel );
            assertEquals( 0, dataModel.getColumnCount() );

            final XGridColumnModel columnModel = UnoRuntime.queryInterface( XGridColumnModel.class,
                gridModelProps.getPropertyValue( "ColumnModel" ) );
            assertNotNull( columnModel );
            assertEquals( 0, columnModel.getColumnCount() );

            dataModel.addRow( null, new Object[] { 1, 2, 3 } );
            assertEquals( 3, dataModel.getColumnCount() );
            assertEquals( 3, columnModel.getColumnCount() );
        }
        finally
        {
            impl_dispose( disposables.toArray());
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private int impl_assertInteger( final Object i_object )
    {
        assertTrue( i_object instanceof Integer );
        return ((Integer)i_object).intValue();
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_assertColumnModelConsistency() throws IndexOutOfBoundsException
    {
        for ( int col = 0; col < m_columnModel.getColumnCount(); ++col )
        {
            final XGridColumn column = m_columnModel.getColumn( col );
            assertNotNull( column );
            assertEquals( "column/model inconsistency: column " + col + " has a wrong index!", col, column.getIndex() );
        }

        final XGridColumn[] allColumns = m_columnModel.getColumns();
        assertEquals( "getColumns returns the wrong number of column objects",
            m_columnModel.getColumnCount(), allColumns.length );
        for ( int col = 0; col < m_columnModel.getColumnCount(); ++col )
        {
            assertTrue( "getColumns inconsistency", impl_areSameInterface( allColumns[col], m_columnModel.getColumn(col) ) );
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_assertEquality( final XGridDataModel i_reference, final XGridDataModel i_compare ) throws IndexOutOfBoundsException
    {
        assertNotNull( i_reference );
        assertNotNull( i_compare );

        assertEquals( "data model comparison: wrong column counts", i_reference.getColumnCount(), i_compare.getColumnCount() );
        assertEquals( "data model comparison: wrong row counts", i_reference.getRowCount(), i_compare.getRowCount() );

        for ( int row = 0; row < i_reference.getRowCount(); ++row )
        {
            assertEquals( "data model comparison: wrong row heading content in row " + row,
                    i_reference.getRowHeading( row ) );
            for ( int col = 0; col < i_reference.getRowCount(); ++col )
            {
                assertEquals( "data model comparison: wrong cell content in cell (" + col + ", " + row + ")",
                    i_reference.getCellData( col, row ) );
                assertEquals( "data model comparison: wrong tooltip content in cell (" + col + ", " + row + ")",
                    i_reference.getCellToolTip( col, row ) );
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_assertEquality( final XGridColumnModel i_reference, final XGridColumnModel i_compare ) throws IndexOutOfBoundsException
    {
        assertEquals( "column model comparison: wrong column counts", i_reference.getColumnCount(), i_compare.getColumnCount() );
        for ( int col = 0; col < i_reference.getColumnCount(); ++col )
        {
            final XGridColumn referenceColumn = i_reference.getColumn( col );
            final XGridColumn compareColumn = i_compare.getColumn( col );
            impl_assertEquality( referenceColumn, compareColumn );
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private void impl_assertEquality( final XGridColumn i_reference, final XGridColumn i_compare )
    {
        final Method[] methods = XGridColumn.class.getMethods();
        for ( int m=0; m<methods.length; ++m )
        {
            if ( !methods[m].getName().startsWith( "get" ) )
                continue;
            try
            {
                final Object referenceValue = methods[m].invoke( i_reference );
                final Object compareValue = methods[m].invoke( i_compare );
                assertEquals( "grid column comparison: column attribute '" + methods[m].getName().substring(3) + "' does not match",
                    referenceValue, compareValue );
            }
            catch ( java.lang.Exception ex )
            {
                fail( " could not retrieve object attributes: " + ex.toString() );
            }
        }
    }

    // -----------------------------------------------------------------------------------------------------------------
    private boolean impl_areSameInterface( final Object i_lhs, final Object i_rhs )
    {
        final XInterface lhs = UnoRuntime.queryInterface( XInterface.class, i_lhs );
        final XInterface rhs = UnoRuntime.queryInterface( XInterface.class, i_rhs );
        return UnoRuntime.areSame( lhs, rhs );
    }

    // -----------------------------------------------------------------------------------------------------------------
    @BeforeClass
    public static void setUpConnection() throws java.lang.Exception
    {
        System.out.println( "--------------------------------------------------------------------------------" );
        System.out.println( "starting class: " + GridControl.class.getName() );
        System.out.print( "connecting ... " );
        m_connection.setUp();
        System.out.println( "done.");

        final long seed = m_randomGenerator.nextLong();
        m_randomGenerator.setSeed( seed );
        System.out.println( "seeding random number generator with " + seed );
    }

    // -----------------------------------------------------------------------------------------------------------------
    @AfterClass
    public static void tearDownConnection()
        throws InterruptedException, com.sun.star.uno.Exception
    {
        System.out.println();
        System.out.println( "tearing down connection" );
        m_connection.tearDown();
        System.out.println( "finished class: " + GridControl.class.getName() );
        System.out.println( "--------------------------------------------------------------------------------" );
    }

    // -----------------------------------------------------------------------------------------------------------------
    public <T> T createInstance( Class<T> i_interfaceClass, final String i_serviceIndentifer ) throws Exception
    {
        return UnoRuntime.queryInterface( i_interfaceClass, createInstance( i_serviceIndentifer ) );
    }

    // -----------------------------------------------------------------------------------------------------------------
    private Object createInstance( final String i_serviceName ) throws Exception
    {
        Object instance = m_context.getServiceManager().createInstanceWithContext( i_serviceName, m_context );
        assertNotNull( "could not create an instance of '" + i_serviceName + "'", instance );
        return instance;
    }
    // -----------------------------------------------------------------------------------------------------------------
    private static final class DisposeListener implements XEventListener
    {
        DisposeListener( final Object i_component )
        {
            m_component = UnoRuntime.queryInterface( XComponent.class, i_component );
            assertNotNull( m_component );
            m_component.addEventListener( this );
        }

        public void disposing( EventObject i_event )
        {
            assertTrue( UnoRuntime.areSame( i_event.Source, m_component ) );
            m_isDisposed = true;
        }

        final boolean isDisposed() { return m_isDisposed; }

        private final XComponent    m_component;
        private boolean             m_isDisposed;
    }

    // -----------------------------------------------------------------------------------------------------------------
    private static final class ColumnModelListener implements XContainerListener
    {
        ColumnModelListener()
        {
        }

        public void elementInserted( ContainerEvent i_event )
        {
            m_insertionEvents.add( i_event );
        }

        public void elementRemoved( ContainerEvent i_event )
        {
            m_removalEvents.add( i_event );
        }

        public void elementReplaced( ContainerEvent i_event )
        {
            m_replacementEvents.add( i_event );
        }

        public void disposing( EventObject eo )
        {
        }

        private List< ContainerEvent > assertExclusiveInsertionEvents()
        {
            assertFalse( m_insertionEvents.isEmpty() );
            assertTrue( m_removalEvents.isEmpty() );
            assertTrue( m_replacementEvents.isEmpty() );
            return m_insertionEvents;
        }

        private List< ContainerEvent > assertExclusiveRemovalEvents()
        {
            assertTrue( m_insertionEvents.isEmpty() );
            assertFalse( m_removalEvents.isEmpty() );
            assertTrue( m_replacementEvents.isEmpty() );
            return m_removalEvents;
        }

        private void reset()
        {
            m_insertionEvents = new ArrayList< ContainerEvent >();
            m_removalEvents = new ArrayList< ContainerEvent >();
            m_replacementEvents = new ArrayList< ContainerEvent >();
        }

        private List< ContainerEvent > getInsertionEvents() { return m_insertionEvents; }
        private List< ContainerEvent > getRemovalEvents() { return m_removalEvents; }

        private List< ContainerEvent > m_insertionEvents = new ArrayList< ContainerEvent >();
        private List< ContainerEvent > m_removalEvents = new ArrayList< ContainerEvent >();
        private List< ContainerEvent > m_replacementEvents = new ArrayList< ContainerEvent >();
    }

    // -----------------------------------------------------------------------------------------------------------------
    private static final OfficeConnection m_connection = new OfficeConnection();
    private static Random m_randomGenerator = new Random();
    private final XComponentContext m_context;

    private XPropertySet                    m_gridControlModel;
    private XGridColumnModel                m_columnModel;
    private XSortableMutableGridDataModel   m_dataModel;
}
