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
import share.DescEntry;

import java.util.HashMap;

public abstract class DataBaseOutProducer implements LogWriter {
    protected HashMap<String,Object> mSqlInput = null;
    protected HashMap<String, String[]> mSqlOutput = null;
    protected String[] mWriteableEntryTypes = null;
    protected SQLExecution mSqlExec;
    protected boolean m_bDebug = false;


    /** Creates a new instance of DataBaseOutProducer
     * @param param The HashMap with test parameters
     */
    public DataBaseOutProducer(HashMap<String,String> param) {
        mSqlInput = new HashMap<String,Object>();
        mSqlInput.putAll(param);

        Object o = param.get("DebugIsActive");
        String debug = null;
        if (o instanceof String)
            debug = (String)o;
        else
            debug = o.toString();
        if (debug != null && (debug.equalsIgnoreCase("true") || debug.equalsIgnoreCase("yes"))) {
            m_bDebug = true;
        }
        // set default for writeable entries: method
        setWriteableEntryTypes(new String[]{"method"});
    }

    /** initialization
     *
     */
    public boolean initialize(DescEntry entry, boolean active) {
        if (entry.UserDefinedParams != null)
            mSqlInput.putAll(entry.UserDefinedParams);

        String jdbcClass = (String)mSqlInput.get("JDBC");
        if (jdbcClass == null)
            jdbcClass = "org.gjt.mm.mysql.Driver";
        String dbURL = (String)mSqlInput.get("DataBaseURL");
        String user = (String)mSqlInput.get("User");
        String password = (String)mSqlInput.get("Password");
        if (user == null)
            user = (String)mSqlInput.get("OperatingSystem");
        if (password == null)
            password = user;

        mSqlExec = new SQLExecution(jdbcClass, dbURL, user, password, m_bDebug);
        mSqlExec.openConnection();
        prepareDataBase(entry.Logger);
        return true;
    }

    public boolean summary(DescEntry entry) {
        mSqlExec.openConnection();
        findTypeInEntryTree(entry, entry.Logger);
        mSqlExec.closeConnection();
        return true;
    }

    /**
     * Step recursively through the entry tree: write all entries of the
     * defined types to the database.
     * @param entry The description entry that is take as root
     * @param log The log writer
     */
    protected boolean findTypeInEntryTree(DescEntry entry, LogWriter log) {
        boolean returnVal = true;
        if (isWriteableEntryType(entry)) {
            returnVal &= insertEntry(entry, log);
        }

        if (entry.SubEntryCount >0) {
            for (int i=0; i<entry.SubEntryCount; i++) {
                returnVal &= findTypeInEntryTree(entry.SubEntries[i], log);
            }
        }
        // if we are not on method leaf, exit here
        // insert one method result into database
        return returnVal;
    }

    /**
     * Insert this entrry to the database.
     * @param entry The entry to write.
     * @param log The log writer.
     */
    protected boolean insertEntry(DescEntry entry, LogWriter log) {
        // copy the swlInput HashMap, so it can be reset easily for the next run
        HashMap<String,Object> copySqlInput = new HashMap<String,Object>();
        copySqlInput.putAll(mSqlInput);
        // put some stuff from entry in the HashMap
        mSqlInput.put("EntryLongName", entry.longName);
        mSqlInput.put("EntryName", entry.entryName);
        mSqlInput.put("EntryState", entry.State);
        mSqlInput.put("EntryType", entry.EntryType);
        boolean result = insertEntry(log);
        // reset the Hashtable
        mSqlInput = copySqlInput;
        return result;

    }

    /**
     * Set the writeable entry types: for example "method", "interface", etc.
     * All these entries are written to the database.
     * @param types A String array with all types that have to be written.
     */
    public void setWriteableEntryTypes(String[] types) {
        mWriteableEntryTypes = types;
    }

    /**
     * Is the entry of the writeable entry type?
     * @param entry The entry that is checked
     * @return True, if it is indeed a writeable entry.
     */
    protected boolean isWriteableEntryType(DescEntry entry) {
        boolean result = false;
        for (int i=0; i<mWriteableEntryTypes.length; i++) {
            if (entry.EntryType.equals(mWriteableEntryTypes[i])) {
                result = true;
                break;
            }
        }
        return result;
    }

    /**
     * Wrap the command of SQLExecution class for transparency.
     */
    protected boolean executeSQLCommand(String command, boolean mergeOutput) {
        return mSqlExec.executeSQLCommand(command, mSqlInput, mSqlOutput, mergeOutput);
    }

    /**
     * Wrap the command of SQLExecution class for transparency.
     */
    protected boolean executeSQLCommand(String command) {
        return mSqlExec.executeSQLCommand(command, mSqlInput, mSqlOutput);
    }

    /**
     * Method to print: empty here
     */
    public void println(String msg) {
    }

    /**
     * Prepare the database: executed once at the beginning.
     * Abstract method, so derived classes have to overwrite it.
     */
    protected abstract boolean prepareDataBase(LogWriter log);

    /**
     * Insert one entr into the database.
     * Abstract method, so derived classes have to overwrite it.
     */
    protected abstract boolean insertEntry(LogWriter log);
}
