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
import java.util.StringTokenizer;
import java.util.ArrayList;
import helper.OSHelper;

public class DB extends DBHelper
{
    private static DB m_aDB = null;

    // private ctor
    private DB()
        {
        }

    private static synchronized DB getDB()
        {
            if (m_aDB == null)
            {
                m_aDB = new DB();
            }
            return m_aDB;
        }

    private String m_sSourceVersion;
    private String m_sDestinationVersion;
    private String m_sDocumentPool;
    private String m_sDocID;
    private String m_sDBDistinct;

    public static void init(String _sDBInfoString)
        {
            if (_sDBInfoString == null) return;
            getDB().fillVariables(_sDBInfoString);
            getDB().updatestate_status("source started");
        }

    public static void test()
        {
            getDB().sql_test();
        }

    public static void source_start()
        {
            getDB().updatestate_status("source started");
        }

    public static void source_finished()
        {
            getDB().updatestate_status( "source finished");
        }

    public static void source_failed(String _sMessage)
        {
            getDB().updatestate_status("source failed");
            getDB().updateinfo_status(_sMessage);
        }

    public static void destination_start()
        {
            getDB().updatestate_status("destination started");
        }

    public static void destination_finished()
        {
            getDB().updatestate_status("PASSED-OK");
        }

    public static void destination_failed(String _sStatus, String _sMessage)
        {
            getDB().updatestate_status(_sStatus);
            getDB().updateinfo_status(_sMessage);
        }
    public static void writeNumberOfPages(int _nPages)
        {
            getDB().updatepagecount_documents(_nPages);
        }
    public static void writeErrorFile(String _sErrorFile)
        {
            getDB().updateerrorfile_status(_sErrorFile);
        }
    public static void writeHTMLFile(String _sHTMLFile)
        {
            getDB().updatehtmlfile_status(_sHTMLFile);
        }

    public static void writeToDB(String _sFilename,
                                 String _sBasename,
                                 String _sFileFormat,
                                 String _sBuildID,
                                 String _sSourceType,
                                 int _nResolution )
        {
            GlobalLogWriter.get().println("DB:   Filename:" + _sFilename);
            GlobalLogWriter.get().println("DB:   Basename:" + _sBasename);
            GlobalLogWriter.get().println("DB: FileFormat:" + _sFileFormat);
            GlobalLogWriter.get().println("DB:    BuildID:" + _sBuildID);
            GlobalLogWriter.get().println("DB: SourceType:" + _sSourceType);
            GlobalLogWriter.get().println("DB: Resolution:" + _nResolution);
        }

    private String getEnvironment()
        {
            if (OSHelper.isWindows())
            {
                return "wntmsci";
            }
            else if ( OSHelper.isSolarisIntel())
            {
                return "unxsoli";
            }
            else if ( OSHelper.isSolarisSparc())
            {
                return "unxsols";
            }
            else if ( OSHelper.isLinuxIntel())
            {
                return "unxlngi";
            }
            else
            {
                GlobalLogWriter.get().println("DB: Unknown environment.");
                GlobalLogWriter.get().println("DB: os.name := " + System.getProperty("os.name").toLowerCase());
                GlobalLogWriter.get().println("DB: os.arch := " + System.getProperty("os.arch"));
                return "";
            }
        }

    // fill some db access important variables with values given out of a simple string
    // DOC_COMPARATOR_DB_INFO_STRING=p:m220,c:m224,d:demo_lla,src:m220,dest:m224,doc:demo_lla,id:294,distinct:81

    private void fillVariables(String _sInfo)
        {
            fillDBConnection(_sInfo);
            getEnvironment();

            StringTokenizer aTokenizer = new StringTokenizer(_sInfo,",",false);
            while (aTokenizer.hasMoreTokens())
            {
                String sPart = aTokenizer.nextToken();
                if (sPart.startsWith("p:"))
                {
                    m_sSourceVersion = sPart.substring(2);
                    GlobalLogWriter.get().println("DB: source version: " + m_sSourceVersion);
                }
                else if (sPart.startsWith("src:"))
                {
                    m_sSourceVersion = sPart.substring(4);
                    GlobalLogWriter.get().println("DB: source version: " + m_sSourceVersion);
                }
                else if (sPart.startsWith("c:"))
                {
                    m_sDestinationVersion = sPart.substring(2);
                    GlobalLogWriter.get().println("DB: destination version: " + m_sDestinationVersion);
                }
                else if (sPart.startsWith("dest:"))
                {
                    m_sDestinationVersion = sPart.substring(5);
                    GlobalLogWriter.get().println("DB: destination version: " + m_sDestinationVersion);
                }
                else if (sPart.startsWith("d:"))
                {
                    m_sDocumentPool = sPart.substring(2);
                    GlobalLogWriter.get().println("DB: documentpool version: " + m_sDocumentPool);
                }
                else if (sPart.startsWith("doc:"))
                {
                    m_sDocumentPool = sPart.substring(4);
                    GlobalLogWriter.get().println("DB: documentpool version: " + m_sDocumentPool);
                }
                else if (sPart.startsWith("id:"))
                {
                    m_sDocID = sPart.substring(3);
                    GlobalLogWriter.get().println("DB: docid: " + m_sDocID);
                }
                else if (sPart.startsWith("distinct:"))
                {
                    m_sDBDistinct = sPart.substring(9);
                    GlobalLogWriter.get().println("DB: distinct: " + m_sDBDistinct);
                }
                else
                {
                }
            }
        }

    private void sql_test()
        {
            String sUUID = getDBDistinct();
            System.out.println("UUID: " + sUUID);
        }

    public ArrayList<String> QuerySQL(Connection _aCon, String _sSQL)
        {
            java.sql.Statement oStmt = null;
            ArrayList<String> aResultList = new ArrayList<String>();
            try
            {
                oStmt = _aCon.createStatement();

                java.sql.ResultSet aResultSet = oStmt.executeQuery(_sSQL);
                java.sql.ResultSetMetaData aResultSetMetaData = aResultSet.getMetaData();

                int nColumnCount = aResultSetMetaData.getColumnCount();         // java sql starts with '1'

                while( aResultSet.next() )
                {
                    StringBuffer aResult = new StringBuffer();
                    try
                    {
                        aResult.append("sqlresult: ");
                        for (int i=1;i<=nColumnCount;i++)
                        {
                            String sColumnName = aResultSetMetaData.getColumnName(i);
                            aResult.append(sColumnName).append("=");
                            String sValue;
                            int nSQLType = aResultSetMetaData.getColumnType(i);
                            switch(nSQLType)
                            {
                            case java.sql.Types.VARCHAR:
                                sValue = "'" + aResultSet.getString(i)  +  "'";
                                break;
                            case java.sql.Types.INTEGER:
                            {
                                int nValue = aResultSet.getInt(i);
                                sValue = String.valueOf(nValue);
                                break;
                            }

                            default:
                                sValue = "UNSUPPORTED TYPE";
                            }
                            aResult.append(sValue).append(", ");
                        }
                        String sResult = aResult.toString();
                        aResultList.add(sResult);
                    }
                    catch (java.sql.SQLException e)
                    {
                    }
                }
            }
            catch (java.sql.SQLException e)
            {
                String sError = e.getMessage();
                GlobalLogWriter.get().println("DB: Original SQL error: " + sError);
                // throw new ValueNotFoundException("Can't execute SQL: " + _sSQL);
            }
            return aResultList;
        }

    private void updatestate_status(String _sStatus)
        {
            Connection aCon = new ShareConnection().getConnection();

            String sSet = "state=" + Quote(_sStatus);
            String sWhere = getWhereClause();
            if (sWhere.length() > 0)
            {
            SQLupdateValue( aCon, "status", sSet, sWhere );
        }
        }
    private void updateinfo_status(String _sInfo)
        {
            Connection aCon = new ShareConnection().getConnection();

            String sSet = "info=" + Quote(_sInfo);
            String sWhere = getWhereClause();
            SQLupdateValue( aCon, "status", sSet, sWhere );
        }
    private void updateerrorfile_status(String _sErrorFile)
        {
            Connection aCon = new ShareConnection().getConnection();

            String sErrorFile = _sErrorFile.replace('\\', '/');

            String sSet = "errorfile=" + Quote(sErrorFile);
            String sWhere = getWhereClause();
            SQLupdateValue( aCon, "status", sSet, sWhere );
        }
    private void updatehtmlfile_status(String _sHtmlFile)
        {
            Connection aCon = new ShareConnection().getConnection();

            String sHtmlFile = _sHtmlFile.replace('\\', '/');

            String sSet = "htmlfile=" + Quote(sHtmlFile);
            String sWhere = getWhereClause();
            SQLupdateValue( aCon, "status", sSet, sWhere );
        }
    private void updatepagecount_documents(int _nPageCount)
        {
            Connection aCon = new ShareConnection().getConnection();

            String sSet = "pagecount=" + _nPageCount;
            String sWhere = getWhereClause();
            SQLupdateValue( aCon, "documents", sSet, sWhere );

        }


    private String getWhereClause()
        {
            StringBuffer aWhereClause = new StringBuffer();
            boolean bAND = false;
            if (m_sDocID != null)
            {
                aWhereClause.append( "docid" ). append(sEqual) . append(m_sDocID);
                bAND = true;
            }
            if (bAND)
            {
                aWhereClause.append(sAND);
            }
            if (m_sDBDistinct != null)
            {
                aWhereClause.append( "dbdistinct2" ). append(sEqual) . append(Quote(m_sDBDistinct));
            }
            return aWhereClause.toString();
        }

    private String getDBDistinct()
        {
            Connection aCon = new ShareConnection().getConnection();

            String sSQL = "SELECT uuid()";
            ArrayList<String> aResultList = QuerySQL(aCon, sSQL);

            for (int i=0;i<aResultList.size();i++)
            {
                String sResult = aResultList.get(i);

                StringTokenizer aTokenizer = new StringTokenizer(sResult,",",false);
                while (aTokenizer.hasMoreTokens())
                {
                    String sToken = aTokenizer.nextToken();
                    int nIndex = sToken.indexOf("uuid()=");
                    int nIndexTuettel = sToken.indexOf("'", nIndex);
                    int nIndexTuettel2 = sToken.lastIndexOf("'");
                    String sUuid = sToken.substring(nIndexTuettel + 1, nIndexTuettel2);
                    return sUuid;
                }
            }

            return "0";
        }

    public static void insertinto_documentcompare(String _sSourceVersion, String _sSourceName, String _sSourceCreatorType,
                                                  String _sDestinationVersion, String _sDestinationName, String _sDestinationCreatorType,
                                                  String _sDocumentPoolDir, String _sDocumentPoolName, String _sMailAddress,
                                                  String _sSpecial, String _sParentDistinct)
        {
            getDB().insertinto_documentcompare_impl(  _sSourceVersion,   _sSourceName,   _sSourceCreatorType,
                                                      _sDestinationVersion,   _sDestinationName,   _sDestinationCreatorType,
                                                      _sDocumentPoolDir,   _sDocumentPoolName,   _sMailAddress,
                                                      _sSpecial, _sParentDistinct);
        }

    private void insertinto_documentcompare_impl(String _sSourceVersion, String _sSourceName, String _sSourceCreatorType,
                                                 String _sDestinationVersion, String _sDestinationName, String _sDestinationCreatorType,
                                                 String _sDocumentPoolDir, String _sDocumentPoolName, String _sMailAddress,
                                                 String _sSpecial, String _sParentDistinct)
        {
            // $sSQLInsert = "INSERT INTO documentcompare
            if (_sParentDistinct == null)
            {
                _sParentDistinct = "";
            }

            Connection aCon = new ShareConnection().getConnection();

            String sValueLine="dbdistinct2, environment, sourceversion, sourcename, sourcecreatortype, destinationversion, destinationname, destinationcreatortype, documentpoolpath, documentpool, mailfeedback, state, special, parentdistinct, startdate";
            String sDocumentPoolDir = _sDocumentPoolDir.replace('\\', '/');
            StringBuffer aDataLine = new StringBuffer();
            aDataLine.append( Quote(getDBDistinct()) ) . append( sComma ) .
                append( Quote( getEnvironment()) ) . append( sComma ) .
                append( Quote( _sSourceVersion) )   . append( sComma ) .
                append( Quote( _sSourceName) )   . append( sComma ) .
                append( Quote( _sSourceCreatorType ) )   . append( sComma ) .
                append( Quote( _sDestinationVersion) )   . append( sComma ) .
                append( Quote( _sDestinationName) )   . append( sComma ) .
                append( Quote( _sDestinationCreatorType ) )   . append( sComma ) .
                append( Quote( sDocumentPoolDir) ) . append( sComma ) .
                append( Quote( _sDocumentPoolName) ) . append( sComma ) .
                append( Quote( _sMailAddress) )    . append( sComma ) .
                append( Quote( "new" )) . append ( sComma ) .
                append( Quote( _sSpecial ) )    . append( sComma ) .
                append( Quote( _sParentDistinct ) )    . append( sComma ) .
                append( Quote( today() ));

            SQLinsertValues(aCon, "documentcompare", sValueLine, aDataLine.toString());
        }

}
