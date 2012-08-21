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

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.StringTokenizer;

/**
 *
 */
public class SQLExecution {

    protected Connection mConnection = null;
    protected Statement mStatement = null;
    protected String mJdbcClass = null;
    protected String mDbURL = null;
    protected String mUser = null;
    protected String mPassword = null;
    protected boolean m_bConnectionOpen = false;
    protected boolean m_bDebug = false;


    /** Creates a new instance of SQLExecution
     * @param jdbcClass The jdbc class for the connection.
     * @param dbUrl The url of the database.
     * @param user The user for connecting the database.
     * @param password The password of throws user.
     */
    public SQLExecution(String jdbcClass, String dbUrl, String user, String password) {
        mJdbcClass = jdbcClass;
        mUser = user;
        mPassword = password;
        mDbURL = dbUrl;
    }

    /** Creates a new instance of SQLExecution with additional debug output.
     * @param jdbcClass The jdbc class for the connection.
     * @param dbUrl The url of the database.
     * @param user The user for connecting the database.
     * @param password The password of throws user.
     * @param debug Write debug information, if true.
     */
    public SQLExecution(String jdbcClass, String dbUrl, String user, String password, boolean debug) {
        mJdbcClass = jdbcClass;
        mUser = user;
        mPassword = password;
        mDbURL = dbUrl;
        m_bDebug = debug;
    }

    /**
     * Open a connection to the DataBase
     * @return True, if no error occurred.
     */
    public boolean openConnection() {
        if(m_bConnectionOpen) return true;
        try {
            Class.forName(mJdbcClass);
        } catch (ClassNotFoundException e) {
            System.err.println("Couldn't find jdbc driver : " + e.getMessage());
            return false;
        }

        try {
            // establish database connection
            mConnection = DriverManager.getConnection(
                                                mDbURL, mUser, mPassword);
            mStatement = mConnection.createStatement();
        }
        catch(java.sql.SQLException e) {
            System.err.println("Couldn't establish a connection: " + e.getMessage());
            return false;
        }
        m_bConnectionOpen = true;
        return true;
    }

    /**
     * Close the connection to the DataBase
     * @return True, if no error occurred.
     */
    public boolean closeConnection() {
        if (!m_bConnectionOpen) return true;
        try {
            // close database connection
            mStatement.close();
            mConnection.close();
        }
        catch(java.sql.SQLException e) {
            System.err.println("Couldn't close the connection: " + e.getMessage());
            return false;
        }
        m_bConnectionOpen = false;
        return true;
    }

    /**
     * Execute an sql command.
     * @param command The command to execute.
     * @param sqlInput Input values for the command.
     * @param sqlOutput The results of the command are put in this HashMap.
     * @return True, if no error occurred.
     */
    public boolean executeSQLCommand(String command, HashMap<String,Object> sqlInput, HashMap<String, String[]> sqlOutput)
                                        throws IllegalArgumentException {
        return executeSQLCommand(command, sqlInput, sqlOutput, false);
    }

    /**
     * Execute an sql command.
     * @param command The command to execute.
     * @param sqlInput Input values for the command.
     * @param sqlOutput The results of the command are put in this HashMap.
     * @param mergeOutputIntoInput The output of the result is put into the
     * sqlInput HashMap.
     * @return True, if no error occurred.
     */
    public boolean executeSQLCommand(String command, HashMap<String,Object> sqlInput, HashMap<String, String[]> sqlOutput, boolean mergeOutputIntoInput)
                                        throws IllegalArgumentException {
        if (sqlOutput == null) {
            sqlOutput = new HashMap<String, String[]>();
            // this has to be true, so the user of this method gets a return
            mergeOutputIntoInput = true;
            if (sqlInput == null) {
                System.out.println("sqlInput and sqlOutput are null: cannot return the results of the sql command.");
                return false;
            }
        }
        ArrayList<String> sqlCommand = new ArrayList<String>();
        sqlCommand.add("");
        boolean update = false;
        // synchronize all "$varname" occurrences in the command string with
        // values from input
        StringTokenizer token = new StringTokenizer(command, " ");
        while (token.hasMoreTokens()) {
            String originalKey = token.nextToken();
            // search for keys, beginning with "$"
            int index = originalKey.indexOf('$');
            if (index != -1) {
                // found key
                String pre = "";
                pre = originalKey.substring(0,index);
                // generate key: remove "$"
                String key = originalKey.substring(index+1);
                String post = "";
                // remove any endings the key might have
                while (key.endsWith(",") || key.endsWith("\'") ||
                       key.endsWith(";") || key.endsWith(")") ||
                       key.endsWith("\"")) {
                    post = key.substring(key.length()-1) + post;
                    key = key.substring(0, key.length()-1);
                }
                // look for key in the Hashtable
                if (sqlInput.containsKey(key)) {
                    // is there a value for the key?
                    Object in = sqlInput.get(key);
                    if (in instanceof String[]) {
                        // value is a String[]
                        String[]vals = (String[])in;
                        if (vals.length != sqlCommand.size() && sqlCommand.size() > 1) {
                            // size of this array and previous array(s) does not match
                            throw new IllegalArgumentException("The key '" + key + "' in command \n'"
                                    + command + "'\n has not the same value count as the keys before.");
                        }
                        // build up the commands
                        boolean addNewVals = (sqlCommand.size() == 1);
                        for (int i=0; i<vals.length; i++) {
                            String value = checkForQuotationMarks(vals[i]);
                            // add the values
                            if (addNewVals && i!=0) {
                                // all values until now were of type String, not String[], so now new values have to be added.
                                sqlCommand.add(i, sqlCommand.get(0) + " " + pre + value + post);
                            }
                            else {
                                // we already have vals.length commands (or are at the first command), so just add.
                                sqlCommand.set(i, sqlCommand.get(i) + " " + pre + value + post);
                            }
                        }
                    }
                    else {
                        // value is a String: no other possibility
                        String value = checkForQuotationMarks((String)sqlInput.get(key));
                        for (int i=0; i<sqlCommand.size(); i++) {
                            sqlCommand.set(i, sqlCommand.get(i) + " " + pre + value + post);
                        }
                    }
                }
                else {
                    // no input value found
                    throw new IllegalArgumentException ("The key '" + key + "' in command \n'"
                    + command + "'\n does not exist in the input values.");
                }
            }
            else {
                // token is not a key, just add it
                for (int i=0; i<sqlCommand.size(); i++)
                    sqlCommand.set(i, sqlCommand.get(i) + " " + originalKey);
                if (originalKey.equalsIgnoreCase("update") ||
                                    originalKey.equalsIgnoreCase("delete") ||
                                    originalKey.equalsIgnoreCase("insert")) {
                    update = true;
                }

            }
        }
        for (int i=0;i<sqlCommand.size(); i++) {
            execute(sqlCommand.get(i), sqlOutput, update);
            // merge output with input
            if (!update && mergeOutputIntoInput) {
                Iterator<String> keys = sqlOutput.keySet().iterator();
                while(keys.hasNext()) {
                    String key = keys.next();
                    String[]val = sqlOutput.get(key);
                    if (val != null && val.length != 0) {
                        if (val.length == 1)
                            sqlInput.put(key, val[0]);
                        else
                            sqlInput.put(key, val);
                    }
                }
            }
        }
        if (!update && sqlOutput == null)
            return false;
        return true;
    }

    /**
     * Execute any SQL command.
     * @param command The command.
     * @param update If true, it is a update/alter command instead of an select
     *          command
     * @param output A HashMap with the result.
     */
    private void execute(String command, HashMap<String, String[]> output, boolean update) {
        if (m_bDebug)
            System.out.println("Debug - SQLExecution - execute Command: " + command);
        try {
            if (update) {
                // make an update
                mStatement.executeUpdate(command);
            }
            else {
                // make a select: collect the result
                ResultSet sqlResult = mStatement.executeQuery(command);
                ResultSetMetaData sqlRSMeta = sqlResult.getMetaData();
                int columnCount = sqlRSMeta.getColumnCount();
                String[] columnNames = new String[columnCount];
                int countRows = 0;
                boolean goThroughRowsTheFirstTime = true;
                for(int i=1; i<=columnCount; i++) {
                    columnNames[i-1] = sqlRSMeta.getColumnName(i);
                    // initialize output
                    ArrayList<String> v = new ArrayList<String>();

                    sqlResult.beforeFirst();
                    while (sqlResult.next()) {
                        String value = sqlResult.getString(i);
                        v.add(value);
                        // the first time: count rows
                        if (goThroughRowsTheFirstTime)
                            countRows++;
                    }
                    // rows are counted
                    if (goThroughRowsTheFirstTime)
                        goThroughRowsTheFirstTime = false;

                    // put result in output HashMap
                    String[]s = new String[countRows];
                    s = v.toArray(s);
                    output.put(columnNames[i-1], s);
                    if (m_bDebug) {
                        if (i == 1) {
                            System.out.print("Debug - SQLExecution - Command returns: ");
                            System.out.print("row: " + columnNames[i-1] + "   vals: ");
                        }
                        for (int j=0; j<s.length; j++)
                            System.out.print(s[j] + " ");
                        if (i == columnCount - 1)
                            System.out.println();
                    }
                }
            }
        }
        catch (java.sql.SQLException e) {
            e.printStackTrace();
        }
    }

    /**
     * Replace <"> with <''> in the value Strings, or the command will fail.
     * @param checkString The String that is checked: a part of the command
     * @return The String, cleared of all quotation marks.
     */
    private String checkForQuotationMarks(String checkString) {
        String returnString = checkString;
        int quotIndex = 0;
        while ((quotIndex = returnString.indexOf('\"')) != -1) {
            String firstHalf = returnString.substring(0, quotIndex);
            String secondHalf = returnString.substring(quotIndex+1);
            returnString = firstHalf + "\'\'" + secondHalf;
        }
        return returnString;
    }

}
