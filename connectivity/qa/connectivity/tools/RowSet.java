/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: RowSet.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2006-07-10 14:18:37 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

package connectivity.tools;

import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.container.XNameAccess;
import com.sun.star.io.XInputStream;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XArray;
import com.sun.star.sdbc.XBlob;
import com.sun.star.sdbc.XClob;
import com.sun.star.sdbc.XRef;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.sdbc.XRowSetListener;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.Date;
import com.sun.star.util.DateTime;
import com.sun.star.util.Time;

public class RowSet implements XRowSet, XRow
{
    private XMultiServiceFactory    m_orb;
    private XRowSet                 m_rowSet;
    private XRow                    m_row;
    private XPropertySet            m_rowSetProps;

    public RowSet( XMultiServiceFactory _orb, String _dataSource, int _commandType, String _command )
    {
        try
        {
            m_rowSetProps = (XPropertySet)UnoRuntime.queryInterface(
                XPropertySet.class, _orb.createInstance("com.sun.star.sdb.RowSet") );
            m_rowSetProps.setPropertyValue( "DataSourceName", _dataSource );
            m_rowSetProps.setPropertyValue( "CommandType", new Integer( _commandType ) );
            m_rowSetProps.setPropertyValue( "Command", _command );

            m_rowSet = (XRowSet)UnoRuntime.queryInterface( XRowSet.class, m_rowSetProps );
            m_row = (XRow)UnoRuntime.queryInterface( XRow.class, m_rowSetProps );
        }
        catch ( Exception e )
        {
            e.printStackTrace(System.err);
            throw new java.lang.InstantiationError();
        }
    }

    // misc
    public int getColumnCount()
    {
        XColumnsSupplier suppCols = (XColumnsSupplier)UnoRuntime.queryInterface(
            XColumnsSupplier.class, m_rowSet );
        XIndexAccess columns = (XIndexAccess)UnoRuntime.queryInterface(
            XIndexAccess.class, suppCols.getColumns() );
        return columns.getCount();
    }

    // XRowSet
    public void execute() throws SQLException
    {
        m_rowSet.execute();
    }

    public void addRowSetListener( XRowSetListener _listener )
    {
        m_rowSet.addRowSetListener( _listener );
    }

    public void removeRowSetListener( XRowSetListener _listener )
    {
        m_rowSet.removeRowSetListener( _listener );
    }

    public boolean next() throws SQLException
    {
        return m_rowSet.next();
    }

    public boolean isBeforeFirst() throws SQLException
    {
        return m_rowSet.isBeforeFirst();
    }

    public boolean isAfterLast() throws SQLException
    {
        return m_rowSet.isAfterLast();
    }

    public boolean isFirst() throws SQLException
    {
        return m_rowSet.isFirst();
    }

    public boolean isLast() throws SQLException
    {
        return m_rowSet.isLast();
    }

    public void beforeFirst() throws SQLException
    {
        m_rowSet.beforeFirst();
    }

    public void afterLast() throws SQLException
    {
        m_rowSet.afterLast();
    }

    public boolean first() throws SQLException
    {
        return m_rowSet.first();
    }

    public boolean last() throws SQLException
    {
        return m_rowSet.last();
    }

    public int getRow() throws SQLException
    {
        return m_rowSet.getRow();
    }

    public boolean absolute(int i) throws SQLException
    {
        return m_rowSet.absolute(i);
    }

    public boolean relative(int i) throws SQLException
    {
        return m_rowSet.relative(i);
    }

    public boolean previous() throws SQLException
    {
        return m_rowSet.previous();
    }

    public void refreshRow() throws SQLException
    {
        m_rowSet.refreshRow();
    }

    public boolean rowUpdated() throws SQLException
    {
        return m_rowSet.rowUpdated();
    }

    public boolean rowInserted() throws SQLException
    {
        return m_rowSet.rowInserted();
    }

    public boolean rowDeleted() throws SQLException
    {
        return m_rowSet.rowDeleted();
    }

    // XRow
    public Object getStatement() throws SQLException
    {
        return m_rowSet.getStatement();
    }

    public boolean wasNull() throws SQLException
    {
        return m_row.wasNull();
    }

    public String getString(int i) throws SQLException
    {
        return m_row.getString(i);
    }

    public boolean getBoolean(int i) throws SQLException
    {
        return m_row.getBoolean(i);
    }

    public byte getByte(int i) throws SQLException
    {
        return m_row.getByte(i);
    }

    public short getShort(int i) throws SQLException
    {
        return m_row.getShort(i);
    }

    public int getInt(int i) throws SQLException
    {
        return m_row.getInt(i);
    }

    public long getLong(int i) throws SQLException
    {
        return m_row.getLong(i);
    }

    public float getFloat(int i) throws SQLException
    {
        return m_row.getFloat(i);
    }

    public double getDouble(int i) throws SQLException
    {
        return m_row.getDouble(i);
    }

    public byte[] getBytes(int i) throws SQLException
    {
        return m_row.getBytes(i);
    }

    public Date getDate(int i) throws SQLException
    {
        return m_row.getDate(i);
    }

    public Time getTime(int i) throws SQLException
    {
        return m_row.getTime(i);
    }

    public DateTime getTimestamp(int i) throws SQLException
    {
        return m_row.getTimestamp(i);
    }

    public XInputStream getBinaryStream(int i) throws SQLException
    {
        return m_row.getBinaryStream(i);
    }

    public XInputStream getCharacterStream(int i) throws SQLException
    {
        return m_row.getCharacterStream(i);
    }

    public Object getObject(int i, XNameAccess xNameAccess) throws SQLException
    {
        return m_row.getObject(i, xNameAccess);
    }

    public XRef getRef(int i) throws SQLException
    {
        return m_row.getRef(i);
    }

    public XBlob getBlob(int i) throws SQLException
    {
        return m_row.getBlob(i);
    }

    public XClob getClob(int i) throws SQLException
    {
        return m_row.getClob(i);
    }

    public XArray getArray(int i) throws SQLException
    {
        return m_row.getArray(i);
    }
};
