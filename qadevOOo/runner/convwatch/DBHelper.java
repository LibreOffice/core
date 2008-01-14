/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DBHelper.java,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2008-01-14 13:17:03 $
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

package convwatch;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.Statement;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.SQLException;

import java.lang.Thread;
import java.util.StringTokenizer;

class ShareConnection
{
    private Connection m_aConnection = null;
    public ShareConnection()
        {}

    public Connection getConnection()
        {
            if (m_aConnection == null)
            {
                try
                {
                    m_aConnection = DBHelper.getMySQLConnection();
                }
                catch(java.sql.SQLException e)
                {
                    GlobalLogWriter.get().println("DB: ERROR: can't connect to DB.");
                    m_aConnection = null;
                }
            }
            return m_aConnection;
        }
}

    class MySQLThread extends Thread
    {
        Connection m_aCon = null;
        String m_sSQL;
        public MySQLThread(Connection _aCon, String _sSQL)
            {
                m_aCon = _aCon;
                m_sSQL = _sSQL;
            }

        public void run()
            {
                Statement oStmt = null;
                if (m_aCon == null)
                {
                    GlobalLogWriter.get().println("DB: ERROR: in ExecSQL, connection not established.");
                    return;
                }

                // Connection oCon = null;
                try
                {
                    // oCon = getMySQLConnection();
                    oStmt = m_aCon.createStatement();

                    GlobalLogWriter.get().println("DB: " + m_sSQL);
                    /* ResultSet oResult = */
                    oStmt.executeUpdate(m_sSQL);
                }
                catch(Exception e)
                {
                    GlobalLogWriter.get().println("DB: Couldn't execute sql string '" + m_sSQL + "'");
                    GlobalLogWriter.get().println("DB: Reason: " + e.getMessage());
                }
            }
    }

public class DBHelper
{
    /**
     * This method inserts given values into<br>
     * the table 'states'
     * @param values a set of comma separated values to be inserted
     */

    public void SQLinsertValues(Connection _aCon, String _sTableName, String value_names, String values)
        {
            if (_aCon == null)
            {
                GlobalLogWriter.get().println("DB: ERROR: in SQLinsertValues, connection not established.");
                return;
            }

            // String aInsertStr = "";
            //
            // aInsertStr = "INSERT INTO " + _sTableName + " (" + value_names + " ) VALUES (" + values + ")";
            // ExecSQL(_aCon, aInsertStr);
            StringBuffer aInsertStr = new StringBuffer();

            aInsertStr.append( "INSERT INTO " ) . append( _sTableName );
            aInsertStr.append( " (").append( value_names ).append ( ")" );
            aInsertStr.append(" VALUES (" ).append( values ).append( ")" );
            ExecSQL(_aCon, aInsertStr.toString() );
        }

    public void SQLupdateValue(Connection _aCon, String _sTableName, String _sSet, String _sWhere)
        {
            if (_aCon == null)
            {
                GlobalLogWriter.get().println("DB: ERROR: in SQLinsertValues, connection not established.");
                return;
            }

            // String aUpdateStr = "";
            //
            // aUpdateStr = "UPDATE " + _sTableName + " SET " + _sSet + " WHERE " + _sWhere;
            // ExecSQL( _aCon, aUpdateStr );
            StringBuffer aUpdateStr = new StringBuffer();

            aUpdateStr.append( "UPDATE " ).append( _sTableName )
                .append( " SET " ).append( _sSet )
                .append( " WHERE " ).append( _sWhere );
            ExecSQL( _aCon, aUpdateStr.toString() );
        }

    private static String m_sDBServerName;
    private static String m_sDBName;
    private static String m_sDBUser;
    private static String m_sDBPasswd;

    protected synchronized void fillDBConnection(String _sInfo)
        {
            StringTokenizer aTokenizer = new StringTokenizer(_sInfo,",",false);
            while (aTokenizer.hasMoreTokens())
            {
                String sPart = aTokenizer.nextToken();
                if (sPart.startsWith("db:"))
                {
                    m_sDBName = sPart.substring(3);
                    // GlobalLogWriter.get().println("DB: source version: " + m_sSourceVersion);
                }
                else if (sPart.startsWith("user:"))
                {
                    m_sDBUser = sPart.substring(5);
                }
                else if (sPart.startsWith("passwd:"))
                {
                    m_sDBPasswd = sPart.substring(7);
                }
                else if (sPart.startsWith("server:"))
                {
                    m_sDBServerName = sPart.substring(7);
                }
            }
        }

    /**
     * This method establishes a Connection<br>
     * with the database 'module_unit' on jakobus
     */

    public static Connection getMySQLConnection() throws SQLException
        {
            try
            {
                Class.forName("org.gjt.mm.mysql.Driver");
                String sConnection = "jdbc:mysql://" + m_sDBServerName + ":3306/" + m_sDBName;
                // Connection mysql = DriverManager.getConnection(
                //    "jdbc:mysql://jakobus:3306/jobs_convwatch","admin","admin");
                Connection mysql = DriverManager.getConnection(sConnection, m_sDBUser, m_sDBPasswd);
                return mysql;
            }
            catch (ClassNotFoundException e)
            {
                GlobalLogWriter.get().println("DB: Class not found exception caught: " + e.getMessage());
                GlobalLogWriter.get().println("DB: Maybe mysql.jar is not added to the classpath.");
            }
            return null;
        }


    /**
     * This method removes all entries of the given<br>
     * module/platform combination
     * @param mdl the name of the module, e.g. sal
     * @param os the name of the platform, e.g. unxsols
     */
    // LLA: public static void SQLdeleteValues(Connection _aCon, String _sEnvironment, String _sUnitName, String _sMethodName, String _sCWS, String _sDate)
    // LLA:     {
    // LLA:         String sSQL =
    // LLA:             "DELETE FROM states WHERE " +
    // LLA:             "     unit=" + DatabaseEntry.Quote(_sUnitName) +
    // LLA:             " AND pf="   + DatabaseEntry.Quote (_sEnvironment) +
    // LLA:             " AND meth=" + DatabaseEntry.Quote (_sMethodName) +
    // LLA:             " AND cws="  + DatabaseEntry.Quote(_sCWS) +
    // LLA:             " AND dt="   + DatabaseEntry.Quote(_sDate);
    // LLA:
    // LLA:         // ExecSQL(_aCon, sSQL);
    // LLA:     }

    protected synchronized void ExecSQL(Connection _aCon, String _sSQL)
            {
                MySQLThread aSQLThread = new MySQLThread(_aCon, _sSQL);
                aSQLThread.start();
            }



    // public static int QueryIntFromSQL(String _sSQL, String _sColumnName, String _sValue)
    //     {
    //         boolean bNeedSecondTry = false;
    //         int nValue = 0;
    //         do
    //         {
    //             try
    //             {
    //                 nValue = QueryIntFromSQL(_sSQL, _sColumnName, _sValue);
    //             }
    //             catch (ValueNotFoundException e)
    //             {
    //                 bNeedSecondTry = true;
    //                 String sSQL = "INSERT INTO " + _sTable + "(" + _sColumnName + ") VALUES (" + _sValue + ")";
    //                 ExecSQL(sSQL);
    //             }
    //         } while (bNeedSecondTry);
    //         return nValue;
    //     }

    public int QueryIntFromSQL(Connection _aCon, String _sSQL, String _sColumnName)
        throws ValueNotFoundException
        {
            Statement oStmt = null;
            Connection oCon = null;
            int nValue = 0;
            try
            {
                // oCon = getMySQLConnection();
                oStmt = _aCon.createStatement();

                ResultSet oResult = oStmt.executeQuery(_sSQL);
                oResult.next();

                try
                {
                    if (_sColumnName.length() == 0)
                    {
                        // take the first row value (started with 1)
                        nValue = oResult.getInt(1);
                    }
                    else
                    {
                        nValue = oResult.getInt(_sColumnName);
                    }
                    // System.out.println("value: " + String.valueOf(nValue));
                }
                catch (SQLException e)
                {
                    String sError = e.getMessage();
                    GlobalLogWriter.get().println("DB: Original SQL error: " + sError);
                    throw new ValueNotFoundException("Cant execute SQL: " + _sSQL);
                }
            }
            catch(SQLException e)
            {
                String sError = e.getMessage();
                GlobalLogWriter.get().println("DB: Couldn't execute sql string " + _sSQL + "\n" + sError);
            }
            return nValue;
        }

    public String Quote(String _sToQuote)
        {
            String ts = "'";
            String ds = "\"";
            int nQuote = _sToQuote.indexOf(ts);
            if (nQuote >= 0)
            {
                return ds + _sToQuote + ds;
            }
            return ts + _sToQuote + ts;
        }

/* default date format in the MySQL DB yyyy-MM-dd */
    public static String today()
        {
            return DateHelper.getDateString("yyyy-MM-dd");
        }

    public static final String sComma = ",";
    public static final String sEqual = "=";
    public static final String sAND = " AND ";

}

