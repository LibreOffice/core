/*************************************************************************
 *
 *  $RCSfile: SQLExecution.java,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change:$Date: 2003-11-18 16:16:58 $
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

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.Statement;
import java.util.Enumeration;
import java.util.Hashtable;
import java.util.StringTokenizer;
import java.util.Vector;

/**
 *
 */
public class SQLExecution {

    protected Statement mStatement = null;
    protected String mJdbcClass = null;
    protected String mDbURL = null;
    protected String mUser = null;
    protected String mPassword = null;
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
     * @return True, if no error occured.
     */
    public boolean openConnection() {
        try {
            Class.forName(mJdbcClass);
        } catch (ClassNotFoundException e) {
            System.err.println("Couldn't find jdbc driver : " + e.getMessage());
            return false;
        }

        try {
            // establish database connection
            Connection connection = DriverManager.getConnection(
                                                mDbURL, mUser, mPassword);
            mStatement = connection.createStatement();
        }
        catch(java.sql.SQLException e) {
            System.err.println("Couldn't establish a connection: " + e.getMessage());
            return false;
        }

        return true;
    }


    /**
     * Execute an sql command.
     * @param command The command to execute.
     * @param sqlInput Input values for the command.
     * @param sqlOutput The results of the command are put in this Hashtable.
     * @return True, if no error occured.
     */
    public boolean executeSQLCommand(String command, Hashtable sqlInput, Hashtable sqlOutput)
                                        throws IllegalArgumentException {
        return executeSQLCommand(command, sqlInput, sqlOutput, false);
    }

    /**
     * Execute an sql command.
     * @param command The command to execute.
     * @param sqlInput Input values for the command.
     * @param sqlOutput The results of the command are put in this Hashtable.
     * @param mergeOutputIntoInput The output of the result is put into the
     * sqlInput Hashtable.
     * @return True, if no error occured.
     */
    public boolean executeSQLCommand(String command, Hashtable sqlInput, Hashtable sqlOutput, boolean mergeOutputIntoInput)
                                        throws IllegalArgumentException {
        Vector sqlCommand = new Vector();
        sqlCommand.add("");
        boolean update = false;
        // synchronize all "$varname" occurences in the command string with
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
                                sqlCommand.add(i, (String)sqlCommand.get(0) + " " + pre + value + post);
                            }
                            else {
                                // we already have vals.length commands (or are at the first command), so just add.
                                sqlCommand.set(i, (String)sqlCommand.get(i) + " " + pre + value + post);
                            }
                        }
                    }
                    else {
                        // value is a String: no other possibility
                        String value = checkForQuotationMarks((String)sqlInput.get(key));
                        for (int i=0; i<sqlCommand.size(); i++) {
                            sqlCommand.set(i, (String)sqlCommand.get(i) + " " + pre + value + post);
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
                    sqlCommand.set(i, (String)sqlCommand.get(i) + " " + originalKey);
                if (originalKey.equalsIgnoreCase("update") ||
                                    originalKey.equalsIgnoreCase("delete") ||
                                    originalKey.equalsIgnoreCase("insert")) {
                    update = true;
                }

            }
        }
        for (int i=0;i<sqlCommand.size(); i++) {
            sqlOutput = execute((String)sqlCommand.get(i), update);
            // merge output with input
            if (!update && mergeOutputIntoInput) {
                Enumeration keys = sqlOutput.keys();
                while(keys.hasMoreElements()) {
                    String key = (String)keys.nextElement();
                    String[]val = (String[])sqlOutput.get(key);
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
     * @return A Hashtable with the result.
     */
    private Hashtable execute(String command, boolean update) {
        Hashtable output = null;
        if (m_bDebug)
            System.out.println("Debug - SQLExecution - execute Command: " + command);
        try {
            if (update) {
                // make an update
                mStatement.executeUpdate(command);
            }
            else {
                // make a select: collect the result
                output = new Hashtable();
                ResultSet sqlResult = mStatement.executeQuery(command);
                ResultSetMetaData sqlRSMeta = sqlResult.getMetaData();
                int columnCount = sqlRSMeta.getColumnCount();
                String[] columnNames = new String[columnCount];
                int countRows = 0;
                boolean goThroughRowsTheFirstTime = true;
                for(int i=1; i<=columnCount; i++) {
                    columnNames[i-1] = sqlRSMeta.getColumnName(i);
                    // initialize output
                    Vector v = new Vector();

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

                    // put result in output Hashtable
                    String[]s = new String[countRows];
                    s = (String[])v.toArray(s);
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
            return null;
        }
        return output;
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
