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

package convwatch;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.Statement;
import java.sql.SQLException;

import java.util.StringTokenizer;

class ShareConnection
{
    private Connection m_aConnection = null;

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
        private final Connection m_aCon;
        private final String m_sSQL;
        public MySQLThread(Connection _aCon, String _sSQL)
            {
                m_aCon = _aCon;
                m_sSQL = _sSQL;
            }

        @Override
        public void run()
            {
                Statement oStmt = null;
                if (m_aCon == null)
                {
                    GlobalLogWriter.get().println("DB: ERROR: in ExecSQL, connection not established.");
                    return;
                }

                try
                {
                    try
                    {
                        oStmt = m_aCon.createStatement();

                        GlobalLogWriter.get().println("DB: " + m_sSQL);
                        /* ResultSet oResult = */
                        oStmt.executeUpdate(m_sSQL);
                    }
                    finally
                    {
                        if (oStmt != null)
                            oStmt.close();
                    }
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
    public void SQLupdateValue(Connection _aCon, String _sTableName, String _sSet, String _sWhere)
        {
            if (_aCon == null)
            {
                GlobalLogWriter.get().println("DB: ERROR: in SQLinsertValues, connection not established.");
                return;
            }

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

    private synchronized void ExecSQL(Connection _aCon, String _sSQL)
            {
                MySQLThread aSQLThread = new MySQLThread(_aCon, _sSQL);
                aSQLThread.start();
            }



    public String Quote(String _sToQuote)
        {
            char ts = '\'';
            char ds = '"';
            int nQuote = _sToQuote.indexOf(ts);
            if (nQuote >= 0)
            {
                return ds + _sToQuote + ds;
            }
            return ts + _sToQuote + ts;
        }

    public static final String sEqual = "=";
    public static final String sAND = " AND ";

}

