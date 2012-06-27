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

    public java.util.HashMap<String,Object> UserDefinedParams = new java.util.HashMap<String,Object>();

}
