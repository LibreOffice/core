/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: DataBaseOutProducer.java,v $
 * $Revision: 1.6 $
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

import share.LogWriter;
import share.DescEntry;

import java.sql.Connection;
import java.sql.Statement;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.util.Vector;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.StringTokenizer;

/**
 *
 * @author  sg128468
 */
public abstract class DataBaseOutProducer implements LogWriter {
    protected Hashtable mSqlInput = null;
    protected Hashtable mSqlOutput = null;
    protected String[] mWriteableEntryTypes = null;
    protected SQLExecution mSqlExec;
    protected boolean m_bDebug = false;


    /** Creates a new instance of DataBaseOutProducer
     * @param param The Hashtable with test parameters
     */
    public DataBaseOutProducer(Hashtable param) {
        mSqlInput = new Hashtable();
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

    /**
     *
     *
     */
    public boolean summary(DescEntry entry) {
        mSqlExec.openConnection();
        findTypeInEntryTree(entry, entry.Logger);
//        checkDataBase(entry.Logger);
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
        // copy the swlInput Hashtable, so it can be reset easily for the next run
        Hashtable copySqlInput = new Hashtable();
        copySqlInput.putAll(mSqlInput);
        // put some stuff from entry in the Hashtable
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

    /**
     *
    protected abstract boolean checkDataBase(LogWriter log);
     */

}
