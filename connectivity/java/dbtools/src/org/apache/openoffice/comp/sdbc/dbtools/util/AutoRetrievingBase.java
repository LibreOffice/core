/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/
package org.apache.openoffice.comp.sdbc.dbtools.util;

public class AutoRetrievingBase {
    private String autoRetrievingStatement; // contains the statement which should be used when query for automatically generated values
    private boolean autoRetrievingEnabled; // set to when we should allow to query for generated values

    public boolean isAutoRetrievingEnabled() {
        return autoRetrievingEnabled;
    }

    public String getAutoRetrievingStatement() {
        return autoRetrievingStatement;
    }

    public void setAutoRetrievingEnabled(boolean autoRetrievingEnabled) {
        this.autoRetrievingEnabled = autoRetrievingEnabled;
    }

    public void setAutoRetrievingStatement(String autoRetrivingStatement) {
        this.autoRetrievingStatement = autoRetrivingStatement;
    }

    /** transform the statement to query for auto generated values
     * @param  insertStatement
     *     The "INSERT" statement, is used to query for column and table names
     * @return
     *     The transformed generated statement.
     */
    public String getTransformedGeneratedStatement(String insertStatement) {
        Osl.ensure(autoRetrievingEnabled, "Illegal call here. isAutoRetrievingEnabled() is false!");
         insertStatement = insertStatement.toUpperCase();
         String statement = "";
         if (insertStatement.startsWith("INSERT")) {
             statement = autoRetrievingStatement;

             int index = 0;
             index = statement.indexOf("$column");
             if (index != -1) {
                 // we need a column
                 // FIXME: do something?
             }

             index = statement.indexOf("$table");
             if (index != -1) {
                 // we need a table
                 int intoIndex = insertStatement.indexOf("INTO ");
                 insertStatement = insertStatement.substring(intoIndex + 5);

                 int firstNonSpace;
                 for (firstNonSpace = 0; firstNonSpace < insertStatement.length();) {
                     int ch = insertStatement.codePointAt(firstNonSpace);
                     if (ch != ' ') {
                         break;
                     }
                     firstNonSpace += Character.charCount(ch);
                 }
                 insertStatement = insertStatement.substring(firstNonSpace);

                 int nextSpace = insertStatement.indexOf(' ');
                 String tableName;
                 if (nextSpace >= 0) {
                     tableName = insertStatement.substring(0, nextSpace);
                 } else {
                     tableName = "";
                 }

                 statement = statement.substring(0, index) + tableName + statement.substring(index + 6);
             }
         }
         return statement;
    }
}
