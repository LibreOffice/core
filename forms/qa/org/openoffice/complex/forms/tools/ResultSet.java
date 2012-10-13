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

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.openoffice.complex.forms.tools;

import com.sun.star.container.XNameAccess;
import com.sun.star.io.XInputStream;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XArray;
import com.sun.star.sdbc.XBlob;
import com.sun.star.sdbc.XClob;
import com.sun.star.sdbc.XRef;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XRow;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

public class ResultSet implements XResultSet, XRow
{
    private final XResultSet    m_resultSet;
    private final XRow          m_row;

    public ResultSet( final Object _resultSet )
    {
        m_resultSet = (XResultSet)UnoRuntime.queryInterface( XResultSet.class, _resultSet );
        m_row = (XRow)UnoRuntime.queryInterface( XRow.class, _resultSet );
    }

    public
    boolean next() throws SQLException
    {
        return m_resultSet.next();
    }

    public
    boolean isBeforeFirst() throws SQLException
    {
        return m_resultSet.isBeforeFirst();
    }

    public
    boolean isAfterLast() throws SQLException
    {
        return m_resultSet.isAfterLast();
    }

    public
    boolean isFirst() throws SQLException
    {
        return m_resultSet.isFirst();
    }

    public
    boolean isLast() throws SQLException
    {
        return m_resultSet.isLast();
    }

    public
    void beforeFirst() throws SQLException
    {
        m_resultSet.beforeFirst();
    }

    public
    void afterLast() throws SQLException
    {
        m_resultSet.afterLast();
    }

    public
    boolean first() throws SQLException
    {
        return m_resultSet.first();
    }

    public
    boolean last() throws SQLException
    {
        return m_resultSet.last();
    }

    public
    int getRow() throws SQLException
    {
        return m_resultSet.getRow();
    }

    public
    boolean absolute( int _row ) throws SQLException
    {
        return m_resultSet.absolute( _row );
    }

    public
    boolean relative( int _offset ) throws SQLException
    {
        return m_resultSet.relative( _offset );
    }

    public
    boolean previous() throws SQLException
    {
        return m_resultSet.previous();
    }

    public
    void refreshRow() throws SQLException
    {
        m_resultSet.refreshRow();
    }

    public
    boolean rowUpdated() throws SQLException
    {
        return m_resultSet.rowUpdated();
    }

    public
    boolean rowInserted() throws SQLException
    {
        return m_resultSet.rowInserted();
    }

    public
    boolean rowDeleted() throws SQLException
    {
        return m_resultSet.rowDeleted();
    }

    public
    Object getStatement() throws SQLException
    {
        return m_resultSet.getStatement();
    }

    public
    boolean wasNull() throws SQLException
    {
        return m_row.wasNull();
    }

    public
    String getString( int _colIndex ) throws SQLException
    {
        return m_row.getString( _colIndex );
    }

    public
    boolean getBoolean( int _colIndex ) throws SQLException
    {
        return m_row.getBoolean( _colIndex );
    }

    public
    byte getByte( int _colIndex ) throws SQLException
    {
        return m_row.getByte( _colIndex );
    }

    public
    short getShort( int _colIndex ) throws SQLException
    {
        return m_row.getShort( _colIndex );
    }

    public
    int getInt( int _colIndex ) throws SQLException
    {
        return m_row.getInt( _colIndex );
    }

    public
    long getLong( int _colIndex ) throws SQLException
    {
        return m_row.getLong( _colIndex );
    }

    public
    float getFloat( int _colIndex ) throws SQLException
    {
        return m_row.getFloat( _colIndex );
    }

    public
    double getDouble( int _colIndex ) throws SQLException
    {
        return m_row.getDouble( _colIndex );
    }

    public
    byte[] getBytes( int _colIndex ) throws SQLException
    {
        return m_row.getBytes( _colIndex );
    }

    public
    Date getDate( int _colIndex ) throws SQLException
    {
        return m_row.getDate( _colIndex );
    }

    public
    Time getTime( int _colIndex ) throws SQLException
    {
        return m_row.getTime( _colIndex );
    }

    public
    DateTime getTimestamp( int _colIndex ) throws SQLException
    {
        return m_row.getTimestamp( _colIndex );
    }

    public
    XInputStream getBinaryStream( int _colIndex ) throws SQLException
    {
        return m_row.getBinaryStream( _colIndex );
    }

    public
    XInputStream getCharacterStream( int _colIndex ) throws SQLException
    {
        return m_row.getCharacterStream( _colIndex );
    }

    public
    Object getObject( int _colIndex, XNameAccess _typeMap ) throws SQLException
    {
        return m_row.getObject( _colIndex, _typeMap );
    }

    public
    XRef getRef( int _colIndex ) throws SQLException
    {
        return m_row.getRef( _colIndex );
    }

    public
    XBlob getBlob( int _colIndex ) throws SQLException
    {
        return m_row.getBlob( _colIndex );
    }

    public
    XClob getClob( int _colIndex ) throws SQLException
    {
        return m_row.getClob( _colIndex );
    }

    public
    XArray getArray( int _colIndex ) throws SQLException
    {
        return m_row.getArray( _colIndex );
    }
}
