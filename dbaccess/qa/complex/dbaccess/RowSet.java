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
package complex.dbaccess;

import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XParametersSupplier;
import com.sun.star.sdb.XResultSetAccess;
import com.sun.star.sdb.XRowSetApproveBroadcaster;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XParameters;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XDeleteRows;
import com.sun.star.sdbcx.XRowLocate;
import com.sun.star.uno.UnoRuntime;

import connectivity.tools.CRMDatabase;
import connectivity.tools.DataSource;
import connectivity.tools.HsqlDatabase;
import connectivity.tools.sdb.Connection;
import java.lang.reflect.Method;
import java.util.Random;

// ---------- junit imports -----------------
import org.junit.Test;
import static org.junit.Assert.*;


public class RowSet extends TestCase
{

    static final int MAX_TABLE_ROWS = 100;
    static final int MAX_FETCH_ROWS = 10;
    private static final String NEXT = "next";
    private static final String TEST21 = "Test21";
    HsqlDatabase m_database;
    DataSource m_dataSource;
    XRowSet m_rowSet;
    XResultSet m_resultSet;
    XResultSetUpdate m_resultSetUpdate;
    XRow m_row;
    XRowLocate m_rowLocate;
    XPropertySet m_rowSetProperties;
    XParametersSupplier m_paramsSupplier;

    private final Object failedResultSetMovementStressGuard = new Object();
    private String failedResultSetMovementStressMessages = "";

    private class ResultSetMovementStress implements Runnable
    {

        private final XResultSet m_resultSet;
        private final XRow m_row;
        private final int m_id;

        private ResultSetMovementStress(XResultSet _resultSet, int _id) throws java.lang.Exception
        {
            m_resultSet = _resultSet;
            m_row = UnoRuntime.queryInterface( XRow.class, m_resultSet );
            m_id = _id;
        }

        public void run()
        {
	    int i=-1;
            try
            {
                m_resultSet.beforeFirst();
                for (i = 0; m_resultSet.next(); ++i)
                {
                    int pos = m_resultSet.getRow();
                    testPosition(m_resultSet, m_row, i + 1, "clone move(" + m_id + ")");
                    int pos2 = m_resultSet.getRow();
                    assertTrue("ResultSetMovementStress wrong position: " + i + " Pos1: " + pos + " Pos2: " + pos2, pos == pos2);
                }
            }
            catch (Exception e)
            {
                synchronized (failedResultSetMovementStressGuard) {
                    failedResultSetMovementStressMessages
                        += "ResultSetMovementStress(" + m_id + ") failed at i="
                        + i + ": " + e + "\n";
                }
            }
        }
    }

    private void createTestCase(boolean _defaultRowSet) throws Exception
    {
        if (m_database == null)
        {
            final CRMDatabase database = new CRMDatabase( getMSF(), false );
            m_database = database.getDatabase();
            m_dataSource = m_database.getDataSource();
        }

        createStructure();

        if (_defaultRowSet)
        {
            createRowSet("TEST1", CommandType.TABLE, true, true);
        }
    }


    /** creates a com.sun.star.sdb.RowSet to use during the test
     *  @param command
     *      the command to use for the RowSet
     *  @param commandType
     *      the command type to use for the RowSet
     *  @param execute
     *      determines whether the RowSet should be executed
     */
    private void createRowSet(String command, int commandType, boolean execute) throws com.sun.star.uno.Exception
    {
        createRowSet(command, commandType, execute, false);
    }


    /** creates a com.sun.star.sdb.RowSet to use during the test
     *  @param command
     *      the command to use for the RowSet
     *  @param commandType
     *      the command type to use for the RowSet
     *  @param execute
     *      determines whether the RowSet should be executed
     *  @param limitFetchSize
     *      determines whether the fetch size of the RowSet should be limited to MAX_FETCH_ROWS
     */
    private void createRowSet(String command, int commandType, boolean execute, boolean limitFetchSize) throws com.sun.star.uno.Exception
    {
        m_rowSet = UnoRuntime.queryInterface( XRowSet.class, getMSF().createInstance( "com.sun.star.sdb.RowSet" ) );
        final XPropertySet rowSetProperties = UnoRuntime.queryInterface( XPropertySet.class, m_rowSet );
        rowSetProperties.setPropertyValue("Command", command);
        rowSetProperties.setPropertyValue("CommandType", Integer.valueOf(commandType));
        rowSetProperties.setPropertyValue("ActiveConnection", m_database.defaultConnection().getXConnection());
        if (limitFetchSize)
        {
            rowSetProperties.setPropertyValue("FetchSize", Integer.valueOf(MAX_FETCH_ROWS));
        }

        m_resultSet = UnoRuntime.queryInterface( XResultSet.class, m_rowSet );
        m_resultSetUpdate = UnoRuntime.queryInterface( XResultSetUpdate.class, m_rowSet );
        m_row = UnoRuntime.queryInterface( XRow.class, m_rowSet );
        m_rowLocate = UnoRuntime.queryInterface( XRowLocate.class, m_resultSet );
        m_rowSetProperties = UnoRuntime.queryInterface( XPropertySet.class, m_rowSet );
        m_paramsSupplier = UnoRuntime.queryInterface( XParametersSupplier.class, m_rowSet );

        if (execute)
        {
            m_rowSet.execute();
        }
    }


    @Test
    public void testRowSet() throws java.lang.Exception
    {

        System.out.println("testing testRowSet");
        createTestCase(true);

        // sequential positioning
        m_resultSet.beforeFirst();
        testSequentialPositining(m_resultSet, m_row);

        // absolute positioning
        testAbsolutePositioning(m_resultSet, m_row);

        // position during modify
        testModifyPosition(m_resultSet, m_row);

        // 3rd test
        test3(createClone(), m_resultSet);
        // 4th test
        test4(m_resultSet);

        // concurrent (multi threaded) access to the row set and its clones
        testConcurrentAccess(m_resultSet);
    }


    XResultSet createClone() throws SQLException
    {
        final XResultSetAccess rowAcc = UnoRuntime.queryInterface( XResultSetAccess.class, m_rowSet );
        return rowAcc.createResultSet();
    }


    void createStructure() throws SQLException
    {
        m_database.executeSQL("DROP TABLE \"TEST1\" IF EXISTS");
        m_database.executeSQL("CREATE TABLE \"TEST1\" (\"ID\" integer not null primary key, \"col2\" varchar(50) )");

        final Connection connection = m_database.defaultConnection();
        final XPreparedStatement prep = connection.prepareStatement("INSERT INTO \"TEST1\" values (?,?)");
        final XParameters para = UnoRuntime.queryInterface( XParameters.class, prep );
        for (int i = 1; i <= MAX_TABLE_ROWS; ++i)
        {
            para.setInt(1, i);
            para.setString(2, "Test" + i);
            prep.executeUpdate();
        }

        connection.refreshTables();
    }


    void testPosition(XResultSet resultSet, XRow row, int expectedValue, String location) throws SQLException
    {
        final int val = row.getInt(1);
        final int pos = resultSet.getRow();
        assertTrue(location + ": value/position do not match: " + pos + " (pos) != " + val + " (val)", val == pos);
        assertTrue(location + ": value/position are not as expected: " + val + " (val) != " + expectedValue + " (expected)", val == expectedValue);
    }


    void testSequentialPositining(XResultSet _resultSet, XRow _row) throws com.sun.star.uno.Exception
    {
        // 1st test
        int i = 1;
        while (_resultSet.next())
        {
            testPosition(_resultSet, _row, i, "testSequentialPositining");
            ++i;
        }
    }


    void testAbsolutePositioning(XResultSet _resultSet, XRow _row) throws com.sun.star.uno.Exception
    {
        for (int i = 1; i <= MAX_FETCH_ROWS; ++i)
        {
            final int calcPos = (MAX_TABLE_ROWS % i) + 1;
            assertTrue("testAbsolutePositioning failed", _resultSet.absolute(calcPos));
            testPosition(_resultSet, _row, calcPos, "testAbsolutePositioning");
        }
    }


    void testModifyPosition(XResultSet _resultSet, XRow _row) throws com.sun.star.uno.Exception
    {
        final int testPos = 3;
        assertTrue("testModifyPosition wants at least " + (testPos+1) + " rows", MAX_FETCH_ROWS >= testPos+1);
        assertTrue("testModifyPosition failed on moving to row " + testPos, _resultSet.absolute(testPos));
        UnoRuntime.queryInterface( XRowUpdate.class, _row ).updateString(2, TEST21);
        testPosition(_resultSet, _row, testPos, "testModifyPosition");
        UnoRuntime.queryInterface( XResultSetUpdate.class, _resultSet ).cancelRowUpdates();
    }


    void test3(XResultSet clone, XResultSet _resultSet) throws com.sun.star.uno.Exception
    {
        final XRow _row = UnoRuntime.queryInterface( XRow.class, _resultSet );
        final XRow cloneRow = UnoRuntime.queryInterface( XRow.class, clone );
        for (int i = 1; i <= MAX_FETCH_ROWS; ++i)
        {
            final int calcPos = (MAX_TABLE_ROWS % i) + 1;
            if (clone.absolute(calcPos))
            {
                testPosition(clone, cloneRow, calcPos, "test3");
                testAbsolutePositioning(_resultSet, _row);
                testAbsolutePositioning(clone, cloneRow);
            }
        }
    }


    void test4(XResultSet _resultSet) throws com.sun.star.uno.Exception
    {
        final XRow _row = UnoRuntime.queryInterface( XRow.class, _resultSet );
        _resultSet.beforeFirst();

        for (int i = 1; i <= MAX_TABLE_ROWS; ++i)
        {
            _resultSet.next();
            final XResultSet clone = createClone();
            final XRow cloneRow = UnoRuntime.queryInterface( XRow.class, clone );
            final int calcPos = MAX_TABLE_ROWS - 1;
            if (calcPos != 0 && clone.absolute(calcPos))
            {
                testPosition(clone, cloneRow, calcPos, "test4: clone");
                testPosition(_resultSet, _row, i, "test4: rowset");
            }
        }
    }


    void testConcurrentAccess(XResultSet _resultSet) throws Exception
    {
        System.out.println("testing Thread");

        _resultSet.beforeFirst();

        final int numberOfThreads = 10;

        final Thread threads[] = new Thread[numberOfThreads];
        for (int i = 0; i < numberOfThreads; ++i)
        {
            threads[i] = new Thread(new ResultSetMovementStress(createClone(), i));
            System.out.println("starting thread " + (i + 1) + " of " + (numberOfThreads));
            threads[i].start();
        }

        for (int i = 0; i < numberOfThreads; ++i)
        {
            threads[i].join();
        }
        synchronized (failedResultSetMovementStressGuard) {
            assertEquals("", failedResultSetMovementStressMessages);
        }
    }


    @Test
    public void testRowSetEvents() throws java.lang.Exception
    {
        System.out.println("testing RowSet Events");
        createTestCase(true);

        // first we create our RowSet object
        final RowSetEventListener pRow = new RowSetEventListener();

        final XColumnsSupplier colSup = UnoRuntime.queryInterface( XColumnsSupplier.class, m_rowSet );
        final XPropertySet col = UnoRuntime.queryInterface( XPropertySet.class, colSup.getColumns().getByName( "ID" ) );
        col.addPropertyChangeListener("Value", pRow);
        m_rowSetProperties.addPropertyChangeListener("IsModified", pRow);
        m_rowSetProperties.addPropertyChangeListener("IsNew", pRow);
        m_rowSetProperties.addPropertyChangeListener("IsRowCountFinal", pRow);
        m_rowSetProperties.addPropertyChangeListener("RowCount", pRow);

        final XRowSetApproveBroadcaster xApBroad = UnoRuntime.queryInterface( XRowSetApproveBroadcaster.class, m_resultSet );
        xApBroad.addRowSetApproveListener(pRow);
        m_rowSet.addRowSetListener(pRow);

        // do some movements to check if we got all notifications
        final Class cResSet = Class.forName("com.sun.star.sdbc.XResultSet");
        final boolean moves[] = new boolean[9];
        for (int i = 0; i < moves.length; ++i)
        {
            moves[i] = false;
        }
        moves[RowSetEventListener.APPROVE_CURSOR_MOVE] = true;
        moves[RowSetEventListener.COLUMN_VALUE] = true;
        moves[RowSetEventListener.CURSOR_MOVED] = true;
        moves[RowSetEventListener.IS_ROW_COUNT_FINAL] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;

        testCursorMove(m_resultSet, cResSet.getMethod("afterLast", (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_ROW_COUNT_FINAL] = false;
        moves[RowSetEventListener.ROW_COUNT] = false;
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod("last", (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod("first", (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod("previous", (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        moves[RowSetEventListener.IS_MODIFIED] = true;
        final XRowUpdate updRow = UnoRuntime.queryInterface( XRowUpdate.class, m_resultSet );
        updRow.updateString(2, TEST21);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_MODIFIED] = false;
        updRow.updateString(2, m_row.getString(2));
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_MODIFIED] = false;
        final Class cupd = Class.forName("com.sun.star.sdbc.XResultSetUpdate");
        final XResultSetUpdate upd = UnoRuntime.queryInterface( XResultSetUpdate.class, m_resultSet );
        testCursorMove(upd, cupd.getMethod("moveToInsertRow", (Class[]) null), pRow, moves, null);

        updRow.updateInt(1, MAX_TABLE_ROWS + 2);
        updRow.updateString(2, "HHHH");
        moves[RowSetEventListener.APPROVE_CURSOR_MOVE] = false;
        moves[RowSetEventListener.CURSOR_MOVED] = false;
        moves[RowSetEventListener.IS_MODIFIED] = true;
        moves[RowSetEventListener.IS_NEW] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;
        moves[RowSetEventListener.APPROVE_ROW_CHANGE] = true;
        moves[RowSetEventListener.ROW_CHANGED] = true;
        testCursorMove(upd, cupd.getMethod("insertRow", (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_NEW] = false;
        moves[RowSetEventListener.ROW_COUNT] = false;
        m_resultSet.first();
        updRow.updateInt(1, MAX_TABLE_ROWS + 3);
        updRow.updateString(2, "__");
        testCursorMove(upd, cupd.getMethod("updateRow", (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_NEW] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;
        m_resultSet.first();
        testCursorMove(upd, cupd.getMethod("deleteRow", (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_NEW] = false;
        moves[RowSetEventListener.COLUMN_VALUE] = true;
        moves[RowSetEventListener.ROW_COUNT] = false;
        m_resultSet.first();
        updRow.updateString(2, TEST21);
        testCursorMove(m_resultSet, cResSet.getMethod("refreshRow", (Class[]) null), pRow, moves, null);

        m_resultSet.first();
        updRow.updateString(2, TEST21);
        testCursorMove(upd, cupd.getMethod("cancelRowUpdates", (Class[]) null), pRow, moves, null);

        for (int i = 0; i < moves.length; ++i)
        {
            moves[i] = false;
        }
        moves[RowSetEventListener.APPROVE_CURSOR_MOVE] = true;
        moves[RowSetEventListener.COLUMN_VALUE] = true;
        moves[RowSetEventListener.CURSOR_MOVED] = true;

        final Class cloc = Class.forName("com.sun.star.sdbcx.XRowLocate");
        m_resultSet.first();
        final Object bookmark = m_rowLocate.getBookmark();
        m_resultSet.next();
        final Object temp[] = new Object[1];
        temp[0] = bookmark;
        Class ctemp[] = new Class[1];
        ctemp[0] = Object.class;
        testCursorMove(m_rowLocate, cloc.getMethod("moveToBookmark", ctemp), pRow, moves, temp);

        final Object temp2[] = new Object[2];
        temp2[0] = bookmark;
        temp2[1] = Integer.valueOf(1);
        final Class ctemp2[] = new Class[2];
        ctemp2[0] = Object.class;
        ctemp2[1] = int.class;
        testCursorMove(m_rowLocate, cloc.getMethod("moveRelativeToBookmark", ctemp2), pRow, moves, temp2);

        for (int i = 0; i < moves.length; ++i)
        {
            moves[i] = false;
        }
        moves[RowSetEventListener.APPROVE_ROW_CHANGE] = true;
        moves[RowSetEventListener.ROW_CHANGED] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;
        final Class cdelRows = Class.forName("com.sun.star.sdbcx.XDeleteRows");
        ctemp[0] = Object[].class;
        final XDeleteRows delRows = UnoRuntime.queryInterface( XDeleteRows.class, m_resultSet );
        final Object bookmarks[] = new Object[5];
        m_resultSet.first();
        for (int i = 0; i < bookmarks.length; ++i)
        {
            m_resultSet.next();
            bookmarks[i] = m_rowLocate.getBookmark();
        }

        temp[0] = bookmarks;
        testCursorMove(delRows, cdelRows.getMethod("deleteRows", ctemp), pRow, moves, temp);

        // now destroy the RowSet
        final XComponent xComp = UnoRuntime.queryInterface( XComponent.class, m_resultSet );
        xComp.dispose();
    }


    private void testCursorMove(Object res, Method _method, RowSetEventListener _evt, boolean _must[], Object args[]) throws java.lang.Exception
    {
        _evt.clearCalling();
        _method.invoke(res, args);

        System.out.println("testing events for " + _method.getName());
        final int calling[] = _evt.getCalling();
        int pos = 1;
        assertTrue("Callings are not in the correct order for APPROVE_CURSOR_MOVE ",
                (!_must[RowSetEventListener.APPROVE_CURSOR_MOVE] || calling[RowSetEventListener.APPROVE_CURSOR_MOVE] == -1) || calling[RowSetEventListener.APPROVE_CURSOR_MOVE] == pos++);
        assertTrue("Callings are not in the correct order for APPROVE_ROW_CHANGE",
                (!_must[RowSetEventListener.APPROVE_ROW_CHANGE] || calling[RowSetEventListener.APPROVE_ROW_CHANGE] == -1) || calling[RowSetEventListener.APPROVE_ROW_CHANGE] == pos++);
        assertTrue("Callings are not in the correct order for COLUMN_VALUE",
                (!_must[RowSetEventListener.COLUMN_VALUE] || calling[RowSetEventListener.COLUMN_VALUE] == -1) || calling[RowSetEventListener.COLUMN_VALUE] == pos++);
        assertTrue("Callings are not in the correct order for CURSOR_MOVED",
                (!_must[RowSetEventListener.CURSOR_MOVED] || calling[RowSetEventListener.CURSOR_MOVED] == -1) || calling[RowSetEventListener.CURSOR_MOVED] == pos++);
        assertTrue("Callings are not in the correct order for ROW_CHANGED",
                (!_must[RowSetEventListener.ROW_CHANGED] || calling[RowSetEventListener.ROW_CHANGED] == -1) || calling[RowSetEventListener.ROW_CHANGED] == pos++);
        assertTrue("Callings are not in the correct order for IS_MODIFIED",
                (!_must[RowSetEventListener.IS_MODIFIED] || calling[RowSetEventListener.IS_MODIFIED] == -1) || calling[RowSetEventListener.IS_MODIFIED] == pos++);
        assertTrue("Callings are not in the correct order for IS_NEW",
                (!_must[RowSetEventListener.IS_NEW] || calling[RowSetEventListener.IS_NEW] == -1) || calling[RowSetEventListener.IS_NEW] == pos++);
        assertTrue("Callings are not in the correct order for ROW_COUNT",
                (!_must[RowSetEventListener.ROW_COUNT] || calling[RowSetEventListener.ROW_COUNT] == -1) || calling[RowSetEventListener.ROW_COUNT] == pos++);
        assertTrue("Callings are not in the correct order for IS_ROW_COUNT_FINAL",
                (!_must[RowSetEventListener.IS_ROW_COUNT_FINAL] || calling[RowSetEventListener.IS_ROW_COUNT_FINAL] == -1) || calling[RowSetEventListener.IS_ROW_COUNT_FINAL] == pos);

        _evt.clearCalling();
    }


    /** returns the current row count of the RowSet
     */
    private int currentRowCount() throws UnknownPropertyException, WrappedTargetException
    {
        final Integer rowCount = (Integer) m_rowSetProperties.getPropertyValue("RowCount");
        return rowCount.intValue();
    }


    /** positions the row set at an arbitrary position between 2 and (current row count - 1)
     */
    private int positionRandom() throws SQLException, UnknownPropertyException, WrappedTargetException
    {
        // note: obviously this should subtract 2 but actually subtract 3
        // because if we have just deleted the current row then
        // ORowSetBase::impl_getRowCount() will lie and currentRowCount()
        // returns 1 more than the actual number of rows and then
        // positionRandom() followed by deleteRow() deletes *last* row
        final int position = (new Random()).nextInt(currentRowCount() - 3) + 2;
        assertTrue("sub task failed: could not position to row no. " + (Integer.valueOf(position)).toString(),
                m_resultSet.absolute(position));
        return m_resultSet.getRow();
    }


    /** moves the result set to a random record between 2 and (current row count - 1), and deletes this record
     *
     *  After returning from this method, the row set is still positioned at the deleted record
     *  @return
     *      the number/position of the record which has been deleted
     */
    private int deleteRandom() throws SQLException, UnknownPropertyException, WrappedTargetException
    {
        // check if the current position and the row count in the result set is changed by a deletion (it should not)
        final int positionBefore = positionRandom();
        final int rowCountBefore = currentRowCount();

        m_resultSetUpdate.deleteRow();

        final int positionAfter = m_resultSet.getRow();
        final int rowCountAfter = currentRowCount();
        assertTrue("position changed during |deleteRow| (it should not)", positionAfter == positionBefore);
        assertTrue("row count changed with a |deleteRow| (it should not)", rowCountBefore == rowCountAfter);
        assertTrue("RowSet does not report the current row as deleted after |deleteRow|", m_resultSet.rowDeleted());

        return positionBefore;
    }


    @Test
    public void testDeleteBehavior() throws Exception
    {
        createTestCase(true);

        // ensure that all records are known
        m_resultSet.last();
        final int initialRowCount = currentRowCount();

        // delete a random row
        int deletedRow = deleteRandom();


        // asking for the bookmark of a deleted row should fail
        boolean caughtException = false;
        try
        {
            m_rowLocate.getBookmark();
        }
        catch (SQLException e)
        {
            caughtException = true;
        }
        assertTrue("asking for the bookmark of a deleted row should throw an exception", caughtException);


        // isXXX methods should return |false| on a deleted row
        assertTrue("one of the isFoo failed after |deleteRow|", !m_resultSet.isBeforeFirst() && !m_resultSet.isAfterLast() && !m_resultSet.isFirst() && !m_resultSet.isLast());
        // note that we can assume that isFirst / isLast also return |false|, since deleteRandom did
        // not position on the first or last record, but inbetween


        // check if moving away from this row in either direction yields the expected results
        assertTrue("|previous| after |deleteRow| failed", m_resultSet.previous());
        final int positionPrevious = m_resultSet.getRow();
        assertTrue("position after |previous| after |deleteRow| is not as expected", positionPrevious == deletedRow - 1);

        deletedRow = deleteRandom();
        assertTrue("|next| after |deleteRow| failed", m_resultSet.next());
        final int positionAfter = m_resultSet.getRow();
        assertTrue("position after |next| after |deleteRow| is not as expected", positionAfter == deletedRow);
        // since the deleted record "vanishes" as soon as the cursor is moved away from it, the absolute position does
        // not change with a |next| call here


        // check if the deleted rows really vanished after moving away from them
        assertTrue("row count did not change as expected after two deletions", initialRowCount - 2 == currentRowCount());


        // check if the deleted row vanishes after moving to the insertion row
        final int rowCountBefore = currentRowCount();
        final int deletedPos = deleteRandom();
        m_resultSetUpdate.moveToInsertRow();
        assertTrue("moving to the insertion row immediately after |deleteRow| does not adjust the row count", rowCountBefore == currentRowCount() + 1);

        m_resultSetUpdate.moveToCurrentRow();
        assertTrue("|moveToCurrentRow| after |deleteRow| + |moveToInsertRow| results in unexpected position",
                (m_resultSet.getRow() == deletedPos) && !m_resultSet.rowDeleted());

        // the same, but this time with deleting the first row (which is not covered by deleteRandom)
        m_resultSet.last();
        m_resultSetUpdate.deleteRow();
        m_resultSetUpdate.moveToInsertRow();
        m_resultSetUpdate.moveToCurrentRow();
        assertTrue("|last| + |deleteRow| + |moveToInsertRow| + |moveToCurrentRow| results in wrong state", m_resultSet.isAfterLast());


        // check if deleting a deleted row fails as expected
        deleteRandom();
        caughtException = false;
        try
        {
            m_resultSetUpdate.deleteRow();
        }
        catch (SQLException e)
        {
            caughtException = true;
        }
        assertTrue("deleting a deleted row succeeded - it shouldn't", caughtException);


        // check if deleteRows fails if it contains the bookmark of a previously-deleted row
        m_resultSet.first();
        final Object firstBookmark = m_rowLocate.getBookmark();
        positionRandom();
        final Object deleteBookmark = m_rowLocate.getBookmark();
        m_resultSetUpdate.deleteRow();
        final XDeleteRows multiDelete = UnoRuntime.queryInterface( XDeleteRows.class, m_resultSet );
        final int[] deleteSuccess = multiDelete.deleteRows(new Object[]
                {
                    firstBookmark, deleteBookmark
                });
        assertTrue("XDeleteRows::deleteRows with the bookmark of an already-deleted row failed",
                (deleteSuccess.length == 2) && (deleteSuccess[0] != 0) && (deleteSuccess[1] == 0));


        // check if refreshing a deleted row fails as expected
        deleteRandom();
        caughtException = false;
        try
        {
            m_resultSet.refreshRow();
        }
        catch (SQLException e)
        {
            caughtException = true;
        }
        assertTrue("refreshing a deleted row succeeded - it shouldn't", caughtException);


        // rowUpdated/rowDeleted
        deleteRandom();
        assertTrue("rowDeleted and/or rowUpdated are wrong on a deleted row", !m_resultSet.rowUpdated() && !m_resultSet.rowInserted());


        // updating values in a deleted row should fail
        deleteRandom();
        final XRowUpdate rowUpdated = UnoRuntime.queryInterface( XRowUpdate.class, m_resultSet );
        caughtException = false;
        try
        {
            rowUpdated.updateString(2, TEST21);
        }
        catch (SQLException e)
        {
            caughtException = true;
        }
        assertTrue("updating values in a deleted row should not succeed", caughtException);
    }


    /** checks whether deletions on the main RowSet properly interfere (or don't interfere) with the movement
     *  on a clone of the RowSet
     */
    @Test
    public void testCloneMovesPlusDeletions() throws Exception
    {
        createTestCase(true);
        // ensure that all records are known
        m_resultSet.last();

        final XResultSet clone = createClone();
        final XRowLocate cloneRowLocate = UnoRuntime.queryInterface( XRowLocate.class, clone );

        positionRandom();


        // move the clone to the same record as the RowSet, and delete this record
        cloneRowLocate.moveToBookmark(m_rowLocate.getBookmark());
        final int clonePosition = clone.getRow();
        m_resultSetUpdate.deleteRow();

        assertTrue("clone doesn't know that its current row has been deleted via the RowSet", clone.rowDeleted());
        assertTrue("clone's position changed somehow during deletion", clonePosition == clone.getRow());


        // move the row set away from the deleted record. This should still not touch the state of the clone
        m_resultSet.previous();

        assertTrue("clone doesn't know (anymore) that its current row has been deleted via the RowSet", clone.rowDeleted());
        assertTrue("clone's position changed somehow during deletion and RowSet-movement", clonePosition == clone.getRow());


        // move the clone away from the deleted record
        clone.next();
        assertTrue("clone still assumes that its row is deleted - but we already moved it", !clone.rowDeleted());


        // check whether deleting the extremes (first / last) work
        m_resultSet.first();
        cloneRowLocate.moveToBookmark(m_rowLocate.getBookmark());
        m_resultSetUpdate.deleteRow();
        clone.previous();
        assertTrue("deleting the first record left the clone in a strange state (after |previous|)", clone.isBeforeFirst());
        clone.next();
        assertTrue("deleting the first record left the clone in a strange state (after |previous| + |next|)", clone.isFirst());

        m_resultSet.last();
        cloneRowLocate.moveToBookmark(m_rowLocate.getBookmark());
        m_resultSetUpdate.deleteRow();
        clone.next();
        assertTrue("deleting the last record left the clone in a strange state (after |next|)", clone.isAfterLast());
        clone.previous();
        assertTrue("deleting the first record left the clone in a strange state (after |next| + |previous|)", clone.isLast());


        // check whether movements of the clone interfere with movements of the RowSet, if the latter is on a deleted row
        final int positionBefore = positionRandom();
        m_resultSetUpdate.deleteRow();
        assertTrue("|deleteRow|, but no |rowDeleted| (this should have been found much earlier!)", m_resultSet.rowDeleted());
        clone.beforeFirst();
        while (clone.next()) {}
        assertTrue("row set forgot that the current row is deleted", m_resultSet.rowDeleted());

        assertTrue("moving to the next record after |deleteRow| and clone moves failed", m_resultSet.next());
        assertTrue("wrong position after |deleteRow| and clone movement", !m_resultSet.isAfterLast() && !m_resultSet.isBeforeFirst());
        assertTrue("wrong absolute position after |deleteRow| and clone movement", m_resultSet.getRow() == positionBefore);
    }


    /** checks whether insertions on the main RowSet properly interfere (or don't interfere) with the movement
     *  on a clone of the RowSet
     */
    @Test
    public void testCloneMovesPlusInsertions() throws Exception
    {
        createTestCase(true);
        // ensure that all records are known
        m_rowSetProperties.setPropertyValue("FetchSize", Integer.valueOf(10));

        final XResultSet clone = createClone();
        final XRow cloneRow = UnoRuntime.queryInterface( XRow.class, clone );


        // first check the basic scenario without the |moveToInsertRow| |moveToCurrentRow|, to ensure that
        // really those are broken, if at all
        m_resultSet.last();
        clone.first();
        clone.absolute(11);
        clone.first();

        final int rowValue1 = m_row.getInt(1);
        final int rowPos = m_resultSet.getRow();
        final int rowValue2 = m_row.getInt(1);
        assertTrue("repeated query for the same column value delivers different values (" + rowValue1 + " and " + rowValue2 + ") on row: " + rowPos,
                rowValue1 == rowValue2);

        testPosition(clone, cloneRow, 1, "mixed clone/rowset move: clone check");
        testPosition(m_resultSet, m_row, MAX_TABLE_ROWS, "mixed clone/rowset move: rowset check");


        // now the complete scenario
        m_resultSet.last();
        m_resultSetUpdate.moveToInsertRow();
        clone.first();
        clone.absolute(11);
        clone.first();
        m_resultSetUpdate.moveToCurrentRow();

        testPosition(clone, cloneRow, 1, "mixed clone/rowset move/insertion: clone check");
        testPosition(m_resultSet, m_row, 100, "mixed clone/rowset move/insertion: rowset check");
    }


    private void testTableParameters() throws com.sun.star.uno.Exception
    {
        // for a row set simply based on a table, there should be not parameters at all
        createRowSet("products", CommandType.TABLE, false);
        verifyParameters(new String[]
            {
            }, "testTableParameters");
    }


    private void testParametersAfterNormalExecute() throws com.sun.star.uno.Exception
    {
        createRowSet("SELECT * FROM \"customers\"", CommandType.COMMAND, true);
        m_rowSetProperties.setPropertyValue("Command", "SELECT * FROM \"customers\" WHERE \"City\" = :city");
        final XParameters rowsetParams = UnoRuntime.queryInterface( XParameters.class, m_rowSet );
        rowsetParams.setString(1, "London");
        m_rowSet.execute();
    }


    private void verifyParameters(String[] _paramNames, String _context) throws com.sun.star.uno.Exception
    {
        final XIndexAccess params = m_paramsSupplier.getParameters();
        final int expected = _paramNames.length;
        final int found = params != null ? params.getCount() : 0;

        assertTrue("wrong number of parameters (expected: " + expected + ", found: " + found + ") in " + _context,
                found == expected);

        if (found == 0)
        {
            return;
        }

        for (int i = 0; i < expected; ++i)
        {
            final XPropertySet parameter = UnoRuntime.queryInterface( XPropertySet.class, params.getByIndex( i ) );

            final String expectedName = _paramNames[i];
            final String foundName = (String) parameter.getPropertyValue("Name");
            assertTrue("wrong parameter name (expected: " + expectedName + ", found: " + foundName + ") in" + _context,
                    expectedName.equals(foundName));
        }
    }


    private void testParametrizedQuery() throws com.sun.star.uno.Exception
    {
        // for a row set based on a parametrized query, those parameters should be properly
        // recognized
        m_dataSource.createQuery("products like", "SELECT * FROM \"products\" WHERE \"Name\" LIKE :product_name");
        createRowSet("products like", CommandType.QUERY, false);
        verifyParameters(new String[]
            {
                "product_name"
            }, "testParametrizedQuery");
    }


    private void testParametersInteraction() throws com.sun.star.uno.Exception
    {
        createRowSet("products like", CommandType.QUERY, false);

        // let's fill in a parameter value via XParameters, and see whether it is respected by the parameters container
        final XParameters rowsetParams = UnoRuntime.queryInterface(XParameters.class, m_rowSet);
        rowsetParams.setString(1, "Apples");

        XIndexAccess params = m_paramsSupplier.getParameters();
        XPropertySet firstParam = UnoRuntime.queryInterface( XPropertySet.class, params.getByIndex( 0 ) );
        Object firstParamValue = firstParam.getPropertyValue("Value");

        assertTrue("XParameters and the parameters container do not properly interact",
                   "Apples".equals(firstParamValue));

        // let's see whether this also survives an execute of the row set
        rowsetParams.setString(1, "Oranges");
        m_rowSet.execute();
        {
            // TODO: the following would not be necessary if the parameters container would *survive*
            // the execution of the row set. It currently doesn't (though the values it represents do).
            // It would be nice, but not strictly necessary, if it would.
            params = m_paramsSupplier.getParameters();
            firstParam = UnoRuntime.queryInterface( XPropertySet.class, params.getByIndex( 0 ) );
        }
        firstParamValue = firstParam.getPropertyValue("Value");
        assertTrue("XParameters and the parameters container do not properly interact, after the row set has been executed",
                   "Oranges".equals(firstParamValue));
    }


    private void testParametersInFilter() throws com.sun.star.uno.Exception
    {
        createRowSet("SELECT * FROM \"customers\"", CommandType.COMMAND, false);
        m_rowSetProperties.setPropertyValue("Filter", "\"City\" = :city");

        m_rowSetProperties.setPropertyValue("ApplyFilter", Boolean.TRUE);
        verifyParameters(new String[]
            {
                "city"
            }, "testParametersInFilter");

        m_rowSetProperties.setPropertyValue("ApplyFilter", Boolean.FALSE);
        verifyParameters(new String[]
            {
            }, "testParametersInFilter");
    }


    /** checks the XParametersSupplier functionality of a RowSet
     */
    @Test
    public void testParameters() throws Exception
    {
        createTestCase(false);
        // use an own RowSet instance, not the one which is also used for the other cases

        testTableParameters();
        testParametrizedQuery();
        testParametersInFilter();

        testParametersAfterNormalExecute();

        testParametersInteraction();
    }
}

