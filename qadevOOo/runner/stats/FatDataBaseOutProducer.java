/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FatDataBaseOutProducer.java,v $
 * $Revision: 1.7 $
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
 ************************************************************************/
package stats;

import java.text.DecimalFormat;
import share.LogWriter;
import share.DescEntry;
import java.util.Hashtable;
import java.util.Calendar;
import java.util.GregorianCalendar;


/**
 *
 */
public class FatDataBaseOutProducer extends DataBaseOutProducer {


    /** Creates a new instance of APIDataBaseOutProducer */
    public FatDataBaseOutProducer(Hashtable param) {
        super(param);
        String testBase = (String)mSqlInput.get("TestBase");
        int sep = testBase.indexOf('_');
        String testLanguage = testBase.substring(0,sep);
        testBase = testBase.substring(sep+1);
        String apiVersion = (String)mSqlInput.get("Version");
        String descriptionString = testLanguage+":"+(String)mSqlInput.get("OperatingSystem")+":"+testBase+":"+apiVersion;
        apiVersion = apiVersion.substring(0, 6);
        // build date
        if (mSqlInput.get("Date") != null) {
            mSqlInput.put("date", mSqlInput.get("Date"));
        }
        if (mSqlInput.get("date") == null) {
            // build date if it's not there
            Calendar cal = new GregorianCalendar();
            DecimalFormat dfmt = new DecimalFormat("00");
            String year = Integer.toString(cal.get(GregorianCalendar.YEAR));
            // month is starting with "0"
            String month = dfmt.format(cal.get(GregorianCalendar.MONTH) + 1);
            String day = dfmt.format(cal.get(GregorianCalendar.DATE));
            String dateString = year + "-" + month + "-" + day;

            mSqlInput.put("date", dateString);
        }

        setWriteableEntryTypes(new String[]{"property", "method", "component", "interface", "service"});

        mSqlInput.put("test_run.description", descriptionString);
        mSqlInput.put("api_version_name", apiVersion);
    }

    protected boolean prepareDataBase(LogWriter log) {
        executeSQLCommand("SHOW TABLES");

        executeSQLCommand("SELECT id AS \"test_run.id\", api_version_id, description, date FROM test_run" +
                          " WHERE date = \"$date\" AND description = \"$test_run.description\";", true);
        String id = (String)mSqlInput.get("test_run.id");
        // create an entry for this test run
        if (id == null) {
            executeSQLCommand("SELECT id AS api_version_id FROM api_version" +
                              " WHERE version = \"$api_version_name\";", true);
            String api_version_id = (String)mSqlInput.get("api_version_id");
            // create an entry for this API
            if (api_version_id == null) {
                executeSQLCommand("INSERT api_version (api_name, version)" +
                                  " VALUES (\"soapi\", \"$api_version_name\")");
                executeSQLCommand("SELECT id AS api_version_id FROM api_version" +
                                  " WHERE version = \"$api_version_name\";", true);
            }
            // complete entry for the test run
            executeSQLCommand("INSERT test_run (api_version_id, description, date)" +
                              " VALUES ($api_version_id, \"$test_run.description\", \"$date\");");
            executeSQLCommand("SELECT test_run.id AS \"test_run.id\", api_version_id, description, date FROM test_run" +
                              " WHERE date = \"$date\" AND description = \"$test_run.description\";", true);
        }
        return true;
    }

    // check the database afterwards
    protected boolean checkDataBase(LogWriter log) {
        return true;
    }

    protected boolean insertEntry(LogWriter log) {

        executeSQLCommand("SELECT id AS \"entry.id\", name AS \"entry.name\" FROM entry WHERE name = \"$EntryLongName\";", true);
        if (mSqlInput.get("entry.id") == null) {
            executeSQLCommand("INSERT entry (name, type)" +
                              " VALUES (\"$EntryLongName\", \"$EntryType\");");
            executeSQLCommand("SELECT id AS \"entry.id\", name AS \"entry.name\" FROM entry WHERE name = \"$EntryLongName\";", true);
        }
        executeSQLCommand("SELECT id AS \"api_entry.id\", api_version_id AS \"api_entry.api_version_id\", entry_id AS \"api_entry.entry_id\" FROM api_entry" +
                          " WHERE entry_id = $entry.id;", true);
        if (mSqlInput.get("api_entry.id") == null) {
            executeSQLCommand("INSERT api_entry (entry_id, api_version_id)"+
                              " VALUES ($entry.id, $api_version_id);");
            executeSQLCommand("SELECT id AS \"api_entry.id\", api_version_id AS \"api_entry.api_version_id\", entry_id AS \"api_entry.entry_id\" FROM api_entry" +
                              " WHERE entry_id = $entry.id;", true);
        }
        executeSQLCommand("SELECT status AS \"test_state.status\" FROM test_state"+
                          " WHERE test_run_id = $test_run.id AND entry_id = $entry.id;", true);

        String status = (String)mSqlInput.get("test_state.status");
        if (status == null) {
            executeSQLCommand("INSERT test_state (test_run_id, entry_id, status)"+
                              " VALUES ($test_run.id, $entry.id, \"$EntryState\");");
        }
        else {
            if (!status.endsWith("OK")) {
                executeSQLCommand("UPDATE test_state SET status = \"$EntryState\""+
                                  " WHERE test_run_id = $test_run.id AND entry_id = $entry.id;");
            }
        }
        return true;
    }

    public Object getWatcher() {
        return null;
    }

    public void setWatcher(Object watcher) {
    }

}
