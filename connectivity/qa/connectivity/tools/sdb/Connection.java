/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package connectivity.tools.sdb;

import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XSingleSelectQueryComposer;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XDatabaseMetaData;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XStatement;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.Exception;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XRefreshable;

/**
 * is a convenience wrapper around a SDB-level connection object
 */
public class Connection
{
    private final   XConnection m_connection;

    public Connection( final XConnection _connection )
    {
        m_connection = _connection;
    }

    public XConnection  getXConnection()
    {
        return m_connection;
    }

    public boolean execute( final String _sql ) throws SQLException
    {
        XStatement statement = createStatement();
        return statement.execute( _sql );
    }

    public XResultSet executeQuery( final String _sql ) throws SQLException
    {
        XStatement statement = createStatement();
        return statement.executeQuery( _sql );
    }

    public int executeUpdate( final String _sql ) throws SQLException
    {
        XStatement statement = createStatement();
        return statement.executeUpdate( _sql );
    }

    public void refreshTables()
    {
        final XTablesSupplier suppTables = UnoRuntime.queryInterface(XTablesSupplier.class, m_connection);
        final XRefreshable refresh = UnoRuntime.queryInterface( XRefreshable.class, suppTables.getTables() );
        refresh.refresh();
    }

    public XSingleSelectQueryComposer createSingleSelectQueryComposer() throws Exception
    {
        final XMultiServiceFactory connectionFactory = UnoRuntime.queryInterface( XMultiServiceFactory.class, m_connection );
        return UnoRuntime.queryInterface(
            XSingleSelectQueryComposer.class, connectionFactory.createInstance( "com.sun.star.sdb.SingleSelectQueryComposer" ) );
    }

    public
    XStatement createStatement() throws SQLException
    {
        return m_connection.createStatement();
    }

    public
    XPreparedStatement prepareStatement( String _sql ) throws SQLException
    {
        return m_connection.prepareStatement( _sql );
    }

    public
    XDatabaseMetaData getMetaData() throws SQLException
    {
        return m_connection.getMetaData();
    }

    public
    void close() throws SQLException
    {
        m_connection.close();
    }
}
