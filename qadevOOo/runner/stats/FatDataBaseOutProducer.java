/*************************************************************************
 *
 *  $RCSfile: FatDataBaseOutProducer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-10-06 12:40:47 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
package stats;

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
        String pre = "";
        Calendar cal = new GregorianCalendar();
        String year = (new Integer(cal.get(GregorianCalendar.YEAR))).toString();
        int c = cal.get(GregorianCalendar.MONTH) + 1;
        pre = (c < 10)?"0":"";
        String month = pre + (new Integer(c)).toString();
        c = cal.get(GregorianCalendar.DATE);
        pre = (c < 10)?"0":"";
        String day = pre + (new Integer(c)).toString();
        String dateString = year + "-" + month + "-" + day;

        mSqlInput.put("date", dateString);
        mSqlInput.put("test_run.description", descriptionString);
        mSqlInput.put("api_version_name", apiVersion);
    }

    protected boolean prepareDataBase(LogWriter log) {
        executeSQLCommand("SELECT id AS \"test_run.id\", api_version_id, description, date FROM test_run" +
                          " WHERE date = \"$date\" AND description = \"$test_run.description\";", true);
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
                              " VALUES ($api_version_id, \"$test_run.description\", \"$date\");");
            executeSQLCommand("SELECT test_run.id, api_version_id, description, date FROM test_run" +
                              " WHERE date = \"$date\" AND description = \"$test_run.description\";", true);
        }
        return true;
    }

    protected boolean checkDataBase(LogWriter log) {
        return true;
    }

    protected boolean insertEntry(LogWriter log) {

        executeSQLCommand("SELECT id as \"entry.id\", name as \"entry.name\" FROM entry WHERE name = \"$EntryLongName\";", true);
        if (mSqlInput.get("entry.id") == null) {
            executeSQLCommand("INSERT entry (name, type)" +
                              " VALUES (\"$EntryLongName\", \"$EntryType\");");
            executeSQLCommand("SELECT id as \"entry.id\", name as \"entry.name\" FROM entry WHERE name = \"$EntryLongName\";", true);
        }
        executeSQLCommand("SELECT id as \"api_entry.id\", api_version_id as \"api_entry.api_version_id\", entry_id as \"api_entry.entry_id\" FROM api_entry" +
                          " WHERE entry_id = $entry.id;", true);
        if (mSqlInput.get("api_entry.id") == null) {
            executeSQLCommand("INSERT api_entry (entry_id, api_version_id)"+
                              " VALUES ($entry.id, $api_version_id);");
            executeSQLCommand("SELECT id as \"api_entry.id\", api_version_id as \"api_entry.api_version_id\", entry_id as \"api_entry.entry_id\" FROM api_entry" +
                              " WHERE entry_id = $entry.id;", true);
        }
        executeSQLCommand("SELECT status as \"test_state.status\" FROM test_state"+
                          " WHERE test_run_id = $test_run.id AND entry_id = $entry.id;", true);

        String status = (String)mSqlInput.get("test_state.status");
        if (status == null) {
            executeSQLCommand("INSERT test_state (test_run_id, entry_id, status)"+
                              " VALUES ($test_run.id, $entry.id, \"$EntryState\");");
        }
        else {
            if (status != "PASSED.OK") {
//                executeSQLCommand("INSERT test_state (test_run_id, entry_id, status)"+
//                                  " VALUES ($test_run.id, $entry.id, \"$EntryState\");");
            }
        }
        return true;
    }

}
