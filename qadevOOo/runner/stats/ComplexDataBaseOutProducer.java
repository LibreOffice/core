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
package stats;

import share.LogWriter;
import java.text.DecimalFormat;
import java.util.HashMap;
import java.util.Calendar;
import java.util.GregorianCalendar;

/**
 *
 *
 */
public class ComplexDataBaseOutProducer extends DataBaseOutProducer {

    /** Creates a new instance of ComplexDataBaseOutProducer */
    public ComplexDataBaseOutProducer(HashMap<String,String> param) {
        super(param);
        // do we have to write debug output?
        Object o = param.get("DebugIsActive");
        if (o != null) {
            if (o instanceof String) {
                String debug = (String)o;
                m_bDebug = (debug.equalsIgnoreCase("yes") || debug.equalsIgnoreCase("true"));
            }

        }

        String testBase = (String)mSqlInput.get("TestBase");
        String apiVersion = (String)mSqlInput.get("Version");
        String os = (String)mSqlInput.get("OperatingSystem");
        if (testBase == null || apiVersion == null || os == null) {
            System.err.println("The ComplexDataBaseOutProducer constructor needs this parameters to work correctly:");
            System.err.println("TestBase - " + testBase);
            System.err.println("Version - " + apiVersion);
            System.err.println("OperatingSystem - " + os);
            System.err.println("Add the missing parameter.");
        }
        int sep = testBase.indexOf('_');
        String testLanguage = testBase.substring(0,sep);
        testBase = testBase.substring(sep+1);

        // 2do fallback?
        String descriptionString = testLanguage+":"+ os +":"+testBase+":"+apiVersion;
        if (apiVersion != null)
            apiVersion = apiVersion.substring(0, 6);

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
        mSqlInput.put("test_run_description", descriptionString);
        mSqlInput.put("api_version_name", apiVersion);
        setWriteableEntryTypes(new String[]{"unit", "method"});
    }

    /**
     * Prepare the database.
     * @parameter log A log writer
     * @return True, if everything worked.
     */
    protected boolean prepareDataBase(LogWriter log) {
        executeSQLCommand("SELECT id AS \"test_run.id\", api_version_id, description, date FROM test_run" +
                          " WHERE date = \"$date\" AND description = \"$test_run_description\";", true);
        String id = (String)mSqlInput.get("test_run.id");
        // create an entry for this test run
        if (id == null) {
            executeSQLCommand("SELECT id as api_version_id FROM api_version" +
                              " WHERE version = \"$api_version_name\";", true);
            String api_version_id = (String)mSqlInput.get("api_version_id");
            // create an entry for this API
            if (api_version_id == null) {
                executeSQLCommand("INSERT api_version (api_name, version)" +
                                  " VALUES (\"soapi\", \"$api_version_name\")");
                executeSQLCommand("SELECT id as api_version_id FROM api_version" +
                                  " WHERE version = \"$api_version_name\";", true);
            }
            executeSQLCommand("INSERT test_run (api_version_id, description, date)" +
                              " VALUES ($api_version_id, \"$test_run_description\", \"$date\");");
            executeSQLCommand("SELECT id AS \"test_run.id\", api_version_id, description, date FROM test_run" +
                              " WHERE date = \"$date\" AND description = \"$test_run_description\";", true);
        }
        return true;
    }

    /**
     * Insert the result of the test of an entry into the database.
     * @parameter log A log writer
     * @return True, if everything worked.
     */
    protected boolean insertEntry(LogWriter log) {

        if ( m_bDebug ) {
            System.out.println("DEBUG - ComplexDataBaseOutProducer: entry.id has to be null: " + (mSqlInput.get("entry.id")==null));
            System.out.println("DEBUG - ComplexDataBaseOutProducer: EntryLongName: " + mSqlInput.get("EntryLongName"));
        }
        executeSQLCommand("SELECT id as \"entry.id\", name as \"entry.name\" FROM entry WHERE name = \"$EntryLongName\";", true);

        if (mSqlInput.get("entry.id") == null) {
            if ( m_bDebug ) {
                System.out.println("DEBUG - ComplexDataBaseOutProducer: No entry.id found, this is a new entry in the database.");
            }
            executeSQLCommand("INSERT entry (name, type)" +
                              " VALUES (\"$EntryLongName\", \"$EntryType\");");
            executeSQLCommand("SELECT id as \"entry.id\", name as \"entry.name\" FROM entry WHERE name = \"$EntryLongName\";", true);
        }


        executeSQLCommand("SELECT id as \"api_entry.id\", api_version_id as \"api_entry.api_version_id\", entry_id as \"api_entry.entry_id\" FROM api_entry" +
                          " WHERE entry_id = $entry.id;", true);
        if (mSqlInput.get("api_entry.id") == null) {
            System.out.println("No api_entry.id found");
            executeSQLCommand("INSERT api_entry (entry_id, api_version_id)"+
                              " VALUES ($entry.id, $api_version_id);");
            executeSQLCommand("SELECT id as \"api_entry.id\", api_version_id as \"api_entry.api_version_id\", entry_id as \"api_entry.entry_id\" FROM api_entry" +
                              " WHERE entry_id = $entry.id;", true);
        }


        executeSQLCommand("SELECT status as \"test_state.status\" FROM test_state"+
                          " WHERE test_run_id = $test_run.id AND entry_id = $entry.id;", true);

        String entryState = (String)mSqlInput.get("EntryState");
        String status = (String)mSqlInput.get("test_state.status");

        if (!entryState.equals("SKIPPED.FAILED")) { // occurs in case of misspellings: do not make an database entry.
            if (status == null) {
                executeSQLCommand("INSERT test_state (test_run_id, entry_id, status)"+
                                  " VALUES ($test_run.id, $entry.id, \"$EntryState\");");
            }
            else {
                executeSQLCommand("UPDATE test_state SET status = \"$EntryState\""+
                                  " where test_run_id =$test_run.id AND entry_id = $entry.id;");
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
