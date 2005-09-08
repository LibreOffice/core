/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DescEntry.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 17:26:16 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
     * Contains information if errors occured while gaining the Description
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
