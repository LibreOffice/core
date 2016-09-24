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

}
