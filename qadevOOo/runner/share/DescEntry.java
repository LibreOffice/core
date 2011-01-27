/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
