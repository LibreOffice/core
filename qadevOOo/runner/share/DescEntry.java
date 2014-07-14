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




package share;

/**
 *
 * Structure to describe the entries to be tested
 *
 */
public class DescEntry {

    /**
     * Contains the name used inside the framework
     */
    public String entryName;
    /**
     * Contains the name that may be used by state writers
     */
    public String longName;
    /**
     * Contains information if this entry is Optional
     */
    public boolean isOptional;
    /**
     * Contains information if this entry should be tested
     */
    public boolean isToTest;
    /**
     * Contains the information about the number of SubEntries
     */
    public int SubEntryCount;
    /**
     * Contains the SubEntries
     */
    public DescEntry[] SubEntries;

    /**
     * Contains information about the Type of the entry<br>
     * possible 'component', 'interface', 'service', 'method', 'property', 'unit'
     */
    public String EntryType;

    /**
     * Contains the ErrorMsg is something went wrong while gaining<br>
     * the description
     */
    public String ErrorMsg;

    /**
     * Contains information if errors occurred while gaining the Description
     */
    public boolean hasErrorMsg;

    /**
     * Contains the state for this entry
     */
    public String State = "UNKNOWN";

    /**
     * Contains the LogWriter to be used by the entry-test
     */

    public share.LogWriter Logger;

    /**
     * Contains an arbitrary set of parameters
     */

    public java.util.Hashtable UserDefinedParams = new java.util.Hashtable();

}
