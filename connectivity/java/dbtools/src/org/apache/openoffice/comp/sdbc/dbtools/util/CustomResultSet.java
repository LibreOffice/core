/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

package org.apache.openoffice.comp.sdbc.dbtools.util;

import java.util.ArrayList;

import org.apache.openoffice.comp.sdbc.dbtools.comphelper.PropertySet;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.IllegalArgumentException;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XArray;
import com.sun.star.sdbc.XBlob;
import com.sun.star.sdbc.XClob;
import com.sun.star.sdbc.XCloseable;
import com.sun.star.sdbc.XColumnLocate;
import com.sun.star.sdbc.XRef;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetMetaData;
import com.sun.star.sdbc.XResultSetMetaDataSupplier;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbcx.CompareBookmark;
import com.sun.star.sdbcx.XRowLocate;
import com.sun.star.uno.AnyConverter;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

public class CustomResultSet extends PropertySet
        implements XResultSet, XCloseable, XRowLocate, XPropertySet, XColumnLocate, XRow, XResultSetMetaDataSupplier {

    private XResultSetMetaData resultSetMetaData;
    private ArrayList<ORowSetValue[]> rows;
    /// 0-based:
    private int currentRow = -1;
    /// 1-based:
    private int currentColumn;

    public CustomResultSet(XResultSetMetaData resultSetMetaData, ArrayList<ORowSetValue[]> rows) {
        this.resultSetMetaData = resultSetMetaData;
        this.rows = rows;
    }

    // XComponent:
    @Override
    protected void postDisposing() {
    }

    // XCloseable:

    public void close() throws SQLException {
        dispose();
    }

    // XResultSet:

    private ORowSetValue getField(int columnIndex) throws SQLException {
        if (isBeforeFirst() || isAfterLast()) {
            throw new SQLException("Row out of range");
        }
        ORowSetValue[] fields = rows.get(currentRow);
        if (columnIndex < 1 || fields.length < columnIndex) {
            throw new SQLException("Column out of range");
        }
        currentColumn = columnIndex;
        return fields[columnIndex - 1];
    }

    public synchronized boolean absolute(int position) throws SQLException {
        checkDisposed();
        if (position >= 0) {
            currentRow = position;
        } else {
            currentRow = rows.size() + position;
        }
        if (currentRow <= -1) {
            currentRow = -1;
            return false;
        }
        if (currentRow >= rows.size()) {
            currentRow = rows.size();
            return false;
        }
        return true;
    }

    public synchronized void afterLast() throws SQLException {
        checkDisposed();
        currentRow = rows.size();
    }

    public synchronized void beforeFirst() throws SQLException {
        checkDisposed();
        currentRow = -1;
    }

    public synchronized boolean first() throws SQLException {
        checkDisposed();
        currentRow = 0;
        return true;
    }

    public synchronized int getRow() throws SQLException {
        checkDisposed();
        return currentRow + 1;
    }

    public synchronized Object getStatement() throws SQLException {
        checkDisposed();
        return null;
    }

    public synchronized boolean isAfterLast() throws SQLException {
        checkDisposed();
        return currentRow == rows.size();
    }

    public synchronized boolean isBeforeFirst() throws SQLException {
        checkDisposed();
        return currentRow == -1;
    }

    public synchronized boolean isFirst() throws SQLException {
        checkDisposed();
        return currentRow == 0;
    }

    public synchronized boolean isLast() throws SQLException {
        checkDisposed();
        return currentRow == (rows.size() - 1);
    }

    public synchronized boolean last() throws SQLException {
        checkDisposed();
        currentRow = rows.size() - 1;
        return true;
    }

    public synchronized boolean next() throws SQLException {
        checkDisposed();
        if (currentRow < rows.size()) {
            ++currentRow;
        }
        return currentRow < rows.size();
    }

    public synchronized boolean previous() throws SQLException {
        checkDisposed();
        if (currentRow > -1) {
            --currentRow;
        }
        return currentRow > -1;
    }

    public synchronized void refreshRow() throws SQLException {
        checkDisposed();
    }

    public synchronized boolean relative(int offset) throws SQLException {
        checkDisposed();
        currentRow += offset;
        if (currentRow <= -1) {
            currentRow = -1;
            return false;
        }
        if (currentRow >= rows.size()) {
            currentRow = rows.size();
            return false;
        }
        return true;
    }

    public synchronized boolean rowDeleted() throws SQLException {
        checkDisposed();
        return false;
    }

    public synchronized boolean rowInserted() throws SQLException {
        checkDisposed();
        return false;
    }

    public synchronized boolean rowUpdated() throws SQLException {
        checkDisposed();
        return false;
    }

    // XResultSetMetaDataSupplier:

    public synchronized XResultSetMetaData getMetaData() throws SQLException {
        checkDisposed();
        return resultSetMetaData;
    }

    // XRow:

    public synchronized XArray getArray(int columnIndex) throws SQLException {
        checkDisposed();
        return null;
    }

    public synchronized XInputStream getBinaryStream(int columnIndex) throws SQLException {
        checkDisposed();
        return null;
    }

    public synchronized XBlob getBlob(int columnIndex) throws SQLException {
        checkDisposed();
        return null;
    }

    public synchronized boolean getBoolean(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getBoolean();
    }

    public synchronized byte getByte(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getInt8();
    }

    public synchronized byte[] getBytes(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getSequence();
    }

    public synchronized XInputStream getCharacterStream(int columnIndex) throws SQLException {
        checkDisposed();
        return null;
    }

    public synchronized XClob getClob(int columnIndex) throws SQLException {
        checkDisposed();
        return null;
    }

    public synchronized Date getDate(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getDate();
    }

    public synchronized double getDouble(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getDouble();
    }

    public synchronized float getFloat(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getFloat();
    }

    public synchronized int getInt(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getInt32();
    }

    public synchronized long getLong(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getLong();
    }

    public synchronized Object getObject(int columnIndex, XNameAccess arg1) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.makeAny();
    }

    public synchronized XRef getRef(int columnIndex) throws SQLException {
        checkDisposed();
        return null;
    }

    public synchronized short getShort(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getInt16();
    }

    public synchronized String getString(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getString();
    }

    public synchronized Time getTime(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getTime();
    }

    public synchronized DateTime getTimestamp(int columnIndex) throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(columnIndex);
        return field.getDateTime();
    }

    public synchronized boolean wasNull() throws SQLException {
        checkDisposed();
        ORowSetValue field = getField(currentColumn);
        return field.isNull();
    }

    // XColumnLocate:

    public synchronized int findColumn(String name) throws SQLException {
        checkDisposed();
        for (int i = 1; i <= resultSetMetaData.getColumnCount(); i++) {
            boolean isCaseSensitive = resultSetMetaData.isCaseSensitive(i);
            String columnName = resultSetMetaData.getColumnName(i);
            boolean matched;
            if (isCaseSensitive) {
                matched = columnName.equals(name);
            } else {
                matched = columnName.equalsIgnoreCase(name);
            }
            if (matched) {
                return i;
            }
        }
        String error = SharedResources.getInstance().getResourceStringWithSubstitution(
                Resources.STR_UNKNOWN_COLUMN_NAME, "$columnname$", name);
        throw new SQLException(error, this, StandardSQLState.SQL_COLUMN_NOT_FOUND.text(), 0, null);
    }

    // XRowLocate:

    public synchronized int compareBookmarks(Object arg0, Object arg1) throws SQLException {
        checkDisposed();

        int bookmark1, bookmark2;
        try {
            bookmark1 = AnyConverter.toInt(arg0);
            bookmark2 = AnyConverter.toInt(arg1);
        } catch (IllegalArgumentException illegalArgumentException) {
            return CompareBookmark.NOT_COMPARABLE;
        }

        if (bookmark1 < bookmark2) {
            return CompareBookmark.LESS;
        } else if (bookmark1 > bookmark2) {
            return CompareBookmark.GREATER;
        } else {
            return CompareBookmark.EQUAL;
        }
    }

    public synchronized Object getBookmark() throws SQLException {
        checkDisposed();
        return currentRow;
    }

    public synchronized boolean hasOrderedBookmarks() throws SQLException {
        checkDisposed();
        return true;
    }

    public synchronized int hashBookmark(Object arg0) throws SQLException {
        checkDisposed();
        int bookmark;
        try {
            bookmark = AnyConverter.toInt(arg0);
        } catch (IllegalArgumentException illegalArgumentException) {
            throw new SQLException("Bad bookmark", this, StandardSQLState.SQL_INVALID_BOOKMARK_VALUE.text(), 0, null);
        }
        return bookmark;
    }

    public synchronized boolean moveRelativeToBookmark(Object arg0, int arg1) throws SQLException {
        checkDisposed();
        int bookmark;
        boolean moved = false;
        try {
            bookmark = AnyConverter.toInt(arg0);
            moved = absolute(bookmark);
            if (moved) {
                moved = relative(arg1);
            }
        } catch (IllegalArgumentException illegalArgumentException) {
        }
        if (!moved) {
            afterLast();
        }
        return moved;
    }

    public synchronized boolean moveToBookmark(Object arg0) throws SQLException {
        checkDisposed();
        int bookmark;
        boolean moved = false;
        try {
            bookmark = AnyConverter.toInt(arg0);
            moved = absolute(bookmark);
        } catch (IllegalArgumentException illegalArgumentException) {
        }
        if (!moved) {
            afterLast();
        }
        return moved;
    }
}
