/*************************************************************************
 *
 *  $RCSfile: DataBaseOutProducer.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change:$Date: 2003-10-06 12:40:36 $
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
        findTypeInEntryTree(entry, entry.Logger);
//        checkDataBase(entry.Logger);
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
